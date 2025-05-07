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


#include <cstdlib>
#include <climits>
#include <platform/assert_pinggy.h>
#include <platform/platform.h>
#include "CertificateFileDetail.hh"

#ifdef __WINDOWS_OS__
static std::string getRealPath(const std::string& path) {
    char rPath[MAX_PATH];
    DWORD length = GetFullPathNameA(path.c_str(), MAX_PATH, rPath, nullptr);

    // Check if the function succeeded and if the path fits in the buffer
    Assert(length > 0 && length < MAX_PATH);

    return std::string(rPath);
}
#else
static std::string getRealPath(std::string path) {
    char rPath[PATH_MAX];
    auto ret = realpath(path.c_str(), rPath);
    Assert(ret);
    return std::string(rPath);
}
#endif

CertificateFileDetail::CertificateFileDetail(std::string keyPath,
        std::string certPath) : keyPath(keyPath), certPath(certPath)
{
    keyRealPath = getRealPath(keyPath);
    certRealPath = getRealPath(certPath);
    auto certPathModTime = std::filesystem::last_write_time(certRealPath);
    auto keyPathModTime = std::filesystem::last_write_time(keyRealPath);
    lastMod = MAX(certPathModTime, keyPathModTime);
}

CertificateFileDetail::~CertificateFileDetail()
{
}

bool CertificateFileDetail::IsModified(bool update)
{
    auto keyPathModTime = std::filesystem::last_write_time(keyRealPath);
    auto certPathModTime = std::filesystem::last_write_time(certRealPath);
    auto ret  = (lastMod < keyPathModTime || lastMod < certPathModTime);
    if (ret && update) {
        lastMod = MAX(certPathModTime, keyPathModTime);
    }
    return ret;
}

