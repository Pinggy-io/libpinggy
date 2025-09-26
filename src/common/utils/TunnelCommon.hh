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


#ifndef __SRC_CPP_PUBLIC_COMMON_UTILS_TUNNELCOMMON_HH__
#define __SRC_CPP_PUBLIC_COMMON_UTILS_TUNNELCOMMON_HH__

#include <platform/SharedPtr.hh>
#include <vector>

typedef tUint16 tForwardingId;

#define InvalidForwardingId 0


#define TunnelType_None     ""
#define TunnelType_Unknown  "unknown"
#define TunnelType_HTTP     "http"
#define TunnelType_TCP      "tcp"
#define TunnelType_TLS      "tls"
#define TunnelType_TLSTCP   "tlstcp"
#define TunnelType_UDP      "udp"


#define Schema_None         ""
#define Schema_HTTP         "http"
#define Schema_HTTPS        "https"
#define Schema_TCP          "tcp"
#define Schema_TLS          "tls"
#define Schema_TLSTCP       "tlstcp"
#define Schema_UDP          "udp"



#define TunnelType_None     ""
#define TunnelType_HTTP     "http"
#define TunnelType_TCP      "tcp"
#define TunnelType_TLS      "tls"
#define TunnelType_TLSTCP   "tlstcp"
#define TunnelType_UDP      "udp"

enum class TunnelMode {
    None    = 0,
    HTTP    = 1<<0,
    TCP     = 1<<1,
    TLS     = 1<<2,
    TLSTCP  = 1<<3,
    UDP     = 1<<4
};

std::ostream&
operator<<(std::ostream& os, TunnelMode m);

TunnelMode
TunnelModeFromString(tString modeStr);

tString
TunnelTypeFromTunnelMode(TunnelMode mode);


enum SpecialPort {
    SpecialPort_Minimum = 3,
    SpecialPort_ConfigTcp = 4, //This needs to remain same.
    SpecialPort_UsageContinuousTcp,
    SpecialPort_UsageLongPollTcp,
    SpecialPort_UsageTcp,
    SpecialPort_UrlTcp,
    SpecialPort_UsageWS,
    SpecialPort_UsageWeb,
    SpecialPort_UrlWeb,
    SpecialPort_StatusPort,
    SpecialPort_GreetingMsgTCP,
    SpecialPort_ErrorPortTCP,
    SpecialPort_ForwardingMapTcp,
    SpecialPort_Maximum,
};

struct SpecialPortConfig: virtual public pinggy::SharedObject
{
public:
    SpecialPortConfig()         { }

    virtual
    ~SpecialPortConfig()        { }

    tPort                       ConfigTcp               = SpecialPort_ConfigTcp;
    tPort                       UsageContinuousTcp      = SpecialPort_UsageContinuousTcp;
    tPort                       UsageOnceLongPollTcp    = SpecialPort_UsageLongPollTcp;
    tPort                       UsageTcp                = SpecialPort_UsageTcp;
    tPort                       UrlTcp                  = SpecialPort_UrlTcp;
    tPort                       StatusPort              = SpecialPort_StatusPort;
    tPort                       GreetingMsgTCP          = SpecialPort_GreetingMsgTCP;
    tPort                       ForwardingMapTcp        = SpecialPort_ForwardingMapTcp;

};
DefineMakeSharedPtr(SpecialPortConfig);

#define SPECIAL_PORT_BASIC_FIELDS \
                            ConfigTcp, \
                            UsageContinuousTcp, \
                            UsageOnceLongPollTcp, \
                            UsageTcp, \
                            UrlTcp, \
                            StatusPort, \
                            GreetingMsgTCP, \
                            ForwardingMapTcp

#define CLIENT_SPECIFIC_USAGES_JSON_FIELDS_MAP \
                                            (ElapsedTime,            elaspedTime), \
                                            (ElapsedTime,            elapsedTime), \
                                            (NumLiveConnections,     numLiveConnections), \
                                            (NumTotalConnections,    numTotalConnections), \
                                            \
                                            (NumTotalResBytes,       numTotalResBytes), \
                                            (NumTotalReqBytes,       numTotalReqBytes), \
                                            (NumTotalTxBytes,        numTotalTxBytes) \

#define NONE_FUNC(x)
#define SAME_FUNC(x) x

#define CLIENT_SPECIFIC_USAGES_FIELDS_FUNC(x, y) \
                        x(ElapsedTime)            y(ElapsedTime)         \
                        x(NumLiveConnections)     y(NumLiveConnections)  \
                        x(NumTotalConnections)    y(NumTotalConnections) \
                        x(NumTotalResBytes)       y(NumTotalResBytes)    \
                        x(NumTotalReqBytes)       y(NumTotalReqBytes)    \
                        x(NumTotalTxBytes)        y(NumTotalTxBytes)

#define CLIENT_SPECIFIC_USAGES_FIELDS \
                CLIENT_SPECIFIC_USAGES_FIELDS_FUNC(SAME_FUNC, NONE_FUNC)

struct ClientSpecificUsages: virtual public pinggy::SharedObject
{
    ClientSpecificUsages()      { }

    virtual
    ~ClientSpecificUsages()     { }


    int64_t                     ElapsedTime;
    int64_t                     NumLiveConnections;
    int64_t                     NumTotalConnections;
    int64_t                     NumTotalResBytes;
    int64_t                     NumTotalReqBytes;
    int64_t                     NumTotalTxBytes;
};
DefineMakeSharedPtr(ClientSpecificUsages);

struct TunnelInfo: virtual public pinggy::SharedObject
{
    TunnelInfo()
                                { }

    virtual
    ~TunnelInfo()
                                { }

    SpecialPortConfigPtr        PortConfig;
    std::vector<tString>        GreetingMsg;

};
DefineMakeSharedPtr(TunnelInfo);

#endif // __SRC_CPP_PUBLIC_COMMON_UTILS_TUNNELCOMMON_HH__
