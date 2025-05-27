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

#include "TransportManager.hh"
#include "Serialization.hh"
#include "Deserialization.hh"
#include <platform/assert_pinggy.h>

#define HANDSHAKE_LENGTH 256

#define TRANSPORT_HEADER_LENGTH 2

#define HANDSHAKE_SIGNATURE        \
"PINGGY                          " \
"                                " \
" ###  # #    #  ###   ### #   # " \
" #  # # ##   # #     #     # #  " \
" ###  # # #  # #  ## #  ##  #   " \
" #    # #  # # #   # #   #  #   " \
" #    # #   ##  ###   ###   #   " \
"                                " \


TransportManager::TransportManager(net::NetworkConnectionPtr netConn, TransportManagerEventHandlerPtr eventHandler,
                                    bool isServer, bool handshakeRequired):
            senderPathRegistry(new PathRegistry()),
            recverPathRegistry(new PathRegistry()),
            sendersNetConn(netConn),
            recversNetConn(netConn),
            eventHandler(eventHandler),
            readingHeader(true),
            expectedLen(HANDSHAKE_LENGTH),
            mismatchedEndianness(false),
            signatureSent(false),
            signatureRcvd(false),
            mismatchedEndianSerialize(false),
            isServer(isServer),
            endTransport(false)
{
    if (!handshakeRequired) {
        signatureRcvd = true;
        signatureSent = true;
        mismatchedEndianness = false;
        mismatchedEndianSerialize = false;
        expectedLen = TRANSPORT_HEADER_LENGTH;
    }
}

TransportManager::TransportManager(net::NetworkConnectionPtr sendersNetConn, net::NetworkConnectionPtr recversNetConn,
                                    TransportManagerEventHandlerPtr eventHandler, bool isServer, bool handshakeRequired):
            senderPathRegistry(new PathRegistry()),
            recverPathRegistry(new PathRegistry()),
            sendersNetConn(sendersNetConn),
            recversNetConn(recversNetConn),
            eventHandler(eventHandler),
            readingHeader(true),
            expectedLen(HANDSHAKE_LENGTH),
            mismatchedEndianness(false),
            signatureSent(false),
            signatureRcvd(false),
            mismatchedEndianSerialize(false),
            isServer(isServer),
            endTransport(false)
{
    if (!handshakeRequired) {
        signatureRcvd = true;
        signatureSent = true;
        mismatchedEndianness = false;
        mismatchedEndianSerialize = false;
        expectedLen = TRANSPORT_HEADER_LENGTH;
    }
}

TransportManager::~TransportManager()
{
}

void
TransportManager::sendSignature()
{
    Assert(signatureSent == false);
    auto stream = NewRawDataPtr();
    tString sign = HANDSHAKE_SIGNATURE;
    stream->AddData(sign);
    auto data = stream->GetData();
    data[HANDSHAKE_LENGTH-1] = (tUint8)IS_BIG_ENDIAN;
    data[HANDSHAKE_LENGTH-2] = (tUint8)isServer;
    sendOrQueueData(stream->Slice(0,HANDSHAKE_LENGTH));
    signatureSent = true;
}

void
TransportManager::recvSignature(RawDataPtr rawData)
{
    if (rawData->Len < HANDSHAKE_LENGTH) {
        if(eventHandler) {
            eventHandler->HandleIncompleteHandshake();
        } else {
            throw std::runtime_error("Handshake not complete");
        }
    }
    bool isRemoteBigEndian = rawData->GetData()[HANDSHAKE_LENGTH-1];
    bool isRemoteServer = rawData->GetData()[HANDSHAKE_LENGTH-2];
    if (isServer && isRemoteServer) {
        ABORT_WITH_MSG("Remote and local both cannot be server")
    }
    if (!isServer) //if local is server, the deserilize is not provision to byteswap
        mismatchedEndianness = (isRemoteBigEndian != IS_BIG_ENDIAN);
    if (isRemoteServer) //if remote is a server we serialize with byteswap
        mismatchedEndianSerialize = (isRemoteBigEndian != IS_BIG_ENDIAN);

    if (memcmp(rawData->GetData(), (char *)HANDSHAKE_SIGNATURE, HANDSHAKE_LENGTH-2)) {
        if(eventHandler) {
            eventHandler->HandleIncompleteHandshake();
        } else {
            throw std::runtime_error("Handshake not complete");
        }
    }
    expectedLen = TRANSPORT_HEADER_LENGTH;
    signatureRcvd = true;
}

void
TransportManager::sendOrQueueData(RawDataPtr rawData)
{
    if (!senderQueue.empty()) {
        senderQueue.push(rawData);
        return;
    }

    auto sent = sendersNetConn->Write(rawData);
    if (sent <= 0) {
        if(sendersNetConn->TryAgain()) {
            senderQueue.push(rawData);
            sendersNetConn->EnableWritePoll();
            return;
        }
        if (eventHandler) {
            eventHandler->HandleConnectionReset(sendersNetConn);
        } else {
            throw net::NetworkConnectionException("Connection Closed", sendersNetConn);
        }
        return;
    }
    rawData->Consume(sent);
    if(rawData->Len) {
        senderQueue.push(rawData);
        sendersNetConn->EnableWritePoll();
    }
}

void
TransportManager::parseHeader(RawDataPtr stream)
{
    Assert(stream->Len == TRANSPORT_HEADER_LENGTH);
    uint16_t dataLen;
    Deserialize_Lit(stream, dataLen, mismatchedEndianness);
    expectedLen = dataLen;
    readingHeader = false;
}

void
TransportManager::parseData(RawDataPtr stream)
{
    uint8_t msgType;
    Deserialize_Lit(stream, msgType, mismatchedEndianness);
    if (msgType == MsgType_Type) {
        LOGT("Parsing PathDef");
        parsePathDefinition(stream);
    } else if (msgType == MsgType_Value) {
        LOGT("Parsing stream");
        parseBody(stream);
    }
    readingHeader = true;
    expectedLen = TRANSPORT_HEADER_LENGTH;
}

void
TransportManager::parsePathDefinition(RawDataPtr stream)
{
    recverPathRegistry->UpdatePathRegistryFromStream(stream, mismatchedEndianness);
}

void
TransportManager::parseBody(RawDataPtr stream)
{
    auto newPathRegistry = recverPathRegistry;
    recverPathRegistry->dirty = false;
    auto deserializer = NEW_DESERIALIZE_PTR(mismatchedEndianness);
    deserializer->Parse(stream, newPathRegistry);
    if(!eventHandler)
        return;
    eventHandler->HandleIncomingDeserialize(deserializer);
}

void TransportManager::closeConnections()
{
    sendersNetConn->DeregisterFDEvenHandler();
    recversNetConn->DeregisterFDEvenHandler();
    sendersNetConn->CloseConn();
    recversNetConn->CloseConn();
}

SerializerPtr TransportManager::GetSerializer()
{
    if (!signatureSent) {
        sendSignature();
    }
    auto pathRegistry = senderPathRegistry;
    auto stream = NewRawDataPtr();
    Serialize_Lit(stream, (uint8_t)MsgType_Value, mismatchedEndianSerialize);
    //TODO we need to cach some serializer. creating them again and again is not sustainable
    return NEW_SERIALIZE_PTR(pathRegistry, mismatchedEndianSerialize, stream, ROOT_PATH_ID, thisPtr);
}

bool
TransportManager::SendMsg(SerializerPtr serializer)
{
    if (!senderQueue.empty())
        return false;
    auto msgHeader = NewRawDataPtr();
    auto header = senderPathRegistry->GetNClearNewlyAddedPath(mismatchedEndianness);
    if (header && header->Len) {
        Serialize_Lit(msgHeader, (uint16_t)header->Len, mismatchedEndianSerialize);
        sendOrQueueData(msgHeader);
        sendOrQueueData(header);
        msgHeader = NewRawDataPtr();
    }

    auto rawBody = serializer->GetStream();
    Serialize_Lit(msgHeader, (uint16_t)rawBody->Len, mismatchedEndianSerialize);
    sendOrQueueData(msgHeader);
    sendOrQueueData(rawBody);
    return true;
}

bool
TransportManager::EndTransport()
{
    if (endTransport)
        return true;
    endTransport = true;
    if (senderQueue.empty()) {
        closeConnections();
    }
    return true;
}

len_t
TransportManager::HandleFDRead(PollableFDPtr)
{
    auto tobeLen = expectedLen;
    if (recvRawData) {
        tobeLen -= recvRawData->Len;
    }
    if (tobeLen <= 0) {
        ABORT_WITH_MSG("cannot read zero byte or less");
    }

    auto [len, newlyRecvedData] = recversNetConn->Read(tobeLen);
    if (len<=0) {
        if (recversNetConn->TryAgain()) {
            return -1;
        }
        if(eventHandler) {
            eventHandler->HandleConnectionReset(recversNetConn);
            return len;
        } else {
            ABORT_WITH_MSG("Connection reset, but no handler found");
        }
    }

    if (recvRawData && recvRawData->Len) {
        recvRawData = recvRawData->Concat(newlyRecvedData);
    } else {
        recvRawData = newlyRecvedData;
    }

    if (recvRawData->Len < expectedLen) {
        return len;
    }

    auto parsableData = recvRawData->Slice(0, expectedLen);
    recvRawData = recvRawData->Slice(expectedLen);

    if (recvRawData->Len == 0) {
        recvRawData = nullptr;
    }

    if (!signatureRcvd) {
        recvSignature(parsableData);
        return len;
    }

    if (readingHeader) {
        parseHeader(parsableData);
    } else {
        parseData(parsableData);
    }

    return len;
}

len_t
TransportManager::HandleFDWrite(PollableFDPtr)
{
    if (senderQueue.empty()) {
        sendersNetConn->DisableWritePoll();
        if(eventHandler)
            eventHandler->HandleReadyToSendBuffer();
        return 0;
    }
    auto rawData = senderQueue.front();
    Assert(rawData->Len);
    auto sent = sendersNetConn->Write(rawData);
    if (sent <= 0) {
        if(sendersNetConn->TryAgain()) {
            return -1; //this is not supposed happened
        }

        if (eventHandler) {
            eventHandler->HandleConnectionReset(sendersNetConn);
        } else {
            ABORT_WITH_MSG("Connection reset, but no handler found");
        }
        return sent;
    }
    rawData->Consume(sent);
    if(rawData->Len == 0) {
        senderQueue.pop();
    }
    if (senderQueue.empty()) {
        if (endTransport) {
            closeConnections();
            return -1;
        }
    }
    return sent;
}

len_t
TransportManager::HandleFDError(PollableFDPtr, int16_t)
{
    return 0;
}

