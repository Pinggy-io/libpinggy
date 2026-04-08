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

#ifndef __SRC_CPP_COMMON_POLL_FUTURETASK_HH__
#define __SRC_CPP_COMMON_POLL_FUTURETASK_HH__

#include "Task.hh"

namespace common
{

template<typename R, typename ... Args>
class FutureTaskImplStatic: public Task
{
public:
    FutureTaskImplStatic(R (*func)(Args ...), Args ...args): func(func), data(std::make_tuple(args...))
                                { }
    template<typename A1>
    FutureTaskImplStatic(R (*func)(A1, Args ...), Args ...args): func(func), data(std::make_tuple(args...))
                                { }

    virtual
    ~FutureTaskImplStatic()     { }

    virtual void
    Fire() override             { if (func) std::apply(func, data); }

    virtual void
    DisArm() override           { func=nullptr; }

    virtual size_t
    DumpMemory(std::ostream& os) override;

    DefineMandatoryTemplateClassFunctions(FutureTaskImplStatic, R, Args...);

private:
    typedef R (*tFunc)(Args...);

    tFunc                       func;

    std::tuple<Args...>         data;
};

template<typename T, typename R, typename ... Args>
class FutureTaskImplMember: public Task
{
public:
    FutureTaskImplMember(std::shared_ptr<T> _t, R (T::*func)(Args ...), Args ...args): t(_t), func(func), data(std::make_tuple(args...))
                                { }

    virtual
    ~FutureTaskImplMember()     { }

    virtual void
    Fire() override;

    virtual void
    DisArm() override           { func=nullptr; }

    virtual size_t
    DumpMemory(std::ostream& os) override;

    DefineMandatoryTemplateClassFunctions(FutureTaskImplMember, T, R, Args...);

private:
    typedef R (T::*tMemFunc)(Args...);

    std::shared_ptr<T>          t;
    tMemFunc                    func;


    std::tuple<Args...>         data;
};


//It will retain reference for sometime. So that a object own't get deleted immediately.
template<typename ... Args>
class FutureTaskImplRetainer: public Task
{
public:
    FutureTaskImplRetainer(std::shared_ptr<Args> ...args): isActive(true), data(std::make_tuple(args...))
                                { }

    virtual
    ~FutureTaskImplRetainer()   { }

    virtual void
    Fire() override             { }

    virtual void
    DisArm() override           { isActive = false; }

    virtual size_t
    DumpMemory(std::ostream& os) override;

    DefineMandatoryTemplateClassFunctions(FutureTaskImplRetainer, Args...);
    // std::shared_ptr<FutureTaskImplRetainer<Args...>>
    // __GetTestObj()
    // {
    //     return thisPtr->DynamicPointerCast<FutureTaskImplRetainer<Args...>>();
    // }

private:
    bool                        isActive;
    std::tuple<std::shared_ptr<Args>...>
                                data;
};



template<typename T, typename R, typename ...Args>
inline void
FutureTaskImplMember<T, R, Args...>::Fire()
{
    if (func)
        std::apply([this](Args ... a) {
            (this->t.get()->*func)(a...);
        }, data);
}

template<typename R, typename ... Args>
inline size_t
FutureTaskImplStatic<R, Args...>::DumpMemory(std::ostream& os)
{
    os << "{\"type\":\"FutureTaskImplStatic\",\"members\":{";
    size_t size = sizeof(func) + sizeof(data);
    os << "\"funcPtr\":{\"type\":\"primitive\",\"size\":" << sizeof(func) << "},";
    os << "\"tupleData\":{\"type\":\"tuple\",\"size\":" << sizeof(data) << "}";
    os << "},\"Consumed\":" << size << "}";
    return size;
}

template<typename T, typename R, typename ... Args>
inline size_t
FutureTaskImplMember<T, R, Args...>::DumpMemory(std::ostream& os)
{
    os << "{\"type\":\"FutureTaskImplMember\",\"members\":{";
    size_t size = sizeof(t) + sizeof(func) + sizeof(data);
    os << "\"objectPtr\":{\"type\":\"shared_ptr\",\"size\":" << sizeof(t) << "},";
    os << "\"memfuncPtr\":{\"type\":\"primitive\",\"size\":" << sizeof(func) << "},";
    os << "\"tupleData\":{\"type\":\"primitive\",\"size\":" << sizeof(data) << "}";
    os << "},\"Consumed\":" << size << "}";
    return size;
}

template<typename ... Args>
inline size_t
FutureTaskImplRetainer<Args...>::DumpMemory(std::ostream& os)
{
    os << "{\"type\":\"FutureTaskImplRetainer\",\"members\":{";
    size_t size = sizeof(isActive) + sizeof(data);
    os << "\"isActive\":{\"type\":\"primitive\",\"size\":" << sizeof(isActive) << "},";
    os << "\"retainedData\":{\"type\":\"primitive\",\"size\":" << sizeof(data) << "}";
    os << "},\"Consumed\":" << size << "}";
    return size;
}

template<typename R, typename ... Args>
std::shared_ptr<FutureTaskImplStatic<R, Args...> >
NewFutureTaskImplPtr(R (*func)(Args ...), Args ...args)
{
    return std::make_shared<FutureTaskImplStatic<R, Args...>>(func, args...);
}

template<typename T, typename R, typename ... Args>
std::shared_ptr<FutureTaskImplMember<T, R, Args...> >
NewFutureTaskImplPtr(std::shared_ptr<T> t, R (T::*func)(Args ...), Args ...args)
{
    return std::make_shared<FutureTaskImplMember<T, R, Args...>>(t, func, args...);
}

template<typename ... Args>
std::shared_ptr<FutureTaskImplRetainer<Args...> >
NewFutureTaskImplRetainerPtr(std::shared_ptr<Args> ...args)
{
    return std::make_shared<FutureTaskImplRetainer<Args...>>(args...);
}

} // namespace common

#endif // __SRC_CPP_COMMON_POLL_FUTURETASK_HH__
