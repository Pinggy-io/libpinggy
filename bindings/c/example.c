#include <stdio.h>
#include <pinggy.h>

struct Tunnel {
    pinggy_ref_t                tunnelRef;
};

static pinggy_void_t
pinggy_authenticated(pinggy_void_p_t userData, pinggy_ref_t ref) {
    struct Tunnel *tunnel = (struct Tunnel *)userData;
    if (tunnel->tunnelRef != ref)
        printf("tunnel not matching\n");
    printf("Pinggy %s\n", __func__);
}

static pinggy_void_t
pinggy_authentication_failed(pinggy_void_p_t userData, pinggy_ref_t ref, pinggy_len_t, pinggy_char_p_p_t) {
    printf("Pinggy %s\n", __func__);
}

static pinggy_void_t
pinggy_primary_forwarding_succeeded(pinggy_void_p_t userData, pinggy_ref_t ref, pinggy_len_t len, pinggy_char_p_p_t arr) {
    printf("Pinggy %s\n", __func__);
    for(int i = 0; i < len; i++) {
        printf("   %s\n", arr[i]);
    }
}

static pinggy_void_t
pinggy_primary_forwrding_failed(pinggy_void_p_t userData, pinggy_ref_t ref, pinggy_const_char_p_t) {
    printf("Pinggy %s\n", __func__);
}

static pinggy_void_t
pinggy_disconnected(pinggy_void_p_t userData, pinggy_ref_t ref, pinggy_const_char_p_t, pinggy_len_t, pinggy_char_p_p_t) {
    printf("Pinggy %s\n", __func__);
}

static pinggy_bool_t
pinggy_new_channel(pinggy_void_p_t userData, pinggy_ref_t ref, pinggy_ref_t) {
    printf("Pinggy %s\n", __func__);
    return pinggy_false;
}

static pinggy_void_t
pinggy_raise_exception(pinggy_const_char_p_t, pinggy_const_char_p_t) {
    printf("Pinggy %s\n", __func__);
}


int
main() {
    char details[1024];
    int ret = 0;
    ret = pinggy_version(sizeof(details), details);
    printf("pinggy_version:         %s\n", details);
    ret = pinggy_git_commit(sizeof(details), details);
    printf("pinggy_git_commit:      %s\n", details);
    ret = pinggy_build_timestamp(sizeof(details), details);
    printf("pinggy_build_timestamp: %s\n", details);
    ret = pinggy_libc_version(sizeof(details), details);
    printf("pinggy_libc_version:    %s\n", details);
    ret = pinggy_build_os(sizeof(details), details);
    printf("pinggy_build_os:        %s\n", details);
    pinggy_set_log_path("/dev/null");
    pinggy_set_on_exception_callback(pinggy_raise_exception);
    pinggy_ref_t config = pinggy_create_config();
    pinggy_config_set_server_address(config, "t.pinggy.io:443");
    pinggy_config_set_sni_server_name(config, "t.pinggy.io");
    // pinggy_config_set_server_address(config, "localhost:7878");
    // pinggy_config_set_sni_server_name(config, "example.com");
    // pinggy_config_set_insecure(config, pinggy_true);
    pinggy_config_set_tcp_forward_to(config, "l:4000");

    pinggy_config_set_basic_auths(config, "[['user1', 'pass1'], ['user2', 'pass two']]");
    char str[1024];
    pinggy_config_get_argument_len(config, 1024, str, NULL);
    printf("aasd %s\n", str);

    struct Tunnel tunnel;

    tunnel.tunnelRef = pinggy_tunnel_initiate(config);
    pinggy_tunnel_set_on_authenticated_callback(tunnel.tunnelRef, pinggy_authenticated, &tunnel);
    pinggy_tunnel_set_on_authentication_failed_callback(tunnel.tunnelRef, pinggy_authentication_failed, &tunnel);
    pinggy_tunnel_set_on_primary_forwarding_succeeded_callback(tunnel.tunnelRef, pinggy_primary_forwarding_succeeded, &tunnel);
    pinggy_tunnel_set_primary_forwarding_failed_callback(tunnel.tunnelRef, pinggy_primary_forwrding_failed, &tunnel);
    pinggy_tunnel_start(tunnel.tunnelRef);
    return 0;
}
