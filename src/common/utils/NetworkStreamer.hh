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

#include <net/NetworkConnection.hh>
#include <ostream>

DeclareClassWithSharedPtr(NetStreamBuf);

class NetOStream : public std::ostream {
    NetStreamBufPtr             buf;

public:
    explicit NetOStream(net::NetworkConnectionPtr);
};


#endif // __SRC_CPP_PRIVATE_COMMON_UTILS_NETWORKSTREAMER_HH__