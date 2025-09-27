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

#include "Session.hh"
#include "Channel.hh"


namespace protocol
{

Session::Session(net::NetworkConnectionPtr netConn, common::PollControllerPtr pollController, bool asServer):
            netConn(netConn),
            serverMode(asServer),
            state(SessionState_Init),
            lastReqId(1023),
            endSent(false),
            keepAliveSentTick(0),
            incomingActivities(false),
            pollController(pollController)
{
    lastChannelId = 4;
    if (asServer)
        lastChannelId += 1;
    lastReqId = 3;
    features = NewSessionFeaturesPtr(PINGGY_SESSION_VERSION);
}

void
Session::SetSessionVersion(tUint32 version)
{
    Assert(state == SessionState_Init);
    features->SetVersion(version);
}

tUint32
Session::GetSessionVersion()
{
    Assert(state >= SessionState_Init);
    return features->GetVersion();
}

void Session::Start(SessionEventHandlerPtr eventHandler)
{
    this->eventHandler = eventHandler;
    transportManager = NewTransportManagerPtr(netConn, thisPtr);
    netConn->RegisterFDEvenHandler(transportManager);
    if (serverMode) {
        auto serverHello = NewServerHelloMsgPtr();
        serverHello->Version = features->GetVersion();
        sendMsg(serverHello);
        state = SessionState_ServerHelloSent;
    } else {
        auto clientHello = NewClientHelloMsgPtr();
        clientHello->Version = features->GetVersion();
        sendMsg(clientHello);
        state = SessionState_ClientHelloSent;
    }
}

void
Session::End(tString reason)
{
    if (!endSent) {
        auto msg = NewDisconnectMsgPtr(reason);
        sendMsg(msg);
        endSent = true;
    }
}

void
Session::Cleanup()
{
    for (auto pair : channels) {
        auto channel = pair.second;
        channel->cleanup();
    }
    channels.clear();
    netConn->DeregisterFDEvenHandler();
    netConn->CloseConn();
    netConn = nullptr;
    transportManager = nullptr;
    eventHandler = nullptr;
}

void
Session::AuthenticateAsClient(tString user, tString arguments, bool advanceParsing)
{
    if (state != SessionState_ClientHelloSent || serverMode) {
        ABORT_WITH_MSG("You are not allowed to authenticate as client");
    }
    auto msg = NewAuthenticateMsgPtr();
    msg->Username = user;
    msg->AdvancedParsing = advanceParsing?1:0;
    msg->Arguments = arguments;
    sendMsg(msg);
    state = SessionState_AuthenticationRequestSent;
}

tReqId
Session::SendRemoteForwardRequest(tInt16 listeningPort, tString listeningHost, tInt16 forwardingPort, tString forwardingHost, TunnelMode mode)
{
    if (state != SessionState_AuthenticatedAsClient) {
        ABORT_WITH_MSG("You are not allowed to authenticate as client");
    }
    auto msg = NewRemoteForwardRequestMsgPtr();
    msg->ReqId = lastReqId + 1;
    lastReqId += 1;
    msg->Bind = listeningHost;
    msg->ListeningPort = listeningPort;
    msg->ForwardingHost = forwardingHost;
    msg->ForwardingPort = forwardingPort;
    msg->Mode = mode;

    sendMsg(msg);
    return msg->ReqId;
}

void
Session::AuthenticationSucceeded(std::vector<tString> messages, TunnelInfoPtr info)
{
    if (state != SessionState_AuthenticationRequestRecved) {
        ABORT_WITH_MSG("Auth not received yet");
    }
    auto msg = NewAuthenticationResponseMsgPtr();
    msg->Success = true;
    msg->Messages = messages;
    msg->TunnelInfo = info;
    state = SessionState_AuthenticatedAsServer;

    sendMsg(msg);
}

void
Session::AuthenticationFailed(tString error, std::vector<tString> messages)
{
    if (state != SessionState_AuthenticationRequestRecved) {
        ABORT_WITH_MSG("Auth not received yet");
    }
    auto msg = NewAuthenticationResponseMsgPtr();
    msg->Success = false;
    msg->Error = error;
    msg->Messages = messages;

    sendMsg(msg);
}

void
Session::AcceptRemoteForwardRequest(tReqId reqId, std::vector<tString> urls)
{
    AcceptRemoteForwardRequest(reqId, InvalidForwardingId, urls, {});
}

void
Session::AcceptRemoteForwardRequest(tReqId reqId, tForwardingId forwardingId, std::vector<tString> urls,
                                    std::vector<RemoteForwardingPtr> remoteForwardings)
{
    if (state != SessionState_AuthenticatedAsServer) {
        ABORT_WITH_MSG("Auth not received yet");
    }
    auto msg = NewRemoteForwardResponseMsgPtr();
    msg->Success = true;
    msg->ReqId = reqId;
    msg->Urls = urls;
    msg->ForwardingId = forwardingId;
    msg->RemoteForwardings = remoteForwardings;
    sendMsg(msg);
}

void
Session::RejectRemoteForwardRequest(tReqId reqId, tString error)
{
    if (state != SessionState_AuthenticatedAsServer) {
        ABORT_WITH_MSG("Auth not received yet");
    }
    auto msg = NewRemoteForwardResponseMsgPtr();
    msg->ReqId = reqId;
    msg->Error = error;
    sendMsg(msg);
}

tUint64
Session::SendKeepAlive()
{
    auto msg = NewKeepAliveMsgPtr(keepAliveSentTick);
    keepAliveSentTick += 1;
    sendMsg(msg);
    return msg->Tick;
}

ChannelPtr
Session::CreateChannel(tUint16 destPort, tString destHost,
                        tUint16 srcPort, tString srcHost, tChannelType chanType)
{
    return CreateServerSideChannel(destPort, destHost, srcPort, srcHost, chanType, TunnelMode::None, InvalidForwardingId);
}

ChannelPtr
Session::CreateServerSideChannel(tUint16 destPort, tString destHost, tUint16 srcPort, tString srcHost, tChannelType chanType, TunnelMode mode, tForwardingId forwardingId)
{
    Assert(chanType > ChannelType_Invalid && chanType < MaxSupportedChannelType);

    auto cPtr                 = new Channel(thisPtr, features);
    auto channel              = NewChannelPtr(cPtr);
    channel->setChannelInfo(destPort, destHost, srcPort, srcHost, chanType, mode, forwardingId);

    return channel;
}

void
Session::SendUsages(ClientSpecificUsagesPtr usage)
{
    auto msg = NewUsagesMsgPtr();
    msg->Usages = usage;
    sendMsg(msg);
}

void
Session::HandleConnectionReset(net::NetworkConnectionPtr netConn)
{
    for (auto ch : channels)
        ch.second->cleanup();

    channels.clear(); //There will be no callback from transport any more
    if (eventHandler)
        eventHandler->HandleSessionConnectionReset();
    if (netConn) {
        netConn->DeregisterFDEvenHandler();
        netConn->CloseConn();
        netConn = nullptr;
    }
    transportManager = nullptr;
    eventHandler = nullptr;
}

void
Session::HandleIncomingDeserialize(DeserializerPtr deserializer)
{
    ProtoMsgPtr tMsg;// = Deserialize(deserializer);
    deserializer->Deserialize("msg", tMsg);
    incomingActivities = true;
    switch(tMsg->msgType) {
        case MsgType_ServerHello:
        {
            if (state != SessionState_ClientHelloSent)
                ABORT_WITH_MSG("Not expected state");

            auto msg = tMsg->DynamicPointerCast<ServerHelloMsg>();
            features->NegotiateVersion(msg->Version);
            eventHandler->HandleSessionInitiated();
        }
        break;

        case MsgType_ClientHello:
        {
            if (state != SessionState_ServerHelloSent)
                ABORT_WITH_MSG("Not expected state");

            auto msg = tMsg->DynamicPointerCast<ClientHelloMsg>();
            features->NegotiateVersion(msg->Version);
            eventHandler->HandleSessionInitiated();
        }
        break;

        case MsgType_Authenticate:
        {
            if (state != SessionState_ServerHelloSent) {
                ABORT_WITH_MSG("Not expected state");
            }
            auto msg = tMsg->DynamicPointerCast<AuthenticateMsg>();
            state = SessionState_AuthenticationRequestRecved;
            eventHandler->HandleSessionAuthenticationRequest(msg->Username, msg->Arguments, msg->AdvancedParsing?true:false);
        }
        break;

        case MsgType_AuthenticationResponse:
        {
            if (state != SessionState_AuthenticationRequestSent) {
                ABORT_WITH_MSG("Not expected state");
            }
            auto msg = tMsg->DynamicPointerCast<AuthenticationResponseMsg>();
            if (msg->Success) {
                state = SessionState_AuthenticatedAsClient;
                eventHandler->HandleSessionAuthenticatedAsClient(msg->Messages, msg->TunnelInfo);
            } else {
                state = SessionState_AuthenticationFailed;
                eventHandler->HandleSessionAuthenticationFailed(msg->Error, msg->Messages);
            }
        }
        break;

        case MsgType_RemoteForwardRequest:
        {
            if(state != SessionState_AuthenticatedAsServer) {
                ABORT_WITH_MSG("Not expected state");
            }
            auto msg = tMsg->DynamicPointerCast<RemoteForwardRequestMsg>();
            eventHandler->HandleSessionRemoteForwardRequest(msg->ReqId, msg->ListeningPort,
                            msg->Bind, msg->ForwardingPort, msg->ForwardingHost, (TunnelMode)msg->Mode);
        }
        break;

        case MsgType_RemoteForwardResponse:
        {
            if(state != SessionState_AuthenticatedAsClient) {
                ABORT_WITH_MSG("Not expected state");
            }
            auto msg = tMsg->DynamicPointerCast<RemoteForwardResponseMsg>();
            handleRemoteForwardResponse(msg->ReqId, msg->Success, msg->ForwardingId, msg->Urls, msg->RemoteForwardings, msg->Error);
        }
        break;

        case MsgType_SetupChannel:
        {
            if (state != SessionState_AuthenticatedAsClient && state != SessionState_AuthenticatedAsServer)
                ABORT_WITH_MSG("Not expected state");
            auto msg = tMsg->DynamicPointerCast<SetupChannelMsg>();
            handleNewChannel(msg);
        }
        break;

        case MsgType_SetupChannelResponse:
        {
            if (state != SessionState_AuthenticatedAsClient && state != SessionState_AuthenticatedAsServer)
                ABORT_WITH_MSG("Not expected state");
            auto msg = tMsg->DynamicPointerCast<SetupChannelResponseMsg>();
            if (channels.find(msg->ChannelId) == channels.end()) {
                // sendWarningMsg(0, "Unknown channel id " + std::to_string(msg->ChannelId) + " " + std::to_string(__LINE__));
                LOGD("Ignoring channel setup response as it is not registered: ", msg->ChannelId);
                break;
            }
            auto channel = channels.at(msg->ChannelId);
            channel->handleNewChannelResponse(msg);
        }
        break;

        case MsgType_ChannelData:
        {
            if (state != SessionState_AuthenticatedAsClient && state != SessionState_AuthenticatedAsServer)
                ABORT_WITH_MSG("Not expected state");
            auto msg = tMsg->DynamicPointerCast<ChannelDataMsg>();
            if (channels.find(msg->ChannelId) == channels.end()) {
                // sendWarningMsg(0, "Unknown channel id " + std::to_string(msg->ChannelId) + " " + std::to_string(__LINE__));
                LOGD("Ignoring channel data as it is not registered: ", msg->ChannelId);
                break;
            }
            auto channel = channels.at(msg->ChannelId);
            channel->handleChannelData(msg);
        }
        break;

        case MsgType_ChannelWindowAdjust:
        {
            if (state != SessionState_AuthenticatedAsClient && state != SessionState_AuthenticatedAsServer)
                ABORT_WITH_MSG("Not expected state");
            auto msg = tMsg->DynamicPointerCast<ChannelWindowAdjustMsg>();
            if (channels.find(msg->ChannelId) == channels.end()) {
                LOGD("Ignoring channel window adjust as it is not registered: ", msg->ChannelId);
                // sendWarningMsg(0, "Unknown channel id " + std::to_string(msg->ChannelId) + " " + std::to_string(__LINE__));
                break;
            }
            auto channel = channels.at(msg->ChannelId);
            channel->handleChannelWindowAdjust(msg);
        }
        break;

        case MsgType_ChannelClose:
        {
            if (state != SessionState_AuthenticatedAsClient && state != SessionState_AuthenticatedAsServer)
                ABORT_WITH_MSG("Not expected state");
            auto msg = tMsg->DynamicPointerCast<ChannelCloseMsg>();
            if (channels.find(msg->ChannelId) == channels.end()) {
                LOGD("Ignoring channel close as it is not registered: ", msg->ChannelId);
                // sendWarningMsg(0, "Unknown channel id " + std::to_string(msg->ChannelId) + " " + std::to_string(__LINE__));
                break;
            }
            LOGD("Channel close request: ", msg->ChannelId);
            auto channel = channels.at(msg->ChannelId);
            channel->handleChannelClose(msg);
        }
        break;

        case MsgType_ChannelError:
        {
            auto msg = tMsg->DynamicPointerCast<ChannelErrorMsg>();
            if (channels.find(msg->ChannelId) == channels.end()) {
                sendWarningMsg(0, "Unknown channel id " + std::to_string(msg->ChannelId) + " " + std::to_string(__LINE__));
                break;
            }
            auto channel = channels.at(msg->ChannelId);
            channel->handleChannelError(msg);
        }
        break;

        case MsgType_Error:
        {
            auto msg = tMsg->DynamicPointerCast<ErrorMsg>();
            eventHandler->HandleSessionError(msg->ErrorNo, msg->What, msg->Recoverable != 0);
        }
        break;

        case MsgType_Warning:
        {
            auto msg = tMsg->DynamicPointerCast<WarningMsg>();
            eventHandler->HandleSessionWarning(msg->ErrorNo, msg->What);
        }
        break;

        case MsgType_KeepAlive:
        {
            auto msg = tMsg->DynamicPointerCast<KeepAliveMsg>();
            auto newMsg = NewKeepAliveResponseMsgPtr(msg->Tick);
            // newMsg->ForTick = msg->Tick;
            sendMsg(newMsg);
        }
        break;

        case MsgType_KeepAliveResponse:
        {
            auto msg = tMsg->DynamicPointerCast<KeepAliveResponseMsg>();
            eventHandler->HandleSessionKeepAliveResponseReceived(msg->ForTick);
        }
        break;

        case MsgType_Disconnect:
        {
            auto msg = tMsg->DynamicPointerCast<DisconnectMsg>();
            endReason = msg->Reason;
            eventHandler->HandleSessionDisconnection(msg->Reason);
        }
        break;

        case MsgType_Usages:
        {
            auto msg = tMsg->DynamicPointerCast<UsagesMsg>();
            auto usages = msg->Usages;
            eventHandler->HandleSessionUsages(usages);
        }
        break;

        default:
            ABORT_WITH_MSG("Unhandled msg");
    }
}

void
Session::HandleIncompleteHandshake()
{
    ABORT_WITH_MSG("Something fishy. Cannot complete handshake");
}

/*
 * This function would be called when the underling channel is ready.
 * We would try to send all the data from our queue. This may now sounds
 * good, but I think this is good enough because this is a multiplexer
 * as well.
 */
void
Session::HandleReadyToSendBuffer()
{
    while (!sendQueue.empty()) {
        auto msg = sendQueue.front();
        auto success = transportManager->GetSerializer()->Serialize("msg", msg)->Send();
        if (success && msg->msgType == MsgType_Disconnect) {
            transportManager->EndTransport(); //this is not immediate
        }
        if (!success) {
            break;
        }
        sendQueue.pop();
    }
}

#define CHAN_ID_WRAP_VAL 0x3fff
uint16_t
Session::getChannelNewId()
{
    // this is one of the most interesting function. It always yeld next value which is two integer apart from the initial.
    // It will return either odd or even depending on the lastChanId only. So, while this class is running in server mode,
    // it will return odd values only. In some weird case, if it runs as client, it will return even values.
    // What ever it returns it will allways be a positive non-zero integer.
    // While these mechanism is fantastic, the value goes as high as (CHAN_ID_WRAP_VAL + 2)
    for(tChannelId i = ((lastChannelId & CHAN_ID_WRAP_VAL) + 2); i != lastChannelId; i=((i & CHAN_ID_WRAP_VAL) + 2) ) {
        if(!chanIdExists(i)) {
            LOGT( "Returning new chanid:", i );
            lastChannelId = i;
            return i;
        }
    }
    Assert(false);
    return 0;
}

bool
Session::chanIdExists(tChannelId chId)
{
    return channels.find(chId) != channels.end();
}

bool
Session::validRemoteChannel(tChannelId channelId)
{
    if (channelId > (CHAN_ID_WRAP_VAL + 2)) //maximum value for channelId can be as large as CHAN_ID_WRAP_VAL + 2
        return false;

    if (!serverMode) //means, remote is server
        return channelId%2 == 1;

    return channelId%2 == 0;
}

bool
Session::sendMsg(ProtoMsgPtr msg, bool queue)
{
    if (endSent)
        return false; //Assert might be appropriate

    if (msg->msgType == MsgType_Disconnect) {
        endSent = true;
    }

    bool success = false;
    if (sendQueue.empty()) {
        success = transportManager->GetSerializer()->Serialize("msg", msg)->Send();
    }
    if (success && msg->msgType == MsgType_Disconnect) {
        transportManager->EndTransport(); //this is not immediate
    }
    if (!success && queue) {
        sendQueue.push(msg);
        return true;
    }
    return success;
}

void
Session::sendErrorMsg(tUint32 errorNo, tString what, bool recoverable)
{
    auto errMsg = NewErrorMsgPtr();
    errMsg->ErrorNo = errorNo;
    errMsg->What = what;
    errMsg->Recoverable = recoverable;
    sendMsg(errMsg);
}

void
Session::sendWarningMsg(tUint32 errorNo, tString what)
{
    auto errMsg = NewWarningMsgPtr();
    errMsg->ErrorNo = errorNo;
    errMsg->What = what;
    sendMsg(errMsg);
}

void
Session::deregisterChannel(ChannelPtr channel)
{
    if (channels.find(channel->channelId) == channels.end()) {
        Assert(false && "Channel does not exists");
        return;
    }

    channels.erase(channel->channelId);
}

void
Session::registerChannel(ChannelPtr channel)
{
    if (channels.find(channel->channelId) != channels.end()) {
        ABORT_WITH_MSG("Channel already register");
        return;
    }

    channels[channel->channelId] = channel;
}

void
Session::handleRemoteForwardResponse(tReqId reqId, tUint8 success, tForwardingId forwardingId, std::vector<tString> urls,
                                        std::vector<RemoteForwardingPtr> remoteForwardings, tString error)
{
    if (success) {
        eventHandler->HandleSessionRemoteForwardingSucceeded(reqId, forwardingId, urls, remoteForwardings);
    } else {
        eventHandler->HandleSessionRemoteForwardingFailed(reqId, error);
    }
}

void
Session::handleNewChannel(SetupChannelMsgPtr msg)
{
    auto chanType = msg->ChannelType;
    tString rejectMsg = "";
    if (!validRemoteChannel(msg->ChannelId)) {
        rejectMsg += "Invalid channelId " + std::to_string(msg->ChannelId) + ". ";
        LOGD("Rejecting channel due to invalid channelId: ", msg->ChannelId);
    }
    if (chanType <= ChannelType_Invalid || chanType >= MaxSupportedChannelType) {
        rejectMsg += "Unknown channel type" + std::to_string(msg->ChannelType) + ". " + " " + std::to_string(__LINE__);
        LOGD("Rejecting channel due to unsuppoted channel type: ", chanType);
    }
    if (!rejectMsg.empty()) {
        auto chMsg          = NewSetupChannelResponseMsgPtr();
        chMsg->Accept       = false;
        chMsg->ChannelId    = msg->ChannelId;
        chMsg->Error        = rejectMsg;
        sendMsg(chMsg);
        return;
    }

    LOGT("New channel request received: ", msg->ChannelId, msg->ConnectToPort);

    auto channel            = NewChannelPtr(new Channel(thisPtr, features));
    channel->initiateIncomingChannel(msg);

    eventHandler->HandleSessionNewChannelRequest(channel);
}

common::PollableTaskPtr
Session::setupChannelCloseTimeout(ChannelPtr channel)
{
    if (!pollController) return nullptr;

    return pollController->SetTimeout(10*SECOND, channel, &Channel::closeTimeoutTriggered);
}

} // namespace protocol
