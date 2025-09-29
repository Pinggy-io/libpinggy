
### Feat-NewSession
* Every thing seems working fine without any issue.
* Currently I am working on basic stuff only. Not removing primary forwarding yet. I can work on that only after deployment or findout a way to make client backword compatible.
* Added support for protocol based greeting msg and usages info communication instead of extra channel. Made it backward compatible. However, not tested.
* Functions Removed:
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
* Modified Callback signature:
    1. pinggy_on_additional_forwarding_succeeded_cb_t
    1. pinggy_on_additional_forwarding_failed_cb_t
* Functions Added:
    1. pinggy_config_add_forwarding
    1. pinggy_config_add_forwarding_simple
    1. pinggy_config_set_forwardings
    1. pinggy_config_reset_forwardings
    1. pinggy_config_get_forwardings
    1. pinggy_config_get_forwardings_len