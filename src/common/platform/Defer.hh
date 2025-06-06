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


#ifndef CPP_COMMON_DEFER_HH_
#define CPP_COMMON_DEFER_HH_

namespace common {


template<typename T>
class Defer
{
public:
    Defer(T Arg);
    virtual ~Defer();
private:
    T func;
};



template<typename T>
inline Defer<T>::Defer(T Arg): func(Arg)
{
}

template<typename T>
inline Defer<T>::~Defer()
{
    func();
}

} /* namespace common */

#define ___DeferCat(a, b) ____DeferCat(a, b)
#define ____DeferCat(a, b) a ## b

#define DEFER(x) common::Defer ___DeferCat(__pop,__LINE__)([&]{x}); (void)___DeferCat(__pop,__LINE__);

#endif /* CPP_COMMON_DEFER_HH_ */
