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

#include "network.h"

#include <sys/types.h>
#ifndef __WINDOWS_OS__
#include <sys/socket.h>
#include <netdb.h>
#include <stddef.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <sys/stat.h>
#else
#include <ws2tcpip.h>
#endif
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "assert_pinggy.h"
#include "log.h"


#define MAXLINE 2048
#define CONTROL_LEN 1024

#ifdef __WINDOWS_OS__
int issockoptsuccess(int opret) {return opret != SOCKET_ERROR;}
int app_socketpair(int domain, int type, int protocol, sock_t sv[2])
{
    struct sockaddr_in addr;
    sock_t listener;
    int addrlen = sizeof(addr);
    u_long mode = 1;

    if (domain == AF_INET6 || domain == AF_UNIX) {
        domain = AF_INET;
    }

    if (protocol || domain != AF_INET || type != SOCK_STREAM) {
        WSASetLastError(WSAEAFNOSUPPORT);
        return -1;
    }

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == INVALID_SOCKET) {
        return -1;
    }

    ZeroMemory(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0; // Any free port

    if (bind(listener, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(listener);
        return -1;
    }

    if (listen(listener, 1) == SOCKET_ERROR) {
        closesocket(listener);
        return -1;
    }

    if (getsockname(listener, (struct sockaddr*)&addr, &addrlen) == SOCKET_ERROR) {
        closesocket(listener);
        return -1;
    }

    sv[0] = socket(AF_INET, SOCK_STREAM, 0);
    if (sv[0] == INVALID_SOCKET) {
        closesocket(listener);
        return -1;
    }

    ioctlsocket(sv[0], FIONBIO, &mode); // Non-blocking mode

    if (connect(sv[0], (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        if (WSAGetLastError() != WSAEWOULDBLOCK) {
            closesocket(sv[0]);
            closesocket(listener);
            return -1;
        }
    }

    sv[1] = accept(listener, NULL, NULL);
    if (sv[1] == INVALID_SOCKET) {
        closesocket(sv[0]);
        closesocket(listener);
        return -1;
    }

    closesocket(listener);

    mode = 0;
    ioctlsocket(sv[0], FIONBIO, &mode); // Blocking mode

    return 0;
}

sock_t app_uds_listener(const char *fpath)
{
    LOGF("You are not supposed listen to uds socket in windows");
    return INVALID_SOCKET;
}

sock_t app_uds_client_connect(const char *fpath)
{
    LOGF("You are not supposed connect to uds server in windows");
    return INVALID_SOCKET;
}

int app_inet_pton(int af, const char *src, void *dst)
{
    return inet_pton(af, src, dst);
}

const char *app_inet_ntop(int af, const void * src, char *dst, socklen_t size)
{
    return inet_ntop(af, src, dst, size);
}

int app_getpeername(sock_t sock, struct sockaddr *name, socklen_t *namelen)
{
    int ret = getpeername(sock, name, namelen);
    if (issockoptsuccess(ret))
        return 0;
    return -1;
}

int app_getsockname(sock_t sock, struct sockaddr *name, socklen_t *namelen)
{
    int ret = getsockname(sock, name, namelen);
    if (issockoptsuccess(ret))
        return 0;
    return -1;
}

int app_getsockopt(sock_t sock, int level, int option_name, void* option_value, socklen_t* option_len)
{
    int ret = getsockopt(sock, level, option_name, option_value, option_len);
    if (issockoptsuccess(ret))
        return 0;
    return -1;
}

int app_setsockopt(sock_t sock, int level, int option_name, const void* option_value, socklen_t option_len)
{
    int ret = setsockopt(sock, level, option_name, option_value, option_len);
    if (issockoptsuccess(ret))
        return 0;
    return -1;
}

uint32_t app_htonl(uint32_t hostlong)
{
    return htonl(hostlong);
}

uint16_t app_htons(uint16_t hostshort)
{
    return htons(hostshort);
}

uint32_t app_ntohl(uint32_t netlong)
{
    return ntohl(netlong);
}

uint16_t app_ntohs(uint16_t netshort)
{
    return ntohs(netshort);
}

int app_shutdown(sock_t sock, int how)
{
    int ret = shutdown(sock, how);
    if (issockoptsuccess(ret))
        return 0;
    return -1;
}

#else

int issockoptsuccess(int opret) {return opret >=0 ;}
int recv_fd(int unix_sock) {

    int             nr;
    char            buf[MAXLINE];
    char            contrl_buf[CONTROL_LEN];

    struct iovec iov = { .iov_base = buf, // Must send at least one byte
                        .iov_len = MAXLINE };

    struct msghdr msg = { .msg_iov = &iov,
                         .msg_iovlen = 1,
                         .msg_control = contrl_buf,
                         .msg_controllen = CONTROL_LEN
    };

    if ((nr = recvmsg(unix_sock, &msg, 0)) < 0) {
        LOGEE("recvmsg");
        return -1;
    }
    else if (nr == 0) {
        LOGE("Connection closed by server\n");
        return 0;
    }

    int recvfd = -1;
    struct cmsghdr* cmsg;
    for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
        if (cmsg->cmsg_level == SOL_SOCKET
            && cmsg->cmsg_type == SCM_RIGHTS) {
            memcpy(&recvfd, CMSG_DATA(cmsg), sizeof(int));

            break;
        }
    }

    return recvfd;
}

int send_fd(int unix_sock, int fd)
{
    struct iovec iov = { .iov_base = ":)", // Must send at least one byte
                        .iov_len = 2 };

    union {
        char buf[CMSG_SPACE(sizeof(fd))];
        struct cmsghdr align;
    } u;

    struct msghdr msg = { .msg_iov = &iov,
                         .msg_iovlen = 1,
                         .msg_control = u.buf,
                         .msg_controllen = sizeof(u.buf) };

    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
    *cmsg = (struct cmsghdr){ .cmsg_level = SOL_SOCKET,
                             .cmsg_type = SCM_RIGHTS,
                             .cmsg_len = CMSG_LEN(sizeof(fd)) };

    memcpy(CMSG_DATA(cmsg), &fd, sizeof(fd));

    return sendmsg(unix_sock, &msg, 0);
}

static sock_t
app_uds_client_connect_with_log(const char *fpath, int log)
{
    sock_t client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd == -1) {
        if (log) LOGEE("Can't open socket");
        return (sock_t)-1;
    }

    // BIND
    struct sockaddr_un server_addr;
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, fpath);
    if (server_addr.sun_path[0] == '@')
        server_addr.sun_path[0] = 0;

    socklen_t sock_len = offsetof(struct sockaddr_un, sun_path) + strlen(fpath);

    if(connect(client_fd, (struct sockaddr *)&server_addr, sock_len))
    {
        if (log) LOGEE("Could not connect %s", fpath);
        close(client_fd);
        return (sock_t)-1;
    }
    return client_fd;
}

sock_t
app_uds_client_connect(const char *fpath) {
    return app_uds_client_connect_with_log(fpath, 1);
}

sock_t
app_uds_listener(const char *fpath)
{
    if (fpath == NULL) {
        errno = ENOENT;
        return -1;
    }
    if (fpath[0] != '@') {
        struct stat st;
        if (stat(fpath, &st) == 0) {
            int temp_fd = app_uds_client_connect_with_log(fpath, 0);
            if (temp_fd < 0)
            {
                unlink(fpath);
            } else {
                close(temp_fd);
                errno = EBUSY;
                return -1;
            }
        }
    }
    sock_t listener_d = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listener_d == -1) {
        LOGEE("Can't open socket");
        return (sock_t)-1;
    }

    int optval = 1;
    setsockopt(listener_d, SOL_SOCKET, SO_REUSEADDR,
                (const void *)&optval , sizeof(int));
    // BIND
    struct sockaddr_un name;
    name.sun_family = AF_UNIX;

    if(0) {
#ifdef __LINUX_OS__
    } else if(fpath[0] == '@') {
        name.sun_path[0] = 0;
        strcpy(name.sun_path+1, fpath+1);
        fpath ++;
#else
    } else if(fpath[0] == '@') {
        Assert(0 && "Not implemented");
#endif
    } else {
        strcpy(name.sun_path, fpath);
    }
    socklen_t sock_len = offsetof(struct sockaddr_un, sun_path) + strlen(fpath) + 1;
    int c = bind(listener_d, (struct sockaddr *)&name, sock_len);
    if(c){
        close(listener_d);
        LOGFE("Can't bind path: `%s`", fpath);
        return (sock_t)-1;
    }

    //LISTEN
    if(listen(listener_d, SOMAXCONN) == -1) {
        LOGEE(" Cannot listen");
        close(listener_d);
        return (sock_t)-1;
    }
    return listener_d;
}


#endif

sock_t app_tcp_listener_ip(in_addr_t ip, port_t port)
{
    sock_t listener_d = socket(AF_INET, SOCK_STREAM, 0);
    if (!IsValidSocket(listener_d)) {
        LOGEE("Can't open socket");
        return InValidSocket;
    }
    int optval = 1;
    setsockopt(listener_d, SOL_SOCKET, SO_REUSEADDR,
                (const void *)&optval , sizeof(int));
    // BIND
    struct sockaddr_in name;
    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    name.sin_addr.s_addr = ip;
    int c = bind(listener_d, (struct sockaddr *)&name, sizeof(name));
    if(!issockoptsuccess(c)){
        LOGEE("Can't bind the port %d", port);
        close(listener_d);
        return InValidSocket;
    }

    //LISTEN
    if(!issockoptsuccess(listen(listener_d, SOMAXCONN))) {
        close(listener_d);
        LOGEE("Cannot listen");
        return InValidSocket;
    }
    return listener_d;
}

sock_t app_tcp6_listener_ip(struct in6_addr ip, port_t port)
{
    sock_t listener_d = socket(AF_INET6, SOCK_STREAM, 0);
    if (!IsValidSocket(listener_d)) {
        LOGEE("Can't open socket");
        return InValidSocket;
    }

    int optval = 1;
    setsockopt(listener_d, SOL_SOCKET, SO_REUSEADDR,
                (const void *)&optval , sizeof(int));
    // BIND
    struct sockaddr_in6 name;
    name.sin6_family = AF_INET6;
    name.sin6_port = htons(port);
    name.sin6_addr = ip;
    int c = bind(listener_d, (struct sockaddr *)&name, sizeof(name));
    if(!issockoptsuccess(c)) {
        LOGEE("Can't bind the port");
        SysSocketClose(listener_d);
        return InValidSocket;
    }

    //LISTEN
    if(!issockoptsuccess(listen(listener_d, SOMAXCONN))) {
        SysSocketClose(listener_d);
        LOGEE("Cannot listen");
        return InValidSocket;
    }
    return listener_d;
}

sock_t app_tcp_listener_host(const char* host, const char *port)
{
    struct addrinfo hints;
    struct addrinfo *res, *rp;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Stream socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */
    int err = 0;

    int s = getaddrinfo(host, port, &hints, &res);
    if(s != 0) {
        return InValidSocket;
    }

    sock_t sock = InValidSocket;
    for(rp = res; rp != NULL; rp=rp->ai_next) {
        if (rp->ai_family == AF_INET6)
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(!IsValidSocket(sock)) {
            LOGEE("Cannot listen");
            continue;
        }

        int optval = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                    (const void *)&optval , sizeof(int));

        int c = bind(sock, rp->ai_addr, rp->ai_addrlen);
        if(!issockoptsuccess(c)) {
            LOGEE("Can't bind the port");
            goto ListenError;
        }

        //LISTEN
        if(!issockoptsuccess(listen(sock, SOMAXCONN))) {
            LOGEE("Cannot listen");
            goto ListenError;
        }
        break;
ListenError:
        err = app_get_errno();
        SysSocketClose(sock);
        sock = InValidSocket;
        app_set_errno(err);
    }
    freeaddrinfo(res);

    return sock;
}

sock_t app_udp_listener_ip(in_addr_t ip, port_t port)
{
    sock_t listener_d = socket(AF_INET, SOCK_DGRAM, 0);
    if (!IsValidSocket(listener_d)) {
        LOGEE("Can't open socket");
        return InValidSocket;
    }

    int optval = 1;
    // BIND
    struct sockaddr_in name;
    name.sin_family = AF_INET;
    name.sin_port = htons(port);
    name.sin_addr.s_addr = ip;
    int c = bind(listener_d, (struct sockaddr *)&name, sizeof(name));
    if(!issockoptsuccess(c)){
        SysSocketClose(listener_d);
        LOGEE("Cannot bind");
        return InValidSocket;
    }

    setsockopt(listener_d, SOL_SOCKET, SO_REUSEADDR,
                (const void *)&optval , sizeof(int));
    //LISTEN
    return listener_d;
}

sock_t app_udp6_listener_ip(struct in6_addr ip, port_t port)
{
    sock_t listener_d = socket(AF_INET6, SOCK_DGRAM, 0);
    if (!IsValidSocket(listener_d)) {
        LOGEE("Can't open socket");
        return InValidSocket;
    }

    int optval = 1;
    // BIND
    struct sockaddr_in6 name;
    name.sin6_family = AF_INET6;
    name.sin6_port = htons(port);
    name.sin6_addr = ip;
    int c = bind(listener_d, (struct sockaddr *)&name, sizeof(name));
    if(!issockoptsuccess(c)){
        LOGEE("Cannot bind");
        SysSocketClose(listener_d);
        return InValidSocket;
    }

    setsockopt(listener_d, SOL_SOCKET, SO_REUSEADDR,
                (const void *)&optval , sizeof(int));
    //LISTEN
    return listener_d;
}


sock_t app_tcp_client_connect_ip(in_addr_t ip, port_t port)
{
    struct sockaddr_in server_addr;
    sock_t client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (!IsValidSocket(client_fd)) {
        LOGEE("Can't open socket");
        return InValidSocket;
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = ip;
    if(!issockoptsuccess(connect(client_fd, (struct sockaddr *)&server_addr, sizeof(server_addr))))
    {
        SysSocketClose(client_fd);
        LOGEE("Could not connect");
        return InValidSocket;
    }
    return client_fd;
}

port_t app_socket_port(sock_t socket) {
    union sockaddr_ip name;
    socklen_t len = sizeof(name);
    if(!issockoptsuccess(getsockname(socket, (struct sockaddr *)&name, &len))){
        return 0;
    }
    return htons(name.inaddr.sin_port);
}

port_t app_peer_port(sock_t socket) {
    union sockaddr_ip name;
    socklen_t len = sizeof(name);
    if(!issockoptsuccess(getpeername(socket, (struct sockaddr *)&name, &len))){
        return 0;
    }
    return htons(name.inaddr.sin_port);
}

sock_t app_tcp_client_connect_host(const char *host, const char *port) {
    struct addrinfo hints;
    struct addrinfo *res, *rp;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Stream socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */

    int s = getaddrinfo(host, port, &hints, &res);
    if(s != 0) {
        return InValidSocket;
    }

    sock_t sock = InValidSocket;
    for(rp = res; rp != NULL; rp=rp->ai_next) {
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if(!IsValidSocket(sock))
            continue;

        if (issockoptsuccess(connect(sock, rp->ai_addr, rp->ai_addrlen)))
            break;                  /* Success */
        int err = app_get_errno();
        SysSocketClose(sock);
        sock = InValidSocket;
        app_set_errno(err);
    }
    freeaddrinfo(res);
    return sock;
}

sock_t app_udp_client_connect_host(const char *host, const char *port, sockaddr_ip *sockAddr)
{
    struct addrinfo hints;
    struct addrinfo *res, *rp;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */

    int s = getaddrinfo(host, port, &hints, &res);
    if(s != 0) {
        LOGE("Cannot get addr info");
        return InValidSocket;
    }

    int found = 0;
    sa_family_t peer_family = AF_UNSPEC;
    for(rp = res; rp != NULL; rp=rp->ai_next) {
        if (rp->ai_family == AF_INET6) {
            peer_family = AF_INET6;
            sockAddr->addr = *(rp->ai_addr);
            found = 1;
        } else if (rp->ai_family == AF_INET) {
            peer_family = AF_INET;
            sockAddr->addr = *(rp->ai_addr);
            found = 1;
            break; //we want ipv4 here because almost all ipv4 can connect to ipv6
        }
    }
    freeaddrinfo(res);
    if (!found) {
        return InValidSocket;
    }

    sock_t sock = InValidSocket;
    for (int i = 0; i < 1; i++) {
        sock = socket(peer_family, SOCK_DGRAM, 0);
        if(!IsValidSocket(sock)) {
            LOGD("Cannot get addr info");
            continue;
        }

        if (peer_family == AF_INET6) {
            struct in6_addr ip = IN6ADDR_ANY_INIT;
            struct sockaddr_in6 name;
            name.sin6_family = AF_INET6;
            name.sin6_port = 0;
            name.sin6_addr = ip;

            int off = 0;
            if (!issockoptsuccess(setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (char *) & off, sizeof(off)))) {
                LOGD("Could not unset IPV6_V6ONLY: %s", app_get_strerror(app_get_errno()));
            }

            if (issockoptsuccess(bind(sock, (struct sockaddr *)&name, sizeof(name)))) {
                break;
            } else {
                LOGD("Cannot bind %d %s", sock, app_get_strerror(app_get_errno()));
            }
        } else if (peer_family == AF_INET) {
            in_addr_t ip = 0;
            struct sockaddr_in name;
            name.sin_family = AF_INET;
            name.sin_port = htons(0);
            name.sin_addr.s_addr = ip;
            if(issockoptsuccess(bind(sock, (struct sockaddr *)&name, sizeof(name)))) {
                break;
            } else {
                LOGD("Cannot bind %d %s", sock, app_get_strerror(app_get_errno()));
            }
        }

        int err = app_get_errno();
        SysSocketClose(sock);
        sock = InValidSocket;
        app_set_errno(err);
    }
    return sock;
}

struct sock_addrinfo *app_getaddrinfo_tcp(const char *host, const char *port)
{
    struct addrinfo hints;
    struct addrinfo *res = NULL, *rp;
    int count = 0;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Stream socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */

    if (getaddrinfo(host, port, &hints, &res) != 0) {
        return NULL;
    }
    for (rp = res; rp; rp = rp->ai_next) {
        count ++;
    }

    struct sock_addrinfo *addresses = malloc(sizeof(struct sock_addrinfo) * (count + 1));
    bzero(addresses, sizeof(struct sock_addrinfo) * (count + 1));

    for (rp = res, count = 0; rp; rp = rp->ai_next, count ++) {
        addresses[count].flags     = rp->ai_flags;
        addresses[count].family    = rp->ai_family;
        addresses[count].socktype  = rp->ai_socktype;
        addresses[count].protocol  = rp->ai_protocol;
        addresses[count].addrlen   = rp->ai_addrlen;
        memcpy(&addresses[count].addr.addr, rp->ai_addr, rp->ai_addrlen);
        //addresses[count].addr.addr = *(rp->ai_addr);
        addresses[count].valid = 1;
    }
    freeaddrinfo(res);
    return addresses;
}

void app_freeaddrinfo(struct sock_addrinfo *res)
{
    if (res) {
        free(res);
    }
}

sock_t app_connect_nonblocking_socket(struct sock_addrinfo *rp, int *connected)
{
    *connected = 0;
    sock_t sock = InValidSocket;
    sock = socket(rp->family, rp->socktype, rp->protocol);
    if(!IsValidSocket(sock))
        return sock;
    set_blocking(sock, 0);
    if (connect(sock, &(rp->addr.addr), rp->addrlen) == 0) {
        *connected = 1;
        return sock;
    }//todo
#ifndef __WINDOWS_OS__
    if (errno != EINPROGRESS) {
        int appErrNo = errno;
        CloseNCleanSocket(sock);
        errno = appErrNo;
    }
#else
    if (app_get_errno() != WSAEWOULDBLOCK) {
        int appErrNo = app_get_errno();
        CloseNCleanSocket(sock);
        app_set_errno(appErrNo);
    }
#endif
    return sock;
}

int32_t set_recv_timeout_ms(sock_t socket, uint64_t timeout) {
#if defined(__MAC_OS__) || defined(__LINUX_OS__)
    struct timeval tv;
    tv.tv_sec = timeout/1000;
    tv.tv_usec = timeout%1000;
    int ret = setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    return ret;
#elif defined(__WINDOWS_OS__)
    DWORD tm = (DWORD)timeout;
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tm, sizeof tm);
    return 0;
#else
#error("Undefined")
#endif
}


int32_t set_send_timeout_ms(sock_t socket, uint64_t timeout) {
#if defined(__MAC_OS__) || defined(__LINUX_OS__)
    struct timeval tv;
    tv.tv_sec = timeout/1000;
    tv.tv_usec = timeout%1000;
#elif defined(__WINDOWS_OS__)
    DWORD tv = timeout;
#else
#error("Undefined")
#endif
    int ret = setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof tv);
    return ret;
}

sock_t app_accept(sock_t sock, sockaddr_ip *addr, socklen_t *len) {
    return accept(sock, (struct sockaddr *)addr, len);
}

ssize_t app_send(sock_t sock, const void *buf, size_t len, int flags) {
    return send(sock, buf, len, flags);
}

ssize_t app_send_to(sock_t sock, const void *buf, size_t len, int flags, sockaddr_ip *addr, socklen_t addrlen) {
    return sendto(sock, buf, len, flags, (struct sockaddr *)addr, addrlen);
}

ssize_t app_recv(sock_t sock, void *buf, size_t len, int flags) {
    return recv(sock, buf, len, flags);
}

ssize_t app_recv_from(sock_t sock, void *buf, size_t len, int flags, sockaddr_ip *addr, socklen_t *addrlen)
{
    return recvfrom(sock, buf, len, flags, (struct sockaddr *)addr, addrlen);
}


char *app_get_strerror(int err) {
#ifdef __WINDOWS_OS__
    static char msg_buffer[256];  // Static buffer for simplicity
    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        msg_buffer,
        sizeof(msg_buffer),
        NULL
    );
    return msg_buffer;
#else
    return strerror(err);
#endif
}


int _closeNCleanSocket(sock_t *fd) {
    int ret = 0;
    if(IsValidSocket(*fd)) {
        ret = SysSocketClose(*fd);
        *fd = InValidSocket;
    }
    return ret;
}


socket_stat getUnackedTcp(sock_t socket) {
    socket_stat sockStat = {.success = 0};
#ifdef __LINUX_OS__
    struct tcp_info tcpInfo;
    socklen_t len = sizeof(tcpInfo);
    if (getsockopt(socket, IPPROTO_TCP, TCP_INFO, &tcpInfo, &len) < 0) {
        LOGEE("Cannot call getSockopt");
        return sockStat;
    }
    sockStat = (socket_stat){
        .success = 1,
        .retransmits    = tcpInfo.tcpi_retransmits,
        .unacked        = tcpInfo.tcpi_unacked,
        .last_data_sent = tcpInfo.tcpi_last_data_sent,
        .last_ack_sent  = tcpInfo.tcpi_last_ack_sent,
        .last_data_recv = tcpInfo.tcpi_last_data_recv,
        .last_ack_recv  = tcpInfo.tcpi_last_ack_recv,
        };
#endif
    return sockStat;
}


sa_family_t get_socket_family(sock_t socket)
{
#if defined(__MAC_OS__) || defined(__WINDOWS_OS__)
struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    if (getsockname(socket, (struct sockaddr*)&addr, &len) < 0) {
        return -1;
    }
    return addr.sin_family;
#else
    int optval;
    socklen_t optlen = sizeof(optval);
    if (getsockopt(socket, SOL_SOCKET, SO_DOMAIN, &optval, &optlen) < 0) {
        return -1;
    }
    return optval;
#endif
}

int get_socket_type(sock_t socket)
{
    int optval;
    socklen_t optlen = sizeof(optval);
    if (getsockopt(socket, SOL_SOCKET, SO_TYPE, (void *)&optval, &optlen) < 0) {
        return -1;
    }
    return optval;
}

int enable_keep_alive(sock_t fd, int keepCnt, int keepIdle, int keepIntvl, int enable) {
    int optval = enable ? 1 : 0;
    if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&optval, sizeof(optval)) < 0) {
        LOGEF(fd, "setsockopt");
        return 0;
    }

    optval = keepCnt;
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, (void*)&optval, sizeof(optval)) < 0) {
        LOGEF(fd, "setsockopt");
        return 0;
    }

#ifdef __LINUX_OS__
    optval = keepIdle;
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, &optval, sizeof(optval)) < 0) {
        LOGEF(fd, "setsockopt ");
        return 0;
    }
#endif

    optval = keepIntvl;
    if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, (void*)&optval, sizeof(optval)) < 0) {
        LOGEF(fd, "setsockopt: ");
        return 0;
    }
    return 1;
}


#ifndef __WINDOWS_OS__

int set_close_on_exec(sock_t fd)
{
    int flags = fcntl(fd, F_GETFD);
    if (flags < 0) {
        return 0;
    }
    flags |= FD_CLOEXEC;
    if(fcntl(fd, F_SETFD, flags) < 0) {
        return 0;
    }
    return 1;
}

int unset_close_on_exec(sock_t fd)
{
    int flags = fcntl(fd, F_GETFD);
    if (flags < 0) {
        return 0;
    }
    flags &= ~FD_CLOEXEC;
    if(fcntl(fd, F_SETFD, flags) < 0) {
        return 0;
    }
    return 1;
}


int set_blocking(sock_t fd, int blocking)
{
    int flags = fcntl(fd, F_GETFL);
    if (flags < 0) {
        return 0;
    }
    int isBlocked = !(flags & O_NONBLOCK);
    if (blocking == isBlocked)
        return 1;
    if (blocking)
        flags &= ~O_NONBLOCK;
    else
        flags |= O_NONBLOCK;

    if(fcntl(fd, F_SETFL, flags) < 0) {
        return 0;
    }
    return 1;
}


int is_blocking(sock_t fd)
{
    int flags = fcntl(fd, F_GETFL);
    if (flags < 0) {
        return 0;
    }

    return !(flags & O_NONBLOCK);
}
#else

int set_close_on_exec(sock_t fd)
{
    return 1;
}

int unset_close_on_exec(sock_t fd)
{
    return 1;
}


int set_blocking(sock_t fd, int blocking)
{
    u_long mode = blocking ? 0 : 1;  // 0 for blocking mode, 1 for non-blocking mode
    int result = ioctlsocket(fd, FIONBIO, &mode);
    if (result != 0) {
        return 0;
    }
    return 1;
}


int is_blocking(sock_t fd)
{
    // LOGE("You are not supposed to call `is_blocking` from windows system");
    // abort();
    return 1;
}
#endif


int
is_ip_address(const char *hostname)
{
    struct in_addr ipv4_addr;
    struct in6_addr ipv6_addr;

    // Check IPv4
    if (app_inet_pton(AF_INET, hostname, &ipv4_addr) == 1)
        return 1;

    // Check IPv6
    if (app_inet_pton(AF_INET6, hostname, &ipv6_addr) == 1)
        return 1;

    // Not an IP address
    return 0;
}

int
ip_port_to_sockaddr(const char *ip_port, union sockaddr_ip *out, socklen_t *outlen)
{
    char ip[INET6_ADDRSTRLEN];
    char *colon = strrchr(ip_port, ':');
    if (!colon) return -1;

    if (!out || !outlen)
        return -1;
    if (*outlen == 0)
        return -1;

    size_t ip_len = colon - ip_port;
    if (ip_len >= sizeof(ip)) return -1;
    strncpy(ip, ip_port, ip_len);
    ip[ip_len] = '\0';

    int port = atoi(colon + 1);
    if (port <= 0 || port > 65535) return -1;

    // Try IPv4 first
    if (*outlen >= sizeof(struct sockaddr_in)) {
        struct sockaddr_in *sin = (struct sockaddr_in *)out;
        memset(out, 0, *outlen);
        sin->sin_family = AF_INET;
        sin->sin_port = htons(port);
        if (inet_pton(AF_INET, ip, &sin->sin_addr) == 1) {
            *outlen = sizeof(struct sockaddr_in);
            return 0;
        }
    }

    // Try IPv6
    if (*outlen >= sizeof(struct sockaddr_in6)) {
        struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)out;
        sin6->sin6_family = AF_INET6;
        sin6->sin6_port = htons(port);
        if (inet_pton(AF_INET6, ip, &sin6->sin6_addr) == 1) {
            *outlen = sizeof(struct sockaddr_in6);
            return 0;
        }
    }

    return -1;
}

#undef MAXLINE //2048
#undef CONTROL_LEN //1024
