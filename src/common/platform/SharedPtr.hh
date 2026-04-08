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


#ifndef CPP_COMMON_SHAREDPTR_H_
#define CPP_COMMON_SHAREDPTR_H_


#include <memory>
#include "platform.h"
#include <iostream>
#include "app_foreach_macro.h"

//#define NO_SHARED_PTR

namespace pinggy {

#ifndef NO_SHARED_PTR

#define INITIAL_MEMORY_DUMP_VISIT_ID 1

struct SharedObject: public std::enable_shared_from_this<SharedObject>
{
    SharedObject() {
        incrementAllocationCount();
    }

    virtual
    ~SharedObject() {
        incrementDeallocationCount();
    }

    virtual tUint64
    Hash()                      { return (tUint64) this; }

    virtual void
    __Init()                    { }

    std::shared_ptr<SharedObject>
    getptr()
    {
        return shared_from_this();
    }

    template<typename D>
    std::shared_ptr<D>
    DynamicPointerCast() noexcept
    {
        return std::dynamic_pointer_cast<D>(shared_from_this());
    }

    template<typename D>
    std::shared_ptr<D>
    StaticPointerCast() noexcept
    {
        return std::static_pointer_cast<D>(shared_from_this());
    }

    template<typename D>
    void
    DynamicPointerCast(std::shared_ptr<D> &ptr) noexcept
    {
        ptr = std::dynamic_pointer_cast<D>(shared_from_this());
    }

    template<typename D>
    void
    StaticPointerCast(std::shared_ptr<D> &ptr) noexcept
    {
        ptr = std::static_pointer_cast<D>(shared_from_this());
    }

    virtual std::string
    GetString() {
        return std::to_string(Hash());
    }

    virtual size_t
    DumpMemory(std::ostream& os)
    {
        os << "\"<not_implemented>\"";
        return MemberClsSize();
    }

    virtual tString
    MemberClsName() = 0;

    virtual size_t
    MemberClsSize() = 0;

    virtual bool
    IsMemoryDumpingAllowed() final;

    static tUint64
    GetAllocationCount();

    static tUint64
    GetDeallocationCount();

    static void
    InitiateNextMemoryDumpVisit();

private:
    static void
    incrementAllocationCount();

    static void
    incrementDeallocationCount();

    tUint8                      mDumpId = INITIAL_MEMORY_DUMP_VISIT_ID;

    static tUint64              gAllocationCount;
    static tUint64              gDeallocationCount;
    static tUint8               gCurrentVisitId;
};

template< class T, class U >
std::shared_ptr<T>
DynamicPointerCast(const std::shared_ptr<U>& r) noexcept
{
    return std::dynamic_pointer_cast<T>(r);
}

template< class T, class U >
std::shared_ptr<T>
DynamicPointerCast(T *,  const std::shared_ptr<U>& r) noexcept
{
    return std::dynamic_pointer_cast<T>(r);
}

template< class T, class U >
std::shared_ptr<T>
StaticPointerCast(const std::shared_ptr<U>& r) noexcept
{
    return std::static_pointer_cast<T>(r);
}

template <typename T>
bool
operator<(const std::shared_ptr<T>& lhs, const std::shared_ptr<T>& rhs)
{
    return lhs->Hash() < rhs->Hash();
}


template <typename T>
bool
operator==(const std::shared_ptr<T>& lhs, const std::shared_ptr<T>& rhs)
{
    return (lhs && rhs) ? lhs->Hash() == rhs->Hash() : false;
}

typedef std::shared_ptr<SharedObject> VoidPtr;
typedef std::weak_ptr<SharedObject> VoidWPtr;

#else

struct SharedObject
{
    virtual ~SharedObject(){};
};

template< class T, class U >
T *
DynamicPointerCast(U *r) noexcept
{
    return dynamic_cast<T*>(r);
}

template< class T, class U >
T *
DynamicPointerCast(T *t,  U *r) noexcept
{
    return dynamic_cast<T*>(r);
}

template< class T, class U >
T *
StaticPointerCast(U *r) noexcept
{
    return (T *)r;
}

typedef SharedObject *VoidPtr;

#endif

} // namespace piggy

typedef pinggy::VoidPtr tVoidPtr;
typedef pinggy::VoidWPtr tVoidWPtr;

#ifndef DefineMakeSharedPtr

#ifndef NO_SHARED_PTR

#define _EXP_EXP_(x) x

//================
// class
//================

#define DeclareSharedPtr2(x, name, name2, ...) \
                    typedef std::shared_ptr<x> name; \
                    typedef std::weak_ptr<x> name2;

#ifndef NDEBUG
#define __FunctionValidation(x, obj) \
    if constexpr (false) { \
        x::StaticClsName(); \
        x::StaticClsSize(); \
        obj->MemberClsName(); \
        obj->MemberClsSize(); \
        std::shared_ptr<x> _o = obj->__GetTestObj(); \
    }
#else
#define __FunctionValidation(x, obj)
#endif

#define DeclareSharedPtr(x, ...) _EXP_EXP_(DeclareSharedPtr2(x, x##Ptr, x##WPtr, ## __VA_ARGS__))
#define DefineMakeCustomSharedPtr(x, name) \
    DeclareSharedPtr(x, name) \
    template <typename ... Arguments> \
    inline name New##x##Ptr(Arguments ... args) \
    { \
        auto _v = std::make_shared<x>(args...); \
        __FunctionValidation(x, _v); \
        _v->__Init(); \
        return _v; \
    } \
    inline name New##x##Ptr(x *y) \
    { \
        auto _v = std::shared_ptr<x>(y); \
        __FunctionValidation(x, _v); \
        _v->__Init(); \
        return _v; \
    }

#define DefineMakeSharedPtr(x) DefineMakeCustomSharedPtr(x, x##Ptr)

#define DefineMakePrivateCustomSharedPtr(x, name) \
    DeclareSharedPtr(x, name) \
    template <typename ... Arguments> \
    inline name New##x##Ptr(x *y) \
    { \
        auto _v = std::shared_ptr<x>(y); \
        __FunctionValidation(x, _v); \
        _v->__Init(); \
        return _v; \
    }

#define DefineMakePrivateSharedPtr(x) DefineMakeCustomSharedPtr(x, x##Ptr)


#define DeclareSharedTemplatePtr(x, ...) //_EXP_EXP_(DeclareSharedTemplatePtr2(x, x##Ptr, x##WPtr, ## __VA_ARGS__))
#define DefineMakeCustomTemplateSharedPtr(x, name) \
    DeclareSharedTemplatePtr(x, name) \
    template <typename T, typename ... Arguments> \
    inline std::shared_ptr<x<T>> New##x##Ptr(T t, Arguments ... args) \
    { \
        auto _v std::make_shared<x<T>>(t, args...); \
        __FunctionValidation(x<T>, _v); \
        _v->__Init(); \
        return _v; \
    } \
    template <typename T, typename ... Arguments> \
    inline std::shared_ptr<x<T>> New##x##Ptr(x<T> *y) \
    { \
        auto _v = std::shared_ptr<x<T>>(y); \
        __FunctionValidation(x<T>, _v); \
        _v->__Init(); \
        return _v; \
    }

#define DefineMakeTemplateSharedPtr(x) DefineMakeCustomTemplateSharedPtr(x, x##Ptr)

#define DeclareStructWithSharedPtr(x) \
    struct x; \
    DeclareSharedPtr(x, x##Ptr);

#define DeclareClassWithSharedPtr(x) \
    class x; \
    DeclareSharedPtr(x, x##Ptr);

#define thisPtr pinggy::DynamicPointerCast(this, shared_from_this())


template< typename T, typename U, typename V >
void
DumpPtr(std::basic_ostream<U, V>& os, const std::shared_ptr<T>& ptr)
{
    if (!ptr) {
        os << "<null>";
        return;
    }
    os << ptr->GetString();
}

#else

#define _EXP_EXP_(x) x
#define DeclareSharedPtr2(x, name, name2, ...) typedef x *name; typedef x *name2;
#define DeclareSharedPtr(x, ...) _EXP_EXP_(DeclareSharedPtr2(x, x##Ptr, x##WPtr, ## __VA_ARGS__))

#define DefineMakeCustomSharedPtr(x, name) \
    DeclareSharedPtr(x, name) \
    template <typename ... Arguments> \
    inline name New##x##Ptr(Arguments ... args) { return new x(args...);} \
    inline name New##x##Ptr(x *y) { return y;}

#define DefineMakeSharedPtr(x) DefineMakeCustomSharedPtr(x, x##Ptr)

#define DeclareStructWithSharedPtr(x) \
    struct x; \
    DeclareSharedPtr(x, x##Ptr);

#define DeclareClassWithSharedPtr(x) \
    class x; \
    DeclareSharedPtr(x, x##Ptr);

#define thisPtr this

#endif //NO_SHARED_PTR

#endif //DefineMakeSharedPtr


#define _OBJ_DUMP__DUMP_MEM_USAGE_REST(var) \
        { \
            size += DumpMemoryUsages(os, TO_STR(var), var); \
            os << ","; \
        }
#define _OBJ_DUMP__DUMP_MEM_USAGE_LAST(var) \
        { \
            size += DumpMemoryUsages(os, TO_STR(var), var); \
        }

#define _OBJ_DUMP__ARG_FETCH_FUNC(x) x

#define _OBJ_DUMP__DUMP_MEM_USAGE_REST_F(f, var) \
        _OBJ_DUMP__DUMP_MEM_USAGE_REST(f(var))

#define _OBJ_DUMP__DUMP_MEM_USAGE_LAST_F(f, var) \
        _OBJ_DUMP__DUMP_MEM_USAGE_LAST(f(var))

#define _OBJ_DUMP__NO_EXCESS 0

#define _OBJ_DUMP__DUMP_MEMORY_BODY_SUPER_F(s_f, f, name, super, excess, ...) \
    { \
        size_t size = excess; \
        os << "{\"type\":\"" TO_STR(name) "\",\"members\":{"; \
        s_f(super) \
        APP_MACRO_FOR_EACH_WITH_ARG_FORNT_END(_OBJ_DUMP__DUMP_MEM_USAGE_REST_F, _OBJ_DUMP__DUMP_MEM_USAGE_LAST_F, f, __VA_ARGS__) \
        os << "},\"Consumed\":" << size<< "}"; \
        return size; \
    }

#define _OBJ_DUMP__SUPER_DUMP(super) \
        os << "\"Super\":"; \
        size += super::DumpMemory(os); \
        os << ",";

#define _OBJ_DUMP__NOSUPER_DUMP(super)

#define DEFINE_DUMP_MEMORY_BODY_SUPER_F(f, name, super, ...) \
    _OBJ_DUMP__DUMP_MEMORY_BODY_SUPER_F(_OBJ_DUMP__SUPER_DUMP, f, name, super, _OBJ_DUMP__NO_EXCESS, __VA_ARGS__)

#define DEFINE_DUMP_MEMORY_BODY_SUPER(name, super, ...) \
    _OBJ_DUMP__DUMP_MEMORY_BODY_SUPER_F(_OBJ_DUMP__SUPER_DUMP, _OBJ_DUMP__ARG_FETCH_FUNC, name, super, _OBJ_DUMP__NO_EXCESS, __VA_ARGS__)

#define DEFINE_DUMP_MEMORY_BODY_SUPER_WITH_EXTRA(name, super, exess, ...) \
    _OBJ_DUMP__DUMP_MEMORY_BODY_SUPER_F(_OBJ_DUMP__SUPER_DUMP, _OBJ_DUMP__ARG_FETCH_FUNC, name, super, exess, __VA_ARGS__)

#define DEFINE_DUMP_MEMORY_BODY(name, ...) \
    _OBJ_DUMP__DUMP_MEMORY_BODY_SUPER_F(_OBJ_DUMP__NOSUPER_DUMP, _OBJ_DUMP__ARG_FETCH_FUNC, name, super, _OBJ_DUMP__NO_EXCESS, __VA_ARGS__)

#define DEFINE_DUMP_MEMORY_BODY_WITH_EXTRA(name, excess, ...) \
    _OBJ_DUMP__DUMP_MEMORY_BODY_SUPER_F(_OBJ_DUMP__NOSUPER_DUMP, _OBJ_DUMP__ARG_FETCH_FUNC, name, super, excess, __VA_ARGS__)

#define DEFINE_DUMP_MEMORY_BODY_F(f, name, ...) \
    _OBJ_DUMP__DUMP_MEMORY_BODY_SUPER_F(_OBJ_DUMP__NOSUPER_DUMP, f, name, super, _OBJ_DUMP__NO_EXCESS, __VA_ARGS__)


#define _OBJ_DUMP__DEFINE_DUMP_MEMORY_NOBODY_SUPER(s_f, name, super, excess) \
    { \
        size_t size = excess; \
        os << "{\"type\":\"" TO_STR(name) "\",\"members\":{"; \
        s_f(super) \
        os << "},\"Consumed\":" << size<< "}"; \
        return size; \
    }

#define DEFINE_DUMP_MEMORY_NOBODY_SUPER(name, super) \
        _OBJ_DUMP__DEFINE_DUMP_MEMORY_NOBODY_SUPER(_OBJ_DUMP__SUPER_DUMP, name, super, _OBJ_DUMP__NO_EXCESS)
#define DEFINE_DUMP_MEMORY_NOBODY(name) \
        _OBJ_DUMP__DEFINE_DUMP_MEMORY_NOBODY_SUPER(_OBJ_DUMP__NOSUPER_DUMP, name, super, _OBJ_DUMP__NO_EXCESS)

#define DEFINE_DUMP_MEMORY_NOBODY_SUPER_WITH_EXTRA(name, super, exess) \
        _OBJ_DUMP__DEFINE_DUMP_MEMORY_NOBODY_SUPER(_OBJ_DUMP__SUPER_DUMP, name, super, exess)

#define DEFINE_DUMP_MEMORY_NOBODY_WITH_EXTRA(name, exess) \
        _OBJ_DUMP__DEFINE_DUMP_MEMORY_NOBODY_SUPER(_OBJ_DUMP__NOSUPER_DUMP, name, super, exess)

#ifdef DEFINE_MEMORY_DUMP
#ifndef INCLUDE_MEMORY_DUMP_DEFINITION
#define INCLUDE_MEMORY_DUMP_DEFINITION
#endif
#else
#ifdef INCLUDE_MEMORY_DUMP_DEFINITION
#undef INCLUDE_MEMORY_DUMP_DEFINITION
#endif
#define INCLUDE_MEMORY_DUMP_DEFINITION
#endif

#define PRIMARY_DEPENDENT_HEADERS(...) // Allow genmemdump to findout dependent headers apart from the default one

//===========================

//=====================
// Mandatory
//=====================

#ifdef DEFINE_MEMORY_DUMP
#define DefineMandatoryDumpFunction(x) \
    virtual size_t  \
    DumpMemory(std::ostream& os) override

#define DefineMandatoryLocalDumpFunction(x) \
    virtual size_t  \
    DumpMemory(std::ostream& os) override { \
        APP_EXPAND(LOCALFILLE_BODY_ ## x) \
    }
#else
#ifdef DEFINE_MEMORY_DUMP_GEN
#define DefineMandatoryDumpFunction(x) \
    virtual size_t  \
    DumpMemory(std::ostream& os) override

#define DefineMandatoryLocalDumpFunction(x) \
    virtual size_t  \
    DumpMemory(std::ostream& os) override
#else
#define DefineMandatoryDumpFunction(x)
#define DefineMandatoryLocalDumpFunction(x)
#endif //if DEFINE_MEMORY_DUMP_GEN
#endif //DEFINE_MEMORY_DUMP

#define _DefineMandatoryClassFunctionsImplNoDump(x) \
    static tString \
    StaticClsName()             { return TO_STR(x); } \
    static size_t \
    StaticClsSize()             { return sizeof(x); } \
    virtual tString \
    MemberClsName() override    { return StaticClsName(); } \
    virtual size_t \
    MemberClsSize() override    { return StaticClsSize(); }\
    std::shared_ptr<x> \
    __GetTestObj()              { return thisPtr->DynamicPointerCast<x>(); }

#define _DefineMandatoryClassFunctionsImpl(x) \
    _DefineMandatoryClassFunctionsImplNoDump(x) \
    DefineMandatoryDumpFunction(x)

#define DefineMandatoryClassFunctionsWOSuper(x) _DefineMandatoryClassFunctionsImpl(x)
#define DefineMandatoryClassFunctionsWithSuper(x, y) _DefineMandatoryClassFunctionsImpl(x)
#define DefineMandatoryClassFunctionsForTest(x) _DefineMandatoryClassFunctionsImplNoDump(x)
#define DefineMandatoryClassFunctionsNoDump(x) _DefineMandatoryClassFunctionsImplNoDump(x)


#define _DefineMandatoryFileLocalClassFunctionsImpl(x) \
    _DefineMandatoryClassFunctionsImplNoDump(x) \
    DefineMandatoryLocalDumpFunction(x)

#define DefineMandatoryFileLocalClassFunctionsWOSuper(x) _DefineMandatoryFileLocalClassFunctionsImpl(x)
#define DefineMandatoryFileLocalClassFunctionsWithSuper(x, y) _DefineMandatoryFileLocalClassFunctionsImpl(x)


#define DefineMandatoryClassFunctionsWOSuperWithExcessSize(x, excess) _DefineMandatoryClassFunctionsImpl(x)
#define DefineMandatoryClassFunctionsWithSuperWithExcessSize(x, y, excess) _DefineMandatoryClassFunctionsImpl(x)


#define DefineMandatoryFileLocalClassFunctionsWOSuperWithExcessSize(x, excess) _DefineMandatoryFileLocalClassFunctionsImpl(x)
#define DefineMandatoryFileLocalClassFunctionsWithSuperWithExcessSize(x, y, excess) _DefineMandatoryFileLocalClassFunctionsImpl(x)


#define DefineMandatoryTemplateClassFunctions(x, ...) \
    static tString \
    StaticClsName()             { return TO_STR(x); } \
    static size_t \
    StaticClsSize()             { return sizeof(x<__VA_ARGS__>); } \
    virtual tString \
    MemberClsName() override    { return StaticClsName(); } \
    virtual size_t \
    MemberClsSize() override    { return StaticClsSize(); } \
    std::shared_ptr<x<__VA_ARGS__>> \
    __GetTestObj()              { return thisPtr-> template DynamicPointerCast<x<__VA_ARGS__>>(); }


#define _DefineMandatoryAbsClassFunctionsImpl(x) \
    DefineMandatoryDumpFunction(x)

#define DefineMandatoryAbsClassFunctionsWOSuper(x) _DefineMandatoryAbsClassFunctionsImpl(x)
#define DefineMandatoryAbsClassFunctionsWithSuper(x, y) _DefineMandatoryAbsClassFunctionsImpl(x)


#endif /* CPP_COMMON_SHAREDPTR_H_ */
