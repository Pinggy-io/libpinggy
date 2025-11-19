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
#define ConnMode_UDP    "udp"

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
    char web_debugger_addr[256];
    uint16_t web_debugger_port;
    int enable_web_debugger;
    char mode[32]; /* lowercased mode string if provided */
    char *error_msg;
} client_app_data_t;
static int
split_forwarding(const char *val, char parts[][128], int max_parts)
{
    int count = 0;
    const char *start = val;

    while (*start && count < max_parts) {
        /* skip empty/repeated colons */
        if (*start == ':') {
            start++;
            continue;
        }

        if (*start == '[') { /* IPv6 bracket handling */
            const char *end = strchr(start, ']');
            if (!end) break;
            size_t len = (size_t)(end - start + 1);
            if (len >= 128) len = 127;
            strncpy(parts[count], start, len);
            parts[count][len] = '\0';
            count++;
            start = end + 1;
            if (*start == ':') start++;
        } else {
            const char *colon = strchr(start, ':');
            if (!colon) {
                /* last token */
                strncpy(parts[count], start, 127);
                parts[count][127] = '\0';
                count++;
                break;
            }
            size_t len = (size_t)(colon - start);
            if (len > 0) {
                if (len >= 128) len = 127;
                strncpy(parts[count], start, len);
                parts[count][len] = '\0';
                count++;
            }
            start = colon + 1;
        }
    }

    return count;
}

/* ---- parse_hostname - mimic  parseHostname behavior ----
   Input: remoteHost string which may be "[schema//]hostname[/port]" or "hostname[/port]"
   Output: allocate schema_out and hostname_out (caller must free), port_out
   Returns 0 on success, non-zero on parse error.
   Behavior:
     - If schema present (prefix before "//") and is one of known ConnMode_* values, keep it,
       otherwise schema_out will be empty string.
     - Port is parsed from trailing '/<port>' if present, otherwise 0.
     - If hostname becomes empty set to empty string (caller may default to "localhost" later).
*/
static int
parse_hostname(const char *host_in, char **schema_out, char **hostname_out, int *port_out)
{
    const char *s = host_in;
    char *schema = NULL;
    char *hostname = NULL;
    int port = 0;

    /* copy input so we can manipulate */
    char *buf = strdup(host_in ? host_in : "");
    if (!buf) return -1;

    char *schema_sep = strstr(buf, "//");
    if (schema_sep) {
        *schema_sep = '\0';
        schema = strdup(buf);
        /* move pointer to after // */
        char *rest = schema_sep + 2;
        hostname = strdup(rest ? rest : "");
    } else {
        schema = strdup("");
        hostname = strdup(buf);
    }

    /* find last '/' to detect port */
    char *last_slash = strrchr(hostname, '/');
    if (last_slash) {
        char *port_str = last_slash + 1;
        if (port_str && *port_str) {
            port = atoi(port_str); 
            /* remove '/port' from hostname */
            *last_slash = '\0';
        }
    }

    /* validate schema: only accept known modes, else clear schema */
    {
        char schema_lower[32] = {0};
        size_t i;
        for (i = 0; i < strlen(schema) && i + 1 < sizeof(schema_lower); ++i) {
            schema_lower[i] = (char)tolower((unsigned char)schema[i]);
        }
        schema_lower[i] = '\0';
        if (!(strcmp(schema_lower, ConnMode_HTTP) == 0 ||
              strcmp(schema_lower, ConnMode_TCP) == 0 ||
              strcmp(schema_lower, ConnMode_TLS) == 0 ||
              strcmp(schema_lower, ConnMode_TLSTCP) == 0 ||
              strcmp(schema_lower, ConnMode_UDP) == 0)) {
            free(schema);
            schema = strdup("");
        } else {
            /* keep normalized (lowercase) schema */
            free(schema);
            schema = strdup(schema_lower);
        }
    }

    *schema_out = schema ? schema : strdup("");
    *hostname_out = hostname ? hostname : strdup("");
    *port_out = port;

    free(buf);
    return 0;
}
static int
parse_forward_tunnel(client_app_data_t* app_data, const char* value)
{
    char parts[8][128];
    int count = split_forwarding_exact(value, parts, 8);

    // Must have at least 2 components
    if (count < 2) return 0;

    const char* host = parts[count - 2];
    const char* port_str = parts[count - 1];

    // Validate port
    int port = atoi(port_str);
    if (port <= 0 || port > 65535) return 0;

    // Build "host:port"
    // WebDebuggerAddr = values[size-2] + ":" + values[size-1];
    snprintf(app_data->web_debugger_port, sizeof(app_data->web_debugger_port),
             "%s:%s", host, port_str);

    app_data->enable_web_debugger = 1;
    return 1;
}
static int
parse_reverse_tunnel(pinggy_ref_t config_ref, const char *value, const char *mode /* may be empty */)
{
    char parts[16][128];
    /* use larger max to be robust; split_forwarding returns up to provided max */
    int count = split_forwarding(value, parts, 16);

    if (count < 2) {
        return 0;
    }

    /* build forwardTo = parts[count-2] + ":" + parts[count-1] */
    char forward_to[512];
    snprintf(forward_to, sizeof(forward_to), "%s:%s", parts[count - 2], parts[count - 1]);

    if (count < 4) {
        /* no remote host specified; schema empty */
        /* In .: config->SdkConfig->AddForwarding(config->Mode, "", forwardTo); */
        pinggy_config_add_forwarding(config_ref, mode ? mode : "", "", forward_to);
        return 1;
    } else {
        /* remoteHost is parts[count - 4] */
        char *schema = NULL;
        char *hostname = NULL;
        int port = 0;
        if (parse_hostname(parts[count - 4], &schema, &hostname, &port) != 0) {
            if (schema) free(schema);
            if (hostname) free(hostname);
            return 0;
        }
        if (hostname[0] == '\0') {
            free(hostname);
            hostname = strdup("localhost");
        }
        /* append :port to hostname (if port > 0) */
        char host_with_port[512];
        if (port > 0) {
            snprintf(host_with_port, sizeof(host_with_port), "%s:%d", hostname, port);
        } else {
            snprintf(host_with_port, sizeof(host_with_port), "%s", hostname);
        }

        if (schema[0] == '\0') {
            /* use default mode */
            pinggy_config_add_forwarding(config_ref, mode ? mode : "", host_with_port, forward_to);
        } else {
            /* use provided schema */
            pinggy_config_add_forwarding(config_ref, schema, host_with_port, forward_to);
        }

        free(schema);
        free(hostname);
        return 1;
    }
}

/* ---- parse_forward_tunnel: set web debugger addr (host:port) and enable flag.
   Equivalent to . parseForwardTunnel(ClientConfigPtr).
*/
static int
parse_forward_tunnel(client_app_data_t *app_data, const char *value)
{
    char parts[8][128];
    int count = split_forwarding(value, parts, 8);
    if (count < 2) return 0;

    /* build addr = parts[count-2] ":" parts[count-1] */
    snprintf(app_data->web_debugger_addr, sizeof(app_data->web_debugger_addr), "%s:%s", parts[count - 2], parts[count - 1]);
    app_data->web_debugger_port = (uint16_t)atoi(parts[count - 1]);
    app_data->enable_web_debugger = 1;
    return 1;
}

/* ---- parse_user: token@server style split's user part parsing
   Behavior mirrors . parseUser:
     - tokens separated by '+'
     - if token equals known mode -> set config type (mode)
     - else -> token is treated as authentication token (last non-mode token wins)
*/
static int
parse_user(pinggy_ref_t config_ref, char *user, char *mode_buf, size_t mode_buf_len)
{
    /* copy user into mutable buffer (strtok modifies it) */
    if (!user) return 0;
    char *work = strdup(user);
    if (!work) return 0;

    char *saveptr = NULL;
    char *p = strtok_r(work, "+", &saveptr);
    char *token_str = NULL;
    int type_set = 0;
    while (p != NULL) {
        /* lowercase p in-place */
        for (char *q = p; *q; ++q) *q = (char)tolower((unsigned char)*q);

        if (strcmp(p, ConnMode_HTTP) == 0 || strcmp(p, ConnMode_TCP) == 0 ||
            strcmp(p, ConnMode_TLS) == 0 || strcmp(p, ConnMode_TLSTCP) == 0) {
            pinggy_config_set_type(config_ref, p);
            /* record mode in app_data if caller wants it */
            if (mode_buf && mode_buf_len > 0) {
                strncpy(mode_buf, p, mode_buf_len - 1);
                mode_buf[mode_buf_len - 1] = '\0';
            }
            type_set = 1;
        } else {
            /* treat as token */
            token_str = p;
        }
        p = strtok_r(NULL, "+", &saveptr);
    }

    if (!type_set) {
        /* default to HTTP as in . parseUser sets empty mode? The . parseUser left mode empty unless provided; but parseArguments later defaults behavior.
           To match original . exactly we should not force HTTP here; keep mode_buf untouched. . only sets default when no mode found in parseArguments flow.
        */
    }

    if (token_str) {
        pinggy_config_set_token(config_ref, token_str);
    }

    free(work);
    return 1;
}

/* ---- parse_user_server: split token@server and call parse_user and set server:port */
static int
parse_user_server(pinggy_ref_t config_ref, char *value, const char *port, char *mode_buf, size_t mode_buf_len)
{
    char *at_sign = strrchr(value, '@');
    char server_addr[256];

    if (at_sign) {
        /* separate user and server by replacing '@' with '\0' */
        *at_sign = '\0';
        char *user = value;
        char *server = at_sign + 1;
        snprintf(server_addr, sizeof(server_addr), "%s:%s", server, port);
        if (!parse_user(config_ref, user, mode_buf, mode_buf_len)) {
            return 0;
        }
    } else {
        snprintf(server_addr, sizeof(server_addr), "%s:%s", value, port);
    }

    pinggy_config_set_server_address(config_ref, server_addr);
    return 1;
}

/* ---- parse_sdk_arguments: store remaining args into SDK config (string join) ----
   Equivalent to . parseSdkArguments
*/
static int
parse_sdk_arguments(pinggy_ref_t config_ref, int argc, char *argv[])
{
    if (argc <= 0) return 1;
    /* join with spaces */
    size_t needed = 0;
    for (int i = 0; i < argc; ++i) {
        if (argv[i]) needed += strlen(argv[i]) + 1;
    }
    char *arg_str = (char *)malloc(needed + 1);
    if (!arg_str) return 0;
    arg_str[0] = '\0';
    for (int i = 0; i < argc; ++i) {
        if (!argv[i]) continue;
        if (strlen(arg_str) > 0) strcat(arg_str, " ");
        strcat(arg_str, argv[i]);
    }
    pinggy_config_set_arguments(config_ref, arg_str);
    free(arg_str);
    return 1;
}

/* ---- parse_arguments: full CLI parsing loop (strictly following . behavior) ----
   Note: We mimic the . command-line options and flow.
*/
static client_app_data_t *
parse_arguments(int argc, char *argv[])
{
    client_app_data_t *app_data = (client_app_data_t*)calloc(1, sizeof(client_app_data_t));
    if (!app_data) return NULL;

    app_data->config_ref = pinggy_create_config();
    if (app_data->config_ref == INVALID_PINGGY_REF) {
        free(app_data);
        return NULL;
    }
    app_data->web_debugger_port = 4300;
    app_data->web_debugger_addr[0] = '\0';
    app_data->enable_web_debugger = 0;
    app_data->mode[0] = '\0';
    app_data->error_msg = NULL;

    int opt;
    int longindex = 0;

    struct cli_option longopts[] = {
        {"help",    0, 0, 'h'},
        {"version", 0, 0, 'v'},
        {"verbose", 0, 0, 'V'},
        {"port",    1, 0, 'p'},
        {"sni",     1, 0, 's'},
        {"insecure",0, 0, 'n'},
        {NULL, 0, 0, 0}
    };

    int exitNow = 0;
    const char *prog = argc > 0 ? argv[0] : "prog";
    char *serverPort = "443";

    while ((opt = cli_getopt_long(argc, argv, "ahvVno:R:L:p:s:r", longopts, &longindex)) != -1) {
        int success = 1;
        switch (opt) {
        case 'h':
            /* print help (user must provide printHelpOptions) */
            /* printHelpOptions(prog); */ /* assume implemented elsewhere */
            exit(0);
            break;
        case 'v':
            printf("v%d.%d.%d\n", PinggyVersionMajor, PinggyVersionMinor, PinggyVersionPatch);
            exit(0);
            break;
        case 'V':
            /* SetGlobalLogEnable(true); -> call equivalent if present */
            break;
        case 'o':
            printf("Output option with value: %s\n", cli_optarg);
            break;
        case 'p':
            serverPort = cli_optarg;
            break;
        case 'R':
            success = parse_reverse_tunnel(app_data->config_ref, cli_optarg, app_data->mode);
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
        case 'a':
            pinggy_config_set_advanced_parsing(app_data->config_ref, pinggy_true);
            break;
        case '?':
        default:
            printf("Unknown option or missing argument\n");
            break;
        }
        if (!success) {
            exitNow = 1;
            break;
        }
    }

    /* remaining non-option arguments: expect token@server */
    if (cli_optind < argc) {
        /* copy argv[cli_optind] because parse_user_server may mutate the string */
        char *valcopy = strdup(argv[cli_optind]);
        if (!valcopy) {
            exitNow = 1;
        } else {
            if (!parse_user_server(app_data->config_ref, valcopy, serverPort, app_data->mode, sizeof(app_data->mode))) {
                free(valcopy);
                exitNow = 1;
            }
            free(valcopy);
        }
    } else {
        exitNow = 1;
    }

    /* In .: after parse_user_server, parseReverseTunnel(config) is called (using stored forwardings)
       For strict translation we assume reverse-tunnels were already processed via -R flags above.
       So skip extra call here.
    */

    /* parse SDK-specific trailing args (if any) */
    if ((cli_optind + 1) < argc) {
        if (!parse_sdk_arguments(app_data->config_ref, argc - (cli_optind + 1), argv + (cli_optind + 1))) {
            exitNow = 1;
        }
    }

    if (exitNow) {
        pinggy_free_ref(app_data->config_ref);
        free(app_data);
        exit(0);
    }

    return app_data;
}

/* ---- Event callbacks: match . prints and behavior ---- */

/* primary forwarding (tunnel) established */
static void
on_primary_forwarding_succeeded(void *user_data, pinggy_ref_t tunnel_ref, pinggy_len_t num_urls, pinggy_char_p_p_t urls)
{
    client_app_data_t *app_data = (client_app_data_t*)user_data;
    printf("Connection completed\n");
    for (size_t i = 0; i < num_urls; ++i) {
        printf("   %s\n", urls[i]);
    }

    /* Print greeting message exactly like . did after tunnel established */
    {
        pinggy_capa_t capa = 0;
        if (pinggy_tunnel_get_greeting_msgs_len(tunnel_ref, 0, NULL, &capa) >= 0 && capa > 0) {
            char *greetings = (char*)malloc(capa + 1);
            if (greetings) {
                if (pinggy_tunnel_get_greeting_msgs(tunnel_ref, capa, greetings) > 0) {
                    greetings[capa] = '\0';
                    printf("Greeting: %s\n", greetings);
                }
                free(greetings);
            }
        }
    }

    if (app_data->enable_web_debugger && app_data->web_debugger_port > 0) {
        pinggy_tunnel_start_web_debugging(tunnel_ref, app_data->web_debugger_port);
    }
}

/* authentication failed - collect reasons into app_data->error_msg like . */
static void
on_authentication_failed(void *user_data, pinggy_ref_t tunnel_ref, pinggy_len_t num_reasons, pinggy_char_p_p_t reasons)
{
    client_app_data_t *app_data = (client_app_data_t*)user_data;
    size_t total_len = 0;
    for (size_t i = 0; i < num_reasons; ++i) {
        total_len += strlen(reasons[i]) + 1;
    }
    if (total_len > 0) {
        app_data->error_msg = (char*)malloc(total_len + 1);
        if (app_data->error_msg) {
            app_data->error_msg[0] = '\0';
            for (size_t i = 0; i < num_reasons; ++i) {
                strcat(app_data->error_msg, reasons[i]);
                if (i + 1 < num_reasons) strcat(app_data->error_msg, " ");
            }
        }
    }
}

/* primary forwarding failed */
static void
on_primary_forwarding_failed(void *user_data, pinggy_ref_t tunnel_ref, const char *msg)
{
    client_app_data_t *app_data = (client_app_data_t*)user_data;
    if (msg) {
        app_data->error_msg = (char*)malloc(strlen(msg) + 1);
        if (app_data->error_msg) strcpy(app_data->error_msg, msg);
    }
}

/* disconnected */
static void
on_disconnected(void *user_data, pinggy_ref_t tunnel_ref, const char *error, pinggy_len_t msg_size, pinggy_char_p_p_t msg)
{
    client_app_data_t *app_data = (client_app_data_t*)user_data;
    if (error) {
        app_data->error_msg = (char*)malloc(strlen(error) + 1);
        if (app_data->error_msg) strcpy(app_data->error_msg, error);
    }
}

/* will reconnect */
static void
on_will_reconnect(void *user_data, pinggy_ref_t tunnel_ref, const char *error, pinggy_len_t num_msgs, pinggy_char_p_p_t messages)
{
    (void)user_data; (void)tunnel_ref; (void)error; (void)num_msgs; (void)messages;
    printf("Reconnecting\n");
}

/* reconnecting */
static void
on_reconnecting(void *user_data, pinggy_ref_t tunnel_ref, pinggy_uint16_t retry_cnt)
{
    (void)user_data; (void)tunnel_ref;
    printf("Trying.. %u\n", (unsigned)retry_cnt);
}

/* reconnection completed */
static void
on_reconnection_completed(void *user_data, pinggy_ref_t tunnel_ref, pinggy_len_t num_urls, pinggy_char_p_p_t urls)
{
    (void)tunnel_ref;
    printf("Reconnected\n");
    for (size_t i = 0; i < num_urls; ++i) {
        printf("   %s\n", urls[i]);
    }

    /* print greeting like  */
    {
        pinggy_capa_t capa = 0;
        if (pinggy_tunnel_get_greeting_msgs_len(tunnel_ref, 0, NULL, &capa) >= 0 && capa > 0) {
            char *greetings = (char*)malloc(capa + 1);
            if (greetings) {
                if (pinggy_tunnel_get_greeting_msgs(tunnel_ref, capa, greetings) > 0) {
                    greetings[capa] = '\0';
                    printf("Greeting: %s\n", greetings);
                }
                free(greetings);
            }
        }
    }
}

/* reconnection failed */ 
static void
on_reconnection_failed(void *user_data, pinggy_ref_t tunnel_ref, pinggy_uint16_t retry_cnt)
{
    (void)user_data; (void)tunnel_ref;
    printf("Reconnection failed after %u tries\n", (unsigned)retry_cnt);
}

/* usage update - (only print update message) */
static void
on_usage_update(void *user_data, pinggy_ref_t tunnel_ref, const char *usages)
{
    (void)user_data; (void)tunnel_ref;
    printf("Update msg: %s\n", usages);
}

/* exception callback */
static void
pinggy_on_raise_exception_cb(const char *where, const char *what)
{
    printf("%s ==> %s\n", where, what);
    exit(1);
}



// upadate after this rest done 
int
main(int argc, char *argv[])
{
#ifdef __WINDOWS_OS__
    WindowsSocketInitialize();
#endif
    pinggy_set_log_enable(pinggy_true);  // enable logs like . output

#if defined(__LINUX_OS__) || defined(__MAC_OS__)
    ignore_sigpipe();
#endif

    pinggy_set_on_exception_callback(pinggy_on_raise_exception_cb);

    client_app_data_t* app_data = parse_arguments(argc, argv);
    if (!app_data) {
        return 1;
    }

    // Optional: same as . sample
    //pinggy_config_set_basic_auths(app_data->config_ref, "[[\"user1\", \"pass1\"], [\"user2\", \"pass two\"]]");

    // Remove the 'return 0' here
    // char str[1024];
    // pinggy_config_get_argument_len(app_data->config_ref, 1024, str, NULL);
    // printf("aasd %s\n", str);
    // return 0;

    // ---- Start the tunnel like in . ----
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
