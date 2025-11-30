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


#ifndef SRC_CPP_COMMON_THREADPOOL_HH_
#define SRC_CPP_COMMON_THREADPOOL_HH_

#include <platform/SharedPtr.hh>
#include <platform/platform.h>
#include "PinggyPoll.hh"
#include "PollableFD.hh"
#include <functional>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <queue>


//based on https://stackoverflow.com/questions/15752659/thread-pooling-in-c11
namespace common {

abstract class ThreadPoolEventHandler: virtual public pinggy::SharedObject {
public:
    virtual
    ~ThreadPoolEventHandler() { };

    virtual void
    EventOccured() = 0;
};

DeclareSharedPtr(ThreadPoolEventHandler);

class ThreadPool final : virtual public pinggy::SharedObject, public PollEventHandler
{

public:
    ThreadPool();

    virtual
    ~ThreadPool();

    virtual void
    Start(uint32_t numThreads=2);

    virtual void
    QueueJob(const std::function<void()>& job);

    virtual void
    Stop();

    virtual void
    StopAfterFork();

    virtual bool
    Busy();

    virtual void
    SetPollController(common::PollControllerPtr);

    virtual void
    RegisterEventHandler(ThreadPoolEventHandlerPtr hndlr);

    virtual void
    DeregisterEventHandler();

    // PollEventHandler
    virtual sock_t
    GetFd() override            { return pollFd; };

    virtual len_t
    HandlePollError(int16_t) override;

    virtual len_t
    HandlePollRecv() override;



protected:
    // virtual int CloseNClear(tString location) override { return 0; }

private:
    void
    threadLoop();

    bool                        started;
    bool                        stopped;
    bool                        should_terminate;           // Tells threads to stop looking for jobs
    std::mutex                  queue_mutex;                  // Prevents data races to the job queue
    std::condition_variable     mutex_condition; // Allows threads to wait on new jobs or termination
    std::vector<std::thread>    threads;
    std::queue<std::function<void()>>
                                jobs;
    sock_t                      pollFd;
    sock_t                      notificationFd;
    ThreadPoolEventHandlerPtr   handler;
    common::PollControllerPtr   pollController;
};
DefineMakeSharedPtr(ThreadPool);

} /* namespace common */

#endif /* SRC_CPP_COMMON_THREADPOOL_HH_ */
