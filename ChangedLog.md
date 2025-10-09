
### Feat-NewSession
* Every thing seems working fine without any issue.
* Currently I am working on basic stuff only. Not removing primary forwarding yet. I can work on that only after deployment or findout a way to make client backword compatible.
* Added support for protocol based greeting msg and usages info communication instead of extra channel. Made it backward compatible. However, not tested.
* Functions removed:
    1. pinggy_config_set_type
    1. pinggy_config_set_udp_type
    1. pinggy_config_set_tcp_forward_to
    1. pinggy_config_set_udp_forward_to
    1. pinggy_config_get_type
    1. pinggy_config_get_type_len
    1. pinggy_config_get_udp_type
    1. pinggy_config_get_udp_type_len
    1. pinggy_config_get_tcp_forward_to
    1. pinggy_config_get_tcp_forward_to_len
    1. pinggy_config_get_udp_forward_to
    1. pinggy_config_get_udp_forward_to_len
    1. pinggy_tunnel_connect
    1. pinggy_tunnel_connect_blocking
    1. pinggy_tunnel_start_forwarding
    1. pinggy_tunnel_start_forwarding_blocking
    1. pinggy_tunnel_set_on_connected_callback
    1. pinggy_tunnel_set_on_authenticated_callback
    1. pinggy_tunnel_set_on_authentication_failed_callback
* Functions added:
    1. pinggy_config_add_forwarding
    1. pinggy_config_add_forwarding_simple
    1. pinggy_config_set_forwardings
    1. pinggy_config_reset_forwardings
    1. pinggy_config_get_forwardings
    1. pinggy_config_get_forwardings_len
    1. pinggy_tunnel_start_non_blocking
    1. pinggy_tunnel_get_state
    1. pinggy_config_get_webdebugger_addr
    1. pinggy_config_get_webdebugger
    1. pinggy_config_set_webdebugger_addr
    1. pinggy_config_set_webdebugger
    1. pinggy_tunnel_get_webdebugging_addr
    1. pinggy_tunnel_get_webdebugging_addr_len
* Functions renamed:
    1. pinggy_tunnel_request_primary_forwarding_blocking -> pinggy_tunnel_start_forwarding_blocking
    1. pinggy_tunnel_request_primary_forwarding -> pinggy_tunnel_start_forwarding
    1. pinggy_tunnel_set_primary_forwarding_succeeded_callback -> pinggy_tunnel_set_on_forwarding_succeeded_callback -> pinggy_tunnel_set_on_tunnel_established_callback
    1. pinggy_tunnel_set_primary_forwarding_failed_callback -> pinggy_tunnel_set_on_forwarding_failed_callback -> pinggy_tunnel_set_on_tunnel_failed_callback
    1. pinggy_tunnel_set_on_forwarding_changed_callback -> pinggy_tunnel_set_on_forwardings_changed_callback
* Function signature changed:
    1. pinggy_tunnel_start_web_debugging
* Callback removed:
    1. pinggy_on_connected_cb_t
    1. pinggy_on_authenticated_cb_t
    1. pinggy_on_authentication_failed_cb_t
* Modified callback signature:
    1. pinggy_on_additional_forwarding_succeeded_cb_t
    1. pinggy_on_additional_forwarding_failed_cb_t
* Renamed callback signature:
    1. pinggy_on_primary_forwarding_succeeded_cb_t -> pinggy_on_forwarding_succeeded_cb_t -> pinggy_on_tunnel_established_cb_t
    1. pinggy_on_primary_forwarding_failed_cb_t -> pinggy_on_forwarding_failed_cb_t -> pinggy_on_tunnel_failed_cb_t
    1. pinggy_on_forwarding_changed_cb_t -> pinggy_on_forwardings_changed_cb_t