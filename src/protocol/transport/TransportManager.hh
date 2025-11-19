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

#ifndef SRC_CPP_PINGGYTRANSPORT_TRANSPORTMANAGER_HH_
#define SRC_CPP_PINGGYTRANSPORT_TRANSPORTMANAGER_HH_

#include "Serialization.hh"
#include "Deserialization.hh"
#include <queue>
#include <platform/SharedPtr.hh>
#include <platform/pinggy_types.h>
#include <utils/RawData.hh>
#include <net/NetworkConnection.hh>

abstract class TransportManagerEventHandler: virtual public pinggy::SharedObject
{
public:
    virtual
    ~TransportManagerEventHandler()
                                { }

    virtual void
    HandleConnectionReset(net::NetworkConnectionPtr netConn) = 0;

    virtual void
    HandleIncomingDeserialize(DeserializerPtr deserializer) = 0;

    virtual void
    HandleIncomingPinggyValue(PinggyValue &)
                                { }

    virtual void
    HandleReadyToSendBuffer() = 0;

    virtual void
    HandleIncompleteHandshake() = 0;
};
DeclareSharedPtr(TransportManagerEventHandler);

class TransportManager : public virtual FDEventHandler
{
private:
    PathRegistryPtr             senderPathRegistry;
    PathRegistryPtr             recverPathRegistry;
    net::NetworkConnectionPtr   sendersNetConn;
    net::NetworkConnectionPtr   recversNetConn;
    TransportManagerEventHandlerPtr
                                eventHandler;

    std::queue<RawDataPtr>      senderQueue;
    bool                        enablePinggyValue;

    bool                        readingHeader;
    RawDataPtr                  recvRawData;
    RawData::tLen               expectedLen;

    bool                        mismatchedEndianness;
    bool                        signatureSent;
    bool                        signatureRcvd;

    bool                        mismatchedEndianSerialize;
    bool                        isServer; //server would not do byte swap ever

    bool                        endTransport;

    void
    sendSignature();

    void
    recvSignature(RawDataPtr rawData);

    void
    sendOrQueueData(RawDataPtr rawData);

    void
    parseHeader(RawDataPtr stream);

    void
    parseData(RawDataPtr stream);

    void
    parsePathDefinition(RawDataPtr stream);

    void
    parseBody(RawDataPtr stream);

    void
    closeConnections();

public:
    TransportManager(net::NetworkConnectionPtr netConn, TransportManagerEventHandlerPtr eventHandler=nullptr,
                        bool isServer = false, bool handshakeRequired=true);
    TransportManager(net::NetworkConnectionPtr sendersNetConn, net::NetworkConnectionPtr recversNetConn,
                        TransportManagerEventHandlerPtr eventHandler=nullptr, bool isServer = false,
                            bool handshakeRequired = true);
    virtual
    ~TransportManager();

    inline void
    EnablePinggyValueMode(bool enable = true)
                                { enablePinggyValue = enable; }

    virtual SerializerPtr
    GetSerializer();

    virtual bool
    SendMsg(SerializerPtr serializer);

    virtual bool
    SendMsg(PinggyValue &v); //not using const because we want take controll of this object

    virtual bool
    EndTransport();

    virtual len_t
    HandleFDRead(PollableFDPtr) override;

    virtual len_t
    HandleFDWrite(PollableFDPtr) override;

    virtual len_t
    HandleFDError(PollableFDPtr, int16_t) override;
};
DefineMakeSharedPtr(TransportManager);


#endif // SRC_CPP_PINGGYTRANSPORT_TRANSPORTMANAGER_HH_

