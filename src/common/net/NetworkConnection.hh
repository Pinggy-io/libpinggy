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


#ifndef CPP_COMMON_NETWORKCONNECTION_HH_
#define CPP_COMMON_NETWORKCONNECTION_HH_

#ifndef ThisIsNetWorkSource
#define ThisIsNetworkConnectionHH_AvoidIt
#endif
#define ThisIsNetworkConnectionRoot

#include <platform/network.h>
#include <utils/Utils.hh>
#include <iostream>
#include <poll/PollableFD.hh>
#include <utils/JsonH.hh>
#include <tuple>

#include "ConFlags.hh"

namespace net {


DeclareClassWithSharedPtr(NetworkConnection);
class NetworkConnectionException: public std::exception, public virtual pinggy::SharedObject
{
public:
    NetworkConnectionException(tString message, NetworkConnectionPtr netConn) : message(message), netConn(netConn){}
    virtual ~NetworkConnectionException() {};

    virtual const char*
    what() const noexcept override      { return message.c_str(); }

    virtual NetworkConnectionPtr
    GetNetConn()                        { return netConn; }

private:
    tString                     message;
    NetworkConnectionPtr        netConn;
};

enum ProbedConnType {
    ProbedConnType_Unknown  = 0,
    ProbedConnType_SSH      = ((tUint64)1)<<0, //Okay to use for client
    ProbedConnType_SSL      = ((tUint64)1)<<1, //Okay to use for client
    ProbedConnType_HTTP     = ((tUint64)1)<<2,
    ProbedConnType_PINGGY   = ((tUint64)1)<<3, //Okay to use for client
    ProbedConnType_RELAY    = ((tUint64)1)<<4,
};

extern "C" {

enum tConnType_Handler {
    ConnType_H_Visitor,
    ConnType_H_Client,
};

enum tConnType_Src {
    ConnType_Src_Plain,
    ConnType_Src_ConnectProxy,
    ConnType_Src_Relay,
    ConnType_Src_SSClient,
    ConnType_Src_Local,
    ConnType_Src_Redirect,
    ConnType_Src_Dashboard,
    ConnType_Src_BashUsages,
};

enum tConnType_Cnt {
    ConnType_Cnt_Unknown,
    ConnType_Cnt_Http,        //It is a http connection
    ConnType_Cnt_Tls,         //It is a tls connection
    ConnType_Cnt_Ssh,         //connection starts with ssh
    ConnType_Cnt_Pinggy,      //connection starts with PINGGY
    ConnType_Cnt_Relay,       //connection starts with RLPINGGY
    ConnType_Cnt_ConnectProxy,
};

enum tConnType_Prt {
    ConnType_Prt_Unknown,
    ConnType_Prt_Tcp,
    ConnType_Prt_Udp,
    ConnType_Prt_Uds,
};


union tConnType {
    struct {
        tUint64                 Enabled:1; // 1

        tUint64                 padding1:1; // 2

        tConnType_Handler       HandlerType:2; // 4

        tUint64                 padding2:4; // 8

        tConnType_Src           SourceType:5; // 13

        tUint64                 padding5:3; // 16

        tConnType_Cnt           ContentType:5; // 21

        tUint64                 padding6:3; // 24

        tConnType_Prt           ProtocolType:4; //28
    };
    tUint64                     Raw;
};
static_assert(sizeof(tConnType) == 8, "Size of ConnectionType must be 8 bytes");

#define ADDRESS_METADATA_SIZE           128
#define NETWORK_METADATA_SIZE           512
#define METADATA_URL_SIZE_RELAY         256
struct AddressMetadata {
    ip_addr                     LocalIp;
    ip_addr                     RemoteIp;
    tUint16                     LocalPort;
    tUint16                     RemotePort;
    //GO is interested upto this
    tUint32                     Flags;
};

#define mAddStaticAssert_Struct(_s, _m, _o) \
    static_assert(offsetof(_s, _m) == _o, \
            "Offset of `" APP_CONVERT_TO_STRING(_m) "` in `" APP_CONVERT_TO_STRING(_s) "` is incorrect")

static_assert(sizeof(ip_addr) == 16, "Size of in6_addr must be 16 bytes");
mAddStaticAssert_Struct(struct AddressMetadata, LocalIp, 0);
mAddStaticAssert_Struct(struct AddressMetadata, RemoteIp, 16);
mAddStaticAssert_Struct(struct AddressMetadata, LocalPort, 32);
mAddStaticAssert_Struct(struct AddressMetadata, RemotePort, 34);
mAddStaticAssert_Struct(struct AddressMetadata, Flags, 36);
struct ConnectionMetadata {
    char                        Identifier[8];
    AddressMetadata             AddrMetadata;
    tUint16                     ConnType;
    tUint16                     ServerNameLen;
    char                        ServerName[256];
    char                        _padding[204];
};

mAddStaticAssert_Struct(struct ConnectionMetadata, Identifier, 0);
mAddStaticAssert_Struct(struct ConnectionMetadata, AddrMetadata, 8);
mAddStaticAssert_Struct(struct ConnectionMetadata, ConnType, 48);
mAddStaticAssert_Struct(struct ConnectionMetadata, ServerNameLen, 50);
mAddStaticAssert_Struct(struct ConnectionMetadata, ServerName, 52);
static_assert(sizeof(struct ConnectionMetadata) == NETWORK_METADATA_SIZE, "Sizeof meta data must be 512");


} // extern "C"

union tNetState {
    struct {
        tUint16                 Ssl:1;
        tUint16                 Tcp:1;
        tUint16                 Uds:1;
        tUint16                 Udp:1;
        tUint16                 Valid:1;
        tUint16                 Connected:1;
        tUint16                 Relayed:1;
        tUint16                 Dummy:1;
        tUint16                 Pollable:1;
        tUint16                 RecvReady:1;
        tUint16                 SendReady:1;
    };
    tUint16                     Raw;
    tNetState(): Raw(0)         { Connected = true; Pollable = true; RecvReady = true; SendReady = true; }

    tNetState
    NewWithSsl()                { auto x = *this; x.Ssl = 1; return x; }

    tNetState
    NewWithTcp()                { auto x = *this; x.Tcp = 1; return x; }

    tNetState
    NewWithUds()                { auto x = *this; x.Uds = 1; return x; }

    tNetState
    NewWithUdp()                { auto x = *this; x.Udp = 1; return x; }

    tNetState
    NewWithValid()              { auto x = *this; x.Valid = 1; return x; }

    tNetState
    NewWithConnected()          { auto x = *this; x.Connected = 1; return x; }

    tNetState
    NewWithRelayed()            { auto x = *this; x.Relayed = 1; return x; }

    tNetState
    NewWithDummy()              { auto x = *this; x.Dummy = 1; return x; }

    tNetState
    NewWithPollable()           { auto x = *this; x.Pollable = 1; return x; }

    tNetState
    NewWithRecvReady()          { auto x = *this; x.RecvReady = 1; return x; }

    tNetState
    NewWithSendReady()          { auto x = *this; x.SendReady = 1; return x; }

};

struct SocketStat: public virtual pinggy::SharedObject {
    uint8_t                     Retransmits;
    uint32_t                    Unacked;
    uint32_t                    LastDataSent;
    uint32_t                    LastAckSent;
    uint32_t                    LastDataRecv;
    uint32_t                    LastAckRecv;
};
DefineMakeSharedPtr(SocketStat);

NLOHMANN_DECLARE_TYPE_NON_INTRUSIVE_CUSTOME_PTR(SocketStat);

abstract class NetworkSocket : public virtual PollableFD {
public:
    virtual
    ~NetworkSocket()            {}

    virtual
    void SetRecvTimeoutms(uint16_t timeout) = 0;

    virtual
    void SetSendTimeoutms(uint16_t timeout) = 0;

    virtual
    tString GetType() = 0;

    virtual
    void SetBlocking(bool block = true) = 0;

    virtual
    bool IsBlocking() = 0;

    virtual bool
    IsPollable() override       { return GetState().Pollable; }

    virtual bool
    IsRecvReady() override      { return GetState().RecvReady; }

    virtual bool
    IsSendReady() override      { return GetState().SendReady; }

    virtual tNetState
    GetState() = 0;

protected:
    virtual bool
    ReassigntoLowerFdPtr(sock_t *fd) final;
};

DeclareSharedPtr(NetworkSocket);

class SocketAddress final : public virtual pinggy::SharedObject {
public:
    SocketAddress(tString addr);

    SocketAddress(const sockaddr_ip addr);

    SocketAddress(const ip_addr, tUint16 port);

    port_t
    GetPort();

    tString
    GetIp();

    tString
    GetPath();

    tString
    ToString();

    bool
    IsIpv6() const              { return ipv6; }

    bool
    IsUds() const               { return uds; }

    bool
    IsValid() const             { return valid; }

    const ip_addr
    GetAddr() const             { return addr; }

    const sockaddr_ip
    GetSockAddr() const         { return sockAddr; }

private:
    void
    parseSockaddr();

    sockaddr_ip                 sockAddr;
    bool                        valid   = false;
    bool                        uds     = false;
    bool                        ipv6    = false;
    tString                     ip;
    port_t                      port;
    tString                     path;
    ip_addr                     addr;
};
DefineMakeSharedPtr(SocketAddress);

abstract class NetworkConnection : public virtual NetworkSocket {
public:
    NetworkConnection()         { connType.Raw = 0; }

    virtual
    ~NetworkConnection()        {}

    virtual std::tuple<ssize_t, RawDataPtr>
    Read(len_t nbyte, int flags = 0);

    virtual ssize_t
    Write(RawDataPtr rwData, int flags = 0);

    virtual std::tuple<ssize_t, RawDataPtr>
    Peek(len_t nbyte);

    virtual ssize_t
    Read(void *buf, size_t nbyte, int flags = 0) = 0;

    virtual ssize_t
    Write(const void *buf, size_t nbyte, int flags = 0) = 0;

    virtual ssize_t
    Peek(void *, size_t)        { return -1; }

    virtual ssize_t
    LastReturn() = 0;

    virtual SocketAddressPtr
    GetPeerAddress() = 0;

    virtual SocketAddressPtr
    GetLocalAddress() = 0;

    virtual bool
    IsSsl() final               { return GetState().Ssl; }

    virtual bool
    IsTcp() final               { return GetState().Tcp; }

    virtual bool
    IsUds() final               { return GetState().Uds; }

    virtual bool
    IsUdp() final               { return GetState().Udp;}

    virtual int
    SslError(int ret) = 0;

    virtual int
    ShutDown(int how) = 0;

    virtual bool
    IsValid() final             { return GetState().Valid; }

    virtual tString
    GetServerName() = 0; //return empty string incase of plaintext connection, sni for ssl

    virtual bool
    EnableKeepAlive(int keepCnt, int keepIdle,
                    int keepIntvl, bool enable = true) = 0;

    virtual bool
    ReassigntoLowerFd() = 0;

    virtual uint32_t //Not required in public
    Flags() const = 0;

    virtual SocketStatPtr //Not required in public
    GetSocketStat() final;

    virtual bool
    TryAgain() = 0;

    virtual const AddressMetadata //Not required in public
    GetAddressMetadata();

    virtual const ConnectionMetadata //Not required in public
    GetConnectionMetadata(tString indentifier = "PINGGY", tString serverName="");

    virtual bool
    IsConnected() final         { return GetState().Connected; }

    virtual bool
    IsRelayed() final           { return GetState().Relayed; }

    virtual bool
    IsDummy() final             { return GetState().Dummy; }

    virtual tConnType
    ConnType() final            { return connType; }

    virtual void
    SetConnType(tConnType connType)
                                { this->connType = connType; }

private:
    tConnType                   connType;
};


DeclareClassWithSharedPtr(NetworkConnectionImpl);
abstract class NonBlockingConnectEventHandler: public virtual pinggy::SharedObject {
public:
    virtual
    ~NonBlockingConnectEventHandler()
                                { }

    virtual len_t
    HandleConnected(NetworkConnectionImplPtr)
                                { return 0; }

    virtual len_t
    HandleConnectionFailed(NetworkConnectionImplPtr)
                                { return 0; }
};
DefineMakeSharedPtr(NonBlockingConnectEventHandler);
class NetworkConnectionImpl: public NetworkConnection
{
public:
    NetworkConnectionImpl(tString host, tString port, bool blockingConnect=true);
#ifndef __WINDOWS_OS__
    NetworkConnectionImpl(tString path);
#endif
    NetworkConnectionImpl(sock_t fd);

    virtual
    ~NetworkConnectionImpl();

    virtual tString
    GetConnectToAddr()          { return hostToConnect + ":" + portToConnect; }

    virtual ssize_t
    Read(void *buf, size_t nbyte, int flags = 0) override;

    virtual ssize_t
    Peek(void *buf, size_t nbyte) override;

    virtual ssize_t
    Write(const void *buf, size_t nbyte, int flags = 0) override;

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
    ShutDown(int how) override  { return app_shutdown(fd, how); }

    virtual tString
    GetType() override          { return Type(); }

    static tString
    Type()                      { return TO_STR(NetworkConnectionImpl); }


    virtual tString
    GetServerName() override    { return ""; }

    virtual SocketAddressPtr
    GetPeerAddress() override;

    virtual SocketAddressPtr
    GetLocalAddress() override;

    virtual bool
    EnableKeepAlive(int keepCnt, int keepIdleSec,
            int keepIntvl, bool enable = true) override;

    virtual bool
    ReassigntoLowerFd() override
                                { return ReassigntoLowerFdPtr(&fd); }

    virtual uint32_t
    Flags() const override      { return flags; }

    virtual void
    SetFlags(uint32_t flags)    { this->flags = flags; }

    virtual void
    SetBlocking(bool block = true) override;

    virtual bool
    IsBlocking() override       { return blocking; }

    virtual bool
    TryAgain() override         { return tryAgain; }


    virtual void
    Connect(NonBlockingConnectEventHandlerPtr handler,
            pinggy::VoidPtr ptr = nullptr, tString tag = "");

    virtual PollableFDPtr
    GetOrig() override          { return thisPtr; }

    virtual tNetState
    GetState() override         { return netState; }

    virtual pinggy::VoidPtr
    GetConnectEventPtr() final  { return connectEventPtr; }

    template<typename T>
    void
    GetConnectEventPtr(std::shared_ptr<T> &ptr)
                                { ptr = connectEventPtr->DynamicPointerCast<T>(); }

    virtual tString
    GetConnectEventTag() final  { return connectEventTag; }

    static std::tuple<NetworkConnectionImplPtr, NetworkConnectionImplPtr>
    CreateConnectionPair();

protected:
    virtual int
    CloseNClear(tString location) override;

    virtual len_t
    HandleConnect() override;

private:
    void
    tryNonBlockingConnect();

    void
    connectTimeoutOccured();

    bool
    getNextAddressToConnect();

    sock_t                      fd;
    int                         soType;
    int                         soFamily;
    SocketAddressPtr            peerAddressCached;
    SocketAddressPtr            localAddressCached;
    uint32_t                    flags;
    ssize_t                     lastReturn;
    bool                        blocking;
    bool                        tryAgain;
    // async connect
    bool                        connecting;
    bool                        cachedAddressTried;
    bool                        fetchAddressFromSystem;
    tString                     hostToConnect;
    tString                     portToConnect;
    sock_addrinfo               currentAddress;
    std::queue<sock_addrinfo>   addressesToConnect;
    NonBlockingConnectEventHandlerPtr
                                connectEventHandler;
    tString                     connectEventTag;
    pinggy::VoidPtr             connectEventPtr;
    common::PollableTaskPtr     connectTimer;

    tNetState                   netState;
};

DefineMakeSharedPtr(NetworkConnectionImpl);

} /* namespace net */

std::ostream&
operator<<(std::ostream& os, const net::SocketAddressPtr& sa);
        //Here call by reference is mandatory


std::ostream&
operator<<(std::ostream& os, net::tConnType& connType);

// std::ostream&
// operator<<(std::ostream& os, const net::tConnType& ct);

#endif /* CPP_COMMON_NETWORKCONNECTION_HH_ */
