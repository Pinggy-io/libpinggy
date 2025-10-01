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
#include <shared_mutex>
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
std::shared_mutex globalMutex;
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
pinggy_is_interrupted()
{
    return app_get_errno() == EINTR;
}
//==============================================================

static pinggy_ref_t
getRef(pinggy::VoidPtr ptr)
{
    std::unique_lock<std::shared_mutex> lock(globalMutex);

    static pinggy_ref_t curRef= 2;
    curRef += 1;

    pinggyReferenceMap[curRef] = ptr;
    return curRef;
}

static pinggy::VoidPtr
getObj(pinggy_ref_t ref)
{
    std::shared_lock<std::shared_mutex> lock(globalMutex);
    if (pinggyReferenceMap.find(ref) == pinggyReferenceMap.end()) {
        return nullptr;
    }
    return pinggyReferenceMap[ref];
}

static pinggy_bool_t
removeRef(pinggy_ref_t ref) {
    std::unique_lock<std::shared_mutex> lock(globalMutex);
    if (pinggyReferenceMap.find(ref) != pinggyReferenceMap.end()) {
        // LOGI("Freeing ref", ref);
        pinggyReferenceMap.erase(ref);
        return pinggy_true;
    } else {
        LOGE("Invalid ref", ref);
        return pinggy_false;
    }
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
    pinggy_channel_on_data_received_cb_t
                                channelDataReceivedCB;
    pinggy_channel_on_readyto_send_cb_t
                                channelReadyToSendCB;
    pinggy_channel_on_error_cb_t   channelErrorCB;
    pinggy_channel_on_cleanup_cb_t channelCleanupCB;

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
    ChannelDataReceived(sdk::SdkChannelWraperPtr) override
    {
        if (!channelDataReceivedCB) return;
        channelDataReceivedCB(channelDataReceivedUserData, channelRef);
    }

    virtual pinggy_void_t
    ChannelReadyToSend(sdk::SdkChannelWraperPtr, tUint32 bufferLen) override
    {
        if (!channelReadyToSendCB) return;
        channelReadyToSendCB(channelReadyToSendUserData, channelRef, bufferLen);
    }

    virtual pinggy_void_t
    ChannelError(sdk::SdkChannelWraperPtr, tString errorText) override
    {
        if (!channelErrorCB) return;
        channelErrorCB(channelErrorUserData, channelRef, errorText.c_str(), errorText.length());
    }

    virtual pinggy_void_t
    ChannelCleanup(sdk::SdkChannelWraperPtr) override
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
    pinggy_on_forwarding_succeeded_cb_t
                                onForwardingSucceededCB;
    pinggy_on_forwarding_failed_cb_t
                                onForwardingFailedCB;
    pinggy_on_additional_forwarding_succeeded_cb_t
                                onAdditionalForwardingSucceededCB;
    pinggy_on_additional_forwarding_failed_cb_t
                                onAdditionalForwardingFailedCB;
    pinggy_on_forwarding_changed_cb_t
                                onForwardingChangedCB;
    pinggy_on_disconnected_cb_t onDisconnectedCB;
    pinggy_on_will_reconnect_cb_t
                                onWillReconnectCB;
    pinggy_on_reconnecting_cb_t onReconnectingCB;
    pinggy_on_reconnection_completed_cb_t
                                onReconnectionCompletedCB;
    pinggy_on_reconnection_failed_cb_t
                                onReconnectionFailedCB;
    pinggy_on_tunnel_error_cb_t onErrorCB;
    pinggy_on_new_channel_cb_t  onNewChannelCB;
    pinggy_on_usage_update_cb_t onUsageUpdateCB;

    pinggy_void_p_t             onConnectedUserData;
    pinggy_void_p_t             onAuthenticatedUserData;
    pinggy_void_p_t             onAuthenticationFailedUserData;
    pinggy_void_p_t             onForwardingSucceededUserData;
    pinggy_void_p_t             onForwardingFailedUserData;
    pinggy_void_p_t             onAdditionalForwardingSucceededUserData;
    pinggy_void_p_t             onAdditionalForwardingFailedUserData;
    pinggy_void_p_t             onForwardingChangedUserData;
    pinggy_void_p_t             onDisconnectedUserData;
    pinggy_void_p_t             onWillReconnectUserData;
    pinggy_void_p_t             onReconnectingUserData;
    pinggy_void_p_t             onReconnectionCompletedUserData;
    pinggy_void_p_t             onReconnectionFailedUserData;
    pinggy_void_p_t             onErrorUserData;
    pinggy_void_p_t             onNewChannelUserData;
    pinggy_void_p_t             onUsageUpdateUserData;

    pinggy_ref_t                sdk;

    ApiEventHandler():
                        onConnectedCB(NULL),
                        onAuthenticatedCB(NULL),
                        onAuthenticationFailedCB(NULL),
                        onForwardingSucceededCB(NULL),
                        onForwardingFailedCB(NULL),
                        onAdditionalForwardingSucceededCB(NULL),
                        onAdditionalForwardingFailedCB(NULL),
                        onForwardingChangedCB(NULL),
                        onDisconnectedCB(NULL),
                        onWillReconnectCB(NULL),
                        onReconnectingCB(NULL),
                        onReconnectionCompletedCB(NULL),
                        onReconnectionFailedCB(NULL),
                        onNewChannelCB(NULL),
                        onUsageUpdateCB(NULL),

                        onAuthenticatedUserData(NULL),
                        onAuthenticationFailedUserData(NULL),
                        onForwardingSucceededUserData(NULL),
                        onForwardingFailedUserData(NULL),
                        onAdditionalForwardingSucceededUserData(NULL),
                        onAdditionalForwardingFailedUserData(NULL),
                        onForwardingChangedUserData(NULL),
                        onDisconnectedUserData(NULL),
                        onWillReconnectUserData(NULL),
                        onReconnectingUserData(NULL),
                        onReconnectionCompletedUserData(NULL),
                        onReconnectionFailedUserData(NULL),
                        onErrorUserData(NULL),
                        onNewChannelUserData(NULL),
                        onUsageUpdateUserData(NULL),
                        sdk(INVALID_PINGGY_REF) {}

    virtual ~ApiEventHandler()  { }

#define GetCStringArray(cVec,vec)                               \
        auto cVec = new pinggy_char_p_t[vec.size()+2];          \
        for (size_t i = 0; i < vec.size(); ++i) {               \
            cVec[i] = new pinggy_char_t[vec[i].length() + 3];   \
            strncpy(cVec[i], vec[i].c_str(), vec[i].length());  \
            cVec[i][vec[i].length()] = 0;                       \
        }
#define ReleaseCStringArray(cVec, vec)                          \
        for (size_t i = 0; i < vec.size(); ++i)                 \
            delete[] cVec[i];                                   \
        delete[] cVec;                                          \

    virtual pinggy_void_t
    OnConnected() override
    {
        if (onConnectedCB) onConnectedCB(onConnectedUserData, sdk);
    }
    virtual pinggy_void_t
    OnAuthenticated() override
    {
        if (onAuthenticatedCB) onAuthenticatedCB(onAuthenticatedUserData, sdk);
    }
    virtual pinggy_void_t
    OnAuthenticationFailed(std::vector<tString> why) override
    {
        if (!onAuthenticationFailedCB) return;
        GetCStringArray(cWhy, why)
        onAuthenticationFailedCB(onAuthenticationFailedUserData, sdk, why.size(), cWhy);
        ReleaseCStringArray(cWhy, why);
    }
    virtual pinggy_void_t
    OnForwardingSucceeded(std::vector<tString> urls) override
    {
        if (!onForwardingSucceededCB) {
            LOGD("onForwardingSucceededCB does not exists");
            return;
        }
        GetCStringArray(cUrls, urls);
        onForwardingSucceededCB(onForwardingSucceededUserData, sdk, urls.size(), cUrls);
        ReleaseCStringArray(cUrls, urls);
    }
    virtual pinggy_void_t
    OnForwardingFailed(tString message) override
    {
        if (!onForwardingFailedCB) return;
        onForwardingFailedCB(onForwardingFailedUserData, sdk, message.c_str());
    }
    virtual pinggy_void_t
    OnAdditionalForwardingSucceeded(tString bindAddress, tString forwardTo, tString forwardingType) override
    {
        if (!onAdditionalForwardingSucceededCB) return;
        onAdditionalForwardingSucceededCB(onAdditionalForwardingSucceededUserData, sdk, bindAddress.c_str(), forwardTo.c_str(), forwardingType.c_str());
    }
    virtual pinggy_void_t
    OnAdditionalForwardingFailed(tString bindAddress, tString forwardTo, tString forwardingType, tString error) override
    {
        if (!onAdditionalForwardingFailedCB) return;
        auto cError = error;
        onAdditionalForwardingFailedCB(onAdditionalForwardingFailedUserData, sdk, bindAddress.c_str(), forwardTo.c_str(), forwardingType.c_str(), cError.c_str());
    }
    virtual pinggy_void_t
    OnForwardingChanged(tString changedMap) override
    {
        if (!onForwardingChangedCB) return;
        onForwardingChangedCB(onForwardingChangedUserData, sdk, changedMap.c_str());
    }
    virtual pinggy_void_t
    OnDisconnected(tString error, std::vector<tString> messages) override
    {
        if (!onDisconnectedCB) return;
        GetCStringArray(cMsg, messages);
        onDisconnectedCB(onDisconnectedUserData, sdk, error.c_str(), messages.size(), cMsg);
        ReleaseCStringArray(cMsg, messages);
    }
    virtual pinggy_void_t
    OnWillReconnect(tString error, std::vector<tString> messages) override
    {
        if (!onWillReconnectCB) return;
        GetCStringArray(cMsg, messages);
        onWillReconnectCB(onWillReconnectUserData, sdk, error.c_str(), messages.size(), cMsg);
        ReleaseCStringArray(cMsg, messages);
    }
    virtual pinggy_void_t
    OnReconnecting(tUint16 cnt) override
    {
        if (!onReconnectingCB) return;
        onReconnectingCB(onReconnectingUserData, sdk, cnt);
    }
    virtual pinggy_void_t
    OnReconnectionCompleted(std::vector<tString> urls) override
    {
        if (!onReconnectionCompletedCB) return;

        GetCStringArray(cUrls, urls);
        onReconnectionCompletedCB(onReconnectionCompletedUserData, sdk, urls.size(), cUrls);
        ReleaseCStringArray(cUrls, urls);
    }
    virtual pinggy_void_t
    OnReconnectionFailed(tUint16 cnt) override
    {
        if (!onReconnectionFailedCB) return;
        onReconnectionFailedCB(onReconnectionFailedUserData, sdk, cnt);
    }
    virtual pinggy_void_t
    OnHandleError(tUint32 errorNo, tString what, tBool recoverable) override
    {
        if (!onErrorCB) return;
        onErrorCB(onErrorUserData, sdk, errorNo, what.c_str(), recoverable?1:0);
    }
    virtual bool
    OnNewVisitorConnectionReceived(sdk::SdkChannelWraperPtr channel) override
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
    virtual pinggy_void_t
    OnUsageUpdate(tString update) override
    {
        if (!onUsageUpdateCB) return;
        onUsageUpdateCB(onUsageUpdateUserData, sdk, update.c_str());
    }

#undef GetCStringArray
#undef ReleaseCStringArray
};
DefineMakeSharedPtr(ApiEventHandler);

//==============================================================

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus



#define ExpectException(x)                                              \
    do {                                                                \
        try{                                                            \
            x                                                           \
        } catch (const std::exception &e) {                             \
            if (exception_callback) {                                   \
                exception_callback("CPP exception:", e.what());         \
            } else {                                                    \
                LOGE("No exception handler found");                     \
            }                                                           \
        }                                                               \
    } while(0)

#define CopyStringToOutputLen(capa_, val_, str_, len_)                  \
        ExpectException(                                                \
            auto str = str_;                                            \
            if (len_) *len_ = str.length()+2;                           \
            if (!val_) return 0;                                        \
            if (capa_ < (str.length()+1) || str.length() == 0)          \
                return 0;                                               \
            memcpy(val_, str.c_str(), str.length()+1);                  \
            return str.length()+1;                                      \
        );                                                              \
        return 0

static pinggy_on_raise_exception_cb_t exception_callback = NULL;

PINGGY_EXPORT pinggy_void_t
pinggy_set_on_exception_callback(pinggy_on_raise_exception_cb_t cb)
{
    exception_callback = cb;
}

PINGGY_EXPORT pinggy_bool_t
pinggy_free_ref(pinggy_ref_t ref)
{
    return removeRef(ref);
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
    ExpectException(
        sdkConf->SetServerAddress(NewUrlPtr(EmptyStringIfNull(server_address), 443));
    );
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_token(pinggy_ref_t ref, pinggy_char_p_t token)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    ExpectException(
        sdkConf->SetToken(EmptyStringIfNull(token));
    );
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_add_forwarding(pinggy_ref_t ref, pinggy_char_p_t forwarding_type, pinggy_char_p_t binding_url, pinggy_char_p_t forward_to)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    ExpectException(
        sdkConf->AddForwarding(EmptyStringIfNull(forwarding_type), EmptyStringIfNull(binding_url), EmptyStringIfNull(forward_to));
    );
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_add_forwarding_simple(pinggy_ref_t ref, pinggy_char_p_t forward_to)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    ExpectException(
        sdkConf->AddForwarding(EmptyStringIfNull(forward_to));
    );
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_forwardings(pinggy_ref_t ref, pinggy_char_p_t forwardings)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    ExpectException(
        sdkConf->SetForwarding(EmptyStringIfNull(forwardings));
    );
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_reset_forwardings(pinggy_ref_t ref)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    ExpectException(
        sdkConf->ResetForwardings();
    );
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_force(pinggy_ref_t ref, pinggy_bool_t force)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    ExpectException(
        sdkConf->SetForce(force);
    );
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_argument(pinggy_ref_t ref, pinggy_char_p_t argument)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    ExpectException(
        sdkConf->SetArguments(EmptyStringIfNull(argument));
    );
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_advanced_parsing(pinggy_ref_t ref, pinggy_bool_t advanced_parsing)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    ExpectException(
        sdkConf->SetAdvancedParsing(advanced_parsing);
    );
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_ssl(pinggy_ref_t ref, pinggy_bool_t ssl)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    ExpectException(
        sdkConf->SetSsl(ssl);
    );
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_auto_reconnect(pinggy_ref_t ref, pinggy_bool_t enable)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    ExpectException(
        sdkConf->SetAutoReconnect(enable);
    );
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_sni_server_name(pinggy_ref_t ref, pinggy_char_p_t sni_server_name)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    ExpectException(
        sdkConf->SetSniServerName(EmptyStringIfNull(sni_server_name));
    );
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_insecure(pinggy_ref_t ref, pinggy_bool_t insecure)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    ExpectException(
        sdkConf->SetInsecure(insecure);
    );
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_max_reconnect_attempts(pinggy_ref_t ref, pinggy_uint16_t attempts)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    ExpectException(
        sdkConf->SetMaxReconnectAttempts(attempts);
    );
}
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_reconnect_interval(pinggy_ref_t ref, pinggy_uint16_t interval)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    ExpectException(
        sdkConf->SetAutoReconnectInterval(interval);
    );
}
//======

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_header_manipulations(pinggy_ref_t ref, pinggy_const_char_p_t header_manipulations)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    ExpectException(
        sdkConf->SetHeaderManipulations(tString(header_manipulations));
    );
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_basic_auths(pinggy_ref_t ref, pinggy_const_char_p_t basic_auths)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    ExpectException(
        sdkConf->SetBasicAuths(tString(basic_auths));
    );
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_bearer_token_auths(pinggy_ref_t ref, pinggy_const_char_p_t bearer_token_auths)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    ExpectException(
        sdkConf->SetBearerTokenAuths(tString(bearer_token_auths));
    );
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_ip_white_list(pinggy_ref_t ref, pinggy_const_char_p_t ip_white_list)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    ExpectException(
        sdkConf->SetIpWhiteList(tString(ip_white_list));
    );
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_reverse_proxy(pinggy_ref_t ref, pinggy_bool_t reverse_proxy)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    ExpectException(
        sdkConf->SetReverseProxy(reverse_proxy == pinggy_true ? true : false);
    );
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_x_forwarded_for(pinggy_ref_t ref, pinggy_bool_t x_forwarded_for)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    ExpectException(
        sdkConf->SetXForwardedFor(x_forwarded_for == pinggy_true ? true : false);
    );
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_https_only(pinggy_ref_t ref, pinggy_bool_t https_only)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    ExpectException(
        sdkConf->SetHttpsOnly(https_only == pinggy_true ? true : false);
    );
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_original_request_url(pinggy_ref_t ref, pinggy_bool_t original_request_url)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    ExpectException(
        sdkConf->SetOriginalRequestUrl(original_request_url == pinggy_true ? true : false);
    );
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_allow_preflight(pinggy_ref_t ref, pinggy_bool_t allow_preflight)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    ExpectException(
        sdkConf->SetAllowPreflight(allow_preflight == pinggy_true ? true : false);
    );
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_no_reverse_proxy(pinggy_ref_t ref, pinggy_bool_t no_reverse_proxy)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    ExpectException(
        sdkConf->SetNoReverseProxy(no_reverse_proxy == pinggy_true ? true : false);
    );
}

PINGGY_EXPORT pinggy_void_t
pinggy_config_set_local_server_tls(pinggy_ref_t ref, pinggy_const_char_p_t local_server_tls)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return;
    }
    ExpectException(
        sdkConf->SetLocalServerTls(tString(local_server_tls));
    );
}



#define SdkConfigCopyStringToOutputLen(capa_, val_, str_, len_)                 \
    do {                                                                        \
        if (len_) *len_ = 0;                                                    \
        auto sdkConf = getSDKConfig(ref);                                       \
        if (!sdkConf) {                                                         \
            LOGE("No sdkConf found for the ref:", ref);                         \
            return 0;                                                           \
        }                                                                       \
        ExpectException(                                                        \
            auto str = sdkConf->str_;                                           \
            if (len_) *len_ = str.length()+2;                                   \
            if (!val_) return 0;                                                \
            if (capa_ < (str.length()+1) || str.length() == 0)                  \
                return 0;                                                       \
            memcpy(val_, str.c_str(), str.length()+1);                          \
            return str.length();                                                \
        );                                                                      \
        return 0;                                                               \
    } while(0)

#define SdkConfigCopyUrlToOutputLen(capa_, val_, url_, len_)                    \
    do {                                                                        \
        if (len_) *len_ = 0;                                                    \
        auto sdkConf = getSDKConfig(ref);                                       \
        if (!sdkConf) {                                                         \
            LOGE("No sdkConf found for the ref:", ref);                         \
            return 0;                                                           \
        }                                                                       \
        ExpectException(                                                        \
            if (!(sdkConf->url_))                                               \
                return 0;                                                       \
            auto str_ = sdkConf->url_->GetSockAddrString();                     \
            if (len_) *len_ = str_.length()+2;                                  \
            if (!val_) return 0;                                                \
            if (capa_ < (str_.length()+1) || str_.length() == 0)                \
                return 0;                                                       \
            memcpy(val_, str_.c_str(), str_.length()+1);                        \
            return str_.length();                                               \
        );                                                                      \
        return 0;                                                               \
    } while(0)

PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_server_address(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val)
{
    return pinggy_config_get_server_address_len(ref, capa, val, NULL);
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_server_address_len(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val, pinggy_capa_p_t max_len)
{
    SdkConfigCopyStringToOutputLen(capa, val, GetServerAddress(), max_len);
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_token(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val)
{
    return pinggy_config_get_token_len(ref, capa, val, NULL);
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_token_len(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val, pinggy_capa_p_t max_len)
{
    SdkConfigCopyStringToOutputLen(capa, val, GetToken(), max_len);
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_forwardings(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val)
{
    return pinggy_config_get_forwardings_len(ref, capa, val, NULL);
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_forwardings_len(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val, pinggy_capa_p_t max_len)
{
    SdkConfigCopyStringToOutputLen(capa, val, GetForwardings(), max_len);
}

PINGGY_EXPORT pinggy_const_bool_t
pinggy_config_get_force(pinggy_ref_t ref)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf)
        return 0;
    return sdkConf->IsForce() ? pinggy_true : pinggy_false;
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_argument(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val)
{
    return pinggy_config_get_argument_len(ref, capa, val, NULL);
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_argument_len(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val, pinggy_capa_p_t max_len)
{
    SdkConfigCopyStringToOutputLen(capa, val, GetArguments(), max_len);
}

PINGGY_EXPORT pinggy_const_bool_t
pinggy_config_get_advanced_parsing(pinggy_ref_t ref)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return pinggy_false;
    }
    return sdkConf->IsAdvancedParsing() ? pinggy_true : pinggy_false;
}

PINGGY_EXPORT pinggy_const_bool_t
pinggy_config_get_ssl(pinggy_ref_t ref)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return pinggy_false;
    }
    return sdkConf->IsSsl() ? pinggy_true : pinggy_false;
}

PINGGY_EXPORT pinggy_const_bool_t
pinggy_config_get_auto_reconnect(pinggy_ref_t ref)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return pinggy_false;
    }
    return sdkConf->IsAutoReconnect() ? pinggy_true : pinggy_false;
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_sni_server_name(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val)
{
    return pinggy_config_get_sni_server_name_len(ref, capa, val, NULL);
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_sni_server_name_len(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val, pinggy_capa_p_t max_len)
{
    SdkConfigCopyStringToOutputLen(capa, val, GetSniServerName(), max_len);
}

PINGGY_EXPORT pinggy_const_bool_t
pinggy_config_get_insecure(pinggy_ref_t ref)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return pinggy_false;
    }
    return sdkConf->IsInsecure() ? pinggy_true : pinggy_false;
}

PINGGY_EXPORT pinggy_uint16_t
pinggy_config_get_max_reconnect_attempts(pinggy_ref_t ref)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return pinggy_false;
    }
    return sdkConf->GetMaxReconnectAttempts();
}

PINGGY_EXPORT pinggy_uint16_t
pinggy_config_get_reconnect_interval(pinggy_ref_t ref)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return pinggy_false;
    }
    return sdkConf->GetAutoReconnectInterval();
}

//====

PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_header_manipulations(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val)
{
    return pinggy_config_get_header_manipulations_len(ref, capa, val, NULL);
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_header_manipulations_len(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val, pinggy_capa_p_t max_len)
{
    SdkConfigCopyStringToOutputLen(capa, val, GetHeaderManipulations(), max_len);
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_basic_auths(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val)
{
    return pinggy_config_get_basic_auths_len(ref, capa, val, NULL);
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_basic_auths_len(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val, pinggy_capa_p_t max_len)
{
    SdkConfigCopyStringToOutputLen(capa, val, GetBasicAuths(), max_len);
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_bearer_token_auths(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val)
{
    return pinggy_config_get_bearer_token_auths_len(ref, capa, val, NULL);
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_bearer_token_auths_len(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val, pinggy_capa_p_t max_len)
{
    SdkConfigCopyStringToOutputLen(capa, val, GetBearerTokenAuths(), max_len);
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_ip_white_list(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val)
{
    return pinggy_config_get_ip_white_list_len(ref, capa, val, NULL);
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_ip_white_list_len(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val, pinggy_capa_p_t max_len)
{
    SdkConfigCopyStringToOutputLen(capa, val, GetIpWhiteList(), max_len);
}

PINGGY_EXPORT pinggy_bool_t
pinggy_config_get_reverse_proxy(pinggy_ref_t ref)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return pinggy_false;
    }
    return sdkConf->IsReverseProxy() ? pinggy_true : pinggy_false;
}

PINGGY_EXPORT pinggy_bool_t
pinggy_config_get_x_forwarded_for(pinggy_ref_t ref)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return pinggy_false;
    }
    return sdkConf->IsXForwardedFor() ? pinggy_true : pinggy_false;
}

PINGGY_EXPORT pinggy_bool_t
pinggy_config_get_https_only(pinggy_ref_t ref)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return pinggy_false;
    }
    return sdkConf->IsHttpsOnly() ? pinggy_true : pinggy_false;
}

PINGGY_EXPORT pinggy_bool_t
pinggy_config_get_original_request_url(pinggy_ref_t ref)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return pinggy_false;
    }
    return sdkConf->IsOriginalRequestUrl() ? pinggy_true : pinggy_false;
}

PINGGY_EXPORT pinggy_bool_t
pinggy_config_get_allow_preflight(pinggy_ref_t ref)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return pinggy_false;
    }
    return sdkConf->IsAllowPreflight() ? pinggy_true : pinggy_false;
}

PINGGY_EXPORT pinggy_bool_t
pinggy_config_get_no_reverse_proxy(pinggy_ref_t ref)
{
    auto sdkConf = getSDKConfig(ref);
    if (!sdkConf) {
        LOGE("No sdkConf found for the ref:", ref);
        return pinggy_false;
    }
    return sdkConf->IsNoReverseProxy() ? pinggy_true : pinggy_false;
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_local_server_tls(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val)
{
    return pinggy_config_get_local_server_tls_len(ref, capa, val, NULL);
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_local_server_tls_len(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val, pinggy_capa_p_t max_len)
{
    SdkConfigCopyStringToOutputLen(capa, val, GetLocalServerTls(), max_len);
}

#undef SdkConfigCopyStringToOutput
#undef SdkConfigCopyUrlToOutput
#undef SdkConfigCopyStringToOutputLen
#undef SdkConfigCopyUrlToOutputLen

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

static pinggy_bool_t
pinggy_tunnel_connect_v2(pinggy_ref_t ref, pinggy_bool_t blocking)
{
    auto sdk =  getSdk(ref);
    if (sdk == nullptr) {
        LOGE("null sdk");
        return pinggy_false;
    }

    try {
        if (!sdk->Connect(blocking == pinggy_true)) {
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
    return pinggy_tunnel_connect_v2(ref, pinggy_false);
}

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_connect_blocking(pinggy_ref_t ref)
{
    return pinggy_tunnel_connect_v2(ref, pinggy_true);
}

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_resume(pinggy_ref_t ref)
{
    return pinggy_tunnel_resume_timeout(ref, -1);
}

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_resume_timeout(pinggy_ref_t ref, pinggy_int32_t timeout)
{
    auto sdk =  getSdk(ref);
    if (sdk == nullptr) {
        LOGE("null sdk");
        return pinggy_false;
    }
    try {
        return sdk->ResumeTunnel(timeout) ? pinggy_true : pinggy_false;
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

static pinggy_void_t
pinggy_tunnel_start_forwarding_v2(pinggy_ref_t ref, pinggy_bool_t blocking)
{
    auto sdk =  getSdk(ref);
    if (sdk == nullptr) {
        LOGE("null sdk");
        return;
    }
    try {
        sdk->StartForwarding(blocking==pinggy_true);
        return;
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
pinggy_tunnel_start_forwarding(pinggy_ref_t ref)
{
    pinggy_tunnel_start_forwarding_v2(ref, pinggy_false);
}

PINGGY_EXPORT pinggy_void_t
pinggy_tunnel_start_forwarding_blocking(pinggy_ref_t ref)
{
    pinggy_tunnel_start_forwarding_v2(ref, pinggy_true);
}

PINGGY_EXPORT pinggy_void_t
pinggy_tunnel_request_additional_forwarding(pinggy_ref_t ref, pinggy_const_char_p_t bindingAddr, pinggy_const_char_p_t forwardTo, pinggy_const_char_p_t forwarding_type)
{
    auto sdk =  getSdk(ref);
    if (sdk == nullptr) {
        LOGE("null sdk");
        return;
    }
    try {
        //tString forwardingType, tString bindingUrl, tString forwardTo
        return sdk->RequestAdditionalForwarding(EmptyStringIfNull(forwarding_type), EmptyStringIfNull(bindingAddr), EmptyStringIfNull(forwardTo));
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
pinggy_tunnel_start_usage_update(pinggy_ref_t ref)
{
    auto sdk =  getSdk(ref);
    if (sdk == nullptr) {
        LOGE("null sdk");
        return;
    }
    try {
        sdk->StartUsagesUpdate();
    } catch (const std::exception &e) {
        if (exception_callback) {
            exception_callback("CPP exception:", e.what());
        } else {
            LOGE("No exception handler found");
        }
    }
}

PINGGY_EXPORT pinggy_void_t
pinggy_tunnel_stop_usage_update(pinggy_ref_t ref)
{
    auto sdk =  getSdk(ref);
    if (sdk == nullptr) {
        LOGE("null sdk");
        return;
    }
    try {
        sdk->StopUsagesUpdate();
    } catch (const std::exception &e) {
        if (exception_callback) {
            exception_callback("CPP exception:", e.what());
        } else {
            LOGE("No exception handler found");
        }
    }
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_tunnel_get_current_usages(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val)
{
    return pinggy_tunnel_get_current_usages_len(ref, capa, val, NULL);
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_tunnel_get_current_usages_len(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val, pinggy_capa_p_t max_len)
{
    if (max_len) *max_len = 0;
    auto sdk =  getSdk(ref);
    if (sdk == nullptr) {
        LOGE("null sdk");
        return 0;
    }
    CopyStringToOutputLen(capa, val, sdk->GetCurrentUsages(), max_len);
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_tunnel_get_greeting_msgs(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val)
{
    return pinggy_tunnel_get_greeting_msgs_len(ref, capa, val, NULL);
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_tunnel_get_greeting_msgs_len(pinggy_ref_t ref, pinggy_capa_t capa, pinggy_char_p_t val, pinggy_capa_p_t max_len)
{
    if (max_len) *max_len = 0;
    auto sdk =  getSdk(ref);
    if (sdk == nullptr) {
        LOGE("null sdk");
        return 0;
    }
    CopyStringToOutputLen(capa, val, sdk->GetGreetingMsg(), max_len);
}

PINGGY_EXPORT pinggy_uint16_t
pinggy_tunnel_get_webdebugging_port(pinggy_ref_t ref)
{
    auto sdk =  getSdk(ref);
    if (sdk == nullptr) {
        LOGE("null sdk");
        return 0;
    }
    try {
        return sdk->GetWebDebugListeningPort();
    } catch (const std::exception &e) {
        if (exception_callback) {
            exception_callback("CPP exception:", e.what());
        } else {
            LOGE("No exception handler found");
        }
    }
    return 0;
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
pinggy_tunnel_set_on_forwarding_succeeded_callback(pinggy_ref_t sdkRef, pinggy_on_forwarding_succeeded_cb_t tunnel_initiated, pinggy_void_p_t user_data)
{
    GetEventHandlerFromSdkRef(sdkRef, aev);
    aev->onForwardingSucceededCB = tunnel_initiated;
    aev->onForwardingSucceededUserData = user_data;
    return pinggy_true;
}

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_forwarding_failed_callback(pinggy_ref_t sdkRef, pinggy_on_forwarding_failed_cb_t tunnel_initiation_failed, pinggy_void_p_t user_data)
{
    GetEventHandlerFromSdkRef(sdkRef, aev);
    aev->onForwardingFailedCB = tunnel_initiation_failed;
    aev->onForwardingFailedUserData = user_data;
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
pinggy_tunnel_set_on_forwarding_changed_callback(pinggy_ref_t sdkRef, pinggy_on_forwarding_changed_cb_t forwarding_changed, pinggy_void_p_t user_data)
{
    GetEventHandlerFromSdkRef(sdkRef, aev);
    aev->onForwardingChangedCB = forwarding_changed;
    aev->onForwardingChangedUserData = user_data;
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
pinggy_tunnel_set_on_will_reconnect_callback(pinggy_ref_t sdkRef, pinggy_on_will_reconnect_cb_t auto_reconnect, pinggy_void_p_t user_data)
{
    GetEventHandlerFromSdkRef(sdkRef, aev);
    aev->onWillReconnectCB = auto_reconnect;
    aev->onWillReconnectUserData = user_data;
    return pinggy_true;
}
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_reconnecting_callback(pinggy_ref_t sdkRef, pinggy_on_reconnecting_cb_t reconnecting, pinggy_void_p_t user_data)
{
    GetEventHandlerFromSdkRef(sdkRef, aev);
    aev->onReconnectingCB = reconnecting;
    aev->onReconnectingUserData = user_data;
    return pinggy_true;
}
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_reconnection_completed_callback(pinggy_ref_t sdkRef, pinggy_on_reconnection_completed_cb_t reconnection_completed, pinggy_void_p_t user_data)
{
    GetEventHandlerFromSdkRef(sdkRef, aev);
    aev->onReconnectionCompletedCB = reconnection_completed;
    aev->onReconnectionCompletedUserData = user_data;
    return pinggy_true;
}
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_reconnection_failed_callback(pinggy_ref_t sdkRef, pinggy_on_reconnection_failed_cb_t reconnection_failed, pinggy_void_p_t user_data)
{
    GetEventHandlerFromSdkRef(sdkRef, aev);
    aev->onReconnectionFailedCB = reconnection_failed;
    aev->onReconnectionFailedUserData = user_data;
    return pinggy_true;
}
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_tunnel_error_callback(pinggy_ref_t sdkRef, pinggy_on_tunnel_error_cb_t error, pinggy_void_p_t user_data)
{
    GetEventHandlerFromSdkRef(sdkRef, aev);
    aev->onErrorCB = error;
    aev->onErrorUserData = user_data;
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
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_usage_update_callback(pinggy_ref_t sdkRef, pinggy_on_usage_update_cb_t update, pinggy_void_p_t user_data)
{
    GetEventHandlerFromSdkRef(sdkRef, aev);
    aev->onUsageUpdateCB = update;
    aev->onUsageUpdateUserData = user_data;
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
pinggy_tunnel_channel_set_on_data_received_callback(pinggy_ref_t channelRef, pinggy_channel_on_data_received_cb_t cb, pinggy_void_p_t user_data)
{
    GetEventHandlerFromChannelRef(channelRef, cev);
    cev->channelDataReceivedCB = cb;
    cev->channelDataReceivedUserData = user_data;
    return pinggy_true;
}

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_set_on_ready_to_send_callback(pinggy_ref_t channelRef, pinggy_channel_on_readyto_send_cb_t cb, pinggy_void_p_t user_data)
{
    GetEventHandlerFromChannelRef(channelRef, cev);
    cev->channelReadyToSendCB = cb;
    cev->channelReadyToSendUserData = user_data;
    return pinggy_true;
}

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_set_on_error_callback(pinggy_ref_t channelRef, pinggy_channel_on_error_cb_t cb, pinggy_void_p_t user_data)
{
    GetEventHandlerFromChannelRef(channelRef, cev);
    cev->channelErrorCB = cb;
    cev->channelErrorUserData = user_data;
    return pinggy_true;
}

PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_set_on_cleanup_callback(pinggy_ref_t channelRef, pinggy_channel_on_cleanup_cb_t cb, pinggy_void_p_t user_data)
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
    return pinggy_tunnel_channel_get_dest_host_len(channelRef, capa, val, NULL);
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_tunnel_channel_get_dest_host_len(pinggy_ref_t channelRef, pinggy_capa_t capa, pinggy_char_p_t val, pinggy_capa_p_t max_len)
{
    auto channel = getSdkChannelWraper(channelRef);
    if (!channel)
        return 0;

    CopyStringToOutputLen(capa, val, channel->GetDestHost(), max_len);
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
    return pinggy_tunnel_channel_get_src_host_len(channelRef, capa, val, NULL);
}


PINGGY_EXPORT pinggy_const_int_t
pinggy_tunnel_channel_get_src_host_len(pinggy_ref_t channelRef, pinggy_capa_t capa, pinggy_char_p_t val, pinggy_capa_p_t max_len)
{
    auto channel = getSdkChannelWraper(channelRef);
    if (!channel)
        return 0;

    CopyStringToOutputLen(capa, val, channel->GetSrcHost(), max_len);
}

//==============================

//==============================================================
PINGGY_EXPORT pinggy_const_int_t
pinggy_version(pinggy_capa_t capa, pinggy_char_p_t val)
{
    return pinggy_version_len(capa, val, NULL);
}

PINGGY_EXPORT pinggy_const_int_t
pinggy_version_len(pinggy_capa_t capa, pinggy_char_p_t val, pinggy_capa_p_t max_len)
{
    tString version = std::to_string(PinggyVersionMajor) + "." + std::to_string(PinggyVersionMinor) + "." + std::to_string(PinggyVersionPatch);
    CopyStringToOutputLen(capa, val, version, max_len);
}

#define DEFINE_CONFIG_GET_FUNC(funcname, macro)                                     \
PINGGY_EXPORT pinggy_const_int_t                                                    \
funcname(pinggy_capa_t capa, pinggy_char_p_t val)                                   \
{                                                                                   \
    return funcname##_len(capa, val, NULL);                                         \
}                                                                                   \
                                                                                    \
PINGGY_EXPORT pinggy_const_int_t                                                    \
funcname##_len(pinggy_capa_t capa, pinggy_char_p_t val, pinggy_capa_p_t max_len)    \
{                                                                                   \
    tString funcname##str = macro;                                                  \
    CopyStringToOutputLen(capa, val, funcname##str, max_len);                       \
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
