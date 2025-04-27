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
 * @brief A brief overview of libpinggy.
 *
 * `libpinggy` is a library that provides the Pinggy SDK for use with various
 * applications. It includes functionalities for tunnel creation and management.
 * Pinggy is a tunneling service provider that exposes local servers (HTTP, TLS, TCP, and UDP)
 * to the Internet. The SDK provides a simple yet powerful `C` interface, allowing
 * applications to set up tunnels directly within the application.
 *
 * ## Key Points
 * - **Reference**:
 *   The SDK uses the concept of references to manage internal objects. References are
 *   randomized unsigned 32-bit integers that refer to objects within the library.
 *   Applications must free a reference using the `pinggy_free_ref` function. All
 *   interfaces use references to identify objects.
 *
 * - **Thread Safety**:
 *   The Pinggy SDK does not use threads for internal tasks on any platform. It is a
 *   single-threaded library. However, an application can run one tunnel per thread.
 *   Tunnels are completely isolated. Operations on tunnels are also thread-safe
 *   when performed via the provided interfaces.
 *
 * ## Usage
 * ```c
 * #include "mylibrary.h"
 *
 * int main() {
 *     MyClass obj;
 *     obj.performTask();
 *     return 0;
 * }
 * ```
 *
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

typedef uint8_t       pinggy_bool_t;
typedef uint32_t      pinggy_ref_t;
typedef char         *pinggy_char_p_t;
typedef char        **pinggy_char_p_p_t;
typedef void          pinggy_void_t;
typedef void         *pinggy_void_p_t;
typedef const char   *pinggy_const_char_p_t;
typedef const int     pinggy_const_int_t;
typedef const         pinggy_bool_t pinggy_const_bool_t;
typedef int           pinggy_int_t;
typedef int16_t       pinggy_len_t;
typedef uint32_t      pinggy_capa_t;
typedef uint32_t      pinggy_uint32_t;
typedef uint16_t      pinggy_uint16_t;
typedef int32_t       pinggy_raw_len_t;

#define pinggy_true 1
#define pinggy_false 0


#ifdef PINGGY_TYPETEST_ENABLED
#define int             Error
#define bool_t          Error
#define char_p_t        Error
#define char_p_p_t      Error
#define void_t          Error
#define const_char_p_t  Error
#define const_int_t     Error
#define const_bool_t    Error
#define int_t           Error
#define void            Error
#define len_t           Error
#endif //PINGGY_NO_TYPETEST

#define INVALID_PINGGY_REF 0


PINGGY_EXPORT pinggy_void_t
pinggy_set_log_path(pinggy_char_p_t);

PINGGY_EXPORT pinggy_void_t
pinggy_set_log_enable(pinggy_bool_t);

//================
typedef pinggy_void_t (*pinggy_authenticated_cb_t)(pinggy_void_p_t, pinggy_ref_t);
typedef pinggy_void_t (*pinggy_authentication_failed_cb_t)(pinggy_void_p_t, pinggy_ref_t, pinggy_len_t, pinggy_char_p_p_t);
typedef pinggy_void_t (*pinggy_primary_forwarding_succeeded_cb_t)(pinggy_void_p_t, pinggy_ref_t, pinggy_len_t, pinggy_char_p_p_t);
typedef pinggy_void_t (*pinggy_primary_forwarding_failed_cb_t)(pinggy_void_p_t, pinggy_ref_t, pinggy_const_char_p_t);
typedef pinggy_void_t (*pinggy_additional_forwarding_succeeded_cb_t)(pinggy_void_p_t, pinggy_ref_t, pinggy_const_char_p_t, pinggy_const_char_p_t);
typedef pinggy_void_t (*pinggy_additional_forwarding_failed_cb_t)(pinggy_void_p_t, pinggy_ref_t, pinggy_const_char_p_t, pinggy_const_char_p_t, pinggy_const_char_p_t);
typedef pinggy_void_t (*pinggy_disconnected_cb_t)(pinggy_void_p_t, pinggy_ref_t, pinggy_const_char_p_t, pinggy_len_t, pinggy_char_p_p_t);
typedef pinggy_void_t (*pinggy_tunnel_error_cb_t)(pinggy_void_p_t, pinggy_ref_t, pinggy_uint32_t, pinggy_const_char_p_t, pinggy_bool_t);
typedef pinggy_bool_t (*pinggy_new_channel_cb_t)(pinggy_void_p_t, pinggy_ref_t, pinggy_ref_t);
typedef pinggy_void_t (*pinggy_raise_exception_cb_t)(pinggy_const_char_p_t, pinggy_const_char_p_t);
//================

/**
 * @brief  Set a function pointer which would handle exception occured inside the library
 * @param  pointer to function with type pinggy_raise_exception_cb_t
 * @return
 */
PINGGY_EXPORT pinggy_void_t
pinggy_set_exception_callback(pinggy_raise_exception_cb_t);

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
 * @brief Set the pinggy server address. by default it is set to `a.pinggy.io:443`
 * @param config  reference to tunnel config
 * @param server_address  a null terminated string in the format `<server>:<port>`
 * @return
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_server_address(pinggy_ref_t config, pinggy_char_p_t server_address);

/**
 * @brief Set the secrete token. sdk does not verify the token locally. It directly send it the server.
 * @param config  reference to tunnel config
 * @param token  a null terminated string
 * @return
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_token(pinggy_ref_t config, pinggy_char_p_t token);

/**
 * @brief Set the tunnel type. the value must be among `tcp`, `http` or `tls`
 * @param config  reference to tunnel config
 * @param type
 * @return
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_type(pinggy_ref_t config, pinggy_char_p_t type);

/**
 * @brief Set the tunnel udp type
 * @param config  reference to tunnel config
 * @param udp_type
 * @return
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_udp_type(pinggy_ref_t config, pinggy_char_p_t udp_type);

/**
 * @brief Set the local server address. It is required even if the
 * application handles forwading by it self.
 * @param config  reference to tunnel config
 * @param tcp_forward_to  a null terminated string containing local tcp server address in the format `<server>:<port>`
 * @return
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_tcp_forward_to(pinggy_ref_t config, pinggy_char_p_t tcp_forward_to);

/**
 * @brief Similar to `pinggy_config_set_tcp_forward_to`
 * @param config  reference to tunnel config
 * @param udp_forward_to  a null terminated string containing local udp server address in the format `<server>:<port>`
 * @return
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_udp_forward_to(pinggy_ref_t config, pinggy_char_p_t udp_forward_to);

/**
 * @brief Set or reset force login enable. More detail at pinggy.io
 * @param config  reference to tunnel config
 * @param force
 * @return
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_force(pinggy_ref_t config, pinggy_bool_t force);

/**
 * @brief Set the command line argument for other setting. It is similar ssh command.
 * @param config  reference to tunnel config
 * @param argument
 * @return
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_argument(pinggy_ref_t config, pinggy_char_p_t argument);

/**
 * @brief Set avdance parsing for http tunnel. It is enabled by default. free http tunnel does not work without advanced parsing
 * @param config  reference to tunnel config
 * @param advanced_parsing
 * @return
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_advanced_parsing(pinggy_ref_t config, pinggy_bool_t advanced_parsing);

/**
 * @brief Set whether to use ssl connection for tunnel setup or. It is by default enabled. This is feature for pinggy developer.
 * @param config  reference to tunnel config
 * @param ssl
 * @return
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_ssl(pinggy_ref_t config, pinggy_bool_t ssl);

/**
 * @brief Another developer only config
 * @param config  reference to tunnel config
 * @param sni_server_name
 * @return
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_sni_server_name(pinggy_ref_t config, pinggy_char_p_t sni_server_name);

/**
 * @brief Another developer only config
 * @param config  reference to tunnel config
 * @param insecure
 * @return
 */
PINGGY_EXPORT pinggy_void_t
pinggy_config_set_insecure(pinggy_ref_t config, pinggy_bool_t insecure);

/**
 * @brief Get the current server address configured in the config
 * @param config  reference to tunnel config
 * @param buffer_len  lenth of the buffer where server address would be copied.
 * @param buffer  pointer to a character array
 * @return lenth of server address copied to the buffer
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_server_address(pinggy_ref_t config, pinggy_capa_t buffer_len, pinggy_char_p_t buffer);

/**
 * @brief Get the current token configure in the config
 * @param config  reference to tunnel config
 * @param buffer_len  lenth of the buffer where token would be copied.
 * @param buffer  pointer to a character array
 * @return lenth of token copied to the buffer
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_token(pinggy_ref_t config, pinggy_capa_t buffer_len, pinggy_char_p_t buffer);

/**
 * @brief Get the current tcp tunnel type
 * @param config  reference to tunnel config
 * @param buffer_len  lenth of the buffer where type would be copied.
 * @param buffer  pointer to a character array
 * @return lenth of type copied to the buffer
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_type(pinggy_ref_t config, pinggy_capa_t buffer_len, pinggy_char_p_t buffer);

/**
 * @brief Get the current udp tunnel type
 * @param config  reference to tunnel config
 * @param buffer_len  lenth of the buffer where udp_type would be copied.
 * @param buffer  pointer to a character array
 * @return lenth of udp_type copied to the buffer
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_udp_type(pinggy_ref_t config, pinggy_capa_t buffer_len, pinggy_char_p_t buffer);

/**
 * @brief Get the current tcp forwarding address
 * @param config  reference to tunnel config
 * @param buffer_len  lenth of the buffer where forwarding address would be copied.
 * @param buffer  pointer to a character array
 * @return lenth of forwarding address copied to the buffer
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_tcp_forward_to(pinggy_ref_t config, pinggy_capa_t buffer_len, pinggy_char_p_t buffer);

/**
 * @brief Get the current udp forwarding address
 * @param config  reference to tunnel config
 * @param buffer_len  lenth of the buffer where forwarding address would be copied.
 * @param buffer  pointer to a character array
 * @return lenth of forwarding address copied to the buffer
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_udp_forward_to(pinggy_ref_t config, pinggy_capa_t buffer_len, pinggy_char_p_t buffer);

/**
 * @brief Get whether force is enabled or not
 * @param config  reference to tunnel config
 * @return return whether force is enabled or not
 */
PINGGY_EXPORT pinggy_const_bool_t
pinggy_config_get_force(pinggy_ref_t config);

/**
 * @brief Get the current arguments
 * @param config  reference to tunnel config
 * @param buffer_len  lenth of the buffer where arguments would be copied.
 * @param buffer  pointer to a character array
 * @return lenth of arguments copied to the buffer
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_argument(pinggy_ref_t config, pinggy_capa_t buffer_len, pinggy_char_p_t buffer);

/**
 * @brief Get whether advanced parsing is enabled or not
 * @param config  reference to tunnel config
 * @return return whether advanced parsing is enabled or not
 */
PINGGY_EXPORT pinggy_const_bool_t
pinggy_config_get_advanced_parsing(pinggy_ref_t config);

/**
 * @brief Get whether ssl is enabled or not
 * @param config  reference to tunnel config
 * @return return whether ssl is enabled or not
 */
PINGGY_EXPORT pinggy_const_bool_t
pinggy_config_get_ssl(pinggy_ref_t config);

/**
 * @brief Get the current sni server name
 * @param config  reference to tunnel config
 * @param buffer_len  lenth of the buffer where sni server name would be copied.
 * @param buffer  pointer to a character array
 * @return lenth of sni server name copied to the buffer
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_config_get_sni_server_name(pinggy_ref_t config, pinggy_capa_t buffer_len, pinggy_char_p_t buffer);

/**
 * @brief Get whether insecure ssl is enabled or not
 * @param config  reference to tunnel config
 * @return return whether insecure ssl is enabled or not
 */
PINGGY_EXPORT pinggy_const_bool_t
pinggy_config_get_insecure(pinggy_ref_t config);

//====================================

/**
 * @brief Tunnel initialization function. It initialize a tunnel object. However, it does not connect to the server, that is it does not setup the tunnel.
 * @param config tunnel config reference
 * @return the reference to newly created tunnel object
 */
PINGGY_EXPORT pinggy_ref_t
pinggy_tunnel_initiate(pinggy_ref_t config);

/**
 * @brief Start and serve the tunnel. It will block indefinitly.
 * @param tunnel
 * @return return whether is was success or not
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_start(pinggy_ref_t tunnel);

/**
 * @brief Connect and authenticate the connection. It would call `authenticate` callback or `authentication_failed` callback if callbacks are setup.
 * Once this function is complected, application needs to call `pinggy_tunnel_resume` in infinite loop to continue tunnel.
 * @param tunnel
 * @return whether authentication was successfull or not
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_connect(pinggy_ref_t tunnel);

/**
 * @brief This is the resume function. Applications are expected to call this function in infinite loop.
 * Application should check return value. If it returns negetive integer and errno not set to EINTR, loop
 * should break. It would work only with `pinggy_tunnel_connect`
 * @param tunnel
 * @return
 */
PINGGY_EXPORT pinggy_int_t
pinggy_tunnel_resume(pinggy_ref_t tunnel);

/**
 * @brief Stop the running tunnel. It can be called from a different thread or from the callbacks
 * @param tunnel
 * @return returns success or not
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_stop(pinggy_ref_t tunnel);

/**
 * @brief Check if the tunnel is active or not.
 * @param tunnel
 * @return returns true if active
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_is_active(pinggy_ref_t tunnel);

/**
 * @brief Start webdebugging server.The tunnel would manage the connection.
 * @param tunnel
 * @param listening_port
 * @return
 */
PINGGY_EXPORT pinggy_uint16_t
pinggy_tunnel_start_web_debugging(pinggy_ref_t tunnel, pinggy_uint16_t listening_port);

/**
 * @brief Request remote forwarding. It would request the server to start the forwarding.
 * Server would in return provide the domain name. It also call tunnel_initiated and tunnel_initiation_failed
 * callbacks
 * @param tunnel
 * @return
 */
PINGGY_EXPORT pinggy_void_t
pinggy_tunnel_request_primary_forwarding(pinggy_ref_t tunnel);

/**
 * @brief
 * @param tunnel
 * @param remote_binding_addr
 * @param forward_to
 * @return
 */
PINGGY_EXPORT pinggy_void_t
pinggy_tunnel_request_additional_forwarding(pinggy_ref_t, pinggy_const_char_p_t, pinggy_const_char_p_t);

//=====================================

/**
 * @brief Set authention completed callback
 * @param tunnel
 * @param authenticated function pointer for handling authenticated
 * @param user_data user data that will pass when library call this call back
 * @return
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_authenticated_callback(pinggy_ref_t tunnel, pinggy_authenticated_cb_t authenticated, pinggy_void_p_t user_data);

/**
 * @brief Set authentication failed callback
 * @param tunnel
 * @param authentication_failed
 * @param user_data user data that will pass when library call this call back
 * @return
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_authentication_failed_callback(pinggy_ref_t tunnel, pinggy_authentication_failed_cb_t authentication_failed, pinggy_void_p_t user_data);

/**
 * @brief Set primary_forwarding_succeeded callback
 * @param tunnel
 * @param primary_forwarding_succeeded callback
 * @param user_data
 * @return
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_primary_forwarding_succeeded_callback(pinggy_ref_t tunnel, pinggy_primary_forwarding_succeeded_cb_t, pinggy_void_p_t user_data);

/**
 * @brief Set primary_forwarding_failed callback
 * @param tunnel
 * @param primary_forwarding_failed callback
 * @param user_data
 * @return
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_primary_forwarding_failed_callback(pinggy_ref_t tunnel, pinggy_primary_forwarding_failed_cb_t, pinggy_void_p_t user_data);

/**
 * @brief Set additional_forwarding_succeeded callback
 * @param tunnel
 * @param additional_forwarding_succeeded
 * @param user_data user data that will pass when library call this call back
 * @return
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_additional_forwarding_succeeded_callback(pinggy_ref_t tunnel, pinggy_additional_forwarding_succeeded_cb_t additional_forwarding_succeeded, pinggy_void_p_t user_data);

/**
 * @brief Set additional_forwarding_failed callback
 * @param tunnel
 * @param additional_forwarding_failed
 * @param user_data user data that will pass when library call this call back
 * @return
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_additional_forwarding_failed_callback(pinggy_ref_t tunnel, pinggy_additional_forwarding_failed_cb_t additional_forwarding_failed, pinggy_void_p_t user_data);

/**
 * @brief tunnel disconnected callback
 * @param tunnel
 * @param disconnected
 * @param user_data user data that will pass when library call this call back
 * @return
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_disconnected_callback(pinggy_ref_t tunnel, pinggy_disconnected_cb_t disconnected, pinggy_void_p_t user_data);

/**
 * @brief set error handler
 * @param tunnel
 * @param tunnel_error
 * @param user_data
 * @return
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_tunnel_error_callback(pinggy_ref_t, pinggy_tunnel_error_cb_t, pinggy_void_p_t);

/**
 * @brief Set new channel callback
 * @param tunnel
 * @param new_channel
 * @param user_data user data that will pass when library call this call back
 * @return
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_set_new_channel_callback(pinggy_ref_t tunnel, pinggy_new_channel_cb_t new_channel, pinggy_void_p_t user_data);

//========================================
//          Channel Functions
//========================================

typedef pinggy_void_t (*pinggy_channel_data_received_cb_t)(pinggy_void_p_t, pinggy_ref_t);
typedef pinggy_void_t (*pinggy_channel_readyto_send_cb_t)(pinggy_void_p_t, pinggy_ref_t, pinggy_uint32_t);
typedef pinggy_void_t (*pinggy_channel_error_cb_t)(pinggy_void_p_t, pinggy_ref_t, pinggy_const_char_p_t, pinggy_len_t);
typedef pinggy_void_t (*pinggy_channel_cleanup_cb_t)(pinggy_void_p_t, pinggy_ref_t);


/**
 * @brief Setup callback to get notified when data is received. It is safe to call `pinggy_tunnel_channel_recv` from here
 * @param channel
 * @param data_received
 * @param user_data user data that will pass when library call this call back
 * @return
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_set_data_received_callback(pinggy_ref_t channel, pinggy_channel_data_received_cb_t data_received, pinggy_void_p_t user_data);

/**
 * @brief Setup a callback to get notified when there is buffer to write data. It would get called whenever the send buffer size changes
 * @param channel
 * @param readyto_send
 * @param user_data user data that will pass when library call this call back
 * @return
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_set_ready_to_send_callback(pinggy_ref_t channel, pinggy_channel_readyto_send_cb_t readyto_send, pinggy_void_p_t user_data);

/**
 * @brief call back to provide errors to the application
 * @param channel
 * @param error
 * @param user_data user data that will pass when library call this call back
 * @return
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_set_error_callback(pinggy_ref_t channel, pinggy_channel_error_cb_t error, pinggy_void_p_t user_data);

/**
 * @brief Callback to tell application to cleanup resources associated with this channel
 * @param channel
 * @param cleanup
 * @param user_data user data that will pass when library call this call back
 * @return
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_set_cleanup_callback(pinggy_ref_t channel, pinggy_channel_cleanup_cb_t cleanup, pinggy_void_p_t user_data);

//====

/**
 * @brief Accept the new channel. If the application accepts the channel, it needs to handle it.
 * @param channel
 * @return
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_accept(pinggy_ref_t channel);

/**
 * @brief Reject the new channel. The reference would be cleaned up as well.
 * @param channel
 * @param reason
 * @return
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_reject(pinggy_ref_t channel, pinggy_char_p_t reason);

/**
 * @brief Close the ongoing connection. Application needs to free the reference after closing the connection
 * @param channel
 * @return
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_close(pinggy_ref_t channel);

/**
 * @brief Send data to the channel
 * @param channel
 * @param data
 * @param data_len
 * @return
 */
PINGGY_EXPORT pinggy_raw_len_t
pinggy_tunnel_channel_send(pinggy_ref_t channel, pinggy_const_char_p_t data, pinggy_raw_len_t data_len);

/**
 * @brief Receive data from the channel. It might happen that application
 * receives less data than what it has requested even if channel has more data.
 * @param channel
 * @param data
 * @param data_len
 * @return
 */
PINGGY_EXPORT pinggy_raw_len_t
pinggy_tunnel_channel_recv(pinggy_ref_t channel, pinggy_char_p_t data, pinggy_raw_len_t data_len);

/**
 * @brief Check if channel has data to recv
 * @param channel
 * @return
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_have_data_to_recv(pinggy_ref_t channel);

/**
 * @brief Check if channel has buffer to send
 * @param channel
 * @return
 */
PINGGY_EXPORT pinggy_uint32_t
pinggy_tunnel_channel_have_buffer_to_send(pinggy_ref_t channel);

/**
 * @brief Check if channel is connected
 * @param channel
 * @return
 */
PINGGY_EXPORT pinggy_bool_t
pinggy_tunnel_channel_is_connected(pinggy_ref_t channel);

/**
 * @brief Check the channel type. It would be udp or tcp.
 * @param channel
 * @return
 */
PINGGY_EXPORT pinggy_uint32_t
pinggy_tunnel_channel_get_type(pinggy_ref_t channel);

/**
 * @brief Get connect to port means where to connect i.e. local server port
 * @param channel
 * @return
 */
PINGGY_EXPORT pinggy_uint16_t
pinggy_tunnel_channel_get_dest_port(pinggy_ref_t channel);

/**
 * @brief Get connect to host i.e. local server hostname
 * @param channel
 * @param buffer_len
 * @param buffer
 * @return
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_tunnel_channel_get_dest_host(pinggy_ref_t channel, pinggy_capa_t buffer_len, pinggy_char_p_t buffer);

/**
 * @brief Get the source port.
 * @param channel
 * @return
 */
PINGGY_EXPORT pinggy_uint16_t
pinggy_tunnel_channel_get_src_port(pinggy_ref_t channel);

/**
 * @brief Get source address
 * @param channel
 * @param buffer_len
 * @param buffer
 * @return
 */
PINGGY_EXPORT pinggy_const_int_t
pinggy_tunnel_channel_get_src_host(pinggy_ref_t channel, pinggy_capa_t buffer_len, pinggy_char_p_t buffer);

//========================================
//==============================================================
#define DECLARE_CONFIG_GET_FUNC(funcname) \
PINGGY_EXPORT pinggy_const_int_t \
funcname(pinggy_capa_t capa, pinggy_char_p_t val)

DECLARE_CONFIG_GET_FUNC(pinggy_version);
DECLARE_CONFIG_GET_FUNC(pinggy_git_commit);
DECLARE_CONFIG_GET_FUNC(pinggy_build_timestamp);
DECLARE_CONFIG_GET_FUNC(pinggy_libc_version);
DECLARE_CONFIG_GET_FUNC(pinggy_build_os);

#undef DECLARE_CONFIG_GET_FUNC
//==============================================================


#ifdef __cplusplus
}
#endif //__cplusplus

#ifndef __cplusplus
// void initi
#endif

#endif // SRC_CPP_SDK_PINGGY_H__
