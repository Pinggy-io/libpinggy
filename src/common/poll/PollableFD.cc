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
#include <utils/TemplateStreaming.hh> //this needs to be the last include

//==============================

len_t
PollableFD::HandlePollRecv_Proxy()
{
    auto pollEventHandler = GetPollEventHandler();
    return pollEventHandler->HandlePollRecv_Redirected();
}

len_t
PollableFD::HandlePollSend_Proxy()
{
    auto pollEventHandler = GetPollEventHandler();
    return pollEventHandler->HandlePollSend_Redirected();
}

len_t
PollableFD::HandlePollError_Proxy(int16_t err)
{
    auto pollEventHandler = GetPollEventHandler();
    return pollEventHandler->HandlePollError_Redirected(err);
}

PollableFDPtr
PollableFD::SetPollController(common::PollControllerPtr pollController)
{
    auto pollEventHandler = GetPollEventHandler();
    pollEventHandler->SetPollController(pollController);
    return thisPtr;
}

PollableFDPtr
PollableFD::RegisterFDEvenHandler(FDEventHandlerPtr fdEventHandler, pinggy::VoidPtr udata, bool edgeTrigger)
{
    auto pollEventHandler = GetPollEventHandler();
    pollEventHandler->RegisterFDEvenHandler(thisPtr, fdEventHandler, udata, edgeTrigger);
    return thisPtr;
}

PollableFDPtr PollableFD::RegisterFDEvenHandler(FDEventHandlerPtr fdEventHandler, std::string tag, bool edgeTrigger)
{
    auto pollEventHandler = GetPollEventHandler();
    pollEventHandler->RegisterFDEvenHandler(thisPtr, fdEventHandler, tag, edgeTrigger);
    return thisPtr;
}

PollableFDPtr
PollableFD::DeregisterFDEvenHandler()
{
    auto pollEventHandler = GetPollEventHandler();
    pollEventHandler->DeregisterFDEvenHandler();
    return thisPtr;
}

void
PollableFD::EnableReadPoll()
{
    auto pollEventHandler = GetPollEventHandler();
    pollEventHandler->EnableReadPoll();
}

void
PollableFD::EnableWritePoll()
{
    auto pollEventHandler = GetPollEventHandler();
    pollEventHandler->EnableWritePoll();
}

void
PollableFD::DisableReadPoll()
{
    auto pollEventHandler = GetPollEventHandler();
    pollEventHandler->DisableReadPoll();
}

void
PollableFD::DisableWritePoll()
{
    auto pollEventHandler = GetPollEventHandler();
    pollEventHandler->DisableWritePoll();
}

void
PollableFD::RaiseDummyReadPoll()
{
    auto pollEventHandler = GetPollEventHandler();
    pollEventHandler->RaiseDummyReadPoll();
}

void
PollableFD::RaiseDummyWritePoll()
{
    auto pollEventHandler = GetPollEventHandler();
    pollEventHandler->RaiseDummyWritePoll();
}

void
PollableFD::RegisterConnectHandler()
{
    auto pollEventHandler = GetPollEventHandler();
    pollEventHandler->RegisterConnectHandler(thisPtr);
}

void
PollableFD::DeregisterConnectHandler()
{
    auto pollEventHandler = GetPollEventHandler();
    pollEventHandler->DeregisterConnectHandler();
}

//===============

void
EventHandlerForPollableFd::SetPollController(common::PollControllerPtr newPollController)
{
    if (redirectWriteEventsForConnection)
        throw std::runtime_error("Non blocking connection going on. Operation not allowed.");
    common::PollStatePtr pollState;
    auto oldController = pollController;
    pollController = newPollController;
    if (oldController && pollController && registeredWithPollController) {
        if (oldController != pollController) {
            pollState = oldController->RetrieveState(thisPtr);
            if (pollState)
                oldController->DeregisterHandler(thisPtr);
            if (pollState) { //We can get pollState only when handler is registered
                pollController->RegisterHandler(thisPtr, false);
                pollController->RestoreState(thisPtr, pollState);
            }
        }
    }
}

void
EventHandlerForPollableFd::RegisterFDEvenHandler(PollableFDPtr pollableFd, FDEventHandlerPtr fdEventHandler, pinggy::VoidPtr udata, bool edgeTrigger)
{
    Assert(pollController);
    if (redirectWriteEventsForConnection)
        throw std::runtime_error("Non blocking connection going on. Operation not allowed.");
    this->udata = udata;
    this->tag = "";
    registerFDEvenHandler(pollableFd, fdEventHandler, edgeTrigger);
}

void
EventHandlerForPollableFd::RegisterFDEvenHandler(PollableFDPtr pollableFd, FDEventHandlerPtr fdEventHandler, tString tag, bool edgeTrigger)
{
    Assert(pollController);
    if (redirectWriteEventsForConnection)
        throw std::runtime_error("Non blocking connection going on. Operation not allowed.");
    this->tag = tag;
    udata = nullptr;
    registerFDEvenHandler(pollableFd, fdEventHandler, edgeTrigger);
}

void
EventHandlerForPollableFd::DeregisterFDEvenHandler()
{
    if (redirectWriteEventsForConnection)
        throw std::runtime_error("Non blocking connection going on. Operation not allowed.");

    if (registeredWithPollController) {
        auto pollableFdLocal = pollableFd;
        if (pollableFdLocal) {
            if (pollController) {
                pollController->DeregisterHandler(thisPtr);
                pollController->RetainObject(fdEventHandler, pollableFd);
            }
            fdEventHandler = nullptr; //To remove the reference loop
            pollableFdLocal->EventHandlerDeregistered();
            pollableFd = nullptr; //To remove the reference loop
        }
    }
    registeredWithPollController = false;
}

void
EventHandlerForPollableFd::RegisterConnectHandler(PollableFDPtr pollableFd)
{
    Assert(pollableFd);
    if (redirectWriteEventsForConnection)
        throw std::runtime_error("Non-blocking connect going on. Operation not allowed.");
    if(!pollController || fdEventHandler) {
        throw std::runtime_error("You need to setup poll controller before you can initiate non-blocking connect");
    }
    if (this->pollableFd) {
        throw std::runtime_error("Already registered");
    }
    this->pollableFd = pollableFd;
    redirectWriteEventsForConnection = true;
    pollController->RegisterHandler(thisPtr, false);
    pollController->DisableReader(thisPtr);
    pollController->EnableWriter(thisPtr);
}

void
EventHandlerForPollableFd::DeregisterConnectHandler()
{
    if (!redirectWriteEventsForConnection)
        throw std::runtime_error("Non blocking connection is not going on. Operation not allowed.");
    if (!pollableFd) {
        throw std::runtime_error("Not registered");
    }
    pollController->DeregisterHandler(thisPtr);
    pollController->RetainObject(pollableFd);
    redirectWriteEventsForConnection = false;
    pollableFd = nullptr;
}

void
EventHandlerForPollableFd::EnableWritePoll()
{
    if (redirectWriteEventsForConnection)
        throw std::runtime_error("Non-blocking connect going on. Operation not allowed.");
    if (!registeredWithPollController) {
        throw std::runtime_error("Not registered");
    }
    if (pollController && fdEventHandler)
        pollController->EnableWriter(thisPtr);
}

void
EventHandlerForPollableFd::DisableWritePoll()
{
    if (redirectWriteEventsForConnection)
        throw std::runtime_error("Non-blocking connect going on. Operation not allowed.");
    if (!registeredWithPollController) {
        throw std::runtime_error("Not registered");
    }
    if (pollController && fdEventHandler)
        pollController->DisableWriter(thisPtr);
}

void
EventHandlerForPollableFd::EnableReadPoll()
{
    if (redirectWriteEventsForConnection)
        throw std::runtime_error("Non-blocking connect going on. Operation not allowed.");
    if (!registeredWithPollController) {
        throw std::runtime_error("Not registered");
    }
    if (pollController && fdEventHandler)
        pollController->EnableReader(thisPtr);
}

void
EventHandlerForPollableFd::DisableReadPoll()
{
    if (redirectWriteEventsForConnection)
        throw std::runtime_error("Non-blocking connect going on. Operation not allowed.");
    if (!registeredWithPollController) {
        throw std::runtime_error("Not registered");
    }
    if (pollController && fdEventHandler)
        pollController->DisableReader(thisPtr);
}

void
EventHandlerForPollableFd::RaiseDummyReadPoll()
{
    if (redirectWriteEventsForConnection)
        throw std::runtime_error("Non-blocking connect going on. Operation not allowed.");
    if(pollController && fdEventHandler && registeredWithPollController)
        pollController->RaiseReadPoll(thisPtr);
    else
        initialReadPoll = true;
}

void
EventHandlerForPollableFd::RaiseDummyWritePoll()
{
    if (redirectWriteEventsForConnection)
        throw std::runtime_error("Non-blocking connect going on. Operation not allowed.");
    if(pollController && fdEventHandler && registeredWithPollController)
        pollController->RaiseWritePoll(thisPtr);
}

len_t
EventHandlerForPollableFd::HandlePollRecv_Redirected()
{
    if (!fdEventHandler)
        return 0;
    auto pollableFdLocal = pollableFd;
    if (udata)
        return fdEventHandler->HandleFDReadWPtr(pollableFdLocal, udata);
    if (!tag.empty())
        return fdEventHandler->HandleFDReadWTag(pollableFdLocal, tag);
    return fdEventHandler->HandleFDRead(pollableFdLocal);
}

len_t
EventHandlerForPollableFd::HandlePollSend_Redirected()
{
    if (!fdEventHandler)
        return 0;
    auto pollableFdLocal = pollableFd;
    if (udata)
        return fdEventHandler->HandleFDWriteWPtr(pollableFdLocal, udata);
    if (!tag.empty())
        return fdEventHandler->HandleFDWriteWTag(pollableFdLocal, tag);
    return fdEventHandler->HandleFDWrite(pollableFdLocal);
}

len_t
EventHandlerForPollableFd::HandlePollError_Redirected(int16_t err)
{
    if (!fdEventHandler)
        return 0;
    auto pollableFdLocal = pollableFd;
    if (udata)
        return fdEventHandler->HandleFDErrorWPtr(pollableFdLocal, udata, err);
    if (!tag.empty())
        return fdEventHandler->HandleFDErrorWTag(pollableFdLocal, tag, err);
    return fdEventHandler->HandleFDError(pollableFdLocal, err);
}

sock_t
EventHandlerForPollableFd::GetFd()
{
    Assert(registeredWithPollController || redirectWriteEventsForConnection);
    if (!registeredWithPollController && !redirectWriteEventsForConnection) return InValidSocket;
    auto pollableFdLocal = pollableFd;
    return pollableFdLocal->GetFd();
}

len_t
EventHandlerForPollableFd::HandlePollRecv()
{
    if (!registeredWithPollController)
        return 0;
    if (redirectEventToFd)
        return root.lock()->HandlePollRecv_Proxy();
    return HandlePollRecv_Redirected();
}

len_t
EventHandlerForPollableFd::HandlePollSend()
{
    auto lPollableFd = pollableFd; //So that pollableFd stays alive till the functionend
    LOGT("Poll Send called: ", lPollableFd->GetFd(), redirectWriteEventsForConnection);
    if (redirectWriteEventsForConnection) {
        return lPollableFd->HandleConnect();
    }
    if (!registeredWithPollController)
        return 0;
    if (redirectEventToFd)
        return root.lock()->HandlePollSend_Proxy();
    return HandlePollSend_Redirected();
}

len_t
EventHandlerForPollableFd::HandlePollError(int16_t err)
{
    if (redirectWriteEventsForConnection) {
        auto pollableFdLocal = pollableFd;
        if (!pollableFdLocal) return 0;
        return pollableFdLocal->HandleConnectError(err);
    }
    if (!registeredWithPollController) // there is a very good chance that this error is already handled via PollSend or PollRecv
        return 0;
    if (redirectEventToFd)
        return root.lock()->HandlePollError_Proxy(err);
    return HandlePollError_Redirected(err);
}

bool
EventHandlerForPollableFd::IsRecvReady()
{
    auto pollableFdLocal = pollableFd;
    return pollable ? true : (pollableFdLocal ? pollableFdLocal->IsRecvReady() : false);
}

bool
EventHandlerForPollableFd::IsSendReady()
{
    auto pollableFdLocal = pollableFd;
    return pollable ? true : (pollableFdLocal ? pollableFdLocal->IsSendReady() : false);
}

void EventHandlerForPollableFd::registerFDEvenHandler(PollableFDPtr pollableFd, FDEventHandlerPtr fdEventHandler, bool edgeTrigger)
{
    this->pollableFd = pollableFd;
    this->fdEventHandler = fdEventHandler;
    registeredWithPollController = true;
    pollController->RegisterHandler(thisPtr, edgeTrigger);
    if (initialReadPoll) {
        pollController->RaiseReadPoll(thisPtr);
        initialReadPoll = false;
    }
    pollableFd->EventHandlerRegistered();
}

INCLUDE_MEMORY_DUMP_DEFINITION
