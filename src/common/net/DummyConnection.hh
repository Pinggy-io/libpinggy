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


#ifndef SRC_CPP_COMMON_NET_DUMMYCONNECTION_HH_
#define SRC_CPP_COMMON_NET_DUMMYCONNECTION_HH_

#include "NetworkConnection.hh"

namespace net {

DeclareStructWithSharedPtr(DummyMetaData);

DeclareClassWithSharedPtr(DummyConnection);

class DummyConnection: public virtual NetworkConnection
{
public:
    virtual
    ~DummyConnection();

    virtual void
    __Init() override;

    static bool
    CreateDummyConnection(DummyConnectionPtr conns[2], int bufferLen = 0);

    static std::tuple<DummyConnectionPtr, DummyConnectionPtr>
    CreateDummyConnectionPair(int bufferLen = 0);

    void
    SetPeerAddress(SocketAddressPtr addr)
                                { peerAddress = addr; }

    void
    SetLocalAddress(SocketAddressPtr addr)
                                { localAddress = addr; }

    void
    SetServerName(const tString &name)
                                { serverName = name; }

    virtual bool
    IsBlocking() override       { return false; }

    virtual std::string
    GetServerName() override    { return serverName; }

    virtual SocketAddressPtr
    GetPeerAddress() override   { return peerAddress; }

    virtual SocketAddressPtr
    GetLocalAddress() override  { return localAddress; }

    virtual sock_t
    GetFd() override            { return INVALID_SOCKET; }

    virtual std::string
    GetType() override          { return Type(); }

    static std::string
    Type()                      { return TO_STR(DummyConnection); }

    virtual uint32_t
    Flags() const override      { return flags; }

    virtual void
    SetFlags(uint32_t v)        { flags = v; }

    virtual ssize_t
    LastReturn() override       { return lastRet; }

    virtual bool
    ReassigntoLowerFd() override
                                { return false; }

    virtual void
    SetBlocking(bool block) override
                                { }

    virtual void
    SetSendTimeoutms(uint16_t timeout) override
                                { }

    virtual void
    SetRecvTimeoutms(uint16_t timeout) override
                                { }

    virtual bool
    EnableKeepAlive(int keepCnt, int keepIdle,
                    int keepIntvl, bool enable) override
                                { return false; }

    virtual int
    SslError(int ret) override;

    virtual std::tuple<ssize_t, RawDataPtr>
    Read(len_t nbyte, int flags = 0) override;

    virtual ssize_t
    Write(RawDataPtr rwData, int flags = 0) override;

    virtual std::tuple<ssize_t, RawDataPtr>
    Peek(len_t nbyte) override;

    virtual ssize_t
    Write(const void *buf, size_t nbyte, int flags) override;

    virtual ssize_t
    Read(void *buf, size_t nbyte, int flags) override;

    virtual ssize_t
    Peek(void *buf, size_t nbyte) override;

    virtual int
    ShutDown(int how) override;

    //==

    virtual bool
    TryAgain() override         { return tryAgain; }

    //PollableFD
    virtual void
    EnableWritePoll() override;

    virtual void
    DisableWritePoll() override;

    virtual void
    EnableReadPoll() override;

    virtual void
    DisableReadPoll() override;

    int16_t
    GetBufferSize();

    virtual tNetState
    GetState() override;

    virtual EventHandlerForPollableFdPtr
    GetPollEventHandler() override
                                { return pollEventObject; }

    virtual void
    ErasePollEventHandler() override
                                { pollEventObject = nullptr; }

protected:

    virtual int
    CloseNClear(tString) override;

    virtual void
    EventHandlerRegistered() override;

    virtual void
    EventHandlerDeregistered() override
                                { isReadPolling = false; isWritePolling = false; }

private:
    DummyConnection();

    void
    setReadPoll();

    void
    setWritePoll();

    void
    setReadPollForCounterPart();

    void
    setWritePollForCounterPart();

    DummyMetaDataPtr            reader;
    DummyMetaDataPtr            writer;
    uint32_t                    flags;
    size_t                      lastRet;
    size_t                      bufferLen;
    bool                        tryAgain;
    bool                        isReadPolling;
    bool                        isWritePolling;
    DummyConnectionWPtr         counterPart;
    tNetState                   netState;

    SocketAddressPtr            peerAddress;
    SocketAddressPtr            localAddress;
    tString                     serverName;
    EventHandlerForPollableFdPtr
                                pollEventObject;
};
DefineMakeSharedPtr(DummyConnection);

} /* namespace net */

#endif /* SRC_CPP_COMMON_NET_DUMMYCONNECTION_HH_ */
