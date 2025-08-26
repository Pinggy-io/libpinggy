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

#include "SdkChannelWraper.hh"
#include "Sdk.hh"

namespace sdk
{

SdkChannelWraper::SdkChannelWraper(protocol::ChannelPtr channel, SdkPtr sdk):
            channel(channel),
            sdk(sdk),
            responded(false)
{
}

bool
SdkChannelWraper::Accept()
{
    responded = true;
    auto var = sdk->LockIfDifferentThread();
    return channel->Accept();
}

bool
SdkChannelWraper::Reject(tString reason)
{
    responded = true;
    auto var = sdk->LockIfDifferentThread();
    return channel->Reject(reason);
}

bool
SdkChannelWraper::Connect()
{
    responded = true;
    auto var = sdk->LockIfDifferentThread();
    return channel->Connect();
}

bool
SdkChannelWraper::Close()
{
    auto var = sdk->LockIfDifferentThread();
    return channel->Close();
}

RawData::tLen
SdkChannelWraper::Send(RawDataPtr data)
{
    auto var = sdk->LockIfDifferentThread();
    return channel->Send(data);
}

std::tuple<RawData::tLen, RawDataPtr>
SdkChannelWraper::Recv(RawData::tLen len)
{
    auto var = sdk->LockIfDifferentThread();
    return channel->Recv(len);
}

void
SdkChannelWraper::ChannelDataReceived(protocol::ChannelPtr)
{
    if (eventHandler)
        eventHandler->ChannelDataReceived(thisPtr);
}

void
SdkChannelWraper::ChannelReadyToSend(protocol::ChannelPtr, tUint32 available)
{
    if (eventHandler)
        eventHandler->ChannelReadyToSend(thisPtr, available);
}

void
SdkChannelWraper::ChannelError(protocol::ChannelPtr, protocol::tError errorCode, tString errorText)
{
    if (eventHandler)
        eventHandler->ChannelError(thisPtr, errorText);
}

void
SdkChannelWraper::ChannelRejected(protocol::ChannelPtr, tString reason)
{
    if (eventHandler)
        eventHandler->ChannelError(thisPtr, reason);
}

void SdkChannelWraper::ChannelCleanup(protocol::ChannelPtr)
{
    if (eventHandler)
        eventHandler->ChannelCleanup(thisPtr);
    channel = nullptr;
    sdk = nullptr;
}

} // namespace sdk
