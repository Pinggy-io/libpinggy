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


#ifndef SRC_CPP_COMMON_ASSERT_HH_
#define SRC_CPP_COMMON_ASSERT_HH_

#include <stdlib.h>

#ifdef __cplusplus
#include "Log.hh"
#else
#include "log.h"
#endif

#ifdef NDEBUG
#define __END_PINGGY_PROC__()
#else
#define __END_PINGGY_PROC__() abort()
#endif

#define __assert__pinggy__(x) { \
    if (!x) { \
        LOGF("Assertion failed: " #x); \
        __END_PINGGY_PROC__(); \
    } \
}

#define Assert(x) __assert__pinggy__((x))

#endif /* SRC_CPP_COMMON_ASSERT_HH_ */
