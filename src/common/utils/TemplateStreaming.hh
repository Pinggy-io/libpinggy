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

 #ifndef __SRC_CPP_PUBLIC_COMMON_UTILS_TEMPLATELOGGING_HH__
 #define __SRC_CPP_PUBLIC_COMMON_UTILS_TEMPLATELOGGING_HH__

#include "Utils.hh"

template< typename T, typename U, typename V >
inline void
__dump_value(std::basic_ostream<U, V>& os, const std::shared_ptr<T>& ptr)
{
    DumpPtr(os, ptr);
}


template< typename T, typename U, typename V >
inline void
__dump_value(std::basic_ostream<U, V>& os, const T &val)
{
    os << val;
}

template< typename T, typename U, typename V >
inline std::basic_ostream<U, V>&
operator<<(std::basic_ostream<U, V>& os, const std::vector<T>& vect)
{
    os << "[";
    bool comma = false;
    for (auto ele : vect) {
        if (comma)
            os << ", ";
        comma = true;
        __dump_value(os, ele);
        // os << ele;
    }
    os << "]";
    return os;
}

template<typename K, typename V, typename T, typename U >
inline std::basic_ostream<T, U>&
operator<<(std::basic_ostream<T, U>& os, const std::map<K, V>& map)
{
    os << "{";
    bool comma = false;
    for (auto ele : map) {
        if (comma)
            os << ", ";
        comma = true;
        os << ele.first << ": ";
        __dump_value(os, ele.second);
        //  << ele.second;
    }
    os << "}";
    return os;

}

template< typename T, typename U, typename V >
inline std::basic_ostream<U, V>&
operator<<(std::basic_ostream<U, V>& os, const std::set<T>& vect)
{
    os << "{";
    bool comma = false;
    for (auto ele : vect) {
        if (comma)
            os << ", ";
        comma = true;
        __dump_value(os, ele);
        // os << ele;
    }
    os << "}";
    return os;
}

template<typename Func, typename... Ts>
inline void
for_each_in_tuple(const std::tuple<Ts...>& tup, Func f) {
    std::apply([&](auto const&... elems) { (f(elems), ...); }, tup);
}

template<typename U, typename V, typename... Args >
inline std::basic_ostream<U, V>&
operator<<(std::basic_ostream<U, V>& os, const std::tuple<Args...>& t)
{
    for_each_in_tuple(t, [&](const auto& x) {
        __dump_value(os, x);
        os << " ";
    });
    return os;
}

template<typename K, typename V, typename T, typename U >
inline std::basic_ostream<T, U>&
operator<<(std::basic_ostream<T, U>& os, const std::pair<K, V>& pair)
{
    os << "(" << pair.first << ", " << pair.second << ")";
    return os;
}


 #endif // __SRC_CPP_PUBLIC_COMMON_UTILS_TEMPLATELOGGING_HH__