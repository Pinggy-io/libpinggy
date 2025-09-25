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

#include "TunnelCommon.hh"
#include "StringUtils.hh"

TunnelMode
TunnelModeFromString(tString modeStr)
{
    auto sl = StringToLower(modeStr);
    if (sl.empty()) {
        return TunnelMode::None;
    } else if (sl == TunnelType_HTTP) {
        return TunnelMode::HTTP;
    } else if (sl == TunnelType_TCP) {
        return TunnelMode::TCP;
    } else if (sl == TunnelType_TLS) {
        return TunnelMode::TLS;
    } else if (sl == TunnelType_TLSTCP) {
        return TunnelMode::TLSTCP;
    } else if (sl == TunnelType_UDP) {
        return TunnelMode::UDP;
    } else {
        return TunnelMode::None;
    }
}

tString
TunnelTypeFromTunnelMode(TunnelMode mode)
{
    switch (mode)
    {
    case TunnelMode::HTTP:
        return TunnelType_HTTP;
    case TunnelMode::TCP:
        return TunnelType_TCP;
    case TunnelMode::TLS:
        return TunnelType_TLS;
    case TunnelMode::TLSTCP:
        return TunnelType_TLSTCP;
    case TunnelMode::UDP:
        return TunnelType_UDP;
    default:
        return TunnelType_Unknown;
    }
}