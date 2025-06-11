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

#include <algorithm>
#include <iostream>
#include <utils/Utils.hh>
#include <platform/assert_pinggy.h>
#include "PinggyPollGeneric.hh"


namespace common {
struct FdMetaData : public virtual PollState {
    bool in, out, et;
    bool dummyIn, dummyOut;
    FdMetaData(bool in, bool out, bool et): in(in), out(out), et(et), dummyIn(false), dummyOut(false) {}
    int GetNumOps() const { return !!in + !!out; }
    virtual bool IsReadEnable() override {return in;}
    virtual bool IsWriteEnable() override {return out;}
    virtual bool IsReadEdgeTriggerEnable() override {return et;}
    virtual bool IsDummyReadEnabled() override {return dummyIn;}
    virtual bool IsDummyWriteEnabled() override {return dummyOut;}
    virtual bool IsPollable() override {return true;}
};
DefineMakeSharedPtr(FdMetaData);

struct NonPollableMetaData : public virtual PollState {
    bool in, out;
    bool dummyIn, dummyOut;
    NonPollableMetaData(): in(false), out(false), dummyIn(false), dummyOut(false) {}
    virtual bool IsReadEnable() override {return in;}
    virtual bool IsWriteEnable() override {return out;}
    virtual bool IsReadEdgeTriggerEnable() override {return false;}
    virtual bool IsDummyReadEnabled() override {return dummyIn;}
    virtual bool IsDummyWriteEnabled() override {return dummyOut;}
    virtual bool IsPollable() override {return false;}
};
DefineMakeSharedPtr(NonPollableMetaData);



PollControllerGeneric::PollControllerGeneric():
            pollEvents(NULL), capaPollEvents(0), numPollEvents(0),
            pollEventsForIterator(NULL), capaPollEventsForIterator(0), numPollEventsForIterator(0),
            reinit(true), notificationFd(InValidSocket), notificationReceiverFd(InValidSocket),
            notified(false), stopPolling(false), polling(false)
{
    //TODO
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
    registerFd(notificationReceiverFd);
}

PollControllerGeneric::~PollControllerGeneric()
{
    CloseNCleanSocket(notificationFd);
    CloseNCleanSocket(notificationReceiverFd);
    if (pollEvents) {
        delete[] pollEvents;
    }
    if (pollEventsForIterator) {
        delete[] pollEventsForIterator;
    }
}

void PollControllerGeneric::RegisterHandler(PollEventHandlerPtr handler, bool edgeTriggered)
{
    if (!handler->IsPollable()) {
        auto metaData = NewNonPollableMetaDataPtr();
        metaData->in = true;
        Assert(nonPollables.find(handler) == nonPollables.end());
        nonPollables[handler] = metaData;
        return;
    }
    sock_t fd = handler->GetFd();

    LOGT("adding fd:" << fd);
    Assert(fd > 0);
    Assert(fds.find(fd) == fds.end());
    fds[fd] = handler;
    socketState[fd] = NewFdMetaDataPtr(false, false, edgeTriggered);

    registerFd(fd);

    enableDisableHandler(fd, POLLIN, true);
}

void PollControllerGeneric::DisableReader(PollEventHandlerPtr handler)
{
    if(!handler->IsPollable()) {
        if (nonPollables.find(handler) != nonPollables.end()) {
            nonPollables[handler]->in = false;
        }
        return;
    }
    sock_t fd = handler->GetFd();
    enableDisableHandler(fd, POLLIN, false);
}

void PollControllerGeneric::EnableReader(PollEventHandlerPtr handler)
{
    if(!handler->IsPollable()) {
        if (nonPollables.find(handler) != nonPollables.end()) {
            nonPollables[handler]->in = true;
        }
        return;
    }
    sock_t fd = handler->GetFd();
    enableDisableHandler(fd, POLLIN, true);
}

void PollControllerGeneric::DisableWriter(PollEventHandlerPtr handler)
{
    if(!handler->IsPollable()) {
        if (nonPollables.find(handler) != nonPollables.end()) {
            nonPollables[handler]->out = false;
        }
        return;
    }
    sock_t fd = handler->GetFd();
    enableDisableHandler(fd, POLLOUT, false);
}

void PollControllerGeneric::EnableWriter(PollEventHandlerPtr handler)
{
    if(!handler->IsPollable()) {
        if (nonPollables.find(handler) != nonPollables.end()) {
            nonPollables[handler]->out = true;
        }
        return;
    }
    sock_t fd = handler->GetFd();
    enableDisableHandler(fd, POLLOUT, true);
}

void PollControllerGeneric::DeregisterHandler(PollEventHandlerPtr handler)
{
    if (!handler->IsPollable()) {
        nonPollables.erase(handler);
        dummyRead4NonPollables.erase(handler);
        dummyWrite4NonPollables.erase(handler);
        return;
    }
    sock_t fd = handler->GetFd();
    LOGT( "removing fd:" << fd);
    Assert(fds.find(fd) != fds.end());

    enableDisableHandler(fd, POLLIN, false);
    enableDisableHandler(fd, POLLOUT, false); //this will remove it automatically.

    fds.erase(fd);

    auto state = socketState[fd];
    socketState.erase(fd);
    dummyReadPoll.erase(fd);

    deregisterFd(fd);
}

void PollControllerGeneric::RaiseReadPoll(PollEventHandlerPtr handler)
{
    if (!handler)
        return;

    if (!handler->IsPollable()) {
        if (nonPollables.find(handler) == nonPollables.end())
            return;
        dummyRead4NonPollables.insert(handler);
        nonPollables[handler]->dummyIn = true;
        return;
    }

    auto fd = handler->GetFd();

    if (!IsValidSocket(fd))
        return;

    dummyReadPoll.insert(fd);

    if (socketState.find(fd) != socketState.end())
        socketState[fd]->dummyIn = true;
}

void PollControllerGeneric::RaiseWritePoll(PollEventHandlerPtr handler)
{
    if (!handler)
        return;

    if (!handler->IsPollable()) {
        if (nonPollables.find(handler) == nonPollables.end())
            return;
        dummyWrite4NonPollables.insert(handler);
        nonPollables[handler]->dummyOut = true;
        return;
    }

    Assert(false);
}

PollStatePtr PollControllerGeneric::RetrieveState(PollEventHandlerPtr handler)
{
    if (handler->IsPollable()) {
        auto fd = handler->GetFd();
        if (socketState.find(fd) == socketState.end())
            return nullptr;
        return socketState[fd];
    }
    if (nonPollables.find(handler) == nonPollables.end())
        return nullptr;
    return nonPollables[handler];
}

void PollControllerGeneric::RestoreState(PollEventHandlerPtr handler, PollStatePtr state)
{
    if (handler->IsPollable()) {
        auto fd = handler->GetFd();
        if (socketState.find(fd) != socketState.end())
            socketState[fd]->et = state->IsReadEdgeTriggerEnable();
    }
    if (state->IsReadEnable())
        EnableReader(handler);
    if (state->IsWriteEnable())
        EnableWriter(handler);
    if (state->IsDummyReadEnabled())
        RaiseReadPoll(handler);
    if (state->IsDummyWriteEnabled())
        RaiseWritePoll(handler);
}

void PollControllerGeneric::DeregisterAllHandlers()
{
    for(auto ent : fds) {
        if(socketState.find(ent.first) == socketState.end())
            DeregisterHandler(ent.second);
    }
    fds.clear();
    socketState.clear();
    nonPollables.clear();
}

void PollControllerGeneric::StartPolling()
{
    if(polling) {
        ABORT_WITH_MSG("Recursive polling call found");
    }
    polling = true;
    stopPolling = false;

    while(fds.size() || nonPollables.size() || HaveFutureTasks()) {
        PollOnce();
        if (stopPolling)
            break;
    }

    polling = false;
}

tInt PollControllerGeneric::PollOnce()
{
    if(fds.size() == 0 && nonPollables.size() == 0 && HaveFutureTasks() == false) {
        app_set_errno(EINVAL);
        return -1;
    }

    if ((dummyReadPoll.size() > 0 || dummyRead4NonPollables.size() > 0 || dummyWrite4NonPollables.size() > 0) && !notified) {
        auto ret = app_send(notificationFd, "1", 1, 0);
        if (ret <= 0) {
            ABORT_WITH_MSG("Error occured")
        }
        notified = true;
    }

    if (capaPollEvents != capaPollEventsForIterator) {
        capaPollEventsForIterator = capaPollEvents;
        auto newPollEvents = new struct pollfd[capaPollEventsForIterator];

        if (pollEventsForIterator) {
            for (auto i = 0; i < numPollEvents; i++) {
                newPollEvents[i] = pollEventsForIterator[i];
            }
            delete[] pollEventsForIterator;
        }
        pollEventsForIterator = newPollEvents;
    }
    if (numPollEvents != numPollEventsForIterator || reinit) {
        numPollEventsForIterator = numPollEvents;
        for (auto i = 0; i < numPollEvents; i++)
            pollEventsForIterator[i] = pollEvents[i];
        reinit = false;
    }

    int timeout = 1000; //1 second by default
    if (HaveFutureTasks()) {
        timeout = (int)(GetNextTaskTimeout()/MILLISECOND);
    }

    int ret = poll(pollEventsForIterator, numPollEventsForIterator, timeout);
    if (ret < 0) {
        LOGE("poll() failed: ", app_get_strerror(app_get_errno()));
        return -1;
    }

    ExecuteCurrentTasks();

    if (ret > 0) { //if timeout happen, poll return 0

        auto newDummyPoll = dummyReadPoll;
        dummyReadPoll.clear();

        for (auto i = 0; i < numPollEventsForIterator; i++) {
            auto pfd = pollEventsForIterator + i;
            auto eventFd = pfd->fd;
            if (eventFd == notificationReceiverFd) {
                if (pfd->revents & POLLIN) {
                    char buf[200];
                    auto ret1 = app_recv(notificationReceiverFd, buf, sizeof(buf), 0);
                    if (ret1 <= 0) {
                        LOGFE("Error:", notificationReceiverFd);
                        ABORT();
                    }
                    notified = false;
                }
                continue;
            }

            if(fds.find(eventFd) == fds.end()) {
                LOGT("Removing Fd: " << eventFd);
                deregisterFd(eventFd);

                if(socketState.find(eventFd) != socketState.end())
                    socketState.erase(eventFd);
                continue;
            }

            newDummyPoll.erase(eventFd);
            auto entry = fds[eventFd];
            if (pfd->revents & POLLIN) {
                entry->HandlePollRecv();
            }
            if (pfd->revents & POLLOUT) {
                entry->HandlePollSend();
            }
            if (pfd->revents & (POLLERR | POLLHUP | POLLNVAL)) {
                entry->HandlePollError(pfd->revents);
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
            } else {
                dummyReadPoll.insert(eventFd);
            }
        }

        pollNonPollables();
    }

    return 0;
}

sock_t PollControllerGeneric::GetFd() {
    // This function is meant to return a file descriptor if needed
    // For simplicity, let's return INVALID_SOCKET
    return INVALID_SOCKET;
}

void PollControllerGeneric::CleanupAfterFork()
{
    CloseNCleanSocket(notificationFd);
    CloseNCleanSocket(notificationReceiverFd);
    fds.clear();
    socketState.clear();
    nonPollables.clear();
}

void PollControllerGeneric::enableDisableHandler(sock_t fd, short mode, bool enable)
{

    if(fds.find(fd) == fds.end())
        return;

    Assert(socketState.find(fd) != socketState.end());
    auto state = socketState[fd];
    if (mode == POLLIN) {
        state->in = enable;
    } else if (mode == POLLOUT) {
        state->out = enable;
    } else {
        ABORT_WITH_MSG("Invalide event");
    }

    for (auto i = 0; i < numPollEvents; i++) {
        auto pfd = pollEvents + i;
        if (pfd->fd == fd) {
            if (enable) {
                pfd->events |= mode;
            } else {
                pfd->events &= ~mode;
            }
            reinit = true;
            break;
        }
    }
}

void PollControllerGeneric::registerFd(sock_t fd)
{
    if (capaPollEvents == numPollEvents) {
        capaPollEvents += 1024;
        auto newPollEvents = new struct pollfd[capaPollEvents];

        if (pollEvents) {
            for (auto i = 0; i < numPollEvents; i++) {
                newPollEvents[i] = pollEvents[i];
            }
            delete[] pollEvents;
        }
        pollEvents = newPollEvents;
    }

    pollEvents[numPollEvents] = {fd, POLLIN, 0};
    numPollEvents += 1; //I know ++ would work here
    reinit = true;
}

void PollControllerGeneric::deregisterFd(sock_t fd)
{
    for(auto i = 0; i < numPollEvents; i++) {
        if (pollEvents[i].fd == fd) {
            pollEvents[i] = pollEvents[numPollEvents - 1]; //basically I am copiying the last one to new
            numPollEvents -= 1;
            break;
        }
    }
    Assert(numPollEvents >= 0)
    reinit = true;
}

void PollControllerGeneric::pollNonPollables()
{
    auto newDummyReadPoll = dummyRead4NonPollables;
    auto newDummyWritePoll = dummyWrite4NonPollables;
    auto newNonPollables = nonPollables;

    dummyRead4NonPollables.clear();
    dummyWrite4NonPollables.clear();

    for(auto x : newDummyReadPoll) { //Decides later if we need to perform multiple loop.
        auto meta = newNonPollables[x];
        if(meta->in && x->IsRecvReady())
            x->HandlePollRecv();
    }
    for (auto x : newDummyWritePoll) {
        auto meta = newNonPollables[x];
        if(meta->out && x->IsSendReady())
            x->HandlePollSend();
    }
}

} // namespace common
