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


#ifndef SRC_CPP_COMMON_LOGC_H_
#define SRC_CPP_COMMON_LOGC_H_

#ifdef __cplusplus
#error log.h should not be included from a cpp. Use Log.hh instead
#endif

#define __ALREADY_LOG_H_INCLUDED__

#ifdef __ALREADY_LOG_HH_INCLUDED__
#error "don't mix log.h and Log.hh"
#endif

#include "Log.hh"


#ifndef __ALREADY_LOG_HH_INCLUDED__
#error "don't mix log.h and Log.hh"
#endif

#ifdef __WINDOWS_OS__
void
c_log(char* fl, int type, char* fmt, ...);
#else
void
c_log(char *fl, int type, char *fmt, ...) __attribute__ ((__format__(printf, 3, 4)));
#endif


#define APP_2STR_(x) #x
#define APP_2STR(x) APP_2STR_(x)


#if LOG_LEVEL <= LogLevelTrace
    #define LOGT(fmt, ...) c_log(__FILE__ ":" APP_2STR(__LINE__), LogLevelTrace, fmt, ##__VA_ARGS__)
#else
    #define LOGT(...) //__LOG(nullLoggerSink,  "TRACE:: ")
    // #define LOGTE(...) //__LOG(nullLoggerSink,  "TRACE:: ")
#endif

#if LOG_LEVEL <= LogLevelDebug
    #define LOGD(fmt, ...) c_log(__FILE__ ":" APP_2STR(__LINE__), LogLevelDebug, fmt, ##__VA_ARGS__)
#else
    #define LOGD(...) //__LOG(nullLoggerSink,  "DEBUG:: ")
    // #define LOGDE(...) //__LOG(nullLoggerSink,  "DEBUG:: ")
#endif

#if LOG_LEVEL <= LogLevelInfo
    #define LOGI(fmt, ...) c_log(__FILE__ ":" APP_2STR(__LINE__), LogLevelInfo, fmt, ##__VA_ARGS__)
#else
    #define LOGI(x) //__LOG(nullLoggerSink,  " INFO:: ")
    // #define LOGIE(x) //__LOG(nullLoggerSink,  " INFO:: ")
#endif

#if LOG_LEVEL <= LogLevelError
    #define LOGE(fmt, ...) c_log(__FILE__ ":" APP_2STR(__LINE__), LogLevelError, fmt, ##__VA_ARGS__)
#else
    #define LOGE(...)// __LOG(nullLoggerSink,  "ERROR:: ")
    // #define LOGEE(...)// __LOG(nullLoggerSink,  "ERROR:: ")
#endif

#if LOG_LEVEL <= LogLevelFatal
    #define LOGF(fmt, ...) c_log(__FILE__ ":" APP_2STR(__LINE__), LogLevelFatal, fmt, ##__VA_ARGS__)
#else
    #define LOGF(...) //__LOG(nullLoggerSink,  "FATAL:: ")
    // #define LOGFE(...) //__LOG(nullLoggerSink,  "FATAL:: ")
#endif


#define LOGTE(fmt, ...) LOGT("err: %d %s : " fmt, app_get_errno(), app_get_strerror(app_get_errno()), ##__VA_ARGS__)
#define LOGDE(fmt, ...) LOGD("err: %d %s : " fmt, app_get_errno(), app_get_strerror(app_get_errno()), ##__VA_ARGS__)
#define LOGIE(fmt, ...) LOGI("err: %d %s : " fmt, app_get_errno(), app_get_strerror(app_get_errno()), ##__VA_ARGS__)
#define LOGEE(fmt, ...) LOGE("err: %d %s : " fmt, app_get_errno(), app_get_strerror(app_get_errno()), ##__VA_ARGS__)
#define LOGFE(fmt, ...) LOGF("err: %d %s : " fmt, app_get_errno(), app_get_strerror(app_get_errno()), ##__VA_ARGS__)

#define LOGTF(fd, fmt, ...) LOGT("fd: %d err: %d %s : " fmt, fd, app_get_errno(), app_get_strerror(app_get_errno()), ##__VA_ARGS__)
#define LOGDF(fd, fmt, ...) LOGD("fd: %d err: %d %s : " fmt, fd, app_get_errno(), app_get_strerror(app_get_errno()), ##__VA_ARGS__)
#define LOGIF(fd, fmt, ...) LOGI("fd: %d err: %d %s : " fmt, fd, app_get_errno(), app_get_strerror(app_get_errno()), ##__VA_ARGS__)
#define LOGEF(fd, fmt, ...) LOGE("fd: %d err: %d %s : " fmt, fd, app_get_errno(), app_get_strerror(app_get_errno()), ##__VA_ARGS__)
#define LOGFF(fd, fmt, ...) LOGF("fd: %d err: %d %s : " fmt, fd, app_get_errno(), app_get_strerror(app_get_errno()), ##__VA_ARGS__)

#endif //SRC_CPP_COMMON_LOGC_H_
