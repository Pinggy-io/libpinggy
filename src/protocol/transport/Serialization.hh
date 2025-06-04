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

// Primary template for checking if a type is a specialization of std::vector
template<typename T>
struct IsVector : std::false_type {};

// Specialization for std::vector
template<typename T, typename Alloc>
struct IsVector<std::vector<T, Alloc>> : std::true_type {};


typedef const char*     tCChar;
const tValueType ValueType_CChar             = (uint8_t)31;

DeclareClassWithSharedPtr(Serializer);
DeclareClassWithSharedPtr(TransportManager);


#define DeclareSerializeHeader(x)                                       \
    inline tValueType GetContentType(t##x) { return ValueType_##x; }    \
    void Serialize_Lit(RawDataPtr stream, t##x item, bool swapBytes);   \

DeclareSerializeHeader(CChar) //This is basically for Serializer only.
FOREACH_ALL_TYPE(DeclareSerializeHeader)

#undef DeclareSerializeHeader

template<typename T>
tValueType GetContentType(std::vector<T>) { return ValueType_Array; }

template<typename T>
tValueType GetContentType(T) {return ValueType_Object;}

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
    bool isArray;
    bool isNotArray;
    bool mismatchedEndianness;
    TransportManagerWPtr transportManager;

    Serializer(PathRegistryPtr pathRegistry, bool mismatchedEndianness=false, RawDataPtr stream=nullptr, tPathId pathId=ROOT_PATH_ID, TransportManagerPtr trans= nullptr);

//=====================================
// deflateForArray
#define DefineSerializeDeflateForArray(x) void deflateForArray(t##x t) { Serialize_Lit(stream, t, mismatchedEndianness);}
DefineSerializeDeflateForArray(CChar) //This is basically for Serializer only.
FOREACH_ALL_TYPE(DefineSerializeDeflateForArray)
#undef DefineSerializeDeflateForArray
    template<typename T>
    void deflateForArray(std::vector<T> vt);
    template<typename T>
    void deflateForArray(T t)
    {
        Deflate(thisPtr, t);
        Serialize_Lit(stream, (uint16_t)RETURN_BACK_PATH_ID, mismatchedEndianness);
    }
//=====================================
// serializeArray
#define DefineSerializeArray(x)                                     \
    void serializeArray(std::vector<t##x> vector_t){                \
        uint16_t cnt = (uint16_t)vector_t.size();                   \
        tValueType contentType = ValueType_Invalid;                 \
        if (cnt > 0) {                                              \
            contentType = ValueType_##x;                            \
        }                                                           \
        Serialize_Lit(stream, cnt, mismatchedEndianness);           \
        Serialize_Lit(stream, contentType, mismatchedEndianness);   \
        for (t##x t : vector_t)                                     \
            Serialize_Lit(stream, t, mismatchedEndianness);         \
    }
DefineSerializeArray(CChar) //This is basically for Serializer only.
FOREACH_ALL_TYPE(DefineSerializeArray)
#undef DefineSerializeArray
    template<typename T>
    void serializeArray(std::vector<T> vector_t);

    friend class TransportManager;

public:
    ~Serializer();

#define DeclareSerializeMemFuncHeader(x) SerializerPtr Serialize(std::string, t##x);

DeclareSerializeMemFuncHeader(CChar) //This is basically for Serializer only.
FOREACH_ALL_TYPE(DeclareSerializeMemFuncHeader)

#undef DeclareSerializeMemFuncHeader

    template<typename T>
    SerializerPtr Serialize(std::string, std::vector<T> t);

    template<typename T>
    SerializerPtr Serialize(std::string, T t);

    RawDataPtr GetStream() { return stream; }

    PathRegistryPtr GetPathRegistry() { return pathRegistry; }

    bool Send();
};
DefineMakePrivateSharedPtr(Serializer)
#define NEW_SERIALIZE_PTR(...) NewSerializerPtr(new Serializer(__VA_ARGS__))

template <typename T>
inline void Serializer::deflateForArray(std::vector<T> vector_t)
{
    Assert(isNotArray == false);
    isArray = true;
    serializeArray(vector_t);
}

template <typename T>
inline void Serializer::serializeArray(std::vector<T> vector_t)
{
    uint16_t cnt = vector_t.size();
    tValueType contentType = ValueType_Invalid;
    if (cnt > 0) {
        contentType = GetContentType(vector_t[0]);
    }
    Serialize_Lit(stream, cnt, mismatchedEndianness);
    Serialize_Lit(stream, contentType, mismatchedEndianness);
    for (auto t : vector_t) {
        auto newPathId = pathRegistry->RegisterPath("", contentType, pathId);
        auto serializer = NEW_SERIALIZE_PTR(pathRegistry, mismatchedEndianness, stream, newPathId);
        serializer->deflateForArray(t);
    }
}

template <typename T>
inline SerializerPtr Serializer::Serialize(std::string key, std::vector<T> vector_t)
{
    Assert(isArray == false);
    isNotArray = true;
    Assert(IsVector<decltype(vector_t)>::value);
    auto type = ValueType_Array; // GetValType(vector_t);
    auto pathId = pathRegistry->RegisterPath(key, type, this->pathId);
    auto serializer = NEW_SERIALIZE_PTR(pathRegistry, mismatchedEndianness, stream, pathId);
    Serialize_Lit(stream, pathId, mismatchedEndianness);
    serializer->serializeArray(vector_t);
    return thisPtr;
}

template<typename T>
inline SerializerPtr Serializer::Serialize(std::string key, T t) {
    Assert(isArray == false);
    isNotArray = true;
    auto type = ValueType_Object;
    Assert(IsVector<decltype(t)>::value == false);
    auto pathId = pathRegistry->RegisterPath(key, type, this->pathId);
    auto s = NEW_SERIALIZE_PTR(pathRegistry, mismatchedEndianness, stream, pathId);
    Deflate(s, t);
    return thisPtr;
}

#endif // SRC_CPP_PINGGYTRANSPORT_SERIALIZATION_HH_
