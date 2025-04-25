import ctypes
import threading
from .loader import cdll

pinggy_thread_local_data = threading.local()

def pinggy_error_check(a, b, c):
    err = None
    try:
        err = pinggy_thread_local_data.value
        if err is not None:
            pinggy_thread_local_data.value = None
    except Exception:
        pass

    if err is not None:
        raise Exception(err)
    return a

#========
pinggy_bool_t                               = ctypes.c_bool
pinggy_ref_t                                = ctypes.c_uint32
pinggy_char_p_t                             = ctypes.c_char_p
pinggy_char_p_p_t                           = ctypes.POINTER(ctypes.c_char_p)
pinggy_void_t                               = None
pinggy_void_p_t                             = ctypes.c_void_p
pinggy_const_char_p_t                       = ctypes.c_char_p
pinggy_const_int_t                          = ctypes.c_int
pinggy_const_bool_t                         = ctypes.c_bool
pinggy_int_t                                = ctypes.c_int
pinggy_len_t                                = ctypes.c_int16
pinggy_capa_t                               = ctypes.c_uint32
pinggy_uint32_t                             = ctypes.c_uint32
pinggy_uint16_t                             = ctypes.c_uint16
pinggy_raw_len_t                            = ctypes.c_int32

pinggy_authenticated_cb_t                   = ctypes.CFUNCTYPE(pinggy_void_t, pinggy_void_p_t, pinggy_ref_t)
pinggy_authentication_failed_cb_t           = ctypes.CFUNCTYPE(pinggy_void_t, pinggy_void_p_t, pinggy_ref_t, pinggy_len_t, pinggy_char_p_p_t)
pinggy_primary_forwarding_succeeded_cb_t    = ctypes.CFUNCTYPE(pinggy_void_t, pinggy_void_p_t, pinggy_ref_t, pinggy_len_t, pinggy_char_p_p_t)
pinggy_primary_forwarding_failed_cb_t       = ctypes.CFUNCTYPE(pinggy_void_t, pinggy_void_p_t, pinggy_ref_t, pinggy_const_char_p_t)
pinggy_additional_forwarding_succeeded_cb_t = ctypes.CFUNCTYPE(pinggy_void_t, pinggy_void_p_t, pinggy_ref_t, pinggy_const_char_p_t, pinggy_const_char_p_t)
pinggy_additional_forwarding_failed_cb_t    = ctypes.CFUNCTYPE(pinggy_void_t, pinggy_void_p_t, pinggy_ref_t, pinggy_const_char_p_t, pinggy_const_char_p_t, pinggy_const_char_p_t)
pinggy_disconnected_cb_t                    = ctypes.CFUNCTYPE(pinggy_void_t, pinggy_void_p_t, pinggy_ref_t, pinggy_const_char_p_t, pinggy_len_t, pinggy_char_p_p_t)
pinggy_tunnel_error_cb_t                    = ctypes.CFUNCTYPE(pinggy_void_t, pinggy_void_p_t, pinggy_ref_t, pinggy_uint32_t, pinggy_char_p_t, pinggy_bool_t)
pinggy_new_channel_cb_t                     = ctypes.CFUNCTYPE(pinggy_bool_t, pinggy_void_p_t, pinggy_ref_t, pinggy_ref_t)
pinggy_raise_exception_cb_t                 = ctypes.CFUNCTYPE(pinggy_void_t, pinggy_const_char_p_t, pinggy_const_char_p_t)


pinggy_channel_data_received_cb_t           = ctypes.CFUNCTYPE(pinggy_void_t, pinggy_void_p_t, pinggy_ref_t)
pinggy_channel_ready_to_send_cb_t           = ctypes.CFUNCTYPE(pinggy_void_t, pinggy_void_p_t, pinggy_ref_t, pinggy_uint32_t)
pinggy_channel_error_cb_t                   = ctypes.CFUNCTYPE(pinggy_void_t, pinggy_void_p_t, pinggy_ref_t, pinggy_const_char_p_t, pinggy_len_t)
pinggy_channel_cleanup_cb_t                 = ctypes.CFUNCTYPE(pinggy_void_t, pinggy_void_p_t, pinggy_ref_t)

#========


pinggy_set_log_path                                         = cdll.pinggy_set_log_path
pinggy_set_log_enable                                       = cdll.pinggy_set_log_enable
pinggy_set_exception_callback                               = cdll.pinggy_set_exception_callback
pinggy_free_ref                                             = cdll.pinggy_free_ref
pinggy_create_config                                        = cdll.pinggy_create_config
pinggy_config_set_server_address                            = cdll.pinggy_config_set_server_address
pinggy_config_set_token                                     = cdll.pinggy_config_set_token
pinggy_config_set_type                                      = cdll.pinggy_config_set_type
pinggy_config_set_udp_type                                  = cdll.pinggy_config_set_udp_type
pinggy_config_set_tcp_forward_to                            = cdll.pinggy_config_set_tcp_forward_to
pinggy_config_set_udp_forward_to                            = cdll.pinggy_config_set_udp_forward_to
pinggy_config_set_force                                     = cdll.pinggy_config_set_force
pinggy_config_set_argument                                  = cdll.pinggy_config_set_argument
pinggy_config_set_advanced_parsing                          = cdll.pinggy_config_set_advanced_parsing
pinggy_config_set_ssl                                       = cdll.pinggy_config_set_ssl
pinggy_config_set_sni_server_name                           = cdll.pinggy_config_set_sni_server_name
pinggy_config_set_insecure                                  = cdll.pinggy_config_set_insecure
pinggy_config_get_server_address                            = cdll.pinggy_config_get_server_address
pinggy_config_get_token                                     = cdll.pinggy_config_get_token
pinggy_config_get_type                                      = cdll.pinggy_config_get_type
pinggy_config_get_udp_type                                  = cdll.pinggy_config_get_udp_type
pinggy_config_get_tcp_forward_to                            = cdll.pinggy_config_get_tcp_forward_to
pinggy_config_get_udp_forward_to                            = cdll.pinggy_config_get_udp_forward_to
pinggy_config_get_force                                     = cdll.pinggy_config_get_force
pinggy_config_get_argument                                  = cdll.pinggy_config_get_argument
pinggy_config_get_advanced_parsing                          = cdll.pinggy_config_get_advanced_parsing
pinggy_config_get_ssl                                       = cdll.pinggy_config_get_ssl
pinggy_config_get_sni_server_name                           = cdll.pinggy_config_get_sni_server_name
pinggy_config_get_insecure                                  = cdll.pinggy_config_get_insecure
pinggy_tunnel_set_authenticated_callback                    = cdll.pinggy_tunnel_set_authenticated_callback
pinggy_tunnel_set_authentication_failed_callback            = cdll.pinggy_tunnel_set_authentication_failed_callback
pinggy_tunnel_set_primary_forwarding_succeeded_callback     = cdll.pinggy_tunnel_set_primary_forwarding_succeeded_callback
pinggy_tunnel_set_primary_forwarding_failed_callback        = cdll.pinggy_tunnel_set_primary_forwarding_failed_callback
pinggy_tunnel_set_additional_forwarding_succeeded_callback  = cdll.pinggy_tunnel_set_additional_forwarding_succeeded_callback
pinggy_tunnel_set_additional_forwarding_failed_callback     = cdll.pinggy_tunnel_set_additional_forwarding_failed_callback
pinggy_tunnel_set_disconnected_callback                     = cdll.pinggy_tunnel_set_disconnected_callback
pinggy_tunnel_set_tunnel_error_callback                     = cdll.pinggy_tunnel_set_tunnel_error_callback
pinggy_tunnel_set_new_channel_callback                      = cdll.pinggy_tunnel_set_new_channel_callback
pinggy_tunnel_initiate                                      = cdll.pinggy_tunnel_initiate
pinggy_tunnel_start                                         = cdll.pinggy_tunnel_start
pinggy_tunnel_connect                                       = cdll.pinggy_tunnel_connect
pinggy_tunnel_resume                                        = cdll.pinggy_tunnel_resume
pinggy_tunnel_stop                                          = cdll.pinggy_tunnel_stop
pinggy_tunnel_start_web_debugging                           = cdll.pinggy_tunnel_start_web_debugging
pinggy_tunnel_request_primary_forwarding                    = cdll.pinggy_tunnel_request_primary_forwarding
pinggy_tunnel_request_additional_forwarding                 = cdll.pinggy_tunnel_request_additional_forwarding
pinggy_tunnel_channel_set_data_received_callback            = cdll.pinggy_tunnel_channel_set_data_received_callback
pinggy_tunnel_channel_set_ready_to_send_callback            = cdll.pinggy_tunnel_channel_set_ready_to_send_callback
pinggy_tunnel_channel_set_error_callback                    = cdll.pinggy_tunnel_channel_set_error_callback
pinggy_tunnel_channel_set_cleanup_callback                  = cdll.pinggy_tunnel_channel_set_cleanup_callback
pinggy_tunnel_channel_accept                                = cdll.pinggy_tunnel_channel_accept
pinggy_tunnel_channel_reject                                = cdll.pinggy_tunnel_channel_reject
pinggy_tunnel_channel_close                                 = cdll.pinggy_tunnel_channel_close
pinggy_tunnel_channel_send                                  = cdll.pinggy_tunnel_channel_send
pinggy_tunnel_channel_recv                                  = cdll.pinggy_tunnel_channel_recv
pinggy_tunnel_channel_have_data_to_recv                     = cdll.pinggy_tunnel_channel_have_data_to_recv
pinggy_tunnel_channel_have_buffer_to_send                   = cdll.pinggy_tunnel_channel_have_buffer_to_send
pinggy_tunnel_channel_is_connected                          = cdll.pinggy_tunnel_channel_is_connected
pinggy_tunnel_channel_get_type                              = cdll.pinggy_tunnel_channel_get_type
pinggy_tunnel_channel_get_dest_port                         = cdll.pinggy_tunnel_channel_get_dest_port
pinggy_tunnel_channel_get_dest_host                         = cdll.pinggy_tunnel_channel_get_dest_host
pinggy_tunnel_channel_get_src_port                          = cdll.pinggy_tunnel_channel_get_src_port
pinggy_tunnel_channel_get_src_host                          = cdll.pinggy_tunnel_channel_get_src_host
pinggy_version                                              = cdll.pinggy_version
pinggy_git_commit                                           = cdll.pinggy_git_commit
pinggy_build_timestamp                                      = cdll.pinggy_build_timestamp
pinggy_libc_version                                         = cdll.pinggy_libc_version
pinggy_build_os                                             = cdll.pinggy_build_os


#==========
pinggy_set_log_path.errcheck                                        = pinggy_error_check
pinggy_set_log_enable.errcheck                                      = pinggy_error_check
pinggy_set_exception_callback.errcheck                              = pinggy_error_check
pinggy_free_ref.errcheck                                            = pinggy_error_check
pinggy_create_config.errcheck                                       = pinggy_error_check
pinggy_config_set_server_address.errcheck                           = pinggy_error_check
pinggy_config_set_token.errcheck                                    = pinggy_error_check
pinggy_config_set_type.errcheck                                     = pinggy_error_check
pinggy_config_set_udp_type.errcheck                                 = pinggy_error_check
pinggy_config_set_tcp_forward_to.errcheck                           = pinggy_error_check
pinggy_config_set_udp_forward_to.errcheck                           = pinggy_error_check
pinggy_config_set_force.errcheck                                    = pinggy_error_check
pinggy_config_set_argument.errcheck                                 = pinggy_error_check
pinggy_config_set_advanced_parsing.errcheck                         = pinggy_error_check
pinggy_config_set_ssl.errcheck                                      = pinggy_error_check
pinggy_config_set_sni_server_name.errcheck                          = pinggy_error_check
pinggy_config_set_insecure.errcheck                                 = pinggy_error_check
pinggy_config_get_server_address.errcheck                           = pinggy_error_check
pinggy_config_get_token.errcheck                                    = pinggy_error_check
pinggy_config_get_type.errcheck                                     = pinggy_error_check
pinggy_config_get_udp_type.errcheck                                 = pinggy_error_check
pinggy_config_get_tcp_forward_to.errcheck                           = pinggy_error_check
pinggy_config_get_udp_forward_to.errcheck                           = pinggy_error_check
pinggy_config_get_force.errcheck                                    = pinggy_error_check
pinggy_config_get_argument.errcheck                                 = pinggy_error_check
pinggy_config_get_advanced_parsing.errcheck                         = pinggy_error_check
pinggy_config_get_ssl.errcheck                                      = pinggy_error_check
pinggy_config_get_sni_server_name.errcheck                          = pinggy_error_check
pinggy_config_get_insecure.errcheck                                 = pinggy_error_check
pinggy_tunnel_set_authenticated_callback.errcheck                   = pinggy_error_check
pinggy_tunnel_set_authentication_failed_callback.errcheck           = pinggy_error_check
pinggy_tunnel_set_primary_forwarding_succeeded_callback.errcheck    = pinggy_error_check
pinggy_tunnel_set_primary_forwarding_failed_callback.errcheck       = pinggy_error_check
pinggy_tunnel_set_additional_forwarding_succeeded_callback.errcheck = pinggy_error_check
pinggy_tunnel_set_additional_forwarding_failed_callback.errcheck    = pinggy_error_check
pinggy_tunnel_set_disconnected_callback.errcheck                    = pinggy_error_check
pinggy_tunnel_set_tunnel_error_callback.errcheck                    = pinggy_error_check
pinggy_tunnel_set_new_channel_callback.errcheck                     = pinggy_error_check
pinggy_tunnel_initiate.errcheck                                     = pinggy_error_check
pinggy_tunnel_start.errcheck                                        = pinggy_error_check
pinggy_tunnel_connect.errcheck                                      = pinggy_error_check
pinggy_tunnel_resume.errcheck                                       = pinggy_error_check
pinggy_tunnel_stop.errcheck                                         = pinggy_error_check
pinggy_tunnel_start_web_debugging.errcheck                          = pinggy_error_check
pinggy_tunnel_request_primary_forwarding.errcheck                   = pinggy_error_check
pinggy_tunnel_request_additional_forwarding.errcheck                = pinggy_error_check
#========
pinggy_set_log_path.restype                                         = pinggy_void_t
pinggy_set_log_enable.restype                                       = pinggy_void_t
pinggy_set_exception_callback.restype                               = pinggy_void_t
pinggy_free_ref.restype                                             = pinggy_bool_t
pinggy_create_config.restype                                        = pinggy_ref_t
pinggy_config_set_server_address.restype                            = pinggy_void_t
pinggy_config_set_token.restype                                     = pinggy_void_t
pinggy_config_set_type.restype                                      = pinggy_void_t
pinggy_config_set_udp_type.restype                                  = pinggy_void_t
pinggy_config_set_tcp_forward_to.restype                            = pinggy_void_t
pinggy_config_set_udp_forward_to.restype                            = pinggy_void_t
pinggy_config_set_force.restype                                     = pinggy_void_t
pinggy_config_set_argument.restype                                  = pinggy_void_t
pinggy_config_set_advanced_parsing.restype                          = pinggy_void_t
pinggy_config_set_ssl.restype                                       = pinggy_void_t
pinggy_config_set_sni_server_name.restype                           = pinggy_void_t
pinggy_config_set_insecure.restype                                  = pinggy_void_t
pinggy_config_get_server_address.restype                            = pinggy_const_int_t
pinggy_config_get_token.restype                                     = pinggy_const_int_t
pinggy_config_get_type.restype                                      = pinggy_const_int_t
pinggy_config_get_udp_type.restype                                  = pinggy_const_int_t
pinggy_config_get_tcp_forward_to.restype                            = pinggy_const_int_t
pinggy_config_get_udp_forward_to.restype                            = pinggy_const_int_t
pinggy_config_get_force.restype                                     = pinggy_const_bool_t
pinggy_config_get_argument.restype                                  = pinggy_const_int_t
pinggy_config_get_advanced_parsing.restype                          = pinggy_const_bool_t
pinggy_config_get_ssl.restype                                       = pinggy_const_bool_t
pinggy_config_get_sni_server_name.restype                           = pinggy_const_int_t
pinggy_config_get_insecure.restype                                  = pinggy_const_bool_t
pinggy_tunnel_set_authenticated_callback.restype                    = pinggy_bool_t
pinggy_tunnel_set_authentication_failed_callback.restype            = pinggy_bool_t
pinggy_tunnel_set_primary_forwarding_succeeded_callback.restype     = pinggy_bool_t
pinggy_tunnel_set_primary_forwarding_failed_callback.restype        = pinggy_bool_t
pinggy_tunnel_set_additional_forwarding_succeeded_callback.restype  = pinggy_bool_t
pinggy_tunnel_set_additional_forwarding_failed_callback.restype     = pinggy_bool_t
pinggy_tunnel_set_disconnected_callback.restype                     = pinggy_bool_t
pinggy_tunnel_set_tunnel_error_callback.restype                     = pinggy_bool_t
pinggy_tunnel_set_new_channel_callback.restype                      = pinggy_bool_t
pinggy_tunnel_initiate.restype                                      = pinggy_ref_t
pinggy_tunnel_start.restype                                         = pinggy_bool_t
pinggy_tunnel_connect.restype                                       = pinggy_bool_t
pinggy_tunnel_resume.restypes                                       = pinggy_int_t
pinggy_tunnel_stop.restypes                                         = pinggy_bool_t
pinggy_tunnel_start_web_debugging.restype                           = pinggy_uint16_t
pinggy_tunnel_request_primary_forwarding.restype                    = pinggy_void_t
pinggy_tunnel_request_additional_forwarding.restype                 = pinggy_void_t
#========
pinggy_set_log_path.argtypes                                        = [pinggy_char_p_t]
pinggy_set_log_enable.argtypes                                      = [pinggy_bool_t]
pinggy_set_exception_callback.argtypes                              = [pinggy_raise_exception_cb_t]
pinggy_free_ref.argtypes                                            = [pinggy_ref_t]
pinggy_create_config.argtypes                                       = []
pinggy_config_set_server_address.argtypes                           = [pinggy_ref_t, pinggy_char_p_t]
pinggy_config_set_token.argtypes                                    = [pinggy_ref_t, pinggy_char_p_t]
pinggy_config_set_type.argtypes                                     = [pinggy_ref_t, pinggy_char_p_t]
pinggy_config_set_udp_type.argtypes                                 = [pinggy_ref_t, pinggy_char_p_t]
pinggy_config_set_tcp_forward_to.argtypes                           = [pinggy_ref_t, pinggy_char_p_t]
pinggy_config_set_udp_forward_to.argtypes                           = [pinggy_ref_t, pinggy_char_p_t]
pinggy_config_set_force.argtypes                                    = [pinggy_ref_t, pinggy_bool_t]
pinggy_config_set_argument.argtypes                                 = [pinggy_ref_t, pinggy_char_p_t]
pinggy_config_set_advanced_parsing.argtypes                         = [pinggy_ref_t, pinggy_bool_t]
pinggy_config_set_ssl.argtypes                                      = [pinggy_ref_t, pinggy_bool_t]
pinggy_config_set_sni_server_name.argtypes                          = [pinggy_ref_t, pinggy_char_p_t]
pinggy_config_set_insecure.argtypes                                 = [pinggy_ref_t, pinggy_bool_t]
pinggy_config_get_server_address.argtypes                           = [pinggy_ref_t, pinggy_capa_t, pinggy_char_p_t]
pinggy_config_get_token.argtypes                                    = [pinggy_ref_t, pinggy_capa_t, pinggy_char_p_t]
pinggy_config_get_type.argtypes                                     = [pinggy_ref_t, pinggy_capa_t, pinggy_char_p_t]
pinggy_config_get_udp_type.argtypes                                 = [pinggy_ref_t, pinggy_capa_t, pinggy_char_p_t]
pinggy_config_get_tcp_forward_to.argtypes                           = [pinggy_ref_t, pinggy_capa_t, pinggy_char_p_t]
pinggy_config_get_udp_forward_to.argtypes                           = [pinggy_ref_t, pinggy_capa_t, pinggy_char_p_t]
pinggy_config_get_force.argtypes                                    = [pinggy_ref_t]
pinggy_config_get_argument.argtypes                                 = [pinggy_ref_t, pinggy_capa_t, pinggy_char_p_t]
pinggy_config_get_advanced_parsing.argtypes                         = [pinggy_ref_t]
pinggy_config_get_ssl.argtypes                                      = [pinggy_ref_t]
pinggy_config_get_sni_server_name.argtypes                          = [pinggy_ref_t, pinggy_capa_t, pinggy_char_p_t]
pinggy_config_get_insecure.argtypes                                 = [pinggy_ref_t]
pinggy_tunnel_set_authenticated_callback.argtypes                   = [pinggy_ref_t, pinggy_authenticated_cb_t, pinggy_void_p_t]
pinggy_tunnel_set_authentication_failed_callback.argtypes           = [pinggy_ref_t, pinggy_authentication_failed_cb_t, pinggy_void_p_t]
pinggy_tunnel_set_primary_forwarding_succeeded_callback.argtypes    = [pinggy_ref_t, pinggy_primary_forwarding_succeeded_cb_t, pinggy_void_p_t]
pinggy_tunnel_set_primary_forwarding_failed_callback.argtypes       = [pinggy_ref_t, pinggy_primary_forwarding_failed_cb_t, pinggy_void_p_t]
pinggy_tunnel_set_additional_forwarding_succeeded_callback.argtypes = [pinggy_ref_t, pinggy_additional_forwarding_succeeded_cb_t, pinggy_void_p_t]
pinggy_tunnel_set_additional_forwarding_failed_callback.argtypes    = [pinggy_ref_t, pinggy_additional_forwarding_failed_cb_t, pinggy_void_p_t]
pinggy_tunnel_set_disconnected_callback.argtypes                    = [pinggy_ref_t, pinggy_disconnected_cb_t, pinggy_void_p_t]
pinggy_tunnel_set_tunnel_error_callback.argtypes                    = [pinggy_ref_t, pinggy_tunnel_error_cb_t, pinggy_void_p_t]
pinggy_tunnel_set_new_channel_callback.argtypes                     = [pinggy_ref_t, pinggy_new_channel_cb_t, pinggy_void_p_t]
pinggy_tunnel_initiate.argtypes                                     = [pinggy_ref_t]
pinggy_tunnel_start.argtypes                                        = [pinggy_ref_t]
pinggy_tunnel_connect.argtypes                                      = [pinggy_ref_t]
pinggy_tunnel_resume.argtypes                                       = [pinggy_ref_t]
pinggy_tunnel_stop.argtypes                                         = [pinggy_ref_t]
pinggy_tunnel_start_web_debugging.argtypes                          = [pinggy_ref_t, pinggy_uint16_t]
pinggy_tunnel_request_primary_forwarding.argtypes                   = [pinggy_ref_t]
pinggy_tunnel_request_additional_forwarding.argtypes                = [pinggy_ref_t, pinggy_const_char_p_t, pinggy_const_char_p_t]

#========
#========

pinggy_tunnel_channel_set_data_received_callback.errcheck           = pinggy_error_check
pinggy_tunnel_channel_set_ready_to_send_callback.errcheck           = pinggy_error_check
pinggy_tunnel_channel_set_error_callback.errcheck                   = pinggy_error_check
pinggy_tunnel_channel_set_cleanup_callback.errcheck                 = pinggy_error_check

pinggy_tunnel_channel_set_data_received_callback.restype            = pinggy_bool_t
pinggy_tunnel_channel_set_ready_to_send_callback.restype            = pinggy_bool_t
pinggy_tunnel_channel_set_error_callback.restype                    = pinggy_bool_t
pinggy_tunnel_channel_set_cleanup_callback.restype                  = pinggy_bool_t

pinggy_tunnel_channel_set_data_received_callback.argtypes           = [pinggy_ref_t, pinggy_channel_data_received_cb_t, pinggy_void_p_t]
pinggy_tunnel_channel_set_ready_to_send_callback.argtypes           = [pinggy_ref_t, pinggy_channel_ready_to_send_cb_t, pinggy_void_p_t]
pinggy_tunnel_channel_set_error_callback.argtypes                   = [pinggy_ref_t, pinggy_channel_error_cb_t, pinggy_void_p_t]
pinggy_tunnel_channel_set_cleanup_callback.argtypes                 = [pinggy_ref_t, pinggy_channel_cleanup_cb_t, pinggy_void_p_t]
#========

pinggy_tunnel_channel_accept.errcheck                               = pinggy_error_check
pinggy_tunnel_channel_reject.errcheck                               = pinggy_error_check
pinggy_tunnel_channel_close.errcheck                                = pinggy_error_check
pinggy_tunnel_channel_send.errcheck                                 = pinggy_error_check
pinggy_tunnel_channel_recv.errcheck                                 = pinggy_error_check
pinggy_tunnel_channel_have_data_to_recv.errcheck                    = pinggy_error_check
pinggy_tunnel_channel_have_buffer_to_send.errcheck                  = pinggy_error_check
pinggy_tunnel_channel_is_connected.errcheck                         = pinggy_error_check
pinggy_tunnel_channel_get_type.errcheck                             = pinggy_error_check
pinggy_tunnel_channel_get_dest_port.errcheck                        = pinggy_error_check
pinggy_tunnel_channel_get_dest_host.errcheck                        = pinggy_error_check
pinggy_tunnel_channel_get_src_port.errcheck                         = pinggy_error_check
pinggy_tunnel_channel_get_src_host.errcheck                         = pinggy_error_check
#========

pinggy_tunnel_channel_accept.restype                                = pinggy_bool_t
pinggy_tunnel_channel_reject.restype                                = pinggy_bool_t
pinggy_tunnel_channel_close.restype                                 = pinggy_bool_t
pinggy_tunnel_channel_send.restype                                  = pinggy_raw_len_t
pinggy_tunnel_channel_recv.restype                                  = pinggy_raw_len_t
pinggy_tunnel_channel_have_data_to_recv.restype                     = pinggy_bool_t
pinggy_tunnel_channel_have_buffer_to_send.restype                   = pinggy_uint32_t
pinggy_tunnel_channel_is_connected.restype                          = pinggy_bool_t
pinggy_tunnel_channel_get_type.restype                              = pinggy_uint32_t
pinggy_tunnel_channel_get_dest_port.restype                         = pinggy_uint16_t
pinggy_tunnel_channel_get_dest_host.restype                         = pinggy_const_int_t
pinggy_tunnel_channel_get_src_port.restype                          = pinggy_uint16_t
pinggy_tunnel_channel_get_src_host.restype                          = pinggy_const_int_t

#========

pinggy_tunnel_channel_accept.argtypes                               = [pinggy_ref_t]
pinggy_tunnel_channel_reject.argtypes                               = [pinggy_ref_t, pinggy_char_p_t]
pinggy_tunnel_channel_close.argtypes                                = [pinggy_ref_t]
pinggy_tunnel_channel_send.argtypes                                 = [pinggy_ref_t, pinggy_const_char_p_t, pinggy_raw_len_t]
pinggy_tunnel_channel_recv.argtypes                                 = [pinggy_ref_t, pinggy_char_p_t, pinggy_raw_len_t]
pinggy_tunnel_channel_have_data_to_recv.argtypes                    = [pinggy_ref_t]
pinggy_tunnel_channel_have_buffer_to_send.argtypes                  = [pinggy_ref_t]
pinggy_tunnel_channel_is_connected.argtypes                         = [pinggy_ref_t]
pinggy_tunnel_channel_get_type.argtypes                             = [pinggy_ref_t]
pinggy_tunnel_channel_get_dest_port.argtypes                        = [pinggy_ref_t]
pinggy_tunnel_channel_get_dest_host.argtypes                        = [pinggy_ref_t, pinggy_capa_t, pinggy_char_p_t]
pinggy_tunnel_channel_get_src_port.argtypes                         = [pinggy_ref_t]
pinggy_tunnel_channel_get_src_host.argtypes                         = [pinggy_ref_t, pinggy_capa_t, pinggy_char_p_t]
#========
pinggy_version.errcheck                                             = pinggy_error_check
pinggy_git_commit.errcheck                                          = pinggy_error_check
pinggy_build_timestamp.errcheck                                     = pinggy_error_check
pinggy_libc_version.errcheck                                        = pinggy_error_check
pinggy_build_os.errcheck                                            = pinggy_error_check

pinggy_version.restype                                              = pinggy_const_int_t
pinggy_git_commit.restype                                           = pinggy_const_int_t
pinggy_build_timestamp.restype                                      = pinggy_const_int_t
pinggy_libc_version.restype                                         = pinggy_const_int_t
pinggy_build_os.restype                                             = pinggy_const_int_t

pinggy_version.argtypes                                             = [pinggy_capa_t, pinggy_char_p_t]
pinggy_git_commit.argtypes                                          = [pinggy_capa_t, pinggy_char_p_t]
pinggy_build_timestamp.argtypes                                     = [pinggy_capa_t, pinggy_char_p_t]
pinggy_libc_version.argtypes                                        = [pinggy_capa_t, pinggy_char_p_t]
pinggy_build_os.argtypes                                            = [pinggy_capa_t, pinggy_char_p_t]
#========

def pinggy_raise_exception(etype, ewhat):
    global pinggy_thread_local_data
    # print("Exception")
    pinggy_thread_local_data.value = etype.decode('utf-8') + "what: " + ewhat.decode('utf-8')
    # print("Seting up value: ", pinggy_thread_local_data.value)
    # raise Exception(etype.decode('utf-8') + "what: " + ewhat.decode('utf-8'))

pinggy_raise_exception = pinggy_raise_exception_cb_t(pinggy_raise_exception)

cdll.pinggy_set_exception_callback(pinggy_raise_exception)

def _getStringArray(l, arr):
    return [arr[i].decode('utf-8') for i in range(l)]

def _get_string_via_cfunc(func, *arg):
    buffer_size = 1024
    buffer = ctypes.create_string_buffer(buffer_size)
    ln = func(*arg, buffer_size, buffer)
    res = buffer.value.decode('utf-8') if ln != 0 else ""
    return res
