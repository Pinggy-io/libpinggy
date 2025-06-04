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

abstract class FDEventHandler : public virtual pinggy::SharedObject {
    public:

    virtual len_t HandleFDRead(PollableFDPtr) {ABORT_WITH_MSG("Not implemented"); return 0;}
    virtual len_t HandleFDWrite(PollableFDPtr) {ABORT_WITH_MSG("Not implemented"); return 0;}
    virtual len_t HandleFDError(PollableFDPtr, int16_t) {Assert(false && "Not implemented"); return 0;}
            // this should fail during development. We do not care about the error in production.

    virtual len_t HandleFDReadWTag(PollableFDPtr, std::string) {ABORT_WITH_MSG("Not implemented"); return 0;}
    virtual len_t HandleFDWriteWTag(PollableFDPtr, std::string) {ABORT_WITH_MSG("Not implemented"); return 0;}
    virtual len_t HandleFDErrorWTag(PollableFDPtr, std::string, int16_t) {ABORT_WITH_MSG("Not implemented"); return 0;};

    virtual len_t HandleFDReadWPtr(PollableFDPtr, pinggy::VoidPtr) {ABORT_WITH_MSG("Not implemented"); return 0;}
    virtual len_t HandleFDWriteWPtr(PollableFDPtr, pinggy::VoidPtr) {ABORT_WITH_MSG("Not implemented"); return 0;}
    virtual len_t HandleFDErrorWPtr(PollableFDPtr, pinggy::VoidPtr, int16_t) {ABORT_WITH_MSG("Not implemented"); return 0;}
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

abstract class PollableFD : public virtual common::PollEventHandler {
public:
    PollableFD(): initialReadPoll_(false), redirectWriteEventsForConnection_(false) {}
    virtual ~PollableFD() {}

    virtual len_t HandlePollRecv() override;
    virtual len_t HandlePollSend() override;
    virtual len_t HandlePollError(int16_t err) override;

    virtual PollableFDPtr SetPollController(common::PollControllerPtr pollController) final;
    virtual common::PollControllerPtr GetPollController() final { return GetPController(); }

    virtual PollableFDPtr RegisterFDEvenHandler(FDEventHandlerPtr fdEventHandler, pinggy::VoidPtr udata = nullptr, bool edgeTrigger = false) final;
    virtual PollableFDPtr RegisterFDEvenHandler(FDEventHandlerPtr fdEventHandler, std::string tag, bool edgeTrigger = false) final;
    virtual PollableFDPtr DeregisterFDEvenHandler() final;

    virtual void SetCloseOnExec() { if (IsValidSocket(GetFd())) set_close_on_exec(GetFd()); }
    virtual void UnSetCloseOnExec() { if (IsValidSocket(GetFd())) unset_close_on_exec(GetFd()); }

    virtual void EnableReadPoll();
    virtual void EnableWritePoll();
    virtual void DisableReadPoll();
    virtual void DisableWritePoll();
    virtual void RaiseDummyReadPoll() final;
    virtual void RaiseDummyWritePoll() final;
    virtual int __CloseNReport(tString location) final { return CloseNClear(location); }

    virtual PollableFDPtr GetOrig() = 0;

protected:
    virtual int CloseNClear(tString location) = 0;
    virtual void EventHandlerRegistered() {};
    virtual void EventHandlerDeregistered() {};
    virtual common::PollControllerPtr GetPController() final {auto ob = GetOrig(); Assert(ob); return ob ? ob->pollController_ : pollController_;}
    virtual void SetPController(common::PollControllerPtr ptr) final {auto ob = GetOrig(); Assert(ob); if(ob) ob->pollController_ = ptr; else pollController_ = ptr;}
    virtual len_t HandleConnect() { ABORT_WITH_MSG("It is not supposed to happen"); return 0; }

    virtual void RegisterConnectHandler() final;
    virtual void DeregisterConnectHandler() final;

    // virtual common::PollStatePtr SlientPollDeregister() final;
    // virtual void SlientPollRegister(common::PollStatePtr) final;
//protected:
private:
    virtual void registerFDEvenHandler(FDEventHandlerPtr fdEventHandler, bool edgeTrigger = false) final;

    virtual FDEventHandlerPtr getFDEventHandler() { return GetOrig()->fdEventHandler_; }
    virtual std::string getTag() { return GetOrig()->tag_; }
    virtual pinggy::VoidPtr getUdata() { return GetOrig()->udata_; }
    virtual bool getInitialReadPoll() { return GetOrig()->initialReadPoll_; }
    virtual PollableFDPtr getRegisteredFD() { return GetOrig()->registeredFD_.lock(); }
    virtual bool isRedirectWriteEventsForConnection() { return GetOrig()->redirectWriteEventsForConnection_; }

    virtual void setFDEventHandler(FDEventHandlerPtr ptr) { GetOrig()->fdEventHandler_ = ptr; }
    virtual void setTag(std::string str) { GetOrig()->tag_ = str; }
    virtual void setUdata(pinggy::VoidPtr ptr) { GetOrig()->udata_ = ptr; }
    virtual void setInitialReadPoll(bool initialReadPoll) { GetOrig()->initialReadPoll_ = initialReadPoll; }
    virtual void setRegisteredFD(PollableFDPtr registeredFD) { GetOrig()->registeredFD_ = registeredFD; }

    common::PollControllerPtr pollController_;
    FDEventHandlerPtr fdEventHandler_;
    std::string tag_;
    pinggy::VoidPtr udata_;
    bool initialReadPoll_;
    PollableFDWPtr registeredFD_;
    bool redirectWriteEventsForConnection_;
};

#endif //CPP_COMMON_POLLABLE_FD_HH_
