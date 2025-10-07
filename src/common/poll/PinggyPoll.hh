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


#ifndef SERVER_PINGGYPOLL_HH_
#define SERVER_PINGGYPOLL_HH_
#include <memory>
#include <platform/network.h>
#include <platform/assert_pinggy.h>
#include <platform/SharedPtr.hh>
#include <chrono>
#include <queue>
#include "FutureTask.hh"


typedef uint64_t tDuration;
typedef uint64_t tTime;

template <typename eleType>
using PriorityQueueMinHeap = std::priority_queue<eleType, std::vector<eleType>, std::greater<eleType>>;

namespace common {

abstract class PollEventHandler : public virtual pinggy::SharedObject {
public:
    virtual
    ~PollEventHandler()         {}

    virtual sock_t
    GetFd() = 0;

    virtual len_t
    HandlePollRecv() = 0;

    virtual len_t
    HandlePollSend()            { Assert(false && "NOT IMPLEMENTED"); return 0;}

    virtual len_t
    HandlePollError(int16_t) = 0;

    virtual bool
    IsPollable()                { return true; }

    virtual bool
    IsRecvReady()               { return true; }

    virtual bool
    IsSendReady()               { return true; }
};
DeclareSharedPtr(PollEventHandler);

abstract class PollState: virtual public pinggy::SharedObject
{
public:
    virtual
    ~PollState()                {}

    virtual bool
    IsReadEnable() = 0;

    virtual bool
    IsWriteEnable() = 0;

    virtual bool
    IsReadEdgeTriggerEnable() = 0;

    virtual bool
    IsDummyReadEnabled() = 0;

    virtual bool
    IsDummyWriteEnabled() = 0;

    virtual bool
    IsPollable() = 0;
};
DeclareSharedPtr(PollState);

#define GetCurrentTimeInSec() \
    (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count())
#define GetCurrentTimeInMS() \
    (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count())

//Pinggy works on millisecond scale only, primarily because of the poll, epoll's behavior. It is meaning less to use microsecond or nanosecond
#if 0
#define GetCurrentTimeInUS() \
    (std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count())
#define GetCurrentTimeInNS() \
    (std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count())
#endif

#define  NANOSECOND         #ERROR "Nano second is not usable. use millisecond" // ((uint64_t)1)
#define  MICROSECOND        #ERROR "Micro second is not usable. use millisecond" //(1000*NANOSECOND)
#define  MILLISECOND        ((uint64_t)1) //(1000*MICROSECOND)
#define  SECOND             (1000*MILLISECOND)
#define  MINUTE             (60*SECOND)
#define  HOUR               (60*MINUTE)
// #define  MILLIS_IN_SECOND   (1000L)
// #define  NANOS_IN_MILLI     (1000L)

class PollController;
class PollableTask : public virtual pinggy::SharedObject{
public:
    PollableTask(TaskPtr task): deadline(0), isRepeat(false), task(task)
                                {}

    virtual
    ~PollableTask()             {}

    virtual void
    DisArm()                    { task = nullptr; isRepeat = false; }

    virtual void
    Fire()                      { if (task) task->Fire(); }

    virtual tUint64
    Hash() override final       { return deadline; }

private:
    friend class PollController;
    tDuration                   alignment;
    tDuration                   timeout;
    tTime                       deadline;
    bool                        isRepeat;
    TaskPtr                     task;
};
DefineMakeSharedPtr(PollableTask);

abstract class PollController : public virtual pinggy::SharedObject {
public:
    PollController();

    virtual
    ~PollController() {};

    virtual void
    RegisterHandler(PollEventHandlerPtr handler, bool edgeTriggered = false) = 0;

    virtual void
    DisableReader(PollEventHandlerPtr handler) = 0;

    virtual void
    EnableReader(PollEventHandlerPtr handler) = 0;

    virtual void
    DisableWriter(PollEventHandlerPtr handler) = 0;

    virtual void
    EnableWriter(PollEventHandlerPtr handler) = 0;

    virtual void
    DeregisterHandler(PollEventHandlerPtr handler) = 0;

    virtual void
    RaiseReadPoll(PollEventHandlerPtr handler) = 0;

    virtual void
    RaiseWritePoll(PollEventHandlerPtr handler) = 0;

    virtual PollStatePtr
    RetrieveState(PollEventHandlerPtr handler) = 0;

    virtual void
    RestoreState(PollEventHandlerPtr handler, PollStatePtr state) = 0;

    virtual void
    DeregisterAllHandlers() = 0;

    virtual void
    StartPolling() = 0;

    virtual tInt
    PollOnce(int timeout = -1) = 0;

    virtual sock_t
    GetFd() = 0;

    virtual void
    CleanupAfterFork() = 0;

    virtual void
    StopPolling() = 0;

    virtual PollableTaskPtr
    AddFutureTask(tDuration timeout, tDuration align, bool repeat, TaskPtr task) final;

    template<typename ...Args>
    PollableTaskPtr
    SetTimeout(tDuration timeout, void (*func)(Args ...), Args ...args);

    template<typename ...Args>
    PollableTaskPtr
    SetTimeout(tDuration timeout, tDuration align, void (*func)(Args ...), Args ...args);

    template<typename T, typename ...Args>
    PollableTaskPtr
    SetTimeout(tDuration timeout, std::shared_ptr<T> _t, void (T::*func)(Args ...), Args ...args);

    template<typename T, typename ...Args>
    PollableTaskPtr
    SetTimeout(tDuration timeout, tDuration align, std::shared_ptr<T> _t, void (T::*func)(Args ...), Args ...args);

    //=========================

    template<typename ...Args>
    PollableTaskPtr
    SetInterval(tDuration timeout, void (*func)(Args ...), Args ...args);

    template<typename ...Args>
    PollableTaskPtr
    SetInterval(tDuration timeout, tDuration align, void (*func)(Args ...), Args ...args);

    template<typename T, typename ...Args>
    PollableTaskPtr
    SetInterval(tDuration timeout, std::shared_ptr<T> _t, void (T::*func)(Args ...), Args ...args);

    template<typename T, typename ...Args>
    PollableTaskPtr
    SetInterval(tDuration timeout, tDuration align, std::shared_ptr<T> _t, void (T::*func)(Args ...), Args ...args);

    virtual tTime
    GetPollTime() final         { return pollTime; }

    virtual void
    SetWaitForFutureTask(bool waitForTask) final
                                { this->waitForTask = waitForTask; }

protected:

    // returns 0 or positive. 0 means immediate
    virtual tDuration
    GetNextTaskTimeout(int timeout = -1) final; //zero if no deadline

    virtual bool
    HaveFutureTasks(int timeout = -1) final { return taskQueue.size() > 0 || timeout > -1; }

    virtual bool
    WaitForFutureTask() final
                            { return waitForTask; }

    virtual void
    ExecuteCurrentTasks() final;

    virtual void
    CleanupAllTasks() final;

private:
    // std::priority_queue<PollableTaskPtr, std::vector<PollableTaskPtr>, std::greater<PollableTaskPtr>>;
    PriorityQueueMinHeap<PollableTaskPtr>
                                taskQueue;

    tTime                       pollTime;
    bool                        waitForTask; //Whether poll should wait for task or not when all the fds are gone

};
DeclareSharedPtr(PollController);

template<typename ...Args>
inline PollableTaskPtr
PollController::SetTimeout(tDuration timeout, void(* func)(Args...), Args ...args)
{
    return SetTimeout(timeout, SECOND, func, args...);
}

template<typename ...Args>
inline PollableTaskPtr
PollController::SetTimeout(tDuration timeout, tDuration align, void(* func)(Args...), Args ...args)
{
    auto task = NewFutureTaskImplPtr(func, args...);
    return AddFutureTask(timeout, align, false, task);
}

template<typename T, typename ...Args>
inline PollableTaskPtr
PollController::SetTimeout(tDuration timeout, std::shared_ptr<T> _t, void (T::*func)(Args...), Args ...args)
{
    return SetTimeout(timeout, SECOND, _t, func, args...);
}

template<typename T, typename ...Args>
inline PollableTaskPtr
PollController::SetTimeout(tDuration timeout, tDuration align, std::shared_ptr<T> _t, void (T::*func)(Args...), Args ...args)
{
    auto task = NewFutureTaskImplPtr(_t, func, args...);
    return AddFutureTask(timeout, align, false, task);
}

//============

template<typename ...Args>
inline PollableTaskPtr PollController::SetInterval(tDuration timeout, void(*func)(Args...), Args ...args)
{
    return SetInterval(timeout, SECOND, func, args...);
}

template<typename ...Args>
inline PollableTaskPtr PollController::SetInterval(tDuration timeout, tDuration align, void(*func)(Args...), Args ...args)
{
    auto task = NewFutureTaskImplPtr(func, args...);
    return AddFutureTask(timeout, align, true, task);
}

template<typename T, typename ...Args>
inline PollableTaskPtr PollController::SetInterval(tDuration timeout, std::shared_ptr<T> _t, void(T::* func)(Args...), Args ...args)
{
    return SetInterval(timeout, SECOND, _t, func, args...);
}

template<typename T, typename ...Args>
inline PollableTaskPtr PollController::SetInterval(tDuration timeout, tDuration align, std::shared_ptr<T> _t, void(T::* func)(Args...), Args ...args)
{
    auto task = NewFutureTaskImplPtr(_t, func, args...);
    return AddFutureTask(timeout, align, true, task);
}

}; //NameSpace Common

#define PinggyTask(x) common::NewFutureTaskImplPtr([&]{x})

#endif /* SERVER_PINGGYPOLL_HH_ */
