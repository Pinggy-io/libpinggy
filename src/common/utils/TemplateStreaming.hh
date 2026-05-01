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
#include <platform/Log.hh>
#include <platform/PinggyWriter.hh>
#include <string> // For tString (std::string)
#include <type_traits>
#include <cstdio>
#include <cstring>

template <typename T, typename = void>
struct pinggy_is_complete : std::false_type {};

template <typename T>
struct pinggy_is_complete<T, decltype(void(sizeof(T)))> : std::true_type {};

template <typename T>
struct pinggy_is_bool : std::false_type {};

template <>
struct pinggy_is_bool<bool> : std::true_type {};

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

//======

inline tString
quoteString(tString var)
{
    return "\"" + var + "\"";
}

template <typename T>
inline size_t
DumpMemoryUsages(std::ostream &os, tString varName, const std::vector<T> &vect)
{
    size_t size = 0; //= sizeof(vect);
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
    size_t size = 0; //sizeof(queue);
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
    size_t size = 0;// sizeof(map);

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
    size_t size = 0; //sizeof(vect);
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

    size_t size = 0; // sizeof(t);

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
    size_t size = 0; // sizeof(pair);
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

template<typename T>
void __print_primitive_value(std::ostream &os, const T& value) {
    if constexpr (std::is_pointer_v<T>) {
        os << ", \"value\":" << static_cast<const void*>(value);
    } else if constexpr (std::is_arithmetic_v<T>) {
        os << ", \"value\":" << value;
    }
}

#define __pp_ff(T) \
inline constexpr std::string_view \
_p_type_name(const T &x) { return TO_STR(T); } \
inline constexpr std::string_view \
_p_type_name(const T * &x) { return "*" TO_STR(T); }
PINGGY_PRIMITIVES_TYPES(__pp_ff)

template<typename T>
inline constexpr std::string_view
_p_type_name(const T &x) { if constexpr (std::is_function_v<T>) return "function"; return ""; }

inline constexpr std::string_view
_p_type_name(const bool &x) { return "bool"; }

template <typename T>
inline size_t
DumpMemoryUsages(std::ostream &os, tString varName, const T &val)
{
    if (!varName.empty())
        os << quoteString(varName) << ": ";
    size_t size = 0;

    os << "{\"type\":\"primitive<" << _p_type_name(val) << ">\",\"size\":" << sizeof(val);

    if constexpr (std::is_pointer_v<T> && std::is_same_v<std::remove_cv_t<std::remove_pointer_t<T>>, char>) {
        if (val && std::strlen(val) < 16) {
            os << ", \"value\":" << val;
        }
    } else if constexpr (std::is_function_v<T>) {
        os << "";
    } else if constexpr (std::is_pointer_v<T>) {
        if constexpr (std::is_function_v<std::remove_pointer_t<std::decay_t<T>>> || std::is_function_v<std::remove_reference_t<T>>) {
            os << "";
        } else {
            os << ", \"value\":\"" << static_cast<const void*>(val) << "\"";\
        }
    } else if constexpr (std::is_same_v<T, tInt8>) {
        std::cout << ", \"value\":" << static_cast<int>(val);
    } else if constexpr (std::is_same_v<T, tUint8>) {
        std::cout << ", \"value\":" << static_cast<tUint>(val);
    } else if constexpr (pinggy_is_bool<T>::value) {
        os << ", \"value\":" << (val ? "true" : "false");
    } else if constexpr (std::is_arithmetic_v<T>) {
        os << ", \"value\":" << val;
    }

    os << "}";
    return size;
}

template <typename T>
inline size_t
__dumpMemoryUsagesImpl(std::ostream &os, tString varName, const std::shared_ptr<T> &ptr, std::true_type)
{
    size_t size = 0; // sizeof(std::shared_ptr<T>);
    if (!varName.empty())
        os << quoteString(varName) << ": ";

    if (ptr) {
        if (!ptr->IsMemoryDumpingAllowed()) {
            os << "\"<already accounted (" << static_cast<const void*>(ptr.get()) << ")>\"";
            return size;
        }
        size += ptr->MemberClsSize();
        os << "{\"type\":\"shared_ptr<" << ptr->MemberClsName();
        os << "(" << static_cast<const void*>(ptr.get()) << ")>\",\"content\":";
        size += ptr->DumpMemory(os);
        os << "}";
    } else {
        os << "null";
    }

    return size;
}

template <typename T>
inline size_t
__dumpMemoryUsagesImpl(std::ostream &os, tString varName, const std::shared_ptr<T> &ptr, std::false_type)
{
    size_t size = 0; // as it is incomplete
    if (!varName.empty())
        os << quoteString(varName) << ": ";

    if (ptr)
        os << "{\"type\":\"shared_ptr<incompleteType(" << static_cast<const void*>(ptr.get()) <<")>\"}";
    else
        os << "null";
    return size;
}

template <typename T>
inline size_t
DumpMemoryUsages(std::ostream &os, tString varName, const std::shared_ptr<T> &ptr)
{
    return __dumpMemoryUsagesImpl(os, varName, ptr, pinggy_is_complete<T>{});
}

template <typename T>
inline size_t
DumpMemoryUsages(std::ostream &os, tString varName, const std::weak_ptr<T> &wptr)
{
    auto aptr = wptr.lock();
    size_t size = 0;
    if (aptr)
        size += sizeof(aptr);
    size += DumpMemoryUsages(os, varName, aptr);
    return size;
}

template <typename T>
inline size_t DumpMemoryUsages(std::ostream &os, tString varName, const T *t)
{
    ABORT_WITH_MSG("Not allowed");
    return 0;
}

//==============

template <typename T>
inline void
DumpValue(std::ostream &os, const T &val)
{
    if constexpr (std::is_same_v<T, tString>) { // Handle tString (std::string)
        if (val.length() < 16) {
            os << val;
        } else {
            os << "<string len=" << val.length() << ">"; // Indicate long char*
        }
    } else if constexpr (std::is_pointer_v<T> && std::is_same_v<std::remove_cv_t<std::remove_pointer_t<T>>, char>) { // C-style string
        if (val && std::strlen(val) < 16) {
            os << val;
        } else if (val) {
            os << "<char* len=" << std::strlen(val) << ">"; // Indicate long char*
        } else {
            os << "nullptr";
        }
    } else if constexpr (std::is_function_v<T>) {
        os << "<func>";
    } else if constexpr (std::is_pointer_v<T>) { // Generic pointer
        if constexpr (std::is_function_v<std::remove_pointer_t<std::decay_t<T>>> || std::is_function_v<std::remove_reference_t<T>>) {
            os << "<ptr>";
        } else {
            os << static_cast<const void*>(val);
        }
    } else if constexpr (std::is_same_v<T, tInt8>) { // Specific integer types
        os << static_cast<int>(val);
    } else if constexpr (std::is_same_v<T, tUint8>) { // Specific unsigned integer types
        os << static_cast<tUint>(val);
    } else if constexpr (pinggy_is_bool<T>::value) {
        os << (val ? "true" : "false");
    } else if constexpr (std::is_arithmetic_v<T>) {
        os << val;
    } else {
        os << "unknown";
    }
}

template <typename T>
inline void
DumpValue(std::ostream &os, const std::shared_ptr<T> &ptr)
{
    if (ptr) {
        // Assuming RawData is a type that has a .size() method for its content
        // and that RawDataPtr is std::shared_ptr<RawData>.
        // This requires RawData to be a complete type where this template is instantiated.
        if constexpr (std::is_same_v<T, RawData>) { // Special handling for RawDataPtr
            os << "<RawDataPtr at " << static_cast<const void*>(ptr.get()) << ", size=" << ptr->Len << ">";
        } else {
            // Generic shared_ptr handling
            os << "<shared_ptr to " << typeid(T).name() << " at " << static_cast<const void*>(ptr.get()) << ">";
        }
    } else {
        os << "nullptr";
    }
}

template <typename T>
inline void
DumpValue(const RawDataPtr &rawData, const T &val)
{
    if constexpr (std::is_same_v<T, tString>) {
        if (val.length() < 16) {
            rawData->AddData(val.c_str(), val.size());
        } else {
            auto s = "<string len=" + std::to_string(val.length()) + ">";
            rawData->AddData(s.c_str(), s.size());
        }
    } else if constexpr (std::is_pointer_v<T> && std::is_same_v<std::remove_cv_t<std::remove_pointer_t<T>>, char>) {
        if (val) {
            auto len = std::strlen(val);
            if (len < 16) {
                rawData->AddData(val, len);
            } else {
                auto s = "<char* len=" + std::to_string(len) + ">";
                rawData->AddData(s.c_str(), s.size());
            }
        } else {
            rawData->AddData("nullptr", 7);
        }
    } else if constexpr (std::is_function_v<T>) {
        rawData->AddData("<func>", 6);
    } else if constexpr (std::is_pointer_v<T>) {
        if constexpr (std::is_function_v<std::remove_pointer_t<std::decay_t<T>>>
                   || std::is_function_v<std::remove_reference_t<T>>) {
            rawData->AddData("<ptr>", 5);
        } else {
            char buf[32];
            auto n = std::snprintf(buf, sizeof(buf), "%p", static_cast<const void*>(val));
            rawData->AddData(buf, static_cast<size_t>(n));
        }
    } else if constexpr (std::is_same_v<T, tInt8>) {
        auto s = std::to_string(static_cast<int>(val));
        rawData->AddData(s.c_str(), s.size());
    } else if constexpr (std::is_same_v<T, tUint8>) {
        auto s = std::to_string(static_cast<unsigned int>(val));
        rawData->AddData(s.c_str(), s.size());
    } else if constexpr (pinggy_is_bool<T>::value) {
        if (val) rawData->AddData("true", 4); else rawData->AddData("false", 5);
    } else if constexpr (std::is_arithmetic_v<T>) {
        auto s = std::to_string(val);
        rawData->AddData(s.c_str(), s.size());
    } else {
        rawData->AddData("unknown", 7);
    }
}

template <typename T>
inline void
DumpValue(const RawDataPtr &rawData, const std::shared_ptr<T> &ptr)
{
    if (ptr) {
        char buf[64];
        int n;
        if constexpr (std::is_same_v<T, RawData>) {
            n = std::snprintf(buf, sizeof(buf), "<RawData size=%d>", static_cast<int>(ptr->Len));
        } else {
            n = std::snprintf(buf, sizeof(buf), "<shared_ptr at %p>",
                              static_cast<const void*>(ptr.get()));
        }
        rawData->AddData(buf, static_cast<size_t>(n));
    } else {
        rawData->AddData("nullptr", 7);
    }
}



#endif // __SRC_CPP_PUBLIC_COMMON_UTILS_TEMPLATELOGGING_HH__
