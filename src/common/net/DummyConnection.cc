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


#include "DummyConnection.hh"
#include <utils/Utils.hh>
#include <queue>
#include <openssl/ssl.h>
//#include

namespace net {
struct DummyMetaData : public virtual pinggy::SharedObject {
    DummyMetaData(): closed(false) {}
    virtual ~DummyMetaData(){}
    std::queue<RawDataPtr> queue;
    bool closed;
};
DefineMakeSharedPtr(DummyMetaData);

#define MAX_BUFFER_CNT 10

bool
DummyConnection::CreateDummyConnection(DummyConnectionPtr conns[2], int bufferLen)
{
    DummyConnectionPtr dc1(new DummyConnection());
    DummyConnectionPtr dc2(new DummyConnection());

    dc1->writer = NewDummyMetaDataPtr();
    dc2->writer = NewDummyMetaDataPtr();

    if (bufferLen == 0)
        bufferLen = MAX_BUFFER_CNT;

    dc1->reader = dc2->writer;
    dc2->reader = dc1->writer;
    dc1->bufferLen = bufferLen;
    dc2->bufferLen = bufferLen;
    dc1->counterPart = dc2;
    dc2->counterPart = dc1;
    conns[0] = dc1;
    conns[1] = dc2;
    return true;
}

DummyConnection::DummyConnection(): flags(0), lastRet(0), tryAgain(false)
{
    netState.Valid = true;
}

void
DummyConnection::setReadPoll()
{
    if (isReadPolling && IsRecvReady()) {
        RaiseDummyReadPoll();
    }
}

void
DummyConnection::setWritePoll()
{
    if (isWritePolling && IsSendReady()) {
        RaiseDummyWritePoll();
    }
}

void
DummyConnection::setReadPollForCounterPart()
{
    if (!counterPart.expired()) {
        counterPart.lock()->setReadPoll();
    }
}

void
DummyConnection::setWritePollForCounterPart()
{
    if (!counterPart.expired()) {
        counterPart.lock()->setWritePoll();
    }
}

DummyConnection::~DummyConnection()
{
    writer->closed = true;
    reader->closed = true;
    LOGT("Ending DummyConnection:", this)
}

int
DummyConnection::SslError(int ret)
{
    if(ret < 0)
        return SSL_ERROR_SYSCALL;
    return SSL_ERROR_NONE;
}

ssize_t
DummyConnection::Write(RawDataPtr rwData, int flags)
{
    tryAgain = false;
    if (writer->closed) {
        setWritePoll();
        return 0;
    }

    if (rwData->Len > 4096) {
        setWritePoll();
        app_set_errno(EMSGSIZE);
        return -1;
    }

    //use can use some technique here or at the reader side to merge data from two packets
    //if it is small.
    if (writer->queue.size() >= bufferLen || rwData->Len <= 0) {
        tryAgain = true;
        app_set_eagain();
        return -1;
    }

    writer->queue.push(rwData->Slice(0));
    setWritePoll();
    setReadPollForCounterPart();
    return rwData->Len;
}

ssize_t
DummyConnection::Write(const void *buf, size_t nbyte, int flags)
{
    if (!buf || !nbyte) {
        tryAgain = true;
        app_set_eagain();
        return -1;
    }

    auto r = NewRawDataPtr(buf, nbyte);

    return Write(r, flags);
}

std::tuple<ssize_t, RawDataPtr>
DummyConnection::Read(len_t nbyte, int flags)
{
    tryAgain = false;
    if (reader->queue.size() == 0) {
        if (reader->closed)
            return {0, nullptr};
        app_set_eagain();
        tryAgain = true;
        LOGT("Return EAGAIN");
        return {-1, nullptr};
    }

    auto x = reader->queue.front();
    auto ret = x->Slice(0, nbyte);
    x->Consume(nbyte);
    if (x->Len == 0)
        reader->queue.pop();
    LOGT(ret->Len, ret->Data);
    setReadPoll();
    setWritePollForCounterPart();
    return {ret->Len, ret};
}

ssize_t
DummyConnection::Read(void *buf, size_t nbyte, int flags)
{
    if (!buf || !nbyte) {
        tryAgain = true;
        app_set_eagain();
        return -1;
    }

    auto [size, rawData] = Read((len_t)nbyte, flags);
    if (size <= 0) {
        return size;
    }

    std::memcpy(buf, rawData->GetData(), rawData->Len);
    return rawData->Len;
}

std::tuple<ssize_t, RawDataPtr>
DummyConnection::Peek(len_t nbyte)
{
    tryAgain = false;
    if (reader->queue.size() == 0) {
        if (reader->closed)
            return {0, nullptr};
        app_set_eagain();
        tryAgain = true;
        LOGT("Return EAGAIN");
        return {-1, nullptr};
    }

    auto x = reader->queue.front();
    auto ret = x->Slice(0, nbyte);
    LOGT(ret->Len, ret->Data);
    setReadPoll();
    return {ret->Len, ret};
}

ssize_t
DummyConnection::Peek(void *buf, size_t nbyte)
{
    if (!buf || !nbyte) {
        tryAgain = true;
        app_set_eagain();
        return -1;
    }

    auto [size, rawData] = Peek((len_t)nbyte);
    if (size <= 0) {
        return size;
    }

    std::memcpy(buf, rawData->GetData(), rawData->Len);
    return rawData->Len;
}

int
DummyConnection::CloseNClear(tString location)
{
    reader->closed = true;
    writer->closed = true;
    lastRet = 0;
    setReadPoll();
    setWritePoll();
    setReadPollForCounterPart();
    setWritePollForCounterPart();
    LOGD(this, location, "Closing");
    return 0;
}

int
DummyConnection::ShutDown(int how)
{
    if (how == SHUT_RD || how == SHUT_RDWR){
        reader->closed = true;
        setReadPoll();
        setWritePollForCounterPart();
    }
    if (how == SHUT_WR || how == SHUT_RDWR) {
        writer->closed = true;
        setWritePoll();
        setReadPollForCounterPart();
    }
    lastRet = 0;
    return 0;
}

bool
DummyConnection::IsSendReady()
{
    return writer->closed || (writer->queue.size() < bufferLen);
}

bool
DummyConnection::IsRecvReady()
{
    return (reader->closed || reader->queue.size());
}

void
DummyConnection::EnableWritePoll()
{
    if (!isWritePolling) {
        NetworkConnection::EnableWritePoll();
        isWritePolling = true;
        if (IsSendReady())
            RaiseDummyWritePoll();
    }
}

void
DummyConnection::EnableReadPoll()
{
    if (!isReadPolling) {
        NetworkConnection::EnableReadPoll();
        isReadPolling = true;
        if (IsRecvReady())
            RaiseDummyReadPoll();
    }
}

void
DummyConnection::DisableReadPoll()
{
    if (isReadPolling) {
        NetworkConnection::DisableReadPoll();
        isReadPolling = false;
    }
}

void
DummyConnection::DisableWritePoll()
{
    if (isWritePolling) {
        NetworkConnection::DisableWritePoll();
        isWritePolling = false;
    }
}

int16_t
DummyConnection::GetBufferSize()
{
    return (int16_t)reader->queue.size();
}

void
DummyConnection::EventHandlerRegistered()
{
    isReadPolling = true;
    isWritePolling = false;
    if (IsRecvReady())
        RaiseDummyReadPoll();
}

} /* namespace net */
