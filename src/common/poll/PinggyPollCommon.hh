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
#ifndef __COMMON_PINGGY_POLL_COMMON_MAC_LINUX_HH__
#define __COMMON_PINGGY_POLL_COMMON_MAC_LINUX_HH__


#ifdef __COMMON_PINGGY_POLL_INCLUDE_COMMON_LINUX_MAC__

struct FdMetaData : public virtual PollState {
    bool in, out, et;
    bool dummyIn, dummyOut;
    FdMetaData(bool in, bool out, bool et): in(in), out(out), et(et), dummyIn(false), dummyOut(false) {}
    int GetNumOps() { return !!in + !!out; }
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



PollControllerLinux::~PollControllerLinux()
{
    if(pollEvents) {
        delete[] pollEvents;
    }
    CloseNCleanSocket(pollfd);
    CloseNCleanSocket(notificationFd);
    CloseNCleanSocket(notificationReceiverFd);
}

/*
 * EPOLLRDHUP
 * EPOLLPRI == POLLPRI
 * EPOLLERR == POLLERR
 * EPOLLHUP == POLLHUP //need a read event
 */



void PollControllerLinux::StartPolling()
{
    if(polling) {
        ABORT_WITH_MSG("Recursive polling call found");
    }
    polling = true;
    stopPolling = false;

    while(fds.size() || nonPollables.size() || HaveFutureTasks()) {
        auto ret = PollOnce();
        if (ret < 0) {
            if (app_get_errno() == EINTR) {
                continue;
            }
            LOGE("kevent|epoll_wait: " << app_get_strerror(app_get_errno()));
            exit(EXIT_FAILURE);
        }
        if (stopPolling)
            break;
    }

    polling = false;
}

inline void common::PollControllerLinux::pollNonPollables()
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
        meta->dummyIn = false;
    }
    for (auto x : newDummyWritePoll) {
        auto meta = newNonPollables[x];
        if(meta->out && x->IsSendReady())
            x->HandlePollSend();
        meta->dummyOut = false;
    }
}

void PollControllerLinux::RegisterHandler(common::PollEventHandlerPtr handler, bool edgeTriggered)
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
    Assert(pollfd > 0);
    Assert(fds.find(fd) == fds.end());
    fds[fd] = handler;
    socketState[fd] = NewFdMetaDataPtr(false, false, edgeTriggered);
    reinit = true;

    enableDisableHandler(fd, PPOLLIN, true);
}

void PollControllerLinux::DisableReader(common::PollEventHandlerPtr handler)
{
    if(!handler->IsPollable()) {
        if (nonPollables.find(handler) != nonPollables.end()) {
            nonPollables[handler]->in = false;
        }
        return;
    }
    sock_t fd = handler->GetFd();
    enableDisableHandler(fd, PPOLLIN, false);
}

void PollControllerLinux::EnableReader(common::PollEventHandlerPtr handler)
{
    if(!handler->IsPollable()) {
        if (nonPollables.find(handler) != nonPollables.end()) {
            nonPollables[handler]->in = true;
        }
        return;
    }
    sock_t fd = handler->GetFd();
    enableDisableHandler(fd, PPOLLIN, true);
}

void PollControllerLinux::DisableWriter(common::PollEventHandlerPtr handler)
{
    if(!handler->IsPollable()) {
        if (nonPollables.find(handler) != nonPollables.end()) {
            nonPollables[handler]->out = false;
        }
        return;
    }
    sock_t fd = handler->GetFd();
    enableDisableHandler(fd, PPOLLOUT, false);
}

void PollControllerLinux::EnableWriter(common::PollEventHandlerPtr handler)
{
    if(!handler->IsPollable()) {
        if (nonPollables.find(handler) != nonPollables.end()) {
            nonPollables[handler]->out = true;
        }
        return;
    }
    sock_t fd = handler->GetFd();
    enableDisableHandler(fd, PPOLLOUT, true);
}

void PollControllerLinux::DeregisterHandler(common::PollEventHandlerPtr handler)
{
    if (!handler->IsPollable()) {
        nonPollables.erase(handler);
        dummyRead4NonPollables.erase(handler);
        dummyWrite4NonPollables.erase(handler);
        return;
    }
    sock_t fd = handler->GetFd();
    LOGT( "removing fd:" << fd);
    Assert(pollfd > 0);
    Assert(fds.find(fd) != fds.end());
    if (fds.find(fd) == fds.end())
        return;

    enableDisableHandler(fd, PPOLLIN, false);
    enableDisableHandler(fd, PPOLLOUT, false); //this will remove it automatically.

    fds.erase(fd);
    reinit = true;

    auto state = socketState[fd];
    socketState.erase(fd);
    dummyReadPoll.erase(fd);
}

void PollControllerLinux::RaiseReadPoll(common::PollEventHandlerPtr handler)
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

void PollControllerLinux::RaiseWritePoll(PollEventHandlerPtr handler)
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

PollStatePtr common::PollControllerLinux::RetrieveState(PollEventHandlerPtr handler)
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

void common::PollControllerLinux::RestoreState(PollEventHandlerPtr handler, PollStatePtr state)
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

void PollControllerLinux::DeregisterAllHandlers()
{
    for(auto ent : fds) {
        if(socketState.find(ent.first) == socketState.end())
            DeregisterHandler(ent.second);
    }
    fds.clear();
    socketState.clear();
    nonPollables.clear();
    reinit = true;
}

void PollControllerLinux::CleanupAfterFork()
{
    CloseNCleanSocket(pollfd);
    CloseNCleanSocket(notificationFd);
    CloseNCleanSocket(notificationReceiverFd);
    fds.clear();
    socketState.clear();
    nonPollables.clear();
}

#else
#error You cannot just include this file.
#endif
#endif //__COMMON_PINGGY_POLL_COMMON_MAC_LINUX_HH__
