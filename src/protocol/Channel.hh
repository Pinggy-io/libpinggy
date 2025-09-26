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
#include "SessionFeatures.hh"

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
    IsConnected()               { return state == ChannelState_Connected; }

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

    TunnelMode
    GetMode()                   { return mode; }

    tForwardingId
    GetForwardingId()           { return forwardingId; }

    void
    SetUserTag(tString tag)     { userTag = tag; }

    tString
    GetUserTag()                { return userTag; }

    void
    SetUserPtr(tVoidPtr ptr)    { userPtr = ptr; }

    tVoidPtr
    GetUserPtr()                { return userPtr; }

    template<typename T>
    std::shared_ptr<T>
    GetUserPtr()                { return userPtr ? userPtr->DynamicPointerCast<T>(): nullptr; }

private:
    Channel(SessionPtr, SessionFeaturesPtr); //constructor

    void
    cleanup(); //Very destructive

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

    void
    setChannelInfo(tUint16 destPort, tString destHost, tUint16 srcPort, tString srcHost,
                    tChannelType chanType, TunnelMode mode, tForwardingId forwardingId);

    void
    initiateIncomingChannel(SetupChannelMsgPtr msg);

    void
    closeTimeoutTriggered();

    friend class                Session;

    enum ChannelState {
        ChannelState_Init = 0,
        ChannelState_Connecting,
        ChannelState_Connect_Responding,
        ChannelState_Connected,
        ChannelState_Closing,
        ChannelState_Close_Responding,
        ChannelState_Closed,
        ChannelState_Rejected
    };

    SessionWPtr                 session;
    tChannelId                  channelId;
    tUint16                     destPort;
    tString                     destHost;
    tUint16                     srcPort;
    tString                     srcHost;
    TunnelMode                  mode;
    tForwardingId               forwardingId;
    tChannelType                chanType;

    tUint32                     remoteWindow;
    tUint32                     localWindow;

    tUint32                     remoteMaxPacket;
    tUint32                     localMaxPacket;

    tUint32                     localConsumed;
    ChannelState                state;
    bool                        allowWrite;

    std::queue<RawDataPtr>      recvQueue;

    ChannelEventHandlerPtr      eventHandler;

    tString                     userTag;
    tVoidPtr                    userPtr;
    SessionFeaturesPtr          features;

    friend class                Session;
};
DefineMakeSharedPtr(Channel);

} // namespace protocol

#endif // SRC_CPP_PROTOCOL_CHANNEL_HH_
