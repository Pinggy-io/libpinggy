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


std::vector<tString>
SplitString(tString str, tString deli, int16_t limit=-1, bool keep=false, bool empty=true);

tString
JoinString(std::vector<tString> strs, tString deli);

tString
StripString(const tString& str);

tString
StripStringChar(const tString& str, char c);

#define NormalizeDomainName(domain) \
    StripStringChar(domain, '.')

tString
StringToUpper(tString str);

tString
StringToLower(tString str);

bool
EndsWith (tString const &fullString, tString const &ending, bool caseSensitive = true);

bool
StartsWith (tString const &fullString, tString const &starting, bool caseSensitive = true);

bool
CaseInsensitiveStringCompare(tString const &a, tString const &b);

tString
StringReplace(tString orig, tString search, tString replace);

struct CaseInsensitiveStringComparator
{
    bool
    operator()(const tString& a, const tString& b) const;
};

class ShlexError : public std::runtime_error {
public:
    explicit
    ShlexError(const tString& message) : std::runtime_error(message)
                                { }
};

std::vector<tString>
ShlexSplitString(const tString &input);

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
