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

#ifndef __SRC_CPP_PUBLIC_COMMON_NET_SSLNETCONNBIO_HH__
#define __SRC_CPP_PUBLIC_COMMON_NET_SSLNETCONNBIO_HH__
#include <openssl/ssl.h>
#include "NetworkConnection.hh"
#include <platform/Log.hh>

namespace net
{

BIO *
netConnBioNewBio(NetworkConnectionPtr netConn);

void
setNetConnOrigFunc(BIO *bio, tString func);

#define SET_NET_CONN_ORIG_FUNC(bio) {LOGE("asd: ", __func__); setNetConnOrigFunc(bio, __func__);}


} // namespace net



#endif // __SRC_CPP_PUBLIC_COMMON_NET_SSLNETCONNBIO_HH__
