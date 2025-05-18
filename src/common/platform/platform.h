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

#ifndef __SRC_CPP_COMMON_PLATFORM_H__
#define __SRC_CPP_COMMON_PLATFORM_H__


//===========================================
//              Define Platform
//===========================================

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
   //define something for Windows (32-bit and 64-bit, this part is common)
#define __WINDOWS_OS__
#define __OS_32BIT
   #ifdef _WIN64
      //define something for Windows (64-bit only)
//#define __OS_64BIT
//#error "Not implemented yet 2"
   #else
      //define something for Windows (32-bit only)
//#error "Not implemented yet 3"
   #endif //_WIN64
#define IS_BIG_ENDIAN 0
#elif __APPLE__

#define __MAC_OS__

#if defined(__BIG_ENDIAN__)
#define IS_BIG_ENDIAN 1
#elif defined(__LITTLE_ENDIAN__)
#define IS_BIG_ENDIAN 0
#else
#error "Unknown endian"
#endif

#elif __linux__
    // linux
#define __LINUX_OS__

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define IS_BIG_ENDIAN 1
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define IS_BIG_ENDIAN 0
#else
#error "Unknown endian"
#endif

    #if defined(__i386__) || defined(__i386) || defined(__X86__) || defined(_M_IX86)
        // 32-bit x86 (common on Unix-based systems)
        #define __OS_32BIT
    #elif defined(__x86_64__) || defined(__amd64__) || defined(_M_X64) || defined(_M_AMD64)
        // 64-bit x86-64 (common on Unix-based systems)
        #define __OS_64BIT
    #elif defined(__arm__) && !defined(__aarch64__)
        // 32-bit ARM
        #define __OS_32BIT
    #elif defined(__aarch64__)
        // 64-bit ARM
        #define __OS_64BIT
    #endif

#elif __unix__ // all unices not caught above
#   error "Not implemented yet"
    // Unix
#elif defined(_POSIX_VERSION)
#   error "Not implemented yet"
    // POSIX
#else
#   error "Unknown compiler"
#endif //defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

//===========================================


#ifdef __WINDOWS_OS__
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <io.h>
#include <processthreadsapi.h>
// #pragma comment(lib, "Ws2_32.lib")  // Link with Ws2_32.lib
#else
#include <arpa/inet.h>
#include <unistd.h>
#include <poll.h>
#endif //__WINDOWS_OS__

#ifdef _MSC_VER
//not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif


#include <stdint.h>

typedef uint16_t        port_t;
typedef int16_t         len_t;
typedef uint16_t        chanId_t;

typedef int             tInt;
typedef int8_t          tInt8;
typedef int16_t         tInt16;
typedef int32_t         tInt32;
typedef int64_t         tInt64;
typedef uint8_t         tUint8;
typedef uint16_t        tUint16;
typedef uint32_t        tUint32;
typedef uint64_t        tUint64;


#ifdef __cplusplus
#include <string>

typedef std::string     tString;

#endif

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __WINDOWS_OS__
typedef int socklen_t;
typedef SSIZE_T ssize_t;
typedef SOCKET sock_t;
typedef DWORD pid_t;

typedef WSAPOLLFD tPollFd;

#define poll WSAPoll

pid_t app_getpid(void);

void* custom_memmem(const void* haystack, size_t haystack_len, const void* needle, size_t needle_len);

#define memmem custom_memmem

void bzero(void *s, size_t n);

int WindowsSocketInitialize();

#else

typedef int sock_t;
typedef struct pollfd tPollFd;

pid_t app_getpid(void);

int WindowsSocketInitialize();

#endif //__WINDOWS_OS__

void ignore_sigpipe();

#ifdef __cplusplus
}
#endif


#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define MAX(x,y) ((x) > (y) ? (x) : (y))

#define abstract


#include "pinggy_types.h"

#define KB 1024
#define MB (KB*KB)


#endif // SRC_CPP_COMMON_PLATFORM_H__
