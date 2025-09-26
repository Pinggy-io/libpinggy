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

 // for setting up the connection modes
 //ex: tcp+token@server so the mode is tcp
static const std::string ConnMode_HTTP      = "http";
static const std::string ConnMode_TCP       = "tcp";
static const std::string ConnMode_TLS       = "tls";
static const std::string ConnMode_TLSTCP    = "tlstcp";
static const std::string ConnMode_UDP       = "udp";

// structure for client configuration
struct ClientConfig : virtual public pinggy::SharedObject
{
    ClientConfig() : // constructor
            SdkConfig(sdk::NewSDKConfigPtr()),
            WebDebuggerPort(4300),
            EnableWebDebugger(false) // turn on/off web debugger
                                {}

    virtual ~ClientConfig()     {}

    std::vector<tString>        forwardings; // for storing the list of reverse tunnels
    sdk::SDKConfigPtr           SdkConfig;
    port_t                      WebDebuggerPort;
    bool                        EnableWebDebugger;
    tString                     Mode;
};

DefineMakeSharedPtr(ClientConfig);  // macro for the clientconfig

static std::vector<tString>

// function to parse forwarding address (forwardingg adreess to components)
// also handls the IPv6 address
parseForwarding(const tString& val)
{
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

std::tuple<tString, tString, tPort>
parseHostname(tString host)
{
    tString schema, hostname;
    tPort port = 0;
    // Expected format: [schema//]hostname[/port]
    // Example: "ssh//example.com/2222", "example.com/2222", "ssh//example.com", "example.com"
    // Parse schema if present

    hostname = host;

    auto schemaSep = hostname.find("//");
    if (schemaSep != tString::npos) {
        schema = hostname.substr(0, schemaSep);
        hostname = hostname.substr(schemaSep + 2);
    }

    // Find last '/' (port separator)
    auto lastSlash = hostname.rfind('/');
    if (lastSlash != tString::npos) {
        auto portStr = hostname.substr(lastSlash + 1);
        port = std::stoi(portStr); //Don't care about exception. it would collasped
        hostname = hostname.substr(0, lastSlash);
    }

    if (schema != ConnMode_HTTP && schema != ConnMode_TCP && schema != ConnMode_TLS && schema != ConnMode_TLSTCP && schema != ConnMode_UDP) {
        schema = "";
    }

    return {schema, hostname, port};
}

// called when the useer provides -R
bool
parseReverseTunnel(ClientConfigPtr config) // if the values is 2 then eastablish single tunnel else multiple tunnels
{
    for (auto value : config->forwardings) {
        auto values = parseForwarding(value);
        if (values.size() < 2) { // fixed logic here
            return false;
        }
        auto forwardTo =  values[values.size() - 2] + ":" + values[values.size() - 1];
        try {
            if (values.size() < 4) {
                config->SdkConfig->AddForwarding(config->Mode, "", forwardTo);
            } else {
                auto remoteHost = values[values.size() - 4];
                auto [schema, hostname, port] = parseHostname(remoteHost);
                if (hostname.empty())
                    hostname = "localhost";
                hostname += ":" + std::to_string(port);
                if (schema.empty()) {
                    config->SdkConfig->AddForwarding(config->Mode, hostname, forwardTo);
                } else {
                    config->SdkConfig->AddForwarding(schema, hostname, forwardTo);
                }
            }
        } catch(std::exception &e) {
            LOGF("Exception occurred:", e.what());
            return false;
        }
    }
    return true;
}

// extarct the last part (i.e the port number)
// called when the user provides -L
// sets up the web debugger port
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
    } catch (...) {
        return false;
    }
    return true;
}


// for storing the user tokens and the connection mode
//ex : tcp@token to connection == TCP and token to token and set this inside the sdkconfig
// what type of tunnel i will be using and the type of authentication
bool
parseUser(ClientConfigPtr config, tString user)
{
    auto values = SplitString(user, "+");
    tString token = "";

    auto sdkConfig = config->SdkConfig;

    for(auto s : values) {
        auto sl = StringToLower(s);
        LOGT("s=" << s, "sl=" << sl);
        if (sl == ConnMode_HTTP || sl == ConnMode_TCP || sl == ConnMode_TLS || sl == ConnMode_TLSTCP || sl == ConnMode_UDP) {
            if (!config->Mode.empty()){
                std::cout << "You cannot provide more than one type of default forwarding type";
                return false;
            }

            config->Mode = sl;
        } else {
            token = "+" + s;
        }
    }

    if (token.length() > 1) {
        sdkConfig->SetToken(token.substr(1));
    }
    LOGI("token: ", sdkConfig->GetToken());
    return true;
}


//splits the token@server so that
// after this the connection info is complete
bool
parseUserServer(ClientConfigPtr config, tString value, tString port)
{
    auto values = SplitString(value, "@");
    if (values.size() > 2) {
        return false;
    }

    bool success = true;
    try {
        config->SdkConfig->SetServerAddress(NewUrlPtr(values[values.size() - 1] + ":" + port));
        if (values.size() > 1) {
            success = parseUser(config, values[values.size() - 2]);
        }
    } catch (...) {
        return false;
    }
    return success;
}

void
printHelpOptions(const char* prog)
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
// Stores all command line arguments in the SDK config for internal tracking
bool
parseSdkArguments(ClientConfigPtr config, int argc, char *argv[])
{
    std::vector<tString> args;
    for (int i = 0; i < argc; i++) {
        args.push_back(tString(argv[i]));
    }
    config->SdkConfig->SetArguments(ShlexJoinStrings(args));

    return true;
}

ClientConfigPtr
parseArguments(int argc, char* argv[])
{
    auto config = NewClientConfigPtr();


    int opt;
    int longindex = 0;

    struct cli_option longopts[] = {
        {"help", cli_no_argument, 0, 'h'},
        {"version", cli_no_argument, 0, 'v'},
        {"verbose", cli_no_argument, 0, 'V'},
        {"port", cli_required_argument, 0, 'p'},
        {"sni", cli_required_argument, 0, 's'},
        {"insecure", cli_required_argument, 0, 'n'}, // fixed typo here
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
            SetGlobalLogEnable(true);
            break;
        case 'o':
            printf("Output option with value: %s\n", cli_optarg);
            break;
        case 'p':
            serverPort = cli_optarg;
            break;
        case 'R':
            config->forwardings.push_back(cli_optarg);
            break;
        case 'L':
            success = parseForwardTunnel(config, cli_optarg);
            break;
        case 'n':
            config->SdkConfig->SetSsl(false);
            config->SdkConfig->SetInsecure(true);
            break;
        case 's':
            config->SdkConfig->SetSniServerName(cli_optarg);
            break;
        case 'r':
            config->SdkConfig->SetAutoReconnect(true);
            break;
        case 'a':
            config->SdkConfig->SetAdvancedParsing(true);
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

    if (!exitNow)
        exitNow = !parseReverseTunnel(config);

    if (exitNow && (cli_optind + 1) < argc) {
        exitNow = !(parseSdkArguments(config, argc - (cli_optind + 1), argv + (cli_optind + 1)));
    }

    if (exitNow)
        exit(0);

    return config;
}
// the event handler part
// this is where the real events are handled like the connection succeds, fails or etc
struct ClientSdkEventHandler : virtual public sdk::SdkEventHandler
{
    ClientSdkEventHandler(ClientConfigPtr config) :
        config(config)
    {}

    virtual
    ~ClientSdkEventHandler()    { }

    virtual void
    OnPrimaryForwardingSucceeded(std::vector<std::string> urls) override;

    virtual void
    OnAuthenticationFailed(std::vector<tString> why) override
    {
        this->error = JoinString(why, " ");
    }

    virtual void
    OnPrimaryForwardingFailed(tString error) override
    {
        this->error = error;
    }

    virtual void
    OnDisconnected(tString error, std::vector<tString> messages) override
    {
        this->error = error; LOGD("Disconnected:", messages);
    }

    virtual void
    OnWillReconnect(tString error, std::vector<tString> messages) override
    {
        std::cout << "Reconnecting" << std::endl;
    }

    virtual void
    OnForwardingChanged(tString urlMap) override
    {
        std::cout << urlMap << std::endl;
    }

    virtual void
    OnReconnecting(tUint16 count) override
    {
        std::cout << "Trying.. " << count << std::endl;
    }

    virtual void
    OnReconnectionCompleted(std::vector<tString> urls) override
    {
        std::cout << "Reconnected" << std::endl;
        auto s = sdk.lock();
        if (s) {
            auto urls = s->GetUrls();
            for (auto url : urls) {
                std::cout << "   " << url << std::endl;
            }
        }
        std::cout << "Greeting: " << sdk.lock()->GetGreetingMsg() << std::endl;
    }

    virtual void
    OnReconnectionFailed(tUint16 tries) override
    {
        std::cout << "Reconnection failed after " << tries << " tries" << std::endl;
    }

    virtual void
    OnUsageUpdate(tString msg) override
    {
        std::cout << "Update msg: " << msg << std::endl;
    }

    tString                     error;
    ClientConfigPtr             config;
    sdk::SdkWPtr                sdk;
};
DefineMakeSharedPtr(ClientSdkEventHandler);

int
main(int argc, char* argv[])
{
    WindowsSocketInitialize();
    InitLogWithCout();
    // SetGlobalLogEnable(false);
    ignore_sigpipe();
    try {
        auto config = parseArguments(argc, argv);
        auto sdkEventHandler = NewClientSdkEventHandlerPtr(config);
        auto sdk = sdk::NewSdkPtr(config->SdkConfig, sdkEventHandler);
        sdkEventHandler->sdk = sdk;

        sdk->StartUsagesUpdate();

        sdk->Connect(true);
        sdk->RequestPrimaryRemoteForwarding(true);
        // for (auto x : config->forwardings) {
        //     sdk->RequestAdditionalRemoteForwarding(x.first, x.second);
        // }

        sdk->Start();

        if (sdkEventHandler->error != "")
            std::cout << "Tunnel ended with msg: " << sdkEventHandler->error << std::endl;

        sdk = nullptr;
        config = nullptr;
        sdkEventHandler = nullptr;
    } catch (std::exception &e) {
        LOGF("Exception occured: ", e.what());
    } catch(...) {
        LOGF("Unknown exception occured");
    }

    return 0;
}

void
ClientSdkEventHandler::OnPrimaryForwardingSucceeded(std::vector<std::string> urls)
{
    std::cout << "Connection completed" << std::endl;
    for (auto url : urls) {
        std::cout << "   " << url << std::endl;
    }

    std::cout << "Greeting: " << sdk.lock()->GetGreetingMsg() << std::endl;
    if (config->EnableWebDebugger && config->WebDebuggerPort > 0) {
        thisPtr->sdk.lock()->StartWebDebugging(config->WebDebuggerPort);
    }
}



/*
CLI arguments → parseArguments()
                ↓
          ClientConfig (modes, tokens, addresses, tunnels)
                ↓
        Create SDK object (C++ Pinggy SDK)
                ↓
           Connect to server
                ↓
        Request main + extra tunnels
                ↓
        Register event callbacks
                ↓
          Start main event loop


*/
