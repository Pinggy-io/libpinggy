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
                                        closeSent(false),
                                        closeRcvd(false),
                                        closed(false),
                                        connectionSent(false),
                                        connectionRcvd(false),
                                        connected(false),
                                        rejected(false)
{
}

Channel::~Channel()
{
}

bool
Channel::Connect()
{
    if  (connectionSent || connectionRcvd)
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
    connectionSent = true;
    return true;
}

bool
Channel::Accept()
{
    if (!connectionRcvd || connectionSent)
        return false;
    if (rejected || connected)
        return false;
    session.lock()->registerChannel(thisPtr);

    auto msg                = NewSetupChannelResponseMsgPtr();
    msg->ChannelId          = channelId;
    msg->Accept             = true;
    msg->InitialWindowSize  = localWindow;
    msg->MaxDataSize        = localMaxPacket;

    session.lock()->sendMsg(msg, true); //It is okay here as we are not going to send any other
    connected = true;
    return true;
}

bool
Channel::Reject(tString reason)
{
    if (!connectionRcvd || connectionSent)
        return false;
    if (rejected || connected)
        return false;
    auto msg = NewSetupChannelResponseMsgPtr();
    msg->ChannelId = channelId;
    msg->Accept = false;
    msg->Error = reason;
    rejected = true;

    session.lock()->sendMsg(msg, true); //It is okay here as we are not going to send any other
    return true;
}

bool
Channel::Close()
{
    if (rejected)
        return true;
    if (closeSent)
        return false;

    if (connectionRcvd && !rejected && !connected) {
        Reject("No action");
        return true;
    }

    auto msg = NewChannelCloseMsgPtr();
    msg->ChannelId = channelId;
    sendOrQueue(msg);
    closeSent = true;
    if (closeRcvd)
        closed = true;
    if (closed)
        session.lock()->deregisterChannel(thisPtr);
    return true;
}

void
Channel::Cleanup()
{
    closed = true;
    closeSent = true;
    closeRcvd = true;
    if (eventHandler)
        eventHandler->ChannelCleanup(thisPtr);
    eventHandler = nullptr; //this is really important as it might held it back
}

RawData::tLen
Channel::Send(RawDataPtr rawData)
{
    if (closeSent || closeRcvd)
        return 0;
    if (!connected)
        return -1;
    if (remoteWindow < (tUint32)rawData->Len) {
        return -1;
    }

    Assert(rawData->Len > 0);

    auto workingRawData = rawData->Slice(0);
    while(remoteMaxPacket  < (tUint32)workingRawData->Len) {
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
    if (recvQueue.empty()) {
        if (closeRcvd || closeSent) {
            return {0, nullptr};
        }
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
    return recvQueue.size() > 0;
}

tUint32
Channel::HaveBufferToWrite()
{
    return remoteWindow;
}

void
Channel::sendOrQueue(ProtoMsgPtr msg)
{
    auto success = session.lock()->sendMsg(msg, true);
    Assert(success);
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
    if (!connectionSent) {
        return;
    }

    //TODO, what if user closes the channel before this response?

    connectionSent = false;

    if (msg->Accept) {
        remoteMaxPacket = msg->MaxDataSize;
        remoteWindow = msg->InitialWindowSize;
        connected = true;
        if (eventHandler) {
            eventHandler->ChannelAccepted(thisPtr);
            eventHandler->ChannelReadyToSend(thisPtr, remoteWindow);
        } else {
            LOGE(channelId, ": Event handler required but not found");
        }
    } else {
        session.lock()->deregisterChannel(thisPtr);
        rejected = true;
        if (eventHandler) {
            eventHandler->ChannelRejected(thisPtr, msg->Error);
            eventHandler = nullptr;
        }
        else
            LOGE(channelId, ": Event handler required but not found");
    }
}

void
Channel::handleChannelData(ChannelDataMsgPtr dataMsg)
{
    if (closeRcvd)
        return;

    if (localWindow < (tUint32)dataMsg->Data->Len) {
        LOGF("localWindow:", localWindow, "is not enough for current dataMsg of size", dataMsg->Data->Len);
        //TODO I am not sure what to do here.
        ABORT_WITH_MSG("Invalid size");
        return;
    }

    recvQueue.push(dataMsg->Data);
    localWindow -= dataMsg->Data->Len;

    if (closeSent) { //Storing data just in case.
        return;
    }

    if (eventHandler)
        eventHandler->ChannelDataReceived(thisPtr);
    else
        LOGE(channelId, ": Event handler required but not found");
}

void
Channel::handleChannelWindowAdjust(ChannelWindowAdjustMsgPtr msg)
{
    if (closeRcvd)
        return;

    remoteWindow += msg->AdditionalBytes;

    if (closeSent) //Event handler may not exists.
        return;

    if (eventHandler)
        eventHandler->ChannelReadyToSend(thisPtr, remoteWindow);
    else
        LOGE(channelId, ": Event handler required but not found");
}

void
Channel::handleChannelClose(ChannelCloseMsgPtr closeMsg)
{
    closeRcvd = true;
    auto alreadyClosed = false;
    if (closeSent) {
        closed = true;
        alreadyClosed = true;
    } else {
        auto msg = NewChannelCloseMsgPtr();
        msg->ChannelId = channelId;
        closeSent = true;
        sendOrQueue(msg);
    }

    if (!alreadyClosed) {
        if (eventHandler)
            eventHandler->ChannelDataReceived(thisPtr); //just notify a read event
        else
            LOGE(channelId, ": Event handler required but not found");
    }

    if (closed)
        session.lock()->deregisterChannel(thisPtr);
}

void
Channel::handleChannelError(ChannelErrorMsgPtr errMsg)
{
    if (closeRcvd || closeSent)
        return;
    if (eventHandler) {
        eventHandler->ChannelError(thisPtr, errMsg->ErrorNo, errMsg->Error);
        eventHandler = nullptr;
    } else {
        LOGE(channelId, ": Event handler required but not found");
    }
}

} // namespace protocol
