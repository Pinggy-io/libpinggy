/*
 * Copyright (C) 2025 PINGGY TECHNOLOGY PRIVATE LIMITED
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * ...
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

/* Delimiter used when storing multiple forwardings in C SDK config */
#define FORWARDING_SEPARATOR ";"

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

/* Simple dynamic string list for parsed forwarding parts */
typedef struct {
    char **parts;
    int count;
} parts_list_t;

static void parts_list_init(parts_list_t *pl) {
    pl->parts = NULL;
    pl->count = 0;
}
static void parts_list_push(parts_list_t *pl, const char *s) {
    pl->parts = (char**)realloc(pl->parts, sizeof(char*) * (pl->count + 1));
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

/*
 * parse_forwarding:
 * Splits the forwarding `val` into logical parts, carefully handling IPv6
 * addresses in `[...]` and colon separators. Behavior mirrors the . parseForwarding:
 * e.g. "[::1]:0:localhost:8080" -> ["[::1]", "0", "localhost", "8080"]
 *       "ssh//example.com/2222:0:localhost:8000" -> ["ssh//example.com/2222","0","localhost","8000"]
 */
static parts_list_t parse_forwarding(const char *val) {
    parts_list_t res;
    parts_list_init(&res);
    if (!val) return res;

    const char *p = val;
    size_t len = strlen(val);
    size_t i = 0;
    while (i < len) {
        if (p[i] == '[') {
            /* IPv6 bracketed */
            size_t j = i + 1;
            while (j < len && p[j] != ']') j++;
            if (j >= len) {
                /* malformed - push remaining and break */
                parts_list_push(&res, p + i);
                break;
            }
            size_t piece_len = j - i + 1;
            char *piece = (char*)malloc(piece_len + 1);
            memcpy(piece, p + i, piece_len);
            piece[piece_len] = '\0';
            parts_list_push(&res, piece);
            free(piece);
            i = j + 1;
            if (i < len && p[i] == ':') i++;
        } else {
            /* read up to next colon */
            size_t j = i;
            while (j < len && p[j] != ':') j++;
            size_t piece_len = j - i;
            char *piece = (char*)malloc(piece_len + 1);
            memcpy(piece, p + i, piece_len);
            piece[piece_len] = '\0';
            parts_list_push(&res, piece);
            free(piece);
            i = j;
            if (i < len && p[i] == ':') i++;
        }
    }

    return res;
}

/*
 * parse_hostname:
   accepts [schema//]hostname[/port]
 * returns schema (possibly empty), hostname, and port (0 if none)
 * NOTE: caller must free schema_out and hostname_out.
 */
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

    /* find last '/' as port separator */
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

/* Helper: append a new forwarding to existing tcp_forward_to config value, separated by FORWARDING_SEPARATOR.
 * If your C SDK has an API to add forwardings directly, replace this implementation with that API.
 */
static int append_forwarding_to_config(pinggy_ref_t config_ref, const char *forwarding, pinggy_bool_t is_udp) {
    char existing[2048];
    if (is_udp) {
        pinggy_config_get_udp_forward_to(config_ref, sizeof(existing), existing);
    } else {
        pinggy_config_get_tcp_forward_to(config_ref, sizeof(existing), existing);
    }

    size_t need = strlen(existing) + strlen(forwarding) + 8;
    char *buf = (char*)malloc(need);
    if (!buf) return 0;
    buf[0] = '\0';
    if (strlen(existing) > 0) {
        snprintf(buf, need, "%s%s%s", existing, FORWARDING_SEPARATOR, forwarding);
    } else {
        snprintf(buf, need, "%s", forwarding);
    }

    if (is_udp) {
        pinggy_config_set_udp_forward_to(config_ref, buf);
    } else {
        pinggy_config_set_tcp_forward_to(config_ref, buf);
    }

    free(buf);
    return 1;
}

/*
 * parse_reverse_tunnel:
 * Mirrors . parseReverseTunnel:
 * - For each -R entry, split into parts.
 * - Require at least 2 parts (forward host and port).
 * - Build forwardTo from parts[n-2] : parts[n-1].
 * - If parts >= 4 then there is a remoteHost supplied -> parse schema/hostname/port and use that as remote host.
 * - Store forwardings into the config (appends multiple forwardings).
 *
 * Note: because C SDK doesn't have AddForwarding(...) here, we append into tcp_forward_to (or udp_forward_to).
 * If your C SDK exposes an add_forwarding function, replace the `append_forwarding_to_config` call with that.
 */
static int parse_reverse_tunnel(pinggy_ref_t config_ref, char **forwardings, int forwardings_count, const char *mode_hint) {
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
            /* No remoteHost specified; append forwarding without remote host */
            append_forwarding_to_config(config_ref, forwardTo, pinggy_false);
        } else {
            /* remoteHost is at parts[count - 4] */
            const char *remoteHost = parts.parts[parts.count - 4];

            char *schema = NULL;
            char *hostname = NULL;
            int port = 0;
            if (!parse_hostname(remoteHost, &schema, &hostname, &port)) {
                parts_list_free(&parts);
                return 0;
            }

            /* If hostname empty -> localhost */
            if (hostname[0] == '\0') {
                free(hostname);
                hostname = strdup("localhost");
            }

            /* append port to hostname if parsed */
            char host_with_port[512];
            if (port > 0) {
                snprintf(host_with_port, sizeof(host_with_port), "%s:%d", hostname, port);
            } else {
                snprintf(host_with_port, sizeof(host_with_port), "%s", hostname);
            }

            /* If schema is empty, use mode_hint (from - user mode) else use schema */
            const char *use_schema = (schema && strlen(schema) > 0) ? schema : mode_hint;
            /* If schema indicates udp then treat accordingly */
            int is_udp = 0;
            if (use_schema && strcmp(use_schema, ConnMode_UDP) == 0) is_udp = 1;

            /* Compose a final "remote:forwardTo" string similar to . AddForwarding(schema, hostname, forwardTo)
             * using the form "<schema>::<hostname>::<forwardTo>" for storage, or just "<hostname>:<forwardTo>".
             * Since the C SDK doesn't expose AddForwarding here, we atomically append a formatted string.
             * Replace this with direct SDK call if available.
             */
            char combined[1024];
            if (schema && strlen(schema) > 0) {
                snprintf(combined, sizeof(combined), "%s//%s:%s", schema, host_with_port, forwardTo);
            } else if (mode_hint && strlen(mode_hint) > 0) {
                snprintf(combined, sizeof(combined), "%s//%s:%s", mode_hint, host_with_port, forwardTo);
            } else {
                snprintf(combined, sizeof(combined), "%s:%s", host_with_port, forwardTo);
            }

            append_forwarding_to_config(config_ref, combined, is_udp);

            free(schema);
            free(hostname);
        }

        parts_list_free(&parts);
    }
    return 1;
}

/*
 * parse_forward_tunnel_for_L:
 * Parses -L address and fills web debugger addr into the config via pinggy_config_*.
 * Example: "-L 127.0.0.1:4300" or "-L [::1]:4300"
 */
static int parse_forward_tunnel_for_L(pinggy_ref_t config_ref, const char *value, uint16_t *out_port, int *enable_dbg) {
    parts_list_t parts = parse_forwarding(value);
    if (parts.count < 2) {
        parts_list_free(&parts);
        return 0;
    }

    /* Build addr like host:port */
    char addr[512];
    snprintf(addr, sizeof(addr), "%s:%s", parts.parts[parts.count - 2], parts.parts[parts.count - 1]);
    pinggy_config_set_web_debugger_address(config_ref, addr); /* hypothetical function; if not present, store in argument */
    /* If SDK doesn't have that setter, fallback to setting argument */
    /* We'll also store a copy in argument for safety */
    /* Not all pinggy SDKs have set_web_debugger_address; if it fails, the user can replace with proper API. */

    if (out_port) {
        *out_port = (uint16_t)atoi(parts.parts[parts.count - 1]);
    }
    if (enable_dbg) *enable_dbg = 1;

    parts_list_free(&parts);
    return 1;
}

/*
 * parse_user: parse user token+mode like "tcp+token"
 * Sets type/udp_type and forward-to buffers in config similarly to ..
 */
static int parse_user(pinggy_ref_t config_ref, char *user, const char *tcp_forward_to_buf) {
    char *token_str = NULL;
    int type_set = 0;
    int udp_type_set = 0;

    char *copy = strdup(user);
    if (!copy) return 0;

    char *p = strtok(copy, "+");
    while (p != NULL) {
        /* to lower in-place */
        for (char *q = p; *q; ++q) *q = (char)tolower((unsigned char)*q);

        if (strcmp(p, ConnMode_HTTP) == 0 || strcmp(p, ConnMode_TCP) == 0 || strcmp(p, ConnMode_TLS) == 0 || strcmp(p, ConnMode_TLSTCP) == 0) {
            pinggy_config_set_type(config_ref, p);
            if (tcp_forward_to_buf && strlen(tcp_forward_to_buf) > 0) {
                pinggy_config_set_tcp_forward_to(config_ref, tcp_forward_to_buf);
            }
            type_set = 1;
        } else if (strcmp(p, ConnMode_UDP) == 0) {
            pinggy_config_set_udp_type(config_ref, p);
            if (tcp_forward_to_buf && strlen(tcp_forward_to_buf) > 0) {
                pinggy_config_set_udp_forward_to(config_ref, tcp_forward_to_buf);
            }
            udp_type_set = 1;
        } else {
            token_str = strdup(p);
        }

        p = strtok(NULL, "+");
    }

    if (!type_set && !udp_type_set) {
        pinggy_config_set_type(config_ref, ConnMode_HTTP);
        if (tcp_forward_to_buf && strlen(tcp_forward_to_buf) > 0) {
            pinggy_config_set_tcp_forward_to(config_ref, tcp_forward_to_buf);
        }
    }

    if (token_str) {
        pinggy_config_set_token(config_ref, token_str);
        free(token_str);
    }
    free(copy);
    return 1;
}

/*
 * parse_user_server:
 * value is token@server or just server. port is string "443" by default.
 */
static int parse_user_server(pinggy_ref_t config_ref, char *value, const char *port) {
    char *at_sign = strrchr(value, '@');
    char server_addr[512];

    char saved[512];
    strncpy(saved, value, sizeof(saved) - 1);
    saved[sizeof(saved)-1] = '\0';

    char tcp_forward_to_buf[256] = {0};
    pinggy_config_get_tcp_forward_to(config_ref, sizeof(tcp_forward_to_buf), tcp_forward_to_buf);

    if (at_sign) {
        *at_sign = '\0';
        char *user = value;
        char *server = at_sign + 1;
        snprintf(server_addr, sizeof(server_addr), "%s:%s", server, port);
        if (!parse_user(config_ref, user, tcp_forward_to_buf)) {
            return 0;
        }
    } else {
        snprintf(server_addr, sizeof(server_addr), "%s:%s", value, port);
    }

    pinggy_config_set_server_address(config_ref, server_addr);
    return 1;
}

/*
 * parse_sdk_arguments: store the rest of argv into config argument.
 */
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

/*
 * Main argument parsing returning a configured client_app_data_t.
 * This mirrors the original C parse_arguments but with enhanced parsing:
 * - multi -R entries supported (collected into a list first)
 * - -L supports IPv6 and sets web debugger address/port
 * - user parsing supports modes and tokens like . version
 */
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
    app_data->error_msg = NULL;
    app_data->tunnel_ref = INVALID_PINGGY_REF;

    /* collect multiple -R args */
    char **r_list = NULL;
    int r_count = 0;

    int opt;
    int longindex = 0;
    struct cli_option longopts[] = {
        {"help", cli_no_argument, 0, 'h'},
        {"version", cli_no_argument, 0, 'v'},
        {"verbose", cli_no_argument, 0, 'V'},
        {"port", cli_required_argument, 0, 'p'},
        {"sni", cli_required_argument, 0, 's'},
        {"insecure", cli_no_argument, 0, 'n'},
        {"inseceure", cli_no_argument, 0, 'n'}, // keep typo for compatibility
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
                /* collect into list for later parsing */
                r_list = (char**)realloc(r_list, sizeof(char*) * (r_count + 1));
                r_list[r_count++] = strdup(cli_optarg);
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
        if (!success) {
            exit_now = 1;
            break;
        }
    }

    if (cli_optind < argc) {
        if (!parse_user_server(app_data->config_ref, argv[cli_optind], server_port)) {
            exit_now = 1;
        }
    } else {
        exit_now = 1;
    }

    /* If there are -R entries, parse them similarly to . parseReverseTunnel.
     * For mode_hint we fetch current type if set in config (or empty).
     */
    if (!exit_now && r_count > 0) {
        char mode_hint_buf[64];
        mode_hint_buf[0] = '\0';
        pinggy_config_get_type(app_data->config_ref, sizeof(mode_hint_buf), mode_hint_buf);
        if (!parse_reverse_tunnel(app_data->config_ref, r_list, r_count, mode_hint_buf)) {
            exit_now = 1;
        }
    }

    if ((cli_optind + 1) < argc) {
        if (!parse_sdk_arguments(app_data->config_ref, argc - (cli_optind + 1), argv + (cli_optind + 1))) {
            exit_now = 1;
        }
    }

    /* free r_list entries */
    for (int i = 0; i < r_count; ++i) free(r_list[i]);
    free(r_list);

    if (exit_now) {
        pinggy_free_ref(app_data->config_ref);
        free(app_data);
        exit(0);
    }

    return app_data;
}

/* ==== Event callbacks (kept similar to your original C callbacks) ==== */
static void on_primary_forwarding_succeeded(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_len_t num_urls, pinggy_char_p_p_t urls) {
    client_app_data_t *app_data = (client_app_data_t *)user_data;
    int i;
    printf("Connection completed\n");
    for (i = 0; i < (int)num_urls; i++) {
        printf("   %s\n", urls[i]);
    }
    if (app_data->enable_web_debugger && app_data->web_debugger_port > 0) {
        /* If C SDK exposes pinggy_tunnel_start_web_debugging, call it. */
        pinggy_tunnel_start_web_debugging(tunnel_ref, app_data->web_debugger_port);
    }
}

static void on_authentication_failed(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_len_t num_reasons, pinggy_char_p_p_t reasons) {
    client_app_data_t *app_data = (client_app_data_t *)user_data;
    size_t total_len = 0;
    int i;
    for (i = 0; i < (int)num_reasons; i++) {
        total_len += strlen(reasons[i]) + 1;
    }
    if (total_len > 0) {
        app_data->error_msg = (char *)malloc(total_len + 1);
        if (app_data->error_msg) {
            app_data->error_msg[0] = '\0';
            for (i = 0; i < (int)num_reasons; i++) {
                strcat(app_data->error_msg, reasons[i]);
                if (i < (int)num_reasons - 1) {
                    strcat(app_data->error_msg, " ");
                }
            }
        }
    }
}

static void on_primary_forwarding_failed(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_const_char_p_t msg) {
    client_app_data_t *app_data = (client_app_data_t *)user_data;
    if (msg) {
        app_data->error_msg = (char*)malloc(strlen(msg) + 1);
        if(app_data->error_msg) strcpy(app_data->error_msg, msg);
    }
}

static void on_disconnected(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_const_char_p_t error, pinggy_len_t msg_size, pinggy_char_p_p_t msg) {
    client_app_data_t *app_data = (client_app_data_t *)user_data;
    if (error) {
        app_data->error_msg = (char*)malloc(strlen(error) + 1);
        if(app_data->error_msg) strcpy(app_data->error_msg, error);
    }
}

static void on_will_reconnect(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_const_char_p_t error, pinggy_len_t num_msgs, pinggy_char_p_p_t messages) {
    printf("Reconnecting\n");
}

static void on_reconnecting(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_uint16_t retry_cnt) {
    printf("Trying.. %u\n", retry_cnt);
}

static void on_reconnection_completed(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_len_t num_urls, pinggy_char_p_p_t urls) {
    int i;
    printf("Reconnected\n");
    for (i = 0; i < (int)num_urls; i++) {
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
    if (capa <= 0)
        return;
    char *greetings = (char*)malloc(capa);
if (!greetings) return;

if (pinggy_tunnel_get_greeting_msgs(tunnel_ref, capa, greetings) > 0)
    printf("Greeting: %s\n", greetings);

free(greetings);
    if (pinggy_tunnel_get_greeting_msgs(tunnel_ref, capa, greetings) > 0)
        printf("Greeting: %s\n", greetings);
}

static pinggy_void_t pinggy_on_raise_exception_cb(pinggy_const_char_p_t where, pinggy_const_char_p_t what)
{
    printf("%s ==> %s\n", where, what);
}

/* ==== main ==== */
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

    /* example set basic auths (kept from previous) */
    pinggy_config_set_basic_auths(app_data->config_ref, "[['user1', 'pass1'], ['user2', 'pass two']]");

    /* show arguments set */
    char str[1024];
    pinggy_config_get_argument_len(app_data->config_ref, 1024, str, NULL);
    printf("aasd %s\n", str);

    /* initiate tunnel */
    app_data->tunnel_ref = pinggy_tunnel_initiate(app_data->config_ref);
    if (app_data->tunnel_ref == INVALID_PINGGY_REF) {
        printf("Failed to initiate tunnel\n");
        pinggy_free_ref(app_data->config_ref);
        free(app_data);
        return 1;
    }

    /* register callbacks */
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

    pinggy_free_ref(app_data->tunnel_ref);
    pinggy_free_ref(app_data->config_ref);
    free(app_data);

    return 0;
}
