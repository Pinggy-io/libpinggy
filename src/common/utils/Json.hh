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

#ifndef SRC_CPP_COMMON_JSON_HH_
#define SRC_CPP_COMMON_JSON_HH_
#include <json/nlohmann_json.hh>


using json = nlohmann::json;


//#var then data
#define _PINGGY_NLOHMANN_JSON_FROM_EXISTS(v1, t1) if(nlohmann_json_j.contains(#t1)) nlohmann_json_j.at(#t1).get_to(vType.v1);
#define _PINGGY_NLOHMANN_JSON_FROM_EXISTS_(v2) _PINGGY_NLOHMANN_JSON_FROM_EXISTS v2
#define _PINGGY_NLOHMANN_JSON_FROM_EXISTS1_(v2) _PINGGY_NLOHMANN_JSON_FROM_EXISTS(v2, v2)

#define _PINGGY_NLOHMANN_JSON_TO_EXISTS(v1, t1) nlohmann_json_j[#t1] = vType.v1;
#define _PINGGY_NLOHMANN_JSON_TO_EXISTS_(v2) _PINGGY_NLOHMANN_JSON_TO_EXISTS v2
#define _PINGGY_NLOHMANN_JSON_TO_EXISTS1_(v2) _PINGGY_NLOHMANN_JSON_TO_EXISTS(v2, v2)

#define NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_CUSTOME(Type, ...)  \
    void to_json(nlohmann::json& nlohmann_json_j, const Type& vType) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(_PINGGY_NLOHMANN_JSON_TO_EXISTS_, __VA_ARGS__)) } \
    void from_json(const nlohmann::json& nlohmann_json_j, Type& vType) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(_PINGGY_NLOHMANN_JSON_FROM_EXISTS_, __VA_ARGS__)) }

#define NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_CUSTOME1(Type, ...)  \
    void to_json(nlohmann::json& nlohmann_json_j, const Type& vType) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(_PINGGY_NLOHMANN_JSON_TO_EXISTS1_, __VA_ARGS__)) } \
    void from_json(const nlohmann::json& nlohmann_json_j, Type& vType) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(_PINGGY_NLOHMANN_JSON_FROM_EXISTS1_, __VA_ARGS__)) }

//var den data
#define _PINGGY_NLOHMANN_JSON_FROM_EXISTS_PTR(v1, t1) if(nlohmann_json_j.contains(#t1) && nlohmann_json_j[#t1] != nullptr) \
                    {\
                        try { \
                            nlohmann_json_j.at(#t1).get_to(nlohmann_json_t->v1); \
                        }catch (nlohmann::detail::exception &e) { \
                            LOGE("Error in parsing: ", #v1, " reason:", e.what()); \
                        } \
                    }
#define _PINGGY_NLOHMANN_JSON_FROM_EXISTS_PTR_(v2) _PINGGY_NLOHMANN_JSON_FROM_EXISTS_PTR v2
#define _PINGGY_NLOHMANN_JSON_FROM_EXISTS_PTR1_(v2) _PINGGY_NLOHMANN_JSON_FROM_EXISTS_PTR (v2,v2)

#define _PINGGY_NLOHMANN_JSON_TO_EXISTS_PTR(v1, t1) nlohmann_json_j[#t1] = nlohmann_json_t->v1;
#define _PINGGY_NLOHMANN_JSON_TO_EXISTS_PTR_(v2) _PINGGY_NLOHMANN_JSON_TO_EXISTS_PTR v2
#define _PINGGY_NLOHMANN_JSON_TO_EXISTS_PTR1_(v2) _PINGGY_NLOHMANN_JSON_TO_EXISTS_PTR (v2,v2)

#define _PINGGY_CREATE_PTR_IF_REQUIRED(Type, ele, args) \
    if (!ele) { \
        ele = New##Type##Ptr args; \
    } \

#define NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_CUSTOME_PTR(Type, ...)  \
    void to_json(nlohmann::json& nlohmann_json_j, const Type##Ptr &nlohmann_json_t) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(_PINGGY_NLOHMANN_JSON_TO_EXISTS_PTR_, __VA_ARGS__)) } \
    void from_json(const nlohmann::json& nlohmann_json_j, Type##Ptr &nlohmann_json_t) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(_PINGGY_NLOHMANN_JSON_FROM_EXISTS_PTR_, __VA_ARGS__)) }


#define NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_CUSTOME_PTR1(Type, ...)  \
    void to_json(nlohmann::json& nlohmann_json_j, const Type##Ptr &nlohmann_json_t) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(_PINGGY_NLOHMANN_JSON_TO_EXISTS_PTR1_, __VA_ARGS__)) } \
    void from_json(const nlohmann::json& nlohmann_json_j, Type##Ptr &nlohmann_json_t) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(_PINGGY_NLOHMANN_JSON_FROM_EXISTS_PTR1_, __VA_ARGS__)) }


#define NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_CUSTOME_NEW_PTR(Type, argsToCreateNewObject, ...) \
    void to_json(nlohmann::json& nlohmann_json_j, const Type##Ptr &nlohmann_json_t) \
    { \
        if (!nlohmann_json_t) return; \
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(_PINGGY_NLOHMANN_JSON_TO_EXISTS_PTR_, __VA_ARGS__)) \
    } \
    void from_json(const nlohmann::json& nlohmann_json_j, Type##Ptr &nlohmann_json_t) \
    { \
        _PINGGY_CREATE_PTR_IF_REQUIRED(Type, nlohmann_json_t, argsToCreateNewObject); \
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(_PINGGY_NLOHMANN_JSON_FROM_EXISTS_PTR_, __VA_ARGS__)) \
    }


#define NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_CUSTOME_NEW_PTR1(Type, argsToCreateNewObject, ...) \
    void to_json(nlohmann::json& nlohmann_json_j, const Type##Ptr &nlohmann_json_t) \
    { \
        if (!nlohmann_json_t) return; \
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(_PINGGY_NLOHMANN_JSON_TO_EXISTS_PTR1_, __VA_ARGS__)) \
    } \
    void from_json(const nlohmann::json& nlohmann_json_j, Type##Ptr &nlohmann_json_t) \
    { \
        _PINGGY_CREATE_PTR_IF_REQUIRED(Type, nlohmann_json_t, argsToCreateNewObject); \
        NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(_PINGGY_NLOHMANN_JSON_FROM_EXISTS_PTR1_, __VA_ARGS__)) \
    }



//data den var
#define _PINGGY_NLOHMANN_JSON_STR_TO_VAR_(v1, t1)  if(nlohmann_json_j.contains(#t1) && nlohmann_json_j[#t1] != nullptr) \
                    { \
                        try { \
                            nlohmann_json_j.at(#t1).get_to(v1); \
                        } catch (nlohmann::detail::exception &e) { \
                            LOGE("Error in parsing: ", #t1, " reason:", e.what()); \
                        } \
                    }
#define _PINGGY_NLOHMANN_JSON_STR_TO_VAR_2(v2) _PINGGY_NLOHMANN_JSON_STR_TO_VAR_ v2
#define _PINGGY_NLOHMANN_JSON_STR_TO_VAR_1(v2) _PINGGY_NLOHMANN_JSON_STR_TO_VAR_ (v2, v2)

#define PINGGY_NLOHMANN_JSON_STR_TO_VAR2(jdata, ...)  \
    {json jd = json::parse(jdata); PINGGY_NLOHMANN_JSON_TO_VAR2(jd, __VA_ARGS__)}
#define PINGGY_NLOHMANN_JSON_STR_TO_VAR1(jdata, ...)  \
    {json jd = json::parse(jdata); PINGGY_NLOHMANN_JSON_TO_VAR1(jd, __VA_ARGS__)}

#define PINGGY_NLOHMANN_JSON_TO_VAR2(jdata, ...)  \
    {json nlohmann_json_j = jdata; NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(_PINGGY_NLOHMANN_JSON_STR_TO_VAR_2, __VA_ARGS__))}
#define PINGGY_NLOHMANN_JSON_TO_VAR1(jdata, ...)  \
    {json nlohmann_json_j = jdata; NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(_PINGGY_NLOHMANN_JSON_STR_TO_VAR_1, __VA_ARGS__))}


//data den var
#define _PINGGY_NLOHMANN_JSON_FROM_VAR_(v1, t1)  nlohmann_json_j[#t1] = v1;
#define _PINGGY_NLOHMANN_JSON_FROM_VAR_2(v2) _PINGGY_NLOHMANN_JSON_FROM_VAR_ v2
#define _PINGGY_NLOHMANN_JSON_FROM_VAR_1(v2) _PINGGY_NLOHMANN_JSON_FROM_VAR_ (v2, v2)

#define PINGGY_NLOHMANN_JSON_FROM_VAR2(jdata, ...)  \
    {json nlohmann_json_j; NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(_PINGGY_NLOHMANN_JSON_FROM_VAR_2, __VA_ARGS__)) jdata = nlohmann_json_j;}

#define PINGGY_NLOHMANN_JSON_FROM_VAR1(jdata, ...)  \
    {json nlohmann_json_j; NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(_PINGGY_NLOHMANN_JSON_FROM_VAR_1, __VA_ARGS__)) jdata = nlohmann_json_j;}

#define PINGGY_NLOHMANN_JSON_STR_FROM_VAR2(jdata, ...)  \
    {json nlohmann_json_j; NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(_PINGGY_NLOHMANN_JSON_FROM_VAR_2, __VA_ARGS__)) jdata = nlohmann_json_j.dump();}

#define PINGGY_NLOHMANN_JSON_STR_FROM_VAR1(jdata, ...)  \
    {json nlohmann_json_j; NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(_PINGGY_NLOHMANN_JSON_FROM_VAR_1, __VA_ARGS__)) jdata = nlohmann_json_j.dump();}



//data den var
#define _PINGGY_NLOHMANN_JSON_FROM_PTR_VAR_(v1, t1)  nlohmann_json_j[#t1] = ptr->v1;
#define _PINGGY_NLOHMANN_JSON_FROM_PTR_VAR_2(v2) _PINGGY_NLOHMANN_JSON_FROM_PTR_VAR_ v2
#define _PINGGY_NLOHMANN_JSON_FROM_PTR_VAR_1(v2) _PINGGY_NLOHMANN_JSON_FROM_PTR_VAR_ (v2, v2)

#define PINGGY_NLOHMANN_JSON_FROM_PTR_VAR2(jdata, pdata, ...)  \
    {json nlohmann_json_j; auto ptr = pdata; NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(_PINGGY_NLOHMANN_JSON_FROM_PTR_VAR_2, __VA_ARGS__)) jdata = nlohmann_json_j;}

#define PINGGY_NLOHMANN_JSON_FROM_PTR_VAR1(jdata, pdata, ...)  \
    {json nlohmann_json_j; auto ptr = pdata; NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(_PINGGY_NLOHMANN_JSON_FROM_PTR_VAR_1, __VA_ARGS__)) jdata = nlohmann_json_j;}



//data den var
#define _PINGGY_NLOHMANN_JSON_TO_PTR_VAR_(v1, t1)  if(nlohmann_json_j.contains(#t1) && nlohmann_json_j[#t1] != nullptr) \
                    { \
                        try { \
                            nlohmann_json_j.at(#t1).get_to(ptr->v1); \
                        } catch (nlohmann::detail::exception &e) { \
                            LOGE("Error in parsing: ", #t1, " reason:", e.what()); \
                        } \
                    }

// #define _STRIP_PARENTHESIS(x) x
#define _PINGGY_NLOHMANN_JSON_TO_PTR_VAR_DEF_(v1, t1, d1) \
                    _PINGGY_NLOHMANN_JSON_TO_PTR_VAR_(v1, t1) \
                    else { \
                        ptr->v1 = d1;\
                    }

#define _PINGGY_NLOHMANN_JSON_TO_PTR_VAR_2(v2) _PINGGY_NLOHMANN_JSON_TO_PTR_VAR_ v2
#define _PINGGY_NLOHMANN_JSON_TO_PTR_VAR_1(v2) _PINGGY_NLOHMANN_JSON_TO_PTR_VAR_ (v2, v2)
#define _PINGGY_NLOHMANN_JSON_TO_PTR_VAR_3(v3) _PINGGY_NLOHMANN_JSON_TO_PTR_VAR_DEF_ v3

#define PINGGY_NLOHMANN_JSON_TO_PTR_VAR2(jdata, pdata, ...)  \
    {auto ptr = pdata; json nlohmann_json_j = jdata; NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(_PINGGY_NLOHMANN_JSON_TO_PTR_VAR_2, __VA_ARGS__))}
#define PINGGY_NLOHMANN_JSON_TO_PTR_VAR1(jdata, pdata, ...)  \
    {auto ptr = pdata; json nlohmann_json_j = jdata; NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(_PINGGY_NLOHMANN_JSON_TO_PTR_VAR_1, __VA_ARGS__))}
#define PINGGY_NLOHMANN_JSON_TO_PTR_VAR3(jdata, pdata, ...)  \
    {auto ptr = pdata; json nlohmann_json_j = jdata; NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(_PINGGY_NLOHMANN_JSON_TO_PTR_VAR_3, __VA_ARGS__))}


#endif //SRC_CPP_COMMON_JSON_HH_
