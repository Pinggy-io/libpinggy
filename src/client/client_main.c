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
#include <stdbool.h>
#include <stdint.h>

#include <pinggy.h>
#include "cli_getopt.h"

#ifndef PLATFORM_CONFIG_INCLUDED

#define PinggyVersionMajor 0
#define PinggyVersionMinor 0
#define PinggyVersionPatch 0
#endif

// Connection modes
static const char *conn_mode_http      = "http";
static const char *conn_mode_tcp       = "tcp";
static const char *conn_mode_tls       = "tls";
static const char *conn_mode_tlstcp    = "tlstcp";
static const char *conn_mode_udp       = "udp";

// --- Helper: String Vector (to replace std::vector<tString>) ---
typedef struct
{
    char         **items;
    size_t         size;
} string_vector;

void
vector_init(string_vector *v)
{
    v->items = NULL; v->size = 0;
}

void
vector_push(string_vector *v, const char *str)
{
    v->items = (char**)realloc(v->items, sizeof(char*) * (v->size + 1));
    v->items[v->size++] = strdup(str);
}

void
vector_free(string_vector *v)
{
    if (!v)
    {
        return;
    }
    for(size_t i=0; i<v->size; i++) {
        free(v->items[i]);
    }
    free(v->items);
    v->items = NULL; v->size = 0;
}

// Helper to split strings (like Utils.hh)
string_vector
split_string(const char *str, const char *delim)
{
    string_vector v; vector_init(&v);
    char *dup = strdup(str);
    char *token = strtok(dup, delim);
    while(token) {
        vector_push(&v, token);
        token = strtok(NULL, delim);
    }
    free(dup);
    return v;
}

// --- Structure for client configuration ---
typedef struct
{
    string_vector       forwardings; // for storing the list of reverse tunnels
    pinggy_ref_t        sdk_config;
    char               *web_debugger_addr;
    bool                enable_web_debugger;
    char               *mode;

    // Added for C context
    pinggy_ref_t        tunnel_ref;
    char               *error;
} client_config;

client_config
*new_client_config_ptr()
{
    client_config *cfg = (client_config*)calloc(1, sizeof(client_config));
    vector_init(&cfg->forwardings);
    cfg->sdk_config = pinggy_create_config();
    cfg->web_debugger_addr = strdup("localhost:4300");
    cfg->enable_web_debugger = false;
    cfg->mode = strdup("");
    return cfg;
}

void
free_client_config(client_config *cfg)
{
    if(!cfg) return;
    vector_free(&cfg->forwardings);
    pinggy_free_ref(cfg->sdk_config);
    if(cfg->web_debugger_addr) free(cfg->web_debugger_addr);
    if(cfg->mode) free(cfg->mode);
    if(cfg->error) free(cfg->error);
    free(cfg);
}

// --- Logic Implementation ---

// function to parse forwarding address
// -R [[[schema//]remotehost[/port]:]0:]localhost:localport

string_vector
parse_forwarding(const char *val)
{
    string_vector result;
    vector_init(&result);

    if (!val) return result;
    char *value = strdup(val);
    char *p = value;

    while (*p != '\0') {
        if (*p == '[') {
            // IPv6 address in brackets
            char *close = strchr(p, ']');
            if (!close) {
                vector_push(&result, p);
                break;
            }
            // Include closing bracket
            size_t len = close - p + 1;
            char *ipv6 = (char*)malloc(len + 1);
            strncpy(ipv6, p, len);
            ipv6[len] = '\0';
            vector_push(&result, ipv6);
            free(ipv6);

            p = close + 1;
            if (*p == ':') p++;
            else break;
        } else {
            // Find next colon
            char *colon = strchr(p, ':');
            if (!colon) {
                vector_push(&result, p);
                break;
            }
            *colon = '\0';
            vector_push(&result, p);
            p = colon + 1;
        }
    }
    free(value);
    return result;
}

// Helper struct for Tuple return
typedef struct {
    char         *schema;
    char         *hostname;
    int           port;
} hostname_tuple;

hostname_tuple
parse_hostname(const char *host)
{
    hostname_tuple t = {NULL, NULL, 0};
    if (!host) return t;

    char *copy = strdup(host);
    char *curr_hostname = copy;

    char *schema_sep = strstr(curr_hostname, "//");
    if (schema_sep) {
        *schema_sep = '\0';
        t.schema = strdup(curr_hostname);
        curr_hostname = schema_sep + 2;
    } else {
        t.schema = strdup("");
    }

    char *last_slash = strrchr(curr_hostname, '/');
    if (last_slash) {
        *last_slash = '\0';
        t.port = atoi(last_slash + 1);
    }

    t.hostname = strdup(curr_hostname);

    // Schema validation logic from C++
    if (strcmp(t.schema, conn_mode_http) != 0 && strcmp(t.schema, conn_mode_tcp) != 0 &&
        strcmp(t.schema, conn_mode_tls) != 0 && strcmp(t.schema, conn_mode_tlstcp) != 0 &&
        strcmp(t.schema, conn_mode_udp) != 0) {
        free(t.schema);
        t.schema = strdup("");
    }

    free(copy);
    return t;
}

// called when the user provides -R
bool
parse_reverse_tunnel(client_config *config)
{
    for (size_t i = 0; i < config->forwardings.size; i++)
    {
        char *value = config->forwardings.items[i];
        string_vector values = parse_forwarding(value);

        if (values.size < 2) {
            vector_free(&values);
            return false;
        }

        char forward_to[512];
        snprintf(forward_to, sizeof(forward_to), "%s:%s",
                 values.items[values.size - 2], values.items[values.size - 1]);

        // Mimic try/catch block logic
        if (values.size < 4) {
            // AddForwarding(mode, "", forward_to)
            pinggy_config_add_forwarding(config->sdk_config, config->mode, "", forward_to);
        } else {
            char *remote_host = values.items[values.size - 4];
            hostname_tuple ht = parse_hostname(remote_host);

            if (!ht.hostname || strlen(ht.hostname) == 0) {
                if (ht.hostname) free(ht.hostname);
                ht.hostname = strdup("localhost");
            }

            char hostname_full[512];
            snprintf(hostname_full, sizeof(hostname_full), "%s:%d", ht.hostname, ht.port);

            if (!ht.schema || strlen(ht.schema) == 0) {
                pinggy_config_add_forwarding(config->sdk_config, config->mode, hostname_full, forward_to);
            } else {
                pinggy_config_add_forwarding(config->sdk_config, ht.schema, hostname_full, forward_to);
            }

            if (ht.schema) free(ht.schema);
            if (ht.hostname) free(ht.hostname);
        }

        vector_free(&values);
    }
    return true;
}

// called when the user provides -L
bool
parse_forward_tunnel(client_config *config, const char *value)
{
    string_vector values = parse_forwarding(value);
    if (values.size < 2) {
        vector_free(&values);
        return false;
    }

    char new_addr[512];
    snprintf(new_addr, sizeof(new_addr), "%s:%s",
             values.items[values.size - 2], values.items[values.size - 1]);

    if(config->web_debugger_addr) free(config->web_debugger_addr);
    config->web_debugger_addr = strdup(new_addr);
    config->enable_web_debugger = true;

    vector_free(&values);
    return true;
}

// parseUser: tcp+token -> Mode=tcp, Token=token
bool
parse_user(client_config *config, const char *user)
{
    string_vector values = split_string(user, "+");
    char token[1024] = "";

    for(size_t i=0; i < values.size; i++) {
        char *s = values.items[i];
        // StringToLower logic (inline)
        char sl[256];
        strncpy(sl, s, 255); sl[255] = '\0';
        for(char *p=sl; *p; ++p) *p = tolower((unsigned char)*p);

        if (strcmp(sl, conn_mode_http) == 0 || strcmp(sl, conn_mode_tcp) == 0 ||
            strcmp(sl, conn_mode_tls) == 0 || strcmp(sl, conn_mode_tlstcp) == 0 ||
            strcmp(sl, conn_mode_udp) == 0) {

            if (strlen(config->mode) > 0) {
                printf("You cannot provide more than one type of default forwarding type");
                vector_free(&values);
                return false;
            }
            if(config->mode) free(config->mode);
            config->mode = strdup(sl);
        } else {
            strcat(token, "+");
            strcat(token, s);
        }
    }

    if (strlen(token) > 1) {
        // Remove leading '+' and set token
        pinggy_config_set_token(config->sdk_config, token + 1);
        printf("token: %s\n", token + 1);
    }

    vector_free(&values);
    return true;
}

// parseUserServer: token@server
bool
parse_user_server(client_config *config, const char *value, const char *port)
{
    string_vector values = split_string(value, "@");
    if (values.size > 2) {
        vector_free(&values);
        return false;
    }

    bool success = true;
    char server_addr[512];
    const char *hostPart = values.items[values.size - 1];
    snprintf(server_addr, sizeof(server_addr), "%s:%s", hostPart, port);
    pinggy_config_set_server_address(config->sdk_config, server_addr);

    if (values.size > 1) {
        success = parse_user(config, values.items[values.size - 2]);
    }

    vector_free(&values);
    return success;
}

void
print_help_options(const char *prog)
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

bool
parse_sdk_arguments(client_config *config, int argc, char *argv[])
{
    size_t len = 0;
    for(int i=0; i<argc; i++) len += strlen(argv[i]) + 1;

    char *args = (char*)malloc(len + 1);
    args[0] = '\0';
    for(int i=0; i<argc; i++) {
        strcat(args, argv[i]);
        if(i < argc-1) strcat(args, " ");
    }

    pinggy_config_set_argument(config->sdk_config, args);
    free(args);
    return true;
}

client_config*
parse_arguments(int argc, char* argv[])
{
    client_config *config = new_client_config_ptr();

    int opt;
    int long_index = 0;

    struct cli_option longopts[] = {
        {"help", cli_no_argument, 0, 'h'},
        {"version", cli_no_argument, 0, 'v'},
        {"verbose", cli_no_argument, 0, 'V'},
        {"port", cli_required_argument, 0, 'p'},
        {"sni", cli_required_argument, 0, 's'},
        {"insecure", cli_required_argument, 0, 'n'},
        {NULL, cli_required_argument, 0, 'R'},
        {NULL, cli_required_argument, 0, 'L'},
        {NULL, cli_required_argument, 0, 'o'},
        {NULL, cli_required_argument, 0, 'n'},
        {0, 0, 0, 0}
    };

    bool exit_now = false;
    char *server_port = "443";
    const char *prog = argv[0];

    while ((opt = cli_getopt_long(argc, argv, "ahvVno:R:L:p:s:r", longopts, &long_index)) != -1) {
        bool success = true;
        switch (opt) {
        case 'h':
            print_help_options(prog);
            exit_now = true;
            exit(0);
            break;
        case 'v':
            printf("v%d.%d.%d\n", PinggyVersionMajor, PinggyVersionMinor, PinggyVersionPatch);
            exit_now = true;
            exit(0);
            break;
        case 'V':
            pinggy_set_log_enable(true);
            break;
        case 'o':
            printf("Output option with value: %s\n", cli_optarg);
            break;
        case 'p':
            server_port = cli_optarg;
            break;
        case 'R':
            vector_push(&config->forwardings, cli_optarg);
            break;
        case 'L':
            success = parse_forward_tunnel(config, cli_optarg);
            break;
        case 'n':
            pinggy_config_set_ssl(config->sdk_config, false);
            pinggy_config_set_insecure(config->sdk_config, true);
            break;
        case 's':
            pinggy_config_set_sni_server_name(config->sdk_config, cli_optarg);
            break;
        case 'r':
            pinggy_config_set_auto_reconnect(config->sdk_config, true);
            break;
        case 'a':
            pinggy_config_set_advanced_parsing(config->sdk_config, true);
            break;
        case '?':
            printf("Unknown option or missing argument\n");
            break;
        }
        if (!success) {
            exit_now = true;
            break;
        }
    }

    if (cli_optind < argc) {
        exit_now = !(parse_user_server(config, argv[cli_optind], server_port));
    } else {
        exit_now = true;
    }

    if (!exit_now)
        exit_now = !parse_reverse_tunnel(config);

    if (!exit_now && (cli_optind + 1) < argc) {
        exit_now = !(parse_sdk_arguments(config, argc - (cli_optind + 1), argv + (cli_optind + 1)));
    }

    if (exit_now) {
        free_client_config(config);
        exit(0);
    }

    return config;
}

// --- Callbacks ---

static void
on_tunnel_established(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_len_t num_urls, pinggy_char_p_p_t urls)
{
    client_config *config = (client_config*)user_data;
    printf("Connection completed\n");
    for (int i = 0; i < (int)num_urls; i++) {
        printf("   %s\n", urls[i]);
    }

    pinggy_capa_t capa = 0;
    pinggy_tunnel_get_greeting_msgs_len(tunnel_ref, 0, NULL, &capa);
    if (capa > 0) {
        char *greeting = (char*)malloc(capa);
        if (pinggy_tunnel_get_greeting_msgs(tunnel_ref, capa, greeting) > 0) {
             printf("Greeting: %s\n", greeting);
        }
        free(greeting);
    }

    if (config->enable_web_debugger && config->web_debugger_addr && strlen(config->web_debugger_addr) > 0) {
        pinggy_config_set_webdebugger_addr(config->sdk_config, config->web_debugger_addr);
        pinggy_tunnel_start_web_debugging(tunnel_ref, config->web_debugger_addr);
    }
}

static void
on_tunnel_failed(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_const_char_p_t msg)
{
    client_config *config = (client_config*)user_data;
    if(msg) config->error = strdup(msg);
}

static void
on_disconnected(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_const_char_p_t error, pinggy_len_t msg_size, pinggy_char_p_p_t msgs)
{
    client_config *config = (client_config*)user_data;
    if(error) config->error = strdup(error);
    if(msgs && msg_size > 0) printf("Disconnected msg: %s\n", msgs[0]);
}

static void
on_will_reconnect(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_const_char_p_t error, pinggy_len_t num_msgs, pinggy_char_p_p_t messages)
{
    printf("Will Reconnect\n");
    for (int i = 0; i < num_msgs; i ++){
        printf("    %s\n", messages[i]);
    }
}

static void
on_reconnecting(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_uint16_t count)
{
    printf("Trying.. %u\n", count);
}

static void
on_reconnection_completed(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_len_t num_urls, pinggy_char_p_p_t urls)
{
    printf("Reconnected\n");
    for (int i = 0; i < (int)num_urls; i++) {
        printf("   %s\n", urls[i]);
    }
}

static void
on_reconnection_failed(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_uint16_t tries)
{
    printf("Reconnection failed after %u tries\n", tries);
}

static void
on_usage_update(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_const_char_p_t msg)
{
    printf("Update msg: %s\n", msg);
}

// **THIS IS THE FUNCTION YOU WERE MISSING**
static void
on_forwardings_changed(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_const_char_p_t url_map)
{
    printf("%s\n", url_map);
}

static pinggy_void_t
pinggy_on_raise_exception_cb(pinggy_const_char_p_t where, pinggy_const_char_p_t what)
{
    printf("Exception: %s ==> %s\n", where, what);
}

// --- Main ---
int
main(int argc, char* argv[])
{
#ifdef __WINDOWS_OS__
    WindowsSocketInitialize();
#endif
    // Enable logs to see the debug info like token and "Initiated"
    pinggy_set_log_enable(true);

#if defined(__LINUX_OS__) || defined(__MAC_OS__)
    ignore_sigpipe();
#endif

    pinggy_set_on_exception_callback(pinggy_on_raise_exception_cb);

    client_config *config = parse_arguments(argc, argv);
    if (!config) {
        return 0;
    }

    config->tunnel_ref = pinggy_tunnel_initiate(config->sdk_config);
    if (config->tunnel_ref == INVALID_PINGGY_REF) {
        printf("Failed to initiate tunnel\n");
        free_client_config(config);
        return 0;
    }

    pinggy_tunnel_set_on_tunnel_established_callback(config->tunnel_ref, on_tunnel_established, config);
    pinggy_tunnel_set_on_tunnel_failed_callback(config->tunnel_ref, on_tunnel_failed, config);
    pinggy_tunnel_set_on_disconnected_callback(config->tunnel_ref, on_disconnected, config);
    pinggy_tunnel_set_on_will_reconnect_callback(config->tunnel_ref, on_will_reconnect, config);
    pinggy_tunnel_set_on_reconnecting_callback(config->tunnel_ref, on_reconnecting, config);
    pinggy_tunnel_set_on_reconnection_completed_callback(config->tunnel_ref, on_reconnection_completed, config);
    pinggy_tunnel_set_on_reconnection_failed_callback(config->tunnel_ref, on_reconnection_failed, config);
    pinggy_tunnel_set_on_usage_update_callback(config->tunnel_ref, on_usage_update, config);

    // Register the forwardings changed callback
    pinggy_tunnel_set_on_forwardings_changed_callback(config->tunnel_ref, on_forwardings_changed, config);

    pinggy_tunnel_start_usage_update(config->tunnel_ref);
    pinggy_tunnel_start(config->tunnel_ref);

    if (config->error && strlen(config->error) > 0) {
        printf("Tunnel ended with msg: %s\n", config->error);
    }

    pinggy_free_ref(config->tunnel_ref);
    free_client_config(config);

    return 0;
}
