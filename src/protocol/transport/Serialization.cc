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

#include "Serialization.hh"
#include <platform/assert_pinggy.h>

#include "TransportManager.hh"

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


#define AddOrThrow(stream, data, len)                       \
    if(!stream->AddData(data, len))                         \
        throw std::runtime_error("Could not serialise")

void Serialize_Lit(RawDataPtr stream, std::string item, bool swapBytes)
{
    uint16_t len = (tUint16)item.length();
    Serialize_Lit(stream, len, swapBytes);
    if (len) {
        AddOrThrow(stream, item.c_str(), (RawData::tLen)item.length());
    }
}

void Serialize_Lit(RawDataPtr stream, tCChar item, bool swapBytes)
{
    uint16_t len = (tUint16)strlen(item);
    Serialize_Lit(stream, len, swapBytes);
    if (len) {
        AddOrThrow(stream, item, len);
    }
}

void Serialize_Lit(RawDataPtr stream, RawDataPtr data, bool swapBytes)
{
    uint16_t len = data->Len;
    Serialize_Lit(stream, len, swapBytes);
    if (len > 0) {
        if (!stream->AddData(data->GetData(), data->Len)) {
            throw std::runtime_error("Error while sending string");
        }
    }
}

#define DefineSerilizeLitBody(x)                                        \
    void Serialize_Lit(RawDataPtr stream, t##x data, bool swapBytes)    \
    {                                                                   \
        auto tempData = swapBytes ? byteSwap(data) : data;              \
        AddOrThrow(stream, &tempData, sizeof(t##x));                    \
    }

#define DefineSerilizationFunctions(x) \
    DefineSerilizeLitBody(x)



FOREACH_LITERALS_TYPE(DefineSerilizationFunctions)


//==========================================


Serializer::Serializer(PathRegistryPtr pathRegistry, bool mismatchedEndianness, RawDataPtr stream, tPathId pathId, TransportManagerPtr trans):
            pathRegistry(pathRegistry),
            stream(stream),
            pathId(pathId),
            isArray(false),
            isNotArray(false),
            mismatchedEndianness(mismatchedEndianness),
            transportManager(trans)
{
    if (!this->stream) {
        this->stream = NewRawDataPtr();
    }
}

Serializer::~Serializer()
{
}

bool Serializer::Send()
{
    return transportManager.lock()->SendMsg(thisPtr);
}

#define DeclareSerializeMemFuncBodyWithKey(_x)                              \
SerializerPtr Serializer::Serialize(std::string key, t##_x t)               \
{                                                                           \
    Assert(isArray == false);                                               \
    isNotArray = true;                                                      \
    Assert(key.length() > 0 && key.find('.') == key.npos);                  \
    auto path = key; /*curPath + "." + key;*/                               \
    auto type = ValueType_##_x;                                             \
    auto pathId = pathRegistry->RegisterPath(path, type, this->pathId);     \
    Serialize_Lit(stream, pathId, mismatchedEndianness);                    \
    Serialize_Lit(stream, t, mismatchedEndianness);                         \
    return thisPtr;                                                         \
}                                                                           \




#define DeclareSerializeMemFuncBody(_x)  DeclareSerializeMemFuncBodyWithKey(_x)



DeclareSerializeMemFuncBody(CChar) //This is basically for Serializer only.
FOREACH_ALL_TYPE(DeclareSerializeMemFuncBody)
