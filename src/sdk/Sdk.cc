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

tString NOTIFICATION_FD = "NOTIFICATION_FD";

#define PINGGY_LIFE_CYCLE_FUNC
#define PINGGY_LIFE_CYCLE_WRAPPER_FUNC
#define PINGGY_ATTRIBUTE_FUNC

//Rules:
// 1. no private function should lock the lockAccess
// 2. Every life cycle function should lock the lockAccess even before starting
// 3. Every non-life cycle functions that needs to access session (direct or undirect)
//    needs lockAccess (unless same thread and running)

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
            // primaryForwardingReqId(0),
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
        throw SdkException("Config not provided.");
    }
}

Sdk::~Sdk()
{
    cleanup();
}

bool PINGGY_LIFE_CYCLE_FUNC
Sdk::Connect(bool block)
{
    if (stopped) {
        throw SdkException("tunnel is stopped");
    }

    //==== Setup =========
    acquireAccessLock();
    DEFER(releaseAccessLock(););

    if (state >= SdkState_Connecting)
        return true;

    state = SdkState_Connecting;

    sdkConfig = sdkConfig->clone();

    sdkConfig->validate();

    initPollController();

    //=============

    return internalConnect(block);
}

bool PINGGY_LIFE_CYCLE_WRAPPER_FUNC
Sdk::Start()
{
    if (stopped) {
        throw SdkException("tunnel is stopped");
    }
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
    if (cleanupNow)
        return false;
    if (session) {
        session->End("Connection close");
    }

    // stopWebDebugger();
    cleanupNow = true;
    return true;
}

bool PINGGY_LIFE_CYCLE_FUNC
Sdk::ResumeTunnel(tInt32 timeout)
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
                    return resumeWithLock(__func__, timeout);
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
                            internalRequestAdditionalRemoteForwarding(ele);
                        }
                    }
                }
                break;

            case SdkState_Reconnect_Failed:
            default:
                {
                    cleanupForReconnection(); //cleaning up last connection
                    if (reconnectCounter >= sdkConfig->maxReconnectAttempts && sdkConfig->maxReconnectAttempts != 0) {
                        reconnectionState = SdkState_Reconnect_Failed;
                        lastError = "Maximum reconnection attempts reached. Exiting.";
                        reconnectNow = false;
                        cleanupNow = true;
                        cleanupReason = lastError;
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

    auto success = resumeWithLock(__func__, timeout);
    return success;
}

std::vector<tString> PINGGY_ATTRIBUTE_FUNC
Sdk::GetUrls()
{
    if (state < SdkState_PrimaryReverseForwardingSucceeded) {
        LOGE("Tunnel is not running");
        return {};
    }
    if (cleanupNow)
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
        LOGD("Same thread. not locking.", curThreadId, runningThreadId);
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
    if (stopped) {
        throw SdkException("tunnel is stopped");
    }

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
    if (stopped) {
        throw SdkException("tunnel is stopped");
    }

    acquireAccessLock();
    DEFER({releaseAccessLock();});

    return internalRequestPrimaryRemoteForwarding(block);
}

void PINGGY_ATTRIBUTE_FUNC
Sdk::RequestAdditionalRemoteForwarding(tString forwardingType, tString bindingUrl, tString forwardTo)
{
    if (stopped) {
        throw SdkException("tunnel is stopped");
    }

    if (state < SdkState_Authenticated) {
        WebDebuggerException("You are not logged in. How did you managed to come here?" );
    }

    if (stopped) {
        throw RemoteForwardingException("tunnel stopped");
    }

    auto forwarding = SDKConfig::parseForwarding(forwardingType, bindingUrl, forwardTo);

    auto lock = LockIfDifferentThread();
    if (state < SdkState_PrimaryReverseForwardingSucceeded) {
        throw RemoteForwardingException("primary reverse forwarding for this tunnel");
    }

    additionalForwardings.push_back(forwarding);

    internalRequestAdditionalRemoteForwarding(forwarding);
}

void
Sdk::RequestAdditionalRemoteForwarding(tString forwardTo)
{
    if (stopped) {
        throw SdkException("tunnel is stopped");
    }

    if (state < SdkState_Authenticated) {
        WebDebuggerException("You are not logged in. How did you managed to come here?" );
    }

    if (stopped) {
        throw RemoteForwardingException("tunnel stopped");
    }

    auto forwarding = SDKConfig::parseForwarding(forwardTo);

    auto lock = LockIfDifferentThread();
    if (state < SdkState_PrimaryReverseForwardingSucceeded) {
        throw RemoteForwardingException("primary reverse forwarding for this tunnel");
    }

    additionalForwardings.push_back(forwarding);

    internalRequestAdditionalRemoteForwarding(forwarding);
}

tPort
Sdk::GetWebDebugListeningPort()
{
    if (!webDebugListener || !webDebugListener->IsListening())
        return 0;
    return webDebugListener->GetListeningPort();
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
    //TODO verify if primary mode allowed or not.
    if (session->GetSessionVersion() < PINGGY_SESSION_VERSION_1_02) {
        LOGE("Not authenticating for sdk version mismatch. Server is older that the sdk.");
        HandleSessionAuthenticationFailed("Incompatible SDK. Kindly upgrade", {"Incompatible SDK. Kindly upgrade"});
        return;
    }
    authenticate();
}

void
Sdk::HandleSessionAuthenticatedAsClient(std::vector<tString> messages, TunnelInfoPtr info)
{
    authenticationMsg = messages;
    state = SdkState_Authenticated;
    reconnectionState = SdkState_Reconnect_Connected;
    LOGD("OnAuthenticated");
    if (info) {
        json j = info->GreetingMsg;
        greetingMsgs = j.dump();
        portConfig = info->PortConfig;
    }
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

    stopped = true;

    pollController->StopPolling();
}

void
Sdk::HandleSessionRemoteForwardingSucceeded(protocol::tReqId reqId, tForwardingId forwardingId, std::vector<tString> urls,
                                            std::vector<RemoteForwardingPtr> remoteForwardings)
{
    LOGT("Remote Fowarding succeeded");

    auto elem = pendingRemoteForwardingRequestMap.find(reqId);
    if (elem != pendingRemoteForwardingRequestMap.end()) {

        if (state >= SdkState_PrimaryReverseForwardingAccepted) {
            pollController->StopPolling();
            ABORT_WITH_MSG("Received multiple primary forwarding");
            return;
        }

        Assert(forwardingId != InvalidForwardingId); //This is not supposed to happen as we would check the version long before this stage

        if (urls.size() > 0) //it would come with the primary forwarding only
            this->urls = urls;

        if (sdkForwardings.find(forwardingId) != sdkForwardings.end()) {
            ABORT_WITH_MSG("This not supposed to happen: ", forwardingId); //cannot test it ever
            return;
        }

        sdkForwardings[forwardingId] = elem->second;

        pendingRemoteForwardingRequestMap.erase(elem);

        updateForwardMap(remoteForwardings);

        if (pendingRemoteForwardingRequestMap.size() > 0)
            return;

        state = SdkState_PrimaryReverseForwardingAccepted;

        // tunnelInitiated();
        // Probably 5 second is not a lot. But, we want it to fail soon.
        primaryForwardingCompleted();
        LOGD("Primary forwarding done");

        return;
    }

    auto elem2 = pendingAdditionalRemoteForwardingMap.find(reqId);
    if (elem2 == pendingAdditionalRemoteForwardingMap.end()) {
        LOGE("reqId does not exists");
        return;
    }

    auto forwarding = elem2->second;
    pendingAdditionalRemoteForwardingMap.erase(reqId);

    if (sdkForwardings.find(forwardingId) != sdkForwardings.end()) {
        ABORT_WITH_MSG("This not supposed to happen"); //cannot test it ever
        return;
    }

    sdkForwardings[forwardingId] = forwarding;

    updateForwardMap(remoteForwardings);

    if (eventHandler && !reconnectNow) {
        eventHandler->OnAdditionalForwardingSucceeded(forwarding->origBindingUrl, forwarding->origForwardTo, forwarding->origForwardingType);
    }
}

void
Sdk::HandleSessionRemoteForwardingFailed(protocol::tReqId reqId, tString error)
{
    LOGE("Remote Fowarding failed", error);

    lastError = error;

    auto elem = pendingRemoteForwardingRequestMap.find(reqId);
    if (elem != pendingRemoteForwardingRequestMap.end()) {
        DEFER({pollController->StopPolling();});
        if (state >= SdkState_PrimaryReverseForwardingAccepted) {
            ABORT_WITH_MSG("Received multiple primary forwarding");
            return;
        }

        handlePrimaryForwardingFailed(error);
        return;
    }

    auto elem2 = pendingAdditionalRemoteForwardingMap.find(reqId);
    if (elem2 == pendingAdditionalRemoteForwardingMap.end()) {
        LOGE("reqId does not exists");
        return;
    }

    // auto [bindAddress, forwardTo] = pendingAdditionalRemoteForwardingMap[reqId];
    auto forwarding = pendingAdditionalRemoteForwardingMap[reqId];
    pendingAdditionalRemoteForwardingMap.erase(reqId);
    if (eventHandler && !reconnectNow) {
        eventHandler->OnAdditionalForwardingFailed(forwarding->origBindingUrl, forwarding->origForwardTo, forwarding->origForwardingType, error);
    }
}

void
Sdk::HandleSessionNewChannelRequest(protocol::ChannelPtr channel)
{
    LOGT("Called `" + tString(__func__) + "`");

    // net::NetworkConnectionPtr netConn;
    auto chanType = channel->GetType();

    auto forwardingId = channel->GetForwardingId();
    if (sdkForwardings.find(forwardingId) == sdkForwardings.end()) {
        channel->Reject("Unknown forwarding");
        return;
    }

    auto forwarding = sdkForwardings[forwardingId];
    auto toHost = forwarding->fwdToHost;
    auto toPort = forwarding->fwdToPort;

    if (chanType == protocol::ChannelType_PTY) {
        channel->Reject("Pty is not acceptable here");
        LOGE("Pty channel received here.");
    } else if (chanType == protocol::ChannelType_Stream) {
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
            LOGE("Could not connect to", forwarding->origForwardTo);
            channel->Reject("Could not connect to provided address");
            return;
        }
    } else if (chanType == protocol::ChannelType_DataGram) {
        if (appHandlesNewChannel && eventHandler) {
            auto done = eventHandler->OnNewVisitorConnectionReceived(NewSdkChannelWraperPtr(channel, thisPtr));
            if (done)
                return;
        }

        net::NetworkConnectionPtr netConn;
        try {
            netConn = net::NewUdpConnectionImplPtr(toHost, std::to_string(toPort));
        } catch(const std::exception& e) {
            LOGE("Could not connect to", forwarding->origForwardTo, " due to ", e.what());
            channel->Reject("Could not connect to provided address");
            return;
        } catch(...) {
            LOGE("Could not connect to", forwarding->origForwardTo);
            channel->Reject("Could not connect to provided address");
            return;
        }

        channel->Accept();
        netConn->SetPollController(pollController);
        auto channelForward = protocol::NewChannelConnectionForwarderPtr(channel, netConn, nullptr);
        channelForward->Start();
    }
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

    // if (eventHandler)
    //     eventHandler->OnDisconnected(reason, {reason});

    // cleanup();
    cleanupNow = true;
    cleanupReason = reason;
}

void
Sdk::HandleSessionConnectionReset()
{
    //Nothing much to do. just stop the poll controller if possible.
    baseConnection = nullptr; //it would be closed by sessios once this function returns.

    // if (eventHandler)
    //     eventHandler->OnDisconnected("Connection reset", {"Connection reset"});

    // cleanup();
    cleanupNow = true;
    // stopped = true;
    cleanupReason = "Connection reset";
}

void
Sdk::HandleSessionError(tUint32 errorNo, tString what, tBool recoverable)
{
    if (eventHandler)
        eventHandler->OnHandleError(errorNo, what, recoverable);

    if (!recoverable) {
        // cleanup();
        cleanupNow = true;
        cleanupReason = what;
    }
}

void
Sdk::HandleSessionUsages(ClientSpecificUsagesPtr usages)
{
    json jdata;
    PINGGY_NLOHMANN_JSON_FROM_PTR_VAR2(jdata, usages, \
                                            CLIENT_SPECIFIC_USAGES_JSON_FIELDS_MAP \
                                    );
    lastUsagesUpdate = jdata.dump();
    if (eventHandler)
        eventHandler->OnUsageUpdate(lastUsagesUpdate);
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
        acquireAccessLock(true);
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
    // if (channel == usageChannel) {
    //     auto [len, data] = usageChannel->Recv(4096);
    //     if (len > 0) {
    //         if (eventHandler && usagesRunning) {
    //             lastUsagesUpdate = data->ToString();
    //             try {
    //                 json jd = json::parse(lastUsagesUpdate);
    //                 if (jd.contains("elaspedTime") && !jd.contains("elapsedTime"))
    //                     jd["elapsedTime"] = jd["elaspedTime"];
    //                 lastUsagesUpdate = jd.dump();
    //             } catch(...) {
    //             }
    //             eventHandler->OnUsageUpdate(lastUsagesUpdate);
    //         }
    //     }
    //     return;
    // }
    // auto tag = channel->GetUserTag();
    // if (tag == PORT_CONF) {
    //     auto data = NewRawDataPtr();
    //     while (channel->HaveDataToRead()) {
    //         auto [len, newData] = channel->Recv(4096);
    //         if (len <= 0) {
    //             channel->Close();
    //             break;
    //         }
    //         data->AddData(newData);
    //     }
    //     try {
    //         json jdata = json::parse(data->ToString());
    //         auto portConfigPtr = NewSpecialPortConfigPtr();
    //         PINGGY_NLOHMANN_JSON_TO_PTR_VAR1(jdata, portConfigPtr,
    //                                         SPECIAL_PORT_BASIC_FIELDS
    //                                     )
    //         thisPtr->portConfig = portConfigPtr;

    //         if (!usageChannel) {
    //             initiateContinousUsages();
    //         }
    //         setupLocalChannelNGetData(portConfig->GreetingMsgTCP, GREETING_MSG_TAG);
    //     } catch(...) {
    //         LOGE("Some error while parsing port config");
    //     }
    // } else if (tag == GREETING_MSG_TAG) {
    //     auto data = NewRawDataPtr();
    //     while (channel->HaveDataToRead()) {
    //         auto [len, newData] = channel->Recv(4096);
    //         if (len <= 0) {
    //             channel->Close();
    //             break;
    //         }
    //         data->AddData(newData);
    //     }
    //     if (data->Len) {
    //         try {
    //             json jdata = json::parse(data->ToString());
    //             if (jdata.contains("Msgs")) {
    //                 greetingMsgs = jdata["Msgs"].dump();
    //             }
    //             LOGD("greeting received");
    //             primaryForwardingCompleted();
    //         } catch(...) {
    //             LOGE("Some error while parsing greeting msg");
    //         }
    //     }
    //     return;
    // }
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
    // if (channel == usageChannel) {
    //     channel->Close();
    //     usageChannel = nullptr;
    //     //TODO reinitiate channel
    //     pollController->SetTimeout(SECOND, thisPtr, &Sdk::initiateContinousUsages);
    // }

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

// void
// Sdk::tunnelInitiated()
// {
//     if (!session->IsImplicitGreeting()){
//         initiateContinousUsages();
//         return;
//     }

//     if (!primaryForwardingCheckTimeout)
//         primaryForwardingCheckTimeout = pollController->SetTimeout(5 * SECOND, thisPtr, &Sdk::handlePrimaryForwardingFailed, tString("could not fetch greetingmsg"));

//     auto channel = session->CreateChannel(4, "localhost", 4300, "localhost");
//     channel->RegisterEventHandler(thisPtr);
//     channel->SetUserTag(PORT_CONF);
//     channel->Connect();
// }

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
    session->SetSessionVersion(PINGGY_SESSION_VERSION_1_02);
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
    if (stopped) return;
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

    if (eventHandler)
        eventHandler->OnDisconnected("Ended", {"Ended"});

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
            // Stop();
            HandleSessionConnectionReset();
        }
    } else {
        session->ResetIncomingActivities();
    }
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

bool
Sdk::resumeWithLock(tString funcName, tInt32 timeout)
{   auto ret = pollController->PollOnce(timeout);
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
    stopped = true;
    LOGD(reason);
}

void
Sdk::primaryForwardingCompleted()
{
    if (eventHandler && !reconnectNow)
        eventHandler->OnPrimaryForwardingSucceeded(urls);
    state = SdkState_PrimaryReverseForwardingSucceeded;
    reconnectionState = SdkState_Reconnect_Forwarded;
    pollController->StopPolling();
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

    // primaryReverseForwardingInitiated = true;
    state = SdkState_PrimaryReverseForwardingInitiated;

    for (auto forwarding : sdkConfig->sdkForwardingList) {
        auto reqId = session->SendRemoteForwardRequest(forwarding->bindingPort, forwarding->bindingDomain,
                                                        forwarding->fwdToPort, forwarding->fwdToHost, forwarding->mode);
        pendingRemoteForwardingRequestMap[reqId] = forwarding;
    }

    if (!block)
        return true;

    startPollingInCurrentThread();

    if (cleanupNow)
        cleanup();

    return state == SdkState_PrimaryReverseForwardingSucceeded ;
}

void
Sdk::acquireAccessLock(bool block)
{
    auto curThreadId = std::this_thread::get_id();
    if (block || curThreadId != runningThreadId) {
        lockAccess.lock();
    } else {
        if (!lockAccess.try_lock()) {
            throw SdkException("Unable to grab mutex.");
        }
    }
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
Sdk::internalRequestAdditionalRemoteForwarding(SdkForwardingPtr forwarding)
{
    auto reqId = session->SendRemoteForwardRequest(forwarding->bindingPort, forwarding->bindingDomain,
                                                    forwarding->fwdToPort, forwarding->fwdToHost, forwarding->mode);

    if (reqId > 0) {
        pendingAdditionalRemoteForwardingMap[reqId] = forwarding;
    }
}

void
Sdk::updateForwardMap(std::vector<RemoteForwardingPtr> remoteForwardings)
{
    if (eventHandler) {
        tString changedUrls;
        try {
            json j = remoteForwardings;
            changedUrls = j.dump();
        } catch(...) {
        }
        eventHandler->OnForwardingChanged(changedUrls);
    }
}


} // namespace sdk

