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

#ifndef SRC_CPP_PUBLIC_SDK_SDK_HH_
#define SRC_CPP_PUBLIC_SDK_SDK_HH_

#include <net/ConnectionListener.hh>
#include <Session.hh>
#include <poll/PinggyPoll.hh>
#include <mutex>
#include <thread>
#include <utils/Semaphore.hh>
#include "SdkChannelWraper.hh"

namespace sdk
{

struct SDKConfig: virtual public pinggy::SharedObject
{
    SDKConfig();

    //The token and any other parameters as well.
    tString                     Token;

    //The tcp tunnel type tcp, tls, tlstcp or http
    tString                     Mode;

    //The udp tunnel type i.e. udp
    tString                     UdpMode;

    //sshOverSsl does not exists here as it use ssl only, no ssh

    // Pinggy server address. It is supposed to be a.pinggy.io or regional server as well.
    UrlPtr                      ServerAddress;

    //this TcpForwarding address
    UrlPtr                      TcpForwardTo;

    //this UdpForwarding address
    UrlPtr                      UdpForwardTo;

    //force login. It add `force` as user name
    bool                        Force;

    //rest of the arguments that we passed to ssh
    tString                     Argument;

    //Whether if we want to run advancedparsing for http.
    // disabling this would disable webdebugger as well.
    bool                        AdvancedParsing;

    //Enable it if you wants to connect with server using
    // encrypted ssl channel or not. Most of the production
    // production server does not support plaintext connection.
    // enable it all the times.
    bool                        Ssl;

    //this needs to set to a.pinggy.io. Some test server may
    // accept values different than a.pinggy.io.
    tString                     SniServerName;

    bool                        Insecure;

private:
    friend class Sdk;

    void                        validate();
    tString                     getUser();
};
DefineMakeSharedPtr(SDKConfig);

abstract class SdkEventHandler: virtual public pinggy::SharedObject
{
public:
    virtual
    ~SdkEventHandler()          { }

    virtual void
    OnConnected()                 { }

    virtual void
    OnAuthenticated()             { } //Not important;

    virtual void
    OnAuthenticationFailed(std::vector<tString> why)
                                { }

    virtual void
    OnPrimaryForwardingSucceeded(std::vector<tString> urls)
                                { }

    virtual void
    OnPrimaryForwardingFailed(tString)
                                { }

    virtual void
    OnRemoteForwardingSuccess(UrlPtr bindAddress, UrlPtr forwardTo)
                                { }

    virtual void
    OnRemoteForwardingFailed(UrlPtr bindAddress, UrlPtr forwardTo, tString error)
                                { }

    virtual void
    OnDisconnected(tString error, std::vector<tString> messages)
                                { }

    virtual void
    KeepAliveResponse(tUint64 forTick)
                                { }

    virtual void
    OnHandleError(tUint32 errorNo, tString what, tBool recoverable)
                                { }

    //return false to let the sdk handle connection
    virtual bool
    OnNewVisitorConnectionReceived(SdkChannelWraperPtr channel)
    {
        return false;
    }
};
DeclareSharedPtr(SdkEventHandler);

DeclareStructWithSharedPtr(PortConfig);
DeclareClassWithSharedPtr(ThreadLock);

class Sdk:
        virtual public protocol::SessionEventHandler,
        virtual public net::ConnectionListenerHandler,
        virtual public FDEventHandler,
        virtual public net::NonBlockingConnectEventHandler
{
public:
    Sdk(SDKConfigPtr config, SdkEventHandlerPtr _eventHandler = nullptr);

    virtual
    ~Sdk();

    bool
    Connect(common::PollControllerPtr pollController=nullptr);

    bool
    Start(common::PollControllerPtr pollController=nullptr);

    bool
    Stop();

    tInt
    ResumeTunnel();

    bool
    IsAuthenticated()           {return authenticated;}

    std::vector<tString>
    GetUrls();

    tUint64
    SendKeepAlive();

    tString
    GetEndMessage();

    SdkEventHandlerPtr
    GetSdkEventHandler();

    ThreadLockPtr
    LockIfDifferentThread();

    port_t
    StartWebDebugging(port_t port=4300);

    void
    RequestPrimaryRemoteForwarding();

    void
    RequestAdditionalRemoteForwarding(UrlPtr bindAddress, UrlPtr forwardTo);

    bool
    IsTunnelActive()            { return (started && (!stopped)); }

//protocol::SessionEventHandler
    virtual void
    HandleSessionInitiated() override;

    virtual void
    HandleSessionAuthenticatedAsClient(std::vector<tString> messages) override;

    virtual void
    HandleSessionAuthenticationFailed(tString error, std::vector<tString> OnAuthenticationFailed) override;

    virtual void
    HandleSessionRemoteForwardingSucceeded(protocol::tReqId reqId, std::vector<tString> urls) override;

    virtual void
    HandleSessionRemoteForwardingFailed(protocol::tReqId reqId, tString error) override;

    virtual void
    HandleSessionNewChannelRequest(protocol::ChannelPtr channel) override;

    virtual void
    HandleSessionKeepAliveResponseReceived(tUint64 tick) override;

    virtual void
    HandleSessionDisconnection(tString reason) override;

    virtual void
    HandleSessionConnectionReset() override;

    virtual void
    HandleSessionError(tUint32 errorNo, tString what, tBool recoverable) override;


//net::ConnectionListenerHandler
    virtual void
    NewVisitor(net::NetworkConnectionPtr netCon) override;

    virtual void
    ConnectionListenerClosed(net::ConnectionListnerPtr listener) override;


//net::FDEventHandler
    virtual len_t
    HandleFDReadWTag(PollableFDPtr pfd, tString tag) override;

    virtual len_t
    HandleFDWriteWTag(PollableFDPtr pfd, tString tag) override;

    virtual len_t
    HandleFDErrorWTag(PollableFDPtr pfd, tString tag, int16_t revents) override;


//net::NonBlockingConnectEventHandler
    virtual len_t
    HandleConnected(net::NetworkConnectionImplPtr) override;

    virtual len_t
    HandleConnectionFailed(net::NetworkConnectionImplPtr) override;


private:
    void
    authenticate();

    void
    tunnelInitiated();

    bool
    startTunnel();

    void
    throwWrongThreadException(tString funcname);

    void
    cleanup();

    net::NetworkConnectionPtr   baseConnection;
    common::PollControllerPtr   pollController;
    protocol::SessionPtr        session;

    bool                        connected;
    bool                        authenticated;
    bool                        started;
    bool                        running;

    protocol::tReqId            primaryForwardingReqId;

    std::vector<tString>        authenticationMsg;
    std::vector<tString>        urls;
    tString                     lastError;
    bool                        globalPollController;
    SDKConfigPtr                sdkConfig;
    SdkEventHandlerPtr          eventHandler;
    net::ConnectionListnerPtr   webDebugListener;
    PortConfigPtr               portConfig;

    std::thread::id             runningThreadId;

    //Innder lock would be always locked by the tunnel
    //To access something, first lock the outer lock, send the notification then try the inner lock
    std::mutex                  lockAccess;
    Semaphore                   semaphore;
    net::NetworkConnectionPtr   notificationConn;

    bool                        primaryReverseForwardingInitiated;
    bool                        block;
    bool                        automatic;
    bool                        primaryForwardingCompleted;
    bool                        stopped;


    std::map<protocol::tReqId, std::tuple<UrlPtr, UrlPtr>> // pendingReqId [remote binding address to localBinding address]
                                pendingRemoteForwardingMap;
    std::map<std::tuple<tString, port_t>, std::tuple<tString, port_t>>
                                remoteForwardings;
};
DefineMakeSharedPtr(Sdk);

} // namespace sdk


#endif // SRC_CPP_PUBLIC_SDK_SDK_HH_
