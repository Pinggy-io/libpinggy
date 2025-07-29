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

#include "SslNetConnBio.hh"

namespace net
{

#define MAX_CONNECT_STR_LEN 256

struct NetConnContainerBioData {
    NetConnContainerBioData(): addr(""){}
    NetworkConnectionPtr        netConn;
    char                        addr[MAX_CONNECT_STR_LEN];
    tString                     origFunc;
};

static int
netConnBioCreate(BIO *bio)
{
    auto myBioData = new NetConnContainerBioData();
    if (!myBioData) {
        LOGE("Could not create custom bio data");
        return 0;
    }
    BIO_set_data(bio, myBioData); // Attach the custom context to the BIO
    BIO_set_init(bio, 1);
    BIO_set_flags(bio, 0);
    return 1;
}

static void
netConnCloseConn(void *myBioData_)
{
    auto myBioData = (NetConnContainerBioData *)myBioData_;
    if (myBioData && myBioData->netConn) {
        myBioData->netConn->DeregisterFDEvenHandler();
        myBioData->netConn->CloseConn();
        myBioData->netConn = nullptr;
    }
}

static int
netConnBioDestroy(BIO *bio)
{
    if (!bio) return 0; // Null check

    auto myBioData = (NetConnContainerBioData *)BIO_get_data(bio);

    if (BIO_get_shutdown(bio)) {
        if (BIO_get_init(bio)) {
            netConnCloseConn(myBioData);
        }
        BIO_set_init(bio, 0);
        BIO_set_flags(bio, 0);
    }

    if (myBioData) {
        // Free any custom resources here if necessary
        delete myBioData;
        myBioData = NULL;
        BIO_set_data(bio, NULL); // Clear the context
    }

    LOGD("FREEING up bio");
    return 1; // Success
}

static int
netConnBioRead(BIO *bio, char *buf, int len)
{
    if (buf == NULL || len == 0)
        return 0;
    auto myBioData = (NetConnContainerBioData *)BIO_get_data(bio);
    Assert(myBioData);
    Assert(myBioData->netConn);

    app_set_errno(0);

    int result = myBioData->netConn->Read(buf, len);
    BIO_clear_retry_flags(bio);

    if (result <= 0) {
        LOGT("Issue with reading (", myBioData->netConn->GetType() , ")(", myBioData->netConn->GetFd() , "):", result, app_get_errno(), app_get_strerror(app_get_errno()), myBioData->origFunc);
        if (myBioData->netConn->TryAgain()) {
            BIO_set_retry_read(bio);
        } else {
            BIO_set_flags(bio, BIO_get_flags(bio) | BIO_FLAGS_IN_EOF);
            LOGT("Issue with reading (", myBioData->netConn->GetType() , ")(", myBioData->netConn->GetFd() , "):", result, app_get_errno(), app_get_strerror(app_get_errno()), myBioData->origFunc);
        }
    }
    return result;                          // Return bytes read or an error
}

static int
netConnBioWrite(BIO *bio, const char *buf, int len)
{
    auto myBioData = (NetConnContainerBioData *)BIO_get_data(bio);
    Assert(myBioData);
    Assert(myBioData->netConn);

    app_set_errno(0);

    int result = myBioData->netConn->Write(buf, len);
    BIO_clear_retry_flags(bio);

    if (result <= 0) {
        if (myBioData->netConn->TryAgain()) {
            BIO_set_retry_write(bio);
        }
        LOGD("Issue with writing (", myBioData->netConn->GetType() , ")(", myBioData->netConn->GetFd() , "): ", app_get_errno(), app_get_strerror(app_get_errno()));
    }

    return result;                          // Return bytes written or an error
}

static long
netConnBioCtrl(BIO *bio, int cmd, long num, void *ptr)
{
    // LOGE("CTRL: ", cmd);
    switch (cmd) {
        case BIO_C_SET_FD:
            if(BIO_get_shutdown(bio)) {
                if (BIO_get_init(bio)) {
                    netConnCloseConn(BIO_get_data(bio));
                }
                BIO_set_flags(bio, 0);
            }
            BIO_set_shutdown(bio, (int)num);
            BIO_set_init(bio, 1);
            return 1;

        case BIO_C_GET_FD:
            return -1;

        case BIO_CTRL_GET_CLOSE:
            return BIO_get_shutdown(bio);
        case BIO_CTRL_SET_CLOSE:
            BIO_set_shutdown(bio, (int)num);
            return 1;
        case BIO_CTRL_DUP:
            return 0;
        case BIO_CTRL_FLUSH:
            return 1; // Flush is a no-op for many BIOs
        case BIO_CTRL_EOF:
            return (BIO_get_flags(bio) & BIO_FLAGS_IN_EOF) != 0;
        case BIO_C_GET_CONNECT:
            if (ptr != NULL && num == 2) {
                auto data = (NetConnContainerBioData *)BIO_get_data(bio);
                if (!data)
                    return 0;
                *(const char **)ptr = data->addr;
                return 1;
            } else {
                return 0;
            }
        case BIO_C_SET_CONNECT:
            if (ptr != NULL && num == 2) {
                auto data = (NetConnContainerBioData *)BIO_get_data(bio);
                if (!data)
                    return 0;
                const char *input = static_cast<const char *>(ptr);
                if (!input) return 0;

                strncpy(data->addr, input, MAX_CONNECT_STR_LEN - 1);
                data->addr[MAX_CONNECT_STR_LEN - 1] = '\0';  // ensure null-termination
                return 1;
            } else {
                return 0;
            }
        default:
            return 0; // Handle other control commands as needed
    }
}


static int
netConnBioPuts(BIO *bio, const char *str)
{
    int n, ret;

    n = strlen(str);
    ret = netConnBioWrite(bio, str, n);
    return ret;
}

BIO *
netConnBioNewBio(NetworkConnectionPtr netConn)
{
    BIO_METHOD *bio_method = BIO_meth_new(BIO_TYPE_SOURCE_SINK, "custom accept bio");

    BIO_meth_set_create(bio_method, netConnBioCreate);
    BIO_meth_set_destroy(bio_method, netConnBioDestroy);
    BIO_meth_set_write(bio_method, netConnBioWrite);
    BIO_meth_set_read(bio_method, netConnBioRead);
    BIO_meth_set_ctrl(bio_method, netConnBioCtrl);
    BIO_meth_set_puts(bio_method, netConnBioPuts);

    BIO *bio = BIO_new(bio_method);
    if (bio) {
        auto myBioData = (NetConnContainerBioData *)BIO_get_data(bio);
        Assert(myBioData);
        Assert(myBioData->netConn == nullptr);

        myBioData->netConn = netConn;
    }

    return bio;
}

void
setNetConnOrigFunc(BIO *bio, tString func)
{
    if (!bio)
        return;

    auto myBioData = (NetConnContainerBioData *)BIO_get_data(bio);
    if (myBioData) {
        myBioData->origFunc = func;
    }
}

} // namespace net
