/*
 * Copyright (C) 2025 PINGGY TECHNOLOGY PRIVATE LIMITED
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
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
#include <stdint.h>

#include <pinggy.h>
#include "cli_getopt.h"

#ifndef PLATFORM_CONFIG_INCLUDED

#define PinggyVersionMajor 0
#define PinggyVersionMinor 0
#define PinggyVersionPatch 0

#define PINGGY_GIT_COMMIT_ID "unknown"
#define PINGGY_BUILD_TIMESTAMP "0000-00-00 00:00:00"
#define PINGGY_LIBC_VERSION "unknown"
#define PINGGY_BUILD_OS "unknown"

#else
#include <platform/platform.h>
#endif

/* Connection mode tokens */
#define ConnMode_HTTP      "http"
#define ConnMode_TCP       "tcp"
#define ConnMode_TLS       "tls"
#define ConnMode_TLSTCP    "tlstcp"
#define ConnMode_UDP       "udp"

#define FORWARDING_SEPARATOR ";"

/* --- Helper: Dynamic String Builder --- */
typedef struct {
    char *data;
    size_t len;
} string_builder_t;

static void sb_init(string_builder_t *sb) {
    sb->data = NULL;
    sb->len = 0;
}

static void sb_append(string_builder_t *sb, const char *str) {
    if (!str || !*str) return;
    size_t str_len = strlen(str);
    size_t new_len = sb->len + str_len;
    
    // Add separator if not empty
    int add_sep = (sb->len > 0);
    if (add_sep) new_len++;

    char *new_data = (char*)realloc(sb->data, new_len + 1);
    if (!new_data) return; // OOM

    sb->data = new_data;
    if (add_sep) {
        sb->data[sb->len] = ';';
        strcpy(sb->data + sb->len + 1, str);
    } else {
        strcpy(sb->data, str);
    }
    sb->len = new_len;
}

static void sb_free(string_builder_t *sb) {
    if (sb->data) free(sb->data);
    sb->data = NULL;
    sb->len = 0;
}

/* --- Parsing Helpers --- */

void printHelpOptions(const char *prog){
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
    char **parts;
    int count;
} parts_list_t;

static void parts_list_init(parts_list_t *pl) {
    pl->parts = NULL;
    pl->count = 0;
}

static void parts_list_push(parts_list_t *pl, const char *s) {
    char **temp = (char**)realloc(pl->parts, sizeof(char*) * (pl->count + 1));
    if (!temp) return;
    pl->parts = temp;
    pl->parts[pl->count] = strdup(s);
    pl->count++;
}

static void parts_list_free(parts_list_t *pl) {
    if (!pl) return;
    for (int i = 0; i < pl->count; ++i) free(pl->parts[i]);
    free(pl->parts);
    pl->parts = NULL;
    pl->count = 0;
}

static parts_list_t parse_forwarding(const char *val) {
    parts_list_t res;
    parts_list_init(&res);
    if (!val) return res;

    const char *p = val;
    size_t len = strlen(val);
    size_t i = 0;
    while (i < len) {
        if (p[i] == '[') {
            size_t j = i + 1;
            while (j < len && p[j] != ']') j++;
            if (j >= len) {
                parts_list_push(&res, p + i);
                break;
            }
            size_t piece_len = j - i + 1;
            char *piece = (char*)malloc(piece_len + 1);
            if (piece) {
                memcpy(piece, p + i, piece_len);
                piece[piece_len] = '\0';
                parts_list_push(&res, piece);
                free(piece);
            }
            i = j + 1;
            if (i < len && p[i] == ':') i++;
        } else {
            size_t j = i;
            while (j < len && p[j] != ':') j++;
            size_t piece_len = j - i;
            char *piece = (char*)malloc(piece_len + 1);
            if (piece) {
                memcpy(piece, p + i, piece_len);
                piece[piece_len] = '\0';
                parts_list_push(&res, piece);
                free(piece);
            }
            i = j;
            if (i < len && p[i] == ':') i++;
        }
    }
    return res;
}

static int parse_hostname(const char *host, char **schema_out, char **hostname_out, int *port_out) {
    if (!host) return 0;
    *schema_out = NULL;
    *hostname_out = NULL;
    *port_out = 0;

    char *copy = strdup(host);
    if (!copy) return 0;

    char *schema = NULL;
    char *hostname = copy;

    char *sep = strstr(copy, "//");
    if (sep) {
        *sep = '\0';
        schema = strdup(copy);
        hostname = sep + 2;
    }

    char *last_slash = strrchr(hostname, '/');
    int port = 0;
    if (last_slash) {
        char *portstr = last_slash + 1;
        if (portstr && *portstr) {
            port = atoi(portstr);
        }
        *last_slash = '\0';
    }

    char *hostdup = strdup(hostname);
    if (schema) *schema_out = schema;
    else *schema_out = strdup("");

    *hostname_out = hostdup;
    *port_out = port;

    free(copy);
    return 1;
}

/*
 * Accumulate forwardings into local String Builders instead of setting config immediately.
 * This prevents the Segfault caused by Reading from the SDK.
 */
static int parse_reverse_tunnel(string_builder_t *tcp_sb, string_builder_t *udp_sb, char **forwardings, int forwardings_count, const char *mode_hint) {
    for (int idx = 0; idx < forwardings_count; ++idx) {
        parts_list_t parts = parse_forwarding(forwardings[idx]);
        if (parts.count < 2) {
            parts_list_free(&parts);
            return 0;
        }

        const char *last_minus1 = parts.parts[parts.count - 2];
        const char *last = parts.parts[parts.count - 1];
        char forwardTo[512];
        snprintf(forwardTo, sizeof(forwardTo), "%s:%s", last_minus1, last);

        if (parts.count < 4) {
            // Default adds to TCP list
            sb_append(tcp_sb, forwardTo);
        } else {
            const char *remoteHost = parts.parts[parts.count - 4];
            char *schema = NULL;
            char *hostname = NULL;
            int port = 0;
            
            if (!parse_hostname(remoteHost, &schema, &hostname, &port)) {
                parts_list_free(&parts);
                return 0;
            }

            if (hostname[0] == '\0') {
                free(hostname);
                hostname = strdup("localhost");
            }

            char host_with_port[512];
            if (port > 0) snprintf(host_with_port, sizeof(host_with_port), "%s:%d", hostname, port);
            else snprintf(host_with_port, sizeof(host_with_port), "%s", hostname);

            const char *use_schema = (schema && strlen(schema) > 0) ? schema : mode_hint;
            int is_udp = 0;
            if (use_schema && strcmp(use_schema, ConnMode_UDP) == 0) is_udp = 1;

            char combined[1024];
            if (schema && strlen(schema) > 0) {
                snprintf(combined, sizeof(combined), "%s::%s::%s", schema, host_with_port, forwardTo);
            } else if (mode_hint && strlen(mode_hint) > 0) {
                snprintf(combined, sizeof(combined), "%s//%s:%s", mode_hint, host_with_port, forwardTo);
            } else {
                snprintf(combined, sizeof(combined), "%s:%s", host_with_port, forwardTo);
            }

            if (is_udp) sb_append(udp_sb, combined);
            else sb_append(tcp_sb, combined);

            free(schema);
            free(hostname);
        }
        parts_list_free(&parts);
    }
    return 1;
}

static int parse_forward_tunnel_for_L(pinggy_ref_t config_ref, const char *value, uint16_t *out_port, int *enable_dbg) {
    parts_list_t parts = parse_forwarding(value);
    if (parts.count < 2) {
        parts_list_free(&parts);
        return 0;
    }

    char addr[512];
    snprintf(addr, sizeof(addr), "%s:%s", parts.parts[parts.count - 2], parts.parts[parts.count - 1]);
    pinggy_config_set_web_debugger_address(config_ref, addr);

    if (out_port) *out_port = (uint16_t)atoi(parts.parts[parts.count - 1]);
    if (enable_dbg) *enable_dbg = 1;

    parts_list_free(&parts);
    return 1;
}

static int parse_user(pinggy_ref_t config_ref, char *user) {
    char *token_str = NULL;
    int type_set = 0;
    int udp_type_set = 0;

    char *copy = strdup(user);
    if (!copy) return 0;

    char *p = strtok(copy, "+");
    while (p != NULL) {
        for (char *q = p; *q; ++q) *q = (char)tolower((unsigned char)*q);

        if (strcmp(p, ConnMode_HTTP) == 0 || strcmp(p, ConnMode_TCP) == 0 || strcmp(p, ConnMode_TLS) == 0 || strcmp(p, ConnMode_TLSTCP) == 0) {
            pinggy_config_set_type(config_ref, p);
            type_set = 1;
        } else if (strcmp(p, ConnMode_UDP) == 0) {
            pinggy_config_set_udp_type(config_ref, p);
            udp_type_set = 1;
        } else {
            if (token_str) free(token_str);
            token_str = strdup(p);
        }
        p = strtok(NULL, "+");
    }

    if (!type_set && !udp_type_set) {
        pinggy_config_set_type(config_ref, ConnMode_HTTP);
    }

    if (token_str) {
        pinggy_config_set_token(config_ref, token_str);
        free(token_str);
    }
    free(copy);
    return 1;
}

static int parse_user_server(pinggy_ref_t config_ref, char *value, const char *port) {
    char *at_sign = strrchr(value, '@');
    char server_addr[512];

    if (at_sign) {
        *at_sign = '\0';
        char *user = value;
        char *server = at_sign + 1;
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

static int parse_sdk_arguments(pinggy_ref_t config_ref, int argc, char *argv[]) {
    if (argc <= 0) return 1;
    size_t total_len = 0;
    for (int i = 0; i < argc; ++i) total_len += strlen(argv[i]) + 1;
    if (total_len == 0) return 1;
    
    char *arg_str = (char*)malloc(total_len + 1);
    if (!arg_str) return 0;
    arg_str[0] = '\0';
    for (int i = 0; i < argc; ++i) {
        strcat(arg_str, argv[i]);
        if (i < argc - 1) strcat(arg_str, " ");
    }
    pinggy_config_set_argument(config_ref, arg_str);
    free(arg_str);
    return 1;
}

typedef struct {
    pinggy_ref_t config_ref;
    uint16_t web_debugger_port;
    pinggy_bool_t enable_web_debugger;
    char *error_msg;
    pinggy_ref_t tunnel_ref;
} client_app_data_t;

static client_app_data_t* parse_arguments(int argc, char *argv[]) {
    client_app_data_t *app_data = (client_app_data_t*)calloc(1, sizeof(client_app_data_t));
    if (!app_data) return NULL;

    app_data->config_ref = pinggy_create_config();
    if (app_data->config_ref == INVALID_PINGGY_REF) {
        free(app_data);
        return NULL;
    }

    app_data->web_debugger_port = 4300;
    app_data->enable_web_debugger = pinggy_false;

    char **r_list = NULL;
    int r_count = 0;

    // Builders for forwarding strings
    string_builder_t tcp_sb, udp_sb;
    sb_init(&tcp_sb);
    sb_init(&udp_sb);

    int opt;
    int longindex = 0;
    struct cli_option longopts[] = {
        {"help", cli_no_argument, 0, 'h'},
        {"version", cli_no_argument, 0, 'v'},
        {"verbose", cli_no_argument, 0, 'V'},
        {"port", cli_required_argument, 0, 'p'},
        {"sni", cli_required_argument, 0, 's'},
        {"insecure", cli_no_argument, 0, 'n'},
        {"inseceure", cli_no_argument, 0, 'n'},
        {0, 0, 0, 0}
    };

    int exit_now = 0;
    char *server_port = "443";
    const char *prog = argv[0];

    while ((opt = cli_getopt_long(argc, argv, "ahvVno:R:L:p:s:r", longopts, &longindex)) != -1) {
        int success = 1;
        switch (opt) {
            case 'a':
                pinggy_config_set_advanced_parsing(app_data->config_ref, pinggy_true);
                break;
            case 'h':
                printHelpOptions(prog);
                exit_now = 1;
                break;
            case 'v':
                printf("v%d.%d.%d\n", PinggyVersionMajor, PinggyVersionMinor, PinggyVersionPatch);
                exit_now = 1;
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
                {
                    char **temp = (char**)realloc(r_list, sizeof(char*) * (r_count + 1));
                    if (temp) {
                        r_list = temp;
                        if(cli_optarg) r_list[r_count++] = strdup(cli_optarg);
                    }
                }
                break;
            case 'L':
                {
                    uint16_t dbgport = 0;
                    int enabled = 0;
                    success = parse_forward_tunnel_for_L(app_data->config_ref, cli_optarg, &dbgport, &enabled);
                    if (success) {
                        app_data->web_debugger_port = dbgport;
                        app_data->enable_web_debugger = enabled ? pinggy_true : pinggy_false;
                    }
                }
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
        if (!success || exit_now) {
            exit_now = 1;
            break;
        }
    }

    if (!exit_now && cli_optind < argc) {
        if (!parse_user_server(app_data->config_ref, argv[cli_optind], server_port)) {
            exit_now = 1;
        }
    } else if (!exit_now) {
        exit_now = 1;
    }

    if (!exit_now && r_count > 0) {
        char mode_hint_buf[64];
        memset(mode_hint_buf, 0, sizeof(mode_hint_buf));
        pinggy_config_get_type(app_data->config_ref, sizeof(mode_hint_buf) - 1, mode_hint_buf);
        
        if (!parse_reverse_tunnel(&tcp_sb, &udp_sb, r_list, r_count, mode_hint_buf)) {
            exit_now = 1;
        }
    }

    // APPLY ACCUMULATED FORWARDINGS TO CONFIG
    if (tcp_sb.data) {
        pinggy_config_set_tcp_forward_to(app_data->config_ref, tcp_sb.data);
    }
    if (udp_sb.data) {
        pinggy_config_set_udp_forward_to(app_data->config_ref, udp_sb.data);
    }

    if (!exit_now && (cli_optind + 1) < argc) {
        if (!parse_sdk_arguments(app_data->config_ref, argc - (cli_optind + 1), argv + (cli_optind + 1))) {
            exit_now = 1;
        }
    }

    for (int i = 0; i < r_count; ++i) free(r_list[i]);
    free(r_list);
    sb_free(&tcp_sb);
    sb_free(&udp_sb);

    if (exit_now) {
        pinggy_free_ref(app_data->config_ref);
        free(app_data);
        return NULL;
    }

    return app_data;
}

/* --- Callbacks & Main (Unchanged logic, just cleanup) --- */

static void on_primary_forwarding_succeeded(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_len_t num_urls, pinggy_char_p_p_t urls) {
    if (!user_data) return;
    client_app_data_t *app_data = (client_app_data_t *)user_data;
    printf("Connection completed\n");
    for (int i = 0; i < (int)num_urls; i++) {
        printf("   %s\n", urls[i]);
    }
    if (app_data->enable_web_debugger && app_data->web_debugger_port > 0) {
        pinggy_tunnel_start_web_debugging(tunnel_ref, app_data->web_debugger_port);
    }
}

static void on_authentication_failed(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_len_t num_reasons, pinggy_char_p_p_t reasons) {
    client_app_data_t *app_data = (client_app_data_t *)user_data;
    size_t total_len = 0;
    for (int i = 0; i < (int)num_reasons; i++) total_len += strlen(reasons[i]) + 1;
    
    if (total_len > 0) {
        app_data->error_msg = (char *)malloc(total_len + 1);
        if (app_data->error_msg) {
            app_data->error_msg[0] = '\0';
            for (int i = 0; i < (int)num_reasons; i++) {
                strcat(app_data->error_msg, reasons[i]);
                if (i < (int)num_reasons - 1) strcat(app_data->error_msg, " ");
            }
        }
    }
}

static void on_primary_forwarding_failed(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_const_char_p_t msg) {
    client_app_data_t *app_data = (client_app_data_t *)user_data;
    if (msg) app_data->error_msg = strdup(msg);
}

static void on_disconnected(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_const_char_p_t error, pinggy_len_t msg_size, pinggy_char_p_p_t msg) {
    client_app_data_t *app_data = (client_app_data_t *)user_data;
    if (error) app_data->error_msg = strdup(error);
}

static void on_will_reconnect(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_const_char_p_t error, pinggy_len_t num_msgs, pinggy_char_p_p_t messages) {
    printf("Reconnecting\n");
}

static void on_reconnecting(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_uint16_t retry_cnt) {
    printf("Trying.. %u\n", retry_cnt);
}

static void on_reconnection_completed(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_len_t num_urls, pinggy_char_p_p_t urls) {
    printf("Reconnected\n");
    for (int i = 0; i < (int)num_urls; i++) {
        printf("   %s\n", urls[i]);
    }
}

static void on_reconnection_failed(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_uint16_t retry_cnt) {
    printf("Reconnection failed after %u tries\n", retry_cnt);
}

static void on_usage_update(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_const_char_p_t usages) {
    printf("Update msg: %s\n", usages);
    pinggy_capa_t capa = 0;
    pinggy_tunnel_get_greeting_msgs_len(tunnel_ref, 0, NULL, &capa);
    
    if (capa <= 0) return;
    
    char *greetings = (char*)malloc(capa);
    if (!greetings) return;

    if (pinggy_tunnel_get_greeting_msgs(tunnel_ref, capa, greetings) > 0) {
        printf("Greeting: %s\n", greetings);
    }
    free(greetings);
}

static pinggy_void_t pinggy_on_raise_exception_cb(pinggy_const_char_p_t where, pinggy_const_char_p_t what)
{
    printf("%s ==> %s\n", where, what);
}

int main(int argc, char *argv[]) {
#ifdef __WINDOWS_OS__
    WindowsSocketInitialize();
#endif
    pinggy_set_log_enable(pinggy_false);
#if defined(__LINUX_OS__) || defined(__MAC_OS__)
    ignore_sigpipe();
#endif

    pinggy_set_on_exception_callback(pinggy_on_raise_exception_cb);

    client_app_data_t *app_data = parse_arguments(argc, argv);
    if (!app_data) {
        return 1;
    }

    pinggy_config_set_basic_auths(app_data->config_ref, "[['user1', 'pass1'], ['user2', 'pass two']]");

    // Safe argument reading
    pinggy_capa_t arg_len = 0;
    char str[1024];
    memset(str, 0, sizeof(str));
    pinggy_config_get_argument_len(app_data->config_ref, 1024, str, &arg_len);
    // printf("Args set: %s\n", str); 

    app_data->tunnel_ref = pinggy_tunnel_initiate(app_data->config_ref);
    if (app_data->tunnel_ref == INVALID_PINGGY_REF) {
        printf("Failed to initiate tunnel\n");
        pinggy_free_ref(app_data->config_ref);
        free(app_data);
        return 1;
    }

    pinggy_tunnel_set_on_primary_forwarding_succeeded_callback(app_data->tunnel_ref, on_primary_forwarding_succeeded, app_data);
    pinggy_tunnel_set_on_authentication_failed_callback(app_data->tunnel_ref, on_authentication_failed, app_data);
    pinggy_tunnel_set_on_primary_forwarding_failed_callback(app_data->tunnel_ref, on_primary_forwarding_failed, app_data);
    pinggy_tunnel_set_on_disconnected_callback(app_data->tunnel_ref, on_disconnected, app_data);
    pinggy_tunnel_set_on_will_reconnect_callback(app_data->tunnel_ref, on_will_reconnect, app_data);
    pinggy_tunnel_set_on_reconnecting_callback(app_data->tunnel_ref, on_reconnecting, app_data);
    pinggy_tunnel_set_on_reconnection_completed_callback(app_data->tunnel_ref, on_reconnection_completed, app_data);
    pinggy_tunnel_set_on_reconnection_failed_callback(app_data->tunnel_ref, on_reconnection_failed, app_data);
    pinggy_tunnel_set_on_usage_update_callback(app_data->tunnel_ref, on_usage_update, app_data);

    pinggy_tunnel_start_usage_update(app_data->tunnel_ref);
    pinggy_tunnel_start(app_data->tunnel_ref);

    if (app_data->error_msg) {
        printf("Tunnel ended with msg: %s\n", app_data->error_msg);
        free(app_data->error_msg);
    }

   // --- SAFE SHUTDOWN ---
pinggy_tunnel_stop(app_data->tunnel_ref);
pinggy_tunnel_wait_until_stopped(app_data->tunnel_ref);

// Now it’s safe to free everything
pinggy_free_ref(app_data->tunnel_ref);
pinggy_free_ref(app_data->config_ref);
free(app_data);


    return 0;
}

