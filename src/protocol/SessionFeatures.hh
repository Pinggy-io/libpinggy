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

#ifndef __SRC_CPP_PUBLIC_PROTOCOL_SESSIONFEATURES_HH__
#define __SRC_CPP_PUBLIC_PROTOCOL_SESSIONFEATURES_HH__

#include <platform/SharedPtr.hh>


#define PINGGY_SESSION_VERSION_0_00 0x0000
#define PINGGY_SESSION_VERSION_1_00 0x1000
#define PINGGY_SESSION_VERSION_1_01 0x1001
#define PINGGY_SESSION_VERSION_1_02 0x1002


#ifndef PINGGY_SESSION_VERSION
#define PINGGY_SESSION_VERSION PINGGY_SESSION_VERSION_1_02 //major minor
#endif

namespace protocol
{

class SessionFeatures: virtual public pinggy::SharedObject
{
public:
    SessionFeatures(tUint32 version);

    virtual
    ~SessionFeatures()          { }

    virtual tUint64
    NegotiateVersion(tUint32 expectedVersion);

    void
    SetVersion(tUint32 version);

    tUint32
    GetVersion()                { return version; }

    /**
     * @brief Whether fastConnect enabled or not. If fastConnect is enabled, session have to handle it.
     * @return
     */
    const bool
    IsFastConnect()
                                { return fastConnect; }

    /**
     * @brief setup close timeout.
     * @return
     */
    const bool
    IsCloseTimeoutChannel()     { return closeTimeOutChannel; }

    /**
     * @brief Whether implicit usages is part of protocol or not.
     * @return
     */
    const bool
    IsImplicitUsages()          { return implicitUsagesAndGreeting; }

    const bool
    IsImplicitGreeting()        { return implicitUsagesAndGreeting; }

    /**
     * @brief If primary forwardingmode enabled. It is old standard where we used to use the concept of primary forwarding.
     *        In primary forwarding mode, tcp and udp both can be forwarded with single request. This is no longer true.
     *        Newer client should not use this technique at all. However, the flag is there for backward compatibility.
     *        Server side backward compatibility is easy to handle and already added code for the same.
     * @return
     */
    const bool
    IsPrimaryForwardingModeEnabled()
                                { return primaryForwardingMode; }

private:

    void
    configureFeatures();

    void
    resetToDefault();

    tUint32                     version;
                //Channel initiator can continue sending data without waiting for Accept msg.
    bool                        fastConnect         = false;
                //Setup a timeout for before a channel give up waiting for close.
    bool                        closeTimeOutChannel = false;
                //for backward compatibility with older sdk.
    bool                        primaryForwardingMode = true;
                //whether usage already present in the protocol or not
    bool                        implicitUsagesAndGreeting = false;
};
DefineMakeSharedPtr(SessionFeatures);

} // namespace protocol

#endif // __SRC_CPP_PUBLIC_PROTOCOL_SESSIONFEATURES_HH__