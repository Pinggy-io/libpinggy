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

#include "PinggyPoll.hh"
#include <utils/TemplateStreaming.hh> //this needs to be the last include



template <typename T>
inline size_t
DumpMemoryUsages(std::ostream &os, tString varName, const PriorityQueueMinHeap<T> &vect)
{
    size_t size = 0; //sizeof(vect);
    if (!varName.empty())
        os << quoteString(varName) << ": ";

    auto nvect = vect;

    os << "{\"type\":\"priority_queue\",\"size\":" << nvect.size() << ",\"elements\":[";
    bool first = true;
    size += vect.size() * sizeof(T);
    while(nvect.size()) {
        if (!first) os << ",";
        auto elem = nvect.top();
        nvect.pop();
        size += DumpMemoryUsages(os, "", elem);
        first = false;
    }
    os << "],\"Consumed\":" << size << "}";
    return size;
}

namespace common {

PollController::PollController() : pollTime(GetCurrentTimeInMS()), waitForTask(true)
{
}

PollableTaskPtr
PollController::AddFutureTask(TaskSchedule schedule, tDuration timeout, tDuration align, bool repeat, TaskPtr task)
{
    auto curTime = pollTime;
    auto deadline = curTime + timeout;
    if (schedule == TaskSchedule::Timer) { //we do not need any of the following computation for
        if (align > 0) {
            align = (align / MILLISECOND) * MILLISECOND;
        }
        align = align > 0 ? align : MILLISECOND;

        tDuration misaligned = timeout%align;
        timeout += (misaligned ? (align - misaligned) : 0); //aligning timeout
        if (timeout == 0) {
            timeout += align;
        }
        deadline = curTime + timeout;
        misaligned = deadline%align;
        deadline += (misaligned ? (align - misaligned) : 0);
    }

    auto pollableTask = NewPollableTaskPtr(task);
    pollableTask->deadline          = deadline;
    pollableTask->isRepeat          = repeat;
    pollableTask->timeout           = timeout;
    pollableTask->alignment         = align;

    switch (schedule)
    {
    case TaskSchedule::Timer:
        taskQueue.push(pollableTask);
        break;

    case TaskSchedule::NextBreak:
        immediateTaskQueue.push(pollableTask);
        break;

    default:
        Assert("Unknown taskSchedule types");

    }

    LOGT("pushing ", pollableTask->deadline);

    return pollableTask;
}

tDuration
PollController::GetNextTaskTimeout(int argTimeout)
{
    argTimeout = argTimeout < -1 ? -1 : argTimeout;
    PollableTaskPtr task = nullptr;
    while(taskQueue.size()) { //This would clear out unnessary task.
        task = taskQueue.top();
        if (task->task)
            break;
        taskQueue.pop();
        task = nullptr;
    }

    if (!task) {
        if (immediateTaskQueue.size()) {
            return argTimeout < 0 ? SECOND : (argTimeout*MILLISECOND);
        }
        return 0;
    }

    if (task->deadline <= pollTime) //task already pending. Need to execute immidiately.
        return 0;

    auto timeout = task->deadline - pollTime;
    if (argTimeout < 0)
        return timeout;
    if (argTimeout == 0)
        return 0;
    tDuration expectedTimeout = ((tUint) argTimeout) * MILLISECOND;

    if (timeout > expectedTimeout)
        timeout = expectedTimeout;

    return timeout;
}

void
PollController::ExecuteCurrentTasks()
{
    pollTime = GetCurrentTimeInMS();

    while (immediateTaskQueue.size() > 0) {
        auto task = immediateTaskQueue.top();
        immediateTaskQueue.pop();
        task->Fire();
    }

    while (taskQueue.size() > 0) {
        auto task = taskQueue.top();

        // LOGT("ExecuteCurrentTasks", (task->deadline > pollTime ? "notnow" : "now"), (tInt64)(task->deadline - pollTime), task->deadline, pollTime, taskQueue.size());
        if (task->deadline > pollTime)
            return;
        Assert(task->deadline <= pollTime); //This is okay

        taskQueue.pop();

        task->Fire();
        if (task->isRepeat) {
            task->deadline += task->timeout;
            LOGT("Repeat pushing");
            taskQueue.push(task);
        }
    }
}

void
PollController::CleanupAllTasks()
{
    while (taskQueue.size() > 0) {
        auto task = taskQueue.top();
        task->DisArm();
        taskQueue.pop();
    }
}

}; // NameSpace Common

INCLUDE_MEMORY_DUMP_DEFINITION
