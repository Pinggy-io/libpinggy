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

#ifndef SRC_CPP_PROTOCOL_CHANNELCONNECTIONFORWARDING_HH_
#define SRC_CPP_PROTOCOL_CHANNELCONNECTIONFORWARDING_HH_

#include <platform/pinggy_types.h>
#include <poll/PollableFD.hh>
#include "Channel.hh"

namespace protocol
{

DeclareClassWithSharedPtr(ChannelConnectionForwarder);

abstract class DataTransferCounter: virtual public pinggy::SharedObject
{
public:
    virtual
    ~DataTransferCounter()      {}

    virtual void
    CounterByteCopiedToChannel(ChannelConnectionForwarderPtr, RawData::tLen) = 0;

    virtual void
    CounterByteCopiedToConnection(ChannelConnectionForwarderPtr, RawData::tLen) = 0;

    virtual void
    CounterChannelConnected(ChannelConnectionForwarderPtr) = 0;

    virtual void
    CounterChannelClosed(ChannelConnectionForwarderPtr) = 0;

    virtual void
    CounterChannelRejected(ChannelConnectionForwarderPtr, tString reason) = 0;
};
DeclareSharedPtr(DataTransferCounter);

class ChannelConnectionForwarder:
            virtual public FDEventHandler,
            virtual public ChannelEventHandler
{
public:
    ChannelConnectionForwarder(ChannelPtr channel, net::NetworkConnectionPtr netConn, DataTransferCounterPtr counter);

    virtual
    ~ChannelConnectionForwarder()
                                { }

    void
    Start();

    void
    EnableCopyFromChannel();

    void
    DisableCopyFromChannel();

    void
    EnableCopyFromNetConn();

    void
    DisableCopyFromNetConn();

//==FDEventHandler
    virtual len_t
    HandleFDRead(PollableFDPtr) override;

    virtual len_t
    HandleFDWrite(PollableFDPtr) override;

    virtual len_t
    HandleFDError(PollableFDPtr, int16_t) override;

//==ChannelEventHandler
    virtual void
    ChannelDataReceived(ChannelPtr) override;

    virtual void
    ChannelReadyToSend(ChannelPtr, tUint32) override;

    virtual void
    ChannelError(ChannelPtr, tError errorCode, tString errorText) override;

    virtual void
    ChannelRejected(ChannelPtr, tString reason) override;

    virtual void
    ChannelAccepted(ChannelPtr) override;

    virtual void
    ChannelCleanup(ChannelPtr) override;

private:

    void
    closeByNetConn();

    ChannelPtr                  channel;
    net::NetworkConnectionPtr   netConn;
    DataTransferCounterPtr      counter;

    bool                        allowCopyFromChannel;
    bool                        allowCopyFromNetConn;
    bool                        fdRecvEnabled;
    bool                        fdSendEnabled;

    RawDataPtr                  dataForChannel;
    RawDataPtr                  dataForNetConn;
};
DefineMakeSharedPtr(ChannelConnectionForwarder);

} // namespace protocol


#endif // SRC_CPP_PROTOCOL_CHANNELCONNECTIONFORWARDING_HH_
