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

#include <Sdk.hh>
#include <platform/Log.hh>
#include <utils/Utils.hh>

#include "cli_getopt.h"

#ifndef PLATFORM_CONFIG_INCLUDED

#define PinggyVersionMajor 0
#define PinggyVersionMinor 0
#define PinggyVersionPatch 0

#define PINGGY_GIT_COMMIT_ID "unknown"
#define PINGGY_BUILD_TIMESTAMP "0000-00-00 00:00:00"
#define PINGGY_LIBC_VERSION "unknown"
#define PINGGY_BUILD_OS "unknown"

#endif


static const std::string ConnMode_HTTP      = "http";
static const std::string ConnMode_TCP       = "tcp";
static const std::string ConnMode_TLS       = "tls";
static const std::string ConnMode_TLSTCP    = "tlstcp";
static const std::string ConnModeExt_UDP    = "udp";

struct ClientConfig: virtual public pinggy::SharedObject
{
    ClientConfig() :
            sdkConfig(sdk::NewSDKConfigPtr()),
            WebDebuggerPort(4300),
            EnableWebDebugger(false)
                                {}

    virtual ~ClientConfig()     {}

    sdk::SDKConfigPtr           sdkConfig;
    port_t                      WebDebuggerPort;
    bool                        EnableWebDebugger;
};
DefineMakeSharedPtr(ClientConfig);

static std::vector<tString>
parseForwarding(const tString& val) {
    std::vector<tString> result;
    tString value = val;

    while (!value.empty()) {
        if (value[0] == '[') {
            // IPv6 address in brackets
            size_t close = value.find(']');
            if (close == tString::npos) {
                result.push_back(value);
                break;
            }
            // Include the closing bracket
            tString ipv6 = value.substr(0, close + 1);
            result.push_back(ipv6);
            // Move past the closing bracket
            value = value.substr(close + 1);
            if (!value.empty() && value[0] == ':') {
                value = value.substr(1);
            } else {
                break;
            }
        } else {
            // Find next colon
            size_t colon = value.find(':');
            if (colon == tString::npos) {
                result.push_back(value);
                break;
            }
            result.push_back(value.substr(0, colon));
            value = value.substr(colon + 1);
        }
    }

    return result;
}

bool
parseReverseTunnel(ClientConfigPtr config, tString value)
{
    auto values = parseForwarding(value);
    if (value.length() < 2) {
        return false;
    }
    auto url =  values[values.size() - 2] + ":" + values[values.size() - 1];
    try {
        config->sdkConfig->TcpForwardTo = NewUrlPtr(url);
    } catch(...) {
        return false;
    }
    return true;
}

bool
parseForwardTunnel(ClientConfigPtr config, tString value)
{
    auto values = parseForwarding(value);
    if (values.size() < 2) {
        return false;
    }

    try {
        config->WebDebuggerPort = std::stoi(values[values.size() - 1]);
        config->EnableWebDebugger = true;
    } catch(...) {
        return false;
    }
    return true;
}

bool
parseUser(ClientConfigPtr config, tString user)
{
    auto values = SplitString(user, "+");
    tString token = "";

    auto sdkConfig = config->sdkConfig;

    auto forwardingAddress = sdkConfig->TcpForwardTo;
    sdkConfig->TcpForwardTo  = nullptr;

    for(auto s : values) {
        auto sl = StringToLower(s);
        LOGT("s=" << s, "sl="<<sl);
        if (sl == ConnMode_HTTP || sl == ConnMode_TCP || sl == ConnMode_TLS || sl == ConnMode_TLSTCP) {
            sdkConfig->TcpForwardTo = forwardingAddress;
            sdkConfig->Mode = sl;
        } else if (sl == ConnModeExt_UDP) {
            sdkConfig->UdpForwardTo = forwardingAddress;
            sdkConfig->UdpMode = sl;
        } else {
            token = "+" + s;
        }
    }

    if (!sdkConfig->TcpForwardTo && !sdkConfig->UdpForwardTo) {
        sdkConfig->TcpForwardTo = forwardingAddress;
        sdkConfig->Mode = ConnMode_HTTP;
    }

    if (token.length() > 1) {
        sdkConfig->Token = token.substr(1);
    }
    LOGI("token: ", sdkConfig->Token);
    return true;
}

bool
parseUserServer(ClientConfigPtr config, tString value, tString port)
{
    auto values = SplitString(value, "@");
    if (values.size() > 2) {
        return false;
    }

    bool success = true;
    try {
        config->sdkConfig->ServerAddress = NewUrlPtr(values[values.size() - 1] + ":" + port);
        if (values.size() > 1) {
            success = parseUser(config, values[values.size() - 2]);
        }
    } catch(...) {
        return false;
    }
    return success;
}

void
printHelpOptions(const char *prog){
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
parseSdkArguments(ClientConfigPtr config, int argc, char *argv[])
{
    std::vector<tString> args;
    for (int i = 0; i < argc; i++) {
        args.push_back(tString(argv[i]));
    }
    config->sdkConfig->Argument = ShlexJoinStrings(args);

    return true;
}

ClientConfigPtr
parseArguments(int argc, char *argv[])
{
    auto config = NewClientConfigPtr();
    // config->sdkConfig->AdvancedParsing = false;


    int opt;
    int longindex = 0;

    struct cli_option longopts[] = {
        {"help", cli_no_argument, 0, 'h'},
        {"version", cli_no_argument, 0, 'v'},
        {"verbose", cli_no_argument, 0, 'V'},
        {"port", cli_required_argument, 0, 'p'},
        {"sni", cli_required_argument, 0, 's'},
        {"inseceure", cli_required_argument, 0, 'n'},
        {NULL, cli_required_argument, 0, 'R'},
        {NULL, cli_required_argument, 0, 'L'},
        {NULL, cli_required_argument, 0, 'o'},
        {NULL, cli_required_argument, 0, 'n'},
        {0, 0, 0, 0} // Terminator
    };
    bool exitNow = false;
    tString serverPort = "443";
    const char *prog = argv[0];
    while ((opt = cli_getopt_long(argc, argv, "ahvVno:R:L:p:s:r", longopts, &longindex)) != -1) {
        bool success = true;
        switch (opt) {
            case 'a':
                config->sdkConfig->AdvancedParsing = true;
                break;
            case 'h':
                // printf("Help option\n");
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
                SetGlobalLogEnable(true);
                break;
            case 'o':
                printf("Output option with value: %s\n", cli_optarg);
                break;
            case 'p':
                serverPort = cli_optarg;
                break;
            case 'R':
                success = parseReverseTunnel(config, cli_optarg);
                break;
            case 'L':
                success = parseForwardTunnel(config, cli_optarg);
                break;
            case 'n':
                config->sdkConfig->Ssl = false;
                break;
            case 's':
                config->sdkConfig->SniServerName = cli_optarg;
                break;
            case 'r':
                config->sdkConfig->AutoReconnect = true;
                break;
            case 256: // Handling for --config
                printf("Config option with value: %s\n", cli_optarg);
                break;
            case 257: // Handling for --debug
                printf("Debug option\n");
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

    if ((cli_optind + 1) < argc) {
        exitNow = !(parseSdkArguments(config, argc - (cli_optind + 1), argv + (cli_optind + 1)));
    }

    if (exitNow)
        exit(0);

    return config;
}

struct ClientSdkEventHandler: virtual public sdk::SdkEventHandler
{
    ClientSdkEventHandler(ClientConfigPtr config):
        config(config)          { }

    virtual
    ~ClientSdkEventHandler()    { }

    virtual void
    OnPrimaryForwardingSucceeded(std::vector<std::string> urls) override;

    virtual void
    OnAuthenticationFailed(std::vector<tString> why) override
                                { this->error = JoinString(why, " "); }

    virtual void
    OnPrimaryForwardingFailed(tString error) override
                                { this->error = error; }

    virtual void
    OnDisconnected(tString error, std::vector<tString> messages) override
                                { this->error = error; }

    virtual void
    OnAutoReconnection(tString error, std::vector<tString> messages) override
    {
        std::cout << "Reconnecting" << std::endl;
    }

    virtual void
    OnReconnecting(tUint16 count) override
    {
        std::cout << "Trying.. " << count << std::endl;
    }

    virtual void
    OnReconnectionCompleted() override
    {
        std::cout << "Reconnected" << std::endl;
    }

    virtual void
    OnReconnectionFailed(tUint16 tries) override
    {
        std::cout << "Reconnection failed after " << tries << " tries" << std::endl;
    }

    tString                     error;
    ClientConfigPtr             config;
    sdk::SdkWPtr                sdk;
};
DefineMakeSharedPtr(ClientSdkEventHandler);

int
main(int argc, char *argv[]) {
    WindowsSocketInitialize();
    InitLogWithCout();
    SetGlobalLogEnable(false);
    auto config = parseArguments(argc, argv);
    auto sdkEventHandler = NewClientSdkEventHandlerPtr(config);
    auto sdk = sdk::NewSdkPtr(config->sdkConfig, sdkEventHandler);
    sdkEventHandler->sdk = sdk;

    sdk->Start();

    if (sdkEventHandler->error != "")
        std::cout << "Tunnel ended with msg: " << sdkEventHandler->error << std::endl;

    sdk = nullptr;
    config = nullptr;
    sdkEventHandler = nullptr;

    return 0;
}

void
ClientSdkEventHandler::OnPrimaryForwardingSucceeded(std::vector<std::string> urls)
{
    std::cout << "Connection completed" << std::endl;
    for (auto url : urls) {
        std::cout << "   " << url << std::endl;
    }
    if (config->EnableWebDebugger && config->WebDebuggerPort > 0) {
        thisPtr->sdk.lock()->StartWebDebugging(config->WebDebuggerPort);
    }
}
