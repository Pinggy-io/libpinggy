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
    void Inflate(DeserializerPtr s, t##x &item); \

FOREACH_ALL_TYPE(DeclareDeserializeHeader)

#undef DeclareDeserializeHeader

DeclareClassWithSharedPtr(ArrayContainer);

class Deserializer: virtual public pinggy::SharedObject
{
private:
    std::map<std::string, DeserializerPtr> children;
    std::map<std::string, std::pair<tValueType, RawDataPtr>> literals;

    tValueType valueType; //for the current object
    tValueType contentType; //for the objects inside arrayOfObjects
    std::vector<RawDataPtr> literalsArray;
    std::vector<DeserializerPtr> arrayOfObjects;

    PinggyValue memValue;

    bool mismatchedEndianness;

    DeserializerPtr getDeserializer(PathDefinitionPtr pathDef);

    DeserializerPtr parseArrayObject(RawDataPtr stream, PathRegistryPtr pathRegistry);
    DeserializerPtr parseArray(RawDataPtr stream, PathRegistryPtr pathRegistry);
    void parseLit(RawDataPtr stream, PathRegistryPtr pathRegistry, PathDefinitionPtr pathDef);
    void pushLiteralToPath(PathDefinitionPtr pathDef, RawDataPtr value);


#define DeclareArrayDeserialize(x) \
    virtual void deserialize_internal(std::vector<t##x> &val);

FOREACH_ALL_TYPE(DeclareArrayDeserialize)
#undef DeclareArrayDeserialize

    template<typename T>
    void deserialize_internal(std::vector<T> &val);
    template<typename T>
    void deserialize_internal(T &val);

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
    Parse(RawDataPtr stream, PathRegistryPtr pathRegistry, std::string curPath = "");

    virtual void
    Decode(RawDataPtr stream, PathRegistryPtr pathRegistry, std::string curPath = "");

    virtual tString
    Dump();

    virtual bool
    HasChild(tString key);

    template<typename T>
    void
    Deserialize(tString key, T &val);

    template<typename T>
    void
    Deserialize(tString key, std::vector<T> &val);

#define DeclareDeserialize(x) \
    virtual void Deserialize(tString key, t##x &val, t##x defaultVal = x##_Default); \
    virtual void Deserialize(tString key, std::vector<t##x> &val);
FOREACH_ALL_TYPE(DeclareDeserialize)
#undef DeclareDeserialize
};
DefineMakeSharedPtr(Deserializer);
#define NEW_DESERIALIZE_PTR(...) NewDeserializerPtr(new Deserializer(__VA_ARGS__))

template <typename T>
inline void Deserializer::deserialize_internal(std::vector<T> &val)
{
    val.clear();
    for(auto rd : arrayOfObjects) {
        T v;
        if (contentType == ValueType_Array) {
            rd->deserialize_internal(v);
        } else {
            ABORT_WITH_MSG("Not possible")
        }
        val.push_back(v);
    }
}

template <typename T>
inline void Deserializer::deserialize_internal(T &val)
{
    if (valueType == ValueType_Object) {
        Inflate(thisPtr, val);
    } else {
        ABORT_WITH_MSG("Not possible", contentType);
    }
}

template <typename T>
inline void Deserializer::Deserialize(tString key, T &val)
{
    if (children.find(key) == children.end()) {
        return;
    }
    Assert(valueType == ValueType_Object);
    auto deserialize = children.at(key);
    Inflate(deserialize, val);
}

template <typename T>
inline void Deserializer::Deserialize(tString key, std::vector<T> &val)
{
    if (children.find(key) == children.end()) {
        return;
    }
    auto item = children.at(key);

    if (item->valueType != ValueType_Array) {
        throw std::runtime_error("Type mismatched");
    }

    val.clear();
    for (auto des : item->arrayOfObjects) {
        T v;
        des->deserialize_internal(v);
        val.push_back(v);
    }
}

#endif // SRC_CPP_PINGGYTRANSPORT_DESERIALIZATION_HH_
