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

#ifndef __SRC_CPP_SDK_API_CPP_HH__
#define __SRC_CPP_SDK_API_CPP_HH__

#include <memory>
#include <stdint.h>
#include <vector>
#include <string>
//This is primarily for swig.

class Sdk;
typedef std::shared_ptr<Sdk> SdkPtr;

class SdkEventHandler;
typedef std::shared_ptr<SdkEventHandler> SdkEventHandlerPtr;

class ApiEventHandler
{
public:
    virtual ~ApiEventHandler(){}
    virtual void Authenticated() = 0;
    virtual void AuthenticationFailed(std::vector<std::string> why) = 0;
    virtual void PrimaryForwardingSucceeded(std::vector<std::string> urls) = 0;
    virtual void PrimaryForwardingFailed(std::string) = 0;
    virtual void Disconnected(std::string error, std::vector<std::string> messages) = 0;
    virtual void KeepAliveResponse(uint64_t forTick) = 0;
};
typedef std::shared_ptr<ApiEventHandler> ApiEventHandlerPtr;

class Tunnel
{
public:
    Tunnel(std::string serverAddress="a.pinggy.io:443");
    virtual ~Tunnel();
    void SetEventHandler(ApiEventHandlerPtr eventHandler);

//================
    void SetServerAddress(char *serverAddress);
    void SetToken(char *token);
    void SetMode(char *mode);
    void SetUdpMode(char *udpMode);
    void SetTcpForwardAddress(char *tcpForwardAddress);
    void SetUdpForwardAddress(char *udpForwardAddress);
    void SetEnableWebDebugger(bool enableWebDebugger);
    void SetWebDebuggerPort(int webDebuggerPort);
    void SetPrintContinueUsage(bool printContinueUsage);
    void SetArguments(std::string arguments);
    void SetAdvancedParsing(bool advancedParsing);
    void SetSsl(bool ssl);
    void SetSniServerName(std::string sniServerName);
    void SetInsecure(bool insecure);
//============
    bool Start();

private:
    SdkPtr sdk;
    SdkEventHandlerPtr sdkEventHandler;
    ApiEventHandlerPtr apiEventHandler;
};


#endif // SRC_CPP_SDK_API_CPP_HH__
