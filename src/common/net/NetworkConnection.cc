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

#include "NetworkConnection.hh"
#include <openssl/ssl.h>

#include <platform/Log.hh>
#include <string.h>
#include <utils/Json.hh>
#include <platform/platform.h>
#include "AddressCache.hh"


namespace net {


NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_CUSTOME_PTR(SocketStat,
                        (Retransmits,   retransmits),
                        (Unacked,       unacked),
                        (LastDataSent,  lastDataSent),
                        (LastAckSent,   lastAckSent),
                        (LastDataRecv,  lastDataRecv),
                        (LastAckRecv,   lastAckRecv)
);

static bool
isIpv6EncodedIpv4(const struct in6_addr *ipv6_encoded_ipv4)
{
    return (ipv6_encoded_ipv4->s6_addr[0] == 0     && ipv6_encoded_ipv4->s6_addr[1] == 0 &&
            ipv6_encoded_ipv4->s6_addr[2] == 0     && ipv6_encoded_ipv4->s6_addr[3] == 0 &&
            ipv6_encoded_ipv4->s6_addr[4] == 0     && ipv6_encoded_ipv4->s6_addr[5] == 0 &&
            ipv6_encoded_ipv4->s6_addr[6] == 0     && ipv6_encoded_ipv4->s6_addr[7] == 0 &&
            ipv6_encoded_ipv4->s6_addr[8] == 0     && ipv6_encoded_ipv4->s6_addr[9] == 0 &&
            ipv6_encoded_ipv4->s6_addr[10] == 0xFF && ipv6_encoded_ipv4->s6_addr[11] == 0xFF);
}


bool
NetworkSocket::ReassigntoLowerFdPtr(sock_t *fd)
{
#ifndef __WINDOWS_OS__
    sock_t newSock = dup(*fd);
    if (newSock < 0)
        return false;
    if (newSock >= *fd) {
        SysSocketClose(newSock);
        return false;
    }
    SysSocketClose(*fd);
    *fd = newSock;
#endif // !__WINDOWS_OS__

    return true;
}

NetworkConnectionImpl::NetworkConnectionImpl(tString host, tString port, bool blockingConnect) :
            fd(INVALID_SOCKET), flags(0), lastReturn(0), blocking(false), tryAgain(false),
            connecting(false), cachedAddressTried(false), fetchAddressFromSystem(false),
            hostToConnect(host), portToConnect(port)
{
    bzero(&currentAddress, sizeof(currentAddress));
    netState.Connected = false;
    if (blockingConnect) {
        auto sock = app_tcp_client_connect_host(host.c_str(), port.c_str());
        if (!IsValidSocket(sock)) {
            throw std::runtime_error("Could not connect: " + tString(app_get_strerror(app_get_errno())));
        }
        fd = sock;
        soType = get_socket_type(fd);
        soFamily = get_socket_family(fd);
        netState.Tcp = (soFamily == AF_INET || soFamily == AF_INET6) && soType == SOCK_STREAM;
        netState.Uds = soFamily == AF_UNIX;
        netState.Connected = true;
        netState.Valid = true;
    }
}

#ifndef __WINDOWS_OS__
NetworkConnectionImpl::NetworkConnectionImpl(tString path) :
            fd(INVALID_SOCKET), flags(0), lastReturn(0), blocking(false), tryAgain(false),
            connecting(false), cachedAddressTried(false), fetchAddressFromSystem(false)
{
    bzero(&currentAddress, sizeof(currentAddress));
    auto sock = app_uds_client_connect(path.c_str());
    if (!IsValidSocket(sock)) {
        throw std::runtime_error("Could not connect: " + tString(app_get_strerror(app_get_errno())));
    }
    fd = sock;

    soType = get_socket_type(fd);
    soFamily = get_socket_family(fd);
    netState.Tcp = (soFamily == AF_INET || soFamily == AF_INET6) && soType == SOCK_STREAM;
    netState.Uds = soFamily == AF_UNIX;

    netState.Connected = true;
    netState.Valid = true;
}
#endif //__WINDOWS_OS__

NetworkConnectionImpl::NetworkConnectionImpl(sock_t fd) :
            fd(fd), flags(0), lastReturn(0), blocking(false), tryAgain(false),
            connecting(false), cachedAddressTried(false), fetchAddressFromSystem(false)
{
    bzero(&currentAddress, sizeof(currentAddress));
    soType = get_socket_type(fd);
    soFamily = get_socket_family(fd);
    netState.Tcp = (soFamily == AF_INET || soFamily == AF_INET6) && soType == SOCK_STREAM;
    netState.Uds = soFamily == AF_UNIX;
    set_close_on_exec(fd);
    // set_blocking(fd, 1);
    blocking = is_blocking(fd);
    netState.Valid = IsValidSocket(fd);
}


NetworkConnectionImpl::~NetworkConnectionImpl()
{
    if(IsValidSocket(fd))
        LOGD(this, "Closing fd:", fd);
    CloseNCleanSocket(fd);
    netState.Valid = false;
}

ssize_t
NetworkConnectionImpl::Read(void *buf, size_t nbyte, int flags)
{
    lastReturn = app_recv(fd, buf, nbyte, flags);
    tryAgain = false;
    if (lastReturn < 0 && app_is_eagain()) {
        tryAgain = true;
    }
    return lastReturn;
}

ssize_t
NetworkConnectionImpl::Write(const void *buf, size_t nbyte, int flags)
{
    lastReturn = app_send(fd, buf, nbyte, flags);
    tryAgain = false;
    if (lastReturn < 0 && app_is_eagain()) {
        tryAgain = true;
    }
    return lastReturn;
}

std::tuple<NetworkConnectionImplPtr, NetworkConnectionImplPtr>
NetworkConnectionImpl::CreateConnectionPair()
{
    sock_t fds[2];
    int ret = app_socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
    if (ret < 0) {
        LOGEE("Error with socket pair");
        return {nullptr, nullptr};
    }

    return {NewNetworkConnectionImplPtr(fds[0]), NewNetworkConnectionImplPtr(fds[1])};
}

int
NetworkConnectionImpl::CloseNClear(tString location)
{
    int ret = 0;
    if(IsValidSocket(fd)) {
        LOGD(this, location, "Closing fd:", fd);
        ret = SysSocketClose(fd);
        InValidateSocket(fd);
        netState.Valid = false;
    }
    return ret;
}

len_t
NetworkConnectionImpl::HandleConnect()
{
    DeregisterConnectHandler();
    if (connectTimer) {
        connectTimer->DisArm();
        connectTimer = nullptr;
        LOGT("DisArming timer: fd:", fd);
    }
    int err;
    socklen_t len = sizeof(err);
    if (app_getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len) < 0) {
        CloseNCleanSocket(fd);
        LOGT("Failed to connect");
        netState.Valid = false;
        tryNonBlockingConnect();
        return 0;
    }

    if (err == 0) {
        netState.Connected = true;

        soType = get_socket_type(fd);
        soFamily = get_socket_family(fd);
        netState.Tcp = (soFamily == AF_INET || soFamily == AF_INET6) && soType == SOCK_STREAM;
        netState.Uds = soFamily == AF_UNIX;

        connectEventHandler->HandleConnected(thisPtr);
        connectEventHandler = nullptr;
        connectEventPtr     = nullptr;
        connectEventTag     = "";

        if (!currentAddress.cached) {
            AddressCache::GetInstance()->SetAddrInfo(hostToConnect, portToConnect, true, currentAddress);
        }
    } else {
        LOGT("Failed to connect");
        CloseNCleanSocket(fd);
        netState.Valid = false;
        tryNonBlockingConnect();
    }
    return 0;
}

void
NetworkConnectionImpl::tryNonBlockingConnect()
{
    while (getNextAddressToConnect()) {
        if (currentAddress.valid == false)
            continue;

        int success = 0;
        LOGT("Trying to connect");
        fd = app_connect_nonblocking_socket(&currentAddress, &success);
        netState.Valid = IsValidSocket(fd);
        if (IsValidSocket(fd)) {
            if (success) {
                connectEventHandler->HandleConnected(thisPtr);
                connectEventHandler = nullptr;
                connectEventPtr     = nullptr;
                connectEventTag     = "";
                if (!currentAddress.cached) {
                    AddressCache::GetInstance()->SetAddrInfo(hostToConnect, portToConnect, true, currentAddress);
                }
            } else {
                RegisterConnectHandler();
                connectTimer = GetPollController()->SetTimeout(5*SECOND, thisPtr, &NetworkConnectionImpl::connectTimeoutOccured);
                LOGT("Setting timer: fd:", fd, (currentAddress.family==AF_INET?"IPv6":"IPv4"));
            }
            return;
        }
    }

    {
        LOGE("Failed to connect to ", hostToConnect + ":" + portToConnect);
        connecting = false;
        connectEventHandler->HandleConnectionFailed(thisPtr);
        connectEventHandler = nullptr;
        connectEventPtr     = nullptr;
        connectEventTag     = "";
        return;
    }
}

void
NetworkConnectionImpl::connectTimeoutOccured()
{
    DeregisterConnectHandler();
    connectTimer = nullptr;
    LOGT("Timeout to connect: fd: ", fd);
    CloseNCleanSocket(fd);
    netState.Valid = false;
    tryNonBlockingConnect();
}

bool
NetworkConnectionImpl::getNextAddressToConnect()
{
    currentAddress = sock_addrinfo{0};
    if (!cachedAddressTried) {
        cachedAddressTried = true;
        fetchAddressFromSystem = true;
        auto addrCache = AddressCache::GetInstance();
        currentAddress = addrCache->GetAddrInfo(hostToConnect, portToConnect, true);
        if (currentAddress.valid)
            return true;
    }

    if (fetchAddressFromSystem) {
        fetchAddressFromSystem = false;
        auto addresses = app_getaddrinfo_tcp(hostToConnect.c_str(), portToConnect.c_str());
        for (auto addr = addresses; addr && addr->valid; addr++) {
            sock_addrinfo tmpAdr = *addr;
            addressesToConnect.push(tmpAdr);
        }
        app_freeaddrinfo(addresses);
    }

    if (addressesToConnect.empty())
        return false;

    currentAddress = addressesToConnect.front();
    addressesToConnect.pop();
    return true;
}

ssize_t
NetworkConnectionImpl::Peek(void *buf, size_t nbytes)
{
    lastReturn = app_recv(fd, buf, nbytes, MSG_PEEK);
    tryAgain = false;
    if (lastReturn < 0 && app_is_eagain()) {
        tryAgain = true;
    }
    return lastReturn;
}


SocketAddressPtr NetworkConnectionImpl::GetPeerAddress()
{
    if (peerAddressCached)
        return peerAddressCached;

    sockaddr_ip sockAddr;
    socklen_t sockLen;
    sockLen = sizeof(sockAddr);
    bzero(&sockAddr, sizeof(sockAddr));
    int ret = app_getpeername(fd, (struct sockaddr *)&sockAddr, &sockLen);
    if (ret < 0) {
        sockAddr.addr.sa_family = AF_UNSPEC;
        LOGEF(GetFd(), "`" + tString(__func__) + "`");
        return NewSocketAddressPtr(sockAddr);
    }

    peerAddressCached = NewSocketAddressPtr(sockAddr);
    return peerAddressCached;
}

SocketAddressPtr
NetworkConnectionImpl::GetLocalAddress()
{
    if (localAddressCached)
        return localAddressCached;

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

    localAddressCached = NewSocketAddressPtr(sockAddr);
    return localAddressCached;
}

int
NetworkConnectionImpl::SslError(int ret)
{
    if(ret < 0)
        return SSL_ERROR_SYSCALL;
    return SSL_ERROR_NONE;
}

SocketAddress::SocketAddress(tString addr): valid(false), uds(false), ipv6(false), port(0)
{
    socklen_t sockaddrlen = sizeof(sockAddr);
    if (ip_port_to_sockaddr(addr.c_str(), &sockAddr, &sockaddrlen) < 0) {
        throw std::runtime_error("failed to extract sockaddr");
    }
    parseSockaddr();
}

SocketAddress::SocketAddress(const sockaddr_ip sockAddr) : sockAddr(sockAddr), valid(false), uds(false), ipv6(false), port(0)
{
    parseSockaddr();
}

SocketAddress::SocketAddress(const ip_addr addr, tUint16 port): port(port), addr(addr)
{
    if (port == 0)
        valid = false;
    else
        valid = true;

    uds = false;


    if ( isIpv6EncodedIpv4(&addr.v6) ) {
        char buf[INET_ADDRSTRLEN];
        if(app_inet_ntop(AF_INET, &addr.v4, buf, INET_ADDRSTRLEN) == NULL) {
            LOGEE("inet_ntop");
            return;
        }
        ip = tString(buf);
        ipv6 = false;
        sockAddr.inaddr.sin_port = app_htons(port);
        sockAddr.inaddr.sin_family = AF_INET;
        sockAddr.inaddr.sin_addr = addr.v4;
    } else {
        char buf[INET6_ADDRSTRLEN];
        if(app_inet_ntop(AF_INET6, &addr.v6, buf, INET6_ADDRSTRLEN) == NULL) {
            LOGEE("inet_ntop");
            return;
        }
        ip = tString(buf);
        ipv6 = true;
        sockAddr.in6addr.sin6_port = app_htons(port);
        sockAddr.in6addr.sin6_family = AF_INET6;
        sockAddr.in6addr.sin6_addr = addr.v6;
    }
}

port_t
SocketAddress::GetPort()
{
    if(!valid || uds)
        return 0;
    return port;
}

tString
SocketAddress::GetIp()
{
    if(!valid || uds)
        return "";
    return ip;
}

tString
SocketAddress::GetPath()
{
    if(!valid || !uds)
        return "";
    return path;
}

tString
SocketAddress::ToString()
{
    if(!valid)
        return "InValid"; //TODO thow exception
    if (uds)
        return path;
    if (ipv6)
        return "["+ip+"]:" + std::to_string(port);
    return ip + ":" + std::to_string(port);
}

void SocketAddress::parseSockaddr()
{
    switch(sockAddr.addr.sa_family) {
        case AF_INET: {
            char buf[INET_ADDRSTRLEN];
            if(app_inet_ntop(sockAddr.addr.sa_family, &sockAddr.inaddr.sin_addr, buf, INET_ADDRSTRLEN) == NULL) {
                LOGEE("inet_ntop");
                break;
            }
            ip = tString(buf);
            port = app_ntohs(sockAddr.inaddr.sin_port);
            valid = true;
            addr = {{{0, 0}}};
            addr.__padd8[10] = 0xff;
            addr.__padd8[11] = 0xff;
            addr.v4 = sockAddr.inaddr.sin_addr;
            ipv6 = false;
            break;
        }
        case AF_INET6: {
            char buf[INET6_ADDRSTRLEN];
            if(isIpv6EncodedIpv4(&sockAddr.in6addr.sin6_addr)) {
                struct in_addr ipv4_addr;
                ipv4_addr.s_addr = *((uint32_t *)&sockAddr.in6addr.sin6_addr.s6_addr[12]);
                if(app_inet_ntop(AF_INET, &ipv4_addr, buf, INET_ADDRSTRLEN) == NULL) {
                    LOGEE("inet_ntop");
                    break;
                }
                ipv6 = false;
            } else {
                if(app_inet_ntop(sockAddr.addr.sa_family, &sockAddr.in6addr.sin6_addr, buf, INET6_ADDRSTRLEN) == NULL) {
                    LOGEE("inet_ntop");
                    break;
                }
                ipv6 = true;
            }
            ip = tString(buf);
            port = app_ntohs(sockAddr.inaddr.sin_port);
            valid = true;
            addr.v6 = sockAddr.in6addr.sin6_addr;
            break;
        }
#ifndef __WINDOWS_OS__
        case AF_UNIX: {
            auto str = sockAddr.unaddr.sun_path;
            tString addr = "";
            if (str[0] == 0) { //TODO This value may be uninitialised
                str += 1;
                addr = "@";
            }
            addr += tString(str);
            path = addr;
            valid = true;
            uds = true;
            ipv6 = false;
            break;
        }
#endif //__WINDOWS_OS__
    }
}

bool
NetworkConnectionImpl::EnableKeepAlive(int keepCnt, int keepIdle,
        int keepIntvl, bool enable)
{

    if(!IsTcp() || !enable)
        return false;

    auto res = enable_keep_alive(fd, keepCnt, keepIdle, keepIntvl, enable ? 1 : 0);
    return res == 1 ? true : false;
}

void
NetworkConnectionImpl::SetBlocking(bool block)
{
    if(set_blocking(fd, block?1:0)) {
        blocking = block;
    }
}

void
NetworkConnectionImpl::Connect(NonBlockingConnectEventHandlerPtr handler,
                                    pinggy::VoidPtr ptr, tString tag)
{
    if (netState.Connected || connecting)
        throw std::runtime_error("Already connected");

    connectEventHandler = handler;
    connectEventTag     = tag;
    connectEventPtr     = ptr;
    connecting = true;
    tryNonBlockingConnect();
}

SocketStatPtr
NetworkConnection::GetSocketStat()
{
    if (!IsTcp())
        return nullptr;
    auto fd = GetFd();
    if (!IsValidSocket(GetFd()))
        return nullptr;

    auto sockStat = getUnackedTcp(fd);
    if (!sockStat.success)
        return nullptr;
    auto ss = NewSocketStatPtr();
    ss->Retransmits = sockStat.retransmits;
    ss->Unacked = sockStat.unacked;
    ss->LastDataSent = sockStat.last_data_sent;
    ss->LastAckSent = sockStat.last_ack_sent;
    ss->LastDataRecv = sockStat.last_data_recv;
    ss->LastAckRecv = sockStat.last_ack_recv;
    return ss;
}

const AddressMetadata
NetworkConnection::GetAddressMetadata()
{
    auto localAddr  = GetLocalAddress();
    auto remoteAddr = GetPeerAddress();
    AddressMetadata metadata;
    bzero(&metadata, sizeof(metadata));
    metadata.LocalIp    = localAddr->GetAddr();
    metadata.RemoteIp   = remoteAddr->GetAddr();
    metadata.LocalPort  = htons(localAddr->GetPort());
    metadata.RemotePort = htons(remoteAddr->GetPort());
    metadata.Flags      = Flags();

    return metadata;
}

const ConnectionMetadata
NetworkConnection::GetConnectionMetadata(tString indentifier, tString serverName)
{
    if (serverName.empty())
        serverName = GetServerName();
    if (serverName.length() > METADATA_URL_SIZE_RELAY) {
        serverName = serverName.substr(0, METADATA_URL_SIZE_RELAY-1);
    }
    ConnectionMetadata metadata;
    bzero(&metadata, sizeof(metadata));
    metadata.AddrMetadata  = GetAddressMetadata();
    metadata.ConnType      = 0;
    metadata.ServerNameLen = htons((tUint16)serverName.length());

    memcpy(metadata.Identifier, indentifier.c_str(), MIN(8,indentifier.length()+1));
    memcpy(metadata.ServerName, serverName.c_str(), serverName.length());
    return metadata;
}

std::tuple<ssize_t, RawDataPtr>
NetworkConnection::Read(len_t nbyte, int flags)
{
    if (nbyte < 0)
        nbyte = 2048;
    RawDataPtr rwData = NewRawDataPtr(MAX(2048, nbyte));
    auto ret = Read(rwData->Data, nbyte, flags);
    if (ret <= 0)
        return {ret, nullptr};
    rwData->Len = ret;
    return {ret, rwData};
}

ssize_t
NetworkConnection::Write(RawDataPtr rwData, int flags)
{
    return Write(rwData->GetData(), rwData->Len, flags);
}

std::tuple<ssize_t, RawDataPtr>
NetworkConnection::Peek(len_t nbyte)
{
    if (nbyte < 0)
        nbyte = 2048;
    RawDataPtr rwData = NewRawDataPtr(MAX(2048, nbyte));
    auto ret = Peek(rwData->Data, nbyte);
    if (ret <= 0)
        return {ret, nullptr};
    rwData->Len = ret;
    return {ret, rwData};
}


} /* namespace net */

std::ostream&
operator <<(std::ostream &os, const net::SocketAddressPtr &sa)
{
    os << sa->ToString();
    return os;
}

std::ostream &
operator<<(std::ostream &os, net::tConnType &connType)
{
    os << "ConnType{";
    os << "HandlerType: " << connType.HandlerType << ", ";
    os << "SourceType: " << connType.SourceType << ", ";
    os << "ContentType: " << connType.ContentType << ", ";
    os << "ProtocolType: " << connType.ProtocolType << ", ";
    os << "Raw: " << connType.Raw;
    os << "}";
    return os;
}
