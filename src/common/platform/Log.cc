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


//#include <stdlib.h>
//#include <string.h>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <stdarg.h>
#include "Log.hh"



//externs
std::ofstream                   __PINGGY_LOGGER_SINK__;
std::string                     __PINGGY_LOG_PREFIX__ = "";
pid_t                           __PINGGY_LOG_PID__ = 0;
// int64_t                         __LastChrono = 0;
bool                            __PINGGY_GLOBAL_ENABLED__ = true;

//Local
std::string                     __logPath = "";

tString
GetLogPrefix()
{
    return __PINGGY_LOG_PREFIX__;
}

void
SetLogPrefix(std::string pref)
{
    __PINGGY_LOG_PREFIX__ = "::" +pref;
    __PINGGY_LOG_PID__ = app_getpid();
}

void
InitLogWithCout()
{
    // if(__PINGGY_LOGGER_SINK__.is_open()) __PINGGY_LOGGER_SINK__.close();
    __PINGGY_LOG_PID__ = app_getpid();
    __logPath = "";
}

void
InitLog(std::string path)
{
    __PINGGY_LOGGER_SINK__.open(path, std::ofstream::out|std::ofstream::app);
    __logPath = path;
    __PINGGY_LOG_PID__ = app_getpid();
}

void
SetGlobalLogEnable(bool enable)
{
    __PINGGY_GLOBAL_ENABLED__ = enable;
}

void
UpdatePidForLog()
{
    __PINGGY_LOG_PID__ = app_getpid();
}


extern "C" {
char                            __LOG_BUF_[4096]; //4KB This should be good enough
                                                  //also this is not thread safe
void
c_log(char *fl, int type, const char *fmt, ...)
{
    va_list args;

    // Get the number of characters needed for the formatted string
    va_start(args, fmt);
    vsnprintf(__LOG_BUF_, sizeof(__LOG_BUF_), fmt, args);
    va_end(args);

    switch (type)
    {
    case LogLevelTrace:
        LOG_C(fl, "TRACE:: ", __LOG_BUF_);
        break;
    case LogLevelDebug:
        LOG_C(fl, "DEBUG:: ", __LOG_BUF_);
        break;
    case LogLevelInfo:
        LOG_C(fl, " INFO:: ", __LOG_BUF_);
        break;
    case LogLevelError:
        LOG_C(fl, "ERROR:: ", __LOG_BUF_);
        break;
    case LogLevelFatal:
        LOG_C(fl, "FATAL:: ", __LOG_BUF_);
        break;
    default:
        break;
    }
}

int
LogOpenSslErrorsCB(const char *str, size_t len, void *u)
{
    struct __PINGGY_LOG_SSL_STRUCT__ *flt = (struct __PINGGY_LOG_SSL_STRUCT__*)u;
    // char *fl = (char *)u;
    c_log(flt->fl, flt->type, "%s: %s", flt->args.c_str(), str);
    return len;
}

} // extern "C"
