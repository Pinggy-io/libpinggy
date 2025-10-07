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

#ifndef __COMMON_PINGGY_POLL_CONTROLLER_GENERIC_HH__
#define __COMMON_PINGGY_POLL_CONTROLLER_GENERIC_HH__

#include <platform/pinggy_types.h>
#include <map>
#include <vector>
#include <set>
#include <platform/platform.h>
#include "PinggyPoll.hh"

namespace common {

DeclareStructWithSharedPtr(FdMetaData);
DeclareStructWithSharedPtr(NonPollableMetaData);

class PollControllerGeneric : public PollController {
public:
    PollControllerGeneric();
    ~PollControllerGeneric() override;

    virtual void RegisterHandler(PollEventHandlerPtr handler, bool edgeTriggered = false) override;
    virtual void DisableReader(PollEventHandlerPtr handler) override;
    virtual void EnableReader(PollEventHandlerPtr handler) override;
    virtual void DisableWriter(PollEventHandlerPtr handler) override;
    virtual void EnableWriter(PollEventHandlerPtr handler) override;
    virtual void DeregisterHandler(PollEventHandlerPtr handler) override;
    virtual void RaiseReadPoll(PollEventHandlerPtr handler) override;
    virtual void RaiseWritePoll(PollEventHandlerPtr handler) override;
    virtual PollStatePtr RetrieveState(PollEventHandlerPtr handler) override;
    virtual void RestoreState(PollEventHandlerPtr handler, PollStatePtr state) override;
    virtual void DeregisterAllHandlers() override;
    virtual void StartPolling() override;
    virtual tInt PollOnce(tInt timeout = -1) override;
    virtual sock_t GetFd() override;
    virtual void CleanupAfterFork() override;
    virtual void StopPolling() override {stopPolling = true;}

private:
    void enableDisableHandler(sock_t fd, short mode, bool enable);

    void registerFd(sock_t fd);
    void deregisterFd(sock_t fd);

    void pollNonPollables();

    std::map<sock_t, PollEventHandlerPtr> fds;
    std::map<sock_t, FdMetaDataPtr > socketState;

    tPollFd       *pollEvents;
    int16_t        capaPollEvents;
    int16_t        numPollEvents;
    tPollFd       *pollEventsForIterator;
    int16_t        capaPollEventsForIterator;
    int16_t        numPollEventsForIterator;
    bool           reinit;

    std::set<sock_t> dummyReadPoll;
    std::set<PollEventHandlerPtr> dummyRead4NonPollables;
    std::set<PollEventHandlerPtr> dummyWrite4NonPollables;
    sock_t notificationFd, notificationReceiverFd;
    bool notified;
    bool stopPolling;
    bool polling;
    std::map<PollEventHandlerPtr, NonPollableMetaDataPtr> nonPollables;
};

DefineMakeSharedPtr(PollControllerGeneric);

} // namespace common

#endif // __COMMON_PINGGY_POLL_CONTROLLER_GENERIC_HH__
