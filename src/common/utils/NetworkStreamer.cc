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


INCLUDE_MEMORY_DUMP_DEFINITION
