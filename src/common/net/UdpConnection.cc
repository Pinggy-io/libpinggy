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

#include "UdpConnection.hh"
#include <openssl/ssl.h>


namespace net
{

int
UdpConnection::SslError(int ret)
{
    return ret < 0 ? SSL_ERROR_SYSCALL : SSL_ERROR_NONE;
}


UdpConnectionImpl::UdpConnectionImpl(tString host, tString port):
            fd(INVALID_SOCKET), flags(0), lastReturn(0), blocking(false), tryAgain(false)
{
    sockaddr_ip peerAddr;
    auto sock = app_udp_client_connect_host(host.c_str(), port.c_str(), &peerAddr);
    if (!IsValidSocket(sock)) {
        throw std::runtime_error("Could not connect: sock " + std::to_string(sock) + " " + tString(app_get_strerror(app_get_errno())));
    }
    fd = sock;
    peerAddress = NewSocketAddressPtr(peerAddr);

    netState.Udp = true;
    netState.Valid = IsValidSocket(fd);
}

//========================
std::tuple<ssize_t, RawDataPtr>
UdpConnectionImpl::Read(len_t nbyte, int flags)
{
    if (!rxData || !rxData->Len) {
        rxData = NewRawDataPtr(2048);
        sockaddr_ip addr;
        socklen_t addrLen = sizeof(addr);
        auto rxLen = app_recv_from(fd, rxData->Data+2, rxData->Capa-2, 0, &addr, &addrLen);
        tryAgain = false;

        if (rxLen < 0 && app_is_eagain()) {
            tryAgain = true;
            rxData = nullptr;
            return {-1, nullptr};
        }
        if (rxLen <= 0) {
            rxData = nullptr;
            return {rxLen, nullptr};
        }
        if (rxLen >= (rxData->Capa-2)) {
            rxData = nullptr;
            tryAgain = true;
            return {-1, nullptr};
        }
        *((uint16_t *)(rxData->Data)) = (uint16_t) app_htons(rxLen);
        rxData->Len = rxLen+2;
    }

    auto ret = rxData->Slice(0, nbyte);
    rxData->Consume(nbyte);
    if (rxData->Len == 0) {
        rxData = nullptr;
    }
    return {ret->Len, ret};
}

ssize_t
UdpConnectionImpl::Write(RawDataPtr rwData, int flags)
{
    uint16_t expectedLen = 0;
    if (!txData) {
        txData = NewRawDataPtr(2048);
        expectedLen = 2;
    } else {
        if (txData->Len < 2) {
            expectedLen = 2 - txData->Len;
        } else {
            expectedLen = app_ntohs(*((uint16_t *)(txData->Data))) + 2;
            expectedLen -= txData->Len;
        }
    }

    auto slice = rwData->Slice(0, expectedLen);
    txData->Concat(slice);

    if (txData->Len <= 2) {
        return slice->Len;
    }

    expectedLen = app_ntohs(*((uint16_t *)(txData->Data))) + 2;
    Assert(expectedLen >= txData->Len);
    if (expectedLen == txData->Len) {
        sockaddr_ip addr = peerAddress->GetSockAddr();
        socklen_t addrlen = addr.addr.sa_family==AF_INET ? sizeof(addr.inaddr) : sizeof(addr.inaddr);
        tryAgain = false;
        auto wrote = app_send_to(fd, txData->GetData()+2, txData->Len-2, 0, &addr, addrlen);
        txData = nullptr;
        if (wrote <= -1  && app_is_eagain()) {
            return slice->Len;
        }
        if (wrote <= 0) {
            LOGE("Socket error: ", app_get_strerror(app_get_errno()));
            return wrote;
        }
    }
    return slice->Len;
}

std::tuple<ssize_t, RawDataPtr>
UdpConnectionImpl::Peek(len_t nbyte)
{
    ABORT_WITH_MSG("NOT IMPLEMENTED FOR UDP");
    return {0, nullptr};
}

int
UdpConnectionImpl::CloseNClear(tString location)
{
    int ret = 0;
    if(IsValidSocket(fd)) {
        LOGI(this, location, "Closing fd:", fd);
        ret = SysSocketClose(fd);
        InValidateSocket(fd);
        netState.Valid = false;
    }
    return ret;
}

int
UdpConnectionImpl::SslError(int ret)
{
    if(ret < 0)
        return SSL_ERROR_SYSCALL;
    return SSL_ERROR_NONE;
}

SocketAddressPtr
UdpConnectionImpl::GetLocalAddress()
{
    if (localAddress)
        return localAddress;

    sockaddr_ip sockAddr;
    socklen_t sockLen;
    bzero(&sockAddr, sizeof(sockAddr));
    sockLen = sizeof(sockAddr);
    int ret = app_getsockname(fd, (struct sockaddr *)&sockAddr, &sockLen);
    if (ret < 0) {
        LOGEF(GetFd(), "`" + tString(__func__) + "`");
        sockAddr.addr.sa_family = AF_UNSPEC;
        return NewSocketAddressPtr(sockAddr);
    }

    localAddress = NewSocketAddressPtr(sockAddr);
    return localAddress;
}

bool
UdpConnectionImpl::EnableKeepAlive(int keepCnt, int keepIdleSec, int keepIntvl, bool enable)
{
    if(!IsTcp() || !enable)
        return false;

    auto res = enable_keep_alive(fd, keepCnt, keepIdleSec, keepIntvl, enable ? 1 : 0);
    return res == 1 ? true : false;
}

void
UdpConnectionImpl::SetBlocking(bool block)
{
    if(set_blocking(fd, block?1:0)) {
        blocking = block;
    }
}

} // namespace net
