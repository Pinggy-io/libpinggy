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

#include "platform.h"

#ifdef __WINDOWS_OS__
#include <stddef.h>
#include <string.h>
#include <stdio.h>

void* custom_memmem(const void* haystack, size_t haystack_len, const void* needle, size_t needle_len)
{
    if (needle_len == 0) {
        return (void*)haystack;  // An empty needle is always found at the start
    }

    if (haystack_len < needle_len) {
        return NULL;  // If the haystack is smaller than the needle, no match is possible
    }

    const char* h = (const char*)haystack;
    const char* n = (const char*)needle;

    for (size_t i = 0; i <= haystack_len - needle_len; ++i) {
        if (memcmp(h + i, n, needle_len) == 0) {
            return (void*)(h + i);  // Return a pointer to the start of the match
        }
    }

    return NULL;  // No match found
}

void bzero(void *s, size_t n)
{
    memset(s, 0, n);
}


pid_t app_getpid(void)
{
    return GetCurrentProcessId();
}

int WindowsSocketInitialize()
{
#ifdef __WINDOWS_OS__
    WSADATA wsaData;
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }
#endif
    return 0;
}

void ignore_sigpipe()
{
}

#endif //__WINDOWS_OS__