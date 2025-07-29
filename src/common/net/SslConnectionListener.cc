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
#include "SslNetConnBio.hh"


namespace net {

static X509 *
create_self_signed_cert(EVP_PKEY* pkey, tString domain)
{
    X509 *x509 = X509_new();
    ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);

    X509_gmtime_adj(X509_get_notBefore(x509), 0);
    X509_gmtime_adj(X509_get_notAfter(x509), 31536000L); // 1 year

    X509_set_pubkey(x509, pkey);

    // Set subject and issuer name
    X509_NAME *name = X509_get_subject_name(x509);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC,
                               (unsigned char *)domain.c_str(), -1, -1, 0);
    // issuer = subject for self-signed
    X509_set_issuer_name(x509, name);

    // Sign it with our own private key
    X509_sign(x509, pkey, EVP_sha256());

    return x509;
}

static EVP_PKEY *
generate_key()
{
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
    EVP_PKEY *pkey = NULL;

    if (!ctx) {
        fprintf(stderr, "EVP_PKEY_CTX_new_id failed\n");
        return NULL;
    }

    if (EVP_PKEY_keygen_init(ctx) <= 0 ||
        EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 2048) <= 0 ||
        EVP_PKEY_keygen(ctx, &pkey) <= 0) {
        fprintf(stderr, "RSA key generation failed\n");
        EVP_PKEY_CTX_free(ctx);
        return NULL;
    }

    EVP_PKEY_CTX_free(ctx);
    return pkey;
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
    SslConnectionListener *sslCon = (SslConnectionListener *)arg;
    return sslCon->ServerNameCallback(ssl, al);
}

SslConnectionListener::SslConnectionListener():
        connectionListener(nullptr),
        initiated(false), selfSignedPkey(NULL)
{
}

SslConnectionListener::SslConnectionListener(tString path):
        connectionListener(NewConnectionListenerImplPtr(path)),
        initiated(false), selfSignedPkey(NULL)
{
}


SslConnectionListener::SslConnectionListener(port_t port, bool ipv6) :
        connectionListener(NewConnectionListenerImplPtr(port, ipv6)),
        initiated(false), selfSignedPkey(NULL)
{
}

SslConnectionListener::SslConnectionListener(ConnectionListenerPtr connListener):
        connectionListener(connListener),
        initiated(false), selfSignedPkey(NULL)
{
}

SslConnectionListener::~SslConnectionListener()
{
    if(defaultCtx) {
        SSL_CTX_free(defaultCtx);         /* I no longer require ssl context */
        defaultCtx = NULL;
    }
    if (selfSignedPkey) {
        EVP_PKEY_free(selfSignedPkey);
        selfSignedPkey = NULL;
    }
}

void
SslConnectionListener::InitiateSSL(tString keyPath, tString chainPath)
{
    if (initiated) {
        ABORT_WITH_MSG("You cannot initiate it twice");
    }
    // initServerCTX();
    loadDefaultCertificate(keyPath, chainPath); /* load certs */
    initiated = true;
}

void SslConnectionListener::InitiateSSLSelfSigned(tString domain)
{
    if (initiated) {
        ABORT_WITH_MSG("You cannot initiate it twice");
    }
    loadSelfSignedCertificate(domain);
    initiated = true;
}

bool
SslConnectionListener::StartListening()
{
//    InitiateSSL();
    if (!initiated) {
        ABORT_WITH_MSG("Initiate ssl listener before start listening");
    }
    if(connectionListener)
        return connectionListener->StartListening();
    else
        return true;
}

NetworkConnectionPtr
SslConnectionListener::Accept()
{
    return AcceptSsl();
}

SslNetworkConnectionPtr
SslConnectionListener::AcceptSsl()
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
SslConnectionListener::AcceptSsl(
        NetworkConnectionPtr netConn)
{
    if (defaultCtx) {
        SSL *ssl;
        ssl = SSL_new(defaultCtx);              /* get new SSL state with context */
        if (netConn->IsRelayed()) {
            auto bio = netConnBioNewBio(netConn);
            if (!bio) {
                LOGSSLE("Error while creating bio");
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
SslConnectionListener::acceptSslInsideThread(
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
SslConnectionListener::AcceptSslWithThread(NetworkConnectionPtr netConn, pinggy::VoidPtr ptr)
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
            LOGSSLE("Error while creating bio");
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
SslConnectionListener::handleFDWPtr(PollableFDPtr pollableFD, pinggy::VoidPtr ptr)
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
        SSL_free(sslPtr->ssl);
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
                netConn->EnableReadPoll();
                netConn->DisableWritePoll();
                return ret;
            case SSL_ERROR_WANT_WRITE:
                LOGT("Enabling WRITE", netConn->GetFd());
                netConn->EnableWritePoll();
                netConn->DisableReadPoll();
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
SslConnectionListener::HandleFDReadWPtr(PollableFDPtr pollableFD, pinggy::VoidPtr ptr)
{
    return handleFDWPtr(pollableFD, ptr);
}

len_t
SslConnectionListener::HandleFDWriteWPtr(PollableFDPtr pollableFD, pinggy::VoidPtr ptr)
{
    return handleFDWPtr(pollableFD, ptr);
}

len_t
SslConnectionListener::HandleFDErrorWPtr(PollableFDPtr pollableFD, pinggy::VoidPtr ptr, int16_t)
{
    pollableFD->DeregisterFDEvenHandler();
    pollableFD->CloseConn();
    return 0;
}

void
SslConnectionListener::AcceptSslAsync(NetworkConnectionPtr netConn, pinggy::VoidPtr ptr)
{
    if (!defaultCtx) {
        ABORT_WITH_MSG("SSL context it required");
    }
    if (!handler) {
        ABORT_WITH_MSG("Register accept handler first");
    }

    netConn->SetRecvTimeoutms(10000);
    netConn->SetBlocking(false);

    SSL *ssl;
    ssl = SSL_new(defaultCtx);              /* get new SSL state with context */
    if (netConn->IsRelayed() || netConn->IsDummy()) {
        auto bio = netConnBioNewBio(netConn);
        if (!bio) {
            LOGSSLE("Error while creating bio");
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

SSL_CTX *SslConnectionListener::initServerCTX()
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

SSL_CTX *SslConnectionListener::loadCertificate(tString keyPath, tString chainPath, SSL_CTX *ctx)
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
SslConnectionListener::loadDefaultCertificate(tString keyPath, tString chainPath) {
    defaultCtx = loadCertificate(keyPath, chainPath);
    auto ret = SSL_CTX_set_tlsext_servername_callback(defaultCtx, serverNameCallback);
    if (ret) {
        SSL_CTX_set_tlsext_servername_arg(defaultCtx, this);
    }
    registerContext(defaultCtx);
}

void SslConnectionListener::loadSelfSignedCertificate(tString domain)
{
    defaultCtx = initServerCTX();

    EVP_PKEY *pkey = generate_key();
    selfSignedPkey = pkey;
    X509 *cert = create_self_signed_cert(pkey, domain);

    // Use cert + key in memory
    SSL_CTX_use_certificate(defaultCtx, cert);
    SSL_CTX_use_PrivateKey(defaultCtx, pkey);

    // Verify key matches cert
    if (!SSL_CTX_check_private_key(defaultCtx)) {
        fprintf(stderr, "Private key does not match the certificate public key\n");
        return;
    }
}

std::vector<tString>
SslConnectionListener::getSeverNames(SSL_CTX *ctx)
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
SslConnectionListener::AddCertificate(tString keyPath, tString chainPath)
{
    if (selfSignedPkey) {
        ABORT_WITH_MSG("Additional certificates not allowed while using selfSigned certificate");
    }
    auto ctx = loadCertificate(keyPath, chainPath);
    if (ctx)
        registerContext(ctx);
}

void
SslConnectionListener::registerContext(SSL_CTX *ctx)
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
SslConnectionListener::ReloadCertificates()
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
SslConnectionListener::ServerNameCallback(SSL *ssl, int *)
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
        LOGT("Trying ", dns);
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
SslConnectionListener::CloseNClear(tString location)
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
SslConnectionListener::IsListening()
{
    return defaultCtx != NULL;
}

sock_t
SslConnectionListener::GetFd()
{
    if(!connectionListener)
        return InValidSocket;
    return connectionListener->GetFd();
}

port_t
SslConnectionListener::GetListeningPort()
{
    if(connectionListener)
        return connectionListener->GetListeningPort();
    return 0;
}

tString
SslConnectionListener::GetListeningPath()
{
    if(connectionListener)
        return connectionListener->GetListeningPath();
    return "";
}

void
SslConnectionListener::RegisterAcceptSslHandler(
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
SslConnectionListener::DeregisterAcceptSslHandler(
        common::PollControllerPtr pollController)
{
    if (threadPoolPtr) {
        threadPoolPtr->DeregisterFDEvenHandler();
        this->handler = nullptr;
    }
}

void
SslConnectionListener::EventOccured()
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
