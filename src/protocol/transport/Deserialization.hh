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

#ifndef SRC_CPP_PINGGYTRANSPORT_DESERIALIZATION_HH_
#define SRC_CPP_PINGGYTRANSPORT_DESERIALIZATION_HH_

#include "TransportCommon.hh"
#include "PathRegistry.hh"
#include <vector>
#include <platform/assert_pinggy.h>
#include "PinggyValue.hh"

DeclareClassWithSharedPtr(Deserializer);

#define DeclareDeserializeHeader(x) \
    void Deserialize_Lit(RawDataPtr stream, t##x &item, bool swapBytes); \

FOREACH_ALL_TYPE(DeclareDeserializeHeader)

#undef DeclareDeserializeHeader

class Deserializer: virtual public pinggy::SharedObject
{
private:
    PinggyValue memValue;

    bool mismatchedEndianness;

#define DeclareDecodeLit(x) \
    PinggyValue::PinggyInternalTypePtr \
    decode##x(RawDataPtr stream, PathRegistryPtr pathRegistry);
FOREACH_ANY_TYPE(DeclareDecodeLit)
#undef DeclareDecodeLit

    PinggyValue::PinggyInternalType_ObjectPtr
    addValueToPath(PinggyValue::PinggyInternalType_ObjectPtr root, PathDefinitionPtr, PinggyValue::PinggyInternalTypePtr value);

    PinggyValue::PinggyInternalTypePtr
    decodeLit(RawDataPtr stream, tValueType valType, PathRegistryPtr pathRegistry);

    friend class TransportManager;
    Deserializer(bool mismatchedEndianness);

    //this is private because we don't want to return a non referenced object
    PinggyValue&
    getDecodedStream()              { return memValue; }

public:
    ~Deserializer();

    virtual void
    Decode(RawDataPtr stream, PathRegistryPtr pathRegistry, std::string curPath = "");

    DefineMandatoryClassFunctionsWOSuper(Deserializer);
};
DefineMakeSharedPtr(Deserializer);
#define NEW_DESERIALIZE_PTR(...) NewDeserializerPtr(new Deserializer(__VA_ARGS__))

#endif // SRC_CPP_PINGGYTRANSPORT_DESERIALIZATION_HH_
