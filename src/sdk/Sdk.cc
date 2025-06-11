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

struct PortConfig: virtual public pinggy::SharedObject
{
    PortConfig():
            ConfigTcp(0), StatusPort(0), UrlTcp(0),
            UsageContinuousTcp(0), UsageOnceLongPollTcp(0), UsageTcp (0)
                                { }

    virtual
    ~PortConfig()               { }

    port_t                      ConfigTcp;
    port_t                      StatusPort;
    port_t                      UrlTcp;
    port_t                      UsageContinuousTcp;
    port_t                      UsageOnceLongPollTcp;
    port_t                      UsageTcp;
};
DefineMakeSharedPtr(PortConfig);

class ThreadLock : public pinggy::SharedObject
{
public:
    ThreadLock(std::mutex &accessLock):
            accessLock(accessLock)
    {
        accessLock.lock();
    }

    virtual
    ~ThreadLock()
    {
        accessLock.unlock();
    }

private:
    std::mutex                  &accessLock;
};
DefineMakeSharedPtr(ThreadLock);


Sdk::Sdk(SDKConfigPtr config, SdkEventHandlerPtr _eventHandler):
            connected(false),
            authenticated(false),
            started(false),
            running(false),
            primaryForwardingReqId(0),
            globalPollController(false),
            sdkConfig(config),
            eventHandler(_eventHandler),
            primaryReverseForwardingInitiated(false),
            block(false),
            automatic(false),
            primaryForwardingCompleted(false),
            stopped(false)
{
    if (!config) {
        sdkConfig = config = NewSDKConfigPtr();
    }
}

Sdk::~Sdk()
{
    if (session)
        session->Cleanup();
    if (webDebugListener)
        webDebugListener->CloseConn();
}

bool
Sdk::Connect(common::PollControllerPtr pollController)
{
    //==== Setup =========

    if (started)
        ABORT_WITH_MSG("Tunnel is already started");

    sdkConfig->validate();

    started = true;
    runningThreadId = std::this_thread::get_id();

    if (pollController) {
        globalPollController = true;
    } else {
#ifdef __WINDOWS_OS__
        pollController = common::NewPollControllerGenericPtr();
#else
        pollController = common::NewPollControllerLinuxPtr();
#endif
    }

    this->pollController = pollController;
    //=============


    auto serverAddress = sdkConfig->ServerAddress;
    baseConnection = net::NewNetworkConnectionImplPtr(serverAddress->GetHost(), serverAddress->GetPortStr());

    if (sdkConfig->Ssl){
        auto sslConnection = net::NewSslNetworkConnectionPtr(baseConnection, sdkConfig->SniServerName);
        sslConnection->SetBaseCertificate(BASE_CERTIFICATE);
        sslConnection->Connect();
        baseConnection = sslConnection;
    }

    baseConnection->SetPollController(pollController);

    session = protocol::NewSessionPtr(baseConnection);
    session->Start(thisPtr);
    LOGT("Session Started");

    if (!baseConnection)
        return false;

    startTunnel();

    return true;
}

bool
Sdk::Start(common::PollControllerPtr pollController)
{
    block = true;
    automatic = true;
    return Connect(pollController); //entry point
}

bool Sdk::Stop()
{
    auto lock = LockIfDifferentThread();
    if (stopped)
        return false;
    session->End("Connection close");
    if (webDebugListener) {
        webDebugListener->CloseConn();
        webDebugListener = nullptr;
    }
    stopped = true;
    return true;
}

tInt
Sdk::ResumeTunnel()
{
    if (!started)
        throw std::runtime_error("tunnel is not started");
    if (stopped)
        return -1;
    lockAccess.lock();
    running = true;
    runningThreadId = std::this_thread::get_id();
    auto ret = pollController->PollOnce();
    running = false;
    lockAccess.unlock();
    return ret;
}

std::vector<tString>
Sdk::GetUrls()
{
    if (!started) {
        LOGE("Tunnel is not running");
        return {};
    }
    if (stopped)
        return {};
    auto var = LockIfDifferentThread();
    LOGD("Returning urls");
    return urls;
}

tUint64
Sdk::SendKeepAlive()
{
    if (!running) {
        LOGE("Tunnel is not running");
        return 0;
    }
    if (stopped)
        return 0;
    auto var = LockIfDifferentThread();
    return session->SendKeepAlive();
}

tString
Sdk::GetEndMessage()
{
    if (!started) {
        return lastError;
    }
    auto var = LockIfDifferentThread();
    return lastError;
}

SdkEventHandlerPtr
Sdk::GetSdkEventHandler()
{
    return eventHandler;
}

ThreadLockPtr Sdk::LockIfDifferentThread()
{
    auto curThreadId = std::this_thread::get_id();
    if (curThreadId == runningThreadId) { //it will never be same unless they are really same. We do not change running thread without lock
        LOGD("Same thread. not locking.")
        return nullptr;
    }
    semaphore.Wait();
    sdk::ThreadLockPtr threadLock = nullptr;
    if (notificationConn) {
        notificationConn->Write(NewRawDataPtr(tString("a")));
        threadLock = NewThreadLockPtr(new ThreadLock(lockAccess));
    }
    semaphore.Notify();
    return threadLock;
}

port_t
Sdk::StartWebDebugging(port_t port)
{
    if (!authenticated) {
        ABORT_WITH_MSG("You are not logged in. How did you managed to come here?" );
    }

    auto lock = LockIfDifferentThread();
    if (webDebugListener) {
        throw WebDebuggerException("Web debugger is running already for this tunnel");
    }

    webDebugListener = net::NewConnectionListnerImplPtr(port, false);
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

void
Sdk::RequestPrimaryRemoteForwarding()
{
    if (!authenticated) {
        ABORT_WITH_MSG("You are not logged in. How did you managed to come here?" );
    }

    auto lock = LockIfDifferentThread();
    if (primaryReverseForwardingInitiated) {
        throw RemoteForwardingException("Primary reverse forwarding is running already for this tunnel");
    }

    if (!sdkConfig->TcpForwardTo && !sdkConfig->UdpForwardTo) {
        ABORT_WITH_MSG("Atleast one of the forwarding is required");
    }

    primaryReverseForwardingInitiated = true;

    tString host = "";
    port_t hostPort = 0;
    if (sdkConfig->TcpForwardTo) {
        host = sdkConfig->TcpForwardTo->GetHost();
        hostPort = sdkConfig->TcpForwardTo->GetPort();
    } else {
        host = sdkConfig->UdpForwardTo->GetHost();
        hostPort = sdkConfig->UdpForwardTo->GetPort();
    }

    primaryForwardingReqId = session->SendRemoteForwardRequest(PRIMARY_REMOTE_FORWARDING_PORT,
                                                                PRIMARY_REMOTE_FORWARDING_HOST,
                                                                hostPort, host);
}

void
Sdk::RequestAdditionalRemoteForwarding(UrlPtr bindAddress, UrlPtr forwardTo)
{
    if (!authenticated) {
        ABORT_WITH_MSG("You are not logged in. How did you managed to come here?" );
    }

    if (!bindAddress) {
        throw RemoteForwardingException("bindAddress cannot be empty");
    }

    if (!forwardTo) {
        throw RemoteForwardingException("forwardTo cannot be empty");
    }

    if (stopped) {
        throw RemoteForwardingException("tunnel stopped");
    }

    auto lock = LockIfDifferentThread();
    if (!primaryReverseForwardingInitiated) {
        throw RemoteForwardingException("primary reverse forwarding for this tunnel");
    }

    auto reqId = session->SendRemoteForwardRequest(bindAddress->GetPort(), bindAddress->GetHost(),
                                                    forwardTo->GetPort(), forwardTo->GetHost());

    if (reqId > 0) {
        pendingRemoteForwardingMap[reqId] = {bindAddress, forwardTo};
    }
}

//==============
void
Sdk::HandleSessionInitiated()
{
    LOGD("Initiated");

    if (!started)
        return;
    connected = true;
    if (eventHandler)
        eventHandler->OnConnected();
    authenticate();
}

void
Sdk::HandleSessionAuthenticatedAsClient(std::vector<tString> messages)
{
    authenticationMsg = messages;
    authenticated = true;
    LOGD("OnAuthenticated");
    if (eventHandler)
        eventHandler->OnAuthenticated();
    if (automatic) {
        RequestPrimaryRemoteForwarding();
    }

}

void
Sdk::HandleSessionAuthenticationFailed(tString error, std::vector<tString> authenticationFailed)
{
    authenticated = false;
    authenticationMsg = authenticationFailed;
    lastError = JoinString(authenticationFailed, "\r\n");
    LOGE("Authentication Failed");

    if (notificationConn && notificationConn->IsValid()) {
        notificationConn->CloseConn();
        notificationConn = nullptr;
    }

    if (eventHandler)
        eventHandler->OnAuthenticationFailed(authenticationMsg);

    if (baseConnection->IsValid()) {
        baseConnection->DeregisterFDEvenHandler();
        baseConnection->CloseConn();
    }

    if (!globalPollController)
        pollController->StopPolling();
}

void
Sdk::HandleSessionRemoteForwardingSucceeded(protocol::tReqId reqId, std::vector<tString> urls)
{
    LOGT("Remote Fowarding succeeded");

    if (primaryForwardingReqId == reqId) {
        if (primaryForwardingCompleted) {
            Assert("Received multiple primary forwarding");
            return;
        }

        if (urls.size() > 0) //it would come with the primary forwarding only
            this->urls = urls;

        tunnelInitiated();
        primaryForwardingCompleted = true;

        if (eventHandler)
            eventHandler->OnPrimaryForwardingSucceeded(urls);
        LOGD("Primary forwarding done");
        return;
    }

    if (pendingRemoteForwardingMap.find(reqId) == pendingRemoteForwardingMap.end()) {
        LOGE("reqId does not exists");
        return;
    }

    auto [bindAddress, forwardTo] = pendingRemoteForwardingMap[reqId];
    pendingRemoteForwardingMap.erase(reqId);

    auto remoteBinding = std::tuple(bindAddress->GetHost(), bindAddress->GetPort());
    auto localForwarding = std::tuple(forwardTo->GetHost(), forwardTo->GetPort());

    if (remoteForwardings.find(remoteBinding) != remoteForwardings.end()) {
        LOGE("This not supposed to happen"); //cannot test it ever
        return;
    }

    remoteForwardings[remoteBinding] = localForwarding;

    if (eventHandler)
        eventHandler->OnRemoteForwardingSuccess(bindAddress, forwardTo);
}

void
Sdk::HandleSessionRemoteForwardingFailed(protocol::tReqId reqId, tString error)
{
    LOGE("Remote Fowarding failed", error);

    lastError = error;

    if (primaryForwardingReqId == reqId) {
        if (primaryForwardingCompleted) {
            Assert("Received multiple primary forwarding");
            return;
        }

        primaryForwardingCompleted = true;

        if (notificationConn && notificationConn->IsValid()) {
            notificationConn->CloseConn();
            notificationConn = nullptr;
        }

        if (eventHandler)
            eventHandler->OnPrimaryForwardingFailed(error);

        if (baseConnection->IsValid()) {
            baseConnection->DeregisterFDEvenHandler();
            baseConnection->CloseConn();
        }

        if (!globalPollController)
            pollController->StopPolling();

        LOGD("Primary forwarding failed");

        return;
    }

    if (pendingRemoteForwardingMap.find(reqId) == pendingRemoteForwardingMap.end()) {
        LOGE("reqId does not exists");
        return;
    }

    auto [bindAddress, forwardTo] = pendingRemoteForwardingMap[reqId];
    pendingRemoteForwardingMap.erase(reqId);
    if (eventHandler)
        eventHandler->OnRemoteForwardingFailed(bindAddress, forwardTo, error);
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
                if (sdkConfig->TcpForwardTo) {
                    toHost = sdkConfig->TcpForwardTo->GetHost();
                    toPort = sdkConfig->TcpForwardTo->GetPort();
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

        if (eventHandler) {
            auto chan = NewSdkChannelWraperPtr(channel, thisPtr);
            eventHandler->OnNewVisitorConnectionReceived(chan);
            if (chan->IsResponeded())
                return;
        }

        try {
            auto netConnImpl = net::NewNetworkConnectionImplPtr(toHost, std::to_string(toPort), false);
            netConnImpl->SetPollController(pollController);
            netConnImpl->Connect(thisPtr, channel);
            return; //we will handle this in different place
        } catch(...) {
            LOGE("Could not connect to", sdkConfig->TcpForwardTo->ToString());
            channel->Reject("Could not connect to provided address");
            return;
        }
    } else if (chanType == protocol::ChannelType_DataGram) {
        if (!sdkConfig->UdpForwardTo) {
            channel->Reject("Udp forwarding not enabled");
            return;
        }

        if (eventHandler) {
            auto done = eventHandler->OnNewVisitorConnectionReceived(NewSdkChannelWraperPtr(channel, thisPtr));
            if (done)
                return;
        }

        try {
            netConn = net::NewUdpConnectionImplPtr(sdkConfig->UdpForwardTo->GetHost(), sdkConfig->UdpForwardTo->GetPortStr());
        } catch(const std::exception& e) {
            LOGE("Could not connect to", sdkConfig->UdpForwardTo->ToString(), " due to ", e.what());
            channel->Reject("Could not connect to provided address");
            return;
        } catch(...) {
            LOGE("Could not connect to", sdkConfig->UdpForwardTo->ToString());
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
    if (eventHandler)
        eventHandler->KeepAliveResponse(tick);
}

void
Sdk::HandleSessionDisconnection(tString reason)
{
    lastError = reason;
    if (!session)
        return;

    session->Cleanup();
    cleanup();

    if (eventHandler)
        eventHandler->OnDisconnected(reason, {reason});
}

void
Sdk::HandleSessionConnectionReset()
{
    //Nothing much to do. just stop the poll controller if possible.
    baseConnection = nullptr; //it would be closed by sessios once this function returns.

    cleanup();

    if (eventHandler)
        eventHandler->OnDisconnected("Connection reset", {"Connection reset"});
}

void
Sdk::HandleSessionError(tUint32 errorNo, tString what, tBool recoverable)
{
    if (!recoverable) {
        session->Cleanup();
        cleanup();
    }
    if (eventHandler)
        eventHandler->OnHandleError(errorNo, what, recoverable);
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
Sdk::ConnectionListenerClosed(net::ConnectionListnerPtr listener)
{
    if (webDebugListener) {
        webDebugListener->CloseConn();
        webDebugListener = nullptr;
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

    if (tag == PORT_CONF) {
        if (len <= 0) {
            if (netConn->TryAgain()) {
                return -1;
            }
            netConn->DeregisterFDEvenHandler();
            netConn->CloseConn();
            return 0;
        }

        try {
            json jdata = json::parse(tString(data->GetData(), data->Len));
            portConfig = NewPortConfigPtr();
            PINGGY_NLOHMANN_JSON_TO_PTR_VAR1(jdata, portConfig,
                                ConfigTcp,
                                StatusPort,
                                UrlTcp,
                                UsageContinuousTcp,
                                UsageOnceLongPollTcp,
                                UsageTcp)
        } catch(...) {
            LOGE("Some error while parsing port config")
        }
    } else if (tag == NOTIFICATION_FD) {
        if (len <= 0) {
            if (netConn->TryAgain())
                return -1;
            netConn->DeregisterFDEvenHandler();
            netConn->CloseConn();
            return len;
        }
        lockAccess.unlock();
        semaphore.Wait();
        lockAccess.lock();
        semaphore.Notify();
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

void
Sdk::authenticate()
{
    if (!connected)
        ABORT_WITH_MSG("You are not connected, how did you managed to call this?");

    session->AuthenticateAsClient(sdkConfig->getUser(), sdkConfig->Argument, sdkConfig->AdvancedParsing);
    LOGT("Authentication sent");
}

void
Sdk::tunnelInitiated()
{
    net::DummyConnectionPtr conns[2];
    if (!net::DummyConnection::CreateDummyConnection(conns)) {
        LOGE("Could not create dummy connection to forward things");
        return;
    }

    conns[0]->SetPollController(pollController)->RegisterFDEvenHandler(thisPtr, PORT_CONF);
    conns[1]->SetPollController(pollController);
    auto channel = session->CreateChannel(4, "localhost", 4300, "localhost");

    auto forwarder = protocol::NewChannelConnectionForwarderPtr(channel, conns[1], nullptr);
    forwarder->Start();
    return;
}

bool
Sdk::startTunnel()
{
    lockAccess.lock();
    running = true;

    auto [_netConn1, _netConn2] = net::NetworkConnectionImpl::CreateConnectionPair();
    auto netConn = _netConn1;
    notificationConn = _netConn2;

    netConn->SetBlocking(false);
    netConn->SetPollController(pollController)->RegisterFDEvenHandler(thisPtr, NOTIFICATION_FD);

    // accessLockInner.lock();

    if (!globalPollController && block)
        this->pollController->StartPolling();

    // accessLockInner.unlock();
    running = false;
    lockAccess.unlock();
    return true;
}

void Sdk::throwWrongThreadException(tString funcname)
{
    auto curThreadId = std::this_thread::get_id();
    if (curThreadId != runningThreadId) {
        throw std::runtime_error("You cannot call " + funcname + " from different thread than the original connection is running");
    }
}

void Sdk::cleanup()
{
    if (!globalPollController) {
        // pollController->DeregisterAllHandlers();
        pollController->StopPolling();
    }
    if (webDebugListener && webDebugListener->IsListening()) {
        webDebugListener->DeregisterFDEvenHandler();
        webDebugListener->CloseConn();
        webDebugListener = nullptr;
    }

    if (notificationConn) {
        notificationConn->CloseConn();
        notificationConn = nullptr;
    }
    stopped = true;
}

//===============================================
//===============================================
//===============================================

SDKConfig::SDKConfig():
    Force(false),
    AdvancedParsing(true),
    Ssl(true),
    SniServerName("a.pinggy.io"),
    Insecure(false)
{
}

void
SDKConfig::validate()
{
    if (!ServerAddress) {
        ServerAddress = NewUrlPtr("a.pinggy.ip:443");
    }

    if (TcpForwardTo && Mode == "") {
        Mode = "http";
    }

    if (UdpForwardTo && UdpMode == "") {
        UdpMode = "udp";
    }

    if (Mode != "http" && Mode != "tcp" && Mode != "tls" && Mode != "tlstcp")
        Mode = "";
    if (UdpMode != "udp")
        UdpMode = "";
    if (Mode.empty() && UdpMode.empty())
        Mode = "http";
}

tString
SDKConfig::getUser()
{
    tString user = "";
    if (!Token.empty()) {
        user += "+" + Token;
    }

    if (!Mode.empty()) {
        user += "+" + Mode;
    }

    if (!UdpMode.empty()) {
        user += "+" + UdpMode;
    }

    if (Force) {
        user += "+force";
    }

    return user.substr(1);
}


} // namespace sdk

