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

namespace common {

PollController::PollController() : pollTime(GetCurrentTimeInNS()), waitForTask(true)
{
}

PollableTaskPtr
PollController::AddFutureTask(tDuration timeout, tDuration align, bool repeat, TaskPtr task)
{
    auto curTime = pollTime;
    auto deadline = curTime + timeout;
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

    auto pollableTask = NewPollableTaskPtr(task);
    pollableTask->deadline          = deadline;
    pollableTask->isRepeat          = repeat;
    pollableTask->timeout           = timeout;
    pollableTask->alignment         = align;

    taskQueue.push(pollableTask);

    LOGT("pushing ", pollableTask->deadline);

    return pollableTask;
}

tDuration PollController::GetNextTaskTimeout()
{
    if (taskQueue.size() == 0)
        return 0;

    auto task = taskQueue.top();

    // LOGT("Get Next Task Timeout", task->deadline - pollTime, task->deadline, pollTime);

    Assert(task->deadline > pollTime);

    return task->deadline - pollTime;
}

void
PollController::ExecuteCurrentTasks()
{
    pollTime = GetCurrentTimeInNS();

    while (taskQueue.size() > 0) {
        auto task = taskQueue.top();

        // LOGT("ExecuteCurrentTasks", (task->deadline > pollTime ? "notnow" : "now"), (tInt64)(task->deadline - pollTime), task->deadline, pollTime, taskQueue.size());
        if (task->deadline > pollTime)
            return;
        Assert(task->deadline <= pollTime);

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
