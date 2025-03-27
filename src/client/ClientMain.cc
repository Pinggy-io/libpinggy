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

bool parseReverseTunnel(sdk::SDKConfigPtr sdkConfig, tString value)
{
    auto values = SplitString(value, ":");
    if (value.length() < 2) {
        return false;
    }
    auto url =  values[values.size() - 2] + ":" + values[values.size() - 1];
    try {
        sdkConfig->TcpForwardTo = NewUrlPtr(url);
    } catch(...) {
        return false;
    }
    return true;
}

bool parseForwardTunnel(sdk::SDKConfigPtr sdkConfig, tString value)
{
    auto values = SplitString(value, ":");
    if (values.size() < 2) {
        return false;
    }

    // try {
    //     sdkConfig->WebDebuggerPort = std::stoi(values[values.size() - 1]);
    //     sdkConfig->EnableWebDebugger = true;
    // } catch(...) {
    //     return false;
    // }
    return true;
}

bool parseUser(sdk::SDKConfigPtr sdkConfig, tString user)
{
    auto values = SplitString(user, ":");
    tString token = "";

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
            sdkConfig->Mode = sl;
        } else {
            token = "+" + sl;
        }
    }

    if (!sdkConfig->TcpForwardTo && !sdkConfig->UdpForwardTo) {
        sdkConfig->TcpForwardTo = forwardingAddress;
        sdkConfig->Mode = ConnMode_HTTP;
    }

    if (token.length() > 1) {
        sdkConfig->Token = token.substr(1);
    }
    return true;
}

bool parseUserServer(sdk::SDKConfigPtr sdkConfig, tString value, tString port)
{
    auto values = SplitString(value, "@");
    if (values.size() > 2) {
        return false;
    }

    bool success = true;
    try {
        sdkConfig->ServerAddress = NewUrlPtr(values[values.size() - 1] + ":" + port);
        if (values.size() > 1) {
            success = parseUser(sdkConfig, values[values.size() - 2]);
        }
    } catch(...) {
        return false;
    }
    return success;
}

sdk::SDKConfigPtr parseArguments(int argc, char *argv[])
{
    auto config = sdk::NewSDKConfigPtr();
    config->AdvancedParsing = false;


    int opt;
    int longindex = 0;

    struct cli_option longopts[] = {
        {"help", cli_no_argument, 0, 'h'},
        {"version", cli_no_argument, 0, 'v'},
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
    while ((opt = cli_getopt_long(argc, argv, "hvno:R:L:p:s:", longopts, &longindex)) != -1) {
        bool success = true;
        switch (opt) {
            case 'h':
                printf("Help option\n");
                exitNow = true;
                break;
            case 'v':
                printf("Version option\n");
                exitNow = true;
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
            // case 'L':
            //     success = parseForwardTunnel(config, cli_optarg);
                // break;
            case 'n':
                config->Ssl = false;
                break;
            case 's':
                config->SniServerName = cli_optarg;
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

    if (exitNow)
        exit(0);

    return config;
}

class ClientSdkEventHandler: virtual public sdk::SdkEventHandler
{
public:
    virtual ~ClientSdkEventHandler(){}
    virtual void PrimaryForwardingSucceeded(std::vector<std::string> urls);
};
DefineMakeSharedPtr(ClientSdkEventHandler);

int main(int argc, char *argv[]) {
    WindowsSocketInitialize();
    InitLogWithCout();
    auto config = parseArguments(argc, argv);
    auto sdk = sdk::NewSdkPtr(config, NewClientSdkEventHandlerPtr());
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

void ClientSdkEventHandler::PrimaryForwardingSucceeded(std::vector<std::string> urls)
{
    std::cout << "Connection completed" << std::endl;
    for (auto url : urls) {
        std::cout << "   " << url << std::endl;
    }
}
