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


#ifndef SRC_CPP_COMMON_UTILS_CERTIFICATEFILEDETAIL_HH_
#define SRC_CPP_COMMON_UTILS_CERTIFICATEFILEDETAIL_HH_

#include <platform/SharedPtr.hh>
#include <filesystem>

typedef std::filesystem::path FsPath;

class CertificateFileDetail: public pinggy::SharedObject
{
    FsPath keyPath;
    FsPath certPath;
    FsPath keyRealPath;
    FsPath certRealPath;
    std::filesystem::file_time_type lastMod;
public:
    CertificateFileDetail(std::string keyPath, std::string certPath);
    virtual ~CertificateFileDetail();
    std::string GetKeyPath() const { return keyPath.string(); }
    std::string GetCertPath() const { return certPath.string(); }
    bool IsModified(bool update = false);
};

DefineMakeSharedPtr(CertificateFileDetail);


#endif /* SRC_CPP_COMMON_UTILS_CERTIFICATEFILEDETAIL_HH_ */
