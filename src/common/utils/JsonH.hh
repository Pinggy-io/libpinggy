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


#ifndef SRC_CPP_COMMON_JSONH_HH_
#define SRC_CPP_COMMON_JSONH_HH_

#include <json/nlohmann_json_fwd.hh>

using json = nlohmann::json;


#define NLOHMANN_DECLARE_TYPE_NON_INTRUSIVE_CUSTOME(Type, ...)  \
    void to_json(nlohmann::json& nlohmann_json_j, const Type& vType); \
    void from_json(const nlohmann::json& nlohmann_json_j, Type& vType);

#define NLOHMANN_DECLARE_TYPE_NON_INTRUSIVE_CUSTOME_PTR(Type, ...)  \
    void to_json(nlohmann::json& nlohmann_json_j, const Type##Ptr nlohmann_json_t); \
    void from_json(const nlohmann::json& nlohmann_json_j, Type##Ptr nlohmann_json_t);

#endif /* SRC_CPP_COMMON_JSONH_HH_ */
