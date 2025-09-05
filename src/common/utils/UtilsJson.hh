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

// This is a separate file just to separte json from original utils

#ifndef __SRC_COMMON_UTILS_UTILSJSON_HH__
#define __SRC_COMMON_UTILS_UTILSJSON_HH__

#include "Utils.hh"
#include "JsonH.hh"

// lets write export import for urlptr
void to_json(nlohmann::json& nlohmann_json_j, const UrlPtr &nlohmann_json_t);
void from_json(const nlohmann::json& nlohmann_json_j, UrlPtr &nlohmann_json_t);


#endif // __SRC_COMMON_UTILS_UTILSJSON_HH__