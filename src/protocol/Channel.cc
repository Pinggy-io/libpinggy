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

#include "Channel.hh"
#include "Session.hh"

namespace protocol
{

/*
# Channel State Transitions

Channel State Diagram
R = Channel registered
D = Channel deregistered
r = Channel registered for fastChannel
d = channel deregistered for fastChannel

                        +----------------------+
                        |          INIT        |
                        +----------------------+
                             |            |
                   [Connect() R ]     [initiateIncomingChannel() r ]
                    ,---'                         '---.
                    v                                 v
        +--------------+                           +----------------------+
        |  CONNECTING  |                           |  CONNECT_RESPONDING  |
        +--------------+                           +----------------------+
            |   |   |                                      |      |
            |   |   |                                      |      |
            |   |    \                                     |      |
            |   |     \                                    |      |
            |   |      \                                   |      |
            |   |  [handleNewChannelResponse(Reject) D ]   |      |
            |   |                 |                       /|      |
            |   |                 |            [Close()]-' |      |
            |   |                 v                /       |      |
            |   |           +----------+          /        |      |
            |   |           | REJECTED | <--[Reject() d ]--'      |
            |   |           +----------+                          |
            |   |                                                 |
            |    \                                      ,---------'
            |     `----.                                |
            |           \                          [Accept() R ]
            |            \                              |
            |   [handleNewChannelResponse(Accept)]      |
            |                 |                         |
            |                 | ,-----------------------'
            |                 |/
            |                 v
            |           +-----------+
            |           | CONNECTED |
            |           +-----------+
            |                 |
            |                 |`---[handleChannelClose()]
            |                 |              |
            |                 |              v
            `--------------.  |     +------------------+
                            \ |     | CLOSE_RESPONDING |
                             \|     +------------------+
                              |              |
                              |              |
                              |        [Close() D ]
                              |              |
                              |              |
                          [Close()]           \
                              |                \
                              v                 \
                         +---------+             |
                         | CLOSING |             |
                         +---------+             |
                              |                  v
                              v             +--------+
               [handleChannelClose() D ]--> | CLOSED |
                                            +--------+



*/


//Following same as golang ssh
#define MAX_PACKET (1<<15)
#define CHANNEL_WINDOW_SIZE (64 * MAX_PACKET)

#define INITIAL_REMOTE_WINDOW_SIZE MAX_PACKET

#define _IS_STATE_ALL_FUNC(x) \
        (state == x) ||

#define _IS_STATE_LAST_FUNC(x) \
        (state == x)

#define IN_STATE_COND(...) \
    APP_MACRO_FOR_EACH_LASTFUNC(_IS_STATE_ALL_FUNC, _IS_STATE_LAST_FUNC, APP_MACRO_DUMMY, APP_MACRO_DUMMY, __VA_ARGS__)

#define NOT_IN_STATE_COND(...) \
        !(IN_STATE_COND(__VA_ARGS__))

#define IGNORE_IF_NOT_IN_STATE_NO_RETURN(...)                           \
        if (NOT_IN_STATE_COND(__VA_ARGS__))                             \
        {                                                               \
            LOGD(__func__, channelId, "Ignoring as current: ", state);  \
            return;                                                     \
        }

#define IGNORE_IF_NOT_IN_STATE_RETURN(ret, ...)                         \
        if (NOT_IN_STATE_COND(__VA_ARGS__))                             \
        {                                                               \
            LOGD(__func__, channelId, "Ignoring as current: ", state);  \
            return ret;                                                 \
        }


Channel::Channel(SessionPtr session, SessionFeaturesPtr features) :
            session(session),
            destPort(0),
            srcPort(0),
            chanType(ChannelType_Stream),
            remoteWindow(INITIAL_REMOTE_WINDOW_SIZE), //This is just give sender a head start
            localWindow(CHANNEL_WINDOW_SIZE),
            remoteMaxPacket(MAX_PACKET),
            localMaxPacket(MAX_PACKET),
            localConsumed(0),
            state(ChannelState_Init),
            allowWrite(false),
            features(features)
{
}

Channel::~Channel()
{
}

bool
Channel::Connect()
{
    IGNORE_IF_NOT_IN_STATE_RETURN(false, ChannelState_Init);

    channelId               = session.lock()->getChannelNewId();
    auto msg                = NewSetupChannelMsgPtr();
    msg->ChannelId          = channelId;
    msg->ConnectToHost      = destHost;
    msg->ConnectToPort      = destPort;
    msg->SrcHost            = srcHost;
    msg->SrcPort            = srcPort;
    msg->Mode               = mode;
    msg->ChannelType        = chanType;
    msg->InitialWindowSize  = localWindow;
    msg->MaxDataSize        = localMaxPacket;

    session.lock()->registerChannel(thisPtr);
    session.lock()->sendMsg(msg, true); //It is okay here as we are not going to send any other
    state = ChannelState_Connecting;
    if (features->IsFastConnect()) {
        allowWrite = true;
    }

    return true;
}

bool
Channel::Accept()
{
    IGNORE_IF_NOT_IN_STATE_RETURN(false, ChannelState_Connect_Responding);

    if (!features->IsFastConnect()) {
        session.lock()->registerChannel(thisPtr);
    }

    auto msg                = NewSetupChannelResponseMsgPtr();
    msg->ChannelId          = channelId;
    msg->Accept             = true;
    msg->InitialWindowSize  = localWindow;
    msg->MaxDataSize        = localMaxPacket;

    session.lock()->sendMsg(msg, true); //It is okay here as we are not going to send any other
    state = ChannelState_Connected;
    allowWrite = true;
    return true;
}

bool
Channel::Reject(tString reason)
{
    IGNORE_IF_NOT_IN_STATE_RETURN(false, ChannelState_Connect_Responding);

    auto msg = NewSetupChannelResponseMsgPtr();
    msg->ChannelId = channelId;
    msg->Accept = false;
    msg->Error = reason;
    allowWrite = false;
    state = ChannelState_Rejected; //this would clear the channel as well. I don't have to wait for close from other end.
                                   //Even if some data arrive from the other end, session would silently reject it.
    if (features->IsFastConnect())
        session.lock()->deregisterChannel(thisPtr);

    session.lock()->sendMsg(msg, true); //It is okay here as we are not going to send any other
    return true;
}

bool
Channel::Close()
{
    // Reject is a special case. final case
    if (state == ChannelState_Connect_Responding) {
        if (eventHandler)
            eventHandler = nullptr;
        return Reject("No action");
    }

    IGNORE_IF_NOT_IN_STATE_RETURN(false, ChannelState_Connected, ChannelState_Close_Responding, ChannelState_Connecting);

    auto msg = NewChannelCloseMsgPtr();
    msg->ChannelId = channelId;
    sendOrQueue(msg);

    allowWrite = false;
    if (state == ChannelState_Close_Responding) {
        state = ChannelState_Closed;
        session.lock()->deregisterChannel(thisPtr);
    } else {
        state = ChannelState_Closing;
    }

    if (eventHandler) {
        eventHandler = nullptr;
    }

    if (features->IsCloseTimeoutChannel() && IN_STATE_COND(ChannelState_Closing)) {
        session.lock()->setupChannelCloseTimeout(thisPtr);
    }

    return true;
}

RawData::tLen
Channel::Send(RawDataPtr rawData)
{
    if (!allowWrite)
        return -1;

    Assert(rawData->Len > 0);

    if (remoteWindow < (tUint32)rawData->Len) { //A precausionjust in case we have sent more data than the remote window.
        return -1;
    }

    auto workingRawData = rawData->Slice(0);
    while(remoteMaxPacket < (tUint32)workingRawData->Len) {
        auto msg        = NewChannelDataMsgPtr();
        msg->ChannelId  = channelId;
        msg->Data       = workingRawData->Slice(0, remoteMaxPacket);

        workingRawData->Consume(remoteMaxPacket);
        sendOrQueue(msg);
    }

    auto msg        = NewChannelDataMsgPtr();
    msg->ChannelId  = channelId;
    msg->Data       = workingRawData->Slice(0);

    workingRawData->Consume();
    sendOrQueue(msg);
    Assert(workingRawData->Len == 0);
    remoteWindow -= rawData->Len;
    return rawData->Len;
}

std::tuple<RawData::tLen, RawDataPtr>
Channel::Recv(RawData::tLen len)
{
    if (NOT_IN_STATE_COND(ChannelState_Connected, ChannelState_Close_Responding)) {
        return {-2, nullptr};
    }

    if (recvQueue.empty()) {
        if (state == ChannelState_Close_Responding)
            return {0, nullptr};
        return {-1, nullptr};
    }

    auto top = recvQueue.front();

    RawDataPtr raw;
    if (top->Len > len) {
        raw = top->Slice(0, len);
        top->Consume(len);
    } else {
        raw = top;
        recvQueue.pop();
    }

    adjustWindow(raw->Len);

    return {raw->Len, raw};
}

bool
Channel::HaveDataToRead()
{
    return recvQueue.size() > 0 || state == ChannelState_Close_Responding;
}

tUint32
Channel::HaveBufferToWrite()
{
    return remoteWindow;
}

void
Channel::cleanup()
{
    auto ev = eventHandler;
    if (ev)
        ev->ChannelCleanup(thisPtr);
    eventHandler = nullptr; //this is really important as it might held it back
}

void
Channel::sendOrQueue(ProtoMsgPtr msg)
{
    auto ev = eventHandler;

    auto success = session.lock()->sendMsg(msg, true);

    if (!success) {
        if (ev)
            ev->ChannelError(thisPtr, 0, "Cannot send msg");
    }
}

void
Channel::adjustWindow(tUint32 len)
{
    localConsumed += len;
    tInt32 sendAdj = 0;
    if (    CHANNEL_WINDOW_SIZE - localWindow > 3*localMaxPacket
         || localWindow < CHANNEL_WINDOW_SIZE/2) {
        sendAdj = localConsumed;
        localConsumed = 0;
        localWindow += sendAdj;
    }

    if (sendAdj) {
        auto msg = NewChannelWindowAdjustMsgPtr();
        msg->ChannelId = channelId;
        msg->AdditionalBytes = sendAdj;
        sendOrQueue(msg);
    }
}

void
Channel::handleNewChannelResponse(SetupChannelResponseMsgPtr msg)
{
    auto ev = eventHandler;

    IGNORE_IF_NOT_IN_STATE_NO_RETURN(ChannelState_Connecting);

    if (msg->Accept) {
        tUint32 sent = INITIAL_REMOTE_WINDOW_SIZE - remoteWindow; //sent will be positive all the time
        remoteMaxPacket = msg->MaxDataSize;
        remoteWindow = msg->InitialWindowSize;
        if (remoteWindow < remoteMaxPacket)
            ABORT_WITH_MSG("Remote window cannot even keep a single packet. " + std::to_string(remoteWindow) + " < " + std::to_string(remoteMaxPacket));
                                    //This is intentional so that server side stays safe
        if (sent <= remoteWindow)
            remoteWindow -= sent; // remote window will be positive.
        else
            remoteWindow = 0; //While this might cause wierd errors, it is okay in this context and other end aborts as soon as it receive data more than its buffer.
                              //Also, this would never happen if remote window is greater than INITIAL_REMOTE_WINDOW_SIZE, which server would comply. Malicious client
                              //may not.

        state = ChannelState_Connected;
        allowWrite = true;
        if (ev) {
            ev->ChannelAccepted(thisPtr);
            ev->ChannelReadyToSend(thisPtr, remoteWindow);
        } else {
            LOGE(channelId, ": Event handler required but not found");
        }
    } else {
        state = ChannelState_Rejected;
        if (features->IsFastConnect()) {
            auto closeMsg = NewChannelCloseMsgPtr();
            closeMsg->ChannelId = channelId;
            sendOrQueue(closeMsg);
        }
        if (ev) {
            ev->ChannelRejected(thisPtr, msg->Error);
            eventHandler = nullptr;
        }
        else
            LOGE(channelId, ": Event handler required but not found");
        session.lock()->deregisterChannel(thisPtr);
    }
}

void
Channel::handleChannelData(ChannelDataMsgPtr dataMsg)
{
    auto ev = eventHandler;

    LOGT(channelId, "Data received of length: ", dataMsg->Data->Len);

    //we are receiving data even after sending close
    if (features->IsFastConnect()) {
        IGNORE_IF_NOT_IN_STATE_NO_RETURN(ChannelState_Connected, ChannelState_Closing, ChannelState_Connect_Responding);
    } else {
        IGNORE_IF_NOT_IN_STATE_NO_RETURN(ChannelState_Connected, ChannelState_Closing);
    }

    if (localWindow < (tUint32)dataMsg->Data->Len) {
        LOGF("localWindow:", localWindow, "is not enough for current dataMsg of size", dataMsg->Data->Len);
        ABORT_WITH_MSG("Invalid size");
        return;
    }

    recvQueue.push(dataMsg->Data);
    localWindow -= dataMsg->Data->Len;


    if (ev)
        ev->ChannelDataReceived(thisPtr);
    else
        LOGE(channelId, ": Event handler required but not found");
}

void
Channel::handleChannelWindowAdjust(ChannelWindowAdjustMsgPtr msg)
{
    auto ev = eventHandler;

    //we are receiving data even after sending close
    IGNORE_IF_NOT_IN_STATE_NO_RETURN(ChannelState_Connected, ChannelState_Close_Responding);

    remoteWindow += msg->AdditionalBytes;

    if (ev)
        ev->ChannelReadyToSend(thisPtr, remoteWindow);
    else
        LOGE(channelId, ": Event handler required but not found. state:", state);
}

void
Channel::handleChannelClose(ChannelCloseMsgPtr closeMsg)
{
    auto ev = eventHandler;

    //we are expecting close when it is connected or closing
    IGNORE_IF_NOT_IN_STATE_NO_RETURN(ChannelState_Connected, ChannelState_Closing);

    if (state == ChannelState_Connected) {
        state = ChannelState_Close_Responding;
        if (ev)
            ev->ChannelDataReceived(thisPtr); //just notify a read event
        else
            LOGE(channelId, ": Event handler required but not found");
    } else {
        state = ChannelState_Closed;
        session.lock()->deregisterChannel(thisPtr);
    }
}

void
Channel::handleChannelError(ChannelErrorMsgPtr errMsg)
{
    auto ev = eventHandler;
    if (ev) {
        ev->ChannelError(thisPtr, errMsg->ErrorNo, errMsg->Error);
        eventHandler = nullptr;
    } else {
        LOGE(channelId, ": Event handler required but not found");
    }
}

void
Channel::setChannelInfo(tUint16 destPort, tString destHost, tUint16 srcPort, tString srcHost, tChannelType chanType, TunnelMode mode)
{
    this->destHost          = destHost;
    this->srcHost           = srcHost;
    this->destPort          = destPort;
    this->srcPort           = srcPort;
    this->chanType          = chanType;
    this->mode              = mode;
}

/*
 * It is supposed to be called only once.
 */
void
Channel::initiateIncomingChannel(SetupChannelMsgPtr msg)
{
    remoteMaxPacket     = msg->MaxDataSize;
    remoteWindow        = msg->InitialWindowSize;
    channelId           = msg->ChannelId;
    destHost            = msg->ConnectToHost;
    destPort            = msg->ConnectToPort;
    srcHost             = msg->SrcHost;
    srcPort             = msg->SrcPort;
    chanType            = (tChannelType)msg->ChannelType;
    mode                = (TunnelMode)msg->Mode;
    state               = ChannelState_Connect_Responding;

    if (remoteWindow < remoteMaxPacket)
        ABORT_WITH_MSG("Remote window cannot even keep a single packet. " + std::to_string(remoteWindow) + " < " + std::to_string(remoteMaxPacket));

    if (features->IsFastConnect()) {
        session.lock()->registerChannel(thisPtr);
    }
}

void
Channel::closeTimeoutTriggered()
{
    IGNORE_IF_NOT_IN_STATE_NO_RETURN(ChannelState_Closing);
    state = ChannelState_Closed;
    session.lock()->deregisterChannel(thisPtr);
}

} // namespace protocol
