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

#include "SharedPtr.hh"


namespace pinggy {

tUint64              SharedObject::gAllocationCount = 0;
tUint64              SharedObject::gDeallocationCount = 0;
tUint8               SharedObject::gCurrentVisitId = INITIAL_MEMORY_DUMP_VISIT_ID;

void
SharedObject::incrementAllocationCount()
{
    gAllocationCount++;
}

void
SharedObject::incrementDeallocationCount()
{
    gDeallocationCount++;
}

bool
SharedObject::IsMemoryDumpingAllowed()
{
    if (gCurrentVisitId == mDumpId)
        return false;
    mDumpId = gCurrentVisitId;
    return true;
}

tUint64
SharedObject::GetAllocationCount()
{
    return gAllocationCount;
}

tUint64
SharedObject::GetDeallocationCount()
{
    return gDeallocationCount;
}

void
SharedObject::InitiateNextMemoryDumpVisit()
{
    gCurrentVisitId += 1;
}

}