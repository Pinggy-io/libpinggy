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

#ifndef SRC_CPP_CLIENT_GETOPT_H_
#define SRC_CPP_CLIENT_GETOPT_H_

#include <stdio.h>
#include <string.h>
#include <platform/platform.h>

// #ifdef __WINDOWS_OS__

#ifdef __cplusplus
extern "C" {
#endif

extern int                      cli_opterr;
extern int                      cli_optind;
extern int                      cli_optopt;
extern char                    *cli_optarg;

struct cli_option {
    const char *name;
    int has_arg;
    int *flag;
    int val;
};

enum { cli_no_argument, cli_required_argument, cli_optional_argument };

int cli_getopt(int argc, char *const argv[], const char *optstring);

int cli_getopt_long(int argc, char *const argv[], const char *optstring,
                const struct cli_option *longopts, int *longindex);


#ifdef __cplusplus
}
#endif

// #endif //if __WINDOWS_OS__

#endif // SRC_CPP_CLIENT_GETOPT_H_