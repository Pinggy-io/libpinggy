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

//#define NO_SHARED_PTR

namespace pinggy {

#ifndef NO_SHARED_PTR

struct SharedObject: public std::enable_shared_from_this<SharedObject>
{
    virtual
    ~SharedObject()             { };

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

#ifndef DefineMakeSharedPtr

#ifndef NO_SHARED_PTR

#define _EXP_EXP_(x) x
#define DeclareSharedPtr2(x, name, name2, ...) \
                    typedef std::shared_ptr<x> name; \
                    typedef std::weak_ptr<x> name2;

#define DeclareSharedPtr(x, ...) _EXP_EXP_(DeclareSharedPtr2(x, x##Ptr, x##WPtr, ## __VA_ARGS__))
#define DefineMakeCustomSharedPtr(x, name) \
    DeclareSharedPtr(x, name) \
    template <typename ... Arguments> \
    inline name New##x##Ptr(Arguments ... args) { auto _v = std::make_shared<x>(args...); _v->__Init(); return _v; } \
    inline name New##x##Ptr(x *y) { auto _v = std::shared_ptr<x>(y); _v->__Init(); return _v;}

#define DefineMakeSharedPtr(x) DefineMakeCustomSharedPtr(x, x##Ptr)

#define DefineMakePrivateCustomSharedPtr(x, name) \
    DeclareSharedPtr(x, name) \
    template <typename ... Arguments> \
    inline name New##x##Ptr(x *y) { return std::shared_ptr<x>(y);}

#define DefineMakePrivateSharedPtr(x) DefineMakeCustomSharedPtr(x, x##Ptr)


#define DeclareSharedTemplatePtr(x, ...) //_EXP_EXP_(DeclareSharedTemplatePtr2(x, x##Ptr, x##WPtr, ## __VA_ARGS__))
#define DefineMakeCustomTemplateSharedPtr(x, name) \
    DeclareSharedTemplatePtr(x, name) \
    template <typename T, typename ... Arguments> \
    inline std::shared_ptr<x<T>> New##x##Ptr(T t, Arguments ... args) { return std::make_shared<x<T>>(t, args...);} \
    template <typename T, typename ... Arguments> \
    inline std::shared_ptr<x<T>> New##x##Ptr(x<T> *y) { return std::shared_ptr<x<T>>(y);}

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



#endif /* CPP_COMMON_SHAREDPTR_H_ */
