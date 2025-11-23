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


#ifndef NETWORK_H_
#define NETWORK_H_
#include "platform.h"
#ifndef __WINDOWS_OS__
#include <sys/un.h>
#include <errno.h>
#else
#endif

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __WINDOWS_OS__
#pragma warning(disable: 4201)
#pragma warning(disable: 4127)
#pragma warning(disable: 4100)
#define MSG_DONTWAIT 0
#else
#define INVALID_SOCKET 0
#endif


typedef struct ip_addr {
    union {
        uint64_t __padd64[2];
        struct {
            uint8_t __padd8[12];
            struct in_addr v4;
        };
        struct in6_addr v6;
    };
} ip_addr;


typedef union sockaddr_ip{
    struct sockaddr addr;
    struct sockaddr_in inaddr;
    struct sockaddr_in6 in6addr;
#ifndef __WINDOWS_OS__
    struct sockaddr_un  unaddr;
#endif
}sockaddr_ip;

struct sock_addrinfo
{
  int                           flags;      /* Input flags.  */
  int                           family;     /* Protocol family for socket.  */
  int                           socktype;   /* Socket type.  */
  int                           protocol;   /* Protocol for socket.  */
  socklen_t                     addrlen;    /* Length of socket address.  */
  union sockaddr_ip             addr;       /* Socket address for socket.  */
  uint16_t                      valid:1;
  uint16_t                      cached:1;
};

typedef struct socket_stat {
    uint8_t     success;
    uint8_t     retransmits;
    uint32_t    unacked;
    uint32_t    last_data_sent;
    uint32_t    last_ack_sent;
    uint32_t    last_data_recv;
    uint32_t    last_ack_recv;
}socket_stat;

#ifdef __WINDOWS_OS__
typedef int32_t sa_family_t;
typedef u_long in_addr_t;

#define SHUT_RD     SD_RECEIVE
#define SHUT_WR     SD_SEND
#define SHUT_RDWR   SD_BOTH

#define MSG_NOSIGNAL 0

#else
#endif //__WINDOWS_OS__

#ifdef __WINDOWS_OS__
int app_socketpair(int domain, int type, int protocol, sock_t sv[2]);
int app_inet_pton(int af, const char *src, void *dst);
const char *app_inet_ntop(int af, const void *src, char *dst, socklen_t size);
int app_getpeername(sock_t sock, struct sockaddr *name, socklen_t *namelen);
int app_getsockname(sock_t sock, struct sockaddr *name, socklen_t *namelen);
int app_getsockopt(sock_t sock, int level, int option_name, void* option_value, socklen_t* option_len);
int app_setsockopt(sock_t sock, int level, int option_name, const void* option_value, socklen_t option_len);
uint32_t app_htonl(uint32_t hostlong);
uint16_t app_htons(uint16_t hostshort);
uint32_t app_ntohl(uint32_t netlong);
uint16_t app_ntohs(uint16_t netshort);
int app_shutdown(sock_t sock, int how);


#define app_get_errno WSAGetLastError
#define app_set_errno WSASetLastError

/*
#define EWOULDBLOCK      WSAEWOULDBLOCK             //Non-blocking operation would block
#define EAGAIN           WSAEWOULDBLOCK             //Non-blocking operation would block
#define EINPROGRESS      WSAEINPROGRESS             //Operation in progress
#define EALREADY         WSAEALREADY                //Operation already in progress
#define EISCONN          WSAEISCONN                 //Socket is already connected
#define ENOTCONN         WSAENOTCONN                //Socket is not connected
#define ECONNRESET       WSAECONNRESET              //Connection reset by peer
#define ECONNREFUSED     WSAECONNREFUSED            //Connection refused
#define ETIMEDOUT        WSAETIMEDOUT               //Connection timed out
#define EHOSTUNREACH     WSAEHOSTUNREACH            //No route to host
#define ENETUNREACH      WSAENETUNREACH             //Network is unreachable
#define EADDRINUSE       WSAEADDRINUSE              //Address already in use
#define EADDRNOTAVAIL    WSAEADDRNOTAVAIL           //Cannot assign requested address
#define EBADF            WSAEBADF                   //Bad file descriptor (invalid socket)
#define EMFILE           WSAEMFILE                  //Too many open files/sockets
#define EACCES           WSAEACCES                  //Permission denied
#define EFAULT           WSAEFAULT                  //Bad address
#define EINVAL           WSAEINVAL                  //Invalid argument
#define EMSGSIZE         WSAEMSGSIZE                //Message too long
#define ENOBUFS          WSAENOBUFS                 //No buffer space available
#define EPIPE            WSAESHUTDOWN               //The socket has been shut down
#define EPERM            WSAEACCES                  //Operation not permitted
#define EDESTADDRREQ     WSAEDESTADDRREQ            //Destination address required
#define ENETDOWN         WSAENETDOWN                //Network is down
#define EPROTOTYPE       WSAEPROTOTYPE              //Wrong protocol type for socket
#define ENOPROTOOPT      WSAENOPROTOOPT             //Protocol not available
#define EPROTONOSUPPORT  WSAEPROTONOSUPPORT         //Protocol not supported
#define ESOCKTNOSUPPORT  WSAESOCKTNOSUPPORT         //Socket type not supported
#define EOPNOTSUPP       WSAEOPNOTSUPP              //Operation not supported on socket
#define ESHUTDOWN        WSAESHUTDOWN               //Cannot send after socket shutdown
#define EINTR            WSAEINTR                   //Interrupted function call
*/

#else
#define app_get_errno() errno
#define app_set_errno(err) errno = err

sock_t recv_fd(sock_t unix_sock);
int send_fd(sock_t unix_sock, sock_t fd);

#define app_socketpair      socketpair
#define app_inet_pton       inet_pton
#define app_inet_ntop       inet_ntop
#define app_getpeername     getpeername
#define app_getsockname     getsockname
#define app_getsockopt      getsockopt
#define app_setsockopt      setsockopt
#define app_htonl           htonl
#define app_htons           htons
#define app_ntohl           ntohl
#define app_ntohs           ntohs
#define app_shutdown        shutdown

#endif


#define app_is_eagain()   (app_get_errno() == EWOULDBLOCK || app_get_errno() == EAGAIN)
#define app_is_timedout() (app_get_errno() == ETIMEDOUT)
#define app_set_eagain()  app_set_errno(EAGAIN)


#define app_tcp_listener_str(_x, _y) (app_tcp_listener_ip(inet_addr(_x), _y))
#define app_tcp_listener(_x) (app_tcp_listener_ip(htonl(INADDR_ANY), _x ))
sock_t app_tcp_listener_ip(in_addr_t ip, port_t port );

#define app_tcp6_listener(_x) (app_tcp6_listener_ip(IN6ADDR_ANY_INIT, _x ))
sock_t app_tcp6_listener_ip(struct in6_addr ip, port_t port );

sock_t app_tcp_listener_host(const char *host, const char *port);

#define app_udp_listener_str(_x, _y) (app_udp_listener_ip(inet_addr(_x), _y))
#define app_udp_listener(_x) (app_udp_listener_ip(htonl(INADDR_ANY), _x))
sock_t app_udp_listener_ip(in_addr_t ip, port_t port);

#define app_udp6_listener(_x) (app_udp6_listener_ip(IN6ADDR_ANY_INIT, _x ))
sock_t app_udp6_listener_ip(struct in6_addr ip, port_t port);

sock_t app_uds_listener(const char *fpath);

sock_t app_uds_client_connect(const char *fpath);

#define app_tcp_client_connect_str(_x, _y) (app_tcp_client_connect_ip(inet_addr(_x), _y))
sock_t app_tcp_client_connect_ip(in_addr_t ip, port_t port);




//=====================
//    System functions
//=====================

sock_t app_tcp_client_connect_host(const char* host, const char* port);
sock_t app_udp_client_connect_host(const char* host, const char* port, sockaddr_ip* sockAddr);

struct sock_addrinfo *app_getaddrinfo_tcp(const char* host, const char* port);
void app_freeaddrinfo(struct sock_addrinfo *res);
sock_t app_connect_nonblocking_socket(struct sock_addrinfo *res, int *connected);

port_t app_socket_port(sock_t socket);

port_t app_peer_port(sock_t socket);

struct socket_stat getUnackedTcp(sock_t socket);

sock_t app_accept(sock_t sock, sockaddr_ip *addr, socklen_t *len);

ssize_t app_send(sock_t sock, const void *buf, size_t len, int flags);
ssize_t app_send_to(sock_t sock, const void *buf, size_t len, int flags, sockaddr_ip *addr, socklen_t addrlen);

ssize_t app_recv(sock_t sock, void *buf, size_t len, int flags);
ssize_t app_recv_from(sock_t sock, void *buf, size_t len, int flags, sockaddr_ip *addr, socklen_t *addrlen);

char *app_get_strerror(int err);

int32_t get_macaddress_based_on_ip_address(union sockaddr_ip *addr, char *macaddr, len_t len);
int32_t get_macaddress_based_on_connected_fd(sock_t socket, char *macaddr, len_t len);

int32_t set_recv_timeout_ms(sock_t socket, uint64_t timeout);
int32_t set_send_timeout_ms(sock_t socket, uint64_t timeout);


sa_family_t get_socket_family(sock_t socket);
int get_socket_type(sock_t socket);
int enable_keep_alive(sock_t fd, int keepCnt, int keepIdle, int keepIntvl, int enable);

#ifdef __WINDOWS_OS__
#define IsValidSocket(fd) (fd != INVALID_SOCKET)
#define SysSocketClose(x) closesocket(x)
#define InValidSocket INVALID_SOCKET
#else
#define SysSocketClose(x) close(x)
#define IsValidSocket(fd) (fd > 0)
#define InValidSocket 0
#endif

int _closeNCleanSocket(sock_t* fd);
#define CloseNCleanSocket(fd) (_closeNCleanSocket(&fd))//({int ret = 0; if(IsValidSocket(fd)) { ret = SysSocketClose(fd); fd = InValidSocket; } ret;})
#define InValidateSocket(fd) { fd = InValidSocket; }

int set_close_on_exec(sock_t fd);
int unset_close_on_exec(sock_t fd);

int set_blocking(sock_t fd, int blocking);
int is_blocking(sock_t fd);

int is_ip_address(const char *hostname);

int ip_port_to_sockaddr(const char *ip_port, union sockaddr_ip *out, socklen_t *outlen);

#ifdef __cplusplus
}
#endif

#endif /* NETWORK_H_ */
