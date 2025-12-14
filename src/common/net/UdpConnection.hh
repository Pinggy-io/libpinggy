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

#ifndef _SRC_COMMON_NET_UDPCONNECTION_HH_
#define _SRC_COMMON_NET_UDPCONNECTION_HH_

#include "NetworkConnection.hh"

namespace net
{

class UdpConnection : public NetworkConnection
{
public:
    virtual
    ~UdpConnection(){}

    virtual ssize_t
    Peek(void *buf, size_t nbyte) override
                                { return -1; }

    virtual int
    SslError(int ret) override;

    virtual tString
    GetServerName() override    { return ""; } //return empty string incase of plaintext connection, sni for ssl
};
DeclareSharedPtr(UdpConnection);

class UdpConnectionImpl : public UdpConnection
{
public:
    UdpConnectionImpl(tString host, tString port);

    virtual
    ~UdpConnectionImpl()        { }

    virtual void
    __Init() override;

    virtual ssize_t
    Read(void *buf, size_t nbyte, int flags = 0) override
                                {ABORT_WITH_MSG("Not Allowed"); return -1;}

    virtual ssize_t
    Write(const void *buf, size_t nbyte, int flags = 0) override
                                {ABORT_WITH_MSG("Not Allowed"); return -1;}

    virtual ssize_t
    Peek(void *buf, size_t nbyte) override
                                {ABORT_WITH_MSG("Not Allowed"); return -1;}

    virtual std::tuple<ssize_t, RawDataPtr>
    Read(len_t nbyte, int flags = 0) override;

    virtual ssize_t
    Write(RawDataPtr rwData, int flags = 0) override;

    virtual std::tuple<ssize_t, RawDataPtr>
    Peek(len_t nbyte) override;

    virtual ssize_t
    LastReturn() override       { return lastReturn; }

    virtual sock_t
    GetFd() override            { return fd; }

    virtual void
    SetRecvTimeoutms(uint16_t timeout) override
                                { set_recv_timeout_ms(fd, timeout); }

    virtual void
    SetSendTimeoutms(uint16_t timeout) override
                                { set_send_timeout_ms(fd, timeout); }

    virtual int
    SslError(int ret) override;

    virtual int
    ShutDown(int how) override
                                { return app_shutdown(fd, how); }

    virtual tString
    GetType() override          { return Type(); }

    static tString
    Type()                      { return TO_STR(UdpConnectionImpl); }

    virtual tString
    GetServerName() override    { return ""; }

    virtual SocketAddressPtr
    GetPeerAddress() override   { return peerAddress; }

    virtual SocketAddressPtr
    GetLocalAddress() override;

    virtual bool
    EnableKeepAlive(int keepCnt, int keepIdleSec, int keepIntvl, bool enable = true) override;

    virtual bool
    ReassigntoLowerFd() override
                                { return ReassigntoLowerFdPtr(&fd); }

    virtual uint32_t
    Flags() const override      { return flags; }

    void
    SetFlags(uint32_t flags)    { this->flags = flags; }

    virtual void
    SetBlocking(bool block = true) override;

    virtual bool
    IsBlocking() override       { return blocking; }

    virtual bool
    TryAgain() override         { return tryAgain; }


    virtual tNetState
    GetState() override         { return netState; }

protected:
    virtual int
    CloseNClear(tString) override;

    virtual EventHandlerForPollableFdPtr
    GetPollEventHandler() override
                                { return pollEventObject; }

    virtual void
    ErasePollEventHandler() override
                                { pollEventObject = nullptr; }

private:
    sock_t                      fd;
    SocketAddressPtr            peerAddress;
    SocketAddressPtr            localAddress;
    uint32_t                    flags;
    ssize_t                     lastReturn;
    bool                        blocking;
    bool                        tryAgain;

    RawDataPtr                  rxData;
    RawDataPtr                  txData;
    tNetState                   netState;
    EventHandlerForPollableFdPtr
                                pollEventObject;

};
DefineMakeSharedPtr(UdpConnectionImpl);

} // namespace net

#endif //_SRC_COMMON_NET_UDPCONNECTION_HH_
