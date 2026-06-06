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
#error "PrimitiveStreaming.hh is internal; include <platform/SharedPtr.hh> instead."
#endif

#ifndef COMMON_PLATFORM_PRIMITIVESTREAMING_HH_
#define COMMON_PLATFORM_PRIMITIVESTREAMING_HH_

#include "platform.h"
#include <platform/Log.hh>
#include <string>
#include <string_view>
#include <type_traits>
#include <ostream>
#include <iostream>
#include <cstring>

template <typename T>
struct pinggy_is_bool : std::false_type {};

template <>
struct pinggy_is_bool<bool> : std::true_type {};

template <typename T, typename = void>
struct pinggy_is_complete : std::false_type {};

template <typename T>
struct pinggy_is_complete<T, decltype(void(sizeof(T)))> : std::true_type {};

inline tString
quoteString(tString var)
{
    return "\"" + var + "\"";
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
DumpMemoryUsages(std::ostream &os, tString varName, const T *t)
{
    ABORT_WITH_MSG("Not allowed");
    return 0;
}

template <typename T>
inline void
DumpValue(std::ostream &os, const T &val)
{
    if constexpr (std::is_same_v<T, tString>) {
        if (val.length() < 16) {
            os << val;
        } else {
            os << "<string len=" << val.length() << ">";
        }
    } else if constexpr (std::is_pointer_v<T> && std::is_same_v<std::remove_cv_t<std::remove_pointer_t<T>>, char>) {
        if (val && std::strlen(val) < 16) {
            os << val;
        } else if (val) {
            os << "<char* len=" << std::strlen(val) << ">";
        } else {
            os << "nullptr";
        }
    } else if constexpr (std::is_function_v<T>) {
        os << "<func>";
    } else if constexpr (std::is_pointer_v<T>) {
        if constexpr (std::is_function_v<std::remove_pointer_t<std::decay_t<T>>> || std::is_function_v<std::remove_reference_t<T>>) {
            os << "<ptr>";
        } else {
            os << static_cast<const void*>(val);
        }
    } else if constexpr (std::is_same_v<T, tInt8>) {
        os << static_cast<int>(val);
    } else if constexpr (std::is_same_v<T, tUint8>) {
        os << static_cast<tUint>(val);
    } else if constexpr (pinggy_is_bool<T>::value) {
        os << (val ? "true" : "false");
    } else if constexpr (std::is_arithmetic_v<T>) {
        os << val;
    } else {
        os << "unknown";
    }
}

#endif // COMMON_PLATFORM_PRIMITIVESTREAMING_HH_
