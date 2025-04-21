/*
 * Copyright (C) 2025 PINGGY TECHNOLOGY PRIVATE LIMITED
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <platform/Log.hh>
#include <openssl/err.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <set>
#include "SslConnectionListener.hh"


namespace net {

struct NetConnContainerBioData {
    NetworkConnectionPtr        netConn;
};

static int
netConnBioCreate(BIO *bio)
{
    auto myBioData = new NetConnContainerBioData();
    if (!myBioData) {
        LOGE("Could not create custom bio data");
        return 0;
    }
    BIO_set_data(bio, myBioData); // Attach the custom context to the BIO
    BIO_set_init(bio, 1);
    return 1;
}

static int
netConnBioDestroy(BIO *bio)
{
    if (!bio) return 0; // Null check
    auto myBioData = (NetConnContainerBioData *)BIO_get_data(bio);
    if (myBioData) {
        // Free any custom resources here if necessary
        delete myBioData;
        BIO_set_data(bio, NULL); // Clear the context
    }
    BIO_set_init(bio, 0); // Mark the BIO as uninitialized
    LOGD("FREEING up bio")
    return 1; // Success
}

static int
netConnBioRead(BIO *bio, char *buf, int len)
{
    auto myBioData = (NetConnContainerBioData *)BIO_get_data(bio);
    Assert(myBioData);
    Assert(myBioData->netConn);

    int result = myBioData->netConn->Read(buf, len);
    if (result <= 0) {
        if (myBioData->netConn->TryAgain()) {
            BIO_set_retry_read(bio);
        }
    }
    return result;                          // Return bytes read or an error
}

static int
netConnBioWrite(BIO *bio, const char *buf, int len)
{
    auto myBioData = (NetConnContainerBioData *)BIO_get_data(bio);
    Assert(myBioData);
    Assert(myBioData->netConn);

    int result = myBioData->netConn->Write(buf, len);
    if (result <= 0) {
        if (myBioData->netConn->TryAgain()) {
            BIO_set_retry_write(bio);
        }
    }

    return result;                          // Return bytes written or an error
}

static long
netConnBioCtrl(BIO *bio, int cmd, long num, void *ptr)
{
    switch (cmd) {
        case BIO_CTRL_FLUSH:
            return 1; // Flush is a no-op for many BIOs
        default:
            return 0; // Handle other control commands as needed
    }
}

static BIO *
netConnBioNewBio(NetworkConnectionPtr netConn)
{
    BIO_METHOD *bio_method = BIO_meth_new(BIO_TYPE_SOURCE_SINK, "custom accept bio");

    BIO_meth_set_create(bio_method, netConnBioCreate);
    BIO_meth_set_destroy(bio_method, netConnBioDestroy);
    BIO_meth_set_write(bio_method, netConnBioWrite);
    BIO_meth_set_read(bio_method, netConnBioRead);
    BIO_meth_set_ctrl(bio_method, netConnBioCtrl);

    BIO *bio = BIO_new(bio_method);
    if (bio) {
        auto myBioData = (NetConnContainerBioData *)BIO_get_data(bio);
        Assert(myBioData);
        Assert(myBioData->netConn == nullptr);

        myBioData->netConn = netConn;
    }

    return bio;
}

#define NEW_SSL_NETWORKCONNECTION_PTR(...) NewSslNetworkConnectionPtr(new SslNetworkConnection(__VA_ARGS__))

struct PendingSsl : virtual public pinggy::SharedObject {
    SSL                        *ssl;
    NetworkConnectionPtr        netConn;
    pinggy::VoidPtr             ptr;
};
DefineMakeSharedPtr(PendingSsl);



static int
serverNameCallback(SSL *ssl, int *al, void *arg)
{
    SslConnectionListner *sslCon = (SslConnectionListner *)arg;
    return sslCon->ServerNameCallback(ssl, al);
}

SslConnectionListner::SslConnectionListner():
        connectionListener(nullptr),
        initiated(false)
{
}

SslConnectionListner::SslConnectionListner(tString path):
        connectionListener(NewConnectionListnerImplPtr(path)),
        initiated(false)
{
}


SslConnectionListner::SslConnectionListner(port_t port, bool ipv6) :
        connectionListener(NewConnectionListnerImplPtr(port, ipv6)),
        initiated(false)
{
}

SslConnectionListner::SslConnectionListner(ConnectionListnerPtr connListener):
        connectionListener(connListener),
        initiated(false)
{
}

SslConnectionListner::~SslConnectionListner()
{
    if(defaultCtx) {
        SSL_CTX_free(defaultCtx);         /* I no longer require ssl context */
        defaultCtx = NULL;
    }
}

void
SslConnectionListner::InitiateSSL(tString keyPath, tString chainPath)
{
    // initServerCTX();
    loadDefaultCertificate(keyPath, chainPath); /* load certs */
    initiated = true;
}

bool
SslConnectionListner::StartListening()
{
//    InitiateSSL();
    if (!initiated)
        abort();
    if(connectionListener)
        return connectionListener->StartListening();
    else
        return true;
}

NetworkConnectionPtr
SslConnectionListner::Accept()
{
    return AcceptSsl();
}

SslNetworkConnectionPtr
SslConnectionListner::AcceptSsl()
{
    if (connectionListener) {
        auto netConn = connectionListener->Accept();
        if(netConn)
            return AcceptSsl(netConn);
        // SSL *ssl;
    }
    return nullptr;
}

SslNetworkConnectionPtr
SslConnectionListner::AcceptSsl(
        NetworkConnectionPtr netConn)
{
    if (defaultCtx) {
        SSL *ssl;
        ssl = SSL_new(defaultCtx);              /* get new SSL state with context */
        if (netConn->IsRelayed()) {
            auto bio = netConnBioNewBio(netConn);
            if (!bio) {
                LOGSSLE("Error while creating bio")
                SSL_free(ssl);
                netConn->CloseConn();
                return nullptr;
            }
            SSL_set_bio(ssl, bio, bio);
        } else {
            SSL_set_fd(ssl, netConn->GetFd());
        }


        if ( SSL_accept(ssl) < 0) {    /* do SSL-protocol accept */
            LOGSSLE("Error at SSL_accept: ");
            SSL_free(ssl);
            return nullptr;
        }
        auto netSsl = NEW_SSL_NETWORKCONNECTION_PTR(ssl, netConn);
        return netSsl;
    }
    return nullptr;
}

void
SslConnectionListner::acceptSslInsideThread(
    sock_t sock, SSL *ssl, pinggy::VoidPtr ptr)
{
    if ( SSL_accept(ssl) < 0) {    /* do SSL-protocol accept */
        LOGSSLE("Error at SSL_accept: ");
        SSL_free(ssl);
        acceptedNetworkConnections.push({sock, NULL, false, ptr});
        return;
    }

    mutex.lock();
    acceptedNetworkConnections.push({sock, ssl, true, ptr});
    mutex.unlock();
}

void
SslConnectionListner::AcceptSslWithThread(NetworkConnectionPtr netConn, pinggy::VoidPtr ptr)
{
    if(threadPoolPtr == nullptr) {
        LOGF("You need to register thread handler before you have.");
        abort();
    }
    if (!defaultCtx) {
        LOGF("SSL context it required");
        abort();
    }

    netConn->SetRecvTimeoutms(10000);

    SSL *ssl;
    ssl = SSL_new(defaultCtx);              /* get new SSL state with context */
    if (netConn->IsRelayed()) {
        auto bio = netConnBioNewBio(netConn);
        if (!bio) {
            LOGSSLE("Error while creating bio")
            SSL_free(ssl);
            netConn->CloseConn();
            return;
        }
        SSL_set_bio(ssl, bio, bio);
    } else {
        SSL_set_fd(ssl, netConn->GetFd());
    }
    std::function<void ()> f = [=] { acceptSslInsideThread(netConn->GetFd(), ssl, ptr); };
    threadPoolPtr->QueueJob(f);
    acceptedNetConnInsideThread[netConn->GetFd()] = netConn;
}

len_t
SslConnectionListner::handleFDWPtr(PollableFDPtr pollableFD, pinggy::VoidPtr ptr)
{
    auto netConn = pollableFD->DynamicPointerCast<NetworkConnection>();
    auto sslPtr = ptr->DynamicPointerCast<PendingSsl>();
    auto ret = SSL_accept(sslPtr->ssl);
    switch (ret)
    {
    case 0:
        LOGI("Cannot accept as connection closed: ", netConn->GetPeerAddress());
        netConn->DeregisterFDEvenHandler();
        netConn->CloseConn();
        break;
    case 1:
        {
            netConn->DeregisterFDEvenHandler();
            auto newNetConn = NEW_SSL_NETWORKCONNECTION_PTR(sslPtr->ssl, sslPtr->netConn);
            LOGT("Accepted as ssl: fd:", newNetConn->GetFd());
            handler->HandleAcceptedSslConnection(newNetConn, sslPtr->ptr);
        }
        break;
    default:
        {
            auto sslErr = SSL_get_error(sslPtr->ssl, ret);
            switch (sslErr)
            {
            case SSL_ERROR_WANT_READ:
                LOGT("Enabling READPOLL", netConn->GetFd());
                // netConn->EnableReadPoll();
                netConn->DisableWritePoll();
                return ret;
            case SSL_ERROR_WANT_WRITE:
                LOGT("Enabling WRITE", netConn->GetFd());
                netConn->EnableWritePoll();
                // netConn->DisableReadPoll();
                return ret;
            default:
                ERR_clear_error();
                LOGI("Cannot accept as unknown error: ", sslErr, netConn->GetPeerAddress());
                netConn->DeregisterFDEvenHandler();
                netConn->CloseConn();
                SSL_free(sslPtr->ssl);
            }
        }
    }
    return ret;
}

len_t
SslConnectionListner::HandleFDReadWPtr(PollableFDPtr pollableFD, pinggy::VoidPtr ptr)
{
    return handleFDWPtr(pollableFD, ptr);
}

len_t
SslConnectionListner::HandleFDWriteWPtr(PollableFDPtr pollableFD, pinggy::VoidPtr ptr)
{
    return handleFDWPtr(pollableFD, ptr);
}

len_t
SslConnectionListner::HandleFDErrorWPtr(PollableFDPtr pollableFD, pinggy::VoidPtr ptr, int16_t)
{
    pollableFD->DeregisterFDEvenHandler();
    pollableFD->CloseConn();
    return 0;
}

void
SslConnectionListner::AcceptSslAsync(NetworkConnectionPtr netConn, pinggy::VoidPtr ptr)
{
    if (!defaultCtx) {
        LOGF("SSL context it required");
        abort();
    }

    netConn->SetRecvTimeoutms(10000);
    netConn->SetBlocking(false);

    SSL *ssl;
    ssl = SSL_new(defaultCtx);              /* get new SSL state with context */
    if (netConn->IsRelayed()) {
        auto bio = netConnBioNewBio(netConn);
        if (!bio) {
            LOGSSLE("Error while creating bio")
            SSL_free(ssl);
            netConn->CloseConn();
            return;
        }
        SSL_set_bio(ssl, bio, bio);
    } else {
        SSL_set_fd(ssl, netConn->GetFd());
    }
    auto sslPtr = NewPendingSslPtr();
    sslPtr->ssl = ssl;
    sslPtr->netConn = netConn;
    sslPtr->ptr = ptr;
    netConn->RegisterFDEvenHandler(thisPtr, sslPtr);
    LOGT("Registered for Async SSL", netConn->GetFd());
}

SSL_CTX *SslConnectionListner::initServerCTX()
{
    const SSL_METHOD *method;
    // OpenSSL_add_all_algorithms();  //Deprecated /* load & register all cryptos, etc. */
    // SSL_load_error_strings();  //Deprecated /* load all error messages */
    method = TLS_server_method();  /* create new server-method instance */
    SSL_CTX *ctx = SSL_CTX_new(method);   /* create new context from method */
    if ( ctx == NULL )
    {
        LOGSSLE("SSL Error at SSL_CTX_new: ");
        ABORT_WITH_MSG("SSL ERROR"); //This is okay for the time being as it is only in server
    }
    SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
    SSL_CTX_set_max_proto_version(ctx, TLS1_3_VERSION);

    return ctx;
}

SSL_CTX *SslConnectionListner::loadCertificate(tString keyPath, tString chainPath, SSL_CTX *ctx)
{
    if (ctx == NULL)
        ctx = initServerCTX();

    if (SSL_CTX_load_verify_locations(ctx, chainPath.c_str(), NULL) != 1) {
        LOGSSLE("Failed at SSL_CTX_load_verify_locations", chainPath);
        ABORT_WITH_MSG("SSL ERROR");
    }

    if (SSL_CTX_set_default_verify_file(ctx) != 1) {
        LOGSSLE("Failed at SSL_CTX_set_default_verify_paths");
        ABORT_WITH_MSG("SSL ERROR");
    }

    if ( SSL_CTX_use_certificate_chain_file(ctx, chainPath.c_str()) <= 0 )
    {
        LOGSSLE("Failed at SSL_CTX_use_certificate_chain_file");
        ABORT_WITH_MSG("SSL ERROR");
    }

    /* set the private key from KeyFile (may be the same as CertFile) */
    if ( SSL_CTX_use_PrivateKey_file(ctx, keyPath.c_str(), SSL_FILETYPE_PEM) <= 0 )
    {
        LOGSSLE("Failed at SSL_CTX_use_PrivateKey_file");
        ABORT_WITH_MSG("SSL ERROR");
    }

    /* verify private key */
    if ( !SSL_CTX_check_private_key(ctx) )
    {
        LOGSSLE("Private key does not match the public certificate.");
        ABORT_WITH_MSG("SSL ERROR");
    }

    auto certDet = NewCertificateFileDetailPtr(keyPath, chainPath);
    certificatePaths[ctx] = certDet;
    return ctx;
}

void
SslConnectionListner::loadDefaultCertificate(tString keyPath, tString chainPath) {
    defaultCtx = loadCertificate(keyPath, chainPath);
    auto ret = SSL_CTX_set_tlsext_servername_callback(defaultCtx, serverNameCallback);
    if (ret) {
        SSL_CTX_set_tlsext_servername_arg(defaultCtx, this);
    }
    registerContext(defaultCtx);
}

std::vector<tString>
SslConnectionListner::getSeverNames(SSL_CTX *ctx)
{
    std::vector<tString> serverNames;
    SSL* ssl = SSL_new(ctx);
    X509* cert = NULL;

    if (!ssl) {
        return serverNames;
    }
    cert = SSL_get_certificate(ssl);
    if (cert == NULL) {
        SSL_free(ssl);
        LOGE("Could not find cert");
        return serverNames;
    }
    //Get Common Name
    tString CN = "";
    X509_NAME *subjectName = X509_get_subject_name(cert);
    if (subjectName) {
        int commonNameIndex = X509_NAME_get_index_by_NID(subjectName, NID_commonName, -1);
        if (commonNameIndex >= 0) {
            X509_NAME_ENTRY* commonNameEntry = X509_NAME_get_entry(subjectName, commonNameIndex);
            if (commonNameEntry) {
                ASN1_STRING* commonNameAsn1 = X509_NAME_ENTRY_get_data(commonNameEntry);
                if (commonNameAsn1) {
                    char *commonName;
                    int commonNameLength = ASN1_STRING_to_UTF8((unsigned char **)&commonName, commonNameAsn1);
                    if (commonNameLength >= 0) {
                        commonName[commonNameLength] = '\0';
                        CN = tString(commonName);
                        serverNames.push_back(CN);
                        LOGT("CN", commonName);
                        OPENSSL_free(commonName);
                    }
                }
            }
        }
    }

    GENERAL_NAMES* sanNames = (GENERAL_NAMES*)X509_get_ext_d2i(cert, NID_subject_alt_name, NULL, NULL);
    if (sanNames) {
        int count = sk_GENERAL_NAME_num(sanNames);
        for (int i = 0; i < count; i++) {
            GENERAL_NAME* name = sk_GENERAL_NAME_value(sanNames, i);
            if (name->type == GEN_DNS) {
                ASN1_IA5STRING* dnsName = name->d.dNSName;
                if (dnsName) {
                    char *sn;
                    int snLen = ASN1_STRING_to_UTF8((unsigned char **)&sn, dnsName);
                    if(snLen > 0) {
                        sn[snLen] = 0;
                        tString SN = tString(sn);
                        if (SN != CN)
                            serverNames.push_back(SN);
                        OPENSSL_free(sn);
                    }
                }
            }
        }
        GENERAL_NAMES_free(sanNames);
    }

    SSL_free(ssl);
    return serverNames;
}

void
SslConnectionListner::AddCertificate(tString keyPath, tString chainPath)
{
    auto ctx = loadCertificate(keyPath, chainPath);
    if (ctx)
        registerContext(ctx);
}

void
SslConnectionListner::registerContext(SSL_CTX *ctx)
{
    for(auto serverName : getSeverNames(ctx)) {
        if (serverName.at(0) == '*') {
            auto pos = serverName.find('.');
            if (pos == serverName.npos)
                continue;
            auto sn = serverName.substr(pos+1);
            if (wildCardSslCtxs.find(sn) != wildCardSslCtxs.end()) {
                LOGI("Certificate for the serverName `" + serverName+ "`(`" + sn +"`) already exists. Removing it.");
                wildCardSslCtxs.erase(serverName);
            }
            LOGI("Loaded certificate for ", serverName, "as", sn);
            wildCardSslCtxs[sn] = ctx;
        } else {
            if (sslCtxs.find(serverName) != sslCtxs.end()) {
                LOGI("Certificate for the serverName `" + serverName+ "` already exists. Removing it.");
                sslCtxs.erase(serverName);
            }
            LOGI("Loaded certificate for ", serverName);
            sslCtxs[serverName] = ctx;
        }
    }
}

bool
SslConnectionListner::ReloadCertificates()
{
    reloadMutex.lock();
    for(auto detail :  certificatePaths) {
        auto ctx = detail.first;
        auto certDet = detail.second;
        if (certDet->IsModified(true)) {
            loadCertificate(certDet->GetKeyPath(), certDet->GetCertPath(), ctx);
            LOGI("Reloaded", certDet->GetCertPath());
        } else {
            LOGT("Not reloaded", certDet->GetCertPath());
        }
    }
    reloadMutex.unlock();
    return true;
}

int
SslConnectionListner::ServerNameCallback(SSL *ssl, int *)
{
    if(!ssl)
        return SSL_TLSEXT_ERR_OK;

    const char* sn = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);

    if (!sn || !sn[0]) {
        LOGE("Server name not found. using default certificate");
        return SSL_TLSEXT_ERR_OK;
    }
    auto serverName = NormalizeDomainName(tString(sn));
    bool found = false;
    reloadMutex.lock();
    if (sslCtxs.find(serverName) != sslCtxs.end()) {
        auto ctx = sslCtxs[serverName];
        if (ctx) {
            LOGI("Changed certificate for serverName: `"+serverName+"`");
            SSL_set_SSL_CTX(ssl, ctx);
            found = true;
        }
    }

    tString dns = serverName;
    while(!found) {
        auto pos = dns.find('.');
        if (pos == dns.npos) {
            break;
        }
        dns = dns.substr(pos+1);
        LOGT("Trying ", dns)
        if (wildCardSslCtxs.find(dns) != wildCardSslCtxs.end()) {;
            auto ctx = wildCardSslCtxs[dns];
            if(ctx) {
                LOGI("Changed certificate for serverName `"+serverName+"` as `*."+dns+"`");
                SSL_set_SSL_CTX(ssl, ctx);
                sslCtxs[serverName] = ctx; //Catching
                found = true;
            }
            break;
        }
        break; //only one wildcart allowed
    }

    if (!found) {
        LOGE("Certificate for `" + serverName + "` not found. Using the default");
    }

    reloadMutex.unlock();


    return SSL_TLSEXT_ERR_OK;
}

int
SslConnectionListner::CloseNClear(tString location)
{
//    DeregisterFDEvenHandler();
    if(connectionListener) {
        connectionListener->CloseConn(location);
        connectionListener = nullptr;
    }
    if(defaultCtx) {
        SSL_CTX_free(defaultCtx);         /* I no longer require ssl context */
        defaultCtx = NULL;
    }
    if(threadPoolPtr) {
        threadPoolPtr->Stop();
    }
    return 0;
}

bool
SslConnectionListner::IsListening()
{
    return defaultCtx != NULL;
}

sock_t
SslConnectionListner::GetFd()
{
    if(!connectionListener)
        return InValidSocket;
    return connectionListener->GetFd();
}

port_t
SslConnectionListner::GetListeningPort()
{
    if(connectionListener)
        return connectionListener->GetListeningPort();
    return 0;
}

tString
SslConnectionListner::GetListeningPath()
{
    if(connectionListener)
        return connectionListener->GetListeningPath();
    return "";
}

void
SslConnectionListner::RegisterAcceptSslHandler(
        common::PollControllerPtr pollController, SslAcceptEventHandlerPtr handler,
        bool thread)
{
    if(this->handler != nullptr) {
        LOGF("You can not register for 2nd time");
        abort();
    }
    if (thread) {
        if (threadPoolPtr == nullptr) {
            threadPoolPtr = common::NewThreadPoolPtr();
            threadPoolPtr->RegisterEventHandler(thisPtr);
            threadPoolPtr->Start();
        }
        pollController->RegisterHandler(threadPoolPtr);
    }
    this->handler = handler;
}

void
SslConnectionListner::DeregisterAcceptSslHandler(
        common::PollControllerPtr pollController)
{
    if (threadPoolPtr) {
        threadPoolPtr->DeregisterFDEvenHandler();
        this->handler = nullptr;
    }
}

void
SslConnectionListner::EventOccured()
{
    NetworkConnectionPtr conn = nullptr;
    sock_t sock = InValidSocket;
    SSL *ssl = NULL;
    bool ok = false;
    pinggy::VoidPtr ptr = nullptr;
    mutex.lock();
    if(!acceptedNetworkConnections.empty()) {
        auto [sock_, ssl_, ok_, ptr_] = acceptedNetworkConnections.front();
        acceptedNetworkConnections.pop();
        sock = sock_;
        ssl = ssl_;
        ok = ok_;
        ptr = ptr_;
    }
    mutex.unlock();

    if (!IsValidSocket(sock)) {
        Assert(false && "Invalid socket");
        return;
    }

    if (acceptedNetConnInsideThread.find(sock) == acceptedNetConnInsideThread.end()) {
        Assert(false && "Conn not found for the socket");
        SysSocketClose(sock);
        return;
    }

    conn = acceptedNetConnInsideThread[sock];
    acceptedNetConnInsideThread.erase(sock);

    if (!conn) {
        Assert(false && "Conn is not there");
        return;
    }

    if (!ok) {
        conn->CloseConn();
        return;
    }


    conn->SetBlocking(false);

    auto newNetConn = NEW_SSL_NETWORKCONNECTION_PTR(ssl, conn);
    // newNetConn->SetPollController(netConn->GetPController());
    LOGT("Accepted as ssl: fd:", newNetConn->GetFd());
    handler->HandleAcceptedSslConnection(newNetConn, ptr);
}

} /* namespace net */
