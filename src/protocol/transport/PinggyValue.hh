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

#ifndef SRC_CPP_PINGGYTRANSPORT_PINGGYVALUE_HH_
#define SRC_CPP_PINGGYTRANSPORT_PINGGYVALUE_HH_

#include "TransportCommon.hh"
#include <map>
#include <vector>
#include <variant>
#include <stdexcept>
#include <type_traits>

#define LiteralCasting(f)                                           \
f(Raw, Raw)                                                         \
f(String, String)                                                   \
f(Int64,  Int8, Int16, Int32, Int64, Uint8, Uint16, Uint32, Uint64) \
f(Int32,  Int8, Int16, Int32, Int64, Uint8, Uint16, Uint32, Uint64) \
f(Int16,  Int8, Int16, Int32, Int64, Uint8, Uint16, Uint32, Uint64) \
f(Int8,   Int8, Int16, Int32, Int64, Uint8, Uint16, Uint32, Uint64) \
f(Uint64, Int8, Int16, Int32, Int64, Uint8, Uint16, Uint32, Uint64) \
f(Uint32, Int8, Int16, Int32, Int64, Uint8, Uint16, Uint32, Uint64) \
f(Uint16, Int8, Int16, Int32, Int64, Uint8, Uint16, Uint32, Uint64) \
f(Uint8,  Int8, Int16, Int32, Int64, Uint8, Uint16, Uint32, Uint64) \
f(Float64, Float32, Float64)                                        \
f(Float32, Float32)                                                 \

#define LiteralCompatibility(f)             \
f(Raw, Raw)                                 \
f(String, String)                           \
f(Int64, Int8, Int16, Int32, Int64)         \
f(Int32, Int8, Int16, Int32)                \
f(Int16, Int8, Int16)                       \
f(Int8, Int8)                               \
f(Uint64, Uint8, Uint16, Uint32, Uint64)    \
f(Uint32, Uint8, Uint16, Uint32)            \
f(Uint16, Uint8, Uint16)                    \
f(Uint8, Uint8)                             \
f(Float64, Float32, Float64)                \
f(Float32, Float32)                         \


//This is a special case where we are not going to use shared_ptr for performance issue.
//Instead only pointer would work better here as shared_ptr has serious performance hit.

#define DeclareNonSharedPtr(x,name) typedef x *name;
#define DeclareClassWithNonSharedPtr(x) class x;            \
    DeclareNonSharedPtr(x, x ## Ptr);

#define DeclareStructWithNonSharedPtr(x) struct x;          \
    DeclareNonSharedPtr(x, x ## Ptr);

#define DefineStaticNewFunc_name(x, name)                   \
    template <typename ... Arguments>                       \
    inline static name                                      \
    New ## x ## Ptr(Arguments ... args)                     \
    {                                                       \
        return new x(args...);                              \
    }                                                       \
    inline static name                                      \
    New ## x ## Ptr(x *y)                                   \
    {                                                       \
        return y;                                           \
    }

#define DefineStaticNewFunc(x)                              \
    DefineStaticNewFunc_name(x, x ## Ptr)

#define DefineMakeCustomNonSharedPtr(x,name)                \
    DeclareNonSharedPtr(x, name)                            \
    DefineStaticNewFunc_name(x, name)
#define DefineMakeNonSharedPtr(x) DefineMakeCustomNonSharedPtr(x, x ## Ptr)

//===========
#if 0
class PType {
public:
    virtual
    ~PType() {}
#define DefineGetter(x) \
    virtual t##x \
    Get##x()                    { throw std::bad_cast(); return x##_Default; }
FOREACH_ALL_TYPE(DefineGetter)
#undef DefineGetter

    virtual size_t
    Size()                      { throw std::bad_cast(); return 0; }

    virtual void
    JsonDump(std::ostream &os)  { throw std::bad_cast(); }

    virtual tString
    JsonString()                { throw std::bad_cast(); return ""; }

#define DefineGetTo(x)          \
    virtual void                        \
    GetTo(t##x &v)              { throw std::bad_cast(); }

FOREACH_ALL_TYPE(DefineGetTo)
#undef DefineGetTo
};
#endif


class PinggyValue {
private:
    friend class Deserializer;
    friend class Serializer;
    DeclareClassWithNonSharedPtr(PinggyInternalType);
    PinggyInternalTypePtr       self;

    void
    cleanUp();

public:
    PinggyValue(PinggyInternalTypePtr v) : self(v)
                                { }

    PinggyValue() : self(nullptr)
                                { }

#define DefineGetter(x) \
    t##x \
    Get##x();
FOREACH_ALL_TYPE(DefineGetter)
#undef DefineGetter

    size_t
    Size();

    void
    JsonDump(std::ostream &os);

    tString
    JsonString();

    tValueType
    GetValueType();

    //=======

#define DefineGetTo(x)          \
    void                        \
    GetTo(t##x &v);

FOREACH_ALL_TYPE(DefineGetTo)
#undef DefineGetTo

    template<typename T>
    void
    GetTo(T &val);

    template<typename T>
    void
    GetTo(std::vector<T> &val);

    template<typename T>
    void
    GetTo(std::map<tString, T> &val);

    template<typename T>
    void
    GetTo(tString key, T &val);

    //=========

    template<typename T>
    void
    GetTo(tString key, T &val, const T &def);

    bool
    HasChildWithKey(tString key);

    //=========

#define DeclareSetFrom(x)           \
    void                            \
    SetFrom(const t##x &v);
FOREACH_ALL_TYPE(DeclareSetFrom)
#undef DeclareSetFrom

    void
    SetFrom(const tCChar &v);

    template<typename T>
    void
    SetFrom(const T &val);

    template<typename T>
    void
    SetFrom(const std::vector<T> &val);

    template<typename T>
    void
    SetFrom(const std::map<tString, T> &val);

    template<typename T>
    void
    SetFrom(tString key, const T &val);

private:
    //===========

    class PinggyInternalType
    {
    friend class Deserializer;
    friend class Serializer;
    public:
        PinggyInternalType()
                                    { }

        virtual
        ~PinggyInternalType()
                                    { }

        virtual tValueType
        GetValueType()              { throw std::bad_cast(); return ValueType_Invalid; }

    #define DefineGetter(x) \
        virtual t##x \
        Get##x();
    FOREACH_ALL_TYPE(DefineGetter)
    #undef DefineGetter

        virtual size_t
        Size();

        virtual void
        JsonDump(std::ostream &os);

        virtual tString
        JsonString();

    #define DefineGetTo(x)                  \
        void                                \
        GetTo(t##x &v)              { v = Get##x(); }

    FOREACH_ALL_TYPE(DefineGetTo)
    #undef DefineGetTo

        template<typename T>
        void
        GetTo(T &val);

        template<typename T>
        void
        GetTo(std::vector<T> &val);

        template<typename T>
        void
        GetTo(std::map<tString, T> &val);

        template<typename T>
        void
        GetTo(tString key, T &val);

        template<typename T>
        void
        GetTo(tString key, T &val, const T def);

    private:
        static std::vector<PinggyInternalTypePtr>
                                    empty_vect;
        static std::map<tString, PinggyInternalTypePtr>
                                    empty_map;
        static PinggyInternalTypePtr
                                    none;
    };
    DefineStaticNewFunc(PinggyInternalType);

#define DeclareGetterForEachType(x)                                 \
    t##x                                                            \
    Get##x() override;
#define DeclareGetterForAllowedTypes(...)                           \
    APP_MACRO_FOR_EACH_FORNT(DeclareGetterForEachType, __VA_ARGS__)

#define DefinePinggyInternalTypeLiteral(x, ...)                     \
    class PinggyInternalType_ ## x : public PinggyInternalType      \
    {                                                               \
        friend class Deserializer;                                  \
        friend class Serializer;                                    \
        public:                                                     \
        PinggyInternalType_ ## x(t##x a) : value(a)                 \
                                    { }                             \
        DeclareGetterForAllowedTypes(__VA_ARGS__)                   \
        size_t                                                      \
        Size() override;                                            \
        void                                                        \
        JsonDump(std::ostream &os) override;                        \
        tValueType                                                  \
        GetValueType() override;                                    \
        private:                                                    \
        t##x value;                                                 \
    };                                                              \
    DefineMakeNonSharedPtr(PinggyInternalType_##x);

    LiteralCasting(DefinePinggyInternalTypeLiteral)

#undef DeclareGetterForEachType
#undef DeclareGetterForAllowedTypes
#undef DefinePinggyInternalTypeLiteral

    class PinggyInternalType_Array : public PinggyInternalType
    {
        friend class Deserializer;
        friend class Serializer;
    public:
        PinggyInternalType_Array(std::vector<PinggyInternalTypePtr> &a) : value(a)
                                { }

        PinggyInternalType_Array()
                                { }

        ~PinggyInternalType_Array();

        size_t
        Size() override         { return value.size(); }

        void
        JsonDump(std::ostream &os) override;

        tValueType
        GetValueType() override;

        template<typename T>
        void
        GetTo(std::vector<T> &val);

        template <typename T>
        void
        SetFrom(const std::vector<T> &val);

    private:
        std::vector<PinggyInternalTypePtr> value;
    };
    DefineMakeNonSharedPtr(PinggyInternalType_Array);

    class PinggyInternalType_Object : public PinggyInternalType
    {
        friend class Deserializer;
        friend class Serializer;
    public:
        PinggyInternalType_Object(std::map<tString, PinggyInternalTypePtr> &a) : value(a)
                                { }

        PinggyInternalType_Object()
                                { }

        ~PinggyInternalType_Object();

        virtual size_t
        Size() override         { return value.size(); }

        std::map<tString, PinggyInternalTypePtr>::iterator
        Find(tString key)       { return value.find(key); }

        std::map<tString, PinggyInternalTypePtr>::iterator
        End()                   { return value.end(); }

        PinggyInternalTypePtr&
        operator[](const tString& key)
                                { return value[key]; }

        virtual void
        JsonDump(std::ostream &os) override;

        tValueType
        GetValueType() override;

        template<typename T>
        void
        GetTo(std::map<tString, T> &val);

        template<typename T>
        void
        GetTo(tString key, T &val);

        template<typename T>
        void
        GetTo(tString key, T &val, const T &def);

        inline bool
        HasChildWithKey(tString key)
        {
            return value.find(key) != value.end();
        }

#define DefineSetFrom(x)                    \
        void                                \
        SetFrom(tString key, const t##x &v);
FOREACH_ALL_TYPE(DefineSetFrom)
#undef DefineSetFrom

        void
        SetFrom(tString key, const tCChar &v);

        template<typename T>
        void
        SetFrom(tString key, const T &v);

        template <typename T>
        void
        SetFrom(const std::map<tString, T> &val);

        void
        Set(tString key, PinggyInternalTypePtr ptr);

    private:
        std::map<tString, PinggyInternalTypePtr> value;
    };
    DefineMakeNonSharedPtr(PinggyInternalType_Object);

};

// typedef PinggyValue *PinggyValuePtr;

//===================

#undef DeclareNonSharedPtr
#undef DeclareClassWithNonSharedPtr
#undef DeclareStructWithNonSharedPtr
#undef DefineStaticNewFunc_name
#undef DefineStaticNewFunc
#undef DefineMakeCustomNonSharedPtr
#undef DefineMakeNonSharedPtr
//===================

template <typename T>
inline void
PinggyValue::PinggyInternalType::GetTo(T &val)
{
    PinggyValue v(this);
    FromPinggyValue(v, val);
}

template <typename T>
inline void
PinggyValue::PinggyInternalType::GetTo(std::vector<T> &val)
{
    auto array = dynamic_cast<PinggyInternalType_ArrayPtr>(this);
    if (!array)
        throw std::bad_cast();

    array->GetTo(val);
}

template <typename T>
inline void
PinggyValue::PinggyInternalType::GetTo(std::map<tString, T> &val)
{
    auto obj = dynamic_cast<PinggyInternalType_ObjectPtr>(this);
    if (!obj)
        throw std::bad_cast();

    obj->GetTo(val);
}

template <typename T>
inline void
PinggyValue::PinggyInternalType::GetTo(tString key, T &val)
{
    auto obj = dynamic_cast<PinggyInternalType_ObjectPtr>(this);
    if (!obj)
        throw std::bad_cast();
    obj->GetTo(key, val);
}

template <typename T>
inline void
PinggyValue::PinggyInternalType::GetTo(tString key, T &val, const T def)
{
    auto obj = dynamic_cast<PinggyInternalType_ObjectPtr>(this);
    if (!obj)
        throw std::bad_cast();
    obj->GetTo(key, val, def);
}

template <typename T>
inline void
PinggyValue::PinggyInternalType_Array::GetTo(std::vector<T> &val)
{
    for (auto elem : value) {
        T v;
        elem->GetTo(v);
        val.push_back(v);
    }
}

template <typename T>
inline void
PinggyValue::PinggyInternalType_Array::SetFrom(const std::vector<T> &val)
{
    for (auto elem : val) {
        PinggyValue v;
        v.SetFrom(elem);
        value.push_back(v.self);
    }
}

template <typename T>
inline void
PinggyValue::PinggyInternalType_Object::GetTo(std::map<tString, T> &val)
{
    for (auto elem : value) {
        T v;
        elem.second->GetTo(v);
        val[elem.first] = v;
    }
}

template <typename T>
inline void
PinggyValue::PinggyInternalType_Object::GetTo(tString key, T &v)
{
    auto elem = value.find(key);
    if (elem != value.end()) {
        elem->second->GetTo(v);
    } else {
        LOGD("Key not found:", key);
    }
}

template <typename T>
inline void
PinggyValue::PinggyInternalType_Object::GetTo(tString key, T &val, const T &def)
{
    auto elem = value.find(key);
    if (elem != value.end()) {
        elem->second->GetTo(val);
    } else {
        val = def;
    }
}

template <typename T>
void
PinggyValue::PinggyInternalType_Object::SetFrom(tString key, const T &v)
{
    PinggyValue pv;
    ToPinggyValue(pv, v);
    value[key] = pv.self;
}

template <typename T>
inline void
PinggyValue::PinggyInternalType_Object::SetFrom(const std::map<tString, T> &val)
{
    for (auto ele : val) {
        SetFrom(ele.first, ele.second);
    }
}

//========================
// PinggyValue
//========================
template<typename T>
inline void
PinggyValue::GetTo(T &val)
{
    FromPinggyValue(*this, val);
}

template<typename T>
inline void
PinggyValue::GetTo(std::vector<T> &val)
{
    auto ptr = dynamic_cast<PinggyInternalType_ArrayPtr>(self);
    if (!ptr) {
        return;
    }
    ptr->GetTo(val);
}

template<typename T>
inline void
PinggyValue::GetTo(std::map<tString, T> &val)
{
    auto ptr = dynamic_cast<PinggyInternalType_ObjectPtr>(self);
    if (!ptr) {
        return;
    }
    ptr->GetTo(val);
}

template<typename T>
inline void
PinggyValue::GetTo(tString key, T &val)
{
    auto ptr = dynamic_cast<PinggyInternalType_ObjectPtr>(self);
    if (!ptr) {
        return;
    }
    ptr->GetTo(key, val);
}

template <typename T>
inline void
PinggyValue::GetTo(tString key, T &val, const T &def)
{
    auto ptr = dynamic_cast<PinggyInternalType_ObjectPtr>(self);
    if (!ptr) {
        return;
    }
    ptr->GetTo(key, val, def);
}

template <typename T>
inline void
PinggyValue::SetFrom(const T &val)
{
    if (self) {
        throw std::invalid_argument("Already assigned value");
    }
    ToPinggyValue(*this, val);
}

template <typename T>
inline void
PinggyValue::SetFrom(const std::vector<T> &val)
{
    if (self) {
        throw std::invalid_argument("Already assigned value");
    }
    auto s = NewPinggyInternalType_ArrayPtr();
    s->SetFrom(val);
    self = s;
}

template <typename T>
inline void
PinggyValue::SetFrom(const std::map<tString, T> &val)
{
    if (self) {
        throw std::invalid_argument("Already assigned value");
    }
    auto s = NewPinggyInternalType_ObjectPtr();
    s->SetFrom(val);
    self = s;
}

template <typename T>
inline void
PinggyValue::SetFrom(tString key, const T &val)
{
    PinggyInternalType_ObjectPtr s = nullptr;
    if (self) {
        s = dynamic_cast<PinggyInternalType_ObjectPtr>(self);
    } else {
        s = NewPinggyInternalType_ObjectPtr();
        self = s;
    }
    if (!s)
        throw std::bad_cast();
    PinggyValue pv;
    pv.SetFrom(val);
    s->Set(key, pv.self);
}

#endif // SRC_CPP_PINGGYTRANSPORT_PINGGYVALUE_HH_
