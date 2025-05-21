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


#include "Sdk.hh"
#include <mutex>
#include <exception>

#define PINGGY_TYPETEST_ENABLED

#include "pinggy.h"

#ifndef PLATFORM_CONFIG_INCLUDED

#define PinggyVersionMajor 0
#define PinggyVersionMinor 0
#define PinggyVersionPatch 0

#define PINGGY_GIT_COMMIT_ID "unknown"
#define PINGGY_BUILD_TIMESTAMP "0000-00-00 00:00:00"
#define PINGGY_LIBC_VERSION "unknown"
#define PINGGY_BUILD_OS "unknown"

#endif


//==============================================================
std::map<pinggy_ref_t, pinggy::VoidPtr> pinggyReferenceMap;
std::mutex globalMutex;
//==============================================================
PINGGY_EXPORT pinggy_void_t
pinggy_set_log_path(pinggy_char_p_t path)
{
    if (!path)
        return;
    InitLog(tString(path));
}
PINGGY_EXPORT pinggy_void_t
pinggy_set_log_enable(pinggy_bool_t val)
{
    SetGlobalLogEnable(val != 0);
}


PINGGY_EXPORT pinggy_bool_t
pinggy_is_interrupted(pinggy_ref_t tunnel)
{
    return app_get_errno() == EINTR;
}
//==============================================================

static pinggy_ref_t
getRef(pinggy::VoidPtr ptr)
{
    std::lock_guard<std::mutex> lock(globalMutex);

    static pinggy_ref_t curRef= 2;
    curRef += 1;

    pinggyReferenceMap[curRef] = ptr;
    return curRef;
}

static pinggy::VoidPtr
getObj(pinggy_ref_t ref)
{
    std::lock_guard<std::mutex> lock(globalMutex);
    if (pinggyReferenceMap.find(ref) == pinggyReferenceMap.end()) {
        return nullptr;
    }
    return pinggyReferenceMap[ref];
}

#define GetObjFuncs2(name,cls) \
static cls##Ptr \
get##name(pinggy_ref_t ref) \
{ \
    auto obj = getObj(ref); \
    if (obj == nullptr) \
        return nullptr; \
    return obj->DynamicPointerCast<cls>(); \
}
#define GetObjFuncs(name) \
        GetObjFuncs2(name,sdk::name)

GetObjFuncs(Sdk);
GetObjFuncs(SDKConfig);
// GetObjFuncs2(PollController, common::PollController);
GetObjFuncs(SdkChannelWraper);

#undef GetObjFuncs
#undef GetObjFuncs2


//==============================================================

struct ApiChannelEventHandler: public virtual sdk::SdkChannelEventHandler
{
    pinggy_channel_data_received_cb_t
                                channelDataReceivedCB;
    pinggy_channel_readyto_send_cb_t
                                channelReadyToSendCB;
    pinggy_channel_error_cb_t   channelErrorCB;
    pinggy_channel_cleanup_cb_t channelCleanupCB;

    pinggy_void_p_t             channelDataReceivedUserData;
    pinggy_void_p_t             channelReadyToSendUserData;
    pinggy_void_p_t             channelErrorUserData;
    pinggy_void_p_t             channelCleanupUserData;

    pinggy_ref_t                channelRef;

    ApiChannelEventHandler():
                            channelDataReceivedCB(NULL),
                            channelReadyToSendCB(NULL),
                            channelErrorCB(NULL),
                            channelCleanupCB(NULL),
                            channelDataReceivedUserData(NULL),
                            channelReadyToSendUserData(NULL),
                            channelErrorUserData(NULL),
                            channelCleanupUserData(NULL),
                            channelRef(INVALID_PINGGY_REF)
                                { }

    virtual pinggy_void_t
    ChannelDataReceived()
    {
        if (!channelDataReceivedCB) return;
        channelDataReceivedCB(channelDataReceivedUserData, channelRef);
    }

    virtual pinggy_void_t
    ChannelReadyToSend(tUint32 bufferLen)
    {
        if (!channelReadyToSendCB) return;
        channelReadyToSendCB(channelReadyToSendUserData, channelRef, bufferLen);
    }

    virtual pinggy_void_t
    ChannelError(tString errorText)
    {
        if (!channelErrorCB) return;
        channelErrorCB(channelErrorUserData, channelRef, errorText.c_str(), errorText.length());
    }

    virtual pinggy_void_t
    ChannelCleanup()
    {
        if (!channelCleanupCB) return;
        channelCleanupCB(channelCleanupUserData, channelRef);
        pinggy_free_ref(channelRef);
    }

};
DefineMakeSharedPtr(ApiChannelEventHandler)

//==============================================================
struct ApiEventHandler: virtual public sdk::SdkEventHandler
{
public:
    pinggy_on_authenticated_cb_t
                                onConnectedCB;
    pinggy_on_authenticated_cb_t
                                onAuthenticatedCB;
    pinggy_on_authentication_failed_cb_t
                                onAuthenticationFailedCB;
    pinggy_on_primary_forwarding_succeeded_cb_t
                                onPrimaryForwardingSucceededCB;
    pinggy_on_primary_forwarding_failed_cb_t
                                onPrimaryForwardingFailedCB;
    pinggy_on_additional_forwarding_succeeded_cb_t
                                onAdditionalForwardingSucceededCB;
    pinggy_on_additional_forwarding_failed_cb_t
                                onAdditionalForwardingFailedCB;
    pinggy_on_disconnected_cb_t onDisconnectedCB;
    pinggy_on_tunnel_error_cb_t onErrorCB;
    pinggy_on_new_channel_cb_t  onNewChannelCB;

    pinggy_void_p_t             onConnectedUserData;
    pinggy_void_p_t             onAuthenticatedUserData;
    pinggy_void_p_t             onAuthenticationFailedUserData;
    pinggy_void_p_t             onPrimaryForwardingSucceededUserData;
    pinggy_void_p_t             onPrimaryForwardingFailedUserData;
    pinggy_void_p_t             onAdditionalForwardingSucceededUserData;
    pinggy_void_p_t             onAdditionalForwardingFailedUserData;
    pinggy_void_p_t             onDisconnectedUserData;
    pinggy_void_p_t             onErrorUserData;
    pinggy_void_p_t             onNewChannelUserData;

    pinggy_ref_t                sdk;

    ApiEventHandler():
                        onConnectedCB(NULL),
                        onAuthenticatedCB(NULL),
                        onAuthenticationFailedCB(NULL),
                        onPrimaryForwardingSucceededCB(NULL),
                        onPrimaryForwardingFailedCB(NULL),
                        onAdditionalForwardingSucceededCB(NULL),
                        onAdditionalForwardingFailedCB(NULL),
                        onDisconnectedCB(NULL),
                        onNewChannelCB(NULL),
                        onAuthenticatedUserData(NULL),
                        onAuthenticationFailedUserData(NULL),
                        onPrimaryForwardingSucceededUserData(NULL),
                        onPrimaryForwardingFailedUserData(NULL),
                        onAdditionalForwardingSucceededUserData(NULL),
                        onAdditionalForwardingFailedUserData(NULL),
                        onDisconnectedUserData(NULL),
                        onNewChannelUserData(NULL),
                        sdk(INVALID_PINGGY_REF) {}

    virtual ~ApiEventHandler()  { }

#define GetCStringArray(cVec,vec) \
        auto cVec = new pinggy_char_p_t[vec.size()+2]; \
        for (size_t i = 0; i < vec.size(); ++i) { \
            cVec[i] = new char[vec[i].length() + 3]; \
            strncpy(cVec[i], vec[i].c_str(), vec[i].length()); \
            cVec[i][vec[i].length()] = 0; \
        }
#define ReleaseCStringArray(cVec, vec) \
        for (size_t i = 0; i < vec.size(); ++i) \
            delete[] cVec[i]; \
        delete[] cVec; \

    virtual pinggy_void_t
    OnConnected()
    {
        if (onConnectedCB) onConnectedCB(onConnectedUserData, sdk);
    }

    virtual pinggy_void_t
    OnAuthenticated()
    {
        if (onAuthenticatedCB) onAuthenticatedCB(onAuthenticatedUserData, sdk);
    }
    virtual pinggy_void_t
    OnAuthenticationFailed(std::vector<tString> why)
    {
        if (!onAuthenticationFailedCB) return;
        GetCStringArray(cWhy, why)
        onAuthenticationFailedCB(onAuthenticationFailedUserData, sdk, why.size(), cWhy);
        ReleaseCStringArray(cWhy, why);
    }
    virtual pinggy_void_t
    OnPrimaryForwardingSucceeded(std::vector<tString> urls)
    {
        if (!onPrimaryForwardingSucceededCB) {
            LOGD("onPrimaryForwardingSucceededCB does not exists");
            return;
        }
        GetCStringArray(cUrls, urls);
        onPrimaryForwardingSucceededCB(onPrimaryForwardingSucceededUserData, sdk, urls.size(), cUrls);
        ReleaseCStringArray(cUrls, urls);
    }
    virtual pinggy_void_t
    OnPrimaryForwardingFailed(tString message)
    {
        if (!onPrimaryForwardingFailedCB) return;
        onPrimaryForwardingFailedCB(onPrimaryForwardingFailedUserData, sdk, message.c_str());
    }
    virtual pinggy_void_t
    OnRemoteForwardingSuccess(UrlPtr bindAddress, UrlPtr forwardTo)
    {
        if (!onAdditionalForwardingSucceededCB) return;
        auto cBindAddress = bindAddress->GetSockAddrString();
        auto cForwardTo = forwardTo->GetSockAddrString();
        onAdditionalForwardingSucceededCB(onAdditionalForwardingSucceededUserData, sdk, cBindAddress.c_str(), cForwardTo.c_str());
    }
    virtual pinggy_void_t
    OnRemoteForwardingFailed(UrlPtr bindAddress, UrlPtr forwardTo, tString error)
    {
        if (!onAdditionalForwardingFailedCB) return;
        auto cBindAddress = bindAddress->GetSockAddrString();
        auto cForwardTo = forwardTo->GetSockAddrString();
        auto cError = error;
        onAdditionalForwardingFailedCB(onAdditionalForwardingFailedUserData, sdk, cBindAddress.c_str(), cForwardTo.c_str(), cError.c_str());
    }
    virtual pinggy_void_t
    OnDisconnected(tString error, std::vector<tString> messages)
    {
        if (!onDisconnectedCB) return;
        GetCStringArray(cMsg, messages);
        onDisconnectedCB(onDisconnectedUserData, sdk, error.c_str(), messages.size(), cMsg);
        ReleaseCStringArray(cMsg, messages);
    }
    virtual pinggy_void_t
    KeepAliveResponse(tUint64 forTick)
    {
    }
    virtual pinggy_void_t
    OnHandleError(tUint32 errorNo, tString what, tBool recoverable)
    {
        if (!onErrorCB) return;
        onErrorCB(onErrorUserData, sdk, errorNo, what.c_str(), recoverable?1:0);
    }
    virtual bool
    OnNewVisitorConnectionReceived(sdk::SdkChannelWraperPtr channel)
    {
        if (!onNewChannelCB) return false;
        auto channelRef             = getRef(channel);
        auto channelHandler         = NewApiChannelEventHandlerPtr();
        channelHandler->channelRef  = channelRef;
        channel->RegisterEventHandler(channelHandler);
        auto ret = onNewChannelCB(onNewChannelUserData, sdk, channelRef) ? true : false;
        if (ret == false)
            pinggy_free_ref(channelRef);

        return ret;
    }

#undef GetCStringArray
#undef ReleaseCStringArray
};
DefineMakeSharedPtr(ApiEventHandler);

//==============================================================

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#define CopyStringToOutput(capa_, val_, str_) \
    if (capa_ < (str_.length()+1) || str_.length() == 0) \
        return 0; \
    memcpy(val_, str_.c_str(), str_.length()+1);

static pinggy_on_raise_exception_cb_t exception_callback = NULL;

PINGGY_EXPORT pinggy_void_t
pinggy_set_on_exception_callback(pinggy_on_raise_exception_cb_t cb)
{
    exception_callback = cb;
}

PINGGY_EXPORT pinggy_bool_t
pinggy_free_ref(pinggy_ref_t ref)
{
    std::lock_guard<std::mutex> lock(globalMutex);
    if (pinggyReferenceMap.find(ref) != pinggyReferenceMap.end()) {
        // LOGI("Freeing ref", ref);
        pinggyReferenceMap.erase(ref);
        return pinggy_true;
    } else {
        LOGE("Invalid ref", ref);
        return pinggy_false;
    }
}

PINGGY_EXPORT pinggy_ref_t
pinggy_create_config()
{
    auto sdkConf = sdk::NewSDKConfigPtr();
    return getRef(sdkConf);
}

#define EmptyStringIfNull(z) (z ? z : "")

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_server_address(pinggy_ref_t ref, pinggy_char_p_t server_address)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    sdkConf->ServerAddress = NewUrlPtr(EmptyStringIfNull(server_address), 443);
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_token(pinggy_ref_t ref, pinggy_char_p_t token)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    sdkConf->Token = EmptyStringIfNull(token);
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_type(pinggy_ref_t ref, pinggy_char_p_t mode)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    sdkConf->Mode = EmptyStringIfNull(mode);
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_udp_type(pinggy_ref_t ref, pinggy_char_p_t udp_type)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    sdkConf->UdpMode = EmptyStringIfNull(udp_type);
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_tcp_forward_to(pinggy_ref_t ref, pinggy_char_p_t tcp_forward_to)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    sdkConf->TcpForwardTo = NewUrlPtr(EmptyStringIfNull(tcp_forward_to));
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_udp_forward_to(pinggy_ref_t ref, pinggy_char_p_t udp_forward_to)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    sdkConf->UdpForwardTo = NewUrlPtr(EmptyStringIfNull(udp_forward_to), 80, "udp");
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_force(pinggy_ref_t ref, pinggy_bool_t force)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    sdkConf->Force = force;
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_argument(pinggy_ref_t ref, pinggy_char_p_t argument)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    sdkConf->Argument = EmptyStringIfNull(argument);
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_advanced_parsing(pinggy_ref_t ref, pinggy_bool_t advanced_parsing)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    sdkConf->AdvancedParsing = advanced_parsing;
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_ssl(pinggy_ref_t ref, pinggy_bool_t ssl)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    sdkConf->Ssl = ssl;
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_sni_server_name(pinggy_ref_t ref, pinggy_char_p_t sni_server_name)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    sdkConf->SniServerName = EmptyStringIfNull(sni_server_name);
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_insecure(pinggy_ref_t ref, pinggy_bool_t insecure)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    sdkConf->Insecure = insecure;
}

// #undef EmptyStringIfNull

#define SdkConfigCopyStringToOutput(capa_, val_, str_) \
    auto sdkConf = getSDKConfig(ref); \
    if (!sdkConf) { \
        LOGE("No sdkConf found for the ref:", ref); \
        return 0; \
    } \
    if (capa_ < (sdkConf->str_.length()+1) || sdkConf->str_.length() == 0) \
        return 0; \
    memcpy(val_, sdkConf->str_.c_str(), sdkConf->str_.length()+1);\
    return sdkConf->str_.length()


#define SdkConfigCopyUrlToOutput(capa_, val_, url_) \
    auto sdkConf = getSDKConfig(ref); \
    if (!sdkConf) { \
        LOGE("No sdkConf found for the ref:", ref); \
        return 0; \
    } \
    if (!(sdkConf->url_)) \
        return 0; \
    auto str_ = sdkConf->url_->GetSockAddrString(); \
    if (capa_ < (str_.length()+1) || str_.length() == 0) \
        return 0; \
    memcpy(val_, str_.c_str(), str_.length()+1);\
    return str_.length()

PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_server_address(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val)
{
    SdkConfigCopyUrlToOutput(capa, val, ServerAddress);
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_token(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val)
{
    SdkConfigCopyStringToOutput(capa, val, Token);
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_type(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val)
{
    SdkConfigCopyStringToOutput(capa, val, Mode);
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_udp_type(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val)
{
    SdkConfigCopyStringToOutput(capa, val, UdpMode);
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_tcp_forward_to(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val)
{
    SdkConfigCopyUrlToOutput(capa, val, TcpForwardTo);
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_udp_forward_to(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val)
{
    SdkConfigCopyUrlToOutput(capa, val, UdpForwardTo);
}

PINGGY_EXPORT pinggy_const_bool_t
pinggy_config_get_force(pinggy_ref_t ref)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf)
        return 0;
    return sdkConf->Force ? pinggy_true : pinggy_false;
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_argument(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val)
{
    SdkConfigCopyStringToOutput(capa, val, Argument);
}

PINGGY_EXPORT pinggy_const_bool_t
pinggy_config_get_advanced_parsing(pinggy_ref_t ref)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return pinggy_false;
    }
    return sdkConf->AdvancedParsing ? pinggy_true : pinggy_false;
}

PINGGY_EXPORT pinggy_const_bool_t
pinggy_config_get_ssl(pinggy_ref_t ref)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return pinggy_false;
    }
    return sdkConf->Ssl ? pinggy_true : pinggy_false;
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_sni_server_name(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val)
{
    SdkConfigCopyStringToOutput(capa, val, SniServerName);
}

PINGGY_EXPORT pinggy_const_bool_t
pinggy_config_get_insecure(pinggy_ref_t ref)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return pinggy_false;
    }
    return sdkConf->Insecure ? pinggy_true : pinggy_false;
}

#undef SdkConfigCopyStringToOutput
#undef SdkConfigCopyUrlToOutput

//========================================

PINGGY_EXPORT pinggy_ref_t
pinggy_tunnel_initiate(pinggy_ref_t ref)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return INVALID_PINGGY_REF;
    }
    auto ev = NewApiEventHandlerPtr();
    auto sdk = sdk::NewSdkPtr(sdkConf, ev);
    auto sdkRef = getRef(sdk);
    ev->sdk = sdkRef;
    return sdkRef;
}

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_start(pinggy_ref_t ref)
{
    auto sdk =  getSdk(ref);
    if (sdk == nullptr) {
        LOGE("null sdk");
        return pinggy_false;
    }
    try {
        if (!sdk->Start()) {
            LOGI("Didn't work");
            return pinggy_false;
        }
    } catch (const std::exception &e) {
        if (exception_callback) {
            exception_callback("CPP exception:", e.what());
        } else {
            LOGE("No exception handler found");
        }
        return pinggy_false;
    }

    return pinggy_true;
}

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_connect(pinggy_ref_t ref)
{
    auto sdk =  getSdk(ref);
    if (sdk == nullptr) {
        LOGE("null sdk");
        return pinggy_false;
    }

    try {
        if (!sdk->Connect()) {
            LOGI("Didn't work");
            return pinggy_false;
        }
    } catch (const std::exception &e) {
        if (exception_callback) {
            exception_callback("CPP exception:", e.what());
        } else {
            LOGE("No exception handler found");
        }
        return pinggy_false;
    }

    return pinggy_true;
}

PINGGY_EXPORT pinggy_int_t
pinggy_tunnel_resume(pinggy_ref_t ref)
{
    auto sdk =  getSdk(ref);
    if (sdk == nullptr) {
        LOGE("null sdk");
        return 0;
    }
    try {
        return sdk->ResumeTunnel();
    } catch (const std::exception &e) {
        if (exception_callback) {
            exception_callback("CPP exception:", e.what());
        } else {
            LOGE("No exception handler found");
        }
        return -1;
    }
}

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_stop(pinggy_ref_t ref)
{
    auto sdk =  getSdk(ref);
    if (sdk == nullptr) {
        LOGE("null sdk");
        return pinggy_false;
    }
    try {
        return sdk->Stop();
    } catch (const std::exception &e) {
        if (exception_callback) {
            exception_callback("CPP exception:", e.what());
        } else {
            LOGE("No exception handler found");
        }
        return pinggy_false;
    }
}

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_is_active(pinggy_ref_t ref)
{
    auto sdk =  getSdk(ref);
    if (sdk == nullptr) {
        LOGE("null sdk");
        return pinggy_false;
    }
    try {
        return sdk->IsTunnelActive();
    } catch (const std::exception &e) {
        if (exception_callback) {
            exception_callback("CPP exception:", e.what());
        } else {
            LOGE("No exception handler found");
        }
        return pinggy_false;
    }
}

PINGGY_EXPORT pinggy_uint16_t
pinggy_tunnel_start_web_debugging(pinggy_ref_t ref, pinggy_uint16_t port)
{
    auto sdk =  getSdk(ref);
    if (sdk == nullptr) {
        LOGE("null sdk");
        return 0;
    }
    try {
        return sdk->StartWebDebugging(port);
    } catch (const std::exception &e) {
        if (exception_callback) {
            exception_callback("CPP exception:", e.what());
        } else {
            LOGE("No exception handler found");
        }
        return 0;
    }
}

PINGGY_EXPORT pinggy_void_t
pinggy_tunnel_request_primary_forwarding(pinggy_ref_t ref)
{
    auto sdk =  getSdk(ref);
    if (sdk == nullptr) {
        LOGE("null sdk");
        return;
    }
    try {
        return sdk->RequestPrimaryRemoteForwarding();
    } catch (const std::exception &e) {
        if (exception_callback) {
            exception_callback("CPP exception:", e.what());
        } else {
            LOGE("No exception handler found");
        }
        return;
    }
}

PINGGY_EXPORT pinggy_void_t
pinggy_tunnel_request_additional_forwarding(pinggy_ref_t ref, pinggy_const_char_p_t bindingAddr, pinggy_const_char_p_t forwardTo)
{
    auto sdk =  getSdk(ref);
    if (sdk == nullptr) {
        LOGE("null sdk");
        return;
    }
    try {
        return sdk->RequestAdditionalRemoteForwarding(NewUrlPtr(EmptyStringIfNull(bindingAddr)), NewUrlPtr(EmptyStringIfNull(forwardTo)));
    } catch (const std::exception &e) {
        if (exception_callback) {
            exception_callback("CPP exception:", e.what());
        } else {
            LOGE("No exception handler found");
        }
        return;
    }
}

//===============================
#define GetEventHandlerFromSdkRef(sdkRef, aev)                  \
    auto sdk =  getSdk(sdkRef);                                 \
    if (sdk == nullptr) {                                       \
        LOGE("null sdk");                                       \
        return pinggy_false;                                    \
    }                                                           \
    auto ev = sdk->GetSdkEventHandler();                        \
    if (ev == nullptr) {                                        \
        LOGE("no event handler found");                         \
        return pinggy_false;                                    \
    }                                                           \
    auto aev = ev->DynamicPointerCast<ApiEventHandler>();       \
    if (aev == nullptr) {                                       \
        LOGE("Unknown event handler");                          \
        return pinggy_false;                                    \
    }

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_connected_callback(pinggy_ref_t sdkRef, pinggy_on_connected_cb_t connected, pinggy_void_p_t user_data)
{
    GetEventHandlerFromSdkRef(sdkRef, aev);
    aev->onConnectedCB = connected;
    aev->onConnectedUserData = user_data;
    return pinggy_true;
}

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_authenticated_callback(pinggy_ref_t sdkRef, pinggy_on_authenticated_cb_t authenticated, pinggy_void_p_t user_data)
{
    GetEventHandlerFromSdkRef(sdkRef, aev);
    aev->onAuthenticatedCB = authenticated;
    aev->onAuthenticatedUserData = user_data;
    return pinggy_true;
}

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_authentication_failed_callback(pinggy_ref_t sdkRef, pinggy_on_authentication_failed_cb_t authenticationFailed, pinggy_void_p_t user_data)
{
    GetEventHandlerFromSdkRef(sdkRef, aev);
    aev->onAuthenticationFailedCB = authenticationFailed;
    aev->onAuthenticationFailedUserData = user_data;
    return pinggy_true;
}

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_primary_forwarding_succeeded_callback(pinggy_ref_t sdkRef, pinggy_on_primary_forwarding_succeeded_cb_t tunnel_initiated, pinggy_void_p_t user_data)
{
    GetEventHandlerFromSdkRef(sdkRef, aev);
    aev->onPrimaryForwardingSucceededCB = tunnel_initiated;
    aev->onPrimaryForwardingSucceededUserData = user_data;
    return pinggy_true;
}

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_primary_forwarding_failed_callback(pinggy_ref_t sdkRef, pinggy_on_primary_forwarding_failed_cb_t tunnel_initiation_failed, pinggy_void_p_t user_data)
{
    GetEventHandlerFromSdkRef(sdkRef, aev);
    aev->onPrimaryForwardingFailedCB = tunnel_initiation_failed;
    aev->onPrimaryForwardingFailedUserData = user_data;
    return pinggy_true;
}

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_additional_forwarding_succeeded_callback(pinggy_ref_t sdkRef, pinggy_on_additional_forwarding_succeeded_cb_t reverseForwardingSucceeded, pinggy_void_p_t user_data)
{
    GetEventHandlerFromSdkRef(sdkRef, aev);
    aev->onAdditionalForwardingSucceededCB = reverseForwardingSucceeded;
    aev->onAdditionalForwardingSucceededUserData = user_data;
    return pinggy_true;
}

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_additional_forwarding_failed_callback(pinggy_ref_t sdkRef, pinggy_on_additional_forwarding_failed_cb_t reverseForwardingFailed, pinggy_void_p_t user_data)
{
    GetEventHandlerFromSdkRef(sdkRef, aev);
    aev->onAdditionalForwardingFailedCB = reverseForwardingFailed;
    aev->onAdditionalForwardingFailedUserData = user_data;
    return pinggy_true;
}

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_disconnected_callback(pinggy_ref_t sdkRef, pinggy_on_disconnected_cb_t disconnected, pinggy_void_p_t user_data)
{
    GetEventHandlerFromSdkRef(sdkRef, aev);
    aev->onDisconnectedCB = disconnected;
    aev->onDisconnectedUserData = user_data;
    return pinggy_true;
}

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_tunnel_error_callback(pinggy_ref_t sdkRef, pinggy_on_tunnel_error_cb_t error, pinggy_void_p_t user_data)
{
    GetEventHandlerFromSdkRef(sdkRef, aev);
    aev->onErrorCB = error;
    aev->onDisconnectedUserData = user_data;
    return pinggy_true;
}

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_new_channel_callback(pinggy_ref_t sdkRef, pinggy_on_new_channel_cb_t new_channel, pinggy_void_p_t user_data)
{
    GetEventHandlerFromSdkRef(sdkRef, aev);
    aev->onNewChannelCB = new_channel;
    aev->onNewChannelUserData = user_data;
    return pinggy_true;
}

#undef GetEventHandlerFromSdkRef


//========================================
//          Channel Functions
//========================================
#define GetEventHandlerFromChannelRef(channelRef, cev)           \
    auto channel = getSdkChannelWraper(channelRef);              \
    if (channel == nullptr) {                                    \
        LOGE("null channel");                                    \
        return pinggy_false;                                     \
    }                                                            \
    auto ev = channel->GetEventHandler();                        \
    if (ev == nullptr) {                                         \
        LOGE("no event handler found");                          \
        return pinggy_false;                                     \
    }                                                            \
    auto cev = ev->DynamicPointerCast<ApiChannelEventHandler>(); \
    if (cev == nullptr) {                                        \
        LOGE("Unknown event handler");                           \
        return pinggy_false;                                     \
    }

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_set_data_received_callback(pinggy_ref_t channelRef, pinggy_channel_data_received_cb_t cb, pinggy_void_p_t user_data)
{
    GetEventHandlerFromChannelRef(channelRef, cev);
    cev->channelDataReceivedCB = cb;
    cev->channelDataReceivedUserData = user_data;
    return pinggy_true;
}

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_set_ready_to_send_callback(pinggy_ref_t channelRef, pinggy_channel_readyto_send_cb_t cb, pinggy_void_p_t user_data)
{
    GetEventHandlerFromChannelRef(channelRef, cev);
    cev->channelReadyToSendCB = cb;
    cev->channelReadyToSendUserData = user_data;
    return pinggy_true;
}

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_set_error_callback(pinggy_ref_t channelRef, pinggy_channel_error_cb_t cb, pinggy_void_p_t user_data)
{
    GetEventHandlerFromChannelRef(channelRef, cev);
    cev->channelErrorCB = cb;
    cev->channelErrorUserData = user_data;
    return pinggy_true;
}

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_set_cleanup_callback(pinggy_ref_t channelRef, pinggy_channel_cleanup_cb_t cb, pinggy_void_p_t user_data)
{
    GetEventHandlerFromChannelRef(channelRef, cev);
    cev->channelCleanupCB = cb;
    cev->channelCleanupUserData = user_data;
    return pinggy_true;
}

//====

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_accept(pinggy_ref_t channelRef)
{
    auto channel = getSdkChannelWraper(channelRef);
    if (!channel)
        return pinggy_false;

    return channel->Accept() ? pinggy_true : pinggy_false;
}

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_reject(pinggy_ref_t channelRef, pinggy_char_p_t reason)
{
    auto channel = getSdkChannelWraper(channelRef);
    if (!channel)
        return pinggy_false;

    pinggy_free_ref(channelRef);

    return channel->Reject(EmptyStringIfNull(reason)) ? pinggy_true : pinggy_false;
}

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_close(pinggy_ref_t channelRef)
{
    auto channel = getSdkChannelWraper(channelRef);
    if (!channel)
        return pinggy_false;

    return channel->Close() ? pinggy_true : pinggy_false;
}

PINGGY_EXPORT pinggy_raw_len_t
pinggy_tunnel_channel_send(pinggy_ref_t channelRef, pinggy_const_char_p_t data, pinggy_raw_len_t len)
{
    auto channel = getSdkChannelWraper(channelRef);
    if (!channel)
        return -1; //TODO raise exception

    if (len <= 0)
        return len;

    auto rawData = NewRawDataPtr(data, len);

    return (pinggy_raw_len_t)channel->Send(rawData);
}

PINGGY_EXPORT pinggy_raw_len_t
pinggy_tunnel_channel_recv(pinggy_ref_t channelRef, pinggy_char_p_t data, pinggy_raw_len_t len)
{
    auto channel = getSdkChannelWraper(channelRef);
    if (!channel)
        return -1; //TODO raise exception

    if (len <= 0)
        return len;

    auto [a, b] = channel->Recv(len);
    auto ret = a;
    auto rawData = b;
    if (ret <= 0) {
        return ret;
    }

    memcpy(data, rawData->GetData(), rawData->Len);

    return rawData->Len;
}

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_have_data_to_recv(pinggy_ref_t channelRef)
{
    auto channel = getSdkChannelWraper(channelRef);
    if (!channel)
        return pinggy_false;

    return channel->HaveDataToRead() ? pinggy_true : pinggy_false;
}

PINGGY_EXPORT pinggy_uint32_t
pinggy_tunnel_channel_have_buffer_to_send(pinggy_ref_t channelRef)
{
    auto channel = getSdkChannelWraper(channelRef);
    if (!channel)
        return pinggy_false;

    return channel->HaveBufferToWrite() ? pinggy_true : pinggy_false;
}

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_is_connected(pinggy_ref_t channelRef)
{
    auto channel = getSdkChannelWraper(channelRef);
    if (!channel)
        return pinggy_false;

    return channel->IsConnected() ? pinggy_true : pinggy_false;
}

PINGGY_EXPORT pinggy_uint32_t
pinggy_tunnel_channel_get_type(pinggy_ref_t channelRef)
{
    auto channel = getSdkChannelWraper(channelRef);
    if (!channel)
        return 0;

    return channel->GetType();
}

PINGGY_EXPORT pinggy_uint16_t
pinggy_tunnel_channel_get_dest_port(pinggy_ref_t channelRef)
{
    auto channel = getSdkChannelWraper(channelRef);
    if (!channel)
        return 0;

    return channel->GetDestPort();
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_tunnel_channel_get_dest_host(pinggy_ref_t channelRef, pinggy_capa_t capa, pinggy_char_p_t val)
{
    auto channel = getSdkChannelWraper(channelRef);
    if (!channel)
        return 0;

    auto str = channel->GetDestHost();
    CopyStringToOutput(capa, val, str);
    return str.length();
}

PINGGY_EXPORT pinggy_uint16_t
pinggy_tunnel_channel_get_src_port(pinggy_ref_t channelRef)
{
    auto channel = getSdkChannelWraper(channelRef);
    if (!channel)
        return 0;

    return channel->GetSrcPort();
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_tunnel_channel_get_src_host(pinggy_ref_t channelRef, pinggy_capa_t capa, pinggy_char_p_t val)
{
    auto channel = getSdkChannelWraper(channelRef);
    if (!channel)
        return 0;

    auto str = channel->GetSrcHost();
    CopyStringToOutput(capa, val, str);
    return str.length();
}

//==============================

//==============================================================
PINGGY_EXPORT pinggy_const_int_t
pinggy_version(pinggy_capa_t capa, pinggy_char_p_t val)
{
    tString str = std::to_string(PinggyVersionMajor) + "." + std::to_string(PinggyVersionMinor) + "." + std::to_string(PinggyVersionPatch);
    CopyStringToOutput(capa, val, str);
    return str.length();
}

#define DEFINE_CONFIG_GET_FUNC(funcname, macro) \
PINGGY_EXPORT pinggy_const_int_t \
funcname(pinggy_capa_t capa, pinggy_char_p_t val) \
{ \
    tString str = macro; \
    CopyStringToOutput(capa, val, str); \
    return str.length(); \
}

DEFINE_CONFIG_GET_FUNC(pinggy_git_commit, PINGGY_GIT_COMMIT_ID);
DEFINE_CONFIG_GET_FUNC(pinggy_build_timestamp, PINGGY_BUILD_TIMESTAMP);
DEFINE_CONFIG_GET_FUNC(pinggy_libc_version, PINGGY_LIBC_VERSION);
DEFINE_CONFIG_GET_FUNC(pinggy_build_os, PINGGY_BUILD_OS);
#undef DEFINE_CONFIG_GET_FUNC
//==============================================================

#ifdef __cplusplus
}
#endif //__cplusplus
