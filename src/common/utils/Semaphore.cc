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

#include "Semaphore.hh"


#if __cplusplus < 202002L

void Semaphore::Notify()
{
    std::unique_lock<std::mutex> lock(mutex);
    ++count; // Increment the count
    condition.notify_one(); // Notify one waiting thread
}

void Semaphore::Wait()
{
    std::unique_lock<std::mutex> lock(mutex);
    condition.wait(lock, [this] { return count > 0; }); // Wait until count is greater than 0
    --count; // Decrement the count
}
#else
void Semaphore::Notify()
{
    semaphore.release();
}

void Semaphore::Wait()
{
    semaphore.acquire();
}
#endif
