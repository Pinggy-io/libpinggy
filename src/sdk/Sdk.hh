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
#include "SdkConfig.hh"

namespace sdk
{

enum SdkState {
    SdkState_Invalid = 0,

    SdkState_Initial,
    SdkState_Started,
    SdkState_Restart,
    SdkState_Reconnecting,
    SdkState_Connecting,
    SdkState_Connected,
    SdkState_SessionInitiating,
    SdkState_SessionInitiated,
    SdkState_Authenticating,
    SdkState_AuthenticationFailed,
    SdkState_Authenticated,
    SdkState_ForwardingInitiated,
    SdkState_ForwardingAccepted,
    SdkState_ForwardingFailed,
    SdkState_ForwardingSucceeded,

    SdkState_Disconnected,

    SdkState_Reconnect_Failed,
    SdkState_Reconnect_Initiated,
    SdkState_Reconnect_Connected,
    SdkState_Reconnect_Forwarded,
    SdkState_ReconnectWaiting,

    SdkState_Stopped,
};

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
    OnForwardingSucceeded(std::vector<tString> urls)
                                { }

    virtual void
    OnForwardingFailed(tString) { }

    virtual void
    OnAdditionalForwardingSucceeded(tString bindAddress, tString forwardTo, tString forwardingType)
                                { }

    virtual void
    OnAdditionalForwardingFailed(tString bindAddress, tString forwardTo, tString forwardingType, tString error)
                                { }

    virtual void
    OnForwardingChanged(tString changedJson)
                                { }

    virtual void
    OnDisconnected(tString error, std::vector<tString> messages)
                                { }

    virtual void
    OnHandleError(tUint32 errorNo, tString what, tBool recoverable)
                                { }

    virtual void
    OnWillReconnect(tString error, std::vector<tString> messages)
                                { }

    virtual void
    OnReconnecting(tUint16)     { }

    virtual void
    OnReconnectionCompleted(std::vector<tString> urls)
                                { }

    virtual void
    OnReconnectionFailed(tUint16)
                                { }

    virtual void
    OnUsageUpdate(tString)      { }

    //return false to let the sdk handle connection
    virtual bool
    OnNewVisitorConnectionReceived(SdkChannelWraperPtr channel)
                                { return false; }
};
DeclareSharedPtr(SdkEventHandler);

DeclareClassWithSharedPtr(ThreadLock);

class Sdk:
        virtual public protocol::SessionEventHandler,
        virtual public net::ConnectionListenerHandler,
        virtual public FDEventHandler,
        virtual public net::NonBlockingConnectEventHandler,
        virtual public protocol::ChannelEventHandler //this is for internal connection using portConfig
{
public:
    Sdk(SDKConfigPtr config, SdkEventHandlerPtr _eventHandler = nullptr);

    virtual
    ~Sdk();

    bool
    Connect(bool block = false);

    bool
    Start(bool block = true);

    bool
    Stop();

    bool
    ResumeTunnel(tInt32 timeout = -1);

    bool
    IsAuthenticated()           {return state >= SdkState_Authenticated;}

    std::vector<tString>
    GetUrls();

    const tString&
    GetEndMessage();

    SdkEventHandlerPtr
    GetSdkEventHandler();

    ThreadLockPtr
    LockIfDifferentThread();

    port_t
    StartWebDebugging(port_t port=4300);

    bool
    StartForwarding(bool block = false);

    void
    RequestAdditionalForwarding(tString forwardingType, tString bindingUrl, tString forwardTo);

    void
    RequestAdditionalForwarding(tString forwardTo);

    bool
    IsTunnelActive()            { return (state >= SdkState_Started && state < SdkState_Stopped); }

    void
    StartUsagesUpdate()         { usagesRunning = true; }

    void
    StopUsagesUpdate()          { usagesRunning = false; }

    const tString&
    GetCurrentUsages()          { return lastUsagesUpdate; }

    const tString&
    GetNextUsagesUpdate()       { return lastUsagesUpdate; }

    const tString&
    GetGreetingMsg()            { return greetingMsgs; }

    tPort
    GetWebDebugListeningPort();

//protocol::SessionEventHandler
    virtual void
    HandleSessionInitiated() override;

    virtual void
    HandleSessionAuthenticatedAsClient(std::vector<tString> messages, TunnelInfoPtr) override;

    virtual void
    HandleSessionAuthenticationFailed(tString error, std::vector<tString> OnAuthenticationFailed) override;

    virtual void
    HandleSessionRemoteForwardingSucceeded(protocol::tReqId reqId, tForwardingId forwardingId, std::vector<tString> urls,
                                            std::vector<RemoteForwardingPtr> remoteForwardings) override;

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

    virtual void
    HandleSessionUsages(ClientSpecificUsagesPtr usages) override;


//net::ConnectionListenerHandler
    virtual void
    NewVisitor(net::NetworkConnectionPtr netCon) override;

    virtual void
    ConnectionListenerClosed(net::ConnectionListenerPtr listener) override;


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

//protocol::ChannelEventHandler
    virtual void
    ChannelDataReceived(protocol::ChannelPtr) override;

    virtual void
    ChannelReadyToSend(protocol::ChannelPtr, tUint32) override;

    virtual void
    ChannelError(protocol::ChannelPtr, protocol::tError errorCode, tString errorText) override;

    virtual void
    ChannelRejected(protocol::ChannelPtr, tString reason) override;

    virtual void
    ChannelAccepted(protocol::ChannelPtr) override
                                { }

    virtual void
    ChannelCleanup(protocol::ChannelPtr) override;

private:
    void
    authenticate();

    void
    internalConnect();

    bool
    startPollingInCurrentThread();

    void
    initiateNotificationChannel();

    void
    throwWrongThreadException(tString funcname);

    void
    cleanup();

    void
    sendKeepAlive();

    void
    keepAliveTimeout(tUint64 tick);

    void
    stopWebDebugger();

    void
    cleanupForReconnection();

    void
    initPollController();

    bool
    resumeWithLock(tString funcName, tInt32 timeout);

    void
    setupLocalChannelNGetData(port_t port, tString tag);

    void
    setState(SdkState s)        { state = s; }

    bool
    internalRequestForwarding();

    void
    acquireAccessLock(bool block = false);

    void
    releaseAccessLock();

    void
    internalRequestAdditionalRemoteForwarding(SdkForwardingPtr forwarding);

    void
    updateForwardMap(std::vector<RemoteForwardingPtr> remoteForwardings);

    void
    reconnectOrStopLoop();

    void
    initiateReconnection();

    void
    releaseBaseConnection();

    net::NetworkConnectionPtr   baseConnection;
    common::PollControllerPtr   pollController;
    protocol::SessionPtr        session;
    bool                        running;

    std::vector<tString>        authenticationMsg;
    std::vector<tString>        urls;
    tString                     lastError;
    SDKConfigPtr                sdkConfig;
    SdkEventHandlerPtr          eventHandler;
    net::ConnectionListenerPtr  webDebugListener;
    SpecialPortConfigPtr        portConfig;

    std::thread::id             runningThreadId;

    //Innder lock would be always locked by the tunnel
    //To access something, first lock the outer lock, send the notification then try the inner lock
    std::mutex                  lockAccess;
    std::mutex                  reconnectLock; //we cannot use lock access as it will be used somewhere else
    SemaphorePtr                semaphore;
    net::NetworkConnectionPtr   notificationConn;
    net::NetworkConnectionPtr   _notificateMonitorConn;

    tUint64                     lastKeepAliveTickReceived;
    SdkState                    state;

    std::map<protocol::tReqId, SdkForwardingPtr> // pendingReqId [remote binding address to localBinding address]
                                pendingAdditionalRemoteForwardingMap;
    std::map<protocol::tReqId, SdkForwardingPtr>
                                pendingRemoteForwardingRequestMap;
    std::map<tForwardingId, SdkForwardingPtr>
                                sdkForwardings;

    common::PollableTaskPtr     keepAliveTask;
    tInt16                      reconnectCounter;

    bool                        usagesRunning;
    tString                     greetingMsgs;
    tString                     lastUsagesUpdate;

    std::map<tString, tString>  forwardingMap;
    std::vector<SdkForwardingPtr>
                                additionalForwardings;
    bool                        appHandlesNewChannel;
    bool                        reconnectMode;

    tString                     cleanupReason;

    friend class ThreadLock;
};
DefineMakeSharedPtr(Sdk);

} // namespace sdk


#endif // SRC_CPP_PUBLIC_SDK_SDK_HH_
