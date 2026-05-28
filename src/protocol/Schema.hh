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

#ifndef SRC_CPP_PROTOCOL_SCHEMA_HH_
#define SRC_CPP_PROTOCOL_SCHEMA_HH_

#include "transport/SchemaHeaderGenerator.hh"
#include <utils/TunnelCommon.hh>

namespace protocol
{

typedef tUint16 tChannelId;
typedef tUint16 tMsgId;
typedef tUint16 tReqId;
typedef tUint32 tError;

enum tChannelType {
    ChannelType_Invalid = 0,
    ChannelType_Stream,
    ChannelType_DataGram,
    ChannelType_PTY, //For future

    MaxSupportedChannelType
};


// Schema format
// f(<MessageName>,
//        arg,
//        (<Member data type>, <Member name>[, <default value if required>[, 1 if parameter whould a option in constructor]])[,
//        (<Member data type>, <Member name>[, <default value if required>[, 1 if parameter whould a option in constructor]])[,
//        (<Member data type>, <Member name>[, <default value if required>[, 1 if parameter whould a option in constructor]])]]
//  )
// if a member have default value, the member would be serialized only if
// it's value differ from default. Although savings are almost insignificant
// it is a saving.

//      dataType                    Name             default value           present_in_constructor]
#define Schema_SchemaDefinition(f, arg)                                                 \
    f(ClientHello,                                                                      \
        arg,                                                                            \
        (tUint32,                   Version),                                           \
        (tUint32,                   Version2,           0),                             \
        (tString,                   Message,            "")                             \
    )                                                                                   \
    f(ServerHello,                                                                      \
        arg,                                                                            \
        (tUint32,                   Version),                                           \
        (tUint32,                   Version2,           0),                             \
        (tString,                   Message,            "")                             \
    )                                                                                   \
    f(Error,                                                                            \
        arg,                                                                            \
        (tMsgId,                    MsgId),                                             \
        (tError,                    ErrorNo,            0,                          1), \
        (tString,                   What,               "",                         1), \
        (tUint8,                    Recoverable,        0,                          1)  \
    )                                                                                   \
    f(Authenticate,                                                                     \
        arg,                                                                            \
        (tMsgId,                    MsgId),                                             \
        (tString,                   Username,           "",                         1), \
        (tUint8,                    AdvancedParsing,    1,                          1), \
        (tString,                   Arguments,          "",                         1)  \
    )                                                                                   \
    f(AuthenticationResponse,                                                           \
        arg,                                                                            \
        (tMsgId,                    MsgId),                                             \
        (tUint8,                    Success,            0,                          1), \
        (std::vector<tString>,      RedirectTo),                                        \
        (tString,                   Error,              "",                         1), \
        (std::vector<tString>,      Messages),                                          \
        (TunnelInfoPtr,             TunnelInfo,         nullptr)                        \
    )                                                                                   \
    f(RemoteForwardRequest,                                                             \
        arg,                                                                            \
        (tMsgId,                    MsgId),                                             \
        (tReqId,                    ReqId,              0,                          1), \
        (tInt16,                    ListeningPort,      0,                          1), \
        (tString,                   Bind,               "",                         1), \
        (tInt16,                    ForwardingPort,     0,                          1), \
        (tString,                   ForwardingHost,     "",                         1), \
        (TunnelMode,                Mode,               TunnelMode::None)               \
    )                                                                                   \
    f(RemoteForwardResponse,                                                            \
        arg,                                                                            \
        (tMsgId,                    MsgId),                                             \
        (tReqId,                    ReqId,              0,                          1), \
        (tForwardingId,             ForwardingId,       InvalidForwardingId,        1), \
        (std::vector<RemoteForwardingPtr>,                                              \
                                    RemoteForwardings),                                 \
        (tUint8,                    Success,            0,                          1), \
        (std::vector<tString>,      Urls),                                              \
        (tString,                   Error,              "",                         1)  \
    )                                                                                   \
    f(SetupChannel,                                                                     \
        arg,                                                                            \
        (tMsgId,                    MsgId),                                             \
        (tForwardingId,             ForwardingId,       InvalidForwardingId,        1), \
        (tChannelId,                ChannelId,          0,                          1), \
        (tUint16,                   ConnectToPort,      0,                          1), \
        (tString,                   ConnectToHost,      "",                         1), \
        (tUint16,                   SrcPort,            0,                          1), \
        (tString,                   SrcHost,            "",                         1), \
        (tInt8,                     ChannelType,        ChannelType_Stream,         1), \
        (tUint32,                   InitialWindowSize,  0,                          1), \
        (tUint32,                   MaxDataSize,        0,                          1)  \
    )                                                                                   \
    f(SetupChannelResponse,                                                             \
        arg,                                                                            \
        (tMsgId,                    MsgId),                                             \
        (tChannelId,                ChannelId,          0,                          1), \
        (tUint8,                    Accept,             0,                          1), \
        (tString,                   Error,              "",                         1), \
        (tUint32,                   InitialWindowSize,  0,                          1), \
        (tUint32,                   MaxDataSize,        0,                          1)  \
    )                                                                                   \
    f(ChannelData,                                                                      \
        arg,                                                                            \
        (tMsgId,                    MsgId),                                             \
        (tChannelId,                ChannelId,          0,                          1), \
        (tRaw,                      Data)                                               \
    )                                                                                   \
    f(ChannelWindowAdjust,                                                              \
        arg,                                                                            \
        (tMsgId,                    MsgId),                                             \
        (tChannelId,                ChannelId,          0,                          1), \
        (tUint32,                   AdditionalBytes,    0,                          1)  \
    )                                                                                   \
    f(ChannelClose,                                                                     \
        arg,                                                                            \
        (tMsgId,                    MsgId),                                             \
        (tChannelId,                ChannelId,          0,                          1)  \
    )                                                                                   \
    f(ChannelError,                                                                     \
        arg,                                                                            \
        (tMsgId,                    MsgId),                                             \
        (tChannelId,                ChannelId,          0,                          1), \
        (tError,                    ErrorNo,            0,                          1), \
        (tString,                   Error,              "",                         1)  \
    )                                                                                   \
    f(KeepAlive,                                                                        \
        arg,                                                                            \
        (tMsgId,                    MsgId),                                             \
        (tUint64,                   Tick,               0,                          1)  \
    )                                                                                   \
    f(KeepAliveResponse,                                                                \
        arg,                                                                            \
        (tMsgId,                    MsgId),                                             \
        (tUint64,                   ForTick,            0,                          1)  \
    )                                                                                   \
    f(Disconnect,                                                                       \
        arg,                                                                            \
        (tMsgId,                    MsgId),                                             \
        (tString,                   Reason,             "",                         1)  \
    )                                                                                   \
    f(Warning,                                                                          \
        arg,                                                                            \
        (tMsgId,                    MsgId),                                             \
        (tError,                    ErrorNo,            0,                          1), \
        (tString,                   What,               "",                         1)  \
    )                                                                                   \
    f(Usages,                                                                           \
        arg,                                                                            \
        (ClientSpecificUsagesPtr,   Usages)                                             \
    )



SCHEMA_HEADER__DEFINE_HEADERS(Proto, Msg, msg, Schema_SchemaDefinition)

} // namespace protocol

#endif // SRC_CPP_PROTOCOL_SCHEMA_HH_
