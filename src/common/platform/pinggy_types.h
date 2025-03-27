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


#ifndef COMMON_PINGGY_TYPES_H_
#define COMMON_PINGGY_TYPES_H_



#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif

#define ENUM_ELEM(pref, name) \
        pref##name

#define TO_STR(_X) #_X

#define ENUM_STRING_ELEM(pref, name) \
        TO_STR(pref##name)

#define CREATE_ENUM_STRINGS_ARRAY(name, pref, S) \
    const char *name##String[] = { \
        S(pref, ENUM_STRING_ELEM) \
    };

#define CREATE_ENUM_STRING_DECLARE(name, pref, S) \
    typedef enum { \
        S(pref, ENUM_ELEM) \
    } name;\
    \
    extern const char *name##String[];






/*
 * Hello
 */
#endif /* COMMON_PINGGY_TYPES_H_ */
