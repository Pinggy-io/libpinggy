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

#ifndef CPP_COMMON_POLLABLE_FD_HH_
#define CPP_COMMON_POLLABLE_FD_HH_

#include "PinggyPoll.hh"
#include <platform/assert_pinggy.h>

DeclareClassWithSharedPtr(PollableFD);

abstract class FDEventHandler : public virtual pinggy::SharedObject
{
    public:

    virtual len_t
    HandleFDRead(PollableFDPtr) {ABORT_WITH_MSG("Not implemented"); return 0;}

    virtual len_t
    HandleFDWrite(PollableFDPtr) {ABORT_WITH_MSG("Not implemented"); return 0;}

    virtual len_t
    HandleFDError(PollableFDPtr, int16_t) {Assert(false && "Not implemented"); return 0;}
            // this should fail during development. We do not care about the error in production.

    virtual len_t
    HandleFDReadWTag(PollableFDPtr, tString) {ABORT_WITH_MSG("Not implemented"); return 0;}

    virtual len_t
    HandleFDWriteWTag(PollableFDPtr, tString) {ABORT_WITH_MSG("Not implemented"); return 0;}

    virtual len_t
    HandleFDErrorWTag(PollableFDPtr, tString, int16_t) {ABORT_WITH_MSG("Not implemented"); return 0;};

    virtual len_t
    HandleFDReadWPtr(PollableFDPtr, pinggy::VoidPtr) {ABORT_WITH_MSG("Not implemented"); return 0;}

    virtual len_t
    HandleFDWriteWPtr(PollableFDPtr, pinggy::VoidPtr) {ABORT_WITH_MSG("Not implemented"); return 0;}

    virtual len_t
    HandleFDErrorWPtr(PollableFDPtr, pinggy::VoidPtr, int16_t) {ABORT_WITH_MSG("Not implemented"); return 0;}
};
DefineMakeSharedPtr(FDEventHandler);

#ifndef __WINDOWS_OS__
#define __CloseConn_2(x, ...) __CloseNReport(__FILE__ ":" APP_CONVERT_TO_STRING(__LINE__))
#define __CloseConn_1(x, ...) __CloseNReport(x + " --> " __FILE__ ":" APP_CONVERT_TO_STRING(__LINE__))
#define __CloseConn__(fn, _1) fn(_1, 1)
#define __CloseConn_(_1, y, x, ...) APP_EXPAND(__CloseConn__(__CloseConn##x, y))
#define CloseConn(...) APP_EXPAND(__CloseConn_(1, ##__VA_ARGS__, _1, _2))

#else

#define CloseConn(...) __CloseNReport(__FILE__ ":" APP_CONVERT_TO_STRING(__LINE__))
//This is not closed to perfect or ok. But I could not do the same as linux here.
#endif //__WINDOWS_OS__

DeclareClassWithSharedPtr(PollableFD);

class EventHandlerForPollableFd final: public virtual common::PollEventHandler
{
public:
    EventHandlerForPollableFd(PollableFDPtr root, bool pollable = true, bool redirectEventToFd = false):
            root(root),
            pollable(pollable),
            redirectEventToFd(redirectEventToFd)
                                { }

    virtual
    ~EventHandlerForPollableFd()
                                { }

    void
    SetPollController(common::PollControllerPtr );

    common::PollControllerPtr
    GetPollController()         { return pollController; }

    void
    RegisterFDEvenHandler(PollableFDPtr, FDEventHandlerPtr fdEventHandler, pinggy::VoidPtr udata = nullptr, bool edgeTrigger = false);

    void
    RegisterFDEvenHandler(PollableFDPtr, FDEventHandlerPtr fdEventHandler, tString tag, bool edgeTrigger = false);

    void
    DeregisterFDEvenHandler();

    void
    RegisterConnectHandler(PollableFDPtr);

    void
    DeregisterConnectHandler();

    void
    EnableWritePoll();

    void
    DisableWritePoll();

    void
    EnableReadPoll();

    void
    DisableReadPoll();

    void
    RaiseDummyReadPoll();

    void
    RaiseDummyWritePoll();

    len_t
    HandlePollRecv_Redirected();

    virtual len_t
    HandlePollSend_Redirected();

    virtual len_t
    HandlePollError_Redirected(int16_t);

    //PollEventHandler
    virtual sock_t
    GetFd() override;

    virtual len_t
    HandlePollRecv() override;

    virtual len_t
    HandlePollSend() override;

    virtual len_t
    HandlePollError(int16_t) override;

    virtual bool
    IsPollable() override       { return pollable; }

    virtual bool
    IsRecvReady() override;

    virtual bool
    IsSendReady() override;


private:
    void
    registerFDEvenHandler(PollableFDPtr, FDEventHandlerPtr fdEventHandler, bool edgeTrigger);

    void
    setPollable(bool val = true)       { pollable = val; }

    void
    setEventRedirect(bool val = false) { redirectEventToFd = val; }

    friend class PollableFD;

    PollableFDPtr               pollableFd;
    PollableFDWPtr              root; // we might need it
    common::PollControllerPtr   pollController;
    FDEventHandlerPtr           fdEventHandler; //making sure that there will be only one fdEventHandler
    tString                     tag;
    pinggy::VoidPtr             udata;
    bool                        initialReadPoll = false;
    bool                        redirectWriteEventsForConnection = false;
    bool                        pollable = true;
    bool                        redirectEventToFd = false;
    bool                        registeredWithPollController = false;
};
DefineMakeSharedPtr(EventHandlerForPollableFd);

abstract class PollableFD : public virtual pinggy::SharedObject {
public:
    PollableFD()                { }

    virtual
    ~PollableFD()               { }

    virtual bool
    IsRecvReady()               { return true; }

    virtual bool
    IsSendReady()               { return true; }

    virtual len_t
    HandlePollRecv_Proxy();

    virtual len_t
    HandlePollSend_Proxy();

    virtual len_t
    HandlePollError_Proxy(int16_t);

    virtual sock_t
    GetFd() = 0;

    virtual PollableFDPtr
    SetPollController(common::PollControllerPtr pollController) final;

    virtual common::PollControllerPtr
    GetPollController() final   { return GetPollEventHandler()->GetPollController(); }

    virtual PollableFDPtr
    RegisterFDEvenHandler(FDEventHandlerPtr fdEventHandler, pinggy::VoidPtr udata = nullptr, bool edgeTrigger = false) final;

    virtual PollableFDPtr
    RegisterFDEvenHandler(FDEventHandlerPtr fdEventHandler, tString tag, bool edgeTrigger = false) final;

    virtual PollableFDPtr
    DeregisterFDEvenHandler() final;

    virtual void
    SetCloseOnExec()            { if (IsValidSocket(GetFd())) set_close_on_exec(GetFd()); }

    virtual void
    UnSetCloseOnExec()          { if (IsValidSocket(GetFd())) unset_close_on_exec(GetFd()); }

    virtual void
    EnableReadPoll();

    virtual void
    EnableWritePoll();

    virtual void
    DisableReadPoll();

    virtual void
    DisableWritePoll();

    virtual void
    RaiseDummyReadPoll() final;

    virtual void
    RaiseDummyWritePoll() final;

    virtual int
    __CloseNReport(tString location) final
                                { return CloseNClear(location); }

    virtual void
    SetEventRedirect(bool val = false) final
                                { GetPollEventHandler()->setEventRedirect(val); }

    virtual EventHandlerForPollableFdPtr
    GetPollEventHandler() = 0;

    virtual void
    ErasePollEventHandler() = 0;

protected:

    virtual int
    CloseNClear(tString location) = 0;

    virtual void
    EventHandlerRegistered()    {};

    virtual void
    EventHandlerDeregistered()  {};

    virtual len_t
    HandleConnect()             { ABORT_WITH_MSG("It is not supposed to happen"); return 0; }

    virtual len_t
    HandleConnectError(tInt16 err)
                                { ABORT_WITH_MSG("It is not supposed to happen"); return 0; }

    virtual void
    RegisterConnectHandler() final;

    virtual void
    DeregisterConnectHandler() final;

private:
    friend class EventHandlerForPollableFd;
};

#endif //CPP_COMMON_POLLABLE_FD_HH_
