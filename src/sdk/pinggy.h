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

/**
 * @file pinggy.h
 * @brief Pinggy SDK C API Header
 *
 * This header provides the C API for the Pinggy SDK, enabling applications to create, configure, and manage tunnels for TCP, UDP, HTTP, and TLS forwarding.
 * The API supports tunnel lifecycle management, configuration, event callbacks, channel data transfer, and querying library/build information.
 *
 * @mainpage Pinggy SDK C API
 *
 * @section intro_sec Introduction
 * The Pinggy SDK allows applications to expose local servers to the internet via secure tunnels. This API provides functions to configure tunnels, manage their lifecycle, handle events via callbacks, and transfer data through channels.
 *
 * @section usage_sec Usage
 * - Create a tunnel configuration using @ref pinggy_create_config.
 * - Set configuration parameters (server address, token, forwarding addresses, etc.).
 * - Initiate a tunnel with @ref pinggy_tunnel_initiate.
 * - Start or connect the tunnel using @ref pinggy_tunnel_start, @ref pinggy_tunnel_connect, or @ref pinggy_tunnel_connect_blocking.
 * - Register event callbacks for tunnel and channel events.
 * - Use channel APIs for data transfer.
 * - Query tunnel and library information as needed.
 *
 * @section callback_sec Callbacks
 * The API supports registering callbacks for tunnel events (connected, authenticated, forwarding succeeded/failed, disconnected, etc.) and channel events (data received, ready to send, error, cleanup).
 *
 * @section thread_sec Thread Safety
 * Some functions may be called from different threads or from within callbacks. Refer to individual function documentation for thread safety guarantees.
 *
 * @section error_sec Error Handling
 * Most functions return a boolean or integer status. Error details may be provided via callbacks or output parameters.
 *
 * @section version_sec Versioning
 * Use @ref pinggy_version and related functions to query the library version, commit, build timestamp, and build OS.
 *
 * @section copyright_sec Copyright
 * Copyright (c) Pinggy.io. All rights reserved.
 *
 * @note This file is intended for use with C and C++ applications. C++ linkage is supported via extern "C".
 */

/*
 * @version 1.0
 * @author
 * @date 2024-12-06
 */


#ifndef __SRC_CPP_SDK_PINGGY_H__
#define __SRC_CPP_SDK_PINGGY_H__

#include <stdint.h>


//=================================


#ifdef __WINDOWS_OS__
    #undef __WINDOWS_OS__
#endif //ifdef __WINDOWS_OS__

#ifdef __MAC_OS__
    #undef __MAC_OS__
#endif //ifdef __MAC_OS__

#ifdef __LINUX_OS__
    #undef __LINUX_OS__
#endif //ifdef __LINUX_OS__

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #define __WINDOWS_OS__
#elif __APPLE__
    #define __MAC_OS__
#elif __linux__
    #define __LINUX_OS__
#elif __unix__ // all unices not caught above
    #error "Not implemented yet"
    // Unix
#elif defined(_POSIX_VERSION)
    #error "Not implemented yet"
    // POSIX
#else
    #error "Unknown compiler"
#endif //defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
//=================================


#ifndef __NOEXPORT_PINGGY_DLL__ //set by cmake
#ifdef __WINDOWS_OS__
#ifdef __EXPORT_PINGGY_DLL__
#define PINGGY_EXPORT __declspec(dllexport)
#else //__EXPORT_PINGGY_DLL__
#define PINGGY_EXPORT __declspec(dllimport)
#endif //__EXPORT_PINGGY_DLL__
#else //__WINDOWS_OS__
#ifdef __EXPORT_PINGGY_DLL__ //set by cmake
#define PINGGY_EXPORT __attribute__((visibility("default")))
#else //__EXPORT_PINGGY_DLL__
#define PINGGY_EXPORT
#endif //__EXPORT_PINGGY_DLL__
#endif //__WINDOWS_OS__
#else //ifndef __NOEXPORT_PINGGY_DLL__
#define PINGGY_EXPORT
#endif //ifdef __NOEXPORT_PINGGY_DLL__

//================


#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

typedef uint8_t                 pinggy_bool_t;
typedef uint32_t                pinggy_ref_t;
typedef char                    pinggy_char_t;
typedef char                   *pinggy_char_p_t;
typedef char                  **pinggy_char_p_p_t;
typedef void                    pinggy_void_t;
typedef void                   *pinggy_void_p_t;
typedef const char             *pinggy_const_char_p_t;
typedef const int               pinggy_const_int_t;
typedef const pinggy_bool_t     pinggy_const_bool_t;
typedef int                     pinggy_int_t;
typedef unsigned int            pinggy_uint_t;
typedef int16_t                 pinggy_len_t;
typedef uint32_t                pinggy_capa_t;
typedef uint32_t               *pinggy_capa_p_t;
typedef uint32_t                pinggy_uint32_t;
typedef uint16_t                pinggy_uint16_t;
typedef int32_t                 pinggy_int32_t;
typedef int32_t                 pinggy_raw_len_t;

#define pinggy_true 1
#define pinggy_false 0

typedef enum TunnelState {
    TunnelState_Invalid = 0,
    TunnelState_Initial,
    TunnelState_Started,
    TunnelState_ReconnectInitiated,
    TunnelState_Reconnecting,
    TunnelState_Connecting,
    TunnelState_Connected,
    TunnelState_SessionInitiating,
    TunnelState_SessionInitiated,
    TunnelState_Authenticating,
    TunnelState_Authenticated,
    TunnelState_ForwardingInitiated,
    TunnelState_ForwardingSucceeded,
    TunnelState_Stopped,
} pinggy_tunnel_state_t;

/**
 * PINGGY_TYPETEST_ENABLED is a type enforcer for code. It make sure that all the code
 * Implementation used exactly same data as the declaration function.
 */
#ifdef PINGGY_TYPETEST_ENABLED
#define void            Error
#define char            Error
#define int             Error
#define uint8_t         Error
#define int16_t         Error
#define uint16_t        Error
#define uint32_t        Error
#define int             Error
#define bool_t          Error
#define void_t          Error
#define int_t           Error
#define len_t           Error
#endif //PINGGY_TYPETEST_ENABLED

#define INVALID_PINGGY_REF 0


PINGGY_EXPORT pinggy_void_t
pinggy_set_log_path(pinggy_char_p_t);

PINGGY_EXPORT pinggy_void_t
pinggy_set_log_enable(pinggy_bool_t);


/**
 * @brief check if interuption occured while running last command. It is just a wrapper for
 * errno==EINTR
 * @return
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_is_interrupted();

//================
/**
 * @typedef pinggy_on_connected_cb_t
 * @brief Callback for when the tunnel is successfully connected to the server.
 * This is an informational callback only and most apps do not need to implement this.
 * @param user_data  User-defined pointer passed during callback registration.
 * @param tunnel_ref Reference to the tunnel object.
 */
typedef pinggy_void_t (*pinggy_on_connected_cb_t)                               \
                            (pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref);

/**
 * @typedef pinggy_on_authenticated_cb_t
 * @brief Callback for when the tunnel is authenticated.
 * Here, authentication means the tunnel is allowed to be set up.
 *
 * This callback can arrive only when the tunnel is being connected, i.e.,
 * `pinggy_tunnel_connect_blocking` is working or `pinggy_tunnel_connect` is
 * called and the app is waiting for an event by calling `pinggy_tunnel_resume`.
 *
 * @param user_data  User-defined pointer passed during callback registration.
 * @param tunnel_ref Reference to the tunnel object.
 */
typedef pinggy_void_t (*pinggy_on_authenticated_cb_t)                           \
                            (pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref);

/**
 * @typedef pinggy_on_authentication_failed_cb_t
 * @brief Callback for when tunnel authentication fails. It is similar to `pinggy_on_authenticated_cb_t`.
 * @param user_data  User-defined pointer passed during callback registration.
 * @param tunnel_ref Reference to the tunnel object.
 * @param num_reasons Size of the `reasons` array.
 * @param reasons Array of strings describing reasons for failure.
 */
typedef pinggy_void_t (*pinggy_on_authentication_failed_cb_t)                   \
                            (pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_len_t num_reasons, pinggy_char_p_p_t reasons);

/**
 * @typedef pinggy_on_forwarding_succeeded_cb_t
 * @brief Callback for when primary forwarding is successfully established.
 *
 * This callback can arrive only when the app has requested primary forwarding using
 * `pinggy_tunnel_start_forwarding_blocking` or has called `pinggy_tunnel_start_forwarding`
 * and is waiting for an event by calling `pinggy_tunnel_resume`.
 *
 * @param user_data  User-defined pointer passed during callback registration.
 * @param tunnel_ref Reference to the tunnel object.
 * @param num_urls Size of the `urls` array.
 * @param urls Array of URLs as strings.
 */
typedef pinggy_void_t (*pinggy_on_forwarding_succeeded_cb_t)            \
                            (pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_len_t num_urls, pinggy_char_p_p_t urls);

/**
 * @typedef pinggy_on_forwarding_failed_cb_t
 * @brief Callback for when primary forwarding fails. The context is the same as
 * `pinggy_on_forwarding_succeeded_cb_t`.
 * @param user_data  User-defined pointer passed during callback registration.
 * @param tunnel_ref Reference to the tunnel object.
 * @param msg Error message string.
 */
typedef pinggy_void_t (*pinggy_on_forwarding_failed_cb_t)               \
                            (pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_const_char_p_t msg);

/**
 * @typedef pinggy_on_additional_forwarding_succeeded_cb_t
 * @brief Callback for when additional forwarding is successfully established.
 * The app can expect this callback when it has called `pinggy_tunnel_request_additional_forwarding`.
 * @param user_data  User-defined pointer passed during callback registration.
 * @param tunnel_ref Reference to the tunnel object.
 * @param bind_addr  The remote bind address as a string.
 * @param forward_to_addr The local forwarding address as a string.
 * @param forwarding_type The forwarding type as a string.
 */
typedef pinggy_void_t (*pinggy_on_additional_forwarding_succeeded_cb_t)         \
                            (pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_const_char_p_t bind_addr, pinggy_const_char_p_t forward_to_addr, pinggy_const_char_p_t forwarding_type);

/**
 * @typedef pinggy_on_additional_forwarding_failed_cb_t
 * @brief Callback for when additional forwarding fails.
 * The app can expect this callback when it has called `pinggy_tunnel_request_additional_forwarding`.
 * @param user_data  User-defined pointer passed during callback registration.
 * @param tunnel_ref Reference to the tunnel object.
 * @param bind_addr  The remote bind address as a string.
 * @param forward_to_addr The local forwarding address as a string.
 * @param forwarding_type The forwarding type as a string.
 * @param error Error message string.
 */
typedef pinggy_void_t (*pinggy_on_additional_forwarding_failed_cb_t)            \
                            (pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_const_char_p_t bind_addr, pinggy_const_char_p_t forward_to_addr, pinggy_const_char_p_t forwarding_type, pinggy_const_char_p_t error);

/**
 * @typedef pinggy_on_forwardings_changed_cb_t
 * @brief Callback for when the forwarding map changes.
 *
 * The app should expect this call at any time as long as the tunnel is running.
 * The `url_map` is a JSON string and the format is not finalized.
 *
 * **Do not use this callback for now.**
 *
 * @param user_data  User-defined pointer passed during callback registration.
 * @param tunnel_ref Reference to the tunnel object.
 * @param url_map JSON string describing the new forwarding map.
 */
typedef pinggy_void_t (*pinggy_on_forwardings_changed_cb_t)                      \
                            (pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_const_char_p_t url_map);

/**
 * @typedef pinggy_on_disconnected_cb_t
 * @brief Callback for when the tunnel is disconnected.
 * @param user_data  User-defined pointer passed during callback registration.
 * @param tunnel_ref Reference to the tunnel object.
 * @param error Error message string.
 * @param msg_size Size of the msg array.
 * @param msg Array of message strings.
 */
typedef pinggy_void_t (*pinggy_on_disconnected_cb_t)                            \
                            (pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_const_char_p_t error, pinggy_len_t msg_size, pinggy_char_p_p_t msg);

/**
 * @typedef pinggy_on_will_reconnect_cb_t
 * @brief Callback for when the tunnel is disconnected and will try to reconnect now.
 * @param user_data  User-defined pointer passed during callback registration.
 * @param tunnel_ref Reference to the tunnel object.
 * @param error Error message string.
 * @param num_msgs Number of messages.
 * @param messages Array of message strings.
 */
typedef pinggy_void_t (*pinggy_on_will_reconnect_cb_t)                          \
                            (pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_const_char_p_t error, pinggy_len_t num_msgs, pinggy_char_p_p_t messages);

/**
 * @typedef pinggy_on_reconnecting_cb_t
 * @brief Callback for each reconnection attempt.
 * @param user_data  User-defined pointer passed during callback registration.
 * @param tunnel_ref Reference to the tunnel object.
 * @param retry_cnt  The current retry count.
 */
typedef pinggy_void_t (*pinggy_on_reconnecting_cb_t)                            \
                            (pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_uint16_t retry_cnt);

/**
 * @typedef pinggy_on_reconnection_completed_cb_t
 * @brief Callback for when reconnection is completed.
 * @param user_data  User-defined pointer passed during callback registration.
 * @param tunnel_ref Reference to the tunnel object.
 * @param num_urls Size of urls array.
 * @param urls Array of URLs as strings.
 */
typedef pinggy_void_t (*pinggy_on_reconnection_completed_cb_t)                  \
                            (pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_len_t num_urls, pinggy_char_p_p_t urls);

/**
 * @typedef pinggy_on_reconnection_failed_cb_t
 * @brief Callback for when reconnection fails after all attempts.
 * @param user_data  User-defined pointer passed during callback registration.
 * @param tunnel_ref Reference to the tunnel object.
 * @param retry_cnt  The number of retries attempted.
 */
typedef pinggy_void_t (*pinggy_on_reconnection_failed_cb_t)                     \
                            (pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_uint16_t retry_cnt);

/**
 * @typedef pinggy_on_usage_update_cb_t
 * @brief Callback for when usage information is updated.
 * @param user_data  User-defined pointer passed during callback registration.
 * @param tunnel_ref Reference to the tunnel object.
 * @param usages JSON string describing usage information. The format of the usages
 * string is `{"elapsedTime":7,"numLiveConnections":6,"numTotalConnections":6,"numTotalReqBytes":16075,"numTotalResBytes":815760,"numTotalTxBytes":831835}`
 */
typedef pinggy_void_t (*pinggy_on_usage_update_cb_t)                            \
                            (pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_const_char_p_t usages);

/**
 * @typedef pinggy_on_tunnel_error_cb_t
 * @brief Callback for when a tunnel error occurs.
 * @param user_data  User-defined pointer passed during callback registration.
 * @param tunnel_ref Reference to the tunnel object.
 * @param error_no   Error number. (For future use. Not used now.)
 * @param error      Error message string.
 * @param recoverable Whether the error is recoverable.
 */
typedef pinggy_void_t (*pinggy_on_tunnel_error_cb_t)                            \
                            (pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_uint32_t error_no, pinggy_const_char_p_t error, pinggy_bool_t recoverable);

/**
 * @typedef pinggy_on_new_channel_cb_t
 * @brief Callback for when a new channel is created. To handle the channel manually,
 * return true. Otherwise, the SDK will assume that the app wants the SDK to handle
 * this new channel.
 *
 * If app decides to handle a channel, it has to accept or reject the channel as soon
 * as possible. Also, it is responsible to free up the channel reference by calling
 * `pinggy_free_ref`.
 *
 * NOTE: This callback is currently disabled. It will be enabled in future versions.
 *
 * @param user_data  User-defined pointer passed during callback registration.
 * @param tunnel_ref Reference to the tunnel object.
 * @param channel_ref Reference to the new channel object.
 * @return Return pinggy_true to specify that app wants to handle it, otherwise false.
 */
typedef pinggy_bool_t (*pinggy_on_new_channel_cb_t)                             \
                            (pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_ref_t channel_ref);

/**
 * @typedef pinggy_on_raise_exception_cb_t
 * @brief Callback for when an exception is raised inside the library.
 *
 * The app is supposed to receive C++ exceptions through this callback. This callback is not specific to any tunnel or channel; it is a global callback. The app can and should assume that it may get different exceptions for different threads, and should throw this exception when the original pinggy_* call returns. Check the Python implementation for an example.
 *
 * @param where String describing where the exception occurred.
 * @param what  String describing what the exception was.
 */
typedef pinggy_void_t (*pinggy_on_raise_exception_cb_t)                         \
                            (pinggy_const_char_p_t where, pinggy_const_char_p_t what);
//================

/**
 * @brief  Set a function pointer which would handle exception occured inside the library
 * @param  pointer to function with type pinggy_on_raise_exception_cb_t
 *
 * @Example
 * The exception callback function can looklike following
 * ```
 * _Thread_local const char *global_where;
 * _Thread_local const char *global_what;
 *
 * void
 * setChars(pinggy_const_char_p_t where, pinggy_const_char_p_t what)
 * {
 *      if (global_where) {
 *          free(global_where);
 *          global_where = NULL;
 *      }
 *      if (global_what) {
 *          free(global_what);
 *          global_what = NULL;
 *      }
 *      if (where) {
 *          global_where = (pinggy_const_char_p_t)malloc(strlen(where)+1);
 *          strcpy(global_where, where);
 *      }
 *      if (what) {
 *          global_what = (pinggy_const_char_p_t)malloc(strlen(where)+1);
 *          strcpy(global_what, what);
 *      }
 * }
 * ```
 * Above code is just an example. User have to decide exactly they want to keep
 * the exception for them
 */
PINGGY_EXPORT pinggy_void_t
pinggy_set_on_exception_callback(pinggy_on_raise_exception_cb_t);

/**
 * @brief Free a internal object refered by the `reference`
 * @param reference to the internal object
 * @return true on success.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_free_ref(pinggy_ref_t reference);

//====================================

/**
 * @brief  Create a new pinggy tunnel config reference and return the reference
 * @return newly create tunnel config reference
 */
PINGGY_EXPORT pinggy_ref_t
pinggy_create_config();

//====================================

/**
 * @brief Sets the server address for the tunnel configuration.
 *
 * By default, the server address is set to "a.pinggy.io:443". The address should be in the format "<server>:<port>" or just "<server>".
 *
 * @param config         Reference to the tunnel config object.
 * @param server_address Null-terminated string specifying the server address.
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_server_address(pinggy_ref_t config, pinggy_char_p_t server_address);

/**
 * @brief Sets the authentication token for the tunnel configuration.
 *
 * The SDK does not verify the token locally; it is sent directly to the server. While you may pass a token+method, this is discouraged and may not be supported in future versions.
 *
 * @param config Reference to the tunnel config object.
 * @param token  Null-terminated string specifying the authentication token.
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_token(pinggy_ref_t config, pinggy_char_p_t token);

/**
 * @brief Adds a new forwarding rule to the tunnel configuration.
 *
 * This function allows you to specify how incoming connections to a `binding_url`
 * on the Pinggy server should be forwarded to a `forward_to` address on your local machine.
 *
 * @param config          Reference to the tunnel config object.
 * @param forwarding_type Null-terminated string specifying the type of forwarding.
 *                        Valid types are "http", "tcp", "udp", "tls", "tlstcp".
 *                        If an empty string or NULL is provided, "http" is assumed.
 * @param binding_url     Null-terminated string specifying the remote address to bind to.
 *                        This can be a domain name, a domain:port combination, or just a port.
 *                        Examples: "example.pinggy.io", "example.pinggy.io:8080", ":80".
 *                        If empty or NULL, the server will assign a default binding.
 * @param forward_to      Null-terminated string specifying the local address to forward to.
 *                        This can be a URL (e.g., "http://localhost:3000"), an IP address
 *                        (e.g., "127.0.0.1:8000"), or just a port (e.g., ":5000").
 *                        If the schema (e.g., "http://") and host are omitted, "localhost"
 *                        is assumed. For example, ":3000" becomes "http://localhost:3000"
 *                        for HTTP forwarding.
 *                        If `forwarding_type` is "http" and `forward_to` specifies an "https"
 *                        schema (e.g., "https://localhost:443"), this implicitly enables
 *                        `local_server_tls` for this specific forwarding rule.
 * @return                pinggy_true on success, pinggy_false on failure.
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_add_forwarding(pinggy_ref_t config, pinggy_char_p_t forwarding_type, pinggy_char_p_t binding_url, pinggy_char_p_t forward_to);

/**
 * @brief Adds a new forwarding rule to the tunnel configuration with simplified parameters.
 *
 * This function provides a simplified way to add forwarding rules. It automatically
 * determines the `forwarding_type` and `binding_url` based on the `forward_to`
 * parameter, following a specific format.
 *
 * The `forward_to` parameter should follow the format `[forwarding_type://][localhost:]port`.
 *
 * - If `forwarding_type` is "https", it means the `forwarding_type` will be "http"
 *   and `local_server_tls` will be enabled for this specific forwarding rule.
 * - If `forwarding_type` is omitted, "http" is assumed.
 * - If `localhost:` is omitted, "localhost" is assumed.
 *
 * Examples:
 * - `pinggy_config_add_forwarding_simple(config, "3000")` will be interpreted as
 *   `pinggy_config_add_forwarding(config, "http", "", "http://localhost:3000")`.
 * - `pinggy_config_add_forwarding_simple(config, "tcp://localhost:22")` will be interpreted as
 *   `pinggy_config_add_forwarding(config, "tcp", "", "tcp://localhost:22")`.
 * - `pinggy_config_add_forwarding_simple(config, "https://8080")` will be interpreted as
 *   `pinggy_config_add_forwarding(config, "http", "", "https://localhost:8080")`
 *   and will also enable `local_server_tls` for this rule.
 *
 * @param config          Reference to the tunnel config object.
 * @param forward_to      Null-terminated string specifying the local address to forward to,
 *                        following the format `[forwarding_type://][localhost:]port`.
 * @return                pinggy_true on success, pinggy_false on failure.
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_add_forwarding_simple(pinggy_ref_t config, pinggy_char_p_t forward_to);

/**
 * @brief Sets multiple forwarding rules for the tunnel configuration.
 *
 * This function allows you to define multiple forwarding rules either as a single
 * simplified forwarding string (similar to `pinggy_config_add_forwarding_simple`)
 * or as a JSON array of forwarding objects.
 *
 * If `forwardings` is a single string, it should follow the format
 * `[forwarding_type://][localhost:]port`.
 *
 * If `forwardings` is a JSON string, it should be an array of objects, where each
 * object defines a forwarding rule with the following properties:
 * - `type`: (Optional) The type of forwarding (e.g., "http", "tcp", "udp", "tls", "tlstcp").
 *   Defaults to "http" if not specified.
 * - `listenAddress`: (Optional) The remote address to bind to. Format: `[host][:port]`.
 *   An empty string or undefined means the server will assign a default binding.
 *   The hostname is ignored for TCP and UDP tunnels. Any schema provided will be ignored.
 * - `address`: The local address to forward to. Format: `[protocol://][host]:port`.
 *   The `protocol` is primarily used to determine if `local_server_tls` should be
 *   enabled for this specific rule (e.g., `https://`). It is ignored otherwise.
 *
 * @param config      Reference to the tunnel config object.
 * @param forwardings Null-terminated string representing either a single simplified
 *                    forwarding rule or a JSON array of forwarding rule objects.
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_forwardings(pinggy_ref_t config, pinggy_char_p_t forwardings);

/**
 * @brief Resets all forwarding rules previously added to the tunnel configuration.
 *
 * This function clears any forwarding rules that were set using `pinggy_config_add_forwarding`,
 * `pinggy_config_add_forwarding_simple`, or `pinggy_config_set_forwardings`.
 * After calling this function, the tunnel configuration will have no active forwarding rules
 * until new ones are added.
 *
 * @param config Reference to the tunnel config object.
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_reset_forwardings(pinggy_ref_t config);

/**
 * @brief Enables or disables force mode for the tunnel configuration.
 *
 * For more details, see: https://pinggy.io/docs/usages/#4-force
 *
 * @param config Reference to the tunnel config object.
 * @param force  Set to pinggy_true to enable force mode, or pinggy_false to disable.
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_force(pinggy_ref_t config, pinggy_bool_t force);

/**
 * @brief Sets additional command-line arguments for the tunnel configuration.
 *
 * @note This is for backward compatibility only. Prefer using individual parameter setting functions.
 *
 * @param config   Reference to the tunnel config object.
 * @param argument Null-terminated string specifying the argument.
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_argument(pinggy_ref_t config, pinggy_char_p_t argument);

/**
 * @brief Enables or disables advanced HTTP parsing for the tunnel.
 *
 * Advanced parsing is enabled by default. Free HTTP tunnels do not work without advanced parsing.
 *
 * @note This function is intended for developers only; normal users should not use it.
 *
 * @param config           Reference to the tunnel config object.
 * @param advanced_parsing Set to pinggy_true to enable, or pinggy_false to disable advanced parsing.
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_advanced_parsing(pinggy_ref_t config, pinggy_bool_t advanced_parsing);

/**
 * @brief Enables or disables SSL for tunnel setup.
 *
 * SSL is enabled by default. No production or test server supports non-SSL connections for tunnel establishment.
 *
 * @param config Reference to the tunnel config object.
 * @param ssl    Set to pinggy_true to enable SSL, or pinggy_false to disable.
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_ssl(pinggy_ref_t config, pinggy_bool_t ssl);

/**
 * @brief Sets the SNI (Server Name Indication) for the tunnel connection.
 *
 * By default, SNI is not required. It may be needed for test servers or development environments.
 *
 * @param config          Reference to the tunnel config object.
 * @param sni_server_name Null-terminated string specifying the SNI server name.
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_sni_server_name(pinggy_ref_t config, pinggy_char_p_t sni_server_name);

/**
 * @brief Enables or disables insecure SSL mode for the tunnel configuration.
 *
 * @param config   Reference to the tunnel config object.
 * @param insecure Set to pinggy_true to enable insecure SSL, or pinggy_false to disable.
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_insecure(pinggy_ref_t config, pinggy_bool_t insecure);

/**
 * @brief Enables or disables auto-reconnection for the tunnel.
 *
 * Auto-reconnection is disabled by default.
 *
 * @param config Reference to the tunnel config object.
 * @param enable Set to pinggy_true to enable auto-reconnect, or pinggy_false to disable.
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_auto_reconnect(pinggy_ref_t config, pinggy_bool_t enable);

/**
 * @brief Sets the maximum number of reconnection attempts before giving up.
 *
 * The default is 20 attempts.
 *
 * @param config   Reference to the tunnel config object.
 * @param num_tries Maximum number of reconnection attempts.
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_max_reconnect_attempts(pinggy_ref_t config, pinggy_uint16_t num_tries);

/**
 * @brief Sets the reconnection retry interval (in seconds).
 *
 * The default interval is 5 seconds. The minimum allowed is 1 second, but intervals below 5 seconds are discouraged.
 *
 * @param config   Reference to the tunnel config object.
 * @param interval Retry interval in seconds.
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_reconnect_interval(pinggy_ref_t config, pinggy_uint16_t interval);

//====

/**
 * @brief Sets the header manipulation configuration for the tunnel.
 *
 * The configuration should be a JSON string describing header operations (add, remove, update). For details and examples, see:
 * https://github.com/Pinggy-io/Wiki/blob/f8fe49883a5277a43e7606618bccd2a04d8ac0dc/TunnelConfigSpec/README.md?plain=1#L58
 *
 * @param config               Reference to the tunnel config object.
 * @param header_manipulations Null-terminated JSON string specifying header manipulations.
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_header_manipulations(pinggy_ref_t config, pinggy_const_char_p_t header_manipulations);

/**
 * @brief Sets the basic authentication configuration for the tunnel.
 *
 * The configuration should be a JSON array of objects with "username" and "password" fields. Passwords are in plaintext.
 * For details and examples, see:
 * https://github.com/Pinggy-io/Wiki/blob/f8fe49883a5277a43e7606618bccd2a04d8ac0dc/TunnelConfigSpec/README.md?plain=1#L51
 *
 * @param config     Reference to the tunnel config object.
 * @param basic_auths Null-terminated JSON string specifying basic authentication users.
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_basic_auths(pinggy_ref_t config, pinggy_const_char_p_t basic_auths);

/**
 * @brief Sets the bearer token authentication configuration for the tunnel.
 *
 * The configuration should be a JSON array of strings, e.g., ["Key1", "Key2"].
 * For details and examples, see:
 * https://github.com/Pinggy-io/Wiki/blob/f8fe49883a5277a43e7606618bccd2a04d8ac0dc/TunnelConfigSpec/README.md?plain=1#L55
 *
 * @param config             Reference to the tunnel config object.
 * @param bearer_token_auths Null-terminated JSON string specifying bearer tokens.
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_bearer_token_auths(pinggy_ref_t config, pinggy_const_char_p_t bearer_token_auths);

/**
 * @brief Sets the IP whitelist for the tunnel configuration.
 *
 * The whitelist should be a JSON array of IP addresses or CIDR blocks, e.g., ["10.0.0.1/32", "19.2.4.5", "::1/1"].
 * For details and examples, see:
 * https://github.com/Pinggy-io/Wiki/blob/f8fe49883a5277a43e7606618bccd2a04d8ac0dc/TunnelConfigSpec/README.md?plain=1#L48
 *
 * @param config        Reference to the tunnel config object.
 * @param ip_white_list Null-terminated JSON string specifying allowed IPs/networks.
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_ip_white_list(pinggy_ref_t config, pinggy_const_char_p_t ip_white_list);

/**
 * @brief Enables or disables reverse proxy mode for the tunnel.
 *
 * In reverse proxy mode, the server sets headers such as Forwarded, X-Forwarded-For, X-Forwarded-Host, and X-Forwarded-Proto.
 * For details, see: https://developer.mozilla.org/en-US/docs/Web/HTTP/Guides/Proxy_servers_and_tunneling
 *
 * @param config        Reference to the tunnel config object.
 * @param reverse_proxy Set to pinggy_true to enable, or pinggy_false to disable reverse proxy mode.
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_reverse_proxy(pinggy_ref_t config, pinggy_bool_t reverse_proxy);

/**
 * @brief Enables or disables the X-Forwarded-For header for the tunnel.
 *
 * If enabled, the server adds the X-Forwarded-For header with the original source address. In reverse proxy mode, this is always enabled and cannot be disabled by this flag.
 *
 * @param config           Reference to the tunnel config object.
 * @param x_forwarded_for  Set to pinggy_true to enable, or pinggy_false to disable X-Forwarded-For.
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_x_forwarded_for(pinggy_ref_t config, pinggy_bool_t x_forwarded_for);

/**
 * @brief Enables or disables HTTPS-only mode for the tunnel.
 *
 * In HTTPS-only mode, the server redirects all HTTP requests to HTTPS using a 301 Moved Permanently response.
 *
 * @param config     Reference to the tunnel config object.
 * @param https_only Set to pinggy_true to enable, or pinggy_false to disable HTTPS-only mode.
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_https_only(pinggy_ref_t config, pinggy_bool_t https_only);

/**
 * @brief Enables or disables original-request-url mode for the tunnel.
 *
 * When enabled, the server adds the X-Pinggy-Url header to requests, containing the original URL requested by the client.
 *
 * @param config                Reference to the tunnel config object.
 * @param original_request_url  Set to pinggy_true to enable, or pinggy_false to disable this mode.
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_original_request_url(pinggy_ref_t config, pinggy_bool_t original_request_url);

/**
 * @brief Enables or disables allow-preflight mode for the tunnel.
 *
 * When enabled, the server responds to CORS preflight (OPTIONS) requests with appropriate headers to allow cross-origin requests.
 *
 * @param config         Reference to the tunnel config object.
 * @param allow_preflight Set to pinggy_true to enable, or pinggy_false to disable allow-preflight mode.
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_allow_preflight(pinggy_ref_t config, pinggy_bool_t allow_preflight);

/**
 * @brief Sets the local server TLS configuration for the tunnel.
 *
 * When enabled, the server connects to the local server using TLS. This is useful if the local server is configured for HTTPS.
 *
 * @param config           Reference to the tunnel config object.
 * @param local_server_tls Null-terminated string specifying the local server TLS configuration.
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_local_server_tls(pinggy_ref_t config, pinggy_const_char_p_t local_server_tls);

//==============================

/**
 * @brief Retrieves the currently configured server address from the tunnel config.
 * @param config      Reference to the tunnel config object.
 * @param buffer_len  Length of the buffer provided for the server address.
 * @param buffer      Pointer to a character array where the server address will be copied.
 * @return            Number of bytes copied to the buffer (including null terminator).
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_server_address(pinggy_ref_t config, pinggy_capa_t buffer_len, pinggy_char_p_t buffer);

/**
 * @brief Retrieves the server address from the tunnel config, and provides the required buffer size.
 * @param config      Reference to the tunnel config object.
 * @param buffer_len  Length of the buffer provided for the server address.
 * @param buffer      Pointer to a character array where the server address will be copied.
 * @param max_len     Pointer to a variable that will be set to the total length required to hold the full server address (including null terminator).
 * @return            Number of bytes copied to the buffer (including null terminator).
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_server_address_len(pinggy_ref_t config, pinggy_capa_t buffer_len, pinggy_char_p_t buffer, pinggy_capa_p_t max_len);

/**
 * @brief Retrieves the currently configured token from the tunnel config.
 * @param config      Reference to the tunnel config object.
 * @param buffer_len  Length of the buffer provided for the token.
 * @param buffer      Pointer to a character array where the token will be copied.
 * @return            Number of bytes copied to the buffer (including null terminator).
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_token(pinggy_ref_t config, pinggy_capa_t buffer_len, pinggy_char_p_t buffer);

/**
 * @brief Retrieves the token from the tunnel config, and provides the required buffer size.
 * @param config      Reference to the tunnel config object.
 * @param buffer_len  Length of the buffer provided for the token.
 * @param buffer      Pointer to a character array where the token will be copied.
 * @param max_len     Pointer to a variable that will be set to the total length required to hold the full token (including null terminator).
 * @return            Number of bytes copied to the buffer (including null terminator).
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_token_len(pinggy_ref_t config, pinggy_capa_t buffer_len, pinggy_char_p_t buffer, pinggy_capa_p_t max_len);

/**
 * @brief Retrieves the forwarding rules (as a JSON string) from the tunnel config.
 * @param config      Reference to the tunnel config object.
 * @param buffer_len  Length of the buffer provided for the forwarding rules string.
 * @param buffer      Pointer to a character array where the forwarding rules JSON will be copied.
 * @return            Number of bytes copied to the buffer (including null terminator).
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_forwardings(pinggy_ref_t config, pinggy_capa_t buffer_len, pinggy_char_p_t buffer);

/**
 * @brief Retrieves the forwarding rules (as a JSON string) from the tunnel config, and provides the required buffer size.
 * @param config      Reference to the tunnel config object.
 * @param buffer_len  Length of the buffer provided for the forwarding rules string.
 * @param buffer      Pointer to a character array where the forwarding rules JSON will be copied.
 * @param max_len     Pointer to a variable that will be set to the total length required to hold the full forwarding rules string (including null terminator).
 * @return            Number of bytes copied to the buffer (including null terminator).
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_forwardings_len(pinggy_ref_t config, pinggy_capa_t buffer_len, pinggy_char_p_t buffer, pinggy_capa_p_t max_len);


/**
 * @brief Checks whether the force mode is enabled in the tunnel config.
 * @param config  Reference to the tunnel config object.
 * @return        pinggy_true if force is enabled, otherwise pinggy_false.
 */
PINGGY_EXPORT pinggy_const_bool_t
pinggy_config_get_force(pinggy_ref_t config);

/**
 * @brief Retrieves the current command-line arguments from the tunnel config.
 * @param config      Reference to the tunnel config object.
 * @param buffer_len  Length of the buffer provided for the arguments string.
 * @param buffer      Pointer to a character array where the arguments will be copied.
 * @return            Number of bytes copied to the buffer (excluding null terminator).
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_argument(pinggy_ref_t config, pinggy_capa_t buffer_len, pinggy_char_p_t buffer);

/**
 * @brief Retrieves the current command-line arguments from the tunnel config, and provides the required buffer size.
 * @param config      Reference to the tunnel config object.
 * @param buffer_len  Length of the buffer provided for the arguments string.
 * @param buffer      Pointer to a character array where the arguments will be copied.
 * @param max_len     Pointer to a variable that will be set to the total length required to hold the full arguments string (including null terminator).
 * @return            Number of bytes copied to the buffer (including null terminator).
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_argument_len(pinggy_ref_t config, pinggy_capa_t buffer_len, pinggy_char_p_t buffer, pinggy_capa_p_t max_len);

/**
 * @brief Checks whether advanced HTTP parsing is enabled in the tunnel config.
 * @param config  Reference to the tunnel config object.
 * @return        pinggy_true if advanced parsing is enabled, otherwise pinggy_false.
 */
PINGGY_EXPORT pinggy_const_bool_t
pinggy_config_get_advanced_parsing(pinggy_ref_t config);

/**
 * @brief Checks whether SSL is enabled for the tunnel setup in the config.
 * @param config  Reference to the tunnel config object.
 * @return        pinggy_true if SSL is enabled, otherwise pinggy_false.
 */
PINGGY_EXPORT pinggy_const_bool_t
pinggy_config_get_ssl(pinggy_ref_t config);

/**
 * @brief Retrieves the SNI (Server Name Indication) server name from the tunnel config.
 * @param config      Reference to the tunnel config object.
 * @param buffer_len  Length of the buffer provided for the SNI server name.
 * @param buffer      Pointer to a character array where the SNI server name will be copied.
 * @return            Number of bytes copied to the buffer (including null terminator).
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_sni_server_name(pinggy_ref_t config, pinggy_capa_t buffer_len, pinggy_char_p_t buffer);

/**
 * @brief Retrieves the SNI server name from the tunnel config, and provides the required buffer size.
 * @param config      Reference to the tunnel config object.
 * @param buffer_len  Length of the buffer provided for the SNI server name.
 * @param buffer      Pointer to a character array where the SNI server name will be copied.
 * @param max_len     Pointer to a variable that will be set to the total length required to hold the full SNI server name (including null terminator).
 * @return            Number of bytes copied to the buffer (including null terminator).
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_sni_server_name_len(pinggy_ref_t config, pinggy_capa_t buffer_len, pinggy_char_p_t buffer, pinggy_capa_p_t max_len);

/**
 * @brief Checks whether insecure SSL mode is enabled in the tunnel config.
 * @param config  Reference to the tunnel config object.
 * @return        pinggy_true if insecure SSL is enabled, otherwise pinggy_false.
 */
PINGGY_EXPORT pinggy_const_bool_t
pinggy_config_get_insecure(pinggy_ref_t config);

/**
 * @brief Checks whether auto-reconnect is enabled in the tunnel config.
 * @param config  Reference to the tunnel config object.
 * @return        pinggy_true if auto-reconnect is enabled, otherwise pinggy_false.
 */
PINGGY_EXPORT pinggy_const_bool_t
pinggy_config_get_auto_reconnect(pinggy_ref_t config);

/**
 * @brief Retrieves the maximum number of reconnection attempts configured before giving up.
 * @param config  Reference to the tunnel config object.
 * @return        The maximum number of reconnection attempts.
 */
PINGGY_EXPORT pinggy_uint16_t
pinggy_config_get_max_reconnect_attempts(pinggy_ref_t config);

/**
 * @brief Retrieves the reconnection retry interval (in seconds) from the tunnel config.
 * @param config  Reference to the tunnel config object.
 * @return        The reconnection retry interval in seconds.
 */
PINGGY_EXPORT pinggy_uint16_t
pinggy_config_get_reconnect_interval(pinggy_ref_t config);

//========

/**
 * @brief Retrieves the header manipulation configuration (as a JSON string) from the tunnel config.
 * @param config      Reference to the tunnel config object.
 * @param buffer_len  Length of the buffer provided for the header manipulations string.
 * @param buffer      Pointer to a character array where the header manipulations JSON will be copied.
 * @return            Number of bytes copied to the buffer (excluding null terminator).
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_header_manipulations(pinggy_ref_t config, pinggy_capa_t buffer_len, pinggy_char_p_t buffer);

/**
 * @brief Retrieves the header manipulation configuration from the tunnel config, and provides the required buffer size.
 * @param config      Reference to the tunnel config object.
 * @param buffer_len  Length of the buffer provided for the header manipulations string.
 * @param buffer      Pointer to a character array where the header manipulations JSON will be copied.
 * @param max_len     Pointer to a variable that will be set to the total length required to hold the full header manipulations string (including null terminator).
 * @return            Number of bytes copied to the buffer (including null terminator).
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_header_manipulations_len(pinggy_ref_t config, pinggy_capa_t buffer_len, pinggy_char_p_t buffer, pinggy_capa_p_t max_len);

/**
 * @brief Retrieves the basic authentication configuration (as a JSON string) from the tunnel config.
 * @param config      Reference to the tunnel config object.
 * @param buffer_len  Length of the buffer provided for the basic auths string.
 * @param buffer      Pointer to a character array where the basic auths JSON will be copied.
 * @return            Number of bytes copied to the buffer (including null terminator).
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_basic_auths(pinggy_ref_t config, pinggy_capa_t buffer_len, pinggy_char_p_t buffer);

/**
 * @brief Retrieves the basic authentication configuration from the tunnel config, and provides the required buffer size.
 * @param config      Reference to the tunnel config object.
 * @param buffer_len  Length of the buffer provided for the basic auths string.
 * @param buffer      Pointer to a character array where the basic auths JSON will be copied.
 * @param max_len     Pointer to a variable that will be set to the total length required to hold the full basic auths string (including null terminator).
 * @return            Number of bytes copied to the buffer (including null terminator).
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_basic_auths_len(pinggy_ref_t config, pinggy_capa_t buffer_len, pinggy_char_p_t buffer, pinggy_capa_p_t max_len);

/**
 * @brief Retrieves the bearer token authentication configuration (as a JSON string) from the tunnel config.
 * @param config      Reference to the tunnel config object.
 * @param buffer_len  Length of the buffer provided for the bearer token auths string.
 * @param buffer      Pointer to a character array where the bearer token auths JSON will be copied.
 * @return            Number of bytes copied to the buffer (including null terminator).
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_bearer_token_auths(pinggy_ref_t config, pinggy_capa_t buffer_len, pinggy_char_p_t buffer);

/**
 * @brief Retrieves the bearer token authentication configuration from the tunnel config, and provides the required buffer size.
 * @param config      Reference to the tunnel config object.
 * @param buffer_len  Length of the buffer provided for the bearer token auths string.
 * @param buffer      Pointer to a character array where the bearer token auths JSON will be copied.
 * @param max_len     Pointer to a variable that will be set to the total length required to hold the full bearer token auths string (including null terminator).
 * @return            Number of bytes copied to the buffer (including null terminator).
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_bearer_token_auths_len(pinggy_ref_t config, pinggy_capa_t buffer_len, pinggy_char_p_t buffer, pinggy_capa_p_t max_len);

/**
 * @brief Retrieves the IP whitelist configuration (as a JSON string) from the tunnel config.
 * @param config      Reference to the tunnel config object.
 * @param buffer_len  Length of the buffer provided for the IP whitelist string.
 * @param buffer      Pointer to a character array where the IP whitelist JSON will be copied.
 * @return            Number of bytes copied to the buffer (including null terminator).
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_ip_white_list(pinggy_ref_t config, pinggy_capa_t buffer_len, pinggy_char_p_t buffer);

/**
 * @brief Retrieves the IP whitelist configuration from the tunnel config, and provides the required buffer size.
 * @param config      Reference to the tunnel config object.
 * @param buffer_len  Length of the buffer provided for the IP whitelist string.
 * @param buffer      Pointer to a character array where the IP whitelist JSON will be copied.
 * @param max_len     Pointer to a variable that will be set to the total length required to hold the full IP whitelist string (including null terminator).
 * @return            Number of bytes copied to the buffer (including null terminator).
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_ip_white_list_len(pinggy_ref_t config, pinggy_capa_t buffer_len, pinggy_char_p_t buffer, pinggy_capa_p_t max_len);

/**
 * @brief Checks whether reverse proxy mode is enabled in the tunnel config.
 * @param config  Reference to the tunnel config object.
 * @return        pinggy_true if reverse proxy is enabled, otherwise pinggy_false.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_config_get_reverse_proxy(pinggy_ref_t config);

/**
 * @brief Checks whether X-Forwarded-For header addition is enabled in the tunnel config.
 * @param config  Reference to the tunnel config object.
 * @return        pinggy_true if X-Forwarded-For is enabled, otherwise pinggy_false.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_config_get_x_forwarded_for(pinggy_ref_t config);

/**
 * @brief Checks whether HTTPS-only mode is enabled in the tunnel config.
 * @param config  Reference to the tunnel config object.
 * @return        pinggy_true if HTTPS-only is enabled, otherwise pinggy_false.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_config_get_https_only(pinggy_ref_t config);

/**
 * @brief Checks whether original-request-url mode is enabled in the tunnel config.
 * @param config  Reference to the tunnel config object.
 * @return        pinggy_true if original-request-url is enabled, otherwise pinggy_false.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_config_get_original_request_url(pinggy_ref_t config);

/**
 * @brief Checks whether allow-preflight mode is enabled in the tunnel config.
 * @param config  Reference to the tunnel config object.
 * @return        pinggy_true if allow-preflight is enabled, otherwise pinggy_false.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_config_get_allow_preflight(pinggy_ref_t config);

/**
 * @brief Retrieves the local server TLS configuration (as a string) from the tunnel config.
 * @param config      Reference to the tunnel config object.
 * @param buffer_len  Length of the buffer provided for the local server TLS string.
 * @param buffer      Pointer to a character array where the local server TLS string will be copied.
 * @return            Number of bytes copied to the buffer (excluding null terminator).
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_local_server_tls(pinggy_ref_t config, pinggy_capa_t buffer_len, pinggy_char_p_t buffer);

/**
 * @brief Retrieves the local server TLS configuration from the tunnel config, and provides the required buffer size.
 * @param config      Reference to the tunnel config object.
 * @param buffer_len  Length of the buffer provided for the local server TLS string.
 * @param buffer      Pointer to a character array where the local server TLS string will be copied.
 * @param max_len     Pointer to a variable that will be set to the total length required to hold the full local server TLS string (including null terminator).
 * @return            Number of bytes copied to the buffer (excluding null terminator).
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_local_server_tls_len(pinggy_ref_t config, pinggy_capa_t buffer_len, pinggy_char_p_t buffer, pinggy_capa_p_t max_len);

//====================================

/**
 * @brief Initializes a new tunnel object using the provided configuration.
 *
 * This function creates a tunnel object but does not connect to the server or set up the tunnel.
 *
 * @param config Reference to the tunnel config object.
 * @return      Reference to the newly created tunnel object.
 */
PINGGY_EXPORT pinggy_ref_t
pinggy_tunnel_initiate(pinggy_ref_t config);

/**
 * @brief Starts and serves the tunnel, blocking indefinitely until stopped.
 *
 * @param tunnel Reference to the tunnel object.
 * @return      pinggy_true on success, pinggy_false on failure.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_start(pinggy_ref_t tunnel);

/**
 * @brief It is similar to resume. However, it also start the the tunnel if not started.
 *
 * @param tunnel Reference to the tunnel object.
 * @return      pinggy_true on success, pinggy_false on failure.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_start_non_blocking(pinggy_ref_t tunnel);

/**
 * @brief Connects and authenticates the tunnel.
 *
 * This function triggers the authentication process and calls the appropriate callbacks if set. After completion, the application must call pinggy_tunnel_resume in a loop to until it receives authentication callbacks.
 *
 * @param tunnel Reference to the tunnel object.
 * @return      pinggy_true if authentication succeeded, pinggy_false otherwise.
 */
// PINGGY_EXPORT pinggy_bool_t
// pinggy_tunnel_connect(pinggy_ref_t tunnel);


/**
 * @brief Connects and authenticates the tunnel (blocking).
 *
 * This function triggers the authentication process and calls the appropriate callbacks if set. Unlike pinggy_tunnel_connect, the application does not need to call pinggy_tunnel_resume after this call.
 *
 * @param tunnel Reference to the tunnel object.
 * @return      pinggy_true if authentication succeeded, pinggy_false otherwise.
 */
// PINGGY_EXPORT pinggy_bool_t
// pinggy_tunnel_connect_blocking(pinggy_ref_t tunnel);

/**
 * @brief Resumes tunnel operation after connect.
 *
 * Applications should call this function in a loop after pinggy_tunnel_connect and pinggy_tunnel_start_forwarding to keep the tunnel active. Returns false when the tunnel should stop.
 *
 * @param tunnel Reference to the tunnel object.
 * @return      pinggy_true to continue, pinggy_false to stop.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_resume(pinggy_ref_t tunnel);

/**
 * @brief Resumes tunnel operation after connect.
 *
 * Applications should call this function in a loop after pinggy_tunnel_connect and pinggy_tunnel_request_primary_forwarding to keep the tunnel active. Returns false when the tunnel should stop.
 * This function also takes a timeout which defines, howlong the sdk may wait. This is the maximum wait time.
 *
 * @param tunnel  Reference to the tunnel object.
 * @param timeout Timeout in ms for the event. negative means it might hold indefinitely. Zero means, it would timeout immediately.
 * @return        pinggy_true to continue, pinggy_false to stop.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_resume_timeout(pinggy_ref_t tunnel, pinggy_int32_t timeout);

/**
 * @brief Stops the running tunnel.
 *
 * This function can be called from a different thread or from within callbacks.
 *
 * @param tunnel Reference to the tunnel object.
 * @return      pinggy_true on success, pinggy_false on failure.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_stop(pinggy_ref_t tunnel);

/**
 * @brief Checks if the tunnel is currently active.
 *
 * @param tunnel Reference to the tunnel object.
 * @return      pinggy_true if the tunnel is active, pinggy_false otherwise.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_is_active(pinggy_ref_t tunnel);

/**
 * @brief Starts a web debugging server managed by the tunnel.
 *
 * @param tunnel         Reference to the tunnel object.
 * @param listening_port Port number to listen on for debugging.
 * @return              The port number actually used for debugging.
 */
PINGGY_EXPORT pinggy_uint16_t
pinggy_tunnel_start_web_debugging(pinggy_ref_t tunnel, pinggy_uint16_t listening_port);

/**
 * @brief Requests remote forwarding from the server.
 *
 * The server will provide a domain name and call the appropriate callbacks. The application must call pinggy_tunnel_resume to receive these callbacks.
 *
 * @param tunnel Reference to the tunnel object.
 */
// PINGGY_EXPORT pinggy_void_t
// pinggy_tunnel_start_forwarding(pinggy_ref_t tunnel);


/**
 * @brief Requests remote forwarding from the server (blocking).
 *
 * The server will provide a domain name and call the appropriate callbacks. The application does not need to call pinggy_tunnel_resume to receive these callbacks.
 *
 * @param tunnel Reference to the tunnel object.
 */
// PINGGY_EXPORT pinggy_void_t
// pinggy_tunnel_start_forwarding_blocking(pinggy_ref_t tunnel);

/**
 * @brief Requests additional remote forwarding from the server.
 *
 * @param tunnel              Reference to the tunnel object.
 * @param remote_binding_url  Null-terminated string specifying the remote binding address. It can contain schema. `forwarding_type` would be derived from the schema if it is empty.
 * @param forward_to          Null-terminated string specifying the local forwarding address.
 * @param forwarding_type     It is equivalen to tunnel_type. It can be one of [http, tcp, tls, tlstcp, udp]
 */
PINGGY_EXPORT pinggy_void_t
pinggy_tunnel_request_additional_forwarding(pinggy_ref_t tunnel, pinggy_const_char_p_t remote_binding_url, pinggy_const_char_p_t forward_to, pinggy_const_char_p_t forwarding_type);

/**
 * @brief Starts continuous usage updates for the tunnel.
 *
 * @param tunnel Reference to the tunnel object.
 */
PINGGY_EXPORT pinggy_void_t
pinggy_tunnel_start_usage_update(pinggy_ref_t tunnel);

/**
 * @brief Stops continuous usage updates for the tunnel.
 *
 * @param tunnel Reference to the tunnel object.
 */
PINGGY_EXPORT pinggy_void_t
pinggy_tunnel_stop_usage_update(pinggy_ref_t tunnel);

/**
 * @brief Retrieves current usage statistics for the tunnel as a JSON string.
 *
 * @param tunnel     Reference to the tunnel object.
 * @param capa       Capacity of the provided buffer.
 * @param val        Pointer to a character buffer to receive the JSON string.
 * @return           Number of bytes copied to the buffer.
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_tunnel_get_current_usages(pinggy_ref_t tunnel, pinggy_capa_t capa, pinggy_char_p_t val);

/**
 * @brief Retrieves current usage statistics for the tunnel as a JSON string, with buffer size information.
 *
 * @param tunnel     Reference to the tunnel object.
 * @param capa       Capacity of the provided buffer.
 * @param val        Pointer to a character buffer to receive the JSON string.
 * @param max_len    Pointer to a variable to receive the required buffer size.
 * @return           Number of bytes copied to the buffer.
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_tunnel_get_current_usages_len(pinggy_ref_t tunnel, pinggy_capa_t capa, pinggy_char_p_t val, pinggy_capa_p_t max_len);

/**
 * @brief Retrieves greeting messages for the tunnel as a JSON string.
 *
 * @param tunnel     Reference to the tunnel object.
 * @param capa       Capacity of the provided buffer.
 * @param val        Pointer to a character buffer to receive the JSON string.
 * @return           Number of bytes copied to the buffer.
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_tunnel_get_greeting_msgs(pinggy_ref_t tunnel, pinggy_capa_t capa, pinggy_char_p_t val);

/**
 * @brief Retrieves greeting messages for the tunnel as a JSON string, with buffer size information.
 *
 * @param tunnel     Reference to the tunnel object.
 * @param capa       Capacity of the provided buffer.
 * @param val        Pointer to a character buffer to receive the JSON string.
 * @param max_len    Pointer to a variable to receive the required buffer size.
 * @return           Number of bytes copied to the buffer.
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_tunnel_get_greeting_msgs_len(pinggy_ref_t tunnel, pinggy_capa_t capa, pinggy_char_p_t val, pinggy_capa_p_t max_len);

PINGGY_EXPORT pinggy_uint16_t
pinggy_tunnel_get_webdebugging_port(pinggy_ref_t tunnel);

PINGGY_EXPORT pinggy_tunnel_state_t
pinggy_tunnel_get_state(pinggy_ref_t tunnel);


//=====================================
//      Callbacks
//=====================================

/**
 * @brief Registers a callback for when the tunnel is successfully connected to the server.
 *
 * @param tunnel    Reference to the tunnel object.
 * @param connected Function pointer for the connected callback.
 * @param user_data User data to be passed to the callback.
 * @return          pinggy_true on success, pinggy_false on failure.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_connected_callback(pinggy_ref_t tunnel, pinggy_on_connected_cb_t connected, pinggy_void_p_t user_data);

/**
 * @brief Registers a callback for when the tunnel is authenticated.
 *
 * @param tunnel         Reference to the tunnel object.
 * @param authenticated  Function pointer for the authenticated callback.
 * @param user_data      User data to be passed to the callback.
 * @return               pinggy_true on success, pinggy_false on failure.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_authenticated_callback(pinggy_ref_t tunnel, pinggy_on_authenticated_cb_t authenticated, pinggy_void_p_t user_data);

/**
 * @brief Registers a callback for when tunnel authentication fails.
 *
 * @param tunnel                Reference to the tunnel object.
 * @param authentication_failed Function pointer for the authentication failed callback.
 * @param user_data             User data to be passed to the callback.
 * @return                      pinggy_true on success, pinggy_false on failure.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_authentication_failed_callback(pinggy_ref_t tunnel, pinggy_on_authentication_failed_cb_t authentication_failed, pinggy_void_p_t user_data);

/**
 * @brief Registers a callback for when primary forwarding is successfully established.
 *
 * @param tunnel                         Reference to the tunnel object.
 * @param forwarding_succeeded   Function pointer for the primary forwarding succeeded callback.
 * @param user_data                      User data to be passed to the callback.
 * @return                               pinggy_true on success, pinggy_false on failure.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_forwarding_succeeded_callback(pinggy_ref_t tunnel, pinggy_on_forwarding_succeeded_cb_t, pinggy_void_p_t user_data);

/**
 * @brief Registers a callback for when primary forwarding fails.
 *
 * @param tunnel                       Reference to the tunnel object.
 * @param forwarding_failed    Function pointer for the primary forwarding failed callback.
 * @param user_data                    User data to be passed to the callback.
 * @return                             pinggy_true on success, pinggy_false on failure.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_forwarding_failed_callback(pinggy_ref_t tunnel, pinggy_on_forwarding_failed_cb_t, pinggy_void_p_t user_data);

/**
 * @brief Registers a callback for when additional forwarding is successfully established.
 *
 * @param tunnel                          Reference to the tunnel object.
 * @param additional_forwarding_succeeded Function pointer for the additional forwarding succeeded callback.
 * @param user_data                       User data to be passed to the callback.
 * @return                                pinggy_true on success, pinggy_false on failure.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_additional_forwarding_succeeded_callback(pinggy_ref_t tunnel, pinggy_on_additional_forwarding_succeeded_cb_t additional_forwarding_succeeded, pinggy_void_p_t user_data);

/**
 * @brief Registers a callback for when additional forwarding fails.
 *
 * @param tunnel                         Reference to the tunnel object.
 * @param additional_forwarding_failed   Function pointer for the additional forwarding failed callback.
 * @param user_data                      User data to be passed to the callback.
 * @return                               pinggy_true on success, pinggy_false on failure.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_additional_forwarding_failed_callback(pinggy_ref_t tunnel, pinggy_on_additional_forwarding_failed_cb_t additional_forwarding_failed, pinggy_void_p_t user_data);

/**
 * @brief Registers a callback for when the forwarding map changes.
 *
 * @param tunnel            Reference to the tunnel object.
 * @param forwarding_changed Function pointer for the forwarding changed callback.
 * @param user_data          User data to be passed to the callback.
 * @return                   pinggy_true on success, pinggy_false on failure.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_forwardings_changed_callback(pinggy_ref_t tunnel, pinggy_on_forwardings_changed_cb_t forwarding_changed, pinggy_void_p_t user_data);

/**
 * @brief Registers a callback for when the tunnel is disconnected.
 *
 * @param tunnel        Reference to the tunnel object.
 * @param disconnected  Function pointer for the disconnected callback.
 * @param user_data     User data to be passed to the callback.
 * @return              pinggy_true on success, pinggy_false on failure.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_disconnected_callback(pinggy_ref_t tunnel, pinggy_on_disconnected_cb_t disconnected, pinggy_void_p_t user_data);

/**
 * @brief Registers a callback for when the tunnel will attempt to reconnect.
 *
 * @param tunnel         Reference to the tunnel object.
 * @param will_reconnect Function pointer for the will reconnect callback.
 * @param user_data      User data to be passed to the callback.
 * @return               pinggy_true on success, pinggy_false on failure.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_will_reconnect_callback(pinggy_ref_t tunnel, pinggy_on_will_reconnect_cb_t will_reconnect, pinggy_void_p_t user_data);

/**
 * @brief Registers a callback for each reconnection attempt.
 *
 * This callback is called just before each reconnection attempt, allowing the application to reset state variables.
 *
 * @param tunnel        Reference to the tunnel object.
 * @param reconnecting  Function pointer for the reconnecting callback.
 * @param user_data     User data to be passed to the callback.
 * @return              pinggy_true on success, pinggy_false on failure.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_reconnecting_callback(pinggy_ref_t tunnel, pinggy_on_reconnecting_cb_t reconnecting, pinggy_void_p_t user_data);

/**
 * @brief Registers a callback for when reconnection is completed successfully.
 *
 * @param tunnel                Reference to the tunnel object.
 * @param reconnection_completed Function pointer for the reconnection completed callback.
 * @param user_data              User data to be passed to the callback.
 * @return                       pinggy_true on success, pinggy_false on failure.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_reconnection_completed_callback(pinggy_ref_t tunnel, pinggy_on_reconnection_completed_cb_t reconnection_completed, pinggy_void_p_t user_data);

/**
 * @brief Registers a callback for when reconnection fails after all attempts.
 *
 * @param tunnel                 Reference to the tunnel object.
 * @param reconnection_failed    Function pointer for the reconnection failed callback.
 * @param user_data              User data to be passed to the callback.
 * @return                       pinggy_true on success, pinggy_false on failure.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_reconnection_failed_callback(pinggy_ref_t tunnel, pinggy_on_reconnection_failed_cb_t reconnection_failed, pinggy_void_p_t user_data);

/**
 * @brief Registers a callback for tunnel errors.
 *
 * @param tunnel       Reference to the tunnel object.
 * @param tunnel_error Function pointer for the tunnel error callback.
 * @param user_data    User data to be passed to the callback.
 * @return             pinggy_true on success, pinggy_false on failure.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_tunnel_error_callback(pinggy_ref_t, pinggy_on_tunnel_error_cb_t, pinggy_void_p_t);

/**
 * @brief Registers a callback for when a new channel is created.
 *
 * @param tunnel      Reference to the tunnel object.
 * @param new_channel Function pointer for the new channel callback.
 * @param user_data   User data to be passed to the callback.
 * @return            pinggy_true on success, pinggy_false on failure.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_new_channel_callback(pinggy_ref_t tunnel, pinggy_on_new_channel_cb_t new_channel, pinggy_void_p_t user_data);

/**
 * @brief Registers a callback for continuous usage updates.
 *
 * @param tunnel         Reference to the tunnel object.
 * @param update         Function pointer for the usage update callback.
 * @param user_data      User data to be passed to the callback.
 * @return               pinggy_true on success, pinggy_false on failure.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_on_usage_update_callback(pinggy_ref_t tunnel, pinggy_on_usage_update_cb_t update, pinggy_void_p_t user_data);

//========================================
//          Channel Functions
//========================================

typedef pinggy_void_t (*pinggy_channel_on_data_received_cb_t)(pinggy_void_p_t, pinggy_ref_t);
typedef pinggy_void_t (*pinggy_channel_on_readyto_send_cb_t)(pinggy_void_p_t, pinggy_ref_t, pinggy_uint32_t);
typedef pinggy_void_t (*pinggy_channel_on_error_cb_t)(pinggy_void_p_t, pinggy_ref_t, pinggy_const_char_p_t, pinggy_len_t);
typedef pinggy_void_t (*pinggy_channel_on_cleanup_cb_t)(pinggy_void_p_t, pinggy_ref_t);




/**
 * @brief Registers a callback to be notified when data is received on a channel.
 *
 * It is safe to call pinggy_tunnel_channel_recv from within this callback.
 *
 * @param channel       Reference to the channel object.
 * @param data_received Function pointer for the data received callback.
 * @param user_data     User data to be passed to the callback.
 * @return              pinggy_true on success, pinggy_false on failure.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_set_on_data_received_callback(pinggy_ref_t channel, pinggy_channel_on_data_received_cb_t data_received, pinggy_void_p_t user_data);

/**
 * @brief Registers a callback to be notified when there is buffer space to send data on a channel.
 *
 * This callback is called whenever the send buffer size changes.
 *
 * @param channel      Reference to the channel object.
 * @param readyto_send Function pointer for the ready to send callback.
 * @param user_data    User data to be passed to the callback.
 * @return             pinggy_true on success, pinggy_false on failure.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_set_on_ready_to_send_callback(pinggy_ref_t channel, pinggy_channel_on_readyto_send_cb_t readyto_send, pinggy_void_p_t user_data);

/**
 * @brief Registers a callback to be notified of channel errors.
 *
 * @param channel   Reference to the channel object.
 * @param error     Function pointer for the error callback.
 * @param user_data User data to be passed to the callback.
 * @return          pinggy_true on success, pinggy_false on failure.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_set_on_error_callback(pinggy_ref_t channel, pinggy_channel_on_error_cb_t error, pinggy_void_p_t user_data);

/**
 * @brief Registers a callback to clean up resources associated with a channel.
 *
 * @param channel   Reference to the channel object.
 * @param cleanup   Function pointer for the cleanup callback.
 * @param user_data User data to be passed to the callback.
 * @return          pinggy_true on success, pinggy_false on failure.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_set_on_cleanup_callback(pinggy_ref_t channel, pinggy_channel_on_cleanup_cb_t cleanup, pinggy_void_p_t user_data);

//====

/**
 * @brief Accepts a new channel for manual handling by the application.
 *
 * If the application accepts the channel, it is responsible for managing it.
 *
 * @param channel Reference to the channel object.
 * @return       pinggy_true on success, pinggy_false on failure.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_accept(pinggy_ref_t channel);

/**
 * @brief Rejects a new channel and cleans up its reference.
 *
 * @param channel Reference to the channel object.
 * @param reason  Null-terminated string specifying the reason for rejection.
 * @return        pinggy_true on success, pinggy_false on failure.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_reject(pinggy_ref_t channel, pinggy_char_p_t reason);

/**
 * @brief Closes the ongoing channel connection.
 *
 * The application must free the channel reference after closing the connection.
 *
 * @param channel Reference to the channel object.
 * @return       pinggy_true on success, pinggy_false on failure.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_close(pinggy_ref_t channel);

/**
 * @brief Sends data to the channel.
 *
 * @param channel  Reference to the channel object.
 * @param data     Pointer to the data to send.
 * @param data_len Length of the data to send.
 * @return         Number of bytes sent, or negative value on error.
 */
PINGGY_EXPORT pinggy_raw_len_t
pinggy_tunnel_channel_send(pinggy_ref_t channel, pinggy_const_char_p_t data, pinggy_raw_len_t data_len);

/**
 * @brief Receives data from the channel.
 *
 * The application may receive less data than requested, even if more data is available.
 *
 * @param channel  Reference to the channel object.
 * @param data     Pointer to the buffer to receive data.
 * @param data_len Length of the buffer.
 * @return         Number of bytes received, or negative value on error.
 */
PINGGY_EXPORT pinggy_raw_len_t
pinggy_tunnel_channel_recv(pinggy_ref_t channel, pinggy_char_p_t data, pinggy_raw_len_t data_len);

/**
 * @brief Checks if the channel has data available to receive.
 *
 * @param channel Reference to the channel object.
 * @return       pinggy_true if data is available, pinggy_false otherwise.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_have_data_to_recv(pinggy_ref_t channel);

/**
 * @brief Checks if the channel has buffer space available to send data.
 *
 * @param channel Reference to the channel object.
 * @return       Number of bytes available in the send buffer.
 */
PINGGY_EXPORT pinggy_uint32_t
pinggy_tunnel_channel_have_buffer_to_send(pinggy_ref_t channel);

/**
 * @brief Checks if the channel is currently connected.
 *
 * @param channel Reference to the channel object.
 * @return       pinggy_true if connected, pinggy_false otherwise.
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_is_connected(pinggy_ref_t channel);

/**
 * @brief Retrieves the type of the channel (UDP or TCP).
 *
 * @param channel Reference to the channel object.
 * @return       Channel type (UDP or TCP).
 */
PINGGY_EXPORT pinggy_uint32_t
pinggy_tunnel_channel_get_type(pinggy_ref_t channel);

/**
 * @brief Retrieves the destination port for the channel (local server port).
 *
 * @param channel Reference to the channel object.
 * @return       Destination port number.
 */
PINGGY_EXPORT pinggy_uint16_t
pinggy_tunnel_channel_get_dest_port(pinggy_ref_t channel);

/**
 * @brief Retrieves the destination host for the channel (local server hostname).
 *
 * @param channel    Reference to the channel object.
 * @param buffer_len Length of the buffer provided for the hostname.
 * @param buffer     Pointer to a character array to receive the hostname.
 * @return           Number of bytes copied to the buffer.
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_tunnel_channel_get_dest_host(pinggy_ref_t channel, pinggy_capa_t buffer_len, pinggy_char_p_t buffer);

/**
 * @brief Retrieves the destination host for the channel, with buffer size information.
 *
 * @param channel    Reference to the channel object.
 * @param buffer_len Length of the buffer provided for the hostname.
 * @param buffer     Pointer to a character array to receive the hostname.
 * @param max_len    Pointer to a variable to receive the required buffer size.
 * @return           Number of bytes copied to the buffer.
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_tunnel_channel_get_dest_host_len(pinggy_ref_t channel, pinggy_capa_t buffer_len, pinggy_char_p_t buffer, pinggy_capa_p_t max_len);

/**
 * @brief Retrieves the source port for the channel.
 *
 * @param channel Reference to the channel object.
 * @return       Source port number.
 */
PINGGY_EXPORT pinggy_uint16_t
pinggy_tunnel_channel_get_src_port(pinggy_ref_t channel);

/**
 * @brief Retrieves the source address for the channel.
 *
 * @param channel    Reference to the channel object.
 * @param buffer_len Length of the buffer provided for the address.
 * @param buffer     Pointer to a character array to receive the address.
 * @return           Number of bytes copied to the buffer.
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_tunnel_channel_get_src_host(pinggy_ref_t channel, pinggy_capa_t buffer_len, pinggy_char_p_t buffer);

/**
 * @brief Retrieves the source address for the channel, with buffer size information.
 *
 * @param channel    Reference to the channel object.
 * @param buffer_len Length of the buffer provided for the address.
 * @param buffer     Pointer to a character array to receive the address.
 * @param max_len    Pointer to a variable to receive the required buffer size.
 * @return           Number of bytes copied to the buffer.
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_tunnel_channel_get_src_host_len(pinggy_ref_t channel, pinggy_capa_t buffer_len, pinggy_char_p_t buffer, pinggy_capa_p_t max_len);

//========================================
//==============================================================

/**
 * @brief Retrieves the Pinggy library version string (from the latest tag).
 *
 * @param capa Capacity of the provided buffer.
 * @param val  Pointer to a character buffer to receive the version string.
 * @return     Number of bytes copied to the buffer.
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_version(pinggy_capa_t capa, pinggy_char_p_t val);

/**
 * @brief Retrieves the Pinggy library version string (from the latest tag), with buffer size information.
 *
 * @param capa    Capacity of the provided buffer.
 * @param val     Pointer to a character buffer to receive the version string.
 * @param max_len Pointer to a variable to receive the required buffer size.
 * @return        Number of bytes copied to the buffer.
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_version_len(pinggy_capa_t capa, pinggy_char_p_t val, pinggy_capa_p_t max_len);

/**
 * @brief Retrieves the exact commit ID for the source at build time.
 *
 * @param capa Capacity of the provided buffer.
 * @param val  Pointer to a character buffer to receive the commit ID.
 * @return     Number of bytes copied to the buffer.
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_git_commit(pinggy_capa_t capa, pinggy_char_p_t val);

/**
 * @brief Retrieves the exact commit ID for the source at build time, with buffer size information.
 *
 * @param capa    Capacity of the provided buffer.
 * @param val     Pointer to a character buffer to receive the commit ID.
 * @param max_len Pointer to a variable to receive the required buffer size.
 * @return        Number of bytes copied to the buffer.
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_git_commit_len(pinggy_capa_t capa, pinggy_char_p_t val, pinggy_capa_p_t max_len);

/**
 * @brief Retrieves the build timestamp for this library.
 *
 * @param capa Capacity of the provided buffer.
 * @param val  Pointer to a character buffer to receive the timestamp.
 * @return     Number of bytes copied to the buffer.
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_build_timestamp(pinggy_capa_t capa, pinggy_char_p_t val);

/**
 * @brief Retrieves the build timestamp for this library, with buffer size information.
 *
 * @param capa    Capacity of the provided buffer.
 * @param val     Pointer to a character buffer to receive the timestamp.
 * @param max_len Pointer to a variable to receive the required buffer size.
 * @return        Number of bytes copied to the buffer.
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_build_timestamp_len(pinggy_capa_t capa, pinggy_char_p_t val, pinggy_capa_p_t max_len);

/**
 * @brief Retrieves the libc version string for the build system.
 *
 * @note The data may be incomplete or incorrect. Allocate a generous buffer as the string can be large.
 *
 * @param capa Capacity of the provided buffer.
 * @param val  Pointer to a character buffer to receive the libc version string.
 * @return     Number of bytes copied to the buffer.
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_libc_version(pinggy_capa_t capa, pinggy_char_p_t val);

/**
 * @brief Retrieves the libc version string for the build system, with buffer size information.
 *
 * @note The data may be incomplete or incorrect. Allocate a generous buffer as the string can be large.
 *
 * @param capa    Capacity of the provided buffer.
 * @param val     Pointer to a character buffer to receive the libc version string.
 * @param max_len Pointer to a variable to receive the required buffer size.
 * @return        Number of bytes copied to the buffer.
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_libc_version_len(pinggy_capa_t capa, pinggy_char_p_t val, pinggy_capa_p_t max_len);

/**
 * @brief Retrieves the operating system information for the build system.
 *
 * @param capa Capacity of the provided buffer.
 * @param val  Pointer to a character buffer to receive the OS information string.
 * @return     Number of bytes copied to the buffer.
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_build_os(pinggy_capa_t capa, pinggy_char_p_t val);

/**
 * @brief Retrieves the operating system information for the build system, with buffer size information.
 *
 * @param capa    Capacity of the provided buffer.
 * @param val     Pointer to a character buffer to receive the OS information string.
 * @param max_len Pointer to a variable to receive the required buffer size.
 * @return        Number of bytes copied to the buffer.
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_build_os_len(pinggy_capa_t capa, pinggy_char_p_t val, pinggy_capa_p_t max_len);
//==============================================================

//==========================================
// FORWARD COMPATIBILITY
//==========================================
#define pinggy_connected_cb_t                                       pinggy_on_connected_cb_t
#define pinggy_authenticated_cb_t                                   pinggy_on_authenticated_cb_t
#define pinggy_authentication_failed_cb_t                           pinggy_on_authentication_failed_cb_t
#define pinggy_forwarding_succeeded_cb_t                            pinggy_on_forwarding_succeeded_cb_t
#define pinggy_forwarding_failed_cb_t                               pinggy_on_forwarding_failed_cb_t
#define pinggy_additional_forwarding_succeeded_cb_t                 pinggy_on_additional_forwarding_succeeded_cb_t
#define pinggy_additional_forwarding_failed_cb_t                    pinggy_on_additional_forwarding_failed_cb_t
#define pinggy_disconnected_cb_t                                    pinggy_on_disconnected_cb_t
#define pinggy_tunnel_error_cb_t                                    pinggy_on_tunnel_error_cb_t
#define pinggy_new_channel_cb_t                                     pinggy_on_new_channel_cb_t
#define pinggy_raise_exception_cb_t                                 pinggy_on_raise_exception_cb_t

#define pinggy_set_exception_callback                               pinggy_set_on_exception_callback
#define pinggy_tunnel_set_connected_callback                        pinggy_tunnel_set_on_connected_callback
#define pinggy_tunnel_set_authenticated_callback                    pinggy_tunnel_set_on_authenticated_callback
#define pinggy_tunnel_set_authentication_failed_callback            pinggy_tunnel_set_on_authentication_failed_callback
#define pinggy_tunnel_set_forwarding_succeeded_callback             pinggy_tunnel_set_on_forwarding_succeeded_callback
#define pinggy_tunnel_set_forwarding_failed_callback                pinggy_tunnel_set_on_forwarding_failed_callback
#define pinggy_tunnel_set_additional_forwarding_succeeded_callback  pinggy_tunnel_set_on_additional_forwarding_succeeded_callback
#define pinggy_tunnel_set_additional_forwarding_failed_callback     pinggy_tunnel_set_on_additional_forwarding_failed_callback
#define pinggy_tunnel_set_disconnected_callback                     pinggy_tunnel_set_on_disconnected_callback
#define pinggy_tunnel_set_tunnel_error_callback                     pinggy_tunnel_set_on_tunnel_error_callback
#define pinggy_tunnel_set_new_channel_callback                      pinggy_tunnel_set_on_new_channel_callback



#ifdef __cplusplus
}
#endif //__cplusplus

#ifndef __cplusplus
// void initi
#endif

#endif // SRC_CPP_SDK_PINGGY_H__
