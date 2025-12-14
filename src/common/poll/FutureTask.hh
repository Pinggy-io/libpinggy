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
                                {}
    template<typename A1>
    FutureTaskImplStatic(R (*func)(A1, Args ...), Args ...args): func(func), data(std::make_tuple(args...))
                                {}

    virtual
    ~FutureTaskImplStatic()     {}

    virtual void
    Fire() override             { if (func) std::apply(func, data); }

    virtual void
    DisArm() override           { func=nullptr; }

private:
    typedef R (*tFunc)(Args...);

    tFunc                       func;

    std::tuple<Args...>         data;
};

template<typename T, typename R, typename ... Args>
class FutureTaskImplMem: public Task
{
public:
    FutureTaskImplMem(std::shared_ptr<T> _t, R (T::*func)(Args ...), Args ...args): t(_t), func(func), data(std::make_tuple(args...))
                                {}

    virtual
    ~FutureTaskImplMem()        {}

    virtual void
    Fire() override;

    virtual void
    DisArm() override           { func=nullptr; }

private:
    typedef R (T::*tMemFunc)(Args...);

    std::shared_ptr<T>          t;
    tMemFunc                    func;


    std::tuple<Args...>         data;
};



template<typename T, typename R, typename ...Args>
inline void
FutureTaskImplMem<T, R, Args...>::Fire()
{
    if (func)
        std::apply([this](Args ... a) {
            (this->t.get()->*func)(a...);
        }, data);
}

template<typename R, typename ... Args>
std::shared_ptr<FutureTaskImplStatic<R, Args...> >
NewFutureTaskImplPtr(R (*func)(Args ...), Args ...args)
{
    return std::make_shared<FutureTaskImplStatic<R, Args...>>(func, args...);
}

template<typename T, typename R, typename ... Args>
std::shared_ptr<FutureTaskImplMem<T, R, Args...> >
NewFutureTaskImplPtr(std::shared_ptr<T> t, R (T::*func)(Args ...), Args ...args)
{
    return std::make_shared<FutureTaskImplMem<T, R, Args...>>(t, func, args...);
}

} // namespace common

#endif // __SRC_CPP_COMMON_POLL_FUTURETASK_HH__
