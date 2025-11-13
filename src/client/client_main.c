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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>


#include <pinggy.h>
#include "cli_getopt.h"

#ifndef PLATFORM_CONFIG_INCLUDED

#define PinggyVersionMajor 0
#define PinggyVersionMinor 1
#define PinggyVersionPatch 0

#define PINGGY_GIT_COMMIT_ID "unknown"
#define PINGGY_BUILD_TIMESTAMP "0000-00-00 00:00:00"
#define PINGGY_LIBC_VERSION "unknown"
#define PINGGY_BUILD_OS "unknown"

#else
#include <platform/platform.h>
#endif


#define ConnMode_HTTP      "http"
#define ConnMode_TCP       "tcp"
#define ConnMode_TLS       "tls"
#define ConnMode_TLSTCP    "tlstcp"
#define ConnModeExt_UDP    "udp"

void
printHelpOptions(const char *prog)
{
    printf("%s [-h|--help] [-v|-version] [--port|-p PORT] [-R ADDR:PORT] [-L ADDR:PORT] token@server\n", prog);
    printf("        -h\n");
    printf("       --help\n");
    printf("                Print this help msg\n");
    printf("\n");
    printf("        -v\n");
    printf("       --version\n");
    printf("                Print version and exit\n");
    printf("\n");
    printf("        -p PORT\n");
    printf("       --port PORT\n");
    printf("                Provide the server port\n");
    printf("\n");
    printf("        -R ADDR:PORT\n");
    printf("                The address to where forwarded connections needs to be forwarded\n");
    printf("\n");
    printf("        -L ADDR:PORT\n");
    printf("                Listening address for webdebugging\n");
    printf("        -r\n");
    printf("            Enable autoreconnect\n");
    printf("\n");
}

typedef struct {
    pinggy_ref_t config_ref;
    uint16_t web_debugger_port;
    pinggy_bool_t enable_web_debugger;
    char *error_msg;
    pinggy_ref_t tunnel_ref;
} client_app_data_t;



static int split_forwarding(const char* val, char parts[][128], int max_parts)
{
    int count = 0;
    const char* start = val;

    while (*start && count < max_parts) {
        if (*start == ':') { // Skip empty or repeated colons
            start++;
            continue;
        }

        if (*start == '[') { // IPv6 [::1] handling
            const char* end = strchr(start, ']');
            if (!end) break;
            size_t len = end - start + 1;
            strncpy(parts[count++], start, len);
            parts[count - 1][len] = '\0';
            start = end + 1;
            if (*start == ':') start++;
        }
        else {
            const char* colon = strchr(start, ':');
            if (!colon) {
                strncpy(parts[count++], start, 127);
                parts[count - 1][127] = '\0';
                break;
            }
            size_t len = colon - start;
            if (len > 0) {
                strncpy(parts[count++], start, len);
                parts[count - 1][len] = '\0';
            }
            start = colon + 1;
        }
    }

    return count;
}


static int parse_forward_tunnel(client_app_data_t* app_data, const char* value)
{
    char parts[8][128];
    int count = split_forwarding(value, parts, 8);
    if (count < 2) return 0;

    int port = atoi(parts[count - 1]);
    if (port <= 0 || port > 65535) return 0;

    app_data->web_debugger_port = (uint16_t)port;
    app_data->enable_web_debugger = pinggy_true;
    return 1;
}

static int parse_reverse_tunnel(pinggy_ref_t config_ref, const char* value)
{
    char parts[8][128];
    int count = split_forwarding(value, parts, 8);

    if (count < 2) {
        printf("Invalid -R argument: '%s'\n", value);
        return 0;
    }

    // Fix: if count == 3 and first part is "0" (or not a hostname), drop it
    if (count == 3 && strcmp(parts[0], "0") == 0) {
        // treat as ADDR:PORT using last two parts
        char url[256];
        snprintf(url, sizeof(url), "%s:%s", parts[1], parts[2]);
        pinggy_config_set_tcp_forward_to(config_ref, url);
        printf(" '%s'\n", url);
        return 1;
    }

    // Normal handling below...
    if (count == 2) {
        char url[256];
        snprintf(url, sizeof(url), "%s:%s", parts[0], parts[1]);
        pinggy_config_set_tcp_forward_to(config_ref, url);
        return 1;
    }

    if (count >= 4) {
        char remote_url[256];
        char local_url[256];
        snprintf(remote_url, sizeof(remote_url), "%s:%s", parts[count - 4], parts[count - 3]);
        snprintf(local_url, sizeof(local_url), "%s:%s", parts[count - 2], parts[count - 1]);
        pinggy_config_set_tcp_forward_to(config_ref, remote_url);
        pinggy_config_set_udp_forward_to(config_ref, local_url);
        return 1;
    }

    printf("Unrecognized -R format: '%s'\n", value);
    return 0;
}


static int parse_user(pinggy_ref_t config_ref, char* user)
{
    char tcp_forward_to_buf[256] = { 0 };
    pinggy_config_get_tcp_forward_to(config_ref, sizeof(tcp_forward_to_buf), tcp_forward_to_buf);
    pinggy_config_set_tcp_forward_to(config_ref, ""); // clear it first

    char* token_str = NULL;
    int type_set = 0;
    int udp_type_set = 0;

    char* p = strtok(user, "+");
    while (p != NULL) {
        // to lower
        int i = 0;
        while (p[i]) {
            p[i] = tolower(p[i]);
            i++;
        }

        if (strcmp(p, ConnMode_HTTP) == 0 || strcmp(p, ConnMode_TCP) == 0 || strcmp(p, ConnMode_TLS) == 0 || strcmp(p, ConnMode_TLSTCP) == 0) {
            pinggy_config_set_type(config_ref, p);
            if (strlen(tcp_forward_to_buf) > 0) {
                pinggy_config_set_tcp_forward_to(config_ref, tcp_forward_to_buf);
            }
            type_set = 1;
        } else if (strcmp(p, ConnModeExt_UDP) == 0) {
            pinggy_config_set_udp_type(config_ref, p);
            if (strlen(tcp_forward_to_buf) > 0) {
                pinggy_config_set_udp_forward_to(config_ref, tcp_forward_to_buf);
            }
            udp_type_set = 1;
        } else {
            token_str = p;
        }
        p = strtok(NULL, "+");
    }

    if (!type_set && !udp_type_set) {
        pinggy_config_set_type(config_ref, ConnMode_HTTP);
        if (strlen(tcp_forward_to_buf) > 0) {
            pinggy_config_set_tcp_forward_to(config_ref, tcp_forward_to_buf);
        }
    }

    if (token_str) {
        pinggy_config_set_token(config_ref, token_str);
    }

    return 1;
}


static int parse_user_server(pinggy_ref_t config_ref, char* value, const char* port)
{
    char* at_sign = strrchr(value, '@');
    char server_addr[256];

    if (at_sign) {
        *at_sign = '\0'; // split string
        char* user = value;
        char* server = at_sign + 1;
        snprintf(server_addr, sizeof(server_addr), "%s:%s", server, port);
        if (!parse_user(config_ref, user)) {
            return 0;
        }
    } else {
        snprintf(server_addr, sizeof(server_addr), "%s:%s", value, port);
    }

    pinggy_config_set_server_address(config_ref, server_addr);
    return 1;
}

static int parse_sdk_arguments(pinggy_ref_t config_ref, int argc, char* argv[])
{
    if (argc <= 0) return 1;

    char arg_str[1024] = { 0 };
    for (int i = 0; i < argc; i++) {
        // Skip empty or malformed-looking args like "::" or single ":"
        if (argv[i] == NULL || strlen(argv[i]) == 0) continue;
        if (strstr(argv[i], "::") != NULL || strcmp(argv[i], ":") == 0) {
            printf("Ignoring suspicious argument: '%s'\n", argv[i]);
            continue;
        }

        strcat(arg_str, argv[i]);
        if (i < argc - 1) strcat(arg_str, " ");
    }

    pinggy_config_set_argument(config_ref, arg_str);
    return 1;
}



static client_app_data_t* parse_arguments(int argc, char* argv[])
{
    client_app_data_t* app_data = (client_app_data_t*)calloc(1, sizeof(client_app_data_t));
    if (!app_data) return NULL;

    app_data->config_ref = pinggy_create_config();
    if (app_data->config_ref == INVALID_PINGGY_REF) {
        free(app_data);
        return NULL;
    }
    app_data->web_debugger_port = 4300;

    int opt;
    int longindex = 0;

    struct cli_option longopts[] = {
        {"help", cli_no_argument, 0, 'h'},
        {"version", cli_no_argument, 0, 'v'},
        {"verbose", cli_no_argument, 0, 'V'},
        {"port", cli_required_argument, 0, 'p'},
        {"sni", cli_required_argument, 0, 's'},
        {"insecure", cli_no_argument, 0, 'n'},
        {"inseceure", cli_no_argument, 0, 'n'}, // for backward compatibility with typo
        {0, 0, 0, 0} // Terminator
    };
    int exit_now = 0;
    char* server_port = "443";
    const char* prog = argv[0];
    while ((opt = cli_getopt_long(argc, argv, "ahvVno:R:L:p:s:r", longopts, &longindex)) != -1) {
        int success = 1;
        switch (opt) {
        case 'a':
            pinggy_config_set_advanced_parsing(app_data->config_ref, pinggy_true);
            break;
        case 'h':
            printHelpOptions(prog);
            exit_now = 1;
            exit(0);
            break;
        case 'v':
            printf("v%d.%d.%d\n", PinggyVersionMajor, PinggyVersionMinor, PinggyVersionPatch);
            exit_now = 1;
            exit(0);
            break;
        case 'V':
            pinggy_set_log_enable(pinggy_true);
            break;
        case 'o':
            printf("Output option with value: %s\n", cli_optarg);
            break;
        case 'p':
            server_port = cli_optarg;
            break;
        case 'R':
            success = parse_reverse_tunnel(app_data->config_ref, cli_optarg);
            break;

        case 'L':
            success = parse_forward_tunnel(app_data, cli_optarg);
            break;
        case 'n':
            pinggy_config_set_ssl(app_data->config_ref, pinggy_false);
            break;
        case 's':
            pinggy_config_set_sni_server_name(app_data->config_ref, cli_optarg);
            break;
        case 'r':
            pinggy_config_set_auto_reconnect(app_data->config_ref, pinggy_true);
            break;
        case '?':
            printf("Unknown option or missing argument\n");
            break;
        }
        if (!success) {
            exit_now = 1;
            break;
        }
    }

    if (cli_optind < argc) {
        if (!parse_user_server(app_data->config_ref, argv[cli_optind], server_port)) {
            exit_now = 1;
        }
    }  else {
        exit_now = 1;
    }

    if ((cli_optind + 1) < argc) {
        if (!parse_sdk_arguments(app_data->config_ref, argc - (cli_optind + 1), argv + (cli_optind + 1))) {
            exit_now = 1;
        }
    }

    if (exit_now) {
        pinggy_free_ref(app_data->config_ref);
        free(app_data);
        exit(0);
    }

    return app_data;
}

static void on_primary_forwarding_succeeded(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_len_t num_urls, pinggy_char_p_p_t urls)
{
    client_app_data_t* app_data = (client_app_data_t*)user_data;
    int i;
    printf("Connection completed\n");
    for (i = 0; i < num_urls; i++) {
        printf("   %s\n", urls[i]);
    }
    if (app_data->enable_web_debugger && app_data->web_debugger_port > 0) {
        pinggy_tunnel_start_web_debugging(tunnel_ref, app_data->web_debugger_port);
    }
}

static void on_authentication_failed(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_len_t num_reasons, pinggy_char_p_p_t reasons)
{
    client_app_data_t* app_data = (client_app_data_t*)user_data;
    size_t total_len = 0;
    int i;
    for (i = 0; i < num_reasons; i++) {
        total_len += strlen(reasons[i]) + 1;
    }
    if (total_len > 0) {
        app_data->error_msg = (char*)malloc(total_len);
        if (app_data->error_msg) {
            app_data->error_msg[0] = '\0';
            for (i = 0; i < num_reasons; i++) {
                strcat(app_data->error_msg, reasons[i]);
                if (i < num_reasons - 1) {
                    strcat(app_data->error_msg, " ");
                }
            }
        }
    }
}

static void on_primary_forwarding_failed(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_const_char_p_t msg)
{
    client_app_data_t* app_data = (client_app_data_t*)user_data;
    if (msg) {
        app_data->error_msg = (char*)malloc(strlen(msg) + 1);
        if (app_data->error_msg) strcpy(app_data->error_msg, msg);
    }
}

static void on_disconnected(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_const_char_p_t error, pinggy_len_t msg_size, pinggy_char_p_p_t msg)
{
    client_app_data_t* app_data = (client_app_data_t*)user_data;
    if (error) {
        app_data->error_msg = (char*)malloc(strlen(error) + 1);
        if (app_data->error_msg) strcpy(app_data->error_msg, error);
    }
}

static void on_will_reconnect(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_const_char_p_t error, pinggy_len_t num_msgs, pinggy_char_p_p_t messages)
{
    printf("Reconnecting\n");
}

static void on_reconnecting(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_uint16_t retry_cnt)
{
    printf("Trying.. %u\n", retry_cnt);
}

static void on_reconnection_completed(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_len_t num_urls, pinggy_char_p_p_t urls)
{
    int i;
    printf("Reconnected\n");
    for (i = 0; i < num_urls; i++) {
        printf("   %s\n", urls[i]);
    }
}

static void on_reconnection_failed(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_uint16_t retry_cnt)
{
    printf("Reconnection failed after %u tries\n", retry_cnt);
}

static void on_usage_update(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_const_char_p_t usages)
{
    printf("Update msg: %s\n", usages);
    pinggy_capa_t capa = 0;
    pinggy_tunnel_get_greeting_msgs_len(tunnel_ref, 0, NULL, &capa);
    if (capa <= 0)
        return;
    char* greetings = alloca(capa);
    if (pinggy_tunnel_get_greeting_msgs(tunnel_ref, capa, greetings) > 0)
        printf("Greeting: %s\n", greetings);
}

static pinggy_void_t
pinggy_on_raise_exception_cb(pinggy_const_char_p_t where, pinggy_const_char_p_t what)
{
    printf("%s ==> %s\n", where, what);
}

int main(int argc, char* argv[])
{
#ifdef __WINDOWS_OS__
    WindowsSocketInitialize();
#endif
    pinggy_set_log_enable(pinggy_true);  // enable logs like C++ output

#if defined(__LINUX_OS__) || defined(__MAC_OS__)
    ignore_sigpipe();
#endif

    pinggy_set_on_exception_callback(pinggy_on_raise_exception_cb);

    client_app_data_t* app_data = parse_arguments(argc, argv);
    if (!app_data) {
        return 1;
    }

    // Optional: same as C++ sample
    //pinggy_config_set_basic_auths(app_data->config_ref, "[[\"user1\", \"pass1\"], [\"user2\", \"pass two\"]]");

    // Remove the 'return 0' here
    // char str[1024];
    // pinggy_config_get_argument_len(app_data->config_ref, 1024, str, NULL);
    // printf("aasd %s\n", str);
    // return 0;

    // ---- Start the tunnel like in C++ ----
    app_data->tunnel_ref = pinggy_tunnel_initiate(app_data->config_ref);
    if (app_data->tunnel_ref == INVALID_PINGGY_REF) {
        printf("Failed to initiate tunnel\n");
        pinggy_free_ref(app_data->config_ref);
        free(app_data);
        return 1;
    }

    // Register callbacks
    pinggy_tunnel_set_on_primary_forwarding_succeeded_callback(app_data->tunnel_ref, on_primary_forwarding_succeeded, app_data);
    pinggy_tunnel_set_on_authentication_failed_callback(app_data->tunnel_ref, on_authentication_failed, app_data);
    pinggy_tunnel_set_on_primary_forwarding_failed_callback(app_data->tunnel_ref, on_primary_forwarding_failed, app_data);
    pinggy_tunnel_set_on_disconnected_callback(app_data->tunnel_ref, on_disconnected, app_data);
    pinggy_tunnel_set_on_will_reconnect_callback(app_data->tunnel_ref, on_will_reconnect, app_data);
    pinggy_tunnel_set_on_reconnecting_callback(app_data->tunnel_ref, on_reconnecting, app_data);
    pinggy_tunnel_set_on_reconnection_completed_callback(app_data->tunnel_ref, on_reconnection_completed, app_data);
    pinggy_tunnel_set_on_reconnection_failed_callback(app_data->tunnel_ref, on_reconnection_failed, app_data);
    pinggy_tunnel_set_on_usage_update_callback(app_data->tunnel_ref, on_usage_update, app_data);

    // Start usage update + tunnel
    pinggy_tunnel_start_usage_update(app_data->tunnel_ref);
    pinggy_tunnel_start(app_data->tunnel_ref);

    // Cleanup
    if (app_data->error_msg) {
        printf("Tunnel ended with msg: %s\n", app_data->error_msg);
        free(app_data->error_msg);
    }

    pinggy_free_ref(app_data->tunnel_ref);
    pinggy_free_ref(app_data->config_ref);
    free(app_data);
    return 0;
}
