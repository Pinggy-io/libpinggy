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

#include "SdkConfig.hh"
#include <utils/StringUtils.hh>
#include <utils/Json.hh>
#include <platform/Log.hh>
#include <platform/network.h>
#include "SdkException.hh"

namespace sdk
{

#define MAX_RECONNECTION_TRY 20

struct HeaderMod : virtual public pinggy::SharedObject
{
    enum class Action {
        Unknown,
        Add,
        Remove,
        Update
    };
    Action                      action;
    tString                     header;
    std::vector<tString>        values;

    HeaderModPtr
    clone();
};
DefineMakeSharedPtr(HeaderMod);


NLOHMANN_JSON_SERIALIZE_ENUM(HeaderMod::Action, {
    {HeaderMod::Action::Unknown,   "unknown"},
    {HeaderMod::Action::Add,       "add"},
    {HeaderMod::Action::Remove,    "remove"},
    {HeaderMod::Action::Update,    "update"},
})

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_CUSTOME_NEW_PTR(HeaderMod,
    (),
    (action, type),
    (header, key),
    (values, value)
);

struct UserPass : virtual public pinggy::SharedObject
{
    tString                     username;
    tString                     password;

    UserPassPtr
    clone();
};
DefineMakeSharedPtr(UserPass);

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_CUSTOME_NEW_PTR1(UserPass,
    (),
    username,
    password
);

SDKConfig::SDKConfig():
    force(false),
    advancedParsing(true),
    ssl(true),
    sniServerName("a.pinggy.io"),
    insecure(false),
    autoReconnect(false),
    maxReconnectAttempts(MAX_RECONNECTION_TRY),
    autoReconnectInterval(5),
    reverseProxy(true),
    xForwardedFor(false),
    httpsOnly(false),
    originalRequestUrl(false),
    allowPreflight(false)
{
}

tString SDKConfig::GetMode()
{
    return mode;
}

tString SDKConfig::GetUdpMode()
{
    return udpMode;
}


#define TO_JSON_STR(x, y)       \
    do {                        \
        json j = x;             \
        y = j.dump();           \
    } while(0)

const tString
SDKConfig::GetHeaderManipulations()
{
    tString val = "";
    TO_JSON_STR(headerManipulations, val);
    return val;
}

const tString
SDKConfig::GetBasicAuths()
{
    tString val = "";
    TO_JSON_STR(basicAuths, val);
    return val;
}

const tString
SDKConfig::GetBearerTokenAuths()
{
    tString val = "";
    TO_JSON_STR(bearerTokenAuths, val);
    return val;
}

const tString
SDKConfig::GetIpWhiteList()
{
    tString val = "";
    TO_JSON_STR(ipWhiteList, val);
    return val;
}

#undef TO_JSON_STR


#define FROM_JSON_STR(x, y)             \
    do {                                \
        if (!y.empty()) {               \
            json j = json::parse(y);    \
            j.get_to(x);                \
        }                               \
    } while(0)


/**
 * AddForwarding adds new forwarding to the forwarding list sdkForwardingList
 * @param forwardingType
 *      forwardingType can be empty or one of the values: http, tcp, tls, udp, tlstcp. It would be set to SdkForwarding::mode.
 *      While forwardingType can be empty, SdkForwarding::mode can't be. The value for SdkForwarding::mode is determined based
 *      on binding url when forwardingType is empty.
 * @param bindingUrl
 *      format [schema://]domain[:port]
 *
 *      bindingUrl may or may not contain a schema. if schema is present, it have to be one of http, tcp, tls, udp, https.
 *      when schema is not present, would be derived from forwarding mode. if both not present, it would be http.
 *      when both of them present, they needs to compatible, http -> [http, https, tcp], tcp -> [tcp], tls -> [tls, tcp], tlstcp -> [tcp], udp -> [udp]
 *      Once schema is finalized, it would update SdkForwarding::mode
 *
 *      Domain is domain only, no ip addess is allowed. It would be put to SdkForwarding::bindingDomain
 *
 *      Port is manadatory when SdkForwarding::mode is one of [tcp, tlstcp, udp]. it would be port to SdkForwarding::bindingPort
 *
 * @param forwardTo
 *      format [schema://][host:]port
 *
 *      schema stored to SdkForwarding::localSchema. You can ignore it unless SdkForwarding::mode is http and schema is https.
 *
 *      default host is localhost unless provided. store it to SdkForwarding::localHost
 *
 *      port is mandatory and there is no default port. store it to SdkForwarding::localPort
 */
void
SDKConfig::AddForwarding(tString forwardingType, tString bindingUrl, tString forwardTo)
{
    // Parse bindingUrl: [schema://]domain[:port]
    tString schema, domain, portStr;
    tPort port = 0;

    auto remoteUrl = NewUrlPtr(bindingUrl, 0, ""); //it might give invalid url exception
    domain = remoteUrl->GetRawHost();
    schema = remoteUrl->GetProtocol();
    port = remoteUrl->GetPort();

    // Validate domain (no IPs allowed)
    if (is_ip_address(domain.c_str())) {
        throw SdkConfigException("IP address not allowed in binding domain");
    }

    auto mode = TunnelModeFromString(StringToLower(forwardingType));
    // Determine mode
    if (mode == TunnelMode::None) {
        if (!schema.empty()) {
            if (schema == Schema_HTTP || schema == Schema_HTTPS) mode = TunnelMode::HTTP;
            else if (schema == Schema_TCP) mode = TunnelMode::TCP;
            else if (schema == Schema_TLS) mode = TunnelMode::TLS;
            else if (schema == Schema_UDP) mode = TunnelMode::UDP;
            else throw SdkConfigException("Unknown schema in bindingUrl");
        } else {
            mode = TunnelMode::HTTP;
        }
    } else {
        // If both schema and mode are present, check compatibility
        if (!schema.empty()) {
            if (mode == TunnelMode::HTTP && !(schema == Schema_HTTP || schema == Schema_HTTPS || schema == Schema_TCP))
                throw SdkConfigException("Incompatible schema and forwardingType");
            if (mode == TunnelMode::TCP && schema != Schema_TCP)
                throw SdkConfigException("Incompatible schema and forwardingType");
            if (mode == TunnelMode::TLS && !(schema == Schema_TLS || schema == Schema_TCP))
                throw SdkConfigException("Incompatible schema and forwardingType");
            if (mode == TunnelMode::TLSTCP && schema != Schema_TCP)
                throw SdkConfigException("Incompatible schema and forwardingType");
            if (mode == TunnelMode::UDP && schema != Schema_UDP)
                throw SdkConfigException("Incompatible schema and forwardingType");
        }
    }

    // Set port requirement for certain types
    if ((mode == TunnelMode::TCP || mode == TunnelMode::TLSTCP || mode == TunnelMode::UDP) && port == 0) {
        throw SdkConfigException("Port is mandatory for tcp, tlstcp, udp types");
    }

    // Parse forwardTo: [schema://][host:]port
    tString localSchema, localHost = "localhost", localPortStr;
    tPort localPort = 0;
    tString forwardToRest = forwardTo;

    auto fwdSchemaSplit = SplitString(forwardToRest, "://", 1);
    if (fwdSchemaSplit.size() == 2) {
        localSchema = StringToLower(fwdSchemaSplit[0]);
        forwardToRest = fwdSchemaSplit[1];
    }

    auto fwdHostPortSplit = SplitString(forwardToRest, ":", 1);
    if (fwdHostPortSplit.size() == 2) {
        localHost = fwdHostPortSplit[0];
        localPortStr = fwdHostPortSplit[1];
    } else {
        localPortStr = forwardToRest;
    }
    try {
        localPort = static_cast<tPort>(std::stoi(localPortStr));
    } catch (...) {
        throw SdkConfigException("Invalid or missing port in forwardTo");
    }

    // Create SdkForwarding object
    auto forwarding = NewSdkForwardingPtr();
    forwarding->mode = mode;
    forwarding->bindingDomain = domain;
    forwarding->bindingPort = port;
    forwarding->localHost = localHost;
    forwarding->localPort = localPort;
    forwarding->localSchema = localSchema;

    // Add to forwarding list
    sdkForwardingList.push_back(forwarding);

    // Set mode/udpMode if not set
    if (mode == TunnelMode::UDP) {
        if (udpMode.empty()) udpMode = TunnelType_UDP;
    } else {
        if (this->mode.empty()) this->mode = TunnelTypeFromTunnelMode(mode);
    }
}

void
SDKConfig::SetHeaderManipulations(tString val)
{
    FROM_JSON_STR(headerManipulations, val);
}

void
SDKConfig::SetBasicAuths(tString val)
{
    FROM_JSON_STR(basicAuths, val);
}

void
SDKConfig::SetBearerTokenAuths(tString val)
{
    FROM_JSON_STR(bearerTokenAuths, val);
}

void
SDKConfig::SetIpWhiteList(tString val)
{
    FROM_JSON_STR(ipWhiteList, val);
}

#undef FROM_JSON_STR

void
SDKConfig::resetArguments() {
    headerManipulations.clear();
    basicAuths.clear();
    bearerTokenAuths.clear();
    ipWhiteList.clear();
    reverseProxy        = true;
    xForwardedFor       = false;
    httpsOnly           = false;
    originalRequestUrl  = false;
    allowPreflight      = false;
    localServerTls      = "";
}

SDKConfigPtr SDKConfig::clone()
{
    auto newConfig = NewSDKConfigPtr();

#define PLAIN_COPY(x) newConfig->x = x
    PLAIN_COPY(token);
    PLAIN_COPY(mode);
    PLAIN_COPY(udpMode);
    PLAIN_COPY(force);
    PLAIN_COPY(advancedParsing);
    PLAIN_COPY(ssl);
    PLAIN_COPY(sniServerName);
    PLAIN_COPY(insecure);
    PLAIN_COPY(autoReconnect);
    PLAIN_COPY(maxReconnectAttempts);
    PLAIN_COPY(autoReconnectInterval);
    PLAIN_COPY(reverseProxy);
    PLAIN_COPY(xForwardedFor);
    PLAIN_COPY(httpsOnly);
    PLAIN_COPY(originalRequestUrl);
    PLAIN_COPY(allowPreflight);
    PLAIN_COPY(localServerTls);

#define URLPTR_COPY(x) newConfig->x = x->Clone()

    URLPTR_COPY(serverAddress);
    URLPTR_COPY(tcpForwardTo);
    URLPTR_COPY(udpForwardTo);


    //Other argument options
    for (auto x : headerManipulations){
        newConfig->headerManipulations.push_back(x->clone());
    }
    for (auto x : basicAuths){
        newConfig->basicAuths.push_back(x->clone());
    }
    for (tString x : bearerTokenAuths){
        newConfig->bearerTokenAuths.push_back(x);
    }
    for (tString x : ipWhiteList){
        newConfig->ipWhiteList.push_back(x);
    }

#undef PLAIN_COPY
#undef URLPTR_COPY

    return newConfig;
}

void
SDKConfig::validate()
{
    if (!serverAddress) {
        serverAddress = NewUrlPtr("a.pinggy.io:443");
    }

    if (tcpForwardTo && mode.empty()) {
        mode = TunnelType_HTTP;
    }

    if (udpForwardTo && udpMode.empty()) {
        udpMode = TunnelType_UDP;
    }

    if (    mode != TunnelType_HTTP
         && mode != TunnelType_TCP
         && mode != TunnelType_TLS
         && mode != TunnelType_TLSTCP)
        mode = TunnelType_None;

    if (udpMode != TunnelType_UDP)
        udpMode = TunnelType_None;

    if (mode.empty() && udpMode.empty())
        mode = TunnelType_HTTP;
}

tString
SDKConfig::getUser()
{
    tString user = "";
    if (!token.empty()) {
        user += "+" + token;
    }

    if (!mode.empty()) {
        user += "+" + mode;
    }

    if (!udpMode.empty()) {
        user += "+" + udpMode;
    }

    if (force) {
        user += "+force";
    }

    return user.substr(1);
}

void
SDKConfig::SetArguments(tString args)
{
    resetArguments();
    auto cmds = ShlexSplitString(args);

    for (auto cmd : cmds) {
        auto vals = SplitString(cmd, ":", 1);
        if (vals.size() < 2 || vals[0].empty()) //sliently ignore unknow cmds
            continue;
        auto cmdType = cmd.at(0);
        switch(cmdType) {
            case 'a':
            case 'u':
            case 'r':
                {
                    auto bodies = SplitString(vals[1], ":", 1);
                    if ((cmdType == 'r' && bodies.size() < 1) || bodies.size() < 2)
                        break;
                    auto hm = NewHeaderModPtr();
                    hm->header = bodies[0];
                    hm->values.push_back(bodies[1]);
                    if (cmdType == 'a') hm->action = HeaderMod::Action::Add;
                    else hm->action = HeaderMod::Action::Add;

                    headerManipulations.push_back(hm);
                }
                break;
            case 'b':
                {
                    auto bodies = SplitString(vals[1], ":", 1);
                    if (bodies.size() < 2)
                        break;
                    auto pu = NewUserPassPtr();
                    pu->username = bodies[0];
                    pu->password = bodies[1];
                    basicAuths.push_back(pu);
                }
                break;
            case 'k':
                bearerTokenAuths.push_back(vals[1]);
                break;
            case 'w':
                {
                    auto bodies = SplitString(vals[1], ",");
                    ipWhiteList.insert(ipWhiteList.end(), bodies.begin(), bodies.end());
                }
                break;
            case 'x':
                {
                    auto bodies = SplitString(vals[1], ":", 1);
                    if (bodies.size() < 1)
                        break;
                    auto keyType = StringToLower(bodies[0]);
                    if (keyType == "https") {
                        httpsOnly = true;
                    } else if (keyType == "xff") {
                        xForwardedFor = true;
                    } else if (keyType == "fullurl") {
                        originalRequestUrl = true;
                    } else if (keyType == "localservertls") {
                        localServerTls = bodies.size() > 1 ? bodies[1] : "localhost" ;
                    } else if (keyType == "passpreflight") {
                        allowPreflight = true;
                    } else if (keyType == "noreverseproxy") {
                        reverseProxy = false;
                    }
                }
        }
    }
}

const tString
SDKConfig::GetArguments()
{
    std::vector<tString> val;

    if (ipWhiteList.size()) {
        tString whitelist = "w:";
        whitelist += JoinString(ipWhiteList, ",");
        val.push_back(whitelist);
    }

    if (basicAuths.size()) {
        for (auto ele : basicAuths)
            val.push_back("b:" + ele->username + ":" + ele->password);
    }

    if (bearerTokenAuths.size()) {
        for (auto key : bearerTokenAuths)
            val.push_back("k:" + key);
    }

    if (headerManipulations.size()) {
        for (auto hm : headerManipulations) {
            switch (hm->action)
            {
            case HeaderMod::Action::Add:
                for (auto v : hm->values)
                    val.push_back("a:" + hm->header + ":" + v);
                break;

            case HeaderMod::Action::Update:
                for (auto v : hm->values)
                    val.push_back("u:" + hm->header + ":" + v);
                break;

            case HeaderMod::Action::Remove:
                val.push_back("r:" + hm->header);
                break;

            default:
                break;
            }
        }
    }

    if (xForwardedFor)
        val.push_back("x:xff");

    if (httpsOnly)
        val.push_back("x:https");

    if (originalRequestUrl)         // bool = False
        val.push_back("x:fullurl");

    if (allowPreflight)             // bool = False
        val.push_back("x:passpreflight");

    if (!reverseProxy)             // bool = False
        val.push_back("x:noreverseproxy");

    if (!localServerTls.empty())
        val.push_back("x:localServerTls:"+localServerTls);

    auto cmds = ShlexJoinStrings(val);

    return cmds;
}

void
SDKConfig::SetGlobalConfig(tString args)
{
    json jdata = json::parse(args);
    tString version = "1.0";
    PINGGY_NLOHMANN_JSON_TO_VAR2(jdata, (version, version));
    if (version != "1.0") {
        throw SdkConfigException("Only version 1.0 is supported");
    }

}

HeaderModPtr HeaderMod::clone()
{
    auto newMode = NewHeaderModPtr();
    newMode->action     = action;
    newMode->header     = header;
    for (tString x : values)
        newMode->values.push_back(x);

    return newMode;
}

UserPassPtr UserPass::clone()
{
    auto up = NewUserPassPtr();
    up->username = username;
    up->password = password;
    return up;
}

} // namespace sdk
