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

#include "Sdk.hh"
#ifndef __WINDOWS_OS__
#include <poll/PinggyPollLinux.hh>
#endif
#include <poll/PinggyPollGeneric.hh>
#include <ChannelConnectionForwarder.hh>
#include <net/UdpConnection.hh>
#include <net/DummyConnection.hh>
#include <net/SslNetworkConnection.hh>
#include <thread>
#include <utils/Json.hh>
#include <utils/Semaphore.hh>
#include <platform/Defer.hh>
#include "SdkException.hh"

const char BASE_CERTIFICATE[] = \
"-----BEGIN CERTIFICATE-----\n"                                      \
"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\n" \
"-----END CERTIFICATE-----";


namespace sdk
{

tString PORT_CONF = "PORT_CONF";
tString NOTIFICATION_FD = "NOTIFICATION_FD";
tString GREETING_MSG_TAG = "GREETING_MSG_TAG";

#define PINGGY_LIFE_CYCLE_FUNC
#define PINGGY_LIFE_CYCLE_WRAPPER_FUNC
#define PINGGY_ATTRIBUTE_FUNC

//Rules:
// 1. no private function should lock the lockAccess
// 2. Every life cycle function should lock the lockAccess even before starting
// 3. Every non-life cycle functions that needs to access session (direct or undirect)
//    needs lockAccess (unless same thread and running)

struct [[nodiscard]] PortConfig: virtual public pinggy::SharedObject
{
    PortConfig():
            ConfigTcp(0),
            StatusPort(0),
            UrlTcp(0),
            UsageContinuousTcp(0),
            UsageOnceLongPollTcp(0),
            UsageTcp(0),
            GreetingMsgTCP(0)
                                { }

    virtual
    ~PortConfig()               { }

    port_t                      ConfigTcp;
    port_t                      StatusPort;
    port_t                      UrlTcp;
    port_t                      UsageContinuousTcp;
    port_t                      UsageOnceLongPollTcp;
    port_t                      UsageTcp;
    port_t                      GreetingMsgTCP;
};
DefineMakeSharedPtr(PortConfig);

class ThreadLock : public pinggy::SharedObject
{
public:
    ThreadLock(SdkPtr sdk):
            sdk(sdk)
    {
        sdk->acquireAccessLock();
    }

    virtual
    ~ThreadLock()
    {
        sdk->releaseAccessLock();
    }

private:
    SdkPtr                  sdk;
};
DefineMakeSharedPtr(ThreadLock);


Sdk::Sdk(SDKConfigPtr config, SdkEventHandlerPtr _eventHandler):
            running(false),
            primaryForwardingReqId(0),
            sdkConfig(config),
            eventHandler(_eventHandler),
            semaphore(NewSemaphorePtr(1)),
            stopped(false),
            reconnectNow(false),
            cleanupNow(false),
            lastKeepAliveTickReceived(0),
            state(SdkState_Initial),
            reconnectionState(SdkState_Initial),
            reconnectCounter(0),
            usagesRunning(false),
            appHandlesNewChannel(false)
{
    if (!config) {
        sdkConfig = config = NewSDKConfigPtr();
    }
}

Sdk::~Sdk()
{
    cleanup();
}

bool PINGGY_LIFE_CYCLE_FUNC
Sdk::Connect(bool block)
{
    //==== Setup =========
    acquireAccessLock();
    DEFER(releaseAccessLock(););

    if (state >= SdkState_Connecting)
        return true;

    state = SdkState_Connecting;

    sdkConfig->validate();

    initPollController();

    //=============

    return internalConnect(block);
}

bool PINGGY_LIFE_CYCLE_WRAPPER_FUNC
Sdk::Start()
{
    // No lockAccess required as it is not calling any private function
    if(!reconnectNow) {
        if (!Connect(true)) { //entry point
            LOGD("Not connected or authenticated");
            return false;
        }
        if (!RequestPrimaryRemoteForwarding(true)) {
            LOGD("Primary forwarding failed");
            return false;
        }
    }

    while(true) {
        auto ret = ResumeTunnel();
        if (!ret)
            break;
    }
    return true;
}

bool PINGGY_ATTRIBUTE_FUNC
Sdk::Stop()
{
    auto lock = LockIfDifferentThread();
    if (stopped)
        return false;
    session->End("Connection close");
    stopWebDebugger();
    stopped = true;
    return true;
}

bool PINGGY_LIFE_CYCLE_FUNC
Sdk::ResumeTunnel()
{
    if (stopped)
        return false;
    acquireAccessLock();
    DEFER({releaseAccessLock();});
    if (reconnectNow) {
        reconnectLock.lock();
        DEFER({reconnectLock.unlock();});
        switch (reconnectionState) {
            case SdkState_ReconnectWaiting:
                { //Ongoing reconnection
                    return resumeWithLock(__func__);
                }
                break;

            case SdkState_Reconnect_Initiated:
                {
                    auto ret = internalConnect(false);
                    reconnectionState = SdkState_ReconnectWaiting;
                    if (!ret) { //initiating  // it needs to be non-blocking
                        LOGD("Not connected or authenticated");
                        pollController->SetTimeout(sdkConfig->GetAutoReconnectInterval() * SECOND, thisPtr,
                                                    &Sdk::setReconnectionState, SdkState_Reconnect_Failed);
                    }
                }
                break;

            case SdkState_Reconnect_Connected:
                {
                    auto ret = internalRequestPrimaryRemoteForwarding(false);
                    reconnectionState = SdkState_ReconnectWaiting;
                    if (!ret) { // it needs to be non-blocking
                        LOGD("Primary forwarding failed");
                        pollController->SetTimeout(sdkConfig->GetAutoReconnectInterval() * SECOND, thisPtr,
                                                    &Sdk::setReconnectionState, SdkState_Reconnect_Failed);
                    }
                }
                break;

            case SdkState_Reconnect_Forwarded: //it should not come here
                {
                    if (webDebugListener && webDebugListener->IsListening()) {
                        webDebugListener->RegisterListenerHandler(pollController, thisPtr, 1);
                    }
                    reconnectCounter = 0;
                    reconnectNow = false;
                    if (eventHandler)
                        eventHandler->OnReconnectionCompleted(this->urls);

                    if (additionalForwardings.size() > 0) {
                        for (auto ele : additionalForwardings) {
                            auto [a, b] = ele;
                            internalRequestAdditionalRemoteForwarding(a, b);
                        }
                    }
                }
                break;

            case SdkState_Reconnect_Failed:
            default:
                {
                    cleanupForReconnection(); //cleaning up last connection
                    if (reconnectCounter >= sdkConfig->maxReconnectAttempts) {
                        reconnectionState = SdkState_Reconnect_Failed;
                        lastError = "Maximum reconnection attempts reached. Exiting.";
                        reconnectNow = false;
                        cleanupNow = true;
                        if (eventHandler)
                            eventHandler->OnReconnectionFailed(reconnectCounter);
                    }
                    reconnectionState = SdkState_Reconnect_Initiated;
                    reconnectCounter += 1;
                }
        }
        return true;
    }

    if (cleanupNow) {
        cleanup();
        return false;
    }

    if (state < SdkState_Connected)
        throw std::runtime_error("tunnel is not started");

    if (stopped)
        return false;

    auto success = resumeWithLock(__func__);
    return success;
}

std::vector<tString> PINGGY_ATTRIBUTE_FUNC
Sdk::GetUrls()
{
    if (state < SdkState_PrimaryReverseForwardingSucceeded) {
        LOGE("Tunnel is not running");
        return {};
    }
    if (stopped)
        return {};
    LOGD("Returning urls");
    return urls;
}

const tString& PINGGY_ATTRIBUTE_FUNC
Sdk::GetEndMessage()
{
    return lastError;
}

SdkEventHandlerPtr PINGGY_ATTRIBUTE_FUNC
Sdk::GetSdkEventHandler()
{
    return eventHandler;
}

// This function wait until running thread blocks
ThreadLockPtr
Sdk::LockIfDifferentThread()
{
    auto curThreadId = std::this_thread::get_id();
    if (curThreadId == runningThreadId) { //it will never be same unless they are really same. We do not change running thread without lock
        LOGD("Same thread. not locking.");
        return nullptr;
    }
    semaphore->Wait();
    sdk::ThreadLockPtr threadLock = nullptr;
    if (notificationConn) {
        notificationConn->Write(NewRawDataPtr(tString("a")));
        threadLock = NewThreadLockPtr(new ThreadLock(thisPtr));
    }
    semaphore->Notify();
    return threadLock;
}

port_t PINGGY_ATTRIBUTE_FUNC
Sdk::StartWebDebugging(port_t port)
{
    if (state < SdkState_Authenticated) {
        throw WebDebuggerException("You are not logged in. How did you managed to come here?" );
    }

    auto lock = LockIfDifferentThread();
    if (webDebugListener) {
        throw WebDebuggerException("Web debugger is running already for this tunnel");
    }

    webDebugListener = net::NewConnectionListenerImplPtr(port, false);
    if (!webDebugListener) {
        throw WebDebuggerException("Webdebug listener could not listen. ignoring");
    }

    if (!webDebugListener->StartListening()) {
        webDebugListener = nullptr; //we don't have to close as it will be closed automatically
        throw WebDebuggerException("Something wrong with the webdebug listener.");
    }

    webDebugListener->RegisterListenerHandler(pollController, thisPtr, 1);

    return webDebugListener->GetListeningPort();
}

#define PRIMARY_REMOTE_FORWARDING_HOST "LOCALHOST"
#define PRIMARY_REMOTE_FORWARDING_PORT 0

bool PINGGY_LIFE_CYCLE_FUNC
Sdk::RequestPrimaryRemoteForwarding(bool block)
{
    acquireAccessLock();
    DEFER({releaseAccessLock();});

    return internalRequestPrimaryRemoteForwarding(block);
}

void PINGGY_ATTRIBUTE_FUNC
Sdk::RequestAdditionalRemoteForwarding(tString bindAddress, tString forwardTo)
{
    if (state < SdkState_Authenticated) {
        WebDebuggerException("You are not logged in. How did you managed to come here?" );
    }

    if (bindAddress.empty()) {
        throw RemoteForwardingException("bindAddress cannot be empty");
    }

    if (forwardTo.empty()) {
        throw RemoteForwardingException("forwardTo cannot be empty");
    }

    if (stopped) {
        throw RemoteForwardingException("tunnel stopped");
    }

    auto lock = LockIfDifferentThread();
    if (state < SdkState_PrimaryReverseForwardingSucceeded) {
        throw RemoteForwardingException("primary reverse forwarding for this tunnel");
    }

    additionalForwardings.push_back({bindAddress, forwardTo});

    internalRequestAdditionalRemoteForwarding(bindAddress, forwardTo);
}

//==============
void
Sdk::HandleSessionInitiated()
{
    LOGD("Initiated");

    if (state != SdkState_SessionInitiating)
        return;

    state = SdkState_SessionInitiated;
    if (eventHandler && !reconnectNow)
        eventHandler->OnConnected();
    authenticate();
}

void
Sdk::HandleSessionAuthenticatedAsClient(std::vector<tString> messages)
{
    authenticationMsg = messages;
    state = SdkState_Authenticated;
    reconnectionState = SdkState_Reconnect_Connected;
    LOGD("OnAuthenticated");
    if (eventHandler && !reconnectNow)
        eventHandler->OnAuthenticated();

    pollController->StopPolling();
}

void
Sdk::HandleSessionAuthenticationFailed(tString error, std::vector<tString> authenticationFailed)
{
    // authenticated = false;
    authenticationMsg = authenticationFailed;
    state = SdkState_AuthenticationFailed;
    reconnectionState = SdkState_Reconnect_Failed;
    lastError = JoinString(authenticationFailed, "\r\n");
    LOGE("Authentication Failed");

    if (notificationConn && notificationConn->IsValid()) {
        notificationConn->CloseConn();
        notificationConn = nullptr;
        _notificateMonitorConn = nullptr;
    }

    if (eventHandler && !reconnectNow)
        eventHandler->OnAuthenticationFailed(authenticationMsg);

    if (baseConnection->IsValid()) {
        baseConnection->DeregisterFDEvenHandler();
        baseConnection->CloseConn();
    }

    pollController->StopPolling();
}

void
Sdk::HandleSessionRemoteForwardingSucceeded(protocol::tReqId reqId, std::vector<tString> urls)
{
    LOGT("Remote Fowarding succeeded");

    if (primaryForwardingReqId == reqId) {
        if (state >= SdkState_PrimaryReverseForwardingAccepted) {
            pollController->StopPolling();
            ABORT_WITH_MSG("Received multiple primary forwarding");
            return;
        }

        if (urls.size() > 0) //it would come with the primary forwarding only
            this->urls = urls;

        state = SdkState_PrimaryReverseForwardingAccepted;

        tunnelInitiated();
        primaryForwardingCheckTimeout = pollController->SetTimeout(5 * SECOND, thisPtr, &Sdk::handlePrimaryForwardingFailed, tString("could not fetch greetingmsg"));
        // Probably 5 second is not a lot. But, we want it to fail soon.
        LOGD("Primary forwarding done");

        updateForwardMapWithPrimaryForwarding();

        return;
    }

    if (pendingRemoteForwardingMap.find(reqId) == pendingRemoteForwardingMap.end()) {
        LOGE("reqId does not exists");
        return;
    }

    auto [bindAddressHost, bindAddressPort, forwardToHost, forwardToPort, bindAddress, forwardTo] = pendingRemoteForwardingMap[reqId];
    pendingRemoteForwardingMap.erase(reqId);

    auto remoteBinding = std::tuple(bindAddressHost, bindAddressPort);
    auto localForwarding = std::tuple(forwardToHost, forwardToPort);

    if (remoteForwardings.find(remoteBinding) != remoteForwardings.end()) {
        LOGE("This not supposed to happen"); //cannot test it ever
        return;
    }

    remoteForwardings[remoteBinding] = localForwarding;

    updateForwardMapWithAdditionalForwarding();

    if (eventHandler && !reconnectNow)
        eventHandler->OnAdditionalForwardingSucceeded(bindAddress, forwardTo);
}

void
Sdk::HandleSessionRemoteForwardingFailed(protocol::tReqId reqId, tString error)
{
    LOGE("Remote Fowarding failed", error);

    lastError = error;

    if (primaryForwardingReqId == reqId) {
        DEFER({pollController->StopPolling();});
        if (state >= SdkState_PrimaryReverseForwardingAccepted) {
            ABORT_WITH_MSG("Received multiple primary forwarding");
            return;
        }

        handlePrimaryForwardingFailed(error);
        return;
    }

    if (pendingRemoteForwardingMap.find(reqId) == pendingRemoteForwardingMap.end()) {
        LOGE("reqId does not exists");
        return;
    }

    // auto [bindAddress, forwardTo] = pendingRemoteForwardingMap[reqId];
    auto [bindAddressHost, bindAddressPort, forwardToHost, forwardToPort, bindAddress, forwardTo] = pendingRemoteForwardingMap[reqId];
    pendingRemoteForwardingMap.erase(reqId);
    if (eventHandler && !reconnectNow)
        eventHandler->OnAdditionalForwardingFailed(bindAddress, forwardTo, error);
}

void
Sdk::HandleSessionNewChannelRequest(protocol::ChannelPtr channel)
{
    LOGT("Called `" + tString(__func__) + "`");

    net::NetworkConnectionPtr netConn;
    auto chanType = channel->GetType();

    if (chanType == protocol::ChannelType_PTY) {
        channel->Reject("Pty is not acceptable here");
        LOGE("Pty channel received here.");
    } else if (chanType == protocol::ChannelType_Stream) {
        auto toHost = channel->GetDestHost();
        auto toPort = channel->GetDestPort();
        { //just to ignore some variable
            auto remoteBinding = std::tuple(toHost, toPort);
            if (remoteForwardings.find(remoteBinding) == remoteForwardings.end()) {
                if (sdkConfig->tcpForwardTo) {
                    toHost = sdkConfig->tcpForwardTo->GetRawHost();
                    toPort = sdkConfig->tcpForwardTo->GetPort();
                } else {
                    channel->Reject("Tcp forwarding not enabled");
                    LOGE("Rejecting tcp forwarding");
                    return;
                }
            } else {
                auto [_1, _2] = remoteForwardings[remoteBinding];
                toHost = _1;
                toPort = _2;
            }
        }

        if (appHandlesNewChannel && eventHandler) {
            auto done = eventHandler->OnNewVisitorConnectionReceived(NewSdkChannelWraperPtr(channel, thisPtr));
            if (done)
                return;
        }

        try {
            auto netConnImpl = net::NewNetworkConnectionImplPtr(toHost, std::to_string(toPort), false);
            netConnImpl->SetPollController(pollController);
            netConnImpl->Connect(thisPtr, channel);
            return; //we will handle this in different place
        } catch(...) {
            LOGE("Could not connect to", sdkConfig->tcpForwardTo->ToString());
            channel->Reject("Could not connect to provided address");
            return;
        }
    } else if (chanType == protocol::ChannelType_DataGram) {
        if (!sdkConfig->udpForwardTo) {
            channel->Reject("Udp forwarding not enabled");
            return;
        }

        if (eventHandler) {
            auto done = eventHandler->OnNewVisitorConnectionReceived(NewSdkChannelWraperPtr(channel, thisPtr));
            if (done)
                return;
        }

        try {
            netConn = net::NewUdpConnectionImplPtr(sdkConfig->udpForwardTo->GetRawHost(), sdkConfig->udpForwardTo->GetPortStr());
        } catch(const std::exception& e) {
            LOGE("Could not connect to", sdkConfig->udpForwardTo->ToString(), " due to ", e.what());
            channel->Reject("Could not connect to provided address");
            return;
        } catch(...) {
            LOGE("Could not connect to", sdkConfig->udpForwardTo->ToString());
            channel->Reject("Could not connect to provided address");
            return;
        }
    }

    channel->Accept();
    netConn->SetPollController(pollController);
    auto channelForward = protocol::NewChannelConnectionForwarderPtr(channel, netConn, nullptr);
    channelForward->Start();
}

void
Sdk::HandleSessionKeepAliveResponseReceived(tUint64 tick)
{
    LOGT("Keepalive response recvd: tick: ", tick);
    lastKeepAliveTickReceived = tick;
}

void
Sdk::HandleSessionDisconnection(tString reason)
{
    lastError = reason;
    if (!session)
        return;

    if (eventHandler)
        eventHandler->OnDisconnected(reason, {reason});

    // cleanup();
    cleanupNow = true;
    pollController->StopPolling();
}

void
Sdk::HandleSessionConnectionReset()
{
    //Nothing much to do. just stop the poll controller if possible.
    baseConnection = nullptr; //it would be closed by sessios once this function returns.

    if (eventHandler)
        eventHandler->OnDisconnected("Connection reset", {"Connection reset"});

    // cleanup();
    cleanupNow = true;
    pollController->StopPolling();
}

void
Sdk::HandleSessionError(tUint32 errorNo, tString what, tBool recoverable)
{
    if (eventHandler)
        eventHandler->OnHandleError(errorNo, what, recoverable);

    if (!recoverable) {
        // cleanup();
        cleanupNow = true;
        pollController->StopPolling();
    }
}

void
Sdk::NewVisitor(net::NetworkConnectionPtr netConn) //Webdebugges
{
    auto addr = netConn->GetPeerAddress();
    auto peerHost = addr->IsUds() ? "localhost" : addr->GetIp();
    auto peerPort = addr->IsUds() ? 1234 : addr->GetPort();
    auto channel = session->CreateChannel(4300, "localhost", peerPort, peerHost);

    auto forwarder = protocol::NewChannelConnectionForwarderPtr(channel, netConn, nullptr);
    forwarder->Start();
}

void
Sdk::ConnectionListenerClosed(net::ConnectionListenerPtr listener)
{
    if (webDebugListener && listener == webDebugListener) {
        stopWebDebugger();
    }
}

len_t
Sdk::HandleFDReadWTag(PollableFDPtr pfd, tString tag)
{
    LOGT("Called `" + tString(__func__)+"`");
    auto netConn = pfd->DynamicPointerCast<net::NetworkConnection>();
    if (!netConn) {
        LOGT("Could not cast pointer");
        return 0;
    }
    auto [_1, _2] = netConn->Read(2048);
    auto len = _1;
    auto data = _2;

    if (tag == NOTIFICATION_FD) {
        if (len <= 0) {
            if (netConn->TryAgain())
                return -1;
            netConn->DeregisterFDEvenHandler();
            netConn->CloseConn();
            return len;
        }
        releaseAccessLock();
        semaphore->Wait();
        acquireAccessLock();
        semaphore->Notify();
        return len;
    }
    return 0;
}

len_t
Sdk::HandleFDWriteWTag(PollableFDPtr pfd, tString tag)
{
    return 0;
}

len_t
Sdk::HandleFDErrorWTag(PollableFDPtr pfd, tString tag, int16_t revents)
{
    pfd->DeregisterFDEvenHandler();
    pfd->CloseConn();
    return 0;
}

len_t
Sdk::HandleConnected(net::NetworkConnectionImplPtr netConn)
{
    LOGT("inside", __func__);
    protocol::ChannelPtr channel;
    if (!netConn)
        return 0;
    netConn->GetConnectEventPtr(channel);
    if (!channel) {
        netConn->CloseConn();
        return 0;
    }
    channel->Accept();

    auto channelForward = protocol::NewChannelConnectionForwarderPtr(channel, netConn, nullptr);
    channelForward->Start();
    return 0;
}

len_t
Sdk::HandleConnectionFailed(net::NetworkConnectionImplPtr netConn)
{
    LOGT("inside", __func__);
    protocol::ChannelPtr channel;
    if (!netConn)
        return 0;
    netConn->GetConnectEventPtr(channel);
    if (channel) {
        channel->Reject("Could not connect to destination");
    }
    return 0;
}

//Channel
void
Sdk::ChannelDataReceived(protocol::ChannelPtr channel)
{
    if (channel == usageChannel) {
        auto [len, data] = usageChannel->Recv(4096);
        if (len > 0) {
            if (eventHandler && usagesRunning) {
                lastUsagesUpdate = data->ToString();
                try {
                    json jd = json::parse(lastUsagesUpdate);
                    if (jd.contains("elaspedTime") && !jd.contains("elapsedTime"))
                        jd["elapsedTime"] = jd["elaspedTime"];
                    lastUsagesUpdate = jd.dump();
                } catch(...) {
                }
                eventHandler->OnUsageUpdate(lastUsagesUpdate);
            }
        }
        return;
    }
    auto tag = channel->GetUserTag();
    if (tag == PORT_CONF) {
        auto data = NewRawDataPtr();
        while (channel->HaveDataToRead()) {
            auto [len, newData] = channel->Recv(4096);
            if (len <= 0) {
                channel->Close();
                break;
            }
            data->AddData(newData);
        }
        try {
            json jdata = json::parse(data->ToString());
            auto portConfigPtr = NewPortConfigPtr();
            PINGGY_NLOHMANN_JSON_TO_PTR_VAR1(jdata, portConfigPtr,
                                ConfigTcp,
                                StatusPort,
                                UrlTcp,
                                UsageContinuousTcp,
                                UsageOnceLongPollTcp,
                                UsageTcp,
                                GreetingMsgTCP
                            )
            thisPtr->portConfig = portConfigPtr;

            if (!usageChannel) {
                initiateContinousUsages();
            }
            setupLocalChannelNGetData(portConfig->GreetingMsgTCP, GREETING_MSG_TAG);
        } catch(...) {
            LOGE("Some error while parsing port config");
        }
    } else if (tag == GREETING_MSG_TAG) {
        auto data = NewRawDataPtr();
        while (channel->HaveDataToRead()) {
            auto [len, newData] = channel->Recv(4096);
            if (len <= 0) {
                channel->Close();
                break;
            }
            data->AddData(newData);
        }
        if (data->Len) {
            try {
                json jdata = json::parse(data->ToString());
                if (jdata.contains("Msgs")) {
                    greetingMsgs = jdata["Msgs"].dump();
                }
                if (primaryForwardingCheckTimeout){
                    primaryForwardingCheckTimeout->DisArm();
                    primaryForwardingCheckTimeout = nullptr;
                }
                LOGD("greeting received");
                if (eventHandler && !reconnectNow)
                    eventHandler->OnPrimaryForwardingSucceeded(urls);
                state = SdkState_PrimaryReverseForwardingSucceeded;
                reconnectionState = SdkState_Reconnect_Forwarded;
                pollController->StopPolling();
            } catch(...) {
                LOGE("Some error while parsing greeting msg");
            }
        }
        return;
    }
    channel->Close();
}

void
Sdk::ChannelReadyToSend(protocol::ChannelPtr, tUint32)
{
}

void
Sdk::ChannelError(protocol::ChannelPtr channel, protocol::tError errorCode, tString errorText)
{
    ChannelCleanup(channel);
}

void
Sdk::ChannelCleanup(protocol::ChannelPtr channel)
{
    if (channel == usageChannel) {
        channel->Close();
        usageChannel = nullptr;
        //TODO reinitiate channel
        pollController->SetTimeout(SECOND, thisPtr, &Sdk::initiateContinousUsages);
    }

    channel->Close();
}

void
Sdk::ChannelRejected(protocol::ChannelPtr channel, tString reason)
{
    ChannelCleanup(channel);
}

void
Sdk::authenticate()
{
    if (state != SdkState_SessionInitiated)
        ABORT_WITH_MSG("You are not connected, how did you managed to call this?");

    state = SdkState_Authenticating;

    session->AuthenticateAsClient(sdkConfig->getUser(), sdkConfig->GetArguments(), sdkConfig->advancedParsing);
    LOGT("Authentication sent");
}

void
Sdk::tunnelInitiated()
{
    auto channel = session->CreateChannel(4, "localhost", 4300, "localhost");
    channel->RegisterEventHandler(thisPtr);
    channel->SetUserTag(PORT_CONF);
    channel->Connect();
}

bool Sdk::internalConnect(bool block)
{
    try {
        auto serverAddress = sdkConfig->serverAddress;
        baseConnection = net::NewNetworkConnectionImplPtr(serverAddress->GetRawHost(), serverAddress->GetPortStr());

        if (sdkConfig->ssl){
            auto sslConnection = net::NewSslNetworkConnectionPtr(baseConnection, sdkConfig->sniServerName);
            sslConnection->SetBaseCertificate(BASE_CERTIFICATE);
            sslConnection->Connect();
            baseConnection = sslConnection;
        }
    } catch (const std::exception &e) {
        LOGE("Exception occured: ", e.what());
        return false;
    }

    if (!baseConnection)
        return false;

    state = SdkState_Connected;

    baseConnection->SetPollController(pollController);

    session = protocol::NewSessionPtr(baseConnection);
    session->Start(thisPtr);
    LOGT("Session Started");

    keepAliveTask = pollController->SetInterval(5 * SECOND, thisPtr, &Sdk::sendKeepAlive);

    initiateNotificationChannel();

    state = SdkState_SessionInitiating;

    if (!block)
        return true;

    startPollingInCurrentThread();

    return state==SdkState_Authenticated; // It would disrupt state is different for what ever reason.
}

bool
Sdk::startPollingInCurrentThread()
{
    pollController->StartPolling();

    return true;
}

void
Sdk::initiateNotificationChannel()
{
    if (!notificationConn) {
        auto [_netConn1, _netConn2] = net::NetworkConnectionImpl::CreateConnectionPair();
        auto netConn = _netConn1;
        notificationConn = _netConn2;

        netConn->SetBlocking(false);
        netConn->SetPollController(pollController)->RegisterFDEvenHandler(thisPtr, NOTIFICATION_FD);
        _notificateMonitorConn = _netConn1;
    }
}

void
Sdk::throwWrongThreadException(tString funcname)
{
    auto curThreadId = std::this_thread::get_id();
    if (curThreadId != runningThreadId) {
        throw std::runtime_error("You cannot call " + funcname + " from different thread than the original connection is running");
    }
}

void
Sdk::cleanup()
{
    if (keepAliveTask) {
        keepAliveTask->DisArm();
        keepAliveTask = nullptr;
    }

    if (session) {
        session->Cleanup();
        session = nullptr;
    }

    stopWebDebugger();

    if (notificationConn) {
        notificationConn->CloseConn();
        notificationConn = nullptr;
        _notificateMonitorConn = nullptr;
    }

    if (pollController) {
        pollController->StopPolling();
        pollController->DeregisterAllHandlers();
        pollController = nullptr;
    }

    if (eventHandler) {
        eventHandler = nullptr;
    }
    stopped = true;
}

void
Sdk::sendKeepAlive()
{
    if (session) {
        auto tick = session->SendKeepAlive();
        pollController->SetTimeout(4*SECOND, thisPtr, &Sdk::keepAliveTimeout, tick);
        LOGT("Sending keepalive");
    }
}

void
Sdk::keepAliveTimeout(tUint64 tick)
{
    if (tick > (lastKeepAliveTickReceived+2) && !session->IsThereIncomingActivities()) {
        LOGI("Connection probably gone");
        if (keepAliveTask) {
            keepAliveTask->DisArm();
            keepAliveTask = nullptr;
        }
        if (sdkConfig->autoReconnect) {
            reconnectNow = true;
            reconnectionState = SdkState_Reconnect_Failed;
            lastError = "Tunnel seems unresponsive. Reconnecting";
            if (eventHandler)
                eventHandler->OnWillReconnect("Connection Reset", {"Reconnecting"});
        } else {
            Stop();
            HandleSessionConnectionReset();
        }
    }
    session->ResetIncomingActivities();
}

void Sdk::stopWebDebugger()
{
    if (webDebugListener && webDebugListener->IsListening()) {
        webDebugListener->DeregisterFDEvenHandler();
        webDebugListener->CloseConn();
        webDebugListener = nullptr;
    }
}

void Sdk::cleanupForReconnection()
{
    if (_notificateMonitorConn)
        _notificateMonitorConn->SetPollController(nullptr);
    if (session) {
        session->Cleanup();
        session = nullptr;
    }
    baseConnection = nullptr;
    pollController->DeregisterAllHandlers();
    pollController = nullptr;
    initPollController();
    if  (_notificateMonitorConn)
        _notificateMonitorConn->SetPollController(pollController)->RegisterFDEvenHandler(thisPtr, NOTIFICATION_FD);
    LOGI("Reconnecting Now");
    if (eventHandler)
        eventHandler->OnReconnecting(reconnectCounter);
}

void Sdk::initPollController()
{
#ifdef __WINDOWS_OS__
    auto pollController = common::NewPollControllerGenericPtr();
#else
    auto pollController = common::NewPollControllerLinuxPtr();
#endif

    this->pollController = pollController;
}

void
Sdk::initiateContinousUsages()
{
    if (usageChannel) {
        return; //No point in raising exception
    }

    if (reconnectNow || stopped)
        return;

    if (!portConfig)
        return;

    usageChannel = session->CreateChannel(portConfig->UsageContinuousTcp, "localhost", 0, "localhost");
    usageChannel->RegisterEventHandler(thisPtr);
    usageChannel->Connect();
}

bool
Sdk::resumeWithLock(tString funcName)
{   auto ret = pollController->PollOnce();
    auto success = (ret < 0 && app_get_errno() != EINTR ? false : true);
    return success;
}

void
Sdk::setupLocalChannelNGetData(port_t port, tString tag)
{
    auto channel = session->CreateChannel(port, "localhost", 0, "localhost");
    channel->SetUserTag(tag);
    channel->RegisterEventHandler(thisPtr);
    channel->Connect();
}

void
Sdk::handlePrimaryForwardingFailed(tString reason)
{
    DEFER({pollController->StopPolling();});
    state = SdkState_PrimaryReverseForwardingFailed;
    reconnectionState = SdkState_Reconnect_Failed;

    if (notificationConn && notificationConn->IsValid()) {
        notificationConn->CloseConn();
        notificationConn = nullptr;
        _notificateMonitorConn = nullptr;
    }

    if (eventHandler && !reconnectNow)
        eventHandler->OnPrimaryForwardingFailed(reason);

    if (baseConnection->IsValid()) {
        baseConnection->DeregisterFDEvenHandler();
        baseConnection->CloseConn();
    }

    LOGD("Primary forwarding failed");
}

bool
Sdk::internalRequestPrimaryRemoteForwarding(bool block)
{
    if (state < SdkState_Authenticated) {
        throw SdkException("Kindly login first");
    }

    if (state > SdkState_Authenticated) {
        return true;
    }

    if (!sdkConfig->tcpForwardTo && !sdkConfig->udpForwardTo) {
        ABORT_WITH_MSG("Atleast one of the forwarding is required");
    }

    // primaryReverseForwardingInitiated = true;
    state = SdkState_PrimaryReverseForwardingInitiated;

    tString host = "";
    port_t hostPort = 0;
    if (sdkConfig->tcpForwardTo) {
        host = sdkConfig->tcpForwardTo->GetRawHost();
        hostPort = sdkConfig->tcpForwardTo->GetPort();
    } else {
        host = sdkConfig->udpForwardTo->GetRawHost();
        hostPort = sdkConfig->udpForwardTo->GetPort();
    }

    primaryForwardingReqId = session->SendRemoteForwardRequest(PRIMARY_REMOTE_FORWARDING_PORT,
                                                                PRIMARY_REMOTE_FORWARDING_HOST,
                                                                hostPort, host);
    if (!block)
        return true;

    startPollingInCurrentThread();

    if (cleanupNow)
        cleanup();

    return state == SdkState_PrimaryReverseForwardingSucceeded ;
}

void
Sdk::acquireAccessLock()
{
    lockAccess.lock();
    running = true;
    runningThreadId = std::this_thread::get_id();
}


void
Sdk::releaseAccessLock()
{
    if (!running)
        throw SdkException(tString(__func__) + tString(" no one accessing lockAccess"));

    running = false;
    lockAccess.unlock();
}

void
Sdk::internalRequestAdditionalRemoteForwarding(tString bindAddress, tString forwardTo)
{
    auto bindUrl = NewUrlPtrNoProto(bindAddress);
    auto forwardUrl = NewUrlPtrNoProto(forwardTo);
    auto reqId = session->SendRemoteForwardRequest(bindUrl->GetPort(), bindUrl->GetRawHost(),
                                                    forwardUrl->GetPort(), forwardUrl->GetRawHost());

    if (reqId > 0) {
        pendingRemoteForwardingMap[reqId] = {   bindUrl->GetRawHost(), bindUrl->GetPort(),
                                                forwardUrl->GetRawHost(), forwardUrl->GetPort(),
                                                bindAddress, forwardTo
                                            };
    }
}

void
Sdk::updateForwardMapWithPrimaryForwarding()
{
    if (eventHandler) {
        tString changedUrls;
        try {
            std::map<tString, std::vector<tString>> changedUrlsMap {{"DEFAULT", urls}};
            json j = changedUrlsMap;
            changedUrls = j.dump();
        } catch(...) {
        }
        eventHandler->OnForwardingChanged(changedUrls);
    }
}

void
Sdk::updateForwardMapWithAdditionalForwarding()
{
    // for the time being this two things are exactly same.
    // however, this might change in the future.
    if (eventHandler) {
        tString changedUrls;
        try {
            std::map<tString, std::vector<tString>> changedUrlsMap {{"DEFAULT", urls}};
            json j = changedUrlsMap;
            changedUrls = j.dump();
        } catch(...) {
        }
        eventHandler->OnForwardingChanged(changedUrls);
    }
}


} // namespace sdk

