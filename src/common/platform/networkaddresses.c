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
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#else
#include <winsock2.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#endif
#ifdef __MAC_OS__
#include <sys/sysctl.h>
#include <net/if_dl.h>
#endif
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "assert_pinggy.h"

#ifdef __MAC_OS__
static int32_t get_mac_based_on_interface(char *interfaceaddr, char *macaddr, len_t lenArg)
{
    int         mib[6];
    size_t      len;
    char            *buf;
    unsigned char       *ptr;
    struct if_msghdr    *ifm;
    struct sockaddr_dl  *sdl;
    if(!macaddr)
        return -1;

    mib[0] = CTL_NET;
    mib[1] = AF_ROUTE;
    mib[2] = 0;
    mib[3] = AF_LINK;
    mib[4] = NET_RT_IFLIST;
    if ((mib[5] = if_nametoindex(interfaceaddr)) == 0) {
        LOGEE("if_nametoindex error");
        return -1;
    }

    if (sysctl(mib, 6, NULL, &len, NULL, 0) < 0) {
        LOGEE("sysctl 1 error");
        return -1;
    }

    if ((buf = malloc(len)) == NULL) {
        LOGEE("malloc error");
        return -1;
    }

    if (sysctl(mib, 6, buf, &len, NULL, 0) < 0) {
        LOGEE("sysctl 2 error");
        return -1;
    }

    ifm = (struct if_msghdr *)buf;
    sdl = (struct sockaddr_dl *)(ifm + 1);
    ptr = (unsigned char *)LLADDR(sdl);
//    memcpy(macaddr, ptr, 6);
    snprintf(macaddr, lenArg, "%02x%02x%02x%02x%02x%02x", *ptr, *(ptr+1), *(ptr+2),
            *(ptr+3), *(ptr+4), *(ptr+5));

    return 0;
}
#endif

#ifdef __LINUX_OS__
static int32_t get_mac_based_on_interface(char *interfaceaddr, char *macaddr, len_t len)
{
    sock_t fd;
    struct ifreq ifr;
    unsigned char *mac;

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name , interfaceaddr , IFNAMSIZ-1);

    ioctl(fd, SIOCGIFHWADDR, &ifr);

    close(fd);

    mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;

    //display mac address
    snprintf(macaddr, len, "%02x%02x%02x%02x%02x%02x" , mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

//    memcpy(macaddr, mac, 6);

    return 0;
}
#endif

#ifndef __WINDOWS_OS__
int32_t get_macaddress_based_on_ip_address(sockaddr_ip *addr,  char *macaddr, len_t len)
{
    struct ifaddrs *ifaddr;
    int family;
    int32_t ret = -1;

    if(!macaddr) return -1;

    if (getifaddrs(&ifaddr) == -1) {
        LOGEE("getifaddrs");
        exit(EXIT_FAILURE);
    }

    /* Walk through linked list, maintaining head pointer so we
       can free list later. */

    for (struct ifaddrs *ifa = ifaddr; ifa != NULL;
            ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        family = ifa->ifa_addr->sa_family;

        if(family != addr->inaddr.sin_family)
            continue;
        if(family == AF_INET) {
            if(((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr != addr->inaddr.sin_addr.s_addr)
                continue;
        }
        else if(family == AF_INET6) {
            if(memcmp(&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr,
                    &(addr->in6addr.sin6_addr),
                    sizeof(addr->in6addr.sin6_addr)
                    ))
                continue;
        }

        /* Display interface name and fmily (including symbolic
           form of the latter for the common families). */


        /* For an AF_INET* interface address, display the address. */
        ret = get_mac_based_on_interface(ifa->ifa_name, macaddr, len);
        break;
    }

    freeifaddrs(ifaddr);
    return ret;
}

#else

int32_t get_macaddress_based_on_ip_address(sockaddr_ip *addr,  char *macaddr, len_t len) {
#if 0
  PIP_ADAPTER_ADDRESSES AdapterAddress;
  DWORD dwBufLen = sizeof(IP_ADAPTER_ADDRESSES);

  ULONG family = addr->inaddr.sin_family;

  AdapterAddress = (IP_ADAPTER_ADDRESSES *) malloc(sizeof(IP_ADAPTER_ADDRESSES));
  if (AdapterAddress == NULL) {
    LOGE("Error allocating memory needed to call GetAdaptersinfo\n");
//    free(mac_addr);
    return -1; // it is safe to call free(NULL)
  }

  // Make an initial call to GetAdaptersInfo to get the necessary size into the dwBufLen variable
  if (GetAdaptersAddresses(family, 0, NULL, AdapterAddress, &dwBufLen) == ERROR_BUFFER_OVERFLOW) {
    free(AdapterAddress);
    AdapterAddress = (IP_ADAPTER_ADDRESSES *) malloc(dwBufLen);
    if (AdapterAddress == NULL) {
      LOGE("Error allocating memory needed to call GetAdaptersinfo\n");
//      free(mac_addr);
      return -1;
    }
  }

  if (GetAdaptersAddresses(family, 0, NULL, AdapterAddress, &dwBufLen) == NO_ERROR) {
    // Contains pointer to current adapter info
      PIP_ADAPTER_ADDRESSES pAdapterAddress = AdapterAddress;
    do {

        PIP_ADAPTER_UNICAST_ADDRESS pu = pAdapterAddress->FirstUnicastAddress;
        while(pu) {
            assert(pu->Address.lpSockaddr->sa_family == AF_INET); //TODOipv6 handling
            struct sockaddr_in *si = (struct sockaddr_in *)&(pu->Address.lpSockaddr);
            char a[100];
            inet_ntop(AF_INET, si, a, sizeof(a));
            LOGT("addr: %s\n", a) ;
            pu = pu->Next;
        }

      pAdapterAddress = pAdapterAddress->Next;
    } while(pAdapterAddress);
  }
  free(AdapterAddress);
  exit(1);
#else
  (void)addr;
//  assert(FALSE && "Not implemented. Contact developer");
  snprintf(macaddr, len, "WindowsDummy");
#endif
  return 0; // caller must free.
}

#endif

int32_t get_macaddress_based_on_connected_fd(sock_t sock, char *macaddr, len_t len) {
    Assert(macaddr && len);
    sockaddr_ip sockaddr;
    socklen_t addr_len = sizeof(sockaddr);
    if(getsockname(sock, (struct sockaddr *) &sockaddr, &addr_len) < 0){
        return -1;
    }
    return get_macaddress_based_on_ip_address(&sockaddr, macaddr, len);
}


