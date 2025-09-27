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

#include "Schema.hh"
#include "transport/SchemaHeaderGenerator.hh"
#include "transport/SchemaBodyGenerator.hh"


DEFINE_TRANSPORT_SERIALIZER_DESERIALIZER_PTR_V2(ClientSpecificUsages,
        CLIENT_SPECIFIC_USAGES_JSON_FIELDS_MAP
)

DEFINE_TRANSPORT_SERIALIZER_DESERIALIZER_PTR_V1(SpecialPortConfig,
        SPECIAL_PORT_BASIC_FIELDS
)

DEFINE_TRANSPORT_SERIALIZER_DESERIALIZER_ENUM_TYPE(TunnelMode, tString,
        {TunnelMode::None,      TunnelType_None},
        {TunnelMode::HTTP,      TunnelType_HTTP},
        {TunnelMode::TCP,       TunnelType_TCP},
        {TunnelMode::TLS,       TunnelType_TLS},
        {TunnelMode::TLSTCP,    TunnelType_TLSTCP},
        {TunnelMode::UDP,       TunnelType_UDP},
)

DEFINE_TRANSPORT_SERIALIZER_DESERIALIZER_PTR_V2(TunnelInfo,
    (GreetingMsg, greetingMsg),
    (PortConfig, portConfig)
)

namespace protocol
{

SCHEMA_BODY__DEFINE_BODIES(Proto, Msg, msg, Schema_SchemaDefinition)

} // namespace protocol
