#include <stdio.h>
#include <pinggy.h>

#define __TO_STR_(x) #x
#define __TO_STR(x) __TO_STR_(x)
#define PRINT_WITH_LINE(...) \
    print ## f(__FILE__ ":" __TO_STR(__LINE__) ":  " __VA_ARGS__);

struct Tunnel {
    pinggy_ref_t                tunnelRef;
};


static pinggy_void_t //(pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_len_t num_urls, pinggy_char_p_p_t urls);
pinggy_tunnel_established_callback(pinggy_void_p_t userData, pinggy_ref_t ref, pinggy_len_t len, pinggy_char_p_p_t arr) {
    PRINT_WITH_LINE("Pinggy %s\n", __func__);
    for(int i = 0; i < len; i++) {
        PRINT_WITH_LINE("   %s\n", arr[i]);
    }
}

static pinggy_void_t
tunnel_failed(pinggy_void_p_t userData, pinggy_ref_t ref, pinggy_const_char_p_t msg) {
    PRINT_WITH_LINE("Pinggy %s: %s\n", __func__, msg);
}

//pinggy_void_p_t user_data, pinggy_ref_t tunnel_ref, pinggy_const_char_p_t error, pinggy_len_t msg_size, pinggy_char_p_p_t msg
static pinggy_void_t
pinggy_disconnected(pinggy_void_p_t userData, pinggy_ref_t ref, pinggy_const_char_p_t error, pinggy_len_t msgLen, pinggy_char_p_p_t msgs) {
    PRINT_WITH_LINE("Pinggy %s: %s\n", __func__, error);
    for (int i = 0; i < msgLen; i++) {
        PRINT_WITH_LINE("\t %s\n", msgs[i]);
    }
}


static pinggy_void_t
pinggy_raise_exception(pinggy_const_char_p_t what, pinggy_const_char_p_t where) {
    PRINT_WITH_LINE("Pinggy %s: %s %s\n", __func__, what, where);
}


int
main() {
    char details[1024];
    int ret = 0;
    ret = pinggy_version(sizeof(details), details);
    PRINT_WITH_LINE("pinggy_version:         %s\n", details);
    ret = pinggy_git_commit(sizeof(details), details);
    PRINT_WITH_LINE("pinggy_git_commit:      %s\n", details);
    ret = pinggy_build_timestamp(sizeof(details), details);
    PRINT_WITH_LINE("pinggy_build_timestamp: %s\n", details);
    ret = pinggy_libc_version(sizeof(details), details);
    PRINT_WITH_LINE("pinggy_libc_version:    %s\n", details);
    ret = pinggy_build_os(sizeof(details), details);
    PRINT_WITH_LINE("pinggy_build_os:        %s\n", details);
    pinggy_set_log_path("/dev/null");
    pinggy_set_on_exception_callback(pinggy_raise_exception);
    pinggy_ref_t config = pinggy_create_config();
    pinggy_config_set_server_address(config, "t.pinggy.io:443");
    pinggy_config_set_sni_server_name(config, "t.pinggy.io");
    pinggy_config_add_forwarding_simple(config, "l:4000");

    // pinggy_config_set_basic_auths(config, "[['user1', 'pass1'], ['user2', 'pass two']]");
    char str[1024];
    pinggy_config_get_argument_len(config, 1024, str, NULL);
    PRINT_WITH_LINE("aasd %s\n", str);

    struct Tunnel tunnel;

    tunnel.tunnelRef = pinggy_tunnel_initiate(config);

    pinggy_tunnel_set_on_tunnel_established_callback(tunnel.tunnelRef, pinggy_tunnel_established_callback, &tunnel);
    pinggy_tunnel_set_on_tunnel_failed_callback(tunnel.tunnelRef, tunnel_failed, &tunnel);
    pinggy_tunnel_start(tunnel.tunnelRef);
    return 0;
}
