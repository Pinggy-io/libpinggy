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

#ifndef SRC_CPP_PINGGYTRANSPORT_SERIALIZATION_HH_
#define SRC_CPP_PINGGYTRANSPORT_SERIALIZATION_HH_
#include "TransportCommon.hh"
#include "PathRegistry.hh"
#include <vector>
#include <platform/assert_pinggy.h>
#include <type_traits>
#include "PinggyValue.hh"

DeclareClassWithSharedPtr(Serializer);
DeclareClassWithSharedPtr(TransportManager);


#define DeclareSerializeHeader(x)                                       \
    void Serialize_Lit(RawDataPtr stream, t##x item, bool swapBytes);   \

DeclareSerializeHeader(CChar) //This is basically for Serializer only.
FOREACH_ALL_TYPE(DeclareSerializeHeader)

#undef DeclareSerializeHeader

void Serialize_Lit(RawDataPtr stream, std::string item, bool swapBytes);

void Serialize_Lit(RawDataPtr stream, RawDataPtr data, bool swapBytes);



//=================

class Serializer : public virtual pinggy::SharedObject
{
private:
    PathRegistryPtr pathRegistry;
    // std::string curPath;
    RawDataPtr stream;
    // tPathId parent;
    tPathId pathId;
    bool mismatchedEndianness;
    TransportManagerWPtr transportManager;

    Serializer(PathRegistryPtr pathRegistry, bool mismatchedEndianness=false, RawDataPtr stream=nullptr, tPathId pathId=ROOT_PATH_ID, TransportManagerPtr trans= nullptr);

    friend class TransportManager;

    void
    encodeLiterals(PinggyValue::PinggyInternalTypePtr value, tPathId parentPathId);

#define declareEncoders(x) \
    void \
    encode_ ## x(PinggyValue::PinggyInternalType_##x##Ptr value, tPathId parentPathId);
FOREACH_ANY_TYPE(declareEncoders)
#undef declareEncoders

    void
    encode(PinggyValue &val);

public:
    ~Serializer();

    RawDataPtr
    GetStream()                 { return stream; }

    PathRegistryPtr
    GetPathRegistry()           { return pathRegistry; }

    DefineMandatoryClassFunctionsWOSuper(Serializer);
};
DefineMakePrivateSharedPtr(Serializer)
#define NEW_SERIALIZE_PTR(...) NewSerializerPtr(new Serializer(__VA_ARGS__))

#endif // SRC_CPP_PINGGYTRANSPORT_SERIALIZATION_HH_
