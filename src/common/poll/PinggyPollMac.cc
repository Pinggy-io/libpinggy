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


#include "PinggyPollLinux.hh"
#include <unistd.h>
#include <fcntl.h>
#include <algorithm>
#include <poll.h>

#include <platform/assert_pinggy.h>
#include <platform/Log.hh>
#include <utils/Utils.hh>

#define MAX_EVENTS 100

namespace common {
#define __COMMON_PINGGY_POLL_INCLUDE_COMMON_LINUX_MAC__

#define PPOLLIN 1
#define PPOLLOUT 2

#include "PinggyPollCommon.hh"

PollControllerLinux::PollControllerLinux():
            reinit(true), pollEvents(NULL), numEvents(0),
            notificationFd(InValidSocket), notificationReceiverFd(InValidSocket),
            notified(false), stopPolling(false), polling(false)
{
    std::string func = "Unknown ";
    pollfd = -1;
    pollfd = kqueue();
    func = "kqueue ";
    if (pollfd == -1) {
        LOGE(func << app_get_errno() << " " << app_get_strerror(app_get_errno()));
        exit(EXIT_FAILURE);
    }
    set_close_on_exec(pollfd);

    sock_t fds[2];
    int ret = app_socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
    if (ret < 0) {
        LOGEE("Error with socket pair");
        exit(EXIT_FAILURE);
    }
    LOGT("socket pair: ", fds[0], fds[1]);
    set_close_on_exec(fds[0]);
    set_close_on_exec(fds[1]);
    set_blocking(fds[0], 0);
    set_blocking(fds[1], 0);
    notificationFd          = fds[0];
    notificationReceiverFd  = fds[1];
    registerNotificationFd();
}

tInt32 PollControllerLinux::PollOnce(tInt32 argTimeout)
{
    if(fds.size() == 0 && nonPollables.size() == 0 && HaveFutureTasks(argTimeout) == false) {
        app_set_errno(EINVAL);
        return -1;
    }

    if(reinit) {
        int extraEvents = 2; //little buffer
        if((int)fds.size() + extraEvents > numEvents) {
            auto ptr = new struct kevent[fds.size()+extraEvents];
            if(pollEvents) delete[] pollEvents;
            numEvents = fds.size() + extraEvents;
            pollEvents = ptr;
        }
        reinit = false;
    }

    if ((dummyReadPoll.size() > 0 || dummyRead4NonPollables.size() > 0 || dummyWrite4NonPollables.size() > 0) && !notified) {
        auto ret = app_send(notificationFd, "1", 1, 0);
        if (ret <= 0) {
            ABORT_WITH_MSG("Error occured");
        }
        notified = true;
    }

    struct timespec localTimeSpec, *localTimeSpecPtr = NULL;
    if (HaveFutureTasks(argTimeout)) {
        auto timeout = GetNextTaskTimeout(argTimeout);
        localTimeSpec = timespec{
                            .tv_sec  = (time_t)(timeout / SECOND),
                            .tv_nsec = (time_t)(timeout % SECOND),
                        };
        localTimeSpecPtr = &localTimeSpec;
    }

    auto nfds = kevent(pollfd, NULL, 0, pollEvents, numEvents, localTimeSpecPtr);

    if (nfds == -1) {
        return -1;
    }

    ExecuteCurrentTasks();

    if (nfds > 0) { //if timeout happen, kqueue return 0
        auto newDummyPoll = dummyReadPoll;
        dummyReadPoll.clear();
        for (int n = 0; n < nfds; ++n) {
            sock_t eventFd = pollEvents[n].ident;
            if (eventFd == notificationReceiverFd) {
                if (pollEvents[n].filter == EVFILT_READ) {
                    char buf[200];
                    auto ret = app_recv(notificationReceiverFd, buf, sizeof(buf), 0);
                    if (ret <= 0) {
                        LOGFE("Error:", notificationReceiverFd);
                        ABORT();
                    }
                } else {
                    LOGFE("Issue: ", pollEvents[n].filter);
                    ABORT();
                }
                notified = false;
                continue;
            }
            if(fds.find(eventFd) == fds.end()) {
                LOGT("Removing Fd: " << eventFd);
                // auto metaData =
                struct kevent event[2];
                EV_SET(&event[1], eventFd, pollEvents[n].filter, EV_DELETE, 0, 0, 0);
                auto ret = kevent(pollfd, event, 2, NULL, 0, NULL);
                if (ret < 0) {
                    LOGEE("Cannot delete fd");
                }
                // epoll_ctl(pollfd, EPOLL_CTL_DEL, eventFd, NULL);
                if(socketState.find(eventFd) != socketState.end())
                    socketState.erase(eventFd);
                continue;
            }
            newDummyPoll.erase(eventFd);
            socketState[eventFd]->dummyIn = false;
            socketState[eventFd]->dummyOut = false;
            auto entry = fds[eventFd];
            if (pollEvents[n].filter == EVFILT_READ)
                entry->HandlePollRecv();
            else if (pollEvents[n].filter == EVFILT_WRITE)
                entry->HandlePollSend();
            else {
                auto ev = pollEvents[n].filter;
                entry->HandlePollError(ev);
            }
        }

        for (sock_t eventFd : newDummyPoll) {
            if(fds.find(eventFd) == fds.end()) {
                continue;
            }
            if(socketState.find(eventFd) == socketState.end())
                continue;

            if(socketState[eventFd]->in) {
                fds[eventFd]->HandlePollRecv();
                socketState[eventFd]->dummyIn = false;
            } else {
                dummyReadPoll.insert(eventFd);
                socketState[eventFd]->dummyOut = false;
            }
        }

        pollNonPollables();
    }
    return 0;
}

void PollControllerLinux::enableDisableHandler(sock_t fd, uint mode, bool enable)
{
    struct kevent ev;
    int16_t evFilt = 0;
    if(fds.find(fd) == fds.end())
        return;

    Assert(socketState.find(fd) != socketState.end());
    auto state = socketState[fd];
    bool isEnabled = false;
    uint16_t flags = 0;
    if (mode == POLL_IN) {
        evFilt = EVFILT_READ;
        isEnabled = state->in;
        if (enable && state->et) {
            flags |= EV_CLEAR;
        }
        state->in = enable;
    } else if (mode == POLL_OUT) {
        evFilt = EVFILT_WRITE;
        isEnabled = state->out;
        state->out = enable;
    } else {
        ABORT_WITH_MSG("Invalide event");
    }

    if (isEnabled == enable)
        return;

    if (enable) {
        flags |= EV_ADD;
    } else {
        flags |= EV_DELETE;
    }

    EV_SET(&ev, fd, evFilt, flags, 0, 0, 0);

    if (kevent(pollfd, &ev, 1, NULL, 0, NULL) == -1) {
        LOGEE("kevent");
        exit(1);
    }
}

void PollControllerLinux::registerNotificationFd()
{
    auto fd = notificationReceiverFd;
    struct kevent ev;
    EV_SET(&ev, fd, EVFILT_READ, EV_ADD, 0, 0, 0);
    if (kevent(pollfd, &ev, 1, NULL, 0, NULL) == -1) {
        LOGEE("kevent");
        exit(1);
    }
}

} /* namespace common */
