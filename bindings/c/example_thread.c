#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pinggy.h>

#define NUM_THREADS 2


struct Tunnel {
    pinggy_ref_t                tunnelRef;
};

static pinggy_void_t
pinggy_authenticated(pinggy_void_p_t userData, pinggy_ref_t ref) {
    struct Tunnel *tunnel = (struct Tunnel *)userData;
    if (tunnel->tunnelRef != ref)
        printf("tunnel not matching\n");
    printf("Pinggy[%d] %s\n", tunnel->tunnelRef, __func__);
}

static pinggy_void_t
pinggy_authentication_failed(pinggy_void_p_t userData, pinggy_ref_t ref, pinggy_len_t len, pinggy_char_p_p_t arr) {
    struct Tunnel *tunnel = (struct Tunnel *)userData;
    if (tunnel->tunnelRef != ref)
        printf("tunnel not matching\n");
    printf("Pinggy[%d] %s\n", tunnel->tunnelRef, __func__);
    printf("Auth failed for %d reasons:\n", len);
    for(int i = 0; i < len; i++) {
        printf("   %s\n", arr[i]);
    }
}

static pinggy_void_t
pinggy_primary_forwarding_succeeded(pinggy_void_p_t userData, pinggy_ref_t ref, pinggy_len_t len, pinggy_char_p_p_t arr) {
    struct Tunnel *tunnel = (struct Tunnel *)userData;
    if (tunnel->tunnelRef != ref)
        printf("tunnel not matching\n");
    printf("Pinggy[%d] %s\n", tunnel->tunnelRef, __func__);
    for(int i = 0; i < len; i++) {
        printf("   %s\n", arr[i]);
    }
}

static pinggy_void_t
pinggy_primary_forwrding_failed(pinggy_void_p_t userData, pinggy_ref_t ref, pinggy_const_char_p_t) {
    struct Tunnel *tunnel = (struct Tunnel *)userData;
    if (tunnel->tunnelRef != ref)
        printf("tunnel not matching\n");
    printf("Pinggy[%d] %s\n", tunnel->tunnelRef, __func__);
}

static pinggy_void_t
pinggy_disconnected(pinggy_void_p_t userData, pinggy_ref_t ref, pinggy_const_char_p_t, pinggy_len_t, pinggy_char_p_p_t) {
    struct Tunnel *tunnel = (struct Tunnel *)userData;
    if (tunnel->tunnelRef != ref)
        printf("tunnel not matching\n");
    printf("Pinggy[%d] %s\n", tunnel->tunnelRef, __func__);
}

static pinggy_bool_t
pinggy_new_channel(pinggy_void_p_t userData, pinggy_ref_t ref, pinggy_ref_t) {
    struct Tunnel *tunnel = (struct Tunnel *)userData;
    if (tunnel->tunnelRef != ref)
        printf("tunnel not matching\n");
    printf("Pinggy[%d] %s\n", tunnel->tunnelRef, __func__);
    return pinggy_false;
}

static pinggy_void_t
pinggy_raise_exception(pinggy_const_char_p_t what, pinggy_const_char_p_t why) {
    printf("Pinggy %s\n", __func__);
    printf("`%s`: `%s`\n", what, why);
}


void
startTunnel()
{

    pinggy_ref_t config = pinggy_create_config();
    pinggy_config_set_server_address(config, "t.pinggy.io:443");
    pinggy_config_set_sni_server_name(config, "t.pinggy.io");
    pinggy_config_set_tcp_forward_to(config, "l:4000");

    struct Tunnel tunnel;

    tunnel.tunnelRef = pinggy_tunnel_initiate(config);
    pinggy_tunnel_set_on_authenticated_callback(tunnel.tunnelRef, pinggy_authenticated, &tunnel);
    pinggy_tunnel_set_on_authentication_failed_callback(tunnel.tunnelRef, pinggy_authentication_failed, &tunnel);
    pinggy_tunnel_set_on_primary_forwarding_succeeded_callback(tunnel.tunnelRef, pinggy_primary_forwarding_succeeded, &tunnel);
    pinggy_tunnel_set_primary_forwarding_failed_callback(tunnel.tunnelRef, pinggy_primary_forwrding_failed, &tunnel);
    pinggy_tunnel_start(tunnel.tunnelRef);
}

// Function executed by each thread
void* threadFunction(void* arg) {
    int threadId = *(int*)arg;  // Get thread ID
    printf("Thread %d: Hello from the thread!\n", threadId);

    // Simulate some work
    startTunnel();

    printf("Thread %d: Goodbye from the thread!\n", threadId);
    return NULL;
}

int main() {

    // pinggy_set_log_path("/dev/null");
    pthread_t threads[NUM_THREADS];  // Array to hold thread identifiers
    int threadIds[NUM_THREADS];     // Array to hold thread IDs
    int result;

    pinggy_set_on_exception_callback(pinggy_raise_exception);

    for (int i = 0; i < NUM_THREADS; i++) {
        threadIds[i] = i;
        printf("Main: Creating thread %d\n", i);

        // Create a new thread
        result = pthread_create(&threads[i], NULL, threadFunction, &threadIds[i]);
        if (result) {
            fprintf(stderr, "Error: Unable to create thread %d\n", i);
            exit(EXIT_FAILURE);
        }
    }

    // Wait for all threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        printf("Main: Thread %d has completed\n", i);
    }

    printf("Main: All threads are done. Exiting program.\n");
    return 0;
}
