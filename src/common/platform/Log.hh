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


#ifndef SRC_CPP_COMMON_LOG_H_
#define SRC_CPP_COMMON_LOG_H_

#ifdef __cplusplus
#include <string>
#include <cstring>
#include <fstream>
#include <chrono>
#include <iostream>
#include <sstream>
#endif //__cplusplus

#include <stdlib.h>
#include "platform.h"
#include "app_foreach_macro.h"

#ifdef __ALREADY_LOG_H_INCLUDED__
#define __ALREADY_LOG_HH_INCLUDED__
#endif

#define LogLevelTrace 1
#define LogLevelDebug 2
#define LogLevelInfo 3
#define LogLevelError 4
#define LogLevelFatal 5
#define LogLevelDisable 100


#ifndef LOG_LEVEL
#define LOG_LEVEL LogLevelTrace
#endif


#define APP_CONVERT_TO_STRING_(x) #x
#define APP_CONVERT_TO_STRING(x) APP_CONVERT_TO_STRING_(x)



#ifdef __cplusplus

extern std::ofstream            __PINGGY_LOGGER_SINK__;
extern std::string              __PINGGY_LOG_PREFIX__;
extern pid_t                    __PINGGY_LOG_PID__;
extern bool                     __PINGGY_GLOBAL_ENABLED__;


#define __LTIME \
    (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count())

#define __LOG(x__, __y) do{ \
    if (__PINGGY_GLOBAL_ENABLED__) { \
        auto ttime = __LTIME; \
        (x__.is_open()?x__:std::cout) << ttime << ":: " __FILE__ ":" \
            APP_CONVERT_TO_STRING(__LINE__) << " " << __PINGGY_LOG_PREFIX__ << "(" << __PINGGY_LOG_PID__ << ")::" __y std::endl; \
    } \
}while(0)

#define __LOG_FL(FL__, x__, __y) do{ \
    if (__PINGGY_GLOBAL_ENABLED__) { \
        auto ttime = __LTIME; \
        (x__.is_open()?x__:std::cout) << ttime << ":: " << FL__ << " " << __PINGGY_LOG_PREFIX__ << "(" << __PINGGY_LOG_PID__ << ")::" __y std::endl; \
    } \
} while(0)


#define LOG_C(FL, mode, ...) __LOG_FL(FL, __PINGGY_LOGGER_SINK__,  mode __EXPAND_LOGS__(__VA_ARGS__))

//#define EXPAND_ARG(x) " " #x ": `" << x << "`"
#define _EXPAND_LOG(x) " " << x <<
#define _EXPAND_LOG_VAR(x) " " #x ": `" << x << "`" <<

#define __EXPAND_LOGS__(...) APP_MACRO_FOR_EACH(_EXPAND_LOG, APP_MACRO_DUMMY, __VA_ARGS__)
#define __EXPAND_LOG_VARS__(...) APP_MACRO_FOR_EACH(_EXPAND_LOG_VAR, APP_MACRO_DUMMY, __VA_ARGS__)

#define UNUSED(x) (void)(x);
#define __EXPAND_UNUSED_VARS__(...)


#if LOG_LEVEL <= LogLevelTrace
    #define LOGT(...) __LOG(__PINGGY_LOGGER_SINK__,  "TRACE:: " __EXPAND_LOGS__(__VA_ARGS__))
#else
    #define LOGT(...)
#endif

#if LOG_LEVEL <= LogLevelDebug
    #define LOGD(...) __LOG(__PINGGY_LOGGER_SINK__,  "DEBUG:: " __EXPAND_LOGS__(__VA_ARGS__))
#else
    #define LOGD(...)
#endif

#if LOG_LEVEL <= LogLevelInfo
    #define LOGI(...) __LOG(__PINGGY_LOGGER_SINK__,  "INFO:: " __EXPAND_LOGS__(__VA_ARGS__))
#else
    #define LOGI(...)
#endif

#if LOG_LEVEL <= LogLevelError
    #define LOGE(...) __LOG(__PINGGY_LOGGER_SINK__,  "ERROR:: " __EXPAND_LOGS__(__VA_ARGS__))
#else
    #define LOGE(...)
#endif

#if LOG_LEVEL <= LogLevelFatal
    #define LOGF(...) __LOG(__PINGGY_LOGGER_SINK__,  "FATAL:: " __EXPAND_LOGS__(__VA_ARGS__))
    #define LOGFC(FL, ...) __LOG_FL(FL, __PINGGY_LOGGER_SINK__,  "FATAL:: " __EXPAND_LOGS__(__VA_ARGS__))
#else
    #define LOGF(...)
    #define LOGFC(...)
#endif

//====

#if LOG_LEVEL <= LogLevelTrace
    #define LOGTV(...) __LOG(__PINGGY_LOGGER_SINK__,  "TRACE:: " __EXPAND_LOG_VARS__(__VA_ARGS__))
    #define LOGSSLT(...) __LOGSSL(LogLevelTrace, __VA_ARGS__)
#else
    #define LOGTV(...)
    #define LOGSSLT(...)
#endif

#if LOG_LEVEL <= LogLevelDebug
    #define LOGDV(...) __LOG(__PINGGY_LOGGER_SINK__,  "DEBUG:: " __EXPAND_LOG_VARS__(__VA_ARGS__))
    #define LOGSSLD(...) __LOGSSL(LogLevelDebug, __VA_ARGS__)
#else
    #define LOGDV(...)
    #define LOGSSLD(...)
#endif

#if LOG_LEVEL <= LogLevelInfo
    #define LOGIV(...) __LOG(__PINGGY_LOGGER_SINK__,  "INFO:: " __EXPAND_LOG_VARS__(__VA_ARGS__))
    #define LOGSSLI(...) __LOGSSL(LogLevelInfo, __VA_ARGS__)
#else
    #define LOGIV(...)
    #define LOGSSLI(...)
#endif

#if LOG_LEVEL <= LogLevelError
    #define LOGEV(...) __LOG(__PINGGY_LOGGER_SINK__,  "ERROR:: " __EXPAND_LOG_VARS__(__VA_ARGS__))
    #define LOGSSLE(...) __LOGSSL(LogLevelError, __VA_ARGS__)
#else
    #define LOGEV(...)
    #define LOGSSLE(...)
#endif

#if LOG_LEVEL <= LogLevelFatal
    #define LOGFV(...) __LOG(__PINGGY_LOGGER_SINK__,  "FATAL:: " __EXPAND_LOG_VARS__(__VA_ARGS__))
    #define LOGSSLF(...) __LOGSSL(LogLevelFatal, __VA_ARGS__)
#else
    #define LOGFV(...)
    #define LOGSSLF(...)
#endif

//===
#define LOGTE(...) LOGT(app_get_errno(), app_get_strerror(app_get_errno()) << ":", ##__VA_ARGS__)
#define LOGDE(...) LOGD(app_get_errno(), app_get_strerror(app_get_errno()) << ":", ##__VA_ARGS__)
#define LOGIE(...) LOGI(app_get_errno(), app_get_strerror(app_get_errno()) << ":", ##__VA_ARGS__)
#define LOGEE(...) LOGE(app_get_errno(), app_get_strerror(app_get_errno()) << ":", ##__VA_ARGS__)
#define LOGFE(...) LOGF(app_get_errno(), app_get_strerror(app_get_errno()) << ":", ##__VA_ARGS__)

#define LOGTF(fd, ...) LOGT("fd:", fd, app_get_errno(), app_get_strerror(app_get_errno()) << ":", ##__VA_ARGS__)
#define LOGDF(fd, ...) LOGD("fd:", fd, app_get_errno(), app_get_strerror(app_get_errno()) << ":", ##__VA_ARGS__)
#define LOGIF(fd, ...) LOGI("fd:", fd, app_get_errno(), app_get_strerror(app_get_errno()) << ":", ##__VA_ARGS__)
#define LOGEF(fd, ...) LOGE("fd:", fd, app_get_errno(), app_get_strerror(app_get_errno()) << ":", ##__VA_ARGS__)
#define LOGFF(fd, ...) LOGF("fd:", fd, app_get_errno(), app_get_strerror(app_get_errno()) << ":", ##__VA_ARGS__)
//====

#ifdef NDEBUG
#define ABORT() exit(1)
#else
#define ABORT() abort()
#endif

#define ABORT_WITH_MSG(...) \
{ \
    LOGF(__VA_ARGS__); \
    ABORT(); \
}

#define __LOGSSL(typ, ...) do{\
    std::stringstream ss; \
    ss << __EXPAND_LOG_VARS__(__VA_ARGS__) ""; \
    std::string myString = ss.str(); \
    struct __PINGGY_LOG_SSL_STRUCT__ u = {(char *)__FILE__ ":" APP_CONVERT_TO_STRING(__LINE__), typ, myString}; \
    ERR_print_errors_cb(LogOpenSslErrorsCB, (void *) &u); \
    ERR_clear_error(); \
}while(0)

#ifdef __cplusplus
extern "C" {
#endif
struct __PINGGY_LOG_SSL_STRUCT__{
    char                       *fl;
    int                         type;
    std::string                 args;
};
int
LogOpenSslErrorsCB(const char *str, size_t len, void *u);
#ifdef __cplusplus
}
#endif

tString
GetLogPrefix();

void
SetLogPrefix(std::string pref);

void
InitLogWithCout();

void
InitLog(std::string path);

void
UpdatePidForLog();

void
SetGlobalLogEnable(bool enable = true);

#endif //cplusplus

#endif /* SRC_CPP_COMMON_LOG_H_ */
