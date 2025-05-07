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

#ifndef SRC_CPP_PROTOCOL_CHANNEL_HH_
#define SRC_CPP_PROTOCOL_CHANNEL_HH_
#include "Schema.hh"
#include <platform/SharedPtr.hh>

namespace protocol
{

DeclareClassWithSharedPtr(Session);
DeclareClassWithSharedPtr(Channel);

abstract class ChannelEventHandler: virtual public pinggy::SharedObject
{
public:
    virtual
    ~ChannelEventHandler()      { }

    virtual void
    ChannelDataReceived(ChannelPtr) = 0;

    virtual void
    ChannelReadyToSend(ChannelPtr, tUint32) = 0;

    virtual void
    ChannelError(ChannelPtr, tError errorCode, tString errorText) = 0;

    virtual void
    ChannelRejected(ChannelPtr, tString reason) = 0;

    virtual void
    ChannelAccepted(ChannelPtr) = 0;

    virtual void
    ChannelConnected(ChannelPtr) = 0;

    virtual void
    ChannelCleanup(ChannelPtr) = 0;
};
DeclareSharedPtr(ChannelEventHandler);


class Channel: virtual public pinggy::SharedObject
{
public:
    virtual
    ~Channel();

    bool
    Connect();

    bool
    Accept();

    bool
    Reject(tString reason);

    bool
    Close();

    void
    Cleanup(); //Very destructive

    void
    RegisterEventHandler(ChannelEventHandlerPtr ev)
                                { eventHandler = ev; }

    RawData::tLen
    Send(RawDataPtr);

    std::tuple<RawData::tLen, RawDataPtr>
    Recv(RawData::tLen len);

    bool
    HaveDataToRead();

    tUint32
    HaveBufferToWrite();

    bool
    IsConnected()               { return connected; }

    tChannelType
    GetType()                   { return chanType; }

    tUint16
    GetDestPort()               { return destPort; }

    tString
    GetDestHost()               { return destHost; }

    tUint16
    GetSrcPort()                { return srcPort; }

    tString
    GetSrcHost()                { return srcHost; }

private:
    Channel(SessionPtr); //constructor

    void
    sendOrQueue(ProtoMsgPtr msg);

    void
    adjustWindow(tUint32 len);

    void
    handleNewChannelResponse(SetupChannelResponseMsgPtr);

    void
    handleChannelData(ChannelDataMsgPtr);

    void
    handleChannelWindowAdjust(ChannelWindowAdjustMsgPtr);

    void
    handleChannelClose(ChannelCloseMsgPtr);

    void
    handleChannelError(ChannelErrorMsgPtr);

    friend class                Session;

    SessionWPtr                 session;
    tChannelId                  channelId;
    tUint16                     destPort;
    tString                     destHost;
    tUint16                     srcPort;
    tString                     srcHost;
    tChannelType                chanType;

    tUint32                     remoteWindow;
    tUint32                     localWindow;

    tUint32                     remoteMaxPacket;
    tUint32                     localMaxPacket;

    tUint32                     localConsumed;

    bool                        closeSent;
    bool                        closeRcvd;
    bool                        closed;

    bool                        connectionSent;
    bool                        connectionRcvd;
    bool                        connected;
    bool                        rejected;

    // std::queue<ProtoMsgPtr> sendQueue;
    std::queue<RawDataPtr>      recvQueue;

    ChannelEventHandlerPtr      eventHandler;

    friend class                Session;
};
DefineMakeSharedPtr(Channel);

} // namespace protocol

#endif // SRC_CPP_PROTOCOL_CHANNEL_HH_
