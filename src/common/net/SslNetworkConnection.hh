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


#ifndef CPP_COMMON_SSLNETWORKCONNECTION_HH_
#define CPP_COMMON_SSLNETWORKCONNECTION_HH_

#include "NetworkConnection.hh"
#include <utils/Utils.hh>


#include <openssl/ssl.h>


namespace net {

DeclareClassWithSharedPtr(SslConnectionListner);
// DeclareClassWithSharedPtr(SslNetworkConnection);

class SslNetworkConnection: public NetworkConnection
{
public:
    SslNetworkConnection(NetworkConnectionPtr netConn, tString serverName);

    virtual
    ~SslNetworkConnection();

    virtual void
    ShowClientCertificate() final;

    virtual void
    ShowServerCertificate() final;

    virtual void
    SetBaseCertificate(tString pem) final;

    virtual void
    Connect() final;

    virtual ssize_t
    Read(void *data, size_t len, int flags = 0) override;

    virtual ssize_t
    Peek(void *buf, size_t nbyte) override;

    virtual ssize_t
    Write(const void *data, size_t len, int flags = 0) override;

    virtual ssize_t
    Write(RawDataPtr rwData, int flags = 0) override;

    virtual ssize_t
    LastReturn() override       { return lastReturn; }

    virtual int
    SslError(int ret) override;

    virtual sock_t
    GetFd() override            { if(netConn) return netConn->GetFd(); return InValidSocket;}

    virtual void
    SetRecvTimeoutms(uint16_t timeout) override;

    virtual void
    SetSendTimeoutms(uint16_t timeout) override;

    virtual int
    ShutDown(int how) override  {return netConn->ShutDown(how);}

    virtual tString
    GetServerName() override;

    virtual SocketAddressPtr
    GetPeerAddress() override   { return (netConn) ? netConn->GetPeerAddress() : nullptr; }

    virtual SocketAddressPtr
    GetLocalAddress() override  { return (netConn) ? netConn->GetLocalAddress() : nullptr; }

    virtual bool
    EnableKeepAlive(int keepCnt, int keepIdle, int keepIntvl, bool enable = true) override
                                { return (netConn) ? netConn->EnableKeepAlive(keepCnt, keepIdle, keepIntvl, enable) : false; }

    virtual bool
    ReassigntoLowerFd() override
                                { return (netConn) ? netConn->ReassigntoLowerFd() : false; }

    virtual uint32_t
    Flags() const override      { return (netConn) ? netConn->Flags() : 0; }

    virtual tString
    GetType() override          { return Type(); }

    static tString
    Type()                      { return TO_STR(SslNetworkConnection); }

    virtual void
    SetBlocking(bool block = true) override
                                { if(netConn) netConn->SetBlocking(block); }

    virtual bool
    IsBlocking() override       { return netConn ? netConn->IsBlocking() : false; }

    virtual bool
    TryAgain() override         { return tryAgain; }

    virtual PollableFDPtr
    GetOrig() override          { return netConn->GetOrig(); }

    virtual tNetState
    GetState() override         { return netConn->GetState().NewWithSsl(); }

    class SslNetworkConnectionException: public std::exception {
    public:
        virtual NetworkConnectionPtr
        getNetConn() = 0;

        virtual tString
        getType() = 0;
    };
#define CustomeException(_name)                                                     \
    class _name##Exception : public SslNetworkConnectionException {                 \
    public:                                                                         \
        explicit                                                                    \
        _name##Exception(const tString& message) :                                  \
                netConn_(nullptr), message_(message)                                \
                                {}                                                  \
        explicit                                                                    \
        _name##Exception(NetworkConnectionPtr netConn, const tString& message) :    \
                netConn_(netConn), message_(message)                                \
                                {}                                                  \
        virtual const char*                                                         \
        what() const noexcept override                                              \
                                { return message_.c_str(); }                        \
        virtual NetworkConnectionPtr                                                \
        getNetConn() override   { return netConn_; }                                \
        virtual tString                                                             \
        getType() override      { return APP_CONVERT_TO_STRING(_name##Exception); } \
    private:                                                                        \
        NetworkConnectionPtr    netConn_;                                           \
        tString                 message_;                                           \
    }
CustomeException(NotConnected);
CustomeException(NotValid);
CustomeException(NotPollable);
CustomeException(ClientSideConnection);
CustomeException(ServerSideConnection);
CustomeException(CannotSetSNI);
CustomeException(CannotConnect);
CustomeException(SslRead);
CustomeException(SslWrite);
CustomeException(Certificate);
#undef CustomeException

protected:
    virtual int
    CloseNClear(tString) override;

    virtual ssize_t
    writeFromCached();

private:
    SslNetworkConnection(SSL *ssl, sock_t fd);
    SslNetworkConnection(SSL *ssl, NetworkConnectionPtr netCon);

    void
    loadBaseCertificate(SSL_CTX *ctx);

    friend class                SslConnectionListner;

    SSL                        *ssl;
    NetworkConnectionPtr        netConn;
    ssize_t                     lastReturn;
    bool                        tryAgain;
    RawDataPtr                  writeBuffer;
    ssize_t                     wroteFromCached;
    bool                        connected;
    bool                        serverSide;
    tString                     sniServerName;
    tString                     rootCertificate;
    bool                        privateCtx;
};

DefineMakeSharedPtr(SslNetworkConnection);


} /* namespace net */

#endif /* CPP_COMMON_SSLNETWORKCONNECTION_HH_ */
