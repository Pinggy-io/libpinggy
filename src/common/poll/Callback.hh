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

#ifndef __SRC_CPP_COMMON_POLL_CALLBACK_HH__
#define __SRC_CPP_COMMON_POLL_CALLBACK_HH__

#include <platform/platform.h>
#include <platform/SharedPtr.hh>

namespace common
{

template<typename ... Args>
abstract class Callback : public virtual pinggy::SharedObject
{
public:
    virtual
    ~Callback()                 {}

    virtual void
    DisArm() = 0;

    virtual void
    Fire(Args ... args) = 0;
};


template<typename ... Args>
class CallbackImplStatic: public virtual pinggy::SharedObject, public Callback<Args...>
{
public:
    CallbackImplStatic(void (*func)(Args ...)): func(func)
                                { }

    virtual
    ~CallbackImplStatic()       { }

    virtual void
    Fire(Args ... args) override
                                { if (func) func(args...); }

    virtual void
    DisArm() override           { func=nullptr; }

    virtual size_t
    DumpMemory(std::ostream& os) override;

    DefineMandatoryTemplateClassFunctions(CallbackImplStatic, Args...);

private:
    typedef void (*tFunc)(Args...);

    tFunc                       func;
};


template<typename T, typename ... Args>
class CallbackImplMember: public virtual pinggy::SharedObject, public Callback<Args...>
{
public:
    CallbackImplMember(std::shared_ptr<T> _t, void (T::*func)(Args ...)): t(_t), func(func)
                                { }

    virtual
    ~CallbackImplMember()       { }

    virtual void
    Fire(Args ... args) override
                                { if (func) (t.get()->*func)(args...); }

    virtual void
    DisArm() override           { func=nullptr; }

    virtual size_t
    DumpMemory(std::ostream& os) override;

    DefineMandatoryTemplateClassFunctions(CallbackImplMember, T, Args...);

private:
    typedef void (T::*tMemFunc)(Args...);

    std::shared_ptr<T>          t;
    tMemFunc                    func;
};


template<typename ... Args>
inline size_t
CallbackImplStatic<Args...>::DumpMemory(std::ostream& os)
{
    os << "{\"type\":\"CallbackImplStatic\",\"members\":{";
    size_t size = sizeof(func);
    os << "\"funcPtr\":{\"type\":\"primitive\",\"size\":" << sizeof(func) << "}";
    os << "},\"Consumed\":" << size << "}";
    return size;
}

template<typename T, typename ... Args>
inline size_t
CallbackImplMember<T, Args...>::DumpMemory(std::ostream& os)
{
    os << "{\"type\":\"CallbackImplMember\",\"members\":{";
    size_t size = sizeof(t) + sizeof(func);
    os << "\"objectPtr\":{\"type\":\"shared_ptr\",\"size\":" << sizeof(t) << "},";
    os << "\"memfuncPtr\":{\"type\":\"primitive\",\"size\":" << sizeof(func) << "}";
    os << "},\"Consumed\":" << size << "}";
    return size;
}

template<typename ... Args>
std::shared_ptr<CallbackImplStatic<Args...> >
NewCallbackImplPtr(void (*func)(Args ...))
{
    return std::make_shared<CallbackImplStatic<Args...>>(func);
}

template<typename T, typename ... Args>
std::shared_ptr<CallbackImplMember<T, Args...> >
NewCallbackImplPtr(std::shared_ptr<T> t, void (T::*func)(Args ...))
{
    return std::make_shared<CallbackImplMember<T, Args...>>(t, func);
}

} // namespace common

#endif // __SRC_CPP_COMMON_POLL_CALLBACK_HH__
