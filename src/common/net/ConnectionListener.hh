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


#ifndef CPP_SERVER_SSL_CONNECTIONLISTNER_HH_
#define CPP_SERVER_SSL_CONNECTIONLISTNER_HH_

#include "NetworkConnection.hh"
#include <utils/Utils.hh>

namespace net {

DeclareClassWithSharedPtr(ConnectionListner);

class ConnectionListnerException: public std::exception, public virtual pinggy::SharedObject
{
public:
    ConnectionListnerException(tString message, ConnectionListnerPtr connListener) : message(message), connListener(connListener)
                                { }

    virtual ~ConnectionListnerException()
                                { }

    virtual const char*
    what() const noexcept override
                                { return message.c_str(); }

    virtual ConnectionListnerPtr
    GetListener()               { return connListener; }

private:
    tString                     message;
    ConnectionListnerPtr        connListener;
};

abstract class ConnectionListenerHandler: public virtual pinggy::SharedObject {
public:
    virtual
    ~ConnectionListenerHandler()
                                { }

    virtual void
    NewVisitor(NetworkConnectionPtr netCon) = 0;

    virtual void
    NewVisitorSocket(sock_t fd, ConnectionListnerPtr listener)
                                { throw ConnectionListnerException("Not implemented", listener); }

    virtual void
    ConnectionListenerClosed(ConnectionListnerPtr listener)
                                { }
};
DeclareClassWithSharedPtr(ConnectionListenerHandler);

class ConnectionListner: public virtual NetworkSocket, public virtual FDEventHandler {
public:
    ConnectionListner(): maxSeqAccepts(1)
                                { }

    virtual
    ~ConnectionListner()        { }

    virtual bool
    StartListening() = 0;

    virtual bool
    IsStarted() = 0;

    virtual sock_t
    AcceptSocket();

    virtual NetworkConnectionPtr
    Accept() = 0;

    virtual bool
    IsListening() = 0;

    virtual port_t
    GetListeningPort() = 0;

    virtual tString
    GetListeningPath() = 0;

    virtual void
    SetFlagsForChild(uint32_t flags) = 0;

    virtual uint32_t
    FlagsForChild() const = 0;

    virtual bool
    TryAgain() = 0;

    virtual void
    RegisterListenerHandler(ConnectionListenerHandlerPtr, len_t acceptConn) final;

    virtual void
    RegisterListenerHandler(common::PollControllerPtr, ConnectionListenerHandlerPtr, len_t acceptConn) final;

    virtual void
    SetMaxSeqAccepts(len_t maxSeqAccepts) final
                                { this->maxSeqAccepts = maxSeqAccepts; }

    virtual len_t
    HandleFDRead(PollableFDPtr) override;

    virtual len_t
    HandleFDError(PollableFDPtr, int16_t) override;

    virtual bool
    IsValid()                   { return IsValidSocket(GetFd()); }

    virtual tNetState
    GetState() override          { return tNetState(); }

private:
    ConnectionListenerHandlerPtr
                                eventHandler;
    len_t                       maxSeqAccepts;
};
DeclareSharedPtr(ConnectionListner);

class ConnectionListnerImpl: public ConnectionListner
{
public:
    ConnectionListnerImpl(sock_t fd);
    ConnectionListnerImpl(tString path);
    ConnectionListnerImpl(port_t port, bool ipv6);

    virtual
    ~ConnectionListnerImpl();

    virtual bool
    StartListening() override;

    virtual bool
    IsStarted() override        { return IsValidSocket(fd); }

    virtual sock_t
    AcceptSocket() override;

    virtual NetworkConnectionPtr
    Accept() override;

    virtual sock_t
    GetFd() override            { return fd; }

    virtual port_t
    GetListeningPort() override { return port; }

    virtual tString
    GetListeningPath() override { return socketPath; }

    virtual bool
    IsListening() override      { return IsValidSocket(fd); }

    virtual void
    SetRecvTimeoutms(uint16_t timeout) override
                                { set_recv_timeout_ms(fd, timeout); }

    virtual void
    SetSendTimeoutms(uint16_t timeout) override
                                { set_send_timeout_ms(fd, timeout); }

    virtual void
    SetFlagsForChild(uint32_t flagsForChild) override
                                { this->flagsForChild = flagsForChild; }

    virtual uint32_t
    FlagsForChild() const override
                                { return flagsForChild; }

    virtual tString
    GetType() override          { return Type(); }

    static tString
    Type()                      { return TO_STR(ConnectionListnerImpl); }

    virtual void
    SetBlocking(bool block = true) override;

    virtual bool
    IsBlocking() override       { return blocking; }

    virtual bool
    TryAgain() override         { return tryAgain; }

    virtual PollableFDPtr
    GetOrig() override          { return thisPtr; }

protected:
    virtual int
    CloseNClear(tString) override;

private:
    sock_t                      fd;
    port_t                      port;
    tString                     socketPath;
    uint32_t                    flagsForChild;
    bool                        ipv6;
    bool                        blocking;
    bool                        tryAgain;
};

DefineMakeSharedPtr(ConnectionListnerImpl);

//=====

} /* namespace net */

#endif /* CPP_SERVER_SSL_CONNECTIONLISTNER_HH_ */
