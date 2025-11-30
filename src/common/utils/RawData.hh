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

    template<typename T>
    T
    Read();

    char *
    GetData() { return Data+Offset; }

    char *
    GetWritableData() { return Data+Offset+Len; }

    tString
    ToString() { return tString(GetData(), Len);}

    void
    ReAlign();

    RawData::tLen
    WritableCapa() { return Capa - Offset - Len; }

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

#endif /* COMMON_RAWDATA_HH_ */

