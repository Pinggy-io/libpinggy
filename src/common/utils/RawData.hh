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


#ifndef COMMON_RAWDATA_HH_
#define COMMON_RAWDATA_HH_

/*
 * RAW DATA
 */

#include <platform/platform.h>
#include <platform/SharedPtr.hh>
#include <string>
#include <cstdio>
#include <cstring>

enum RawDataError{
    RawData_NoError = 0,
    RawData_Error = 1
};

DeclareStructWithSharedPtr(RawData);


struct RawData : virtual pinggy::SharedObject{
    typedef int32_t             tLen;

    RawData(const void *, RawData::tLen len);

    RawData(void *, RawData::tLen len, bool copy);

    RawData(RawData::tLen capa = 2048);

    virtual
    ~RawData();

    static RawDataPtr
    WrapRawData(void *data, RawData::tLen len);

    RawDataPtr
    Slice(RawData::tLen offset, RawData::tLen len = -1);

    bool
    Reset();

    bool
    AddData(const void *, RawData::tLen);

    bool
    AddData(std::string str);

    bool
    AddData(RawDataPtr other);

    bool
    AddData(char c);

    template<typename T>
    bool
    AddData(T m);

    RawDataPtr
    Concat(RawDataPtr other);

    char *
    Consume(RawData::tLen len = -1);

    RawData::tLen
    Consume(char *buf, RawData::tLen capa);

    template<typename T>
    T
    Read();

    char *
    GetData()                   { return Data+Offset; }

    RawData::tLen
    GetData(char *buf, RawData::tLen capa);

    char *
    GetWritableData()           { return Data+Offset+Len; }

    void
    ReAlign();

    RawData::tLen
    WritableCapa()              { return Capa - Offset - Len; }

    virtual void
    Repr(std::ostream &os) override
    {
        os << tString(GetData(), Len);
    }

#define RAW_DATA_EXTRA_SIZE (Capa > 0 ? sizeof(*Data)*Capa : 0)

    DefineMandatoryClassFunctionsWOSuperWithExcessSize(RawData, RAW_DATA_EXTRA_SIZE);
        //we are using macro because we cannot find any other mechanism parse these functions yet.

    char *                      Data;
    RawData::tLen               Len;
    RawData::tLen               Offset;
    const RawData::tLen         Capa;
    RawDataError                Error;

private:
    bool                        owner;
    bool                        movedata;
    RawDataPtr                  parent;
};

DefineMakeSharedPtr(RawData);

#ifndef NO_SHARED_PTR
inline RawDataPtr NewRawDataNoCopyPtr(void *data, RawData::tLen len) {
    return std::make_shared<RawData>(data, len, false);
}
inline RawDataPtr NewRawDataPtr(std::string str) {
    return std::make_shared<RawData>(str.c_str(), (RawData::tLen)str.length());
}
#else
inline RawDataPtr NewRawDataNoCopyPtr(void *data, RawData::tLen len) {
    return new RawData(data, len, false);
}
inline RawDataPtr NewRawDataPtr(std::string str) {
    return new RawData(str.c_str(), (RawData::tLen)str.length());
}
#endif

#ifndef NO_SHARED_PTR
typedef std::shared_ptr<RawDataPtr> RawDataPtrPtr;

inline RawDataPtrPtr operator& (RawDataPtr rdp) {
    return std::make_shared<RawDataPtr>(rdp);
}

inline RawDataPtr operator+(RawDataPtr rdp, RawData::tLen len) {
    if (len > 0)
        rdp->Consume(len);
    return rdp;
}

inline RawDataPtr operator+(RawDataPtr rdp, const std::string &str) {
    rdp->AddData(str);
    return rdp;
}

inline RawDataPtr operator+(RawDataPtr rdp, const RawDataPtr &rdp1) {
    rdp->AddData(rdp1->GetData(), rdp1->Len);
    return rdp;
}
#else
typedef RawData **RawDataPtrPtr;
#endif

template <typename T>
inline bool
RawData::AddData(T m)
{
    return AddData((const void *)&m, sizeof(m));
}

template <typename T>
inline T
RawData::Read()
{
    auto data = GetData();
    T ele = *((T *)data);
    Consume(sizeof(T));
    return ele;
}

inline void
DumpValue(std::ostream &os, const RawDataPtr &ptr)
{
    if (ptr) {
        os << "<RawDataPtr at " << static_cast<const void*>(ptr.get()) << ", size=" << ptr->Len << ">";
    } else {
        os << "nullptr";
    }
}

template <typename T>
inline void
DumpValue(const RawDataPtr &rawData, const T &val)
{
    if constexpr (std::is_same_v<T, tString>) {
        if (val.length() < 16) {
            rawData->AddData(val.c_str(), val.size());
        } else {
            auto s = "<string len=" + std::to_string(val.length()) + ">";
            rawData->AddData(s.c_str(), s.size());
        }
    } else if constexpr (std::is_pointer_v<T> && std::is_same_v<std::remove_cv_t<std::remove_pointer_t<T>>, char>) {
        if (val) {
            auto len = std::strlen(val);
            if (len < 16) {
                rawData->AddData(val, len);
            } else {
                auto s = "<char* len=" + std::to_string(len) + ">";
                rawData->AddData(s.c_str(), s.size());
            }
        } else {
            rawData->AddData("nullptr", 7);
        }
    } else if constexpr (std::is_function_v<T>) {
        rawData->AddData("<func>", 6);
    } else if constexpr (std::is_pointer_v<T>) {
        if constexpr (std::is_function_v<std::remove_pointer_t<std::decay_t<T>>>
                   || std::is_function_v<std::remove_reference_t<T>>) {
            rawData->AddData("<ptr>", 5);
        } else {
            char buf[32];
            auto n = std::snprintf(buf, sizeof(buf), "%p", static_cast<const void*>(val));
            rawData->AddData(buf, static_cast<size_t>(n));
        }
    } else if constexpr (std::is_same_v<T, tInt8>) {
        auto s = std::to_string(static_cast<int>(val));
        rawData->AddData(s.c_str(), s.size());
    } else if constexpr (std::is_same_v<T, tUint8>) {
        auto s = std::to_string(static_cast<unsigned int>(val));
        rawData->AddData(s.c_str(), s.size());
    } else if constexpr (std::is_same_v<T, bool>) {
        if (val) rawData->AddData("true", 4); else rawData->AddData("false", 5);
    } else if constexpr (std::is_arithmetic_v<T>) {
        auto s = std::to_string(val);
        rawData->AddData(s.c_str(), s.size());
    } else {
        rawData->AddData("unknown", 7);
    }
}

template <typename T>
inline void
DumpValue(const RawDataPtr &rawData, const std::shared_ptr<T> &ptr)
{
    if (ptr) {
        char buf[64];
        int n;
        if constexpr (std::is_same_v<T, RawData>) {
            n = std::snprintf(buf, sizeof(buf), "<RawData size=%d>", static_cast<int>(ptr->Len));
        } else {
            n = std::snprintf(buf, sizeof(buf), "<shared_ptr at %p>",
                              static_cast<const void*>(ptr.get()));
        }
        rawData->AddData(buf, static_cast<size_t>(n));
    } else {
        rawData->AddData("nullptr", 7);
    }
}

#endif /* COMMON_RAWDATA_HH_ */

