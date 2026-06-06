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

#ifndef __PINGGY_SHAREDPTR_INTERNAL_INCLUDE__
#error "ContainerStreaming.hh is internal; include <platform/SharedPtr.hh> instead."
#endif

#ifndef COMMON_PLATFORM_CONTAINERSTREAMING_HH_
#define COMMON_PLATFORM_CONTAINERSTREAMING_HH_

#include "PrimitiveStreaming.hh"
#include <vector>
#include <queue>
#include <map>
#include <set>
#include <tuple>
#include <utility>
#include <ostream>

// Forward declarations so phase-1 unqualified lookup inside any container template
// can find every other container's overload (cross-container nesting like
// vector<tuple<...>> would otherwise fail because ADL on std types only searches std).

template< typename T, typename U, typename V >
std::basic_ostream<U, V>&
operator<<(std::basic_ostream<U, V>& os, const std::vector<T>& vect);

template<typename K, typename V, typename T, typename U >
std::basic_ostream<T, U>&
operator<<(std::basic_ostream<T, U>& os, const std::map<K, V>& map);

template< typename T, typename U, typename V >
std::basic_ostream<U, V>&
operator<<(std::basic_ostream<U, V>& os, const std::set<T>& vect);

template<typename U, typename V, typename... Args >
std::basic_ostream<U, V>&
operator<<(std::basic_ostream<U, V>& os, const std::tuple<Args...>& t);

template<typename K, typename V, typename T, typename U >
std::basic_ostream<T, U>&
operator<<(std::basic_ostream<T, U>& os, const std::pair<K, V>& pair);

template<typename T>
size_t
DumpMemoryUsages(std::ostream& os, tString varName, const std::vector<T>& vect);

template<typename T>
size_t
DumpMemoryUsages(std::ostream& os, tString varName, const std::queue<T>& queue);

template<typename K, typename V>
size_t
DumpMemoryUsages(std::ostream& os, tString varName, const std::map<K, V>& map);

template<typename T>
size_t
DumpMemoryUsages(std::ostream& os, tString varName, const std::set<T>& vect);

template<typename... Args >
size_t
DumpMemoryUsages(std::ostream& os, tString varName, const std::tuple<Args...>& t);

template<typename K, typename V>
size_t
DumpMemoryUsages(std::ostream& os, tString varName, const std::pair<K, V>& pair);

size_t
DumpMemoryUsages(std::ostream& os, tString varName, const tString& val);

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
        os << ele;
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
        os << ele.second;
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
        os << ele;
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
        os << x << " ";
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

template <typename T>
inline size_t
DumpMemoryUsages(std::ostream &os, tString varName, const std::vector<T> &vect)
{
    size_t size = 0;
    if (!varName.empty())
        os << quoteString(varName) << ": ";

    os << "{\"type\":\"vector\",\"numelem\":" << vect.size() << ",\"capacity\":" << vect.capacity() << ",\"elements\":[";

    bool first = true;
    size += vect.capacity() * sizeof(T);
    for (auto && elem : vect) {
        if (!first)
            os << ",";
        size += DumpMemoryUsages(os, "", elem);
        first = false;
    }

    os << "],\"Consumed\":" << size << "}";
    return size;
}

template <typename T>
inline size_t
DumpMemoryUsages(std::ostream &os, tString varName, const std::queue<T> &queue)
{
    size_t size = 0;
    if (!varName.empty())
        os << quoteString(varName) << ": ";

    auto nvect = queue;

    os << "{\"type\":\"queue\",\"size\":" << nvect.size() << ",\"elements\":[";
    bool first = true;
    size = queue.size() * sizeof(T);
    while(nvect.size()) {
        if (!first) os << ",";
        auto elem = nvect.front();
        nvect.pop();
        size += DumpMemoryUsages(os, "", elem);
        first = false;
    }
    os << "],\"Consumed\":" << size << "}";
    return size;
}

template <typename K, typename V>
inline size_t
DumpMemoryUsages(std::ostream &os, tString varName, const std::map<K, V> &map)
{
    size_t size = 0;

    if (!varName.empty())
        os << quoteString(varName) << ": ";
    os << "{\"type\":\"map\",\"numelem\":" << map.size() << ",\"elements\":[";
    bool first = true;
    size += map.size() * (sizeof(K) + sizeof(V));
    for (auto elem : map) {
        if (!first)
            os << ",";
        os << "{";
        size += DumpMemoryUsages(os, "key", elem.first);
        os << ",";
        size += DumpMemoryUsages(os, "val", elem.second);
        os << "}";
        first = false;
    }
    os << "],\"Consumed\":" << size << "}";
    return size;
}

template <typename T>
inline size_t
DumpMemoryUsages(std::ostream &os, tString varName, const std::set<T> &vect)
{
    size_t size = 0;
    if (!varName.empty())
        os << quoteString(varName) << ": ";

    os << "{\"type\":\"set\",\"numelem\":" << vect.size() << ",\"elements\":[";

    bool first = true;
    size += vect.size() * sizeof(T);
    for (auto elem : vect) {
        if (!first)
            os << ",";
        size += DumpMemoryUsages(os, "", elem);
        first = false;
    }

    os << "],\"Consumed\":" << size << "}";
    return size;
}

template <typename... Args>
inline size_t
DumpMemoryUsages(std::ostream &os, tString varName, const std::tuple<Args...> &t)
{

    size_t size = 0;

    if (!varName.empty())
        os << quoteString(varName) << ": ";

    os << "{\"type\":\"tuple\",\"elements\":[";

    auto cnt = 0;
    bool first = true;
    for_each_in_tuple(t, [&](const auto& x) {
        if (!first) os << ",";
        size += sizeof(x);
        size += DumpMemoryUsages(os, "", x);
        first = false;
        cnt += 1;
    });

    os << "],\"capacity\":" << cnt << ",\"Consumed\":" << size << "}";
    return size;
}

template <typename K, typename V>
inline size_t
DumpMemoryUsages(std::ostream &os, tString varName, const std::pair<K, V> &pair)
{
    size_t size = 0;
    if (!varName.empty())
        os << quoteString(varName) << ": ";

    os << "{\"type\":\"pair\",\"elements\":{";
    size += sizeof(K);
    size += DumpMemoryUsages(os, "key", pair.first);
    os << ",";
    size += sizeof(V);
    size += DumpMemoryUsages(os, "val", pair.second);
    os << "},\"Consumed\":" << size << "}";
    return size;
}

#endif // COMMON_PLATFORM_CONTAINERSTREAMING_HH_
