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

#include "RawData.hh"
#include <cstring>

/*
 * Raw data
 */

RawData::RawData(const void * _data, RawData::tLen _len): Data(new char[_len]), Len(_len), Offset(0), Capa(_len), Error(RawData_NoError), owner(true), movedata(true) {;
    std::memcpy(Data, _data, Len);
}


RawData::RawData(void * _data, RawData::tLen _len, bool copy):
                    Data(NULL), Len(_len), Offset(0), Capa(_len), Error(RawData_NoError),
                    owner(copy), movedata(copy)
{
    if (copy) {
        Data = new char[Len];
        std::memcpy(Data, _data, Len);
    } else {
        Data = (char *)_data;
    }
}

RawData::RawData(RawData::tLen _capa):
            Data(new char[_capa]), Len(0), Offset(0),
            Capa(_capa), Error(RawData_NoError), owner(true),
            movedata(true)
{
}

RawData::~RawData()
{
    if (Data && owner) delete[] Data;
    Data = nullptr;
    Len = 0;
}

RawDataPtr RawData::WrapRawData(void *data, RawData::tLen len)
{
    auto r = NewRawDataNoCopyPtr(data, len);
    r->movedata = true;
    return r;
}

RawDataPtr RawData::Slice(RawData::tLen offset, RawData::tLen len)
{
    if (offset < 0)
        offset = 0;
    if (offset >= Len) {
        offset = Len;
        len = 0;
    }

    if (len < 0) {
        len = Len - offset;
    }
    if (len > (Len - offset)) {
        len = Len - offset;
    }

    auto ret = NewRawDataNoCopyPtr(GetData()+offset, len);
    ret->parent = thisPtr;
    return ret;
}

bool RawData::Reset()
{
    if (!movedata) return false;
    Len = 0;
    Offset = 0;
    return true;
}

/*
 * Would increase the buffer and copy content
 */
bool RawData::AddData(const void *buf, RawData::tLen len)
{
    if (!movedata) return false;
    if (Offset + Len + len >= Capa) {
        if (Offset) {
            memmove(Data, Data+Offset, Len);
            Offset = 0;
        }
        if (Len + len > Capa) {
            if (!owner)
                return false;
            auto newData = new char[Len + len];
            memcpy(newData, Data+Offset, Len);
            delete[] Data;
            Data = newData;
        }
        Offset = 0;
    }
    memcpy(Data+Len+Offset, buf, len);
    Len += len;
    return true;
}

bool RawData::AddData(std::string str)
{
    return AddData(str.c_str(), (RawData::tLen)str.length());
}

bool RawData::AddData(RawDataPtr other)
{
    return AddData(other->Data, other->Len);
}

bool RawData::AddData(char c)
{
    return AddData((const void *)&c, 1);
}

RawDataPtr RawData::Concat(RawDataPtr other)
{
    if (AddData(other))
        return thisPtr;
    auto newRawData = NewRawDataPtr();
    if(!newRawData->AddData(thisPtr)) {
        return nullptr;
    }
    if(!newRawData->AddData(other)) {
        return nullptr;
    }
    return newRawData;
}

char *RawData::Consume(RawData::tLen len)
{
    if (len > Len || len < 0) {
        len = Len;
    }
    if (len == 0)
        return NULL;
    auto ptr = Data + Offset;
    Len -= len;
    Offset += len;
    return ptr;
}

void RawData::ReAlign()
{
    if (!movedata) return;
    if (!owner) return;
    if (Len > 0)
        memmove(Data, Data + Offset, Len);
    Offset = 0;
}
