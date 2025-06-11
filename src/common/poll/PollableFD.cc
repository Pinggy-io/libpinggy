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


#include "PollableFD.hh"

//==============================

PollableFDPtr
PollableFD::SetPollController(common::PollControllerPtr pollController)
{
    if (isRedirectWriteEventsForConnection())
        throw std::runtime_error("Non blocking connection going on. Operation not allowed.");
    common::PollStatePtr pollState;
    auto oldController = GetPController();
    SetPController(pollController);
    if (oldController) {
        if (oldController != pollController) {
            pollState = oldController->RetrieveState(thisPtr);
            if (pollController && pollState) {
                pollController->RegisterHandler(thisPtr, false);
                pollController->RestoreState(thisPtr, pollState);
            }
            oldController->DeregisterHandler(thisPtr);
        }
    }
    return thisPtr;
}

PollableFDPtr
PollableFD::RegisterFDEvenHandler(FDEventHandlerPtr fdEventHandler, pinggy::VoidPtr udata, bool edgeTrigger)
{
    Assert(GetPController());
    if (isRedirectWriteEventsForConnection())
        throw std::runtime_error("Non blocking connection going on. Operation not allowed.");
    setUdata(udata);
    setTag("");
    setRegisteredFD(thisPtr);
    registerFDEvenHandler(fdEventHandler, edgeTrigger);
    return thisPtr;
}

void PollableFD::registerFDEvenHandler(FDEventHandlerPtr fdEventHandler, bool edgeTrigger)
{
    setFDEventHandler(fdEventHandler);
    if (GetPController()) {
        GetPController()->RegisterHandler(getRegisteredFD(), edgeTrigger);
        if (getInitialReadPoll()) {
            GetPController()->RaiseReadPoll(getRegisteredFD());
            setInitialReadPoll(false);
        }
    }
    getRegisteredFD()->EventHandlerRegistered();
}

PollableFDPtr PollableFD::RegisterFDEvenHandler(FDEventHandlerPtr fdEventHandler, std::string tag, bool edgeTrigger)
{
    Assert(GetPController());
    if (isRedirectWriteEventsForConnection())
        throw std::runtime_error("Non blocking connection going on. Operation not allowed.");
    setTag(tag);
    setUdata(nullptr);
    setRegisteredFD(thisPtr);
    registerFDEvenHandler(fdEventHandler, edgeTrigger);
    return thisPtr;
}

PollableFDPtr
PollableFD::DeregisterFDEvenHandler()
{
    if (isRedirectWriteEventsForConnection())
        throw std::runtime_error("Non blocking connection going on. Operation not allowed.");
    auto registeredFd = getRegisteredFD();
    if (getFDEventHandler()) {
        if (GetPController())
            GetPController()->DeregisterHandler(registeredFd);
        setFDEventHandler(nullptr);
        setRegisteredFD(nullptr);
        registeredFd->EventHandlerDeregistered();
    }
    return thisPtr;
}


void
PollableFD::WritePollEnabled()
{
    if (isRedirectWriteEventsForConnection())
        throw std::runtime_error("Non blocking connection going on. Operation not allowed.");
    if(GetPController() && getFDEventHandler())
        GetPController()->EnableWriter(getRegisteredFD());
}

void
PollableFD::WritePollDisabled()
{
    if (isRedirectWriteEventsForConnection())
        throw std::runtime_error("Non blocking connection going on. Operation not allowed.");
    if(GetPController() && getFDEventHandler())
        GetPController()->DisableWriter(getRegisteredFD());
}

void
PollableFD::ReadPollEnabled()
{
    if (isRedirectWriteEventsForConnection())
        throw std::runtime_error("Non blocking connection going on. Operation not allowed.");
    if(GetPController() && getFDEventHandler())
        GetPController()->EnableReader(getRegisteredFD());
}

void
PollableFD::ReadPollDisabled()
{
    if (isRedirectWriteEventsForConnection())
        throw std::runtime_error("Non blocking connection going on. Operation not allowed.");
    if(GetPController() && getFDEventHandler())
        GetPController()->DisableReader(getRegisteredFD());
}

void
PollableFD::EnableReadPoll()
{
    notifyReadPollEnabled();
}

void
PollableFD::EnableWritePoll()
{
    notifyWritePollEnabled();
}

void
PollableFD::DisableReadPoll()
{
    notifyReadPollDisabled();
}

void
PollableFD::DisableWritePoll()
{
    notifyWritePollDisabled();
}

void
PollableFD::RaiseDummyReadPoll()
{
    if (isRedirectWriteEventsForConnection())
        throw std::runtime_error("Non blocking connection going on. Operation not allowed.");
    if(GetPController() && getFDEventHandler())
        GetPController()->RaiseReadPoll(getRegisteredFD());
    else
        setInitialReadPoll(true);
}

void
PollableFD::RaiseDummyWritePoll()
{
    if (isRedirectWriteEventsForConnection())
        throw std::runtime_error("Non blocking connection going on. Operation not allowed.");
    if(GetPController() && getFDEventHandler())
        GetPController()->RaiseWritePoll(getRegisteredFD());
}

void
PollableFD::RegisterConnectHandler()
{
    Assert(GetOrig() == thisPtr);
    if (redirectWriteEventsForConnection_)
        throw std::runtime_error("Non blocking connection going on. Operation not allowed.");
    if(!GetPController() || getFDEventHandler()) {
        throw std::runtime_error("You need to setup poll controller before you can initiate non-blocking connect");
    }
    GetPController()->RegisterHandler(thisPtr, false);
    GetPController()->DisableReader(thisPtr);
    GetPController()->EnableWriter(thisPtr);
    redirectWriteEventsForConnection_ = true;
}

void
PollableFD::DeregisterConnectHandler()
{
    if (!isRedirectWriteEventsForConnection())
        throw std::runtime_error("Non blocking connection is not going on. Operation not allowed.");
    GetPController()->DeregisterHandler(thisPtr);
    redirectWriteEventsForConnection_ = false;
}

len_t
PollableFD::HandlePollRecv()
{
    if (!getFDEventHandler())
        return 0;
    if (getUdata())
        return getFDEventHandler()->HandleFDReadWPtr(getRegisteredFD(), getUdata());
    if (!getTag().empty())
        return getFDEventHandler()->HandleFDReadWTag(getRegisteredFD(), getTag());
    return getFDEventHandler()->HandleFDRead(getRegisteredFD());
}

len_t
PollableFD::HandlePollSend()
{
    if (redirectWriteEventsForConnection_) {
        return HandleConnect();
    }
    if (!getFDEventHandler())
        return 0;
    if (getUdata())
        return getFDEventHandler()->HandleFDWriteWPtr(getRegisteredFD(), getUdata());
    if (!getTag().empty())
        return getFDEventHandler()->HandleFDWriteWTag(getRegisteredFD(), getTag());
    return getFDEventHandler()->HandleFDWrite(getRegisteredFD());
}

len_t
PollableFD::HandlePollError(int16_t err)
{
    if (!getFDEventHandler())
        return 0;
    if (getUdata())
        return getFDEventHandler()->HandleFDErrorWPtr(getRegisteredFD(), getUdata(), err);
    if (!getTag().empty())
        return getFDEventHandler()->HandleFDErrorWTag(getRegisteredFD(), getTag(), err);
    return getFDEventHandler()->HandleFDError(getRegisteredFD(), err);
}

