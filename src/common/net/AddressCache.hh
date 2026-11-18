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

#ifndef __SRC_COMMON_NET_ADDRESSCACHE_HH__
#define __SRC_COMMON_NET_ADDRESSCACHE_HH__

#include <platform/platform.h>
#include <platform/network.h>
#include <platform/SharedPtr.hh>
#include <map>
#include <mutex>
#include <optional>

namespace net
{

class AddressCache: virtual public pinggy::SharedObject
{
public:
    // Delete copy constructor and assignment operator to prevent copies
    AddressCache(const AddressCache&) = delete;
    AddressCache& operator=(const AddressCache&) = delete;

    // Static method to access the single instance. The cache is shared by every
    // tunnel in the process, and each tunnel runs on its own thread, so this and
    // the two functions below can be called from several threads at once.
    static std::shared_ptr<AddressCache>
    GetInstance();

    std::optional<sock_addrinfo>
    GetAddrInfo(tString host, tString port, bool tcp = true);

    void
    SetAddrInfo(tString host, tString port, bool tcp, sock_addrinfo addr);

    DefineMandatoryClassFunctionsWOSuper(AddressCache);

private:
    AddressCache()              { }

    std::map<std::tuple<tString, tString, bool>, sock_addrinfo>
                                addrInfoMap;
    std::mutex                  lock; // guards addrInfoMap
};
DefineMakeSharedPtr(AddressCache);


} // namespace net

#endif // __SRC_COMMON_NET_ADDRESSCACHE_HH__
