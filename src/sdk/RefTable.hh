#include <cstdint>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>

#include <platform/SharedPtr.hh>
#include "pinggy.h"



class RefTable
{
private:
    std::vector<tVoidPtr>       table;
    std::queue<pinggy_ref_t>    freeList;
    std::mutex                  lock;
    pinggy_ref_t                randomSeed;

    pinggy_void_t
    grow()
    {
        pinggy_ref_t oldSize = static_cast<pinggy_ref_t>(table.size());

        pinggy_ref_t newSize = (oldSize == 0) ? 1024 : oldSize * 2;

        table.resize(newSize);

        if (oldSize == 0)
            oldSize += 1; //just to avoid zero.

        for (pinggy_ref_t i = oldSize; i < newSize; ++i)
            freeList.push(i);
    }

public:
    explicit
    RefTable(pinggy_ref_t seed):randomSeed(seed)
    {
        grow();
    }

    pinggy_ref_t
    GetRef(tVoidPtr ptr)
    {
        std::lock_guard<std::mutex> guard(lock);

        if (freeList.empty())
            grow();

        pinggy_ref_t ref = freeList.front();
        freeList.pop();

        table[ref] = ptr;

        return ref+randomSeed;
    }

    tVoidPtr
    GetObj(pinggy_ref_t ref)
    {
        ref -= randomSeed;
        if (ref >= table.size())
            return nullptr;

        auto &entry = table[ref];

        return entry;
    }

    bool
    RemoveRef(pinggy_ref_t ref)
    {
        ref -= randomSeed;

        std::lock_guard<std::mutex> guard(lock);

        if (ref >= table.size())
            return false;

        auto& entry = table[ref];

        entry.reset();

        freeList.push(ref);

        return true;
    }
};