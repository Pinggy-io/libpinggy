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

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_CUSTOME_NEW_PTR(SdkForwarding,
    (),
    (forwardingUrl,         address),
    (bindingUrl,        listenAddress),
    (mode,    type)
)

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

// tString SDKConfig::GetMode()
// {
//     return mode;
// }

// tString SDKConfig::GetUdpMode()
// {
//     return udpMode;
// }


tString
SDKConfig::GetForwardings()
{
    json j = sdkForwardingList;
    return j.dump();
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


void
SDKConfig::AddForwarding(tString forwardingType, tString bindingUrl, tString forwardTo)
{
    auto forwarding = parseForwarding(forwardingType, bindingUrl, forwardTo);
    // Add to forwarding list
    sdkForwardingList.push_back(forwarding);
}

void
SDKConfig::AddForwarding(tString forwardTo)
{
    auto forwarding = parseForwarding(forwardTo);
    // Add to forwarding list
    sdkForwardingList.push_back(forwarding);
}

void
SDKConfig::SetForwarding(tString val)
{
    try {
        LOGD("The forwardings: ", val);
        // json j = json::parse(forwardings);
        std::vector<SdkForwardingPtr> parsedForwardings;
        FROM_JSON_STR(parsedForwardings, val);
        ResetForwardings();
        for (auto forward : parsedForwardings) {
            AddForwarding(forward->origForwardingType, forward->origBindingUrl, forward->origForwardTo);
        }
    } catch (std::exception &e) {
        ResetForwardings();
        AddForwarding(val);
        // LOGD(e.what());
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
    // PLAIN_COPY(mode);
    // PLAIN_COPY(udpMode);
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
    PLAIN_COPY(webDebug);
    PLAIN_COPY(webDebugBindAddr);

#define URLPTR_COPY(x) newConfig->x = x->Clone()

    URLPTR_COPY(serverAddress);
    // URLPTR_COPY(tcpForwardTo);
    // URLPTR_COPY(udpForwardTo);


    //Other argument options
    for (auto x : headerManipulations) {
        newConfig->headerManipulations.push_back(x->clone());
    }
    for (auto x : basicAuths) {
        newConfig->basicAuths.push_back(x->clone());
    }
    for (tString x : bearerTokenAuths) {
        newConfig->bearerTokenAuths.push_back(x);
    }
    for (tString x : ipWhiteList) {
        newConfig->ipWhiteList.push_back(x);
    }
    for (auto x : sdkForwardingList) {
        newConfig->sdkForwardingList.push_back(x->Clone());
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
}

tString
SDKConfig::getUser()
{
    tString user = "";
    if (!token.empty()) {
        auto tparts = SplitString(token, "+");
        for (auto x : tparts) {
            auto sl = StringToLower(x);
            if (sl == TunnelType_HTTP || sl == TunnelType_TCP || sl == TunnelType_TLS || sl == TunnelType_TLSTCP || sl == TunnelType_UDP ) {
                continue;
            }
            user += "+" + x;
        }
    }

    if (force) {
        user += "+force";
    }
    if (user.empty())
        return "";

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

/**
 * AddForwarding adds new forwarding to the forwarding list sdkForwardingList
 * @param forwardingType
 *      forwardingType can be empty or one of the values: http, tcp, tls, udp, tlstcp. It would be set to SdkForwarding::mode.
 *      forwardingType can not be empty. Raise exception if it is empty.
 * @param bindingUrl
 *      format [schema://]domain[:port]
 *
 *      bindingUrl may or may not contain a schema. if schema is ignored if exists.
 *
 *      Domain is domain only, no ip addess is allowed. It would be put to SdkForwarding::bindingDomain
 *
 *      Port would be port to SdkForwarding::bindingPort. Port is useful only for [tcp, tlstcp, udp]. Port should be
 *      valid or zero (for all the registered port for the token). Server rejects forwarding with invalid port for forwarding type
 *      [tcp, tlstcp, udp].
 *
 * @param forwardTo
 *      format [schema://][host:]port
 *
 *      schema stored to SdkForwarding::fwdToSchema. You can ignore it unless SdkForwarding::mode is http and schema is https.
 *
 *      default host is localhost unless provided. store it to SdkForwarding::fwdToHost
 *
 *      port is mandatory and there is no default port. store it to SdkForwarding::fwdToPort
 */
SdkForwardingPtr SDKConfig::parseForwarding(tString forwardingType, tString bindingUrl, tString forwardTo)
{
    // Parse bindingUrl: [schema://]domain[:port]
    tString schema, domain, portStr;
    tPort port = 0;

    auto schemaSplit = SplitString(bindingUrl, "://", 1);
    if (schemaSplit.size() == 2) {
        schema = StringToLower(schemaSplit[0]);
        bindingUrl = schemaSplit[1];
    }

    auto portSplit = SplitString(bindingUrl, ":", 1);
    if (portSplit.size() == 2) {
        domain = portSplit[0];
        portStr = portSplit[1];
        port = static_cast<tPort>(std::stoi(portStr));
    } else {
        domain = bindingUrl;
    }

    // Validate domain (no IPs allowed)
    if (is_ip_address(domain.c_str())) {
        throw SdkConfigException("IP address not allowed in binding domain");
    }

    auto sl = StringToLower(forwardingType);
    if (!sl.empty() && sl != TunnelType_HTTP && sl != TunnelType_TCP && sl != TunnelType_TLS && sl != TunnelType_TLSTCP && sl != TunnelType_UDP ) {
        throw SdkConfigException("Invalid forwarding type `" + forwardingType + "`");
    }
    auto mode = TunnelModeFromString(sl);
    // Determine mode
    if (mode == TunnelMode::None) {
        mode = TunnelMode::HTTP;
    }

    // Parse forwardTo: [schema://][host:]port
    tString fwdToSchema, fwdToHost = "localhost", localPortStr;
    tPort localPort = 0;
    tString forwardToRest = forwardTo;

    auto fwdSchemaSplit = SplitString(forwardToRest, "://", 1);
    if (fwdSchemaSplit.size() == 2) {
        fwdToSchema = StringToLower(fwdSchemaSplit[0]);
        forwardToRest = fwdSchemaSplit[1];
    }

    auto fwdHostPortSplit = SplitString(forwardToRest, ":", 1);
    if (fwdHostPortSplit.size() == 2) {
        fwdToHost = fwdHostPortSplit[0];
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
    forwarding->mode                    = mode;
    forwarding->bindingDomain           = domain;
    forwarding->bindingPort             = port;
    forwarding->bindingUrl             = domain + (port > 0 ? ":" + portStr : "");

    forwarding->fwdToHost               = fwdToHost;
    forwarding->fwdToPort               = localPort;
    forwarding->forwardingUrl           = (fwdToSchema == "https" ? "https://" : "") + fwdToHost + ":" + localPortStr;

    forwarding->localServerTls          = fwdToSchema == "https";

    forwarding->origBindingUrl          = bindingUrl;
    forwarding->origForwardTo           = forwardTo;
    forwarding->origForwardingType      = forwardingType;

    return forwarding;
}

SdkForwardingPtr
SDKConfig::parseForwarding(tString forwardTo)
{
    // Parse forwardTo: [schema://][host:]port
    tString fwdToSchema, fwdToHost = "localhost", localPortStr;
    tPort localPort = 0;
    tString forwardToRest = forwardTo;

    auto fwdSchemaSplit = SplitString(forwardToRest, "://", 1);
    if (fwdSchemaSplit.size() == 2) {
        fwdToSchema = StringToLower(fwdSchemaSplit[0]);
        forwardToRest = fwdSchemaSplit[1];
    }

    auto sl = StringToLower(fwdToSchema);
    if (!sl.empty() && sl != TunnelType_HTTP && sl != TunnelType_TCP && sl != TunnelType_TLS && sl != TunnelType_TLSTCP && sl != TunnelType_UDP ) {
        throw SdkConfigException("Invalid forwarding type `" + fwdToSchema + "`");
    }

    bool localServerTls = false;
    if (sl == "https") { // very special case
        localServerTls = true;
        sl = TunnelType_HTTP;
    }

    auto mode = TunnelModeFromString(sl);
    // Determine mode
    if (mode == TunnelMode::None) {
        mode = TunnelMode::HTTP;
    }

    auto fwdHostPortSplit = SplitString(forwardToRest, ":", 1);
    if (fwdHostPortSplit.size() == 2) {
        fwdToHost = fwdHostPortSplit[0];
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
    forwarding->mode                    = mode;
    forwarding->bindingDomain           = "";
    forwarding->bindingPort             = 0;
    forwarding->bindingUrl             = "";

    forwarding->fwdToHost               = fwdToHost;
    forwarding->fwdToPort               = localPort;
    forwarding->forwardingUrl           = (fwdToSchema == "https" ? "https://" : "") + fwdToHost + ":" + localPortStr;

    forwarding->localServerTls          = localServerTls;

    forwarding->origBindingUrl          = "";
    forwarding->origForwardTo           = forwardTo;
    forwarding->origForwardingType      = "";

    return forwarding;
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

SdkForwardingPtr
SdkForwarding::Clone()
{
    auto newForwarding = NewSdkForwardingPtr();
    newForwarding->mode               = mode;
    newForwarding->bindingPort        = bindingPort;
    newForwarding->bindingDomain      = bindingDomain;
    newForwarding->bindingUrl         = bindingUrl;
    newForwarding->fwdToPort          = fwdToPort;
    newForwarding->fwdToHost          = fwdToHost;
    newForwarding->forwardingUrl      = forwardingUrl;
    newForwarding->localServerTls     = localServerTls;
    newForwarding->origForwardTo      = origForwardTo;
    newForwarding->origBindingUrl     = origBindingUrl;
    newForwarding->origForwardingType = origForwardingType;

    return newForwarding;
}

} // namespace sdk
