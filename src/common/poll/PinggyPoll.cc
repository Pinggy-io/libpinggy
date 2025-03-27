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

PollController::PollController() : pollTime(GetCurrentTimeInNS())
{
}

PollableTaskPtr
PollController::AddFutureTask(tDuration timeout, tDuration align, tDuration repeat, TaskPtr task)
{
    auto curTime = pollTime;
    auto deadline = curTime + timeout;
    if (align > 0) {
        align = (align / MILLISECOND) * MILLISECOND;
    }
    align = align > 0 ? align : MILLISECOND;
    if ( align > 0 ) {
        deadline = ((curTime + align - 1 + timeout)/align) * align;
    }
    if (repeat > 0) {
        repeat = ((repeat + align - 1)/align)*align;
    }
    auto pollableTask = NewPollableTaskPtr(task);
    pollableTask->deadline = deadline;
    pollableTask->repeat = repeat;

    taskQueue.push(pollableTask);

    LOGT("pushing ", pollableTask->deadline);

    return pollableTask;
}

tDuration PollController::GetNextTaskTimeout()
{
    if (taskQueue.size() == 0)
        return 0;

    auto task = taskQueue.top();

    LOGT("Get Next Task Timeout", task->deadline - pollTime, task->deadline, pollTime);

    Assert(task->deadline > pollTime);

    return task->deadline - pollTime;
}

bool PollController::HaveFutureTasks()
{
    return taskQueue.size() > 0;
}

void PollController::ExecuteCurrentTasks()
{
    pollTime = GetCurrentTimeInNS();

    while (taskQueue.size() > 0) {
        auto task = taskQueue.top();

        LOGT("ExecuteCurrentTasks", (task->deadline > pollTime ? "notnow" : "now"), (tInt64)(task->deadline - pollTime), task->deadline, pollTime, taskQueue.size());
        if (task->deadline > pollTime)
            return;
        Assert(task->deadline <= pollTime);

        taskQueue.pop();

        task->Fire();
        if (task->repeat > 0) {
            task->deadline += task->repeat;
            LOGT("Repeat pushing")
            taskQueue.push(task);
        }
    }
}

}; // NameSpace Common
