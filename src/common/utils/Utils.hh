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


#ifndef COMMON_UTILS_HH_
#define COMMON_UTILS_HH_

#include <iostream>
#include <filesystem>
#include <memory>
#include <vector>
#include "RawData.hh"
#include "StringUtils.hh"
#include "CertificateFileDetail.hh"

enum ENDIAN_NESS {
    ENDIAN_UNKNOWN = 0,
    ENDIAN_LITTLE,
    ENDIAN_BIG
};

template< typename T >
struct ArrayDeleter
{
  void operator ()( T const * p)
  {
    delete[] p;
  }
};

template< typename T >
struct NoDeleter
{
  void operator ()( T const * p)
  {
      (void)p;
  }
};

class Url {
public:
    Url(tString url, int defaultPort = 80, tString defaultProto = "http");

    virtual
    ~Url();

    tString
    ToString()                  { return protocol + "://" + GetHost() + ":" + std::to_string(port) + path; }

    tString
    GetSockAddrString()         { return GetHost() + ":" + portStr; }

    tString
    GetHost()                   { return (host.empty() || !ipv6) ? host : "["+host+"]"; }

    const tString&
    GetRawHost()                { return host; }

    const tString&
    GetPath() const             { return path; }

    port_t
    GetPort() const             { return port; }

    tString
    GetPortStr() const          { return portStr; }

    const tString&
    GetProtocol() const         { return protocol; }

    const tString&
    GetQuery() const            { return query; }

    void
    SetHost(tString host)       { this->host = host; }

    void
    SetPath(tString path)       { this->path = path; }

    void
    SetPort(port_t port)        { this->port = port; this->portStr = std::to_string(port); }

    void
    SetProtocol(tString protocol)
                                { this->protocol = protocol; }

    void
    SetQuery(tString query)     { this->query = query; }



private:
    tString                     protocol;
    tString                     host;
    port_t                      port;
    tString                     portStr;
    tString                     path;
    tString                     query;
    bool                        ipv6;
};

DefineMakeSharedPtr(Url);

std::ostream&
operator<<(std::ostream& os, const UrlPtr& url);

FsPath
CreateTemporaryDirectory(tString templat="util-temp-XXXXX");

bool
DeleteDirTree(FsPath dirPath);

#endif /* SERVER_UTILS_HH_ */
