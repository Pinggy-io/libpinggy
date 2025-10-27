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

#ifndef SRC_CPP_PINGGYTRANSPORT_TRANSPORTCOMMON_HH_
#define SRC_CPP_PINGGYTRANSPORT_TRANSPORTCOMMON_HH_

#include <stdint.h>
#include <string>
#include <utils/RawData.hh>
#include <platform/Log.hh>


typedef uint16_t tPathId;

typedef uint8_t  tValueType;

typedef uint8_t  tMsgType;

typedef std::string     tString;
typedef RawDataPtr      tRaw;
typedef int8_t          tInt8;
typedef int16_t         tInt16;
typedef int32_t         tInt32;
typedef int64_t         tInt64;
// typedef int128_t        tInt128;
typedef uint8_t         tUint8;
typedef uint16_t        tUint16;
typedef uint32_t        tUint32;
typedef uint64_t        tUint64;
// typedef uint128_t       tUint128;
typedef float           tFloat32;
typedef double          tFloat64;
typedef uint8_t         tBool;

enum MsgType {
    MsgType_Invalid             = (uint8_t)  0,
    MsgType_Value               = (uint8_t)  1,
    MsgType_Type                = (uint8_t)  2
};

const size_t ValueType_Int8_Len     =  1;
const size_t ValueType_Int16_Len    =  2;
const size_t ValueType_Int32_Len    =  4;
const size_t ValueType_Int64_Len    =  8;
const size_t ValueType_Int128_Len   = 16;
const size_t ValueType_Uint8_Len    =  1;
const size_t ValueType_Uint16_Len   =  2;
const size_t ValueType_Uint32_Len   =  4;
const size_t ValueType_Uint64_Len   =  8;
const size_t ValueType_Uint128_Len  = 16;
const size_t ValueType_Float32_Len  =  4;
const size_t ValueType_Float64_Len  =  8;


const tString ValueType_String_Str  = "String";
const tString ValueType_Raw_Str     = "Raw";
const tString ValueType_Int8_Str    = "Int8";
const tString ValueType_Int16_Str   = "Int16";
const tString ValueType_Int32_Str   = "Int32";
const tString ValueType_Int64_Str   = "Int64";
const tString ValueType_Int128_Str  = "Int128";
const tString ValueType_Uint8_Str   = "Uint8";
const tString ValueType_Uint16_Str  = "Uint16";
const tString ValueType_Uint32_Str  = "Uint32";
const tString ValueType_Uint64_Str  = "Uint64";
const tString ValueType_Uint128_Str = "Uint128";
const tString ValueType_Float32_Str = "Float32";
const tString ValueType_Float64_Str = "Float64";

const tString   String_Default  = "";
const tRaw      Raw_Default     = nullptr;
const tInt8     Int8_Default    = 0;
const tInt16    Int16_Default   = 0;
const tInt32    Int32_Default   = 0;
const tInt64    Int64_Default   = 0;
// const tInt128   Int128_Default  = 0;
const tUint8    Uint8_Default   = 0;
const tUint16   Uint16_Default  = 0;
const tUint32   Uint32_Default  = 0;
const tUint64   Uint64_Default  = 0;
// const tUint128  Uint128_Default = 0;
const tFloat32  Float32_Default = 0.0;
const tFloat64  Float64_Default = 0.0;
const tBool     Bool_Default    = 0;



enum {
    ValueType_Invalid           = (uint8_t) 0,
    ValueType_Array             = (uint8_t)11,
    ValueType_Object            = (uint8_t)21,
    ValueType_ObjectEnd         = (uint8_t)29,
    ValueType_String            = (uint8_t)31,
    ValueType_Raw               = (uint8_t)41,
    ValueType_Int8              = (uint8_t)51,
    ValueType_Int16             = (uint8_t)52,
    ValueType_Int32             = (uint8_t)53,
    ValueType_Int64             = (uint8_t)54,
    ValueType_Int128            = (uint8_t)55,
    ValueType_Uint8             = (uint8_t)56,
    ValueType_Uint16            = (uint8_t)57,
    ValueType_Uint32            = (uint8_t)58,
    ValueType_Uint64            = (uint8_t)59,
    ValueType_Uint128           = (uint8_t)61,
    ValueType_Float32           = (uint8_t)71,
    ValueType_Float64           = (uint8_t)81
};


typedef const char*     tCChar;
const tValueType ValueType_CChar             = (uint8_t)ValueType_String;


#define ROOT_PATH_ID 128
#define RETURN_BACK_PATH_ID 1


#define FOREACH_LITERALS_TYPE(f) \
        f(Int8) \
        f(Int16) \
        f(Int32) \
        f(Int64) \
        f(Uint8) \
        f(Uint16) \
        f(Uint32) \
        f(Uint64) \
        f(Float32) \
        f(Float64)

#define FOREACH_ALL_TYPE(f) \
        f(String) \
        f(Raw) \
        FOREACH_LITERALS_TYPE(f)

#define FOREACH_ANY_TYPE(f) \
        FOREACH_ALL_TYPE(f) \
        f(Array) \
        f(Object)


#define FOREACH_LITERALS_TYPE_1(f, x) \
        f(Int8, x) \
        f(Int16, x) \
        f(Int32, x) \
        f(Int64, x) \
        f(Uint8, x) \
        f(Uint16, x) \
        f(Uint32, x) \
        f(Uint64, x) \
        f(Float32, x) \
        f(Float64, x)

#define FOREACH_ALL_TYPE_1(f, x) \
        f(String, x) \
        f(Raw, x) \
        FOREACH_LITERALS_TYPE_1(f, x)

#define FOREACH_ANY_TYPE_1(f, x) \
        FOREACH_ALL_TYPE_1(f, x) \
        f(Array, x) \
        f(Object, x)



#define FOREACH_LITERALS_TYPE_2(f, x, y) \
        f(Int8, x, y) \
        f(Int16, x, y) \
        f(Int32, x, y) \
        f(Int64, x, y) \
        f(Uint8, x, y) \
        f(Uint16, x, y) \
        f(Uint32, x, y) \
        f(Uint64, x, y) \
        f(Float32, x, y) \
        f(Float64, x, y)

#define FOREACH_ALL_TYPE_2(f, x, y) \
        f(String, x, y) \
        f(Raw, x, y) \
        FOREACH_LITERALS_TYPE_2(f, x, y)

#define FOREACH_ANY_TYPE_2(f, x, y) \
        FOREACH_ALL_TYPE_2(f, x, y) \
        f(Array, x, y) \
        f(Object, x, y)

#endif // SRC_CPP_PINGGYTRANSPORT_TRANSPORTCOMMON_HH_
