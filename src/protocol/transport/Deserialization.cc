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
#include <utils/Utils.hh>


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

class DecodeException : public std::exception {
private:
    tString msg;
public:
    // Constructor to initialize the error message
    DecodeException(const tString msg): msg(msg) { }

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

//==========================================

Deserializer::Deserializer(bool mismatchedEndianness) :
            mismatchedEndianness(mismatchedEndianness)
{
}

Deserializer::~Deserializer()
{
    if (memValue.self) {
        delete memValue.self;
        memValue.self = nullptr;
    }
}

#define DefineDecodeLit(x) \
PinggyValue::PinggyInternalTypePtr \
Deserializer::decode##x(RawDataPtr stream, PathRegistryPtr pathRegistry) \
{ \
    t##x val; \
    Deserialize_Lit(stream, val, mismatchedEndianness); \
    return PinggyValue::NewPinggyInternalType_##x##Ptr(val); \
}
FOREACH_ALL_TYPE(DefineDecodeLit)
#undef DefineDecodeLit

PinggyValue::PinggyInternalTypePtr
Deserializer::decodeArray(RawDataPtr stream, PathRegistryPtr pathRegistry)
{
    tUint16 cnt = 0;
    tUint8  valType = ValueType_Invalid;
    Deserialize_Lit(stream, cnt, mismatchedEndianness);
    Deserialize_Lit(stream, valType, mismatchedEndianness);
    std::vector<PinggyValue::PinggyInternalTypePtr> values;
    for (tUint16 i = 0; i < cnt; i ++) {
        auto val = decodeLit(stream, valType, pathRegistry);
        values.push_back(val);
    }
    return PinggyValue::NewPinggyInternalType_ArrayPtr(values);
}


PinggyValue::PinggyInternalTypePtr
Deserializer::decodeObject(RawDataPtr stream, PathRegistryPtr pathRegistry)
{
    auto obj = PinggyValue::NewPinggyInternalType_ObjectPtr();
    while (true) {
        tUint16 pathId = 0;
        Deserialize_Lit(stream, pathId, mismatchedEndianness);
        if (pathId == RETURN_BACK_PATH_ID)
            break;
        auto pathDef = pathRegistry->GetPathDefForId(pathId);
        auto valType = pathDef->ValType;
        auto val = decodeLit(stream, valType, pathRegistry);
        obj = addValueToPath(obj, pathDef, val);
    }
    return obj;
}

PinggyValue::PinggyInternalTypePtr
Deserializer::decodeLit(RawDataPtr stream, tValueType valType, PathRegistryPtr pathRegistry)
{
    PinggyValue::PinggyInternalTypePtr value;
    switch (valType) {
#define SwitchCaseAnyType(x) \
        case ValueType_##x: \
            value = decode##x(stream, pathRegistry); \
            break;
FOREACH_ANY_TYPE(SwitchCaseAnyType)
#undef SwitchCaseAnyType
        default:
            ABORT_WITH_MSG("Unknown msg"); //TODO replace with std::exception
    }
    return value;
}

void
Deserializer::Decode(RawDataPtr stream, PathRegistryPtr pathRegistry, std::string curPath)
{
    auto rootValue = PinggyValue::NewPinggyInternalType_ObjectPtr();

    while(stream && stream->Len) {
        tPathId pathId;
        Deserialize_Lit(stream, pathId, mismatchedEndianness);
        auto pathDef = pathRegistry->GetPathDefForId(pathId);
        auto value = decodeLit(stream, pathDef->ValType, pathRegistry);
        rootValue = addValueToPath(rootValue, pathDef, value);
    }
    memValue.self = rootValue;
}

PinggyValue::PinggyInternalType_ObjectPtr
Deserializer::addValueToPath(PinggyValue::PinggyInternalType_ObjectPtr root, PathDefinitionPtr pathDef, PinggyValue::PinggyInternalTypePtr value)
{
    auto path = pathDef->Path;
    if (path.size() == 0) {
        delete root;
        if (value->Size() != 0) {
            throw DecodeException("Object cannot be empty here");
        }
        auto vptr = dynamic_cast<PinggyValue::PinggyInternalType_ObjectPtr>(value);
        return vptr;
    }
    auto rv = root;
    std::vector<tString>::size_type i = 0;
    for (; i < path.size() - 1; i++) {
        auto ele = rv->Find(path[i]);
        if (ele == rv->End()) {
            (*rv)[path[i]] = PinggyValue::NewPinggyInternalType_ObjectPtr();
        }
        rv = dynamic_cast<PinggyValue::PinggyInternalType_ObjectPtr>((*rv)[path[i]]);
    }
    (*rv)[path[i]] = value;
    return root;
}

//========================================

INCLUDE_MEMORY_DUMP_DEFINITION
