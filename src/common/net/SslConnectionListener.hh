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


#ifndef CPP_SERVER_SSL_SSLConnectionListener_HH_
#define CPP_SERVER_SSL_SSLConnectionListener_HH_

#include <openssl/ssl.h>
#include <map>
#include <ctime>

#include "SslNetworkConnection.hh"
#include "ConnectionListener.hh"
#include <poll/ThreadPool.hh>
#include <utils/Utils.hh>

namespace net {

abstract class SslAcceptEventHandler : virtual public pinggy::SharedObject
{
public:
    virtual ~SslAcceptEventHandler(){}
    virtual void HandleAcceptedSslConnection(SslNetworkConnectionPtr sslConn, pinggy::VoidPtr ptr) = 0;
};
DeclareSharedPtr(SslAcceptEventHandler);

class SslConnectionListener: public virtual ConnectionListener, public common::ThreadPoolEventHandler
{
public:
    SslConnectionListener();

    SslConnectionListener(tString path);

    SslConnectionListener(port_t port, bool ipv6);

    SslConnectionListener(ConnectionListenerPtr connListener);

    virtual
    ~SslConnectionListener();

    virtual void
    InitiateSSL(tString keyPath, tString chainPath);

    virtual void
    InitiateSSLSelfSigned(tString domain);

    virtual void
    AddCertificate(tString keyPath, tString chainPath);

    virtual bool
    StartListening() override;

    virtual bool
    IsStarted() override { Assert(false); return false; }

    virtual NetworkConnectionPtr
    Accept() override;

    virtual SslNetworkConnectionPtr
    AcceptSsl();

    virtual SslNetworkConnectionPtr
    AcceptSsl(NetworkConnectionPtr netConn);

    virtual port_t
    GetListeningPort() override;

    virtual tString
    GetListeningPath() override;

    virtual void
    SetRecvTimeoutms(uint16_t timeout) override {connectionListener->SetRecvTimeoutms(timeout);}

    virtual void
    SetSendTimeoutms(uint16_t timeout) override {connectionListener->SetSendTimeoutms(timeout);}

    virtual void
    AcceptSslWithThread(NetworkConnectionPtr netConn, pinggy::VoidPtr ptr = nullptr);

    virtual void
    AcceptSslAsync(NetworkConnectionPtr netConn, pinggy::VoidPtr ptr = nullptr);

    virtual void
    RegisterAcceptSslHandler(common::PollControllerPtr pollController, SslAcceptEventHandlerPtr handler, bool thread);

    virtual void
    DeregisterAcceptSslHandler(common::PollControllerPtr pollController);

    virtual void
    EventOccured() override;

    virtual void
    CleanupBeforeExec() { CloseConn(); if(threadPoolPtr) { threadPoolPtr->StopAfterFork(); }}

    virtual void
    SetFlagsForChild(uint32_t flags) override { if (connectionListener) connectionListener->SetFlagsForChild(flags); }

    virtual uint32_t
    FlagsForChild() const override { return connectionListener ? connectionListener->FlagsForChild() : 0; }

    virtual int
    ServerNameCallback(SSL *ssl, int *);

    virtual bool
    ReloadCertificates();

    virtual tString
    GetType() override { return Type(); }

    static tString
    Type() { return TO_STR(SslConnectionListener); }

    virtual void
    SetBlocking(bool block = true) override { if(connectionListener) connectionListener->SetBlocking(block); }

    virtual bool
    IsBlocking() override { return connectionListener ? connectionListener->IsBlocking() : false; }

    virtual bool
    TryAgain() override { return connectionListener ? connectionListener->TryAgain() : false; }

    //FDEventHandler
    virtual len_t
    HandleFDReadWPtr(PollableFDPtr,  pinggy::VoidPtr ptr) override;

    virtual len_t
    HandleFDWriteWPtr(PollableFDPtr, pinggy::VoidPtr ptr) override;

    virtual len_t
    HandleFDErrorWPtr(PollableFDPtr, pinggy::VoidPtr ptr, int16_t) override;

    virtual PollableFDPtr GetOrig() override { return connectionListener->GetOrig(); }


    virtual void
    SetAcceptRawSocket() override
                                { if(connectionListener) connectionListener->SetAcceptRawSocket(); }

    virtual bool
    GetAcceptRawSocket() override
                                { return connectionListener ? connectionListener->GetAcceptRawSocket(): false; }

protected:
    virtual int
    CloseNClear(tString) override;

private:
    virtual void
    acceptSslInsideThread(sock_t, SSL *ssl, pinggy::VoidPtr ptr);

    len_t
    handleFDWPtr(PollableFDPtr, pinggy::VoidPtr ptr);

    SSL_CTX *
    initServerCTX();

    SSL_CTX *
    loadCertificate(tString keyPath, tString chainPath, SSL_CTX *ctx = NULL);

    void
    loadDefaultCertificate(tString keyPath, tString chainPath);

    void
    loadSelfSignedCertificate(tString domain);

    void
    registerContext(SSL_CTX *ctx);

    std::vector<tString>
    getSeverNames(SSL_CTX *);

    virtual bool
    IsListening() override;

    virtual sock_t
    GetFd() override;

    SSL_CTX                    *defaultCtx;
    std::map<tString, SSL_CTX *>
                                sslCtxs;
    std::map<tString, SSL_CTX *>
                                wildCardSslCtxs;
    ConnectionListenerPtr       connectionListener;
    std::map<SSL_CTX *, CertificateFileDetailPtr>
                                certificatePaths;

    common::ThreadPoolPtr       threadPoolPtr;
    SslAcceptEventHandlerPtr    handler;

    std::queue<std::tuple<sock_t, SSL *, bool, pinggy::VoidPtr> >
                                acceptedNetworkConnections;
    std::map<sock_t, net::NetworkConnectionPtr>
                                acceptedNetConnInsideThread;

    std::mutex                  mutex;
    std::mutex                  reloadMutex;

    bool                        initiated;
    EVP_PKEY                   *selfSignedPkey;
};

DefineMakeSharedPtr(SslConnectionListener);

} /* namespace net */

#endif /* CPP_SERVER_SSL_SSLConnectionListener_HH_ */
