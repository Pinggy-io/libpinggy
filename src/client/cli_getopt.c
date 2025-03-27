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


#include "cli_getopt.h"

int                             cli_opterr = 1;
int                             cli_optind = 1;
int                             cli_optopt;
char                           *cli_optarg;

int cli_getopt(int argc, char *const argv[], const char *optstring)
{
    static int optpos = 1; // Position in the current argument
    char *current_arg;

    // No more options to process
    if (cli_optind >= argc || argv[cli_optind][0] != '-' || argv[cli_optind][1] == '\0') {
        return -1;
    }

    current_arg = argv[cli_optind];

    // Handle "--" indicating the end of options
    if (strcmp(current_arg, "--") == 0) {
        cli_optind++;
        return -1;
    }

    cli_optopt = current_arg[optpos]; // Get the current option character
    const char *opt_spec = strchr(optstring, cli_optopt);

    // Invalid option
    if (!opt_spec) {
        if (cli_opterr) {
            fprintf(stderr, "Unknown option '-%c'\n", cli_optopt);
        }
        if (current_arg[++optpos] == '\0') {
            cli_optind++;
            optpos = 1;
        }
        return '?';
    }

    // Option requires an argument
    if (opt_spec[1] == ':') {
        if (current_arg[optpos + 1] != '\0') {
            cli_optarg = &current_arg[optpos + 1]; // The argument is attached (e.g., -oarg)
            cli_optind++;
        } else if (cli_optind + 1 < argc) {
            cli_optarg = argv[++cli_optind]; // The argument is the next element
            cli_optind++;
        } else {
            if (cli_opterr) {
                fprintf(stderr, "Option '-%c' requires an argument\n", cli_optopt);
            }
            cli_optind++;
            optpos = 1;
            return ':';
        }
        optpos = 1;
    } else {
        // Option does not require an argument
        if (current_arg[++optpos] == '\0') {
            cli_optind++;
            optpos = 1;
        }
        cli_optarg = NULL;
    }

    return cli_optopt;
}

int cli_getopt_long(int argc, char *const argv[], const char *optstring,
                const struct cli_option *longopts, int *longindex) {
    static int optpos = 1;
    char *current_arg;

    if (cli_optind >= argc || argv[cli_optind][0] != '-' || argv[cli_optind][1] == '\0') {
        return -1; // No more options to process
    }

    // Handle "--" indicating the end of options
    if (strcmp(argv[cli_optind], "--") == 0) {
        cli_optind++;
        return -1;
    }

    // Check if it's a long option "--option" or "--option=value"
    if (argv[cli_optind][1] == '-') {
        current_arg = &argv[cli_optind][2]; // Get the option name (skip "--")
        char *equals_sign = strchr(current_arg, '=');

        for (int i = 0; longopts[i].name != NULL; i++) {
            // Check for match before the '='
            if ((equals_sign && strncmp(current_arg, longopts[i].name, equals_sign - current_arg) == 0) ||
                (!equals_sign && strcmp(current_arg, longopts[i].name) == 0)) {

                if (longindex != NULL) {
                    *longindex = i;
                }

                // If there's a flag, set it and return 0
                if (longopts[i].flag != NULL) {
                    *longopts[i].flag = longopts[i].val;
                    cli_optind++;
                    return 0;
                }

                // If there's an argument after '='
                if (equals_sign) {
                    if (longopts[i].has_arg == cli_no_argument) {
                        if (cli_opterr) {
                            fprintf(stderr, "Option '--%s' doesn't allow an argument\n", longopts[i].name);
                        }
                        return '?';
                    }
                    cli_optarg = equals_sign + 1;
                    cli_optind++;
                    return longopts[i].val;
                }

                // Handle long options with required or optional arguments
                if (longopts[i].has_arg == cli_required_argument) {
                    if (cli_optind + 1 < argc) {
                        cli_optarg = argv[++cli_optind];
                    } else {
                        if (cli_opterr) {
                            fprintf(stderr, "Option '--%s' requires an argument\n", longopts[i].name);
                        }
                        return '?';
                    }
                } else if (longopts[i].has_arg == cli_optional_argument) {
                    if (cli_optind + 1 < argc && argv[cli_optind + 1][0] != '-') {
                        cli_optarg = argv[++cli_optind];
                    }
                }
                cli_optind++;
                return longopts[i].val;
            }
        }

        // If no match is found for the long option
        if (cli_opterr) {
            fprintf(stderr, "Unknown option '--%s'\n", current_arg);
        }
        cli_optind++;
        return '?';
    }

    // Handle short options "-o" and "-ovalue"
    current_arg = argv[cli_optind];
    cli_optopt = current_arg[optpos]; // Get the current option character
    const char *opt_spec = strchr(optstring, cli_optopt);

    if (!opt_spec) {
        if (cli_opterr) {
            fprintf(stderr, "Unknown option '-%c'\n", cli_optopt);
        }
        if (current_arg[++optpos] == '\0') {
            cli_optind++;
            optpos = 1;
        }
        return '?';
    }

    if (opt_spec[1] == ':') { // Option requires an argument
        if (current_arg[optpos + 1] != '\0') {
            cli_optarg = &current_arg[optpos + 1]; // Argument is attached (e.g., -ovalue)
            cli_optind++;
        } else if (cli_optind + 1 < argc) {
            cli_optarg = argv[++cli_optind]; // Argument is next in the list (e.g., -o value)
            cli_optind++;
        } else {
            if (cli_opterr) {
                fprintf(stderr, "Option '-%c' requires an argument\n", cli_optopt);
            }
            cli_optind++;
            optpos = 1;
            return ':';
        }
        optpos = 1;
    } else {
        if (current_arg[++optpos] == '\0') {
            cli_optind++;
            optpos = 1;
        }
        cli_optarg = NULL;
    }

    return cli_optopt;
}
