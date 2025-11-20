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

#ifndef __SRC_CPP_PROTOCOL_TRANSPORT_SCHEMABODYGENERATOR_HH__
#define __SRC_CPP_PROTOCOL_TRANSPORT_SCHEMABODYGENERATOR_HH__

#include "SchemaHeaderGenerator.hh"
#include <algorithm>

//==============================================================================
//    Define exception
//==============================================================================

#define _SCHEMA_BODY_DefineExceptionClasses(RootClass, ClassSuffix)             \
class RootClass##ClassSuffix##SerializationException:public std::exception,     \
    public virtual pinggy::SharedObject                                         \
{                                                                               \
public:                                                                         \
    RootClass##ClassSuffix##SerializationException(tString message) :           \
        message(message) {}                                                     \
    virtual ~ RootClass##ClassSuffix##SerializationException() {};              \
                                                                                \
    virtual const char*                                                         \
    what() const noexcept override      { return message.c_str(); }             \
                                                                                \
private:                                                                        \
    tString                     message;                                        \
};                                                                              \
                                                                                \
class RootClass##ClassSuffix##DeserializationException:public std::exception,   \
        public virtual pinggy::SharedObject                                     \
{                                                                               \
public:                                                                         \
    RootClass##ClassSuffix##DeserializationException(tString message) :         \
        message(message) {}                                                     \
    virtual ~ RootClass##ClassSuffix##DeserializationException() {};            \
                                                                                \
    virtual const char*                                                         \
    what() const noexcept override      { return message.c_str(); }             \
                                                                                \
private:                                                                        \
    tString                     message;                                        \
};

//==============================================================================
//    Define constructors
//==============================================================================

#define _SCHEMA_BODY_IfDefaultInitializerList_0(...)
#define _SCHEMA_BODY_IfDefaultInitializerList_1(val, def, ...) , val(def)
#define _SCHEMA_BODY_IfDefaultInitializerList_2(val, def, ...) , val(a##val)
#define _SCHEMA_BODY_IfDefaultInitializerList_(x, y, z, ...) x(y, z)

#define _SCHEMA_BODY_IfDefaultInitializerList(val, def, _, isDef, ...)          \
    _SCHEMA_BODY_IfDefaultInitializerList_(                                     \
        _SCHEMA_BODY_IfDefaultInitializerList_##isDef, val, def)

#define _SCHEMA_BODY_DefineVarInitializerList_(x, y, ...)                       \
    APP_EXPAND(_SCHEMA_BODY_IfDefaultInitializerList(y, ##__VA_ARGS__, 2, 1, 0))

#define _SCHEMA_BODY_DefineVarInitializerList(x)                                \
    _SCHEMA_BODY_DefineVarInitializerList_ x

#define _SCHEMA_BODY_IfDefaultDefaultArgument_0(...)
#define _SCHEMA_BODY_IfDefaultDefaultArgument_1(type, val, def, ...) type a##val,
#define _SCHEMA_BODY_IfDefaultDefaultArgument_(_1, _2, _3, _4, ...) _1(_2, _3, _4)

#define _SCHEMA_BODY_IfDefaultDefaultArgument(type, val, def, _, isDef, ...)    \
    _SCHEMA_BODY_IfDefaultDefaultArgument_(                                     \
        _SCHEMA_BODY_IfDefaultDefaultArgument_##isDef, type, val, def)

#define _SCHEMA_BODY_DefineVarDefaultArgument_(_1, _2, ...)                     \
    APP_EXPAND(_SCHEMA_BODY_IfDefaultDefaultArgument(_1, _2,                    \
        ##__VA_ARGS__, 1, 0, 0))

#define _SCHEMA_BODY_DefineVarDefaultArgument(_1)                               \
    _SCHEMA_BODY_DefineVarDefaultArgument_ _1

#define _SCHEMA_BODY_DefineVarDefaultArgument(_1)                               \
    _SCHEMA_BODY_DefineVarDefaultArgument_ _1


#define _SCHEMA_BODY_DefineMsgConstructor_(ClassName, RootClass,                \
    ClassSuffix, ...)                                                           \
                                                                                \
ClassName##ClassSuffix::ClassName##ClassSuffix(                                 \
    APP_MACRO_FOR_EACH_FORNT(_SCHEMA_BODY_DefineVarDefaultArgument,             \
        __VA_ARGS__) tInt32 __UNUSED_ARG__) :                                   \
            RootClass##ClassSuffix(ClassSuffix##Type_##ClassName)               \
            APP_MACRO_FOR_EACH_FORNT(_SCHEMA_BODY_DefineVarInitializerList,     \
                                        __VA_ARGS__)                            \
{                                                                               \
}                                                                               \

#define _SCHEMA_BODY_DefineMsgConstructor(x, vars, ...)                         \
    _SCHEMA_HEADER_StripParenAndExpand(_SCHEMA_BODY_DefineMsgConstructor_, x,   \
        _SCHEMA_HEADER_StripParen vars, __VA_ARGS__)


//==============================================================================
//    Define functions
//==============================================================================

#define _SCHEMA_BODY_IfDefaultSerilize_0(...)
#define _SCHEMA_BODY_IfDefaultSerilize_1(val, def, ...) if (val != def)
#define _SCHEMA_BODY_IfDefaultSerilize_(x, y, z, ...) x(y, z)

#define _SCHEMA_BODY_IfDefaultSerilize(val, def, _, isDef, ...)                 \
    _SCHEMA_BODY_IfDefaultSerilize_(_SCHEMA_BODY_IfDefaultSerilize##isDef,      \
         val, def)

#define _SCHEMA_BODY_DefineVarSerializer_(x, y, ...)                            \
    APP_EXPAND(_SCHEMA_BODY_IfDefaultSerilize(clsPtr->y, ##__VA_ARGS__, _1,     \
         _0, _0))                                                               \
    serializer->Serialize(#y, clsPtr->y);

#define _SCHEMA_BODY_DefineVarSerializer(x) _SCHEMA_BODY_DefineVarSerializer_ x

#define _SCHEMA_BODY_IfDefaultDeserialize_0(val, ...)                           \
    deserializer->Deserialize(#val, clsPtr->val);

#define _SCHEMA_BODY_IfDefaultDeserialize_1(val, def, ...)                      \
    deserializer->Deserialize(#val, clsPtr->val, def);

#define _SCHEMA_BODY_IfDefaultDeserialize_(x, y, z, ...) x(y, z)

#define _SCHEMA_BODY_IfDefaultDeserialize(val, def, _, isDef, ...)              \
    _SCHEMA_BODY_IfDefaultDeserialize_(                                         \
        _SCHEMA_BODY_IfDefaultDeserialize##isDef, val, def)

#define _SCHEMA_BODY_DefineVarDeserializer_(x, y, ...)                          \
    APP_EXPAND(_SCHEMA_BODY_IfDefaultDeserialize(y, ##__VA_ARGS__, _1, _0, _0))

#define _SCHEMA_BODY_DefineVarDeserializer(x)                                   \
    _SCHEMA_BODY_DefineVarDeserializer_ x

//===========
#define _SCHEMA_BODY_IfDefaultToPinggy_0(...)
#define _SCHEMA_BODY_IfDefaultToPinggy_1(val, def, ...) if (val != def)
#define _SCHEMA_BODY_IfDefaultToPinggy_(x, y, z, ...) x(y, z)

#define _SCHEMA_BODY_IfDefaultToPinggy(val, def, _, isDef, ...)                 \
    _SCHEMA_BODY_IfDefaultToPinggy_(_SCHEMA_BODY_IfDefaultToPinggy##isDef,      \
         val, def)

#define _SCHEMA_BODY_DefineVarToPinggyValue_(x, y, ...)                         \
    APP_EXPAND(_SCHEMA_BODY_IfDefaultToPinggy(clsPtr->y, ##__VA_ARGS__, _1,     \
         _0, _0))                                                               \
    pv.SetFrom(#y, clsPtr->y);

#define _SCHEMA_BODY_DefineVarToPinggyValue(x)                                  \
    _SCHEMA_BODY_DefineVarToPinggyValue_ x
//===========
#define _SCHEMA_BODY_IfDefaultFromPinggy_0(typ, val, ...)                       \
    pv.GetTo(#val, clsPtr->val);

#define _SCHEMA_BODY_IfDefaultFromPinggy_1(typ, val, def, ...)                  \
    pv.GetTo<typ>(#val, clsPtr->val, def);

#define _SCHEMA_BODY_IfDefaultFromPinggy_(x, y, z, w, ...) x(y, z, w)

#define _SCHEMA_BODY_IfDefaultFromPinggy(typ, val, def, _, isDef, ...)          \
    _SCHEMA_BODY_IfDefaultFromPinggy_(                                          \
        _SCHEMA_BODY_IfDefaultFromPinggy##isDef, typ, val, def)

#define _SCHEMA_BODY_DefineVarFromPinggyValue_(x, y, ...)                       \
    APP_EXPAND(_SCHEMA_BODY_IfDefaultFromPinggy(x, y, ##__VA_ARGS__, _1, _0, _0))

#define _SCHEMA_BODY_DefineVarFromPinggyValue(x)                                \
    _SCHEMA_BODY_DefineVarFromPinggyValue_ x
//===========

#define _SCHEMA_BODY_DefineProtocolFunctions_(ClassName, RootClass,             \
    ClassSuffix, ClassSmallSuffix, ...)                                         \
                                                                                \
static void Deflate(SerializerPtr serializer,                                   \
    ClassName##ClassSuffix##Ptr clsPtr)                                         \
{                                                                               \
    APP_MACRO_FOR_EACH_FORNT(_SCHEMA_BODY_DefineVarSerializer, __VA_ARGS__)     \
}                                                                               \
static void Inflate(DeserializerPtr deserializer,                               \
    ClassName##ClassSuffix##Ptr &clsPtr)                                        \
{                                                                               \
    APP_MACRO_FOR_EACH_FORNT(_SCHEMA_BODY_DefineVarDeserializer, __VA_ARGS__)   \
}                                                                               \
static void ToPinggyValue(PinggyValue &pv,                                      \
    const ClassName##ClassSuffix##Ptr &clsPtr)                                  \
{                                                                               \
    APP_MACRO_FOR_EACH_FORNT(_SCHEMA_BODY_DefineVarToPinggyValue, __VA_ARGS__)  \
}                                                                               \
static void FromPinggyValue(PinggyValue &pv,                                    \
    ClassName##ClassSuffix##Ptr &clsPtr)                                        \
{                                                                               \
    APP_MACRO_FOR_EACH_FORNT(_SCHEMA_BODY_DefineVarFromPinggyValue,             \
        __VA_ARGS__)                                                            \
}

#define _SCHEMA_BODY_DefineProtocolFunctions(x, vars, ...)                      \
    _SCHEMA_HEADER_StripParenAndExpand(_SCHEMA_BODY_DefineProtocolFunctions_,   \
        x, _SCHEMA_HEADER_StripParen vars, __VA_ARGS__)


//==============================================================================
//    Define Static Map
//==============================================================================

#define _SCHEMA_BODY_DefineStaticMapElement_(ClassName, RootClass, ClassSuffix, \
    ClassSmallSuffix, ...)                                                      \
    { APP_EXPAND(TO_STR(ClassSuffix##Type_##ClassName)),                        \
        ClassSuffix##Type_##ClassName},                                         \

#define _SCHEMA_BODY_DefineStaticMapElement(ClassName, vars, ...)               \
_SCHEMA_HEADER_StripParenAndExpand(_SCHEMA_BODY_DefineStaticMapElement_,        \
    ClassName, _SCHEMA_HEADER_StripParen vars, __VA_ARGS__)


#define _SCHEMA_BODY_DefineMsgTypArray(y, x, ...) APP_EXPAND(TO_STR(y)),

#define _SCHEMA_BODY_DefineStaticMap(RootClass, ClassSuffix,                    \
    ClassSmallSuffix, Definition)                                               \
static std::map<tString, tUint8> _##RootClass##_##ClassSmallSuffix##TypeMap = { \
    Definition(_SCHEMA_BODY_DefineStaticMapElement, (RootClass,                 \
        ClassSuffix, ClassSmallSuffix))                                         \
};                                                                              \
static tString _##RootClass##_##ClassSmallSuffix##TypeArray[] = {               \
    "Invalid",                                                                  \
    Definition(_SCHEMA_BODY_DefineMsgTypArray, ClassSuffix)                     \
};

//==============================================================================
//    Define Global Inflate function
//==============================================================================

#define _SCHEMA_BODY_DefineInflate_(ClassName, RootClass, ClassSuffix,          \
    ClassSmallSuffix, ...)                                                      \
    case ClassSuffix##Type_##ClassName:                                         \
    {                                                                           \
        auto tmp##ClassSuffix = New##ClassName##ClassSuffix##Ptr();             \
        deserializer->Deserialize(#ClassName, tmp##ClassSuffix);                \
        ClassSmallSuffix = tmp##ClassSuffix;                                    \
    }                                                                           \
    break;                                                                      \

#define _SCHEMA_BODY_DefineInflate(ClassName, vars, ...)                        \
    _SCHEMA_HEADER_StripParenAndExpand(_SCHEMA_BODY_DefineInflate_, ClassName,  \
        _SCHEMA_HEADER_StripParen vars, __VA_ARGS__)


#define _SCHEMA_BODY_DefineFromPinggyValue_(ClassName, RootClass, ClassSuffix,  \
    ClassSmallSuffix, ...)                                                      \
    case ClassSuffix##Type_##ClassName:                                         \
    {                                                                           \
        auto tmp##ClassSuffix = New##ClassName##ClassSuffix##Ptr();             \
        pv.GetTo(#ClassName, tmp##ClassSuffix);                                 \
        ClassSmallSuffix = tmp##ClassSuffix;                                    \
    }                                                                           \
    break;                                                                      \

#define _SCHEMA_BODY_DefineFromPinggyValue(ClassName, vars, ...)                \
    _SCHEMA_HEADER_StripParenAndExpand(_SCHEMA_BODY_DefineFromPinggyValue_,     \
        ClassName, _SCHEMA_HEADER_StripParen vars, __VA_ARGS__)

#define _SCHEMA_BODY_DefineInflateFunction(RootClass, ClassSuffix,              \
    ClassSmallSuffix, Definition)                                               \
void Inflate(DeserializerPtr deserializer,                                      \
    RootClass##ClassSuffix##Ptr &ClassSmallSuffix)                              \
{                                                                               \
    tUint8 ClassSmallSuffix##Type = ClassSuffix##Type_Invalid;                  \
    bool found = false;                                                         \
    deserializer->Deserialize(APP_EXPAND(TO_STR(ClassSmallSuffix##Type)),       \
        ClassSmallSuffix##Type);                                                \
                                                                                \
    if (ClassSmallSuffix##Type > ClassSuffix##Type_Invalid                      \
            && ClassSmallSuffix##Type < ClassSuffix##Type_Count) {              \
        if (deserializer->HasChild(                                             \
            _##RootClass##_##ClassSmallSuffix##TypeArray[                       \
                ClassSmallSuffix##Type]))                                       \
            found = true;                                                       \
    }                                                                           \
                                                                                \
    if (!found) {                                                               \
        for(tUint8 t = ClassSuffix##Type_Invalid;                               \
            t < ClassSuffix##Type_Count; t++) {                                 \
            if (deserializer->HasChild(                                         \
                _##RootClass##_##ClassSmallSuffix##TypeArray[t])) {             \
                found = true;                                                   \
                ClassSmallSuffix##Type = t;                                     \
                break;                                                          \
            }                                                                   \
        }                                                                       \
    }                                                                           \
                                                                                \
    if (!found) {                                                               \
        throw RootClass##ClassSuffix##DeserializationException(                 \
            APP_EXPAND(TO_STR(ClassSmallSuffix##Type)) " not found");           \
        return;                                                                 \
    }                                                                           \
                                                                                \
    switch(ClassSmallSuffix##Type) {                                            \
    Definition(_SCHEMA_BODY_DefineInflate, (RootClass,                          \
        ClassSuffix, ClassSmallSuffix))                                         \
    default:                                                                    \
        throw RootClass##ClassSuffix##DeserializationException(                 \
            "Unknown " APP_EXPAND(TO_STR(ClassSmallSuffix##Type)) );            \
        return;                                                                 \
    }                                                                           \
}                                                                               \
                                                                                \
void                                                                            \
FromPinggyValue(PinggyValue &pv, RootClass##ClassSuffix##Ptr &ClassSmallSuffix) \
{                                                                               \
    tUint8 ClassSmallSuffix##Type = ClassSuffix##Type_Invalid;                  \
    bool found = false;                                                         \
    pv.GetTo(APP_EXPAND(TO_STR(ClassSmallSuffix##Type)),                        \
        ClassSmallSuffix##Type);                                                \
                                                                                \
    if (ClassSmallSuffix##Type > ClassSuffix##Type_Invalid                      \
            && ClassSmallSuffix##Type < ClassSuffix##Type_Count) {              \
        if (pv.HasChildWithKey(                                                 \
            _##RootClass##_##ClassSmallSuffix##TypeArray[                       \
                ClassSmallSuffix##Type]))                                       \
            found = true;                                                       \
    }                                                                           \
                                                                                \
    if (!found) {                                                               \
        for(tUint8 t = ClassSuffix##Type_Invalid;                               \
            t < ClassSuffix##Type_Count; t++) {                                 \
            if (pv.HasChildWithKey(                                             \
                _##RootClass##_##ClassSmallSuffix##TypeArray[t])) {             \
                found = true;                                                   \
                ClassSmallSuffix##Type = t;                                     \
                break;                                                          \
            }                                                                   \
        }                                                                       \
    }                                                                           \
                                                                                \
    if (!found) {                                                               \
        throw RootClass##ClassSuffix##DeserializationException(                 \
            APP_EXPAND(TO_STR(ClassSmallSuffix##Type)) " not found");           \
        return;                                                                 \
    }                                                                           \
                                                                                \
    switch(ClassSmallSuffix##Type) {                                            \
    Definition(_SCHEMA_BODY_DefineFromPinggyValue, (RootClass,                  \
        ClassSuffix, ClassSmallSuffix))                                         \
    default:                                                                    \
        throw RootClass##ClassSuffix##DeserializationException(                 \
            "Unknown " APP_EXPAND(TO_STR(ClassSmallSuffix##Type)) );            \
        return;                                                                 \
    }                                                                           \
}


//==============================================================================
//    Define Global Deflate function
//==============================================================================

#define _SCHEMA_BODY_DefineDeflate_(ClassName, RootClass,                       \
    ClassSuffix, ClassSmallSuffix, ...)                                         \
    case ClassSuffix##Type_##ClassName:                                         \
    {                                                                           \
        serializer->Serialize(APP_EXPAND(TO_STR(ClassSmallSuffix##Type)),       \
            (uint8_t)ClassSuffix##Type_##ClassName);                            \
        ClassName##ClassSuffix##Ptr tmp##ClassSuffix =                          \
            ClassSmallSuffix->DynamicPointerCast<ClassName##ClassSuffix>();     \
        serializer->Serialize(#ClassName, tmp##ClassSuffix);                    \
    }                                                                           \
    break;                                                                      \

#define _SCHEMA_BODY_DefineDeflate(ClassName, vars, ...)                        \
    _SCHEMA_HEADER_StripParenAndExpand(_SCHEMA_BODY_DefineDeflate_, ClassName,  \
        _SCHEMA_HEADER_StripParen vars, __VA_ARGS__)

//==================

#define _SCHEMA_BODY_DefineToPinggyValue_(ClassName, RootClass,                 \
    ClassSuffix, ClassSmallSuffix, ...)                                         \
    case ClassSuffix##Type_##ClassName:                                         \
    {                                                                           \
        pv.SetFrom(APP_EXPAND(TO_STR(ClassSmallSuffix##Type)),                  \
            (uint8_t)ClassSuffix##Type_##ClassName);                            \
        ClassName##ClassSuffix##Ptr tmp##ClassSuffix =                          \
            ClassSmallSuffix->DynamicPointerCast<ClassName##ClassSuffix>();     \
        pv.SetFrom(#ClassName, tmp##ClassSuffix);                               \
    }                                                                           \
    break;                                                                      \

#define _SCHEMA_BODY_DefineToPinggyValue(ClassName, vars, ...)                  \
    _SCHEMA_HEADER_StripParenAndExpand(_SCHEMA_BODY_DefineToPinggyValue_,       \
        ClassName, _SCHEMA_HEADER_StripParen vars, __VA_ARGS__)
//==================

#define _SCHEMA_BODY_DefineDeflateFunction(RootClass, ClassSuffix,              \
    ClassSmallSuffix, Definition)                                               \
void Deflate(SerializerPtr serializer,                                          \
    RootClass##ClassSuffix##Ptr ClassSmallSuffix)                               \
{                                                                               \
    switch(ClassSmallSuffix->ClassSmallSuffix##Type) {                          \
Definition(_SCHEMA_BODY_DefineDeflate, (RootClass,                              \
    ClassSuffix, ClassSmallSuffix))                                             \
    default:                                                                    \
        throw RootClass##ClassSuffix##SerializationException(                   \
            "Unknown " APP_EXPAND(TO_STR(ClassSmallSuffix##Type)) );            \
        return;                                                                 \
    }                                                                           \
}                                                                               \
void ToPinggyValue(PinggyValue &pv,                                             \
    const RootClass##ClassSuffix##Ptr &ClassSmallSuffix)                        \
{                                                                               \
    switch(ClassSmallSuffix->ClassSmallSuffix##Type) {                          \
Definition(_SCHEMA_BODY_DefineToPinggyValue, (RootClass,                        \
    ClassSuffix, ClassSmallSuffix))                                             \
    default:                                                                    \
        throw RootClass##ClassSuffix##SerializationException(                   \
            "Unknown " APP_EXPAND(TO_STR(ClassSmallSuffix##Type)) );            \
        return;                                                                 \
    }                                                                           \
}                                                                               \

#define _SCHEMA_BODY_DefineMsgTypeStr(y, x, ...)                                \
    APP_CONVERT_TO_STRING(x##Type_##y),

//==============================================================================

#define SCHEMA_BODY__DEFINE_BODIES(RootClass, ClassSuffix,                      \
                                        ClassSmallSuffix, Definition)           \
        _SCHEMA_BODY_DefineExceptionClasses(RootClass, ClassSuffix)             \
        Definition(_SCHEMA_BODY_DefineMsgConstructor, (RootClass, ClassSuffix)) \
        Definition(_SCHEMA_BODY_DefineProtocolFunctions, (RootClass,            \
            ClassSuffix, ClassSmallSuffix))                                     \
        _SCHEMA_BODY_DefineStaticMap(RootClass, ClassSuffix,                    \
            ClassSmallSuffix, Definition)                                       \
        _SCHEMA_BODY_DefineInflateFunction(RootClass, ClassSuffix,              \
            ClassSmallSuffix, Definition)                                       \
        _SCHEMA_BODY_DefineDeflateFunction(RootClass, ClassSuffix,              \
            ClassSmallSuffix, Definition)                                       \
        tString RootClass##ClassSuffix::ClassSuffix##Type##Str[] = {"Invalid",  \
            Definition(_SCHEMA_BODY_DefineMsgTypeStr, ClassSuffix) "All"};

//==============================================================================

#define _TRANSPORT_VAR_SERIALIZER_PTR_(x, y)                                    \
    if (objPtr)                                                                 \
        serializer->Serialize(#y, objPtr->x);

#define _TRANSPORT_VAR_DESERIALIZER_PTR_(x, y)                                  \
    deserializer->Deserialize(#y, objPtr->x);

#define _TRANSPORT_VAR_SERIALIZER_PTR_v1(x)                                     \
    _TRANSPORT_VAR_SERIALIZER_PTR_(x, x)

#define _TRANSPORT_VAR_DESERIALIZER_PTR_v1(x)                                   \
    _TRANSPORT_VAR_DESERIALIZER_PTR_(x, x)

#define _TRANSPORT_VAR_SERIALIZER_PTR_v2(x)                                     \
    _TRANSPORT_VAR_SERIALIZER_PTR_ x

#define _TRANSPORT_VAR_DESERIALIZER_PTR_v2(x)                                   \
    _TRANSPORT_VAR_DESERIALIZER_PTR_ x

#define _TRANSPORT_VAR_TOPINGGY_PTR_(x, y)                                      \
    if (objPtr)                                                                 \
        pv.SetFrom(#y, objPtr->x);

#define _TRANSPORT_VAR_TOPINGGY_PTR_v1(x)                                       \
    _TRANSPORT_VAR_TOPINGGY_PTR_(x, x)

#define _TRANSPORT_VAR_TOPINGGY_PTR_v2(x)                                       \
    _TRANSPORT_VAR_TOPINGGY_PTR_ x

#define _TRANSPORT_VAR_FROMPINGGY_PTR_(x, y)                                    \
    pv.GetTo(#y, objPtr->x);

#define _TRANSPORT_VAR_FROMPINGGY_PTR_v1(x)                                     \
    _TRANSPORT_VAR_FROMPINGGY_PTR_(x, x)

#define _TRANSPORT_VAR_FROMPINGGY_PTR_v2(x)                                     \
    _TRANSPORT_VAR_FROMPINGGY_PTR_ x

#define DEFINE_TRANSPORT_SERIALIZER_DESERIALIZER_PTR_V2(cls, ...)               \
static void                                                                     \
Deflate(SerializerPtr serializer, cls##Ptr objPtr)                              \
{                                                                               \
    APP_MACRO_FOR_EACH_FORNT(_TRANSPORT_VAR_SERIALIZER_PTR_v2, __VA_ARGS__)     \
}                                                                               \
static void                                                                     \
ToPinggyValue(PinggyValue &pv, const cls##Ptr &objPtr)                          \
{                                                                               \
    APP_MACRO_FOR_EACH_FORNT(_TRANSPORT_VAR_TOPINGGY_PTR_v2, __VA_ARGS__)       \
}                                                                               \
static void                                                                     \
Inflate(DeserializerPtr deserializer, cls##Ptr &objPtr)                         \
{                                                                               \
    objPtr = New##cls##Ptr();                                                   \
    APP_MACRO_FOR_EACH_FORNT(_TRANSPORT_VAR_DESERIALIZER_PTR_v2, __VA_ARGS__)   \
}                                                                               \
static void                                                                     \
FromPinggyValue(PinggyValue &pv, cls##Ptr &objPtr)                              \
{                                                                               \
    objPtr = New##cls##Ptr();                                                   \
    APP_MACRO_FOR_EACH_FORNT(_TRANSPORT_VAR_FROMPINGGY_PTR_v2, __VA_ARGS__)     \
}

#define DEFINE_TRANSPORT_SERIALIZER_DESERIALIZER_PTR_V1(cls, ...)               \
static void                                                                     \
Deflate(SerializerPtr serializer, cls##Ptr objPtr)                              \
{                                                                               \
    APP_MACRO_FOR_EACH_FORNT(_TRANSPORT_VAR_SERIALIZER_PTR_v1, __VA_ARGS__)     \
}                                                                               \
static void                                                                     \
ToPinggyValue(PinggyValue &pv, const cls##Ptr &objPtr)                          \
{                                                                               \
    APP_MACRO_FOR_EACH_FORNT(_TRANSPORT_VAR_TOPINGGY_PTR_v1, __VA_ARGS__)       \
}                                                                               \
static void                                                                     \
Inflate(DeserializerPtr deserializer, cls##Ptr &objPtr)                         \
{                                                                               \
    objPtr = New##cls##Ptr();                                                   \
    APP_MACRO_FOR_EACH_FORNT(_TRANSPORT_VAR_DESERIALIZER_PTR_v1, __VA_ARGS__)   \
}                                                                               \
static void                                                                     \
FromPinggyValue(PinggyValue &pv, cls##Ptr &objPtr)                              \
{                                                                               \
    objPtr = New##cls##Ptr();                                                   \
    APP_MACRO_FOR_EACH_FORNT(_TRANSPORT_VAR_FROMPINGGY_PTR_v1, __VA_ARGS__)     \
}


#define DEFINE_TRANSPORT_SERIALIZER_DESERIALIZER_ENUM_CAST(type, cast, def)     \
static void                                                                     \
Deflate(SerializerPtr serializer, type mode)                                    \
{                                                                               \
    serializer->Serialize("v", (castTo)mode);                                   \
}                                                                               \
static void                                                                     \
ToPinggyValue(PinggyValue &pv, const type &mode)                                \
{                                                                               \
    pv.ToPinggyValue((castTo)mode);                                             \
}                                                                               \
static void                                                                     \
Inflate(DeserializerPtr deserializer, type &mode)                               \
{                                                                               \
    castTo v = def;                                                             \
    deserializer->Deserialize("v", v);                                          \
    mode = (type)v;                                                             \
}                                                                               \
static void                                                                     \
FromPinggyValue(PinggyValue &pv, type &mode)                                    \
{                                                                               \
    castTo v = def;                                                             \
    if (pv.HasChildWithKey("v"))                                                \
        pv.GetTo("v", v);                                                       \
    else                                                                        \
        pv.GetTo(v);                                                            \
    mode = (type)v;                                                             \
}


#define DEFINE_TRANSPORT_SERIALIZER_DESERIALIZER_ENUM_TYPE(type, type2, ...)    \
inline void                                                                     \
Deflate(SerializerPtr serializer, type e)                                       \
{                                                                               \
    static_assert(std::is_enum<type>::value, TO_STR(type) " must be an enum!"); \
    static const std::pair<type, type2> m[] = __VA_ARGS__;                      \
    auto it = std::find_if(std::begin(m), std::end(m),                          \
                        [e](const std::pair<type, type2> &ej_pair) ->           \
                            bool { return ej_pair.first == e; });               \
    auto v = ((it != std::end(m)) ? it : std::begin(m))->second;                \
    serializer->Serialize("v", v);                                              \
}                                                                               \
inline void                                                                     \
ToPinggyValue(PinggyValue &pv, const type &e)                                   \
{                                                                               \
    static_assert(std::is_enum<type>::value, TO_STR(type) " must be an enum!"); \
    static const std::pair<type, type2> m[] = __VA_ARGS__;                      \
    auto it = std::find_if(std::begin(m), std::end(m),                          \
                        [e](const std::pair<type, type2> &ej_pair) ->           \
                            bool { return ej_pair.first == e; });               \
    auto v = ((it != std::end(m)) ? it : std::begin(m))->second;                \
    pv.SetFrom(v);                                                              \
}                                                                               \
inline void                                                                     \
Inflate(DeserializerPtr deserializer, type &e)                                  \
{                                                                               \
    static_assert(std::is_enum<type>::value, TO_STR(type) " must be an enum!"); \
    static const std::pair<type, type2> m[] = __VA_ARGS__;                      \
    type2 v = std::begin(m)->second;                                            \
    deserializer->Deserialize("v", v);                                          \
    auto it = std::find_if(std::begin(m), std::end(m),                          \
                        [v](const std::pair<type, type2> &ej_pair) ->           \
                            bool { return ej_pair.second == v; });              \
    e = ((it != std::end(m)) ? it : std::begin(m))->first;                      \
}                                                                               \
inline void                                                                     \
FromPinggyValue(PinggyValue &pv, type &e)                                       \
{                                                                               \
    static_assert(std::is_enum<type>::value, TO_STR(type) " must be an enum!"); \
    static const std::pair<type, type2> m[] = __VA_ARGS__;                      \
    type2 v = std::begin(m)->second;                                            \
    if (pv.HasChildWithKey("v"))                                                \
        pv.GetTo("v", v);                                                       \
    else                                                                        \
        pv.GetTo(v);                                                            \
    auto it = std::find_if(std::begin(m), std::end(m),                          \
                        [v](const std::pair<type, type2> &ej_pair) ->           \
                            bool { return ej_pair.second == v; });              \
    e = ((it != std::end(m)) ? it : std::begin(m))->first;                      \
}


//==============================================================================

#define _SCHEMA_BODY_HandleIncomingMsg_(ClassName, RootClass,                   \
    Suffix, SuffixSmall, ...)                                                   \
    case Suffix##Type_##ClassName:                                              \
        {                                                                       \
            auto SuffixSmall =                                                  \
                tmp##Suffix->DynamicPointerCast<ClassName##Suffix>();           \
            eventHandler->Handle##ClassName##Suffix(thisPtr, SuffixSmall);      \
        }                                                                       \
        break;

#define _SCHEMA_BODY_HandleIncomingMsg(x, vars, ...)                            \
    _SCHEMA_HEADER_StripParenAndExpand(_SCHEMA_BODY_HandleIncomingMsg_, x,      \
        _SCHEMA_HEADER_StripParen vars, __VA_ARGS__)



#define DEFINE_HANDLING_CLASS(HandlingClassName, RootClass, ClassSuffix,        \
                                        ClassSmallSuffix, Definition)           \
HandlingClassName::HandlingClassName(net::NetworkConnectionPtr netConn,         \
    HandlingClassName##EventHandlerPtr handlerPtr):                             \
    netConn(netConn), eventHandler(handlerPtr), running(false),                 \
    pinggyValueMode(false)                                                      \
{                                                                               \
    netConn->SetBlocking(false);                                                \
}                                                                               \
                                                                                \
HandlingClassName::~HandlingClassName()                                         \
{                                                                               \
    if (transportManager) {                                                     \
        transportManager->EndTransport();                                       \
    }                                                                           \
    if (netConn) {                                                              \
        netConn->DeregisterFDEvenHandler();                                     \
        netConn->CloseConn();                                                   \
    }                                                                           \
}                                                                               \
                                                                                \
void HandlingClassName::EnablePinggyValueMode(bool enable) {                    \
    pinggyValueMode = enable;                                                   \
    if (transportManager) {                                                     \
        transportManager->EnablePinggyValueMode(enable);                        \
    }                                                                           \
}                                                                               \
                                                                                \
bool HandlingClassName::Start(bool handshakeRequired) {                         \
    transportManager = NewTransportManagerPtr(netConn, thisPtr, false,          \
        handshakeRequired);                                                     \
    netConn->RegisterFDEvenHandler(transportManager);                           \
    running = true;                                                             \
    if (pinggyValueMode) transportManager->EnablePinggyValueMode(true);         \
    return true;                                                                \
}                                                                               \
                                                                                \
bool HandlingClassName::Stop() {                                                \
    if (transportManager) {                                                     \
        transportManager->EndTransport();                                       \
        transportManager = nullptr;                                             \
    }                                                                           \
    if (netConn) {                                                              \
        netConn->CloseConn();                                                   \
        netConn = nullptr;                                                      \
    }                                                                           \
    running = false;                                                            \
    return true;                                                                \
}                                                                               \
                                                                                \
bool HandlingClassName::Send##ClassSuffix(RootClass##ClassSuffix##Ptr           \
    ClassSmallSuffix, bool queue)                                               \
{                                                                               \
    bool success = false;                                                       \
    if (sendQueue.empty()) {                                                    \
        try {                                                                   \
            if (pinggyValueMode) {                                              \
                PinggyValue pv;                                                 \
                pv.SetFrom(TO_STR(ClassSmallSuffix), ClassSmallSuffix);         \
                success = transportManager->SendMsg(pv);                        \
            } else {                                                            \
                success = transportManager->GetSerializer()->Serialize(         \
                    TO_STR(ClassSmallSuffix),  ClassSmallSuffix)->Send();       \
            }                                                                   \
        } catch(RootClass##ClassSuffix##SerializationException &e){             \
            LOGE("Exception occurred: ", e.what());                             \
            Stop();                                                             \
            return false;                                                       \
        }                                                                       \
    }                                                                           \
    if (!success && queue) {                                                    \
        sendQueue.push(ClassSmallSuffix);                                       \
        return true;                                                            \
    }                                                                           \
    return success;                                                             \
}                                                                               \
                                                                                \
void HandlingClassName::HandleConnectionReset(net::NetworkConnectionPtr         \
    netConn)                                                                    \
{                                                                               \
    if (eventHandler) {                                                         \
        eventHandler->Handle##RootClass##ConnectionReset(thisPtr);              \
    }                                                                           \
    if (netConn) {                                                              \
        netConn->DeregisterFDEvenHandler();                                     \
        netConn->CloseConn();                                                   \
        netConn = nullptr;                                                      \
    }                                                                           \
    transportManager = nullptr;                                                 \
    eventHandler = nullptr;                                                     \
    running = false;                                                            \
}                                                                               \
                                                                                \
void HandlingClassName::HandleIncomingDeserialize(DeserializerPtr deserializer) \
{                                                                               \
    RootClass##ClassSuffix##Ptr tmp##ClassSuffix;                               \
    try {                                                                       \
        deserializer->Deserialize(TO_STR(ClassSmallSuffix), tmp##ClassSuffix);  \
    } catch(RootClass##ClassSuffix##DeserializationException &e){               \
        LOGE("Exception occurred: ", e.what());                                 \
        Stop();                                                                 \
        return;                                                                 \
    }                                                                           \
    if (!tmp##ClassSuffix) {                                                    \
        Stop();                                                                 \
        return;                                                                 \
    }                                                                           \
    switch(tmp##ClassSuffix->ClassSmallSuffix##Type) {                          \
    Definition(_SCHEMA_BODY_HandleIncomingMsg,                                  \
        (RootClass, ClassSuffix, ClassSmallSuffix))                             \
        default:                                                                \
            LOGE("Unknown  " APP_EXPAND(TO_STR(ClassSmallSuffix##Type))         \
                ". Stoping...");                                                \
            Stop();                                                             \
            return;                                                             \
    }                                                                           \
}                                                                               \
                                                                                \
void HandlingClassName::HandleIncomingPinggyValue(PinggyValue &pv)              \
{                                                                               \
    RootClass##ClassSuffix##Ptr tmp##ClassSuffix;                               \
    try {                                                                       \
        pv.GetTo(TO_STR(ClassSmallSuffix), tmp##ClassSuffix);                   \
    } catch(RootClass##ClassSuffix##DeserializationException &e){               \
        LOGE("Exception occurred: ", e.what());                                 \
        Stop();                                                                 \
        return;                                                                 \
    }                                                                           \
    if (!tmp##ClassSuffix) {                                                    \
        Stop();                                                                 \
        return;                                                                 \
    }                                                                           \
    switch(tmp##ClassSuffix->ClassSmallSuffix##Type) {                          \
    Definition(_SCHEMA_BODY_HandleIncomingMsg,                                  \
        (RootClass, ClassSuffix, ClassSmallSuffix))                             \
        default:                                                                \
            LOGE("Unknown  " APP_EXPAND(TO_STR(ClassSmallSuffix##Type))         \
                ". Stoping...");                                                \
            Stop();                                                             \
            return;                                                             \
    }                                                                           \
}                                                                               \
                                                                                \
void HandlingClassName::HandleReadyToSendBuffer()                               \
{                                                                               \
    while (!sendQueue.empty()) {                                                \
        auto ClassSmallSuffix = sendQueue.front();                              \
        auto success = false;                                                   \
        try {                                                                   \
            success = transportManager->GetSerializer()->Serialize(             \
                TO_STR(ClassSmallSuffix), ClassSmallSuffix)->Send();            \
        } catch(RootClass##ClassSuffix##SerializationException &e){             \
            LOGE("Exception occurred: ", e.what());                             \
            Stop();                                                             \
            return;                                                             \
        }                                                                       \
        if (!success) {                                                         \
            break;                                                              \
        }                                                                       \
        sendQueue.pop();                                                        \
    }                                                                           \
}                                                                               \
                                                                                \
void HandlingClassName::HandleIncompleteHandshake()                             \
{                                                                               \
    LOGE("Something fishy. Cannot complete handshake");                         \
    Stop();                                                                     \
}


#endif // SRC_CPP_PROTOCOL_TRANSPORT_SCHEMABODYGENERATOR_HH__
