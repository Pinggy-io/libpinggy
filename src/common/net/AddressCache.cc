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

#include "AddressCache.hh"

namespace net
{


std::shared_ptr<AddressCache>
AddressCache::GetInstance()
{
    // A function-local static is initialised exactly once, even when several
    // threads get here together. It is allocated on the heap and never freed,
    // so a caller's thread that is still running a tunnel while the process
    // exits can never see a destroyed cache.
    static auto *instance = new std::shared_ptr<AddressCache>(new AddressCache());
    return *instance;
}

std::optional<sock_addrinfo>
AddressCache::GetAddrInfo(tString host, tString port, bool tcp)
{
    auto key = std::tuple(host, port, tcp);
    std::lock_guard<std::mutex> guard(lock);
    auto entry = addrInfoMap.find(key);
    if (entry == addrInfoMap.end())
        return std::nullopt;
    return entry->second;
}

void
AddressCache::SetAddrInfo(tString host, tString port, bool tcp, sock_addrinfo addr)
{
    auto key = std::tuple(host, port, tcp);
    addr.cached = 1;
    std::lock_guard<std::mutex> guard(lock);
    addrInfoMap[key] = addr;
}

} // namespace net

INCLUDE_MEMORY_DUMP_DEFINITION
