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

#ifndef __SRC_CPP_SDK_SDKCHANNELWRAPER_HH__
#define __SRC_CPP_SDK_SDKCHANNELWRAPER_HH__

#include <Channel.hh>

namespace sdk
{

DeclareClassWithSharedPtr(Sdk);

abstract class SdkChannelEventHandler: virtual public pinggy::SharedObject
{
public:
    virtual
    ~SdkChannelEventHandler()   {}

    virtual void
    ChannelDataReceived() = 0;

    virtual void
    ChannelReadyToSend(tUint32) = 0;

    virtual void
    ChannelError(tString errorText) = 0;

    virtual void
    ChannelCleanup() = 0;

private:
    /* data */
};
DefineMakeSharedPtr(SdkChannelEventHandler);

class SdkChannelWraper: virtual public protocol::ChannelEventHandler
{
public:
    SdkChannelWraper(protocol::ChannelPtr, SdkPtr sdk);
    virtual ~SdkChannelWraper() {}

    bool
    Accept();

    bool
    Reject(tString reason);

    bool
    Close();

    RawData::tLen
    Send(RawDataPtr);

    std::tuple<RawData::tLen, RawDataPtr>
    Recv(RawData::tLen len);

    bool
    HaveDataToRead()            { return channel->HaveDataToRead(); }

    tUint32
    HaveBufferToWrite()         { return channel->HaveBufferToWrite(); }

    bool
    IsConnected()               { return channel->IsConnected(); }

    protocol::tChannelType
    GetType()                   { return channel->GetType(); }

    tUint16
    GetDestPort()               { return channel->GetDestPort(); }

    tString
    GetDestHost()               { return channel->GetDestHost(); }

    tUint16
    GetSrcPort()                { return channel->GetSrcPort(); }

    tString
    GetSrcHost()                { return channel->GetSrcHost(); }

    void
    RegisterEventHandler(SdkChannelEventHandlerPtr ev)
                                { eventHandler = ev; }

    SdkChannelEventHandlerPtr
    GetEventHandler()           { return eventHandler; }

    bool
    IsResponeded()              { return responded; }

//ChannelEventHandler
    virtual void
    ChannelDataReceived(protocol::ChannelPtr) override;

    virtual void
    ChannelReadyToSend(protocol::ChannelPtr, tUint32) override;

    virtual void
    ChannelError(protocol::ChannelPtr, protocol::tError errorCode, tString errorText) override;

    virtual void
    ChannelRejected(protocol::ChannelPtr, tString reason) override
                                { }

    virtual void
    ChannelAccepted(protocol::ChannelPtr) override
                                { }

    virtual void
    ChannelCleanup(protocol::ChannelPtr) override;

private:
    protocol::ChannelPtr        channel;
    SdkPtr                      sdk;
    SdkChannelEventHandlerPtr   eventHandler;
    bool                        responded;
};
DefineMakeSharedPtr(SdkChannelWraper);

} // namespace sdk



#endif // __SRC_CPP_SDK_SDKCHANNELWRAPER_HH__
