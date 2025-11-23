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

#define PPOLLIN EPOLLIN
#define PPOLLOUT EPOLLOUT

#include "PinggyPollCommon.hh"

PollControllerLinux::PollControllerLinux():
            reinit(true), pollEvents(NULL), numEvents(0),
            notificationFd(InValidSocket), notificationReceiverFd(InValidSocket),
            notified(false), stopPolling(false), polling(false)
{
    std::string func = "Unknown ";
    pollfd = -1;
    pollfd = epoll_create1(EPOLL_CLOEXEC);
    func = "epoll_create1 ";
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
            auto ptr = new struct epoll_event[fds.size()+extraEvents];
            if(pollEvents) delete[] pollEvents;
            numEvents = fds.size() + extraEvents;
            pollEvents = ptr;
        }
        reinit = false;
    }

    if ((dummyReadPoll.size() > 0 || dummyRead4NonPollables.size() > 0 || dummyWrite4NonPollables.size() > 0) && !notified) {
        auto ret = app_send(notificationFd, "1", 1, 0);
        if (ret <= 0) {
            ABORT_WITH_MSG("Error occured")
        }
        notified = true;
    }

    int timeout = -1;
    if (HaveFutureTasks(argTimeout)) {
        timeout = (int)(GetNextTaskTimeout(argTimeout)/MILLISECOND);
    }

    auto nfds = epoll_wait(pollfd, pollEvents, numEvents, timeout);

    if (nfds == -1) {
        return -1;
    }

    ExecuteCurrentTasks();

    if (nfds > 0) {
        auto newDummyPoll = dummyReadPoll;
        dummyReadPoll.clear();
        for (int n = 0; n < nfds; ++n) {
            sock_t eventFd = pollEvents[n].data.fd;
            if (eventFd == notificationReceiverFd) {
                if (pollEvents[n].events & EPOLLIN) {
                    char buf[200];
                    auto ret = app_recv(notificationReceiverFd, buf, sizeof(buf), 0);
                    if (ret <= 0) {
                        LOGFE("Error:", notificationReceiverFd);
                        ABORT();
                    }
                } else {
                    LOGFE("Issue: ", pollEvents[n].events);
                    ABORT();
                }
                notified = false;
                continue;
            }
            if(fds.find(eventFd) == fds.end()) {
                LOGT("Removing Fd: " << eventFd);
                epoll_ctl(pollfd, EPOLL_CTL_DEL, eventFd, NULL);
                if(socketState.find(eventFd) != socketState.end())
                    socketState.erase(eventFd);
                continue;
            }
            newDummyPoll.erase(eventFd);
            socketState[eventFd]->dummyIn = false;
            socketState[eventFd]->dummyOut = false;
            auto entry = fds[eventFd];
            auto ev = pollEvents[n].events;
            if (ev & EPOLLIN)
                entry->HandlePollRecv();
            if (ev & EPOLLOUT)
                entry->HandlePollSend();
            if (ev & (~(EPOLLIN|EPOLLOUT)) ) {
                if (ev & EPOLLHUP) {
                    if (ev & EPOLLIN) {
                        LOGD("EPOLLHUP, ignoring ", ev, " fd: ", eventFd);
                        continue;
                    }
                }
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
    struct epoll_event ev, *evPtr = NULL;
    ev.data.fd = fd;
    ev.events = 0;
    evPtr = &ev;


    if(fds.find(fd) == fds.end())
        return;

//    LOGD("Disabling fd:" << fd);

    Assert(socketState.find(fd) != socketState.end());
    auto state = socketState[fd];

    bool in = state->in;
    bool out = state->out;

    if(mode&PPOLLIN)
        in = enable;
    if(mode&PPOLLOUT)
        out = enable;

    auto oldCnt = state->GetNumOps();
    state->in = in;
    state->out = out;
    auto newCnt = state->GetNumOps();

    if (newCnt == oldCnt)
        return;

    auto operation = EPOLL_CTL_MOD;
    if (oldCnt == 0)
        operation = EPOLL_CTL_ADD;
    else if (newCnt == 0)
        operation = EPOLL_CTL_DEL;

    if (in) {
        evPtr->events |= EPOLLIN;
        if (state->et)
            evPtr->events |= EPOLLET;
    }
    if (out)
        evPtr->events |= EPOLLOUT;


    if (operation == EPOLL_CTL_DEL)
        evPtr = NULL;

    if (epoll_ctl(pollfd, operation, fd, evPtr) == -1) {
        LOGE("epoll_ctl: " << app_get_strerror(app_get_errno()) << " Exiting");
        exit(1);
    }
}

void PollControllerLinux::registerNotificationFd()
{
    auto fd = notificationReceiverFd;
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    if (epoll_ctl(pollfd, EPOLL_CTL_ADD, fd, &ev) != 0) {
        LOGE("epoll_ctl: " << app_get_strerror(app_get_errno()) << " Exiting");
        exit(1);
    }
}

} /* namespace common */
