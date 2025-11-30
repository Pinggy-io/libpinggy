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


#include "SslNetworkConnection.hh"
#include <platform/Log.hh>
#include <openssl/err.h>
#include "SslNetConnBio.hh"

namespace net {

static tString
sslErrorToString(int err)
{
    auto syserr = app_get_errno();
    unsigned long detail = ERR_get_error(); // More specific internal error if available
    char buf[256];

    std::ostringstream oss;

    switch (err)
    {
        case SSL_ERROR_NONE:
            return "SSL_ERROR_NONE: No error";

        case SSL_ERROR_WANT_READ:
            return "SSL_ERROR_WANT_READ: Operation blocked (need to read more data)";

        case SSL_ERROR_WANT_WRITE:
            return "SSL_ERROR_WANT_WRITE: Operation blocked (need to write more data)";

        case SSL_ERROR_WANT_X509_LOOKUP:
            return "SSL_ERROR_WANT_X509_LOOKUP: Callback required for certificate lookup";

        case SSL_ERROR_ZERO_RETURN:
            return "SSL_ERROR_ZERO_RETURN: TLS connection closed cleanly";

        case SSL_ERROR_SYSCALL:
            if (detail != 0) {
                ERR_error_string_n(detail, buf, sizeof(buf));
                oss << "SSL_ERROR_SYSCALL: " << buf;
            } else {
                oss << "SSL_ERROR_SYSCALL: System call error: " << app_get_strerror(syserr);
            }
            return oss.str();

        case SSL_ERROR_SSL:
            if (detail != 0) {
                ERR_error_string_n(detail, buf, sizeof(buf));
                oss << "SSL_ERROR_SSL: " << buf;
            } else {
                oss << "SSL_ERROR_SSL: Unspecified protocol error";
            }
            return oss.str();

        default:
            oss << "Unknown SSL error code: " << err;
            return oss.str();
    }
}

class SslConnectFutureTaskHandler: virtual public SslConnectHandler
{
public:
    SslConnectFutureTaskHandler(common::TaskPtr onSuccess, common::TaskPtr onFailed): onSuccess(onSuccess), onFailed(onFailed) { }

    virtual void
    SslConnected(SslNetworkConnectionPtr sslConnPtr, pinggy::VoidPtr asyncConnectPtr) override
    {
        onSuccess->Fire();
    }

    virtual void
    SslConnectionFailed(SslNetworkConnectionPtr sslConnPtr, pinggy::VoidPtr asyncConnectPtr) override
    {
        onFailed->Fire();
    }

private:
    common::TaskPtr             onSuccess;
    common::TaskPtr             onFailed;
};
DefineMakeSharedPtr(SslConnectFutureTaskHandler);

SslNetworkConnection::SslNetworkConnection(SSL *ssl, sock_t fd):
        ssl(ssl),
        netConn(NewNetworkConnectionImplPtr(fd)),
        lastReturn(0),
        wroteFromCached(0),
        connected(true),
        serverSide(true),
        privateCtx(false),
        asyncConnectCompleted(true),
        minTlsVersion(TLS1_3_VERSION),
        maxTlsVersion(TLS1_3_VERSION)
{
}

SslNetworkConnection::SslNetworkConnection(SSL *ssl, NetworkConnectionPtr netCon):
        ssl(ssl),
        netConn(netCon),
        lastReturn(0),
        wroteFromCached(0),
        connected(true),
        serverSide(true),
        privateCtx(false),
        asyncConnectCompleted(true),
        minTlsVersion(TLS1_3_VERSION),
        maxTlsVersion(TLS1_3_VERSION)
{
    if (netConn == nullptr || !netConn->IsValid())
        throw NotValidException(netConn, "netConn is not valid");
    if (!netConn->IsConnected())
        throw NotConnectedException(netConn, "netConn is not connected");
    if (netConn->IsSsl())
        throw NotValidException(netConn, "netConn already ssl");
    if (!netConn->IsPollable())
        throw NotPollableException(netConn, "netConn already not pollable");
}

SslNetworkConnection::SslNetworkConnection(NetworkConnectionPtr netConn, tString serverName):
        ssl(NULL),
        netConn(netConn),
        lastReturn(0),
        wroteFromCached(0),
        connected(false),
        serverSide(false),
        sniServerName(serverName),
        privateCtx(false),
        asyncConnectCompleted(true),
        minTlsVersion(TLS1_3_VERSION),
        maxTlsVersion(TLS1_3_VERSION)
{
}

SslNetworkConnection::~SslNetworkConnection()
{
    if(ssl) {
        auto ctx = SSL_get_SSL_CTX(ssl);
        SSL_free(ssl);         /* release SSL state */
        if (privateCtx)
            SSL_CTX_free(ctx);
        ssl = NULL;
    }
}

void
SslNetworkConnection::ShowClientCertificate()
{
    if (!serverSide)
        throw ClientSideConnectionException(thisPtr, "this is client side connection");
    X509 *cert;
    char *line;
    cert = SSL_get_peer_certificate(ssl); /* Get certificates (if available) */
    if ( cert != NULL )
    {
        LOGI( "Server certificates:" );
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        LOGI( "Subject: " << line );
        free(line);
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        LOGI( "Issuer " << line );
        free(line);
        X509_free(cert);
    }
    else
        LOGI( "No certificates." );
}

void
SslNetworkConnection::ShowServerCertificate()
{
    if (serverSide)
        throw ServerSideConnectionException(thisPtr, "this is server side connection");
    if (!connected)
        throw NotConnectedException(thisPtr, "ssl not connected");

    X509 *cert;
    char line[1024];
    cert = SSL_get_peer_certificate(ssl); /* get the server's certificate */
    if ( cert != NULL )
    {
        LOGI( "Server certificates:" );
        X509_NAME_oneline(X509_get_subject_name(cert), line, 1024);
        LOGI( "Subject: " << line );
        //free(line);       /* free the malloc'ed string */
        X509_NAME_oneline(X509_get_issuer_name(cert), line, 1024);
        LOGI( "Issuer: " << line );
        //free(line);       /* free the malloc'ed string */
        X509_free(cert);     /* free the malloc'ed certificate copy */
    }
    else
        LOGI( "Info: No client certificates configured." );
}

void
SslNetworkConnection::SetBaseCertificate(tString pem)
{
    rootCertificate = pem;
}

#define CloseAndFreeFailedConnect(ssl, ctx, netConn)    \
    do{                                                 \
        SSL_free(ssl);                                  \
        freeCtxIfCreated(ctx);                          \
        netConn->CloseConn();                           \
        ssl = NULL;                                     \
    }while(0)

void
SslNetworkConnection::Connect(SSL_CTX *ctx)
{
    if (serverSide)
        throw ServerSideConnectionException("Attempting connect call from server side connection");
    if (connected)
        throw CannotConnectException("Attempting connect call from already established connection");

    ctx = createCtxIfNotPresent(ctx);

    ssl = SSL_new(ctx);
    if (ssl == NULL) {
        freeCtxIfCreated(ctx);
        netConn->CloseConn();
        throw CannotConnectException("Cannot create new ssl object");
    }

    if (netConn->IsRelayed() || netConn->IsDummy()) {
        auto bio = netConnBioNewBio(netConn);
        if (!bio) {
            LOGSSLE("Error while creating bio");
            CloseAndFreeFailedConnect(ssl, ctx, netConn);
            return;
        }
        SSL_set_bio(ssl, bio, bio);
    } else {
        SSL_set_fd(ssl, netConn->GetFd());
    }

    if (!SSL_set_tlsext_host_name(ssl, sniServerName.c_str())) {
        LOGSSLE("Cannot set sni");
        CloseAndFreeFailedConnect(ssl, ctx, netConn);
        throw CannotSetSNIException("Cannot set sni");
    }

    // Establish a secure connection
    auto ret = SSL_connect(ssl);
    if (ret <= 0) {
        LOGSSLE("Error while initiation ssl");
        auto sslErr = SSL_get_error(ssl, ret);
        auto reason = sslErrorToString(sslErr);
        CloseAndFreeFailedConnect(ssl, ctx, netConn);
        throw CannotConnectException("Cannot perform ssl connect: "+ reason);
    }
    connected = true;
}

void
SslNetworkConnection::ConnectAsync(common::TaskPtr onSuccess, common::TaskPtr onFailed, SSL_CTX *ctx, pinggy::VoidPtr data)
{
    auto handler = NewSslConnectFutureTaskHandlerPtr(onSuccess, onFailed);
    ConnectAsync(handler, data, ctx);
}

void
SslNetworkConnection::ConnectAsync(SslConnectHandlerPtr handler, pinggy::VoidPtr asyncConnectPtr, SSL_CTX *ctx)
{
    if (serverSide)
        throw ServerSideConnectionException("Attempting connect call from server side connection");
    if (connected)
        throw CannotConnectException("Attempting connect call from already established connection");

    asyncConnectCompleted = false;

    ctx = createCtxIfNotPresent(ctx);

    ssl = SSL_new(ctx);
    if (ssl == NULL) {
        freeCtxIfCreated(ctx);
        throw CannotConnectException("Cannot create new ssl object");
    }

    netConn->SetBlocking(false);

    if (netConn->IsRelayed() || netConn->IsDummy() || 1) {
        auto bio = netConnBioNewBio(netConn);
        if (!bio) {
            LOGSSLE("Error while creating bio");
            CloseAndFreeFailedConnect(ssl, ctx, netConn);
            return;
        }
        SSL_set_bio(ssl, bio, bio);
    } else {
        SSL_set_fd(ssl, netConn->GetFd());
    }

    if (!SSL_set_tlsext_host_name(ssl, sniServerName.c_str())) {
        LOGSSLE("Cannot set sni");
        throw CannotSetSNIException("Cannot set sni");
    }
    this->asyncConnectPtr = asyncConnectPtr;
    asyncConnectHandler = handler;
    // handleFD();
    this->RegisterFDEvenHandler(thisPtr);
    DisableReadPoll();
    EnableWritePoll();
    LOGE("Async connection started for fd: ", netConn->GetFd());
}

#undef CloseAndFreeFailedConnect

ssize_t
SslNetworkConnection::Read(void *data, size_t len, int flags)
{
    if (!connected || !asyncConnectCompleted)
        throw SslReadException("Ssl connection is not established");

    auto ret = (ssize_t)SSL_read(ssl, data, (int)len);
    LOGT("READ: ", GetFd(), "len:", len, "read:", ret);
    tryAgain = false;
    if (ret <= 0) {
        auto err = SSL_get_error(ssl, ret);
        if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
            tryAgain = true;
        } else if (err == SSL_ERROR_SYSCALL) {
            if(app_is_eagain()) {
                tryAgain = true;
            } else {
                LOGEE("Error while reading");
            }
        } else {
            LOGSSLE("Error while reading: sslerror: ", SSL_get_error(ssl, ret), "errno:", app_get_errno());
        }
    }
    lastReturn = ret;

    /*
        Ssl has a concept of msg. It process entire msg at one time. It only the requested amount,
        not more than that. It keeps the rest of the data in the buffer and return rest (or part of it)
        int the next read call. It do not look for more data unless `read ahead` is set using
        SSL_CTX_set_read_ahead() function. We have to make sure that it is not set.
    */
    LOGT("Pending Data?", SSL_pending(ssl));
    if (SSL_has_pending(ssl)) {
        RaiseDummyReadPoll();
    }

    return lastReturn;
}

ssize_t
SslNetworkConnection::Peek(void *data, size_t len)
{
    if (!connected || !asyncConnectCompleted)
        throw SslReadException("Ssl connection is not established");

    LOGT("PEEK: ", GetFd(), "len:", len);

    lastReturn = (ssize_t)SSL_peek(ssl, data, (int)len);
    tryAgain = false;
    if (lastReturn <= 0) {
        auto err = SSL_get_error(ssl, lastReturn);
        if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
            tryAgain = true;
        } else if (err == SSL_ERROR_SYSCALL) {
            if(app_is_eagain()) {
                tryAgain = true;
            }
        } else {
            LOGSSLE("Error while PEEKING, sslerror: ", SSL_get_error(ssl, lastReturn), "errno: ", app_get_errno());
        }
    }
    return lastReturn;
}

#define MIN(x,y) ((x) < (y) ? (x) : (y))

/*
    SSL_Write is weird in non-blocking mode. It is returns with SSL_ERROR_WANT_WRITE
    or SSL_ERROR_WANT_READ, it expect that next write call would be done with exactly
    same arguments. It is wierd, but it is what it is.

    Check: https://www.openssl.org/docs/man1.1.1/man3/SSL_write.html
 */


ssize_t
SslNetworkConnection::Write(RawDataPtr rwData, int flags)
{
    if (!connected || !asyncConnectCompleted)
        throw SslWriteException("Ssl connection is not established");

    if (writeBuffer) {
        if (wroteFromCached)
            ABORT_WITH_MSG("wroteFromCached should not be non-zero");
        auto ret = writeFromCached();
        if (ret <= 0) {
            LOGT("Wrror while writing from cache");
            return ret;
        }
        wroteFromCached = ret;
    }

    ssize_t length = rwData->Len;
    if (wroteFromCached) {
        ssize_t consumed = 0;
        if (wroteFromCached >= length) {
            consumed = length;
            wroteFromCached -= length;
        } else {
            consumed = wroteFromCached;
            wroteFromCached = 0;
        }
        LOGT("Consumed: ", consumed);
        return consumed;
    }

    writeBuffer = rwData->Slice(0);

    return writeFromCached();
}

ssize_t
SslNetworkConnection::Write(const void *data, size_t len, int flags)
{
    if (!connected || !asyncConnectCompleted)
        throw SslWriteException("Ssl connection is not established");

    ABORT_WITH_MSG("Deprecated: use rawdata based function");
    if (writeBuffer) {
        if (wroteFromCached)
            ABORT_WITH_MSG("wroteFromCached should not be non-zero");
        auto ret = writeFromCached();
        if (ret <= 0) {
            LOGT("Wrror while writing from cache");
            return ret;
        }
        wroteFromCached = ret;
    }

    char *cdata = (char *)data;
    ssize_t length = len;
    if (wroteFromCached) {
        ssize_t consumed = 0;
        if (wroteFromCached >= length) {
            consumed = length;
            wroteFromCached -= length;
        } else {
            consumed = wroteFromCached;
            cdata += wroteFromCached;
            length -= wroteFromCached;
            wroteFromCached = 0;
        }
        LOGT("Consumed: ", consumed);
        return consumed;
    }

    length = MIN(length, 2048);


    writeBuffer = NewRawDataPtr(cdata, length);

    return writeFromCached();
}

int
SslNetworkConnection::SslError(int len)
{
    return SSL_get_error(ssl, len);
}

int
SslNetworkConnection::CloseNClear(tString location)
{
    if(ssl) {
        auto ctx = SSL_get_SSL_CTX(ssl);
        SSL_free(ssl);         /* release SSL state */
        if (privateCtx)
            SSL_CTX_free(ctx);
        ssl = NULL;
    }
    if(netConn) {
        netConn->CloseConn(location);
        // netConn = nullptr;
    }
    return 0;
}

void
SslNetworkConnection::SetRecvTimeoutms(uint16_t timeout)
{
    if(netConn)
        netConn->SetRecvTimeoutms(timeout);
}

void
SslNetworkConnection::SetSendTimeoutms(uint16_t timeout)
{
    if(netConn)
        netConn->SetSendTimeoutms(timeout);
}

tString
SslNetworkConnection::GetServerName()
{
    auto name = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
    if (name)
        return NormalizeDomainName(tString(name));
    return "";
}


len_t
SslNetworkConnection::HandleFDRead(PollableFDPtr)
{
    handleFD();
    return 0;
}

len_t
SslNetworkConnection::HandleFDWrite(PollableFDPtr)
{
    handleFD();
    return 0;
}

len_t
SslNetworkConnection::HandleFDError(PollableFDPtr fdPtr, int16_t ecode)
{
    LOGD("Closing by `HandleFDErrorWTag` for fd: ", fdPtr->GetFd(), " errno: ", ecode);
    DeregisterFDEvenHandler();
    netConn->CloseConn();
    netConn = nullptr;
    // SSL_free(ssl);
    asyncConnectHandler->SslConnectionFailed(thisPtr, asyncConnectPtr);
    return 0;
}

ssize_t
SslNetworkConnection::writeFromCached()
{
    Assert (writeBuffer && writeBuffer->Len);

    lastReturn = SSL_write(ssl, writeBuffer->GetData(), writeBuffer->Len);
    tryAgain = false;
    if (lastReturn <= 0) {
        auto err = SSL_get_error(ssl, lastReturn);
        if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
            tryAgain = true;
        } else if (err == SSL_ERROR_SYSCALL) {
            if(app_is_eagain()) {
                tryAgain = true;
            }
        } else {
            LOGSSLE("Error while connection: ");
        }
        return lastReturn;
    }

    writeBuffer->Consume(lastReturn);

    if (writeBuffer->Len == 0) {
        writeBuffer = nullptr;
    }
    return lastReturn;
}

void
SslNetworkConnection::loadBaseCertificate(SSL_CTX *ctx, tString certificate)
{
    if (certificate.empty())
        return;

    BIO *bio;
    X509 *cert;

    bio = BIO_new_mem_buf((void*)certificate.c_str(), (int)certificate.length());
    if (!bio) {
        perror("Unable to create BIO for certificate");
        throw CertificateException("Cannot load base certificate");
    }

    cert = PEM_read_bio_X509(bio, NULL, 0, NULL);
    if (!cert) {
        LOGSSLF("Unable to create BIO for certificate");
        throw CertificateException("Unable to create BIO for certificate");
    }

    if (!SSL_CTX_add_client_CA(ctx, cert)) {
        LOGSSLF("Unable to add certificate to SSL context");
        throw CertificateException("Unable to use certificate in SSL context");
    }

    if (!SSL_CTX_use_certificate(ctx, cert)) {
        LOGSSLF("Unable to use certificate in SSL context");
        throw CertificateException("Unable to use certificate in SSL context");
    }

    X509_free(cert);
    BIO_free(bio);
}

len_t
SslNetworkConnection::handleFD()
{
    // Establish a secure connection
    LOGT("Handling connect");
    auto ret = SSL_connect(ssl);
    switch(ret) {
        case 0:
            DeregisterFDEvenHandler();
            asyncConnectHandler->SslConnectionFailed(thisPtr, asyncConnectPtr);
            netConn->CloseConn();
            netConn = nullptr;
            // SSL_free(ssl);
            LOGD("SSL connection failed: ", netConn->GetPeerAddress(), netConn->GetFd());
            return 0;
        case 1:
            connected = true;
            asyncConnectCompleted = true;
            DeregisterFDEvenHandler();
            asyncConnectHandler->SslConnected(thisPtr, asyncConnectPtr);
            LOGD("SSL connected: ", netConn->GetPeerAddress(), netConn->GetFd());
            return 1;
        default:
        {
            auto sslErr = SSL_get_error(ssl, ret);
            ERR_clear_error();
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
                LOGE("Cannot connect as unknown error: ", sslErr, netConn->GetPeerAddress(), netConn->GetFd());
                DeregisterFDEvenHandler();
                // netConn->CloseConn();
                // netConn = nullptr;
                // SSL_free(ssl);
                asyncConnectHandler->SslConnectionFailed(thisPtr, asyncConnectPtr);
            }
            return ret;
        }
    }
}

SSL_CTX *
SslNetworkConnection::createCtxIfNotPresent(SSL_CTX *ctx)
{
    if (!ctx) {
        privateCtx = true;
        ctx = SslNetworkConnection::CreateSslContext(minTlsVersion, maxTlsVersion, rootCertificate);
    }
    return ctx;
}

void
SslNetworkConnection::freeCtxIfCreated(SSL_CTX *&ctx)
{
    if (privateCtx && ctx) {
        SSL_CTX_free(ctx);
        ctx = NULL;
        privateCtx = false;
    }
}

SSL_CTX *
SslNetworkConnection::CreateSslContext(int minVersion, int maxVersion, tString certificate)
{
    auto method = TLS_client_method();  /* Create new client-method instance */
    SSL_CTX *ctx = SSL_CTX_new(method);   /* Create new context */
    if ( ctx == NULL )
    {
        LOGSSLF("SSL_CTX_new");
        throw CannotConnectException("Cannot create new context");
    }
    SSL_CTX_set_min_proto_version(ctx, minVersion);
    SSL_CTX_set_max_proto_version(ctx, maxVersion);

    loadBaseCertificate(ctx, certificate);
    return ctx;
}

} /* namespace net */
