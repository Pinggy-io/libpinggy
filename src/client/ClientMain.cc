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
#include <poll/PinggyPollGeneric.hh>
#ifndef __WINDOWS_OS__
#include <poll/PinggyPollLinux.hh>
// #include <getopt.h>
#else
#endif

#include "cli_getopt.h"


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

bool
parseReverseTunnel(ClientConfigPtr config, tString value)
{
    auto values = SplitString(value, ":");
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
    auto values = SplitString(value, ":");
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
        {"inseceure", cli_required_argument, 0, 'i'},
        {NULL, cli_required_argument, 0, 'R'},
        {NULL, cli_required_argument, 0, 'L'},
        {NULL, cli_required_argument, 0, 'o'},
        {NULL, cli_required_argument, 0, 'n'},
        {0, 0, 0, 0} // Terminator
    };
    bool exitNow = false;
    tString serverPort = "443";
    const char *prog = argv[0];
    while ((opt = cli_getopt_long(argc, argv, "ahvVno:R:L:p:s:", longopts, &longindex)) != -1) {
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
    OnPrimaryForwardingSucceeded(std::vector<std::string> urls);

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
#ifndef __WINDOWS_OS__
    auto pollController = common::NewPollControllerLinuxPtr();
#else
    auto pollController = common::NewPollControllerGenericPtr();
#endif
    sdk->Start(pollController);
    pollController->StartPolling();

    LOGI("Tunnel ended with msg: ", sdk->GetEndMessage());

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
