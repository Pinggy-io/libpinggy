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
static const char *ConnMode_HTTP      = "http";
static const char *ConnMode_TCP       = "tcp";
static const char *ConnMode_TLS       = "tls";
static const char *ConnMode_TLSTCP    = "tlstcp";
static const char *ConnMode_UDP       = "udp";

// --- Helper: String Vector (to replace std::vector<tString>) ---
typedef struct 
{
    char **items;
    size_t size;
} StringVector;

void 
Vector_Init(StringVector *v) 
{ 
    v->items = NULL; v->size = 0; 
}

void 
Vector_Push(StringVector *v, const char *str) 
{
    v->items = (char**)realloc(v->items, sizeof(char*) * (v->size + 1));
    v->items[v->size++] = strdup(str);
}

void 
Vector_Free(StringVector *v) 
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
StringVector 
SplitString(const char *str, const char *delim) 
{
    StringVector v; Vector_Init(&v);
    char *dup = strdup(str);
    char *token = strtok(dup, delim);
    while(token) { 
        Vector_Push(&v, token);
        token = strtok(NULL, delim);
    }
    free(dup);
    return v;
}

// --- Structure for client configuration ---
typedef struct 
{
    StringVector        forwardings; // for storing the list of reverse tunnels
    pinggy_ref_t        SdkConfig;
    char               *WebDebuggerAddr;
    bool                EnableWebDebugger;
    char                *Mode;
    
    // Added for C context
    pinggy_ref_t        tunnel_ref; 
    char *error; 
} ClientConfig;

ClientConfig 
*NewClientConfigPtr()
{
    ClientConfig *cfg = (ClientConfig*)calloc(1, sizeof(ClientConfig));
    Vector_Init(&cfg->forwardings);
    cfg->SdkConfig = pinggy_create_config();
    cfg->WebDebuggerAddr = strdup("localhost:4300");
    cfg->EnableWebDebugger = false;
    cfg->Mode = strdup("");
    return cfg;
}

void 
FreeClientConfig(ClientConfig *cfg) 
{
    if(!cfg) return;
    Vector_Free(&cfg->forwardings);
    pinggy_free_ref(cfg->SdkConfig);
    if(cfg->WebDebuggerAddr) free(cfg->WebDebuggerAddr);
    if(cfg->Mode) free(cfg->Mode);
    if(cfg->error) free(cfg->error);
    free(cfg);
}

// --- Logic Implementation ---

// function to parse forwarding address
// -R [[[schema//]remotehost[/port]:]0:]localhost:localport

StringVector 
parseForwarding(const char *val)
{
    StringVector result;
    Vector_Init(&result);
    
    if (!val) return result;
    char *value = strdup(val);
    char *p = value;
    
    while (*p != '\0') {
        if (*p == '[') {
            // IPv6 address in brackets
            char *close = strchr(p, ']');
            if (!close) {
                Vector_Push(&result, p);
                break;
            }
            // Include closing bracket
            size_t len = close - p + 1;
            char *ipv6 = (char*)malloc(len + 1);
            strncpy(ipv6, p, len);
            ipv6[len] = '\0';
            Vector_Push(&result, ipv6);
            free(ipv6);
            
            p = close + 1;
            if (*p == ':') p++;
            else break;
        } else {
            // Find next colon
            char *colon = strchr(p, ':');
            if (!colon) {
                Vector_Push(&result, p);
                break;
            }
            *colon = '\0';
            Vector_Push(&result, p);
            p = colon + 1;
        }
    }
    free(value);
    return result;
}

// Helper struct for Tuple return
typedef struct {
    char *schema;
    char *hostname;
    int port;
} HostnameTuple;

HostnameTuple 
parseHostname(const char *host)
{
    HostnameTuple t = {NULL, NULL, 0};
    if (!host) return t;

    char *copy = strdup(host);
    char *curr_hostname = copy;

    char *schemaSep = strstr(curr_hostname, "//");
    if (schemaSep) {
        *schemaSep = '\0';
        t.schema = strdup(curr_hostname);
        curr_hostname = schemaSep + 2;
    } else {
        t.schema = strdup("");
    }

    char *lastSlash = strrchr(curr_hostname, '/');
    if (lastSlash) {
        *lastSlash = '\0';
        t.port = atoi(lastSlash + 1);
    }

    t.hostname = strdup(curr_hostname);

    // Schema validation logic from C++
    if (strcmp(t.schema, ConnMode_HTTP) != 0 && strcmp(t.schema, ConnMode_TCP) != 0 && 
        strcmp(t.schema, ConnMode_TLS) != 0 && strcmp(t.schema, ConnMode_TLSTCP) != 0 && 
        strcmp(t.schema, ConnMode_UDP) != 0) {
        free(t.schema);
        t.schema = strdup("");
    }
    
    free(copy);
    return t;
}

// called when the user provides -R
bool 
parseReverseTunnel(ClientConfig *config)
{
    for (size_t i = 0; i < config->forwardings.size; i++) {
        char *value = config->forwardings.items[i];
        StringVector values = parseForwarding(value);
        
        if (values.size < 2) {
            Vector_Free(&values);
            return false;
        }

        // forwardTo = values[size-2] + ":" + values[size-1]
        char forwardTo[512];
        snprintf(forwardTo, sizeof(forwardTo), "%s:%s", 
                 values.items[values.size - 2], values.items[values.size - 1]);

        // Mimic try/catch block logic
        if (values.size < 4) {
            // AddForwarding(mode, "", forwardTo)
            pinggy_config_add_forwarding(config->SdkConfig, config->Mode, "", forwardTo);
        } else {
            char *remoteHost = values.items[values.size - 4];
            HostnameTuple ht = parseHostname(remoteHost);
            
            if (!ht.hostname || strlen(ht.hostname) == 0) {
                if (ht.hostname) free(ht.hostname);
                ht.hostname = strdup("localhost");
            }
            
            char hostname_full[512];
            snprintf(hostname_full, sizeof(hostname_full), "%s:%d", ht.hostname, ht.port);
            
            if (!ht.schema || strlen(ht.schema) == 0) {
                pinggy_config_add_forwarding(config->SdkConfig, config->Mode, hostname_full, forwardTo);
            } else {
                pinggy_config_add_forwarding(config->SdkConfig, ht.schema, hostname_full, forwardTo);
            }
            
            if (ht.schema) free(ht.schema);
            if (ht.hostname) free(ht.hostname);
        }
        
        Vector_Free(&values);
    }
    return true;
}

// called when the user provides -L
bool 
parseForwardTunnel(ClientConfig *config, const char *value)
{
    StringVector values = parseForwarding(value);
    if (values.size < 2) {
        Vector_Free(&values);
        return false;
    }

    char newAddr[512];
    snprintf(newAddr, sizeof(newAddr), "%s:%s", 
             values.items[values.size - 2], values.items[values.size - 1]);
    
    if(config->WebDebuggerAddr) free(config->WebDebuggerAddr);
    config->WebDebuggerAddr = strdup(newAddr);
    config->EnableWebDebugger = true;
    
    Vector_Free(&values);
    return true;
}

// parseUser: tcp+token -> Mode=tcp, Token=token
bool 
parseUser(ClientConfig *config, const char *user)
{
    StringVector values = SplitString(user, "+");
    char token[1024] = "";

    for(size_t i=0; i < values.size; i++) {
        char *s = values.items[i];
        // StringToLower logic (inline)
        char sl[256];
        strncpy(sl, s, 255); sl[255] = '\0';
        for(char *p=sl; *p; ++p) *p = tolower((unsigned char)*p);
        
        if (strcmp(sl, ConnMode_HTTP) == 0 || strcmp(sl, ConnMode_TCP) == 0 || 
            strcmp(sl, ConnMode_TLS) == 0 || strcmp(sl, ConnMode_TLSTCP) == 0 || 
            strcmp(sl, ConnMode_UDP) == 0) {
            
            if (strlen(config->Mode) > 0) {
                printf("You cannot provide more than one type of default forwarding type");
                Vector_Free(&values);
                return false;
            }
            if(config->Mode) free(config->Mode);
            config->Mode = strdup(sl);
        } else {
            strcat(token, "+");
            strcat(token, s);
        }
    }

    if (strlen(token) > 1) {
        // Remove leading '+' and set token
        pinggy_config_set_token(config->SdkConfig, token + 1);
        printf("token: %s\n", token + 1); 
    }
    
    Vector_Free(&values);
    return true;
}

// parseUserServer: token@server
bool 
parseUserServer(ClientConfig *config, const char *value, const char *port)
{
    StringVector values = SplitString(value, "@");
    if (values.size > 2) {
        Vector_Free(&values);
        return false;
    }

    bool success = true;
    // SetServerAddress logic
    char serverAddr[512];
    const char *hostPart = values.items[values.size - 1];
    snprintf(serverAddr, sizeof(serverAddr), "%s:%s", hostPart, port);
    pinggy_config_set_server_address(config->SdkConfig, serverAddr);

    if (values.size > 1) {
        success = parseUser(config, values.items[values.size - 2]);
    }
    
    Vector_Free(&values);
    return success;
}

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

bool 
parseSdkArguments(ClientConfig *config, int argc, char *argv[])
{
    size_t len = 0;
    for(int i=0; i<argc; i++) len += strlen(argv[i]) + 1;
    
    char *args = (char*)malloc(len + 1);
    args[0] = '\0';
    for(int i=0; i<argc; i++) {
        strcat(args, argv[i]);
        if(i < argc-1) strcat(args, " ");
    }
    
    pinggy_config_set_argument(config->SdkConfig, args);
    free(args);
    return true;
}

ClientConfig* 
parseArguments(int argc, char* argv[])
{
    ClientConfig *config = NewClientConfigPtr();

    int opt;
    int longindex = 0;
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
    
    bool exitNow = false;
    char *serverPort = "443";
    const char *prog = argv[0];

    while ((opt = cli_getopt_long(argc, argv, "ahvVno:R:L:p:s:r", longopts, &longindex)) != -1) {
        bool success = true;
        switch (opt) {
        case 'h':
            printHelpOptions(prog);
            exitNow = true;
            exit(0);
            break;
        case 'v':
            printf("v%d.%d.%d\n", PinggyVersionMajor, PinggyVersionMinor, PinggyVersionPatch);
            exitNow = true;
            exit(0);
            break;
        case 'V':
            pinggy_set_log_enable(true);
            break;
        case 'o':
            printf("Output option with value: %s\n", cli_optarg);
            break;
        case 'p':
            serverPort = cli_optarg;
            break;
        case 'R':
            Vector_Push(&config->forwardings, cli_optarg);
            break;
        case 'L':
            success = parseForwardTunnel(config, cli_optarg);
            break;
        case 'n':
            pinggy_config_set_ssl(config->SdkConfig, false);
            pinggy_config_set_insecure(config->SdkConfig, true); 
            break;
        case 's':
            pinggy_config_set_sni_server_name(config->SdkConfig, cli_optarg);
            break;
        case 'r':
            pinggy_config_set_auto_reconnect(config->SdkConfig, true);
            break;
        case 'a':
            pinggy_config_set_advanced_parsing(config->SdkConfig, true);
            break;
        case '?':
            printf("Unknown option or missing argument\n");
            break;
        }
        if (!success) {
            exitNow = true;
            break;
        }
    }

    if (cli_optind < argc) {
        exitNow = !(parseUserServer(config, argv[cli_optind], serverPort));
    } else {
        exitNow = true;
    }

    if (!exitNow)
        exitNow = !parseReverseTunnel(config);

    if (!exitNow && (cli_optind + 1) < argc) {
        exitNow = !(parseSdkArguments(config, argc - (cli_optind + 1), argv + (cli_optind + 1)));
    }

    if (exitNow) {
        FreeClientConfig(config);
        exit(0);
    }

    return config;
}

// --- Callbacks ---

static void 
OnTunnelEstablished(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_len_t num_urls, pinggy_char_p_p_t urls)
{
    ClientConfig *config = (ClientConfig*)user_data;
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

    if (config->EnableWebDebugger && config->WebDebuggerAddr && strlen(config->WebDebuggerAddr) > 0) {
        pinggy_config_set_webdebugger_addr(config->SdkConfig, config->WebDebuggerAddr);
        pinggy_tunnel_start_web_debugging(tunnel_ref, config->WebDebuggerAddr);
    }
}

static void 
OnTunnelFailed(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_const_char_p_t msg)
{
    ClientConfig *config = (ClientConfig*)user_data;
    if(msg) config->error = strdup(msg);
}

static void 
OnDisconnected(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_const_char_p_t error, pinggy_len_t msg_size, pinggy_char_p_p_t msgs)
{
    ClientConfig *config = (ClientConfig*)user_data;
    if(error) config->error = strdup(error);
    if(msgs && msg_size > 0) printf("Disconnected msg: %s\n", msgs[0]);
}

static void 
OnWillReconnect(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_const_char_p_t error, pinggy_len_t num_msgs, pinggy_char_p_p_t messages)
{
    printf("Reconnecting\n");
}

static void 
OnReconnecting(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_uint16_t count)
{
    printf("Trying.. %u\n", count);
}

static void 
OnReconnectionCompleted(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_len_t num_urls, pinggy_char_p_p_t urls)
{
    printf("Reconnected\n");
    for (int i = 0; i < (int)num_urls; i++) {
        printf("   %s\n", urls[i]);
    }
}

static void 
OnReconnectionFailed(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_uint16_t tries)
{
    printf("Reconnection failed after %u tries\n", tries);
}

static void 
OnUsageUpdate(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_const_char_p_t msg)
{
    printf("Update msg: %s\n", msg);
}

// **THIS IS THE FUNCTION YOU WERE MISSING**
static void 
OnForwardingsChanged(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_const_char_p_t url_map)
{
    printf("%s\n", url_map);
}

static pinggy_void_t 
pinggy_on_raise_exception_cb(pinggy_const_char_p_t where, pinggy_const_char_p_t what)
{
    printf("Exception: %s ==> %s\n", where, what);
}

// --- Main ---
int main(int argc, char* argv[])
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

    ClientConfig *config = parseArguments(argc, argv);
    if (!config) {
        return 0;
    }

    config->tunnel_ref = pinggy_tunnel_initiate(config->SdkConfig);
    if (config->tunnel_ref == INVALID_PINGGY_REF) {
        printf("Failed to initiate tunnel\n");
        FreeClientConfig(config);
        return 0;
    }

    pinggy_tunnel_set_on_tunnel_established_callback(config->tunnel_ref, OnTunnelEstablished, config);
    pinggy_tunnel_set_on_tunnel_failed_callback(config->tunnel_ref, OnTunnelFailed, config);
    pinggy_tunnel_set_on_disconnected_callback(config->tunnel_ref, OnDisconnected, config);
    pinggy_tunnel_set_on_will_reconnect_callback(config->tunnel_ref, OnWillReconnect, config);
    pinggy_tunnel_set_on_reconnecting_callback(config->tunnel_ref, OnReconnecting, config);
    pinggy_tunnel_set_on_reconnection_completed_callback(config->tunnel_ref, OnReconnectionCompleted, config);
    pinggy_tunnel_set_on_reconnection_failed_callback(config->tunnel_ref, OnReconnectionFailed, config);
    pinggy_tunnel_set_on_usage_update_callback(config->tunnel_ref, OnUsageUpdate, config);
    
    // Register the forwardings changed callback
    pinggy_tunnel_set_on_forwardings_changed_callback(config->tunnel_ref, OnForwardingsChanged, config);

    pinggy_tunnel_start_usage_update(config->tunnel_ref);
    pinggy_tunnel_start(config->tunnel_ref);

    if (config->error && strlen(config->error) > 0) {
        printf("Tunnel ended with msg: %s\n", config->error);
    }

    pinggy_free_ref(config->tunnel_ref);
    FreeClientConfig(config);

    return 0;
}