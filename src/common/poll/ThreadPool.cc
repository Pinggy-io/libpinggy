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


#include <stdlib.h>
#include <platform/Log.hh>
#include "ThreadPool.hh"

namespace common {

ThreadPool::ThreadPool():started(false), stopped(false),
            should_terminate(false), pollFd(InValidSocket), notificationFd(InValidSocket) {}

ThreadPool::~ThreadPool()
{
    Stop();
    CloseNCleanSocket(pollFd);
    CloseNCleanSocket(notificationFd);
}

void ThreadPool::Start(uint32_t numThreads)
{
    if(started)
        abort();

    sock_t fds[2];
    if (app_socketpair(AF_UNIX, SOCK_STREAM, 0, fds) < 0) {
        abort();
    }

    notificationFd = fds[1];
    pollFd = fds[0];

    // const uint32_t num_threads = 2; // I do not need more than 1. However it is good to have more than one.
            //std::thread::hardware_concurrency(); // Max # of threads the system supports
    threads.resize(numThreads);
    for (uint32_t i = 0; i < numThreads; i++) {
        threads.at(i) = std::thread(&ThreadPool::threadLoop, this);
    }
}

void ThreadPool::QueueJob(const std::function<void()> &job)
{
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        jobs.push(job);
    }
    mutex_condition.notify_one();
}

void ThreadPool::Stop()
{
    if(stopped)
        return;
    started = false;
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        should_terminate = true;
    }
    mutex_condition.notify_all();
    for (std::thread& active_thread : threads) {
        active_thread.join();
    }
    threads.clear();
    LOGI("Thread pool stopped");
}


void ThreadPool::StopAfterFork()
{
    should_terminate = true;
//    threads.clear();
    CloseNCleanSocket(pollFd);
    CloseNCleanSocket(notificationFd);
}

bool ThreadPool::Busy()
{
    bool poolbusy;
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        poolbusy = jobs.empty();
    }
    return poolbusy;
}

void ThreadPool::RegisterEventHandler(ThreadPoolEventHandlerPtr hndlr)
{
    if (handler != nullptr)
        abort();
    handler = hndlr;
}

void ThreadPool::threadLoop()
{
    while (true) {
        std::function<void()> job;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            mutex_condition.wait(lock, [this] {
                return !jobs.empty() || should_terminate;
            });
            if (should_terminate) {
                LOGI("Thread loop stopped");
                return;
            }
            job = jobs.front();
            jobs.pop();
        }
        job();
        if(handler != nullptr) {
            auto ret = write(notificationFd, "1", 1);
            if(ret < 1) {
                LOGF("Threadpool misbehaving"); //TODO brainstorm
                abort();
            }
        }
    }
}

len_t ThreadPool::HandlePollError(int16_t shortInt)
{
    return 0;
}

len_t ThreadPool::HandlePollRecv()
{
    char buf[100];
    auto len = read(pollFd, buf, 100);
    if(len == 0) {
//        LOGF("Unknown Errors");
        abort();
    }
    for(;len > 0; len--)
        handler->EventOccured();
    return 0;
}


} /* namespace common */
