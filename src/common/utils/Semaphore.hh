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

#ifndef __SRC_CPP_COMMON_UTILS_SEMAPHORE_HH__
#define __SRC_CPP_COMMON_UTILS_SEMAPHORE_HH__

// Implement Semaphore only if C++ version is less than C++20
#if __cplusplus < 202002L

#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>

#else
#include <semaphore> // Use C++20's standard semaphore if available
// using Semaphore = std::counting_semaphore<>;
#endif

class Semaphore {
public:
#if __cplusplus < 202002L
    explicit Semaphore(int count = 1) : count(count) {}
#else
    explicit Semaphore(int count = 1) : semaphore(count) {}
#endif
    void Notify();

    void Wait();

private:
#if __cplusplus < 202002L
    std::mutex mutex; // Mutex for synchronizing access
    std::condition_variable condition; // Condition variable for blocking threads
    int count; // Count of available resources
#else
    std::counting_semaphore semaphore;
#endif
};



#endif // SRC_CPP_COMMON_UTILS_SEMAPHORE_HH__
