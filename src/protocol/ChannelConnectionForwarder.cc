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

#include "ChannelConnectionForwarder.hh"

namespace protocol
{


ChannelConnectionForwarder::ChannelConnectionForwarder(ChannelPtr channel, net::NetworkConnectionPtr netConn, DataTransferCounterPtr counter):
                    channel(channel), netConn(netConn), counter(counter),
                    allowCopyFromChannel(false), allowCopyFromNetConn(false),
                    fdRecvEnabled(false), fdSendEnabled(false)
{
}

void
ChannelConnectionForwarder::Start()
{
    netConn->RegisterFDEvenHandler(thisPtr);
    channel->RegisterEventHandler(thisPtr);
    if (!channel->IsConnected()) { //It is not yet accepted
        netConn->DisableReadPoll();
        netConn->DisableWritePoll();
        channel->Connect();
    } else {
        allowCopyFromNetConn = true;
        allowCopyFromChannel = true;
        fdRecvEnabled = true;
    }
}

void
ChannelConnectionForwarder::EnableCopyFromChannel() //playres
{
    if (allowCopyFromChannel) return;
    if (!fdSendEnabled) {
        netConn->EnableWritePoll();
        fdSendEnabled = true;
    }
    allowCopyFromChannel = true;
}

void
ChannelConnectionForwarder::DisableCopyFromChannel() //pauseres
{
    if (!allowCopyFromChannel) return;
    if (fdSendEnabled) {
        netConn->DisableWritePoll();
        fdSendEnabled = false;
    }
    allowCopyFromChannel = false;
}

void
ChannelConnectionForwarder::EnableCopyFromNetConn() //playreq
{
    LOGTV("Enabling copy from netConn", netConn, allowCopyFromNetConn);
    if(allowCopyFromNetConn) return;
    if (!fdRecvEnabled) {
        netConn->EnableReadPoll();
        fdRecvEnabled = true;
    }
    allowCopyFromNetConn = true; //what if there are data in dataForChannel
    if (!dataForChannel || !dataForChannel->Len) {
        netConn->RaiseDummyReadPoll();
    }
}

void
ChannelConnectionForwarder::DisableCopyFromNetConn() //pauseres
{
    LOGTV("Disabling copy from netConn", netConn->GetFd());
    if(!allowCopyFromNetConn) return;
    if (fdRecvEnabled) {
        netConn->DisableReadPoll();
        fdRecvEnabled = false;
    }
    allowCopyFromNetConn = false;
}

//==FDEventHandler
len_t
ChannelConnectionForwarder::HandleFDRead(PollableFDPtr)
{
    LOGT("Called `" + std::string(__func__) + "`");
    if (!dataForChannel || !dataForChannel->Len) {
        auto [_1, _2] = netConn->Read(16*KB);
        auto len = _1;
        dataForChannel = _2;
        if (len <= 0) {
            if (netConn->TryAgain()) {
                // netConn->DisableReadPoll();
                LOGT("Called `" + std::string(__func__) + "` false alert try again", netConn->TryAgain(), app_get_errno(), len);
                // fdRecvEnabled = false;
                return len;
            }
            LOGT("Called `" + std::string(__func__) + "` close by netconn");
            closeByNetConn();
            return len;
        }
    }
    auto availableBuffer = channel->HaveBufferToWrite();
    if ((RawData::tLen)availableBuffer < dataForChannel->Len) {
        netConn->DisableReadPoll();
        LOGT("Called `" + std::string(__func__) + "` Disabling ", availableBuffer, dataForChannel->Len);
        fdRecvEnabled = false;
        return -1;
    }
    auto sent = channel->Send(dataForChannel);
    if (sent == 0) { //TODO impossible statements
        LOGT("Called `" + std::string(__func__) + "` close by netconn");
        closeByNetConn();
        return sent;
    }
    if (sent < 0) {
        netConn->DisableReadPoll();
        LOGT("Called `" + std::string(__func__) + "` Disabling");
        fdRecvEnabled = false;
        return sent;
    }
    dataForChannel->Consume(sent);
    if (dataForChannel->Len == 0)
        dataForChannel = nullptr;

    if (counter) {
        counter->CounterByteCopiedToChannel(thisPtr, sent);
    }
    return sent;
}

len_t
ChannelConnectionForwarder::HandleFDWrite(PollableFDPtr)
{
    LOGT("Called `" + std::string(__func__) + "`");
    if (!dataForNetConn || !dataForNetConn->Len) {
        auto [_1, _2] = channel->Recv(16*KB);
        auto len = _1;
        dataForNetConn = _2;
        if (len == 0) {
            closeByNetConn();
            LOGT("Called `" + std::string(__func__) + "` closeByNetConn");
            return len;
        }
        if (len < 0) {
            LOGT("Called `" + std::string(__func__) + "` Disabling", netConn->TryAgain(), app_get_errno(), len);
            netConn->DisableWritePoll();
            fdSendEnabled = false;
            return len;
        }
    }

    auto len = netConn->Write(dataForNetConn);
    if (len <= 0) {
        if (netConn->TryAgain()) {
            LOGT("Called `" + std::string(__func__) + "` false alert try again");
            return len;
        }
        closeByNetConn();
        LOGT("Called `" + std::string(__func__) + "` Close by netconn");
        return len;
    }

    dataForNetConn->Consume(len);
    if (dataForNetConn->Len == 0)
        dataForNetConn = nullptr;

    if (counter) {
        counter->CounterByteCopiedToConnection(thisPtr, len);
    }

    return len;
}

len_t
ChannelConnectionForwarder::HandleFDError(PollableFDPtr, int16_t)
{
    // Assert(false);
    closeByNetConn(); //causes segmentation fault
    return 0;
}

//==ChannelEventHandler
void ChannelConnectionForwarder::ChannelDataReceived(ChannelPtr)
{
    if (fdSendEnabled)
        return;
    if (!allowCopyFromChannel)
        return;

    //we should copy some amount from here.
    netConn->EnableWritePoll();
    fdSendEnabled = true;
}

void
ChannelConnectionForwarder::ChannelReadyToSend(ChannelPtr, tUint32 availableBuffer)
{
    if (fdRecvEnabled)
        return;
    if (!allowCopyFromNetConn) {
        LOGT("Called `" + std::string(__func__) + "` Not allowed to copy");
        return;
    }

    if (!dataForChannel || !dataForChannel->Len || dataForChannel->Len > (RawData::tLen)availableBuffer) {
        LOGT("Called `" + std::string(__func__) + "` Not enoungh buffer");
        return; //it is meaningless to enable
    }

    LOGT("Called `" + std::string(__func__) + "` Enabling ", availableBuffer, dataForChannel->Len);
    //We can copy here. However, I want to perform copy only once in the code for consistency
    netConn->EnableReadPoll();
    if (dataForChannel->Len) {
        netConn->RaiseDummyReadPoll();
    }
    fdRecvEnabled = true;
}

void
ChannelConnectionForwarder::ChannelError(ChannelPtr, tError errorCode, tString errorText)
{
    LOGE("Error occured with channel", errorText);
}

void
ChannelConnectionForwarder::ChannelRejected(ChannelPtr, tString reason)
{
    LOGE("Channel rejected by remote. Silently closing");
    netConn->DeregisterFDEvenHandler();
    netConn->CloseConn();
    netConn = nullptr;
    channel = nullptr;
    if (counter) {
        counter->CounterChannelRejected(thisPtr, reason);
    }
}

void
ChannelConnectionForwarder::ChannelAccepted(ChannelPtr)
{
    if (!channel->IsConnected()) {
        ABORT_WITH_MSG("Only connected channel can be accepted.")
        return; //this weird
    }

    netConn->EnableReadPoll();
    allowCopyFromNetConn = true;
    allowCopyFromChannel = true;
    fdRecvEnabled = true;
    if (counter) {
        counter->CounterChannelConnected(thisPtr);
    }
}

void
ChannelConnectionForwarder::ChannelCleanup(ChannelPtr)
{
    if (counter) {
        counter->CounterChannelClosed(thisPtr);
        counter = nullptr;
    }

    if (netConn) {
        netConn->DeregisterFDEvenHandler();
        netConn->CloseConn();
        netConn = nullptr;
    }

    if (channel) {
        channel = nullptr;
    }
}

void ChannelConnectionForwarder::closeByNetConn()
{
    if (counter) {
        counter->CounterChannelClosed(thisPtr);
        counter = nullptr;
    }

    if (netConn) {
        netConn->DeregisterFDEvenHandler();
        netConn->CloseConn();
        netConn = nullptr;
    }

    if (channel) {
        channel->Close();
        channel = nullptr;
    }
}

} // namespace protocol
