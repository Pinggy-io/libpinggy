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

#include "PathRegistry.hh"
#include "Serialization.hh"
#include "Deserialization.hh"
#include <platform/SharedPtr.hh>
#include <memory>
#include <utils/Json.hh>
#include <utils/Utils.hh>
#include <string>


PathRegistry::PathRegistry() : currentId(ROOT_PATH_ID), dirty(false)
{
}

PathRegistry::~PathRegistry()
{
}

tPathId
PathRegistry::RegisterPath(const tString &basepath, tValueType valType, tPathId parent)
{
    PathDefinitionPtr pathDef;
    std::vector<tString> path = {};
    if (basepath != "") {
        path = getPath(parent);
        path.push_back(basepath);
    }

    auto key = std::tuple(basepath, parent, valType);
    if (pathToPathDefinition.find(key) == pathToPathDefinition.end()) {
        currentId += 1;
        pathDef                         = NewPathDefinitionPtr();
        pathDef->Path                   = path;
        pathDef->PathId                 = currentId;
        pathDef->Parent                 = parent;
        pathDef->ValType                = valType;
        pathDef->Basename               = basepath;
        idToPathDefinition[currentId]   = pathDef;
        pathToPathDefinition[key]       = pathDef;
        newlyAddedPaths.push_back(pathDef);
    } else {
        pathDef = pathToPathDefinition[key];
    }
    return pathDef->PathId;
}

void
PathRegistry::UpdatePathRegistryFromStream(RawDataPtr stream, bool mismatchedEndianness)
{
    std::set<std::pair<tPathId, tPathId>> newPathIds;
    while(stream && stream->Len) {

        auto pathDef = NewPathDefinitionPtr();
        tString pathString;

        Deserialize_Lit(stream, pathDef->PathId, mismatchedEndianness);
        Deserialize_Lit(stream, pathString, mismatchedEndianness);
        Deserialize_Lit(stream, pathDef->Parent, mismatchedEndianness);
        Deserialize_Lit(stream, pathDef->ValType, mismatchedEndianness);
        Deserialize_Lit(stream, pathDef->Basename, mismatchedEndianness);

        if (!pathString.empty()) {
            pathDef->Path = SplitString(pathString.substr(1), ".");
        }

        {
            auto key                        = std::tuple(pathDef->Basename, pathDef->PathId, pathDef->ValType);
            auto pathId                     = pathDef->PathId;
            idToPathDefinition[pathId]      = pathDef;
            pathToPathDefinition[key]       = pathDef;
            // LOGE("newPath: ", pathDef->PathId, pathString, (int)pathDef->ValType);
        }
        newPathIds.insert(std::pair(pathDef->PathId, pathDef->Parent));
    }

    for(auto newPath : newPathIds) {
        if(newPath.second > ROOT_PATH_ID) {
            auto parentPathDef = idToPathDefinition[newPath.second];
            parentPathDef->Children.insert(newPath.first);
        }
    }
}

tString
PathRegistry::DumpPaths()
{
    json jdata;
    for(auto item : pathToPathDefinition) {
        auto pathDef = item.second;
        auto key = std::to_string(pathDef->PathId);

        tString pathStr = "";
        if (!pathDef->Path.empty())
            pathStr = "." + JoinString(pathDef->Path, ".");


        jdata[key] = {
                        {"pathVector",  pathDef->Path},
                        {"basename",    pathDef->Basename},
                        {"parent",      pathDef->Parent},
                        {"valType",     pathDef->ValType},
                        {"path",        pathStr},
                        {"children",    pathDef->Children}
                    };
    }
    return jdata.dump();
}

std::vector<tString>
PathRegistry::getPath(tPathId parent)
{
    if(parent <= ROOT_PATH_ID)
        return {};
    auto pathDef = idToPathDefinition.at(parent);
    return pathDef->Path;
}

PathDefinitionPtr
PathRegistry::GetPathDefForId(tPathId pathId)
{
    return idToPathDefinition.at(pathId);
}

RawDataPtr
PathRegistry::GetNClearNewlyAddedPath(bool mismatchedEndianness)
{
    if (newlyAddedPaths.size() == 0)
        return nullptr;

    auto rawData = NewRawDataPtr();

    Serialize_Lit(rawData, (uint8_t)MsgType_Type, mismatchedEndianness);

    for (auto pathDef : newlyAddedPaths) {
        tString pathString = "";
        if (!pathDef->Path.empty()) {
            pathString  = "." + JoinString(pathDef->Path, ".");
        }

        Serialize_Lit(rawData, pathDef->PathId, mismatchedEndianness);
        Serialize_Lit(rawData, pathString, mismatchedEndianness);
        Serialize_Lit(rawData, pathDef->Parent, mismatchedEndianness);
        Serialize_Lit(rawData, pathDef->ValType, mismatchedEndianness);
        Serialize_Lit(rawData, pathDef->Basename, mismatchedEndianness);
        // LOGE("newPath: ", pathDef->PathId, pathString, (int)pathDef->ValType);
    }

    newlyAddedPaths.clear();
    return rawData;
}

RawDataPtr
PathRegistry::GetAllPaths(bool mismatchedEndianness)
{
    if (pathToPathDefinition.size() == 0)
        return nullptr;

    auto rawData = NewRawDataPtr();

    Serialize_Lit(rawData, (uint8_t)MsgType_Type, mismatchedEndianness);
    for(auto item : pathToPathDefinition) {
        auto pathDef = item.second;

        tString pathString = "";
        if (!pathDef->Path.empty()) {
            pathString  = "." + JoinString(pathDef->Path, ".");
        }

        Serialize_Lit(rawData, pathDef->PathId, mismatchedEndianness);
        Serialize_Lit(rawData, pathString, mismatchedEndianness);
        Serialize_Lit(rawData, pathDef->Parent, mismatchedEndianness);
        Serialize_Lit(rawData, pathDef->ValType, mismatchedEndianness);
        Serialize_Lit(rawData, pathDef->Basename, mismatchedEndianness);
    }

    return rawData;
}
