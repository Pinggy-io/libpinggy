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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <string.h>

#include <algorithm>
#include <cctype>

#include "StringUtils.hh"


/// @brief Split a string
/// @param str original string
/// @param deli delimeter
/// @param limit No token parse before stoping
/// @param keep whether or not keep the token
/// @return Splitted string
std::vector<std::string> SplitString(std::string str, std::string deli,
                                     int16_t limit, bool keep, bool empty)
{
    auto valueStr = str;
    std::string::size_type start = 0;
    std::string::size_type end = valueStr.find(deli);
    std::vector<std::string> ret;
    std::string::size_type len = 0;
    for(std::string::size_type l = limit; end != std::string::npos && l;) {
        len = end - start;
        if(keep)
            len += deli.length();
        if (empty || len) {
            ret.push_back(valueStr.substr(start, len));
            l--;
        }
        start = end + deli.length();
        end = valueStr.find(deli, start);
    }

    if (start < valueStr.length()) {
        while(!empty && end != std::string::npos) {
            len = end - start;
            if (len > 0)
                break;
            start = end + deli.length();
            end = valueStr.find(deli, start);
        }
        ret.push_back(valueStr.substr(start));
    }

    return ret;
}

std::string JoinString(std::vector<std::string> strings, std::string deli)
{
    std::string joinedString;
    bool addDeli = false;
    for(auto str : strings) {
        if (addDeli)
            joinedString += deli;
        joinedString += str;

        addDeli = true;
    }
    return joinedString;
}

std::string StripString(const std::string& str) {
    // Find the first non-whitespace character
    size_t start = 0;
    while (start < str.length() && std::isspace(str[start])) {
        start++;
    }

    // Find the last non-whitespace character
    size_t end = str.length();
    while (end > start && std::isspace(str[end - 1])) {
        end--;
    }

    // Return the substring without leading and trailing whitespaces
    return str.substr(start, end - start);
}

std::string StringToUpper(std::string str)
{
    std::string dest = str;
    std::transform(dest.begin(), dest.end(), dest.begin(), ::toupper);
    return dest;
}

std::string StringToLower(std::string str)
{
    std::string dest = str;
    std::transform(dest.begin(), dest.end(), dest.begin(), ::tolower);
    return dest;
}

bool EndsWith (std::string const &fullString, std::string const &ending, bool caseSensitive) {
    if (fullString.length() >= ending.length()) {
        if (caseSensitive)
            return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
        auto haystack = fullString.c_str();
        auto needle = ending.c_str();
        return strncasecmp(haystack + (fullString.length() - ending.length()), needle, ending.length());
    } else {
        return false;
    }
}

bool StartsWith (std::string const &fullString, std::string const &starting, bool caseSensitive) {
    if (fullString.length() >= starting.length()) {
        if (caseSensitive)
            return (0 == fullString.compare (0, starting.length(), starting));
        auto haystack = fullString.c_str();
        auto needle = starting.c_str();
        return strncasecmp(haystack, needle, starting.length()) == 0; //needle is definitly null terminated
    } else {
        return false;
    }
}


std::vector<std::string> ShlexSplitString(const std::string &input)
{
    std::vector<std::string> tokens;
    size_t i = 0;
    size_t len = input.length();

    try {
        while (i < len) {
            // Skip leading whitespace
            while (i < len && std::isspace(input[i])) {
                i++;
            }
            if (i >= len) break;

            // Detect if the token starts with a quote
            char quote_char = '\0';
            if (input[i] == '\'' || input[i] == '"') {
                quote_char = input[i];
                i++;
            }

            std::string token;
            bool in_quotes = (quote_char != '\0');

            while (i < len && (in_quotes || !std::isspace(input[i]))) {
                if (input[i] == '\\') {
                    // Handle escape sequences
                    i++;
                    if (i < len) {
                        token += input[i++];
                    } else {
                        throw ShlexError("Trailing backslash at the end of input");
                    }
                } else if (in_quotes && input[i] == quote_char) {
                    // Closing quote
                    in_quotes = false;
                    i++;
                    break;
                } else if (!in_quotes && (input[i] == '\'' || input[i] == '"')) {
                    // Start of a new quote while not in a quoted token
                    break;
                } else {
                    // Regular character
                    token += input[i++];
                }
            }

            if (in_quotes) {
                throw ShlexError("Unmatched quote detected: " + std::string(1, quote_char));
            }

            // Only add non-empty tokens
            if (!token.empty()) {
                tokens.push_back(token);
            }

            // Skip trailing whitespace
            while (i < len && std::isspace(input[i])) {
                i++;
            }
        }
    } catch (const std::exception &) {
        // std::cerr << "Error during shlex split: " << e.what() << std::endl;
        throw;
    }

    return tokens;
}
bool CaseInsensitiveStringCompare(std::string const &a, std::string const &b) {
    std::string aLower, bLower;
    aLower.resize(a.size());
    bLower.resize(b.size());
    std::transform(a.begin(), a.end(), aLower.begin(), ::tolower);
    std::transform(b.begin(), b.end(), bLower.begin(), ::tolower);
    return aLower == bLower;
}

tString StringReplace(tString orig, tString search, tString replacement)
{
    size_t pos = 0;
    while ((pos = orig.find(search, pos)) != std::string::npos) {
        orig.replace(pos, search.length(), replacement);
        pos += replacement.length();  // Move past the last replacement
    }
    return orig;
}

bool CaseInsensitiveStringComparator::operator ()(const std::string &a,
        const std::string &b) const
{
    // Convert strings to lowercase for comparison
    std::string aLower, bLower;
    aLower.resize(a.size());
    bLower.resize(b.size());
    std::transform(a.begin(), a.end(), aLower.begin(), ::tolower);
    std::transform(b.begin(), b.end(), bLower.begin(), ::tolower);
    return aLower < bLower;
}

