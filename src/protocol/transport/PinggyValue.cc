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

#include "PinggyValue.hh"
#include <sstream>
#include <stdexcept>


//============
//   GETTER
//============

template<typename L, typename R>
bool
checkIfAssignmentPossibleAndAssign(L &l, R r) {
    if constexpr (std::is_signed_v<L> == std::is_signed_v<R>) {
        if (r >= std::numeric_limits<L>::min() && r <= std::numeric_limits<L>::max()) {
            l = static_cast<L>(r);
            return true;
        }
    } else if constexpr (std::is_signed_v<R>) { //signed -> unsinged
        if (r >= 0 && static_cast<std::make_unsigned_t<R>>(r) <= std::numeric_limits<L>::max()) {
            l = static_cast<L>(r);
            return true;
        }
    } else {
        if (r <= static_cast<std::make_unsigned_t<L>>(std::numeric_limits<L>::max())) {
            l = static_cast<L>(r);
            return true;
        }
    }
    return false;
}

#define FOR_EACH_FRONT_checkIfAssignmentPossibleAndAssign(L, R) \
inline bool \
checkIfAssignmentPossibleAndAssign(t##L &l, t##R r) { l = r; return true; }

#define Define_checkIfAssignmentPossibleAndAssign_Func(x, ...) \
APP_MACRO_FOR_EACH_WITH_ARG_FORNT(FOR_EACH_FRONT_checkIfAssignmentPossibleAndAssign, x, __VA_ARGS__)

LiteralCompatibility(Define_checkIfAssignmentPossibleAndAssign_Func)

//==
template<typename L, typename R>
bool
checkIfCompatible(L l, R r) { return false; }

#define FOR_EACH_FRONT_checkIfCompatible(L, R) \
inline bool \
checkIfCompatible(t##L &l, t##R r) { return true; }

#define Define_checkIfCompatible_Func(x, ...) \
APP_MACRO_FOR_EACH_WITH_ARG_FORNT(FOR_EACH_FRONT_checkIfCompatible, x, __VA_ARGS__)
LiteralCompatibility(Define_checkIfCompatible_Func)

//===================
//===================
#define SwitchForEachType(x, y, z) \
    case ValueType_##x: { \
        auto ptr = static_cast<PinggyInternalType_##x *>(this); \
        y(ptr, z) \
    } \
    break;
#define AllSwitched(typ, func)                                              \
        switch (type)                                                       \
        {                                                                   \
            FOREACH_ALL_TYPE_2(SwitchForEachType, func, typ)                \
        default:                                                            \
            throw std::bad_cast();                                          \
        }                                                                   \

//===================
// PinggyInternalType
//===================

#if 1

#define DefineAllGetter(typ)                                                \
    t##typ                                                                  \
    PinggyValue::PinggyInternalType::Get##typ()                                          \
    {                                                                       \
        throw std::bad_cast();                                              \
        return typ##_Default;                                               \
    }

FOREACH_ALL_TYPE(DefineAllGetter)
#undef DefineAllGetter

size_t
PinggyValue::PinggyInternalType::Size()
{
    return 0;
}
void
PinggyValue::PinggyInternalType::JsonDump(std::ostream &os)
{
    throw std::bad_cast();
}

tString
PinggyValue::PinggyInternalType::JsonString()
{
    std::stringstream os;
    JsonDump(os);
    return os.str();
}

#endif

//===================
// PinggyInternalType X
//===================

#if 1

#define DefineDumpFunc1(x) \
void \
PinggyValue::PinggyInternalType_ ## x :: JsonDump(std::ostream &os) \
{ \
    os << value; \
}

#define DefineDumpFunc2(x) \
void \
PinggyValue::PinggyInternalType_ ## x :: JsonDump(std::ostream &os) \
{ \
    os << (int)value; \
}


DefineDumpFunc2(Int8)
DefineDumpFunc1(Int16)
DefineDumpFunc1(Int32)
DefineDumpFunc1(Int64)
DefineDumpFunc2(Uint8)
DefineDumpFunc1(Uint16)
DefineDumpFunc1(Uint32)
DefineDumpFunc1(Uint64)
DefineDumpFunc1(Float32)
DefineDumpFunc1(Float64)

void PinggyValue::PinggyInternalType_String :: JsonDump(std::ostream &os) { os << "\"" << value << "\""; }
void PinggyValue::PinggyInternalType_Raw :: JsonDump(std::ostream &os) { os << "\"<Raw of Len:"<< value->Len << ">\""; }


#define DefineSizeFunc(x) \
size_t \
PinggyValue::PinggyInternalType_ ## x :: Size() \
{ \
    return 1; \
}
FOREACH_LITERALS_TYPE(DefineSizeFunc);


size_t
PinggyValue::PinggyInternalType_String::Size()
{
    return value.length();
}

size_t
PinggyValue::PinggyInternalType_Raw::Size()
{
    return value->Len;
}

#define DefineGetValueType(x)                               \
tValueType                                                  \
PinggyValue::PinggyInternalType_##x ::GetValueType()        \
{                                                           \
    return ValueType_##x;                                   \
}
FOREACH_ALL_TYPE(DefineGetValueType)
#undef DefineGetValueType


#define DefineGetterForEachType(y, x)                           \
    t##x                                                        \
    PinggyValue::PinggyInternalType_##y ::Get##x()              \
    {                                                           \
        t##x r = x##_Default;                                   \
        if (checkIfCompatible(r, value))                        \
            r = value;                                          \
        else if (!checkIfAssignmentPossibleAndAssign(r, value)) \
            throw std::bad_cast();                              \
        return r;                                               \
    }                                                           \

#define DefineGetterForAllowedTypes(x, ...) \
    APP_MACRO_FOR_EACH_WITH_ARG_FORNT(DefineGetterForEachType, x, __VA_ARGS__)

#define DefineGetterFuncLiteral(x, ...) \
    DefineGetterForAllowedTypes(x, __VA_ARGS__)

LiteralCasting(DefineGetterFuncLiteral)

#endif


//===================
// PinggyInternalType_Array
//===================

#if 1
PinggyValue::PinggyInternalType_Array::~PinggyInternalType_Array()
{
    for (auto ele : value) {
        delete ele;
    }
}

void
PinggyValue::PinggyInternalType_Array::JsonDump(std::ostream &os)
{
    os << "[";
    bool comma = false;
    for (auto ele : value) {
        if (comma)
            os << ", ";
        ele->JsonDump(os);
        comma = true;
    }
    os << "]";
}

tValueType
PinggyValue::PinggyInternalType_Array ::GetValueType()
{
    return ValueType_Array;
}

#endif

//===================
// PinggyInternalType_Object
//===================

#if 1
PinggyValue::PinggyInternalType_Object::~PinggyInternalType_Object()
{
    for (auto ele : value) {
        delete ele.second;
    }
}

void
PinggyValue::PinggyInternalType_Object::JsonDump(std::ostream &os)
{
    os << "{";
    bool comma = false;
    for (auto ele : value) {
        if (comma)
            os << ", ";
        os << "\"" << ele.first << "\":";
        ele.second->JsonDump(os);
        comma = true;
    }
    os << "}";
}

#define DefineSetFrom(x)                                                        \
    void                                                                        \
    PinggyValue::PinggyInternalType_Object::SetFrom(tString key, const t##x &v) \
    {                                                                           \
        value[key] = NewPinggyInternalType_##x##Ptr(v);                         \
    }
FOREACH_ALL_TYPE(DefineSetFrom)
#undef DefineSetFrom


void
PinggyValue::PinggyInternalType_Object::SetFrom(tString key, const tCChar &v)
{
    value[key] = NewPinggyInternalType_StringPtr(tString(v));
}

void
PinggyValue::PinggyInternalType_Object::Set(tString key, PinggyInternalTypePtr ptr)
{
    value[key] = ptr;
}

tValueType
PinggyValue::PinggyInternalType_Object ::GetValueType()
{
    return ValueType_Object;
}

#endif


//===================
// PinggyValue
//===================

#define DefineGetter(x)             \
    t##x                            \
    PinggyValue::Get##x()           \
    {                               \
        if (self)                   \
            return self->Get##x();  \
        throw std::bad_cast();      \
        return x##_Default;         \
    }
FOREACH_ALL_TYPE(DefineGetter)
#undef DefineGetter

#define DefineGetTo(x)              \
    void                            \
    PinggyValue::GetTo(t##x &v)     \
    {                               \
        v = Get##x();               \
    }
FOREACH_ALL_TYPE(DefineGetTo)
#undef DefineGetTo

#define DefineSetFrom(x)                                            \
    void                                                            \
    PinggyValue::SetFrom(const t##x &v)                             \
    {                                                               \
        if (self)                                                   \
        {                                                           \
            throw std::invalid_argument("Already assigned value");  \
        }                                                           \
        self = PinggyValue::NewPinggyInternalType_##x##Ptr(v);      \
    }
FOREACH_ALL_TYPE(DefineSetFrom)
#undef DefineSetFrom

void
PinggyValue::SetFrom(const tCChar &v)
{
    if (self) {
        throw std::invalid_argument("Already assigned value");
    }
    self = PinggyValue::NewPinggyInternalType_StringPtr(tString(v));
}

size_t
PinggyValue::Size()
{
    if (self)
        return self->Size();
    return 0;
}

void
PinggyValue::JsonDump(std::ostream &os)
{
    if (self)
        return self->JsonDump(os);
}

tString
PinggyValue::JsonString()
{
    if (self)
        return self->JsonString();
    return "";
}

tValueType
PinggyValue::GetValueType()
{
    if (self)
        return self->GetValueType();
    return ValueType_Invalid;
}

bool
PinggyValue::HasChildWithKey(tString key)
{
    if (!self)
        return false;

    auto ptr = dynamic_cast<PinggyInternalType_ObjectPtr>(self);
    if (!ptr)
        return false;

    return ptr->HasChildWithKey(key);
}

void
PinggyValue::cleanUp()
{
    if (self) {
        delete self;
        self = nullptr;
    }
}
