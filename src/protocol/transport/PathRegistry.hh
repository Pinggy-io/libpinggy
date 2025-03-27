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

#ifndef SRC_CPP_PINGGYTRANSPORT_PATHREGISTRY_HH_
#define SRC_CPP_PINGGYTRANSPORT_PATHREGISTRY_HH_

#include <memory>
#include <platform/pinggy_types.h>
#include "TransportCommon.hh"
#include <unordered_map>
#include <map>
#include <vector>
#include <set>



struct PathDefinition {
    tPathId                     PathId;
    tString                     Basename;
    std::vector<tString>        Path;
    std::set<tPathId>           Children;
    tPathId                     Parent;
    tValueType                  ValType;
    PathDefinition(): PathId(ROOT_PATH_ID), Parent(0), ValType(ValueType_Invalid)
                                { }
};
DefineMakeSharedPtr(PathDefinition);

DeclareClassWithSharedPtr(PathRegistry);

inline bool
operator<(const std::pair<tString, tPathId>& lhs, const std::pair<tString, tPathId>& rhs)
{
    if (lhs.second == rhs.second) {
        return lhs.first < lhs.first;
    }
    return lhs.second < rhs.second;
}

inline bool
operator==(const std::pair<tString, tPathId>& lhs, const std::pair<tString, tPathId>& rhs)
{
    return lhs.second == rhs.second && lhs.first == rhs.first;
}

// typedef std::shared_ptr<std::map<std::pair<tString, tPathId>, PathDefinitionPtr>> tPathToPathDefinitionPtr;
// typedef std::shared_ptr<std::unordered_map<tPathId, PathDefinitionPtr>> tIdToPathDefinitionPtr;


typedef std::map<std::tuple<tString, tPathId, tValueType>, PathDefinitionPtr>
                                tPathToPathDefinition;
typedef std::unordered_map<tPathId, PathDefinitionPtr>
                                tIdToPathDefinition;

class PathRegistry: virtual public pinggy::SharedObject
{
private:
    PathRegistry();

    tPathToPathDefinition       pathToPathDefinition;
    tIdToPathDefinition         idToPathDefinition;
    std::vector<PathDefinitionPtr>
                                newlyAddedPaths;
    uint16_t                    currentId;
    bool                        dirty;

    std::vector<tString>
    getPath(tPathId parent);

    friend class                TransportManager;

public:
    virtual
    ~PathRegistry();

    tPathId
    RegisterPath(const tString &path, tValueType valueType, tPathId parent);

    void
    UpdatePathRegistryFromStream(RawDataPtr stream, bool mismatchedEndianness);

    RawDataPtr
    GetNClearNewlyAddedPath(bool mismatchedEndianness);

    RawDataPtr
    GetAllPaths(bool mismatchedEndianness);

    PathDefinitionPtr
    GetPathDefForId(tPathId pathId);

    tString
    DumpPaths();
};

DefineMakePrivateSharedPtr(PathRegistry);


#endif // SRC_CPP_PINGGYTRANSPORT_PATHREGISTRY_HH_
