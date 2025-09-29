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


#define ThisIsNetWorkSource

#include <platform/Log.hh>
#include "ConnectionListener.hh"

#define MAX_CONSECUTIVE_OPEN_FILE_ERROR 1024

namespace net {

ConnectionListenerImpl::ConnectionListenerImpl(sock_t fd):
        fd(fd), port(0), flagsForChild(0), ipv6(false),
        blocking(true), tryAgain(false)
{
    if(IsValidSocket(fd)) {
        port = app_socket_port(fd);
        set_close_on_exec(fd);
    }
}

ConnectionListenerImpl::ConnectionListenerImpl(std::string path):
        fd(InValidSocket), port(0),
        socketPath(path), flagsForChild(0), ipv6(false),
        blocking(true), tryAgain(false)
{
}

ConnectionListenerImpl::ConnectionListenerImpl(port_t port, bool ipv6):
        fd(InValidSocket), port(port),
        flagsForChild(0), ipv6(ipv6),
        blocking(true), tryAgain(false)
{
}

ConnectionListenerImpl::~ConnectionListenerImpl()
{
    LOGT("Removing" << fd);
    CloseNCleanSocket(fd);
}

bool
ConnectionListenerImpl::StartListening()
{
    if (IsValidSocket(fd)) {
        return true;
    }
    if(!socketPath.empty()) {
        fd = app_uds_listener(socketPath.c_str());
        if (!IsValidSocket(fd)) {
            LOGF("Error listening `" << socketPath << "` error:", app_get_errno(), app_get_strerror(app_get_errno()));
            return false;
        }
        set_close_on_exec(fd);
        LOGI("Listening to `" << socketPath << "`");
    } else {
        if (ipv6) {
            fd = app_tcp6_listener(port);
        } else {
            fd = app_tcp_listener(port);
        }
        if(IsValidSocket(fd)) {
            port = app_socket_port(fd);
            set_close_on_exec(fd);
        }
        LOGI("Listening to `http://localhost:" << port << "` (" << fd << ")");
    }
    if (IsValidSocket(fd)) {
        blocking = is_blocking(fd);
    }
    return IsValidSocket(fd);
}

sock_t
ConnectionListenerImpl::AcceptSocket()
{
    sock_t newsock = app_accept(fd, NULL, NULL);
    if (newsock < 0) {
        if (app_is_eagain()) {
            tryAgain = true;
        } else {
            LOGEF(GetFd(), "Error in NETCONN");
        }
    }
    return newsock;
}

NetworkConnectionPtr
ConnectionListenerImpl::Accept()
{
    tryAgain = false;
    sock_t newsock = app_accept(fd, NULL, NULL);
    if (newsock < 0) {
        if (app_is_eagain()) {
            tryAgain = true;
        } else {
            LOGEF(GetFd(), "Error in NETCONN");
        }
    }
    if (!IsValidSocket(newsock)) {
        LOGE("Invalid socket");
        return nullptr;
    }
    auto netConn = NewNetworkConnectionImplPtr(newsock);
    netConn->SetFlags(flagsForChild);
    netConn->SetConnType(ConnTypeForChild());
    netConn->SetBlocking(true);
    netConn->SetPollController(GetPController());
    return netConn;
}

int
ConnectionListenerImpl::CloseNClear(tString location)
{
    if(IsValidSocket(fd)) {
        LOGD(this, location, "Closing fd:", fd);
        auto ret = SysSocketClose(fd);
        InValidateSocket(fd);
        return ret;
    }
    return -1;
}

void
ConnectionListenerImpl::SetBlocking(bool block)
{
    if(set_blocking(fd, block?1:0)) {
        blocking = block;
    }
}

void
ConnectionListener::RegisterListenerHandler(ConnectionListenerHandlerPtr hndlr, len_t acceptConn)
{
    eventHandler = hndlr;
    RegisterFDEvenHandler(thisPtr);
    maxSeqAccepts = acceptConn;
}

void
ConnectionListener::RegisterListenerHandler(common::PollControllerPtr pollController, ConnectionListenerHandlerPtr hndlr, len_t acceptConn)
{
    eventHandler = hndlr;
    SetPollController(pollController);
    RegisterFDEvenHandler(thisPtr);
    maxSeqAccepts = acceptConn;
}

len_t
ConnectionListener::HandleFDRead(PollableFDPtr)
{
    auto blocking = IsBlocking();
    len_t cnt = 0;
    while(1) {
        if (GetAcceptRawSocket()) {
            auto newFd = AcceptSocket();
            if (!IsValidSocket(newFd)) {
                if (!TryAgain()) {
                    LOGEF(GetFd(), "Error in NETCONN");
                    DeregisterFDEvenHandler();
                    eventHandler->ConnectionListenerClosed(thisPtr);
                    CloseConn();
                }
                return cnt;
            }
            eventHandler->NewVisitorSocket(newFd, thisPtr);
        } else {
            auto netConn = Accept();
            if (!netConn) {
                if (!TryAgain()) {
                    LOGEF(GetFd(), "Error in NETCONN");
                    DeregisterFDEvenHandler();
                    eventHandler->ConnectionListenerClosed(thisPtr);
                    CloseConn();
                }
                return cnt;
            }
            eventHandler->NewVisitor(netConn);
        }
        cnt += 1;
        if (blocking || cnt >= maxSeqAccepts)
            break;
    }
    return cnt;
}

len_t
ConnectionListener::HandleFDError(PollableFDPtr, int16_t shortInt)
{
    LOGE("HandlerPollError: " << GetFd() << " " << app_get_strerror(shortInt));
    return 0;
}

//=============

sock_t
ConnectionListener::AcceptSocket()
{
    throw ConnectionListenerException("Not implemented", thisPtr);
    return INVALID_SOCKET;
}

} /* namespace net */
