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

#include "ConnectionMetadata.hh"

namespace net
{


const AddressMetadata
GetAddressMetadata(NetworkConnectionPtr netConn)
{
    auto localAddr  = netConn->GetLocalAddress();
    auto remoteAddr = netConn->GetPeerAddress();
    AddressMetadata metadata;
        metadata.LocalIp    = localAddr->GetAddr();
        metadata.RemoteIp   = remoteAddr->GetAddr();
        metadata.LocalPort  = htons(localAddr->GetPort());
        metadata.RemotePort = htons(remoteAddr->GetPort());
        metadata.Flags      = netConn->Flags();

    return metadata;
}

const ConnectionMetadata
GetConnectionMetadata(NetworkConnectionPtr netConn)
{
    auto serverName = netConn->GetServerName();
    if (serverName.length() > METADATA_URL_SIZE_RELAY) {
        serverName = serverName.substr(0, METADATA_URL_SIZE_RELAY);
    }
    ConnectionMetadata metadata;
        metadata.AddrMetadata  = GetAddressMetadata(netConn);
        metadata.ConnType      = 0;
        metadata.ServerNameLen = htons((tUint16)serverName.length());

    strncpy(metadata.Identifier, (char *)"PINGGY", 8);
    memcpy(metadata.ServerName, serverName.c_str(), METADATA_URL_SIZE_RELAY);
    return metadata;
}

} // namespace net

