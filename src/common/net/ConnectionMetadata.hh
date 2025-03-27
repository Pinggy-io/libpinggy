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

#ifndef __SRC_CPP_COMMON_NET_CONNECTIONMETADATA_HH__
#define __SRC_CPP_COMMON_NET_CONNECTIONMETADATA_HH__

#include <net/NetworkConnection.hh>

namespace net
{

extern "C" {
struct AddressMetadata {
    ip_addr                     LocalIp;
    ip_addr                     RemoteIp;
    tUint16                     LocalPort;
    tUint16                     RemotePort;
    //GO is interested upto this
    tUint32                     Flags;
};

#define mAddStaticAssert_Struct(_s, _m, _o) \
    static_assert(offsetof(_s, _m) == _o, \
            "Offset of `" APP_CONVERT_TO_STRING(_m) "` in `" APP_CONVERT_TO_STRING(_s) "` is incorrect")

static_assert(sizeof(ip_addr) == 16, "Size of in6_addr must be 16 bytes");
mAddStaticAssert_Struct(struct AddressMetadata, LocalIp, 0);
mAddStaticAssert_Struct(struct AddressMetadata, RemoteIp, 16);
mAddStaticAssert_Struct(struct AddressMetadata, LocalPort, 32);
mAddStaticAssert_Struct(struct AddressMetadata, RemotePort, 34);
mAddStaticAssert_Struct(struct AddressMetadata, Flags, 36);
struct ConnectionMetadata {
    char                        Identifier[8];
    AddressMetadata             AddrMetadata;
    tUint16                     ConnType;
    tUint16                     ServerNameLen;
    char                        ServerName[256];
    char                        _padding[204];
};

mAddStaticAssert_Struct(struct ConnectionMetadata, Identifier, 0);
mAddStaticAssert_Struct(struct ConnectionMetadata, AddrMetadata, 8);
mAddStaticAssert_Struct(struct ConnectionMetadata, ConnType, 48);
mAddStaticAssert_Struct(struct ConnectionMetadata, ServerNameLen, 50);
mAddStaticAssert_Struct(struct ConnectionMetadata, ServerName, 52);
static_assert(sizeof(struct ConnectionMetadata) == NETWORK_METADATA_SIZE, "Sizeof meta data must be 512");
}

const AddressMetadata
GetAddressMetadata(NetworkConnectionPtr);

const ConnectionMetadata
GetConnectionMetadata(NetworkConnectionPtr);

} // namespace net

#endif // __SRC_CPP_COMMON_NET_CONNECTIONMETADATA_HH__
