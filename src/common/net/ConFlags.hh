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

#ifndef __SRC_CPP_PUBLIC_COMMON_NET_CONFLAGS_HH__
#define __SRC_CPP_PUBLIC_COMMON_NET_CONFLAGS_HH__

namespace net
{


enum ConnectionFlags_Public { //Not required in public
    ConFlag_Reserved_1          = 1 << 0,
    ConFlag_Reserved_2          = 1 << 1,
    ConFlag_Reserved_3          = 1 << 2,
    ConFlag_Reserved_4          = 1 << 3,
    ConFlag_Reserved_5          = 1 << 4,
    ConFlag_Reserved_6          = 1 << 5,
    ConFlag_Reserved_7          = 1 << 6,
    ConFlag_Reserved_8          = 1 << 7,
    ConFlag_AcceptSocket        = 1 << 8,
    ConFlag_Reserved_9          = 1 << 9,

    ConFlag_Reserved_15         = 1 << 15,
    ConFlag_Reserved_16         = 1 << 16,
    ConFlag_Reserved_17         = 1 << 17,
    ConFlag_Reserved_18         = 1 << 18,

};

} // namespace net

#endif // __SRC_CPP_PUBLIC_COMMON_NET_CONFLAGS_HH__