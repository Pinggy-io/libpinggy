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

#ifndef __SRC_CPP_SDK_SDKEXCEPTION_HH__
#define __SRC_CPP_SDK_SDKEXCEPTION_HH__

#include <platform/platform.h>
#include <platform/SharedPtr.hh>

namespace sdk
{

class WebDebuggerException: public std::exception, public virtual pinggy::SharedObject
{
public:
    WebDebuggerException(tString message) : message(message){}
    virtual ~WebDebuggerException() {};

    virtual const char*
    what() const noexcept override      { return message.c_str(); }

private:
    tString                     message;
};

class RemoteForwardingException: public std::exception, public virtual pinggy::SharedObject
{
public:
    RemoteForwardingException(tString message) : message(message){}
    virtual ~RemoteForwardingException() {};

    virtual const char*
    what() const noexcept override      { return message.c_str(); }

private:
    tString                     message;
};


} // namespace sdk


#endif // __SRC_CPP_SDK_SDKEXCEPTION_HH__
