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

#ifndef SRC_CPP_PROTOCOL_PROTOCOL_HH_
#define SRC_CPP_PROTOCOL_PROTOCOL_HH_

#include <platform/pinggy_types.h>
#include <net/NetworkConnection.hh>
#include "transport/TransportManager.hh"
#include "Channel.hh"
#include "Schema.hh"
#include <queue>

namespace protocol
{

enum tSessionState {
    SessionState_Init,
    SessionState_ClientHelloSent,
    SessionState_ServerHelloSent,
    SessionState_AuthenticationRequestSent,
    SessionState_AuthenticationRequestRecved,
    SessionState_AuthenticatedAsClient,
    SessionState_AuthenticatedAsServer,
    SessionState_AuthenticationFailed,
    SessionState_Failed
};

abstract class SessionEventHandler: virtual public pinggy::SharedObject
{
public:
    virtual
    ~SessionEventHandler()      { }

    virtual void
    HandleSessionInitiated()    { }

    virtual bool
    HandleSessionAuthenticationRequest(tString username, tString arguments, bool advancedParsing)
                                { return false; }

    virtual void
    HandleSessionAuthenticatedAsClient(std::vector<tString> messages)
                                { }

    virtual void
    HandleSessionAuthenticationFailed(tString error, std::vector<tString> authenticationFailed)
                                { ABORT_WITH_MSG("Not implemented"); }

    virtual void
    HandleSessionRemoteForwardRequest(tReqId reqId, tInt16 listeningPort, tString listeningHost, tInt16 forwardingPort, tString forwardingHost)
                                { ABORT_WITH_MSG("Not implemented"); }

    virtual void
    HandleSessionRemoteForwardingSucceeded(tReqId, std::vector<tString>)
                                { ABORT_WITH_MSG("Not implemented"); }

    virtual void
    HandleSessionRemoteForwardingFailed(tReqId, tString)
                                { ABORT_WITH_MSG("Not implemented"); }

    virtual void
    HandleSessionNewChannelRequest(ChannelPtr channel)
                                { ABORT_WITH_MSG("Not implemented"); }

    virtual void
    HandleSessionError(tUint32 errorNo, tString what, tBool recoverable)
                                { ABORT_WITH_MSG("Not implemented"); }

    virtual void
    HandleSessionKeepAliveResponseReceived(tUint64 forTick)
                                { LOGT("KeepAliveResponse received for tick"); }

    virtual void
    HandleSessionDisconnection(tString reason)
                                { ABORT_WITH_MSG("Not supposed to handle it"); }

    virtual void
    HandleSessionConnectionReset()
                                { ABORT_WITH_MSG("Not implemented"); }
};
DeclareSharedPtr(SessionEventHandler);

class Session: virtual public TransportManagerEventHandler
{
public:
    Session(net::NetworkConnectionPtr netConn, bool asServer=false);

    virtual
    ~Session()                  { }

    virtual void
    Start(SessionEventHandlerPtr eventHandler);

    virtual void
    End(tString reason); //Formal disconnection. It would wait till all the packets are sent

    virtual void
    Cleanup(); //Informal. It would cleanup everything without triggering any connection event

    virtual void
    AuthenticateAsClient(tString user, tString arguments, bool advanceParsing = true);

    virtual tReqId
    SendRemoteForwardRequest(tInt16 listeningPort, tString listeningHost, tInt16 forwardingPort, tString forwardingHost);

    virtual void
    AuthenticationSucceeded(std::vector<tString> messages);

    virtual void
    AuthenticationFailed(tString error, std::vector<tString> messages);

    virtual void
    AcceptRemoteForwardRequest(tReqId reqId, std::vector<tString> urls);

    virtual void
    RejectRemoteForwardRequest(tReqId reqId, tString error);

    virtual tUint64
    SendKeepAlive();

    virtual ChannelPtr
    CreateChannel(tUint16 destPort, tString destHost,
                    tUint16 srcPort, tString srcHost, tChannelType chanType = ChannelType_Stream);

    virtual tString
    GetMessage()                { return endReason; }

// TransportManagerEventHandler
    virtual void
    HandleConnectionReset(net::NetworkConnectionPtr netConn) override;

    virtual void
    HandleIncomingDeserialize(DeserializerPtr deserializer) override;

    virtual void
    HandleIncompleteHandshake() override;

    virtual void
    HandleReadyToSendBuffer() override;

private:
    uint16_t
    getChannelNewId();

    bool
    chanIdExists(tChannelId chId);

    bool
    validRemoteChannel(tChannelId channelId);

    bool
    sendMsg(ProtoMsgPtr, bool queue = true);

    void
    sendErrorMsg(tUint32 errorNo, tString what, bool recoverable=false);

    void
    deregisterChannel(ChannelPtr channel);

    void
    registerChannel(ChannelPtr channel);

    void
    handleRemoteForwardResponse(tReqId reqId, tUint8 success, std::vector<tString> urls, tString error);

    void
    handleNewChannel(SetupChannelMsgPtr newChannelMsg);

    friend class                Channel;

    net::NetworkConnectionPtr   netConn;
    TransportManagerPtr         transportManager;
    bool                        serverMode;
    tSessionState               state;
    SessionEventHandlerPtr      eventHandler;

    std::map<tChannelId, ChannelPtr>
                                channels;

    tReqId                      lastReqId;
    tChannelId                  lastChannelId;
    std::queue<ProtoMsgPtr>     sendQueue;
    bool                        endSent;
    tString                     endReason;
    tUint64                     keepAliveSentTick;
};
DefineMakeSharedPtr(Session);


} // namespace protocol

#endif // SRC_CPP_PROTOCOL_PROTOCOL_HH_
