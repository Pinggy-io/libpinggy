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


#ifndef COMMON_PINGGYPOLLLINUX_HH_
#define COMMON_PINGGYPOLLLINUX_HH_

#include <platform/pinggy_types.h>
#include "PinggyPoll.hh"
#include <sys/wait.h>
#ifdef __LINUX_OS__
#include <sys/epoll.h>
#elif defined(__MAC_OS__)
#include <sys/event.h>
#endif // __LINUX_OS__

#include <set>
#include <map>


namespace common {

DeclareStructWithSharedPtr(FdMetaData);
DeclareStructWithSharedPtr(NonPollableMetaData);

class PollControllerLinux: public PollController
{
public:
    PollControllerLinux();
    virtual ~PollControllerLinux();
    virtual tInt PollOnce(tInt timeout = -1) override;
    virtual void StartPolling() override;
    virtual void DisableReader(PollEventHandlerPtr handler) override;
    virtual void DeregisterHandler(PollEventHandlerPtr handler) override;
    virtual void EnableReader(PollEventHandlerPtr handler) override;
    virtual void DisableWriter(PollEventHandlerPtr handler) override;
    virtual void EnableWriter(PollEventHandlerPtr handler) override;
    virtual void RaiseReadPoll(PollEventHandlerPtr handler) override;
    virtual void RaiseWritePoll(PollEventHandlerPtr handler) override;
    virtual void RegisterHandler(PollEventHandlerPtr handler, bool edgeTriggered = false) override;
    virtual PollStatePtr RetrieveState(PollEventHandlerPtr handler) override;
    virtual void RestoreState(PollEventHandlerPtr handler, PollStatePtr state) override;
    virtual void DeregisterAllHandlers() override;
    virtual int GetFd() override { return pollfd; }
    virtual void CleanupAfterFork() override;
    virtual void StopPolling() override {stopPolling = true;}

private:
    void enableDisableHandler(sock_t fd, uint mode, bool enable);

    void registerNotificationFd();

    void pollNonPollables();

    sock_t pollfd;
    bool reinit;
    std::map<sock_t, PollEventHandlerPtr> fds;
    std::map<sock_t, FdMetaDataPtr > socketState;

#ifdef __LINUX_OS__
    struct epoll_event *pollEvents;
#elif defined(__MAC_OS__)
    struct kevent *pollEvents;
#endif // __LINUX_OS__
    int numEvents;
    std::set<sock_t> dummyReadPoll;
    std::set<PollEventHandlerPtr> dummyRead4NonPollables;
    std::set<PollEventHandlerPtr> dummyWrite4NonPollables;
    sock_t notificationFd, notificationReceiverFd;
    bool notified;
    bool stopPolling;
    bool polling;
    std::map<PollEventHandlerPtr, NonPollableMetaDataPtr> nonPollables;
};

DefineMakeSharedPtr(PollControllerLinux);

} /* namespace common */


#endif /* COMMON_PINGGYPOLLLINUX_HH_ */
