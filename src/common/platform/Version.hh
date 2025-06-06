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


#ifndef COMMON_VERSTION_HH_
#define COMMON_VERSTION_HH_


#define MAKE_VERSION_FROM_HUMAN_READABLE(x, y, z)  ((x << 16) + (y << 8) + z)

#define PINGGY_COMM_VERSION \
            MAKE_VERSION_FROM_HUMAN_READABLE(1,3,4)
                                           //1.3.0

#define PINGGY_VERSION \
            MAKE_VERSION_FROM_HUMAN_READABLE(1,3,4)
                                            //1.3.1

#define PINGGY_LAST_COMPATIABLE_VERSION \
            PINGGY_COMM_VERSION

//#undef MAKE_VERSION_FROM_HUMAN_READABLE

#define PRINTABLE_VERSION(x)  (x >> 16) << "." << ((x&0xff00) >> 8) << "." << (x&0xff)

#define VERSION_STRING(x) std::to_string(x >> 16) + "." + std::to_string((x&0xff00) >> 8) + "." + std::to_string(x&0xff)

#endif /* COMMON_VERSTION_HH_ */
