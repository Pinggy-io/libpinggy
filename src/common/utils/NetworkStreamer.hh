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

#ifndef __SRC_CPP_PRIVATE_COMMON_UTILS_NETWORKSTREAMER_HH__
#define __SRC_CPP_PRIVATE_COMMON_UTILS_NETWORKSTREAMER_HH__

#include <platform/SharedPtr.hh>
#include <ostream>
#include <streambuf>
#include <cstddef>

// Forward-declare net::NetworkConnection
namespace net {
    DeclareClassWithSharedPtr(NetworkConnection);
}

DeclareClassWithSharedPtr(NetStreamBuf);

class NetOStream : public std::ostream {
    NetStreamBufPtr             buf;

public:
    explicit NetOStream(net::NetworkConnectionPtr);
};

//
// LogCallbackAwareOStream
//
// Ostream that captures bytes from a <<-chain into an internal fixed-size
// stack buffer and fires a registered C function pointer at end-of-line
// (`endl` / `'\n'` / `sync()`).

class LogCallbackAwareOStream : public std::ostream {
public:
    typedef void (*CallbackFn)(void *user_data, int level, const char *message);

    LogCallbackAwareOStream(CallbackFn cb, void *user_data, int level);

private:
    class StreamBuf : public std::streambuf {
    public:
        StreamBuf(CallbackFn cb, void *ud, int level);

    protected:
        int             overflow(int ch) override;
        std::streamsize xsputn(const char* s, std::streamsize n) override;
        int             sync() override;

    private:
        void            flush_line();
        CallbackFn      cb_;
        void           *ud_;
        int             level_;
        char            buf_[1024];
        size_t          pos_;
    };

    StreamBuf                   buf_;
};

#endif // __SRC_CPP_PRIVATE_COMMON_UTILS_NETWORKSTREAMER_HH__