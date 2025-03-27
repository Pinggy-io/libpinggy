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


#ifndef SRC_CPP_COMMON_UTILS_STRINGUTILS_HH_
#define SRC_CPP_COMMON_UTILS_STRINGUTILS_HH_
#include <string>
#include <vector>
#include <stdexcept>
#include <platform/platform.h>


std::vector<std::string> SplitString(std::string str, std::string deli, int16_t limit=-1, bool keep=false, bool empty=true);
std::string JoinString(std::vector<std::string> strs, std::string deli);
std::string StripString(const std::string& str);
std::string StringToUpper(std::string str);
std::string StringToLower(std::string str);
bool EndsWith (std::string const &fullString, std::string const &ending, bool caseSensitive = true);
bool StartsWith (std::string const &fullString, std::string const &starting, bool caseSensitive = true);
bool CaseInsensitiveStringCompare(std::string const &a, std::string const &b);
tString StringReplace(tString orig, tString search, tString replace);

struct CaseInsensitiveStringComparator {
    bool operator()(const std::string& a, const std::string& b) const;
};

class ShlexError : public std::runtime_error {
public:
    explicit ShlexError(const std::string& message) : std::runtime_error(message) {}
};

std::vector<std::string> ShlexSplitString(const std::string &input);

#if 0
// Case insensetive string

struct CiCharTraits: public std::char_traits<char>
// just inherit all the other functions
//  that we don't need to override
{
    static bool eq(char c1, char c2)
    {
        return toupper(c1) == toupper(c2);
    }

    static bool ne(char c1, char c2)
    {
        return toupper(c1) != toupper(c2);
    }

    static bool lt(char c1, char c2)
    {
        return toupper(c1) < toupper(c2);
    }

    static int compare(const char *s1, const char *s2, size_t n)
    {
        (void)n;
//        return std::memicmp(s1, s2, n);
        for(; *s1 && *s2 && eq(*s1, *s2); s1++, s2++);
        return toupper(*s1) - toupper(*s2);
        // if available on your compiler,
        //  otherwise you can roll your own
    }

    static const char*
    find(const char *s, int n, char a)
    {
        while (n-- > 0 && toupper(*s) != toupper(a))
        {
            ++s;
        }
        return s;
    }
};

typedef std::basic_string<char, CiCharTraits> CIString;
#endif



#endif /* SRC_CPP_COMMON_UTILS_STRINGUTILS_HH_ */
