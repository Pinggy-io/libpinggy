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

#include "Deserialization.hh"
#include <platform/assert_pinggy.h>
#include <map>


const std::map<tValueType, tString> typeToTypeNameMap = {
                    {ValueType_String,   ValueType_String_Str },
                    {ValueType_Raw,      ValueType_Raw_Str    },
                    {ValueType_Int8,     ValueType_Int8_Str   },
                    {ValueType_Int16,    ValueType_Int16_Str  },
                    {ValueType_Int32,    ValueType_Int32_Str  },
                    {ValueType_Int64,    ValueType_Int64_Str  },
                    {ValueType_Int128,   ValueType_Int128_Str },
                    {ValueType_Uint8,    ValueType_Uint8_Str  },
                    {ValueType_Uint16,   ValueType_Uint16_Str },
                    {ValueType_Uint32,   ValueType_Uint32_Str },
                    {ValueType_Uint64,   ValueType_Uint64_Str },
                    {ValueType_Uint128,  ValueType_Uint128_Str},
                    {ValueType_Float32,  ValueType_Float32_Str},
                    {ValueType_Float64,  ValueType_Float64_Str}
        };

#define _SWAP_U16(x) ((x & 0xff) << 8) | ((x & 0xff00) >> 8)
#define SWAP_16(x) _SWAP_U16(((uint16_t)x))

#define _SWAP_U32(x) ((x & 0xffU) << 24) | ((x & 0xff00U) << 8) | ((x & 0xff0000U) >> 8) | ((x & 0xff000000U) >> 24)
#define SWAP_32(x) _SWAP_U32(((uint32_t)x))

#ifdef __OS_32BIT
#define _SWAP_U64(x) ( ((uint64_t)(_SWAP_U32((x & 0xffffffffULL)))) << 32) | \
                          (_SWAP_U32(  (uint32_t)((x & (0xffffffffULL << 32))>>32) ))
#else
#define _SWAP_U64(x) ( ((uint64_t)(_SWAP_U32((x & 0xffffffffUL)))) << 32) | \
                          (_SWAP_U32(  (uint32_t)((x & (0xffffffffUL << 32))>>32) ))
#endif
#define SWAP_64(x) _SWAP_U64(((uint64_t)x))

inline static tUint8   byteSwap(tUint8   x)          {return x;}
inline static tInt8    byteSwap(tInt8    x)          {return x;}
inline static tUint16  byteSwap(tUint16  x)          {x = SWAP_16(x); return x;}
inline static tInt16   byteSwap(tInt16   x)          {x = (tInt16)SWAP_16(x); return x;}
inline static tUint32  byteSwap(tUint32  x)          {x = SWAP_32(x); return x;}
inline static tInt32   byteSwap(tInt32   x)          {x = (tInt32)SWAP_32(x); return x;}
inline static tUint64  byteSwap(tUint64  x)          {x = SWAP_64(x); return x;}
inline static tInt64   byteSwap(tInt64   x)          {x = (tInt32)SWAP_64(x); return x;}
inline static tFloat32 byteSwap(tFloat32 x)          {union {tFloat32 f; tUint32 u;} val; val.f = x; val.u = SWAP_32(val.u); return val.f;}
inline static tFloat64 byteSwap(tFloat64 x)          {union {tFloat64 f; tUint64 u;} val; val.f = x; val.u = SWAP_64(val.u); return val.f;}

class ArrayContainer: virtual public pinggy::SharedObject
{
public:
    ArrayContainer(tValueType valueType): valueType(valueType) {}
    virtual ~ArrayContainer() {};

    tString dump();

    tValueType valueType;
    std::vector<ArrayContainerPtr> literalsArray;
    std::vector<RawDataPtr> literals;
    std::vector<DeserializerPtr> arrayOfObjects;
};
DefineMakeSharedPtr(ArrayContainer);
class CustingException : public std::exception {
private:
    tString msg;
public:
    // Constructor to initialize the error message
    CustingException(const tValueType from, const tValueType to) {
        msg = "Cannot convert from `" +
                    typeToTypeNameMap.at(from) + "("+std::to_string(from)+")` to `" +
                    typeToTypeNameMap.at(to) + "("+std::to_string(to)+")`";
    }

    // Override the what() method from std::exception
    virtual const char* what() const noexcept override {
        return msg.c_str();
    }
};


#define RetrieveOrThrow(strem, data, len) \
    { \
        if (stream->Len < (RawData::tLen)len) { \
            throw std::runtime_error("Don't have enough data to deserialize"); \
        } \
        memcpy(data, stream->GetData(), len); \
        stream->Consume(len); \
    }

void Deserialize_Lit(RawDataPtr stream, std::string &item, bool swapBytes)
{
    uint16_t len = 0;
    Deserialize_Lit(stream, len, swapBytes);
    if (swapBytes) len = byteSwap(len);
    if (len) {
        if (stream->Len < len) {
            throw std::runtime_error("Don't have enough data to deserialize");
        }
        item = std::string(stream->GetData(), len);
        stream->Consume(len);
    }
}

void Deserialize_Lit(RawDataPtr stream, RawDataPtr &data, bool swapBytes)
{
    uint16_t len = 0;
    Deserialize_Lit(stream, len, swapBytes);
    if (swapBytes) len = byteSwap(len);
    if (len) {
        if (stream->Len < len) {
            throw std::runtime_error("Don't have enough data to deserialize");
        }
        data = stream->Slice(0,len);
        stream->Consume(len);
    }
}

#define DefineDeserilizeLitBody(x) \
    void Deserialize_Lit(RawDataPtr stream, t##x &data, bool swapBytes) \
    { \
        RetrieveOrThrow(stream, &data, sizeof(t##x)); \
        if (swapBytes) data = byteSwap(data); \
    }

#define DefineDeserilizationFunctions(x) \
    DefineDeserilizeLitBody(x)


FOREACH_LITERALS_TYPE(DefineDeserilizationFunctions)

#define FOR_EACH_FRONT_CASE(x) \
        case ValueType_##x: \
            { \
                t##x tVal; \
                Deserialize_Lit(stream, tVal, swapBytes); \
                val = tVal; \
            } \
            break;

#define DefineDeserializeLiteralAllowedTypes(...) \
    APP_MACRO_FOR_EACH_FORNT(FOR_EACH_FRONT_CASE, __VA_ARGS__)

#define DefineDeserializeLiteralWithType(x, ...) \
void deserializeLiteralWithType(tRaw stream, t##x &val, bool swapBytes, tValueType type) \
{ \
    switch (type) \
    { \
        DefineDeserializeLiteralAllowedTypes(__VA_ARGS__) \
    default: \
        throw CustingException(type, ValueType_##x);\
    } \
} \

DefineDeserializeLiteralWithType(Raw, Raw)
DefineDeserializeLiteralWithType(String, String)
DefineDeserializeLiteralWithType(Int64, Int8, Int16, Int32, Int64)
DefineDeserializeLiteralWithType(Int32, Int8, Int16, Int32)
DefineDeserializeLiteralWithType(Int16, Int8, Int16)
DefineDeserializeLiteralWithType(Int8, Int8)
DefineDeserializeLiteralWithType(Uint64, Uint8, Uint16, Uint32, Uint64)
DefineDeserializeLiteralWithType(Uint32, Uint8, Uint16, Uint32)
DefineDeserializeLiteralWithType(Uint16, Uint8, Uint16)
DefineDeserializeLiteralWithType(Uint8, Uint8)
DefineDeserializeLiteralWithType(Float64, Float32, Float64)
DefineDeserializeLiteralWithType(Float32, Float32)

//==========================================

Deserializer::Deserializer(bool mismatchedEndianness) :
            valueType(ValueType_Object), contentType(ValueType_Invalid), mismatchedEndianness(mismatchedEndianness)
{
}

Deserializer::~Deserializer()
{
}

DeserializerPtr Deserializer::getDeserializer(PathDefinitionPtr pathDef)
{
    auto curPtr = thisPtr;
    for(size_t i = 0; (pathDef->Path.size() - 1) > i; i++) {
        auto basename = pathDef->Path[i];
        if (curPtr->children.find(basename) == curPtr->children.end()) {
            auto nextPtr = NEW_DESERIALIZE_PTR(mismatchedEndianness);
            nextPtr->mismatchedEndianness = mismatchedEndianness;
            curPtr->children[basename] = nextPtr;
        }
        auto nextPtr = curPtr->children[basename];
        curPtr = nextPtr;
    }
    return curPtr;
}

DeserializerPtr Deserializer::parseArrayObject(RawDataPtr stream, PathRegistryPtr pathRegistry)
{
        auto newDeseirializer = NEW_DESERIALIZE_PTR(mismatchedEndianness);
        while (true) {
            tPathId pathId = 0;
            Deserialize_Lit(stream, pathId, mismatchedEndianness);
            if (pathId == RETURN_BACK_PATH_ID)
                break;
            auto pathDef = pathRegistry->GetPathDefForId(pathId);
            newDeseirializer->parseLit(stream, pathRegistry, pathDef);
        }
        return newDeseirializer;
}

DeserializerPtr Deserializer::parseArray(RawDataPtr stream, PathRegistryPtr pathRegistry)
{
    uint16_t cnt;
    tValueType contentType;
    Deserialize_Lit(stream, cnt, mismatchedEndianness);
    Deserialize_Lit(stream, contentType, mismatchedEndianness);
    auto arrayContainer = NEW_DESERIALIZE_PTR(mismatchedEndianness);
    arrayContainer->contentType = contentType;
    arrayContainer->valueType = ValueType_Array;
    for(auto i = 0; i < cnt; i++) {
        switch (contentType)
        {
        case ValueType_Object:
            {
                auto val = parseArrayObject(stream, pathRegistry);
                arrayContainer->arrayOfObjects.push_back(val);
            }
            break;
        case ValueType_Array:
            {
                auto val = parseArray(stream, pathRegistry);
                arrayContainer->arrayOfObjects.push_back(val);
            }
            break;
        case ValueType_String:
        case ValueType_Raw:
            {
                uint16_t len;
                Deserialize_Lit(stream->Slice(0), len, mismatchedEndianness);
                auto val = stream->Slice(0, len+2);
                stream->Consume(len+2);
                arrayContainer->literalsArray.push_back(val);
            }
            break;
#define caseLiteral(x) \
        case ValueType_##x: \
            { \
                if (stream->Len < (RawData::tLen)ValueType_##x##_Len) \
                    throw std::runtime_error("Don't have enough data to deserialize"); \
                auto val = stream->Slice(0, ValueType_##x##_Len); \
                stream->Consume(ValueType_##x##_Len); \
                arrayContainer->literalsArray.push_back(val);\
            }\
            break;
FOREACH_LITERALS_TYPE(caseLiteral)
#undef caseLiteral
        default:
            break;
        }
    }

    return arrayContainer;
}

void Deserializer::parseLit(RawDataPtr stream, PathRegistryPtr pathRegistry, PathDefinitionPtr pathDef)
{
    switch (pathDef->ValType)
    {
    case ValueType_Array:
        {
            auto val = parseArray(stream, pathRegistry);
            auto deserializer = getDeserializer(pathDef);
            deserializer->children[pathDef->Basename] = val;
        }
        break;
    case ValueType_Object:
        throw std::runtime_error("OBJECT Type not allowed here");
        break;

    case ValueType_String:
    case ValueType_Raw:
        {
            uint16_t len;
            Deserialize_Lit(stream->Slice(0), len, mismatchedEndianness);
            if (stream->Len < len + 2)
                throw std::runtime_error("Don't have enough data to deserialize");
            auto val = stream->Slice(0, len + 2);
            stream->Consume(len + 2);
            pushLiteralToPath(pathDef, val);
        }
        break;
#define caseLiteral(x) \
    case ValueType_##x: \
        { \
            if (stream->Len < (RawData::tLen)ValueType_##x##_Len) \
                throw std::runtime_error("Don't have enough data to deserialize"); \
            auto val = stream->Slice(0, ValueType_##x##_Len); \
            stream->Consume(ValueType_##x##_Len); \
            pushLiteralToPath(pathDef, val); \
        } \
        break;
FOREACH_LITERALS_TYPE(caseLiteral)
#undef caseLiteral

    default:
        throw std::runtime_error("Unknown type of msg " + std::to_string(pathDef->ValType)+ " pathId: " + std::to_string(pathDef->PathId));
        break;
    }
}

void Deserializer::pushLiteralToPath(PathDefinitionPtr pathDef, RawDataPtr value)
{
    auto deserializer = getDeserializer(pathDef);
    deserializer->literals[pathDef->Basename] = {pathDef->ValType, value};
}

void Deserializer::Parse(RawDataPtr stream, PathRegistryPtr pathRegistry, std::string curPath)
{
    while(stream && stream->Len) {
        tPathId pathId;
        Deserialize_Lit(stream, pathId, mismatchedEndianness);
        auto pathDef = pathRegistry->GetPathDefForId(pathId);
        parseLit(stream, pathRegistry, pathDef);
    }
}

tString Deserializer::Dump()
{
    tString dump = "";
    if (valueType == ValueType_Object) {
        dump = "{";
        for (auto value : literals) {
            dump += "\"" + value.first + "\": ";
            dump += std::to_string(value.second.first) + ",";
        }
        for (auto value : children) {
            dump += "\"" + value.first + "\": ";
            dump += value.second->Dump() + ",";
        }
        dump += "}";
    } else if (valueType == ValueType_Array) {
        dump = "[";
        for (auto item : literalsArray) {
            dump += std::to_string(valueType) + ",";
        }
        for (auto item : this->arrayOfObjects) {
            dump += item->Dump() + ",";
        }
        dump += "]";
    } else {
        throw std::runtime_error("Unexpected type " + std::to_string(valueType));
    }
    return dump;
}

#define DefineDeserialize(x)                                                        \
void Deserializer::Deserialize(tString key, t##x &val, t##x defaultVal)             \
{                                                                                   \
    if (literals.find(key) == literals.end()) {                                     \
        val = defaultVal;                                                           \
        return;                                                                     \
    }                                                                               \
    auto item = literals.at(key);                                                   \
    deserializeLiteralWithType(item.second, val, mismatchedEndianness, item.first); \
}                                                                                   \
                                                                                    \
void Deserializer::Deserialize(tString key, std::vector<t##x> &val)                 \
{                                                                                   \
    if (children.find(key) == children.end()) {                                     \
        return;                                                                     \
    }                                                                               \
    auto item = children.at(key);                                                   \
    if (item->valueType != ValueType_Array) {                                       \
        throw std::runtime_error("Type mismatched");                                \
    }                                                                               \
    item->deserialize_internal(val);                                                         \
}                                                                                   \
                                                                                    \
void Deserializer::deserialize_internal(std::vector<t##x> &val)                              \
{                                                                                   \
    val.clear();                                                                    \
    for(auto rd : literalsArray) {                                                  \
        t##x v;                                                                     \
        deserializeLiteralWithType(rd, v, mismatchedEndianness, contentType);       \
        val.push_back(v);                                                           \
    }                                                                               \
}

FOREACH_ALL_TYPE(DefineDeserialize)

tString ArrayContainer::dump()
{
    tString dump = "[";
    for (auto item : literals) {
        dump += std::to_string(valueType) + ",";
    }
    for (auto item : literalsArray) {
        dump += item->dump() + ",";
    }
    for (auto item : this->arrayOfObjects) {
        dump += item->Dump() + ",";
    }
    dump += "]";
    return dump;
}

//========================================
