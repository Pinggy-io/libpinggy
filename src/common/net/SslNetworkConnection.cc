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

namespace net {

SslNetworkConnection::SslNetworkConnection(SSL *ssl, sock_t fd):
        ssl(ssl), netConn(NewNetworkConnectionImplPtr(fd)), lastReturn(0), wroteFromCached(0),
            connected(true), serverSide(true), privateCtx(false)
{
}

SslNetworkConnection::SslNetworkConnection(SSL *ssl,
        NetworkConnectionPtr netCon): ssl(ssl), netConn(netCon), lastReturn(0), wroteFromCached(0),
            connected(true), serverSide(true), privateCtx(false)
{
    if (netConn == nullptr || !netConn->IsValid())
        throw NotValidException(netConn, "netConn is not valid");
    if (netConn->IsSsl())
        throw NotValidException(netConn, "netConn already ssl");
    if (!netConn->IsPollable())
        throw NotPollableException(netConn, "netConn already not pollable");
}

SslNetworkConnection::SslNetworkConnection(NetworkConnectionPtr netConn,
        tString serverName): ssl(NULL), netConn(netConn), lastReturn(0), wroteFromCached(0),
            connected(false), serverSide(false), sniServerName(serverName), privateCtx(false)
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

void
SslNetworkConnection::Connect()
{
    if (serverSide)
        throw ServerSideConnectionException("Attempting connect call from server side connection");
    if (connected)
        throw CannotConnectException("Attempting connect call from already established connection");

    auto method = TLS_client_method();  /* Create new client-method instance */
    auto ctx = SSL_CTX_new(method);   /* Create new context */
    if ( ctx == NULL )
    {
        LOGSSLF("SSL_CTX_new");
        throw CannotConnectException("Cannot create new context");
    }
    privateCtx = true;
    SSL_CTX_set_min_proto_version(ctx, TLS1_3_VERSION);
    SSL_CTX_set_max_proto_version(ctx, TLS1_3_VERSION);

    loadBaseCertificate(ctx);

    ssl = SSL_new(ctx);
    if (ssl == NULL) {
        throw CannotConnectException("Cannot create new ssl object");
    }

    SSL_set_fd(ssl, netConn->GetFd());

    if (!SSL_set_tlsext_host_name(ssl, sniServerName.c_str())) {
        LOGSSLE("Cannot set sni");
        // SSL_free(ssl);
        // ssl = NULL;
        // SSL_CTX_free(ctx);
        // ctx = NULL;
        throw CannotSetSNIException("Cannot set sni");
    }

    // Establish a secure connection
    auto ret = SSL_connect(ssl);
    if (ret <= 0) {
        LOGSSLE("Error while initiation ssl");
        // SSL_free(ssl);
        // SSL_CTX_free(ctx);
        throw CannotConnectException("Cannot perform ssl connect");
    }
    connected = true;
}

ssize_t
SslNetworkConnection::Read(void *data, size_t len, int flags)
{
    if (!connected)
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
    if (!connected)
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
    if (!connected)
        throw SslWriteException("Ssl connection is not established");
    if (writeBuffer) {
        if (wroteFromCached)
            ABORT_WITH_MSG("wroteFromCached should not be non-zero");
        auto ret = writeFromCached();
        if (ret <= 0) {
            LOGT("Wrror while writing from cache")
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
    if (!connected)
        throw SslWriteException("Ssl connection is not established");
    ABORT_WITH_MSG("Deprecated: use rawdata based function");
    if (writeBuffer) {
        if (wroteFromCached)
            ABORT_WITH_MSG("wroteFromCached should not be non-zero");
        auto ret = writeFromCached();
        if (ret <= 0) {
            LOGT("Wrror while writing from cache")
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
    if(netConn)
        return netConn->CloseConn(location);
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
SslNetworkConnection::loadBaseCertificate(SSL_CTX *ctx)
{
    if (rootCertificate.empty())
        return;

    BIO *bio;
    X509 *cert;

    bio = BIO_new_mem_buf((void*)rootCertificate.c_str(), (int)rootCertificate.length());
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

} /* namespace net */
