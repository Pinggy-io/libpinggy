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


#include "Utils.hh"
#include <string>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#ifdef __WINDOWS_OS__
#include <filesystem>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif
#include <ctime>
#include <regex>
#include <platform/assert_pinggy.h> //platform
#include <platform/Log.hh> //platform

static std::regex urlRegex = std::regex(R"(^(?:(\w+):\/\/)?([^\/:#?]+)(?::(\d+))?([^?#]*)(?:\?([^#]*))?(?:#(.*))?$)");
static std::regex urlRegexIpv6 = std::regex("^(.*?):\\/\\/(\\[?[^\\]/]+\\]?)?(?::([0-9]+))?(\\/.*)?$");

#if 0
Url::Url(std::string aurl): port(0) {

    std::smatch match;
    auto url = aurl;
    if (std::regex_match(url, match, urlRegexIpv6)) {
    }else if (std::regex_match(url, match, urlRegex)) {
    } else {
        throw std::invalid_argument("Invalid URL format");
    }

    protocol = match[1];
    host = match[2];
    portStr = match[3];
    path = match[4];
    query = match[5];
    fragment = match[6];

    if(protocol.empty())
        protocol = "http";
    if(portStr.empty())
        portStr = "80";
    port = std::stoi(portStr);
}

#elif 1
Url::Url(std::string url): port(0) {
    std::string nurl = url;
    std::transform(nurl.begin(), nurl.end(), nurl.begin(), [](unsigned char c){ return std::tolower(c); });
    protocol = "http";
    port = 80;
    portStr = "80";
    query = "";
    path = "/";
    std::string::size_type pos = 0;
    auto cloneSlashPos = nurl.find("://");
    if(cloneSlashPos != std::string::npos) {
        protocol = url.substr(pos, cloneSlashPos);
        pos += cloneSlashPos + 3;
    }
    auto colonpos = nurl.find(":", pos);
    auto slashpos = nurl.find("/", pos);
    auto querypos = nurl.find("?", pos);
    auto hostEnd = MIN(colonpos, MIN(slashpos, querypos));
    auto v6str = nurl.find("[", pos);
    if (v6str != nurl.npos && v6str == pos) { //IP address cannot start after it
        auto v6end = nurl.find("]", pos);
        if(v6end == nurl.npos)
            throw std::invalid_argument("Invalid URL format");
        hostEnd = v6end+1;
        colonpos = nurl.find(":", hostEnd);
    }
    host = url.substr(pos, hostEnd - pos);
    if(host.length())
        pos += host.length();
    if(colonpos < slashpos && colonpos < querypos) {
        portStr = url.substr(colonpos+1, MIN(slashpos, querypos) - colonpos);
        port = std::stoi(portStr);
        portStr = std::to_string(port);
        pos = MIN(slashpos, querypos);
    }
    if(pos != std::string::npos && querypos > pos) { //Let sanitize path
        // path = nurl.substr(pos, querypos-pos);
        bool lastSlash = false;
        path = "";
        for(auto i = pos; i < querypos && i < nurl.length(); i++) {
            if (nurl[i] == '/') {
                if (lastSlash)
                    continue;
                lastSlash = true;
            } else {
                lastSlash = false;
            }
            path += url[i];
        }
    }
    if(querypos != std::string::npos)
        query = url.substr(querypos);
    if(path.length() == 0) {
        path = "/";
    }
}
#else
#include <httpparser/urlparser.h>
Url::Url(std::string url): port(0) {
    httpparser::UrlParser urlParser;
    Assert(urlParser.parse(url));
    protocol = urlParser.scheme();
    host = urlParser.hostname();
    auto port_s = urlParser.port();
    port = port_s == "" ? 0 : std::stoi(port_s);
    path = urlParser.path();
    query = urlParser.query();
}
#endif //#if 0

Url::~Url() {
}

std::ostream &operator<<(std::ostream &os, const UrlPtr &url)
{
    os << url->ToString();
    return os;
}

#ifdef __WINDOWS_OS__
FsPath CreateTemporaryDirectory(const std::string &templat) {
    // Get the path to the temporary directory as a wide string
    wchar_t tempPath[MAX_PATH];
    if (GetTempPathW(MAX_PATH, tempPath) == 0) {
        throw std::runtime_error("Failed to get temp path");
    }

    // Convert the template to a wide string and construct the full path
    std::wstring wideTemplat(templat.begin(), templat.end());
    FsPath tmpDirPath = FsPath(tempPath) / wideTemplat;

    // Create the directory
    if (!CreateDirectoryW(tmpDirPath.c_str(), NULL)) {
        throw std::runtime_error("Failed to create temporary directory");
    }

    // Return the path as an FsPath object
    return tmpDirPath;
}

bool DeleteDirTree(FsPath dirPath) {
    try {
        // Iterate over the directory and remove each file or subdirectory
        for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
            std::filesystem::remove_all(entry.path());
        }

        // Remove the main directory itself
        std::filesystem::remove(dirPath);

        return true;
    } catch (const std::filesystem::filesystem_error&) {
        // Handle errors (e.g., log them)
        return false;
    }
}
#else
static std::string FindTempDir()
{
    const char *tmpDir = std::getenv("TMPDIR");
    if (tmpDir == NULL || tmpDir[0] == 0)
        tmpDir = "/tmp";
    return std::string(tmpDir);
}

FsPath CreateTemporaryDirectory(std::string templat)
{

    std::string tmpDirPath = FindTempDir();
    tmpDirPath += "/" + templat;
    char *tmp = new char[tmpDirPath.length() + 2]; //a buffer
    std::strcpy(tmp, tmpDirPath.c_str());
    auto dirPath = std::string(mkdtemp(tmp));
    delete[] tmp;
    return FsPath(dirPath);
}

bool DeleteDirTree(FsPath dirPathArg)
{
    std::string dirPath = dirPathArg.string();
    DIR *dirp = opendir(dirPath.c_str());
    if (dirp == NULL)
        return false;
    for(struct dirent *dir = readdir(dirp); dir; dir = readdir(dirp))
    {
        std::string tmpPath = dirPath + std::string("/") + std::string(dir->d_name);
        remove(tmpPath.c_str());
    }
    closedir(dirp);
    if(rmdir(dirPath.c_str()) == -1)
        return false;
    return true;
}
#endif //__WINDOWS_OS__
