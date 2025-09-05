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

#ifndef __SRC_CPP_PUBLIC_SDK_SDKCONFIG_HH__
#define __SRC_CPP_PUBLIC_SDK_SDKCONFIG_HH__

#include <utils/Utils.hh>
#include <vector>

namespace sdk
{

class Sdk;

DeclareStructWithSharedPtr(HeaderMod);
DeclareStructWithSharedPtr(UserPass);

struct SDKConfig: virtual public pinggy::SharedObject
{
    SDKConfig();


    tString
    GetToken()                  { return token; }

    tString
    GetMode()                   { return mode; }

    tString
    GetUdpMode()                { return udpMode; }

    UrlPtr
    GetServerAddress()          { return serverAddress; }

    UrlPtr
    GetTcpForwardTo()           { return tcpForwardTo; }

    UrlPtr
    GetUdpForwardTo()           { return udpForwardTo; }

    bool
    IsForce()                   { return force; }

    bool
    IsAdvancedParsing()         { return advancedParsing; }

    bool
    IsSsl()                     { return ssl; }

    tString
    GetSniServerName()          { return sniServerName; }

    bool
    IsInsecure()                { return insecure; }

    bool
    IsAutoReconnect()           { return autoReconnect; }

    tUint16
    GetMaxReconnectAttempts()   { return maxReconnectAttempts; }

    const tString
    GetHeaderManipulations();

    const tString
    GetBasicAuths();

    const tString
    GetBearerTokenAuths();

    const tString
    GetIpWhiteList();

    bool
    IsReverseProxy()
                                { return reverseProxy; }

    bool
    IsXForwarderFor()
                                { return xForwarderFor; }

    bool
    IsHttpsOnly()
                                { return httpsOnly; }

    bool
    IsOriginalRequestUrl()
                                { return originalRequestUrl; }

    bool
    IsAllowPreflight()
                                { return allowPreflight; }

    bool
    IsNoReverseProxy()
                                { return !reverseProxy; }

    const tString &
    GetLocalServerTls()
                                { return localServerTls; }

    void
    SetToken(tString token)     { isAllowed(); this->token = token; }

    void
    SetMode(tString mode)       { isAllowed(); this->mode = mode; }

    void
    SetUdpMode(tString udpMode) { isAllowed(); this->udpMode = udpMode; }

    void
    SetServerAddress(UrlPtr serverAddress)
                                { isAllowed(); this->serverAddress = serverAddress; }

    void
    SetTcpForwardTo(UrlPtr tcpForwardTo)
                                { isAllowed(); this->tcpForwardTo = tcpForwardTo; }

    void
    SetUdpForwardTo(UrlPtr udpForwardTo)
                                { isAllowed(); this->udpForwardTo = udpForwardTo; }

    void
    SetForce(bool force)        { isAllowed(); this->force = force; }

    void
    SetAdvancedParsing(bool advancedParsing)
                                { isAllowed(); this->advancedParsing = advancedParsing; }

    void
    SetSsl(bool ssl)            { isAllowed(); this->ssl = ssl; }

    void
    SetSniServerName(tString sniServerName)
                                { isAllowed(); this->sniServerName = sniServerName; }

    void
    SetInsecure(bool insecure)  { isAllowed(); this->insecure = insecure; }

    void
    SetAutoReconnect(bool autoReconnect)
                                { isAllowed(); this->autoReconnect = autoReconnect; }

    void
    SetMaxReconnectAttempts(tUint16 maxReconnectAttempts)
                                { isAllowed(); this->maxReconnectAttempts = maxReconnectAttempts; }

    void
    SetHeaderManipulations(tString val);

    void
    SetBasicAuths(tString val);

    void
    SetBearerTokenAuths(tString val);

    void
    SetIpWhiteList(tString val);

    void
    SetReverseProxy(bool val)
                                { reverseProxy = val; }

    void
    SetXForwarderFor(bool val)
                                { xForwarderFor = val; }

    void
    SetHttpsOnly(bool val)
                                { httpsOnly = val; }

    void
    SetOriginalRequestUrl(bool val)
                                { originalRequestUrl = val; }

    void
    SetAllowPreflight(bool val)
                                { allowPreflight = val; }

    void
    SetNoReverseProxy(bool val)
                                { reverseProxy = !val; }

    void
    SetLocalServerTls(tString val)
                                { localServerTls = val; }

    //===================

    void
    SetArguments(tString args);

    const tString
    GetArguments();

    void
    SetGlobalConfig(tString args);

private:
    friend class Sdk;

    //The token and any other parameters as well.
    tString                     token;

    //The tcp tunnel type tcp, tls, tlstcp or http
    tString                     mode;

    //The udp tunnel type i.e. udp
    tString                     udpMode;

    //sshOverSsl does not exists here as it use ssl only, no ssh

    // Pinggy server address. It is supposed to be a.pinggy.io or regional server as well.
    UrlPtr                      serverAddress;

    //this TcpForwarding address
    UrlPtr                      tcpForwardTo;

    //this UdpForwarding address
    UrlPtr                      udpForwardTo;

    //force login. It add `force` as user name
    bool                        force;

    //Whether if we want to run advancedparsing for http.
    // disabling this would disable webdebugger as well.
    bool                        advancedParsing;

    //Enable it if you wants to connect with server using
    // encrypted ssl channel or not. Most of the production
    // production server does not support plaintext connection.
    // enable it all the times.
    bool                        ssl;

    //this needs to set to a.pinggy.io. Some test server may
    // accept values different than a.pinggy.io.
    tString                     sniServerName;

    bool                        insecure;

    bool                        autoReconnect;

    tUint16                     maxReconnectAttempts;

    //Other argument options
    std::vector<HeaderModPtr>   headerManipulations;
    std::vector<UserPassPtr>    basicAuths;
    std::vector<tString>        bearerTokenAuths;
    std::vector<tString>        ipWhiteList;
    bool                        reverseProxy;
    bool                        xForwarderFor;
    bool                        httpsOnly;
    bool                        originalRequestUrl;
    bool                        allowPreflight;
    tString                     localServerTls;

    void
    isAllowed()                 { }

    void
    validate();

    tString
    getUser();

    void
    resetArguments();
};
DefineMakeSharedPtr(SDKConfig);

} // namespace sdk


#endif // __SRC_CPP_PUBLIC_SDK_SDKCONFIG_HH__