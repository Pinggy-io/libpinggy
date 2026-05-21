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

#include "NetworkStreamer.hh"
#include <net/NetworkConnection.hh>     
#include <cstdio>                       // for EOF
#include "TemplateStreaming.hh" //this needs to be the last include

class NetStreamBuf :
        public std::streambuf,
        public pinggy::SharedObject
{
public:
    NetStreamBuf(net::NetworkConnectionPtr w) : writer(w)
                                { }

    DefineMandatoryFileLocalClassFunctionsWOSuper(NetStreamBuf);

protected:
    int
    overflow(int ch) override
    {
        if (ch == EOF) {
            return !EOF;
        }
        char c = static_cast<char>(ch);
        if (writer->Write(&c, 1, MSG_WAITALL) <= 0) {
            return EOF; // Indicate write error
        }
        return ch; // Indicate success
    }

    std::streamsize
    xsputn(const char* s, std::streamsize n) override
                                { return writer->Write(s, n, MSG_WAITALL); }

    int
    sync() override             { return flush_buffer() ? 0 : -1; }

private:
    bool
    flush_buffer()              { return true; }

    net::NetworkConnectionPtr   writer;
};

DefineMakeSharedPtr(NetStreamBuf);


NetOStream::NetOStream(net::NetworkConnectionPtr netConn)
        : std::ostream(nullptr)
{
    buf = NewNetStreamBufPtr(netConn);
    rdbuf(buf.get());
}

LogCallbackAwareOStream::StreamBuf::StreamBuf(CallbackFn cb, void *ud, int level)
        : cb_(cb), ud_(ud), level_(level), pos_(0)
{ }

int
LogCallbackAwareOStream::StreamBuf::overflow(int ch)
{
    if (ch == EOF) return !EOF;
    if (pos_ >= sizeof(buf_) - 1) flush_line();
    buf_[pos_++] = static_cast<char>(ch);
    if (ch == '\n') flush_line();
    return ch;
}

std::streamsize
LogCallbackAwareOStream::StreamBuf::xsputn(const char *s, std::streamsize n)
{
    for (std::streamsize i = 0; i < n; ++i) {
        if (pos_ >= sizeof(buf_) - 1) flush_line();
        buf_[pos_++] = s[i];
        if (s[i] == '\n') flush_line();
    }
    return n;
}

int
LogCallbackAwareOStream::StreamBuf::sync()
{
    flush_line();
    return 0;
}

void
LogCallbackAwareOStream::StreamBuf::flush_line()
{
    if (pos_ == 0 || !cb_) { pos_ = 0; return; }
    // Strip trailing newline so consumers don't get double-newlines.
    size_t n = (buf_[pos_-1] == '\n') ? pos_-1 : pos_;
    buf_[n] = '\0';
    cb_(ud_, level_, buf_);
    pos_ = 0;
}

LogCallbackAwareOStream::LogCallbackAwareOStream(CallbackFn cb, void *ud, int level)
        : std::ostream(nullptr), buf_(cb, ud, level)
{
    rdbuf(&buf_);
}


INCLUDE_MEMORY_DUMP_DEFINITION
