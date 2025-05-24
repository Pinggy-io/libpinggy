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
std::vector<tString>
SplitString(tString str, tString deli,
            int16_t limit, bool keep, bool empty)
{
    auto valueStr = str;
    tString::size_type start = 0;
    tString::size_type end = valueStr.find(deli);
    std::vector<tString> ret;
    tString::size_type len = 0;
    for(tString::size_type l = limit; end != tString::npos && l;) {
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
        while(!empty && end != tString::npos) {
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

tString
JoinString(std::vector<tString> strings, tString deli)
{
    tString joinedString;
    bool addDeli = false;
    for(auto str : strings) {
        if (addDeli)
            joinedString += deli;
        joinedString += str;

        addDeli = true;
    }
    return joinedString;
}

tString
StripString(const tString& str)
{
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

tString
StripStringChar(const tString& str, char c)
{
    // Find the first non-whitespace character
    size_t start = 0;
    while (start < str.length() && str[start] == c) {
        start++;
    }

    // Find the last non-whitespace character
    size_t end = str.length();
    while (end > start && str[end - 1] == c) {
        end--;
    }

    // Return the substring without leading and trailing whitespaces
    return str.substr(start, end - start);
}

tString
StringToUpper(tString str)
{
    tString dest = str;
    std::transform(dest.begin(), dest.end(), dest.begin(), ::toupper);
    return dest;
}

tString StringToLower(tString str)
{
    tString dest = str;
    std::transform(dest.begin(), dest.end(), dest.begin(), ::tolower);
    return dest;
}

bool
EndsWith(tString const &fullString, tString const &ending, bool caseSensitive)
{
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

bool
StartsWith(tString const &fullString, tString const &starting, bool caseSensitive)
{
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


std::vector<tString>
ShlexSplitString(const tString &input)
{
    std::vector<tString> tokens;
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

            tString token;
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
                throw ShlexError("Unmatched quote detected: " + tString(1, quote_char));
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


tString
ShlexJoinStrings(const std::vector<tString>& tokens)
{
    tString result;

    for (size_t i = 0; i < tokens.size(); ++i) {
        const tString& token = tokens[i];

        // Check if quoting is needed
        bool needs_quotes = false;
        for (char c : token) {
            if (std::isspace(c) || c == '\'' || c == '"' || c == '\\') {
                needs_quotes = true;
                break;
            }
        }

        if (token == "") {
            result += "\"\"";
        } else if (!needs_quotes) {
            result += token;
        } else {
            result += '"';  // Use double quotes universally
            for (char c : token) {
                if (c == '\\' || c == '"' || c == '$' || c == '`') {
                    result += '\\';  // Escape special characters in double quotes
                }
                result += c;
            }
            result += '"';
        }

        if (i + 1 < tokens.size()) {
            result += ' ';
        }
    }

    return result;
}

bool
CaseInsensitiveStringCompare(tString const &a, tString const &b)
{
    tString aLower, bLower;
    aLower.resize(a.size());
    bLower.resize(b.size());
    std::transform(a.begin(), a.end(), aLower.begin(), ::tolower);
    std::transform(b.begin(), b.end(), bLower.begin(), ::tolower);
    return aLower == bLower;
}

tString
StringReplace(tString orig, tString search, tString replacement)
{
    size_t pos = 0;
    while ((pos = orig.find(search, pos)) != tString::npos) {
        orig.replace(pos, search.length(), replacement);
        pos += replacement.length();  // Move past the last replacement
    }
    return orig;
}

bool
CaseInsensitiveStringComparator::operator ()(const tString &a,
        const tString &b) const
{
    // Convert strings to lowercase for comparison
    tString aLower, bLower;
    aLower.resize(a.size());
    bLower.resize(b.size());
    std::transform(a.begin(), a.end(), aLower.begin(), ::tolower);
    std::transform(b.begin(), b.end(), bLower.begin(), ::tolower);
    return aLower < bLower;
}

