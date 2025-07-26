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

    INIT --[Connect()]--> CONNECTING
    INIT --[initiateIncomingChannel()]--> CONNECT_RESPONDING

    CONNECTING --[handleNewChannelResponse(Accept)]--> CONNECTED
    CONNECTING --[handleNewChannelResponse(Reject)]--> REJECTED
    CONNECTING --[Close()]--> CLOSING

    CONNECT_RESPONDING --[Accept()]--> CONNECTED
    CONNECT_RESPONDING --[Reject()]--> REJECTED
    CONNECT_RESPONDING --[Close()]--> REJECTED

    CONNECTED --[Close()]--> CLOSING
    CONNECTED --[Recv() == 0]--> CLOSE_RESPONDING

    CLOSE_RESPONDING --[Close()]--> CLOSED
    CLOSING --[handleChannelClose()]--> CLOSED


Channel State Diagram
                    +--------+
                    | INIT   |
                    +--------+
                     /      \
          Connect() /        \ initiateIncomingChannel()
                   v          v
           +--------------+   +----------------------+
           |  CONNECTING  |   |  CONNECT_RESPONDING  |
           +--------------+   +----------------------+
             |     |    |           |        |
             |     |    |           |        |--[Close()]---> +-----------+
             |     |    |           |        |                | REJECTED  |
             |     |    |           |        `--[Reject()]--> +-----------+
             |     |    |       [Accept()]
             |     |    |           |
             |     |    |           v
             |     |    |       +-------------+
             |     |    |       |  CONNECTED  |
             |     |    |       +-------------+
             |     |    |           |                   +------------------+
             |     |    |           |--[Recv() == 0]--> | CLOSE_RESPONDING |
             |     |    |           |                   +------------------+
             |     |    |           |
             |     |    |           |               +----------+
             |     |    |           `--[Close()]--> | CLOSEING |
             |     |    |                           +----------+
             |     |    |
             |     |    |                                         +-----------+
             |     |    `-->[handleNewChannelResponse(Reject)]--> | REJECTED  |
             |     |                                              +-----------+
             |     |
             |     |                                         +-------------+
             |     `-->[handleNewChannelResponse(Accept)]--> |  CONNECTED  |
             |                                               +-------------+
         [Close()]
             |
             v
        +-----------+                              +--------+
        | CLOSING   | -->[handleChannelClose()]--> | CLOSED |
        +-----------+                              +--------+

*/


//Following same as golang ssh
#define MAX_PACKET (1<<15)
#define CHANNEL_WINDOW_SIZE (64 * MAX_PACKET)


Channel::Channel(SessionPtr session) : session(session),
                                        destPort(0),
                                        srcPort(0),
                                        chanType(ChannelType_Stream),
                                        remoteWindow(0),
                                        localWindow(CHANNEL_WINDOW_SIZE),
                                        remoteMaxPacket(0),
                                        localMaxPacket(MAX_PACKET),
                                        localConsumed(0),
                                        state(ChannelState_Init),
                                        allowWrite(false),
                                        allowRecv(false)
{
}

Channel::~Channel()
{
}

bool
Channel::Connect()
{
    if  (state != ChannelState_Init)
        return false;

    channelId               = session.lock()->getChannelNewId();
    auto msg                = NewSetupChannelMsgPtr();
    msg->ChannelId          = channelId;
    msg->ConnectToHost      = destHost;
    msg->ConnectToPort      = destPort;
    msg->SrcHost            = srcHost;
    msg->SrcPort            = srcPort;
    msg->ChannelType        = chanType;
    msg->InitialWindowSize  = localWindow;
    msg->MaxDataSize        = localMaxPacket;

    session.lock()->registerChannel(thisPtr);
    session.lock()->sendMsg(msg, true); //It is okay here as we are not going to send any other
    state = ChannelState_Connecting;
    return true;
}

bool
Channel::Accept()
{
    if (state != ChannelState_Connect_Responding)
        return false;

    session.lock()->registerChannel(thisPtr);

    auto msg                = NewSetupChannelResponseMsgPtr();
    msg->ChannelId          = channelId;
    msg->Accept             = true;
    msg->InitialWindowSize  = localWindow;
    msg->MaxDataSize        = localMaxPacket;

    session.lock()->sendMsg(msg, true); //It is okay here as we are not going to send any other
    state = ChannelState_Connected;
    allowWrite = true;
    allowRecv = true;
    return true;
}

bool
Channel::Reject(tString reason)
{
    if (state != ChannelState_Connect_Responding)
        return false;
    auto msg = NewSetupChannelResponseMsgPtr();
    msg->ChannelId = channelId;
    msg->Accept = false;
    msg->Error = reason;
    state = ChannelState_Rejected; //this would clear the channel as well

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

    if (    state != ChannelState_Connected
         && state != ChannelState_Close_Responding
         && state != ChannelState_Connecting)
        return false;

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

    return true;
}

RawData::tLen
Channel::Send(RawDataPtr rawData)
{
    if (!allowWrite)
        return -1;

    Assert(rawData->Len > 0);

    if (remoteWindow < (tUint32)rawData->Len) {
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
    if (state != ChannelState_Connected) {
        return {-2, nullptr};
    }

    if (recvQueue.empty()) {
        return {-1, nullptr};
    }

    auto top = recvQueue.front();
    if (top->Len == 0) { // it mean close received
        recvQueue.pop();
        state = ChannelState_Close_Responding;
        return {0, nullptr};
    }

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
    return recvQueue.size() > 0;
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
    if (state != ChannelState_Connected)
        return;
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
    if (state != ChannelState_Connecting)
        return;

    if (msg->Accept) {
        remoteMaxPacket = msg->MaxDataSize;
        remoteWindow = msg->InitialWindowSize;
        state = ChannelState_Connected;
        allowRecv = true;
        allowWrite = true;
        if (ev) {
            ev->ChannelAccepted(thisPtr);
            ev->ChannelReadyToSend(thisPtr, remoteWindow);
        } else {
            LOGE(channelId, ": Event handler required but not found");
        }
    } else {
        state = ChannelState_Rejected;
        if (ev) {
            ev->ChannelRejected(thisPtr, msg->Error);
            eventHandler = nullptr;
        }
        else
            LOGE(channelId, ": Event handler required but not found");
    }
}

void
Channel::handleChannelData(ChannelDataMsgPtr dataMsg)
{
    auto ev = eventHandler;

    if (state != ChannelState_Connected && state != ChannelState_Closing) //we are receiving data even after sending close
        return;

    if (!allowRecv) {
        LOGD("Recv not allowed here");
        return; //We have receive close already
    }

    if (localWindow < (tUint32)dataMsg->Data->Len) {
        LOGF("localWindow:", localWindow, "is not enough for current dataMsg of size", dataMsg->Data->Len);
        //TODO I am not sure what to do here.
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

    if (state != ChannelState_Connected && state != ChannelState_Closing) //we are receiving data even after sending close
        return;

    remoteWindow += msg->AdditionalBytes;

    if (ev)
        ev->ChannelReadyToSend(thisPtr, remoteWindow);
    else
        LOGE(channelId, ": Event handler required but not found");
}

void
Channel::handleChannelClose(ChannelCloseMsgPtr closeMsg)
{
    auto ev = eventHandler;

    if (state != ChannelState_Connected && state != ChannelState_Closing) //we are expecting close when it is connected or closed from this side
        return;

    if (!allowRecv) {
        LOGD("Recv not allowed here");
        return; //We have receive close already
    }

    if (state == ChannelState_Connected) {
        recvQueue.push(NewRawDataPtr());
        allowRecv = false;
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
Channel::setChannelInfo(tUint16 destPort, tString destHost, tUint16 srcPort, tString srcHost, tChannelType chanType)
{
    this->destHost         = destHost;
    this->srcHost          = srcHost;
    this->destPort         = destPort;
    this->srcPort          = srcPort;
    this->chanType         = chanType;
}

/*
 * It is supposed to be called only once.
 */
void
Channel::initiateIncomingChannel(SetupChannelMsgPtr msg)
{
    remoteMaxPacket    = msg->MaxDataSize;
    remoteWindow       = msg->InitialWindowSize;
    channelId          = msg->ChannelId;
    destHost           = msg->ConnectToHost;
    destPort           = msg->ConnectToPort;
    srcHost            = msg->SrcHost;
    srcPort            = msg->SrcPort;
    chanType           = (tChannelType)msg->ChannelType;
    state              = ChannelState_Connect_Responding;
}

} // namespace protocol
