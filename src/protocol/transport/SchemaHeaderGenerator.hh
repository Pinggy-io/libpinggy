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

#ifndef __SRC_CPP_PROTOCOL_TRANSPORT_SCHEMAHEADERGENERATOR_HH__
#define __SRC_CPP_PROTOCOL_TRANSPORT_SCHEMAHEADERGENERATOR_HH__

#include <platform/platform.h>
#include "TransportManager.hh"

/*
So, this two files (_SchemaHeaderGenerator.hh and SchemaBodyGenerator.hh) are there
to automatically generate schema that uses our new transport layer. One can easily
use the transport without any schema, however, writing it is tidious and difficult.

The schema generator generates a suites of classes that can be used to transmit or
receive data. All the classes are defined by the schema definition. There will be a
base class (root class from the entire suite).

The schema definition is a macro function and it needs to be defined as follows

    #define <Definition Name>(f, arg)
    f(<MessageName>,
           arg,
           (<Member data type>, <Member name>[, <default value if required>[, 1 if parameter need to be an argument in constructor]])[,
           (<Member data type>, <Member name>[, <default value if required>[, 1 if parameter need to be an argument in constructor]])[,
           (<Member data type>, <Member name>[, <default value if required>[, 1 if parameter need to be an argument in constructor]])]]
     )

if a member have default value, the member would be serialized only if
it's value differ from default. Although savings are almost insignificant
it is a saving.

There is an option to make a constructor parameter optional. However, kindly be
careful there.

There are only header of interest here:

_SCHEMA_HEADER_DefineHeaders(RootClass, ClassSuffix, ClassSmallSuffix, SchemaDefinition)

The root class name would be RootClass##ClassSuffix. Rest of the classes will have
ClassSuffix as suffix in their names. ClassSmallSuffix is used in several cases. An example
is given here.

```
#define Definition(f, arg)                                                      \
    f(ClientHello,                                                              \
        arg,                                                                    \
        (tUint32,               Version,            0)                          \
    )                                                                           \

_SCHEMA_HEADER_DefineHeaders(Proto, Msg, msg, Definition)
```

*/

//==============================================================================
//==============================================================================
//     DO NO CHANGE BELOW THIS
//==============================================================================
//==============================================================================

#define _SCHEMA_HEADER_StripParen(...) __VA_ARGS__

#define _SCHEMA_HEADER_StripParenAndExpand(macro, extra, ...)                   \
    APP_EXPAND(macro(extra, __VA_ARGS__))

#define _SCHEMA_HEADER_IfDefaultDefaultArgument_0(...)                          \

#define _SCHEMA_HEADER_IfDefaultDefaultArgument_1(type, val, def, ...)          \
    type a##val=def,                                                            \


#define _SCHEMA_HEADER_IfDefaultDefaultArgument_(_1, _2, _3, _4, ...)           \
    _1(_2, _3, _4)                                                              \


#define _SCHEMA_HEADER_IfDefaultDefaultArgument(type, val, def, _, isDef, ...)  \
    _SCHEMA_HEADER_IfDefaultDefaultArgument_                                    \
        (_SCHEMA_HEADER_IfDefaultDefaultArgument_##isDef, type, val, def)       \


#define _SCHEMA_HEADER_DefineVarDefaultArgument_(_1, _2, ...)                   \
    APP_EXPAND(_SCHEMA_HEADER_IfDefaultDefaultArgument                          \
        (_1, _2, ##__VA_ARGS__, 1, 0, 0))                                       \

#define _SCHEMA_HEADER_DefineVarDefaultArgument(_1)                             \
    _SCHEMA_HEADER_DefineVarDefaultArgument_ _1                                 \

#define _SCHEMA_HEADER_DefineVariable_(x, y, ...) x y;

#define _SCHEMA_HEADER_DefineVariable(x) _SCHEMA_HEADER_DefineVariable_ x


#define _SCHEMA_HEADER_DefineProtocolClass_(ClassName, RootClass,               \
                                            ClassSuffix, ...)                   \
class ClassName##ClassSuffix:                                                   \
            virtual public RootClass##ClassSuffix                               \
{                                                                               \
public:                                                                         \
    APP_MACRO_FOR_EACH_FORNT(_SCHEMA_HEADER_DefineVariable, __VA_ARGS__)        \
    ClassName##ClassSuffix(APP_MACRO_FOR_EACH_FORNT(                            \
                        _SCHEMA_HEADER_DefineVarDefaultArgument, __VA_ARGS__)   \
                            tInt32 __UNUSED_ARG__ = 0);                         \
    virtual ~ ClassName##ClassSuffix() {}                                       \
};                                                                              \
DefineMakeSharedPtr(ClassName##ClassSuffix);                                    \


#define _SCHEMA_HEADER_DefineProtocolClass(x, vars, ...)                        \
    _SCHEMA_HEADER_StripParenAndExpand(_SCHEMA_HEADER_DefineProtocolClass_, x,  \
        _SCHEMA_HEADER_StripParen vars, __VA_ARGS__)



#define _SCHEMA_HEADER_DefineMsgType(y, x, ...) x##Type_##y,

#define SCHEMA_HEADER__DEFINE_HEADERS(RootClass, ClassSuffix,                   \
                                        ClassSmallSuffix, Definition)           \
enum t##ClassSuffix##Type {                                                     \
    ClassSuffix##Type_Invalid = 0,                                              \
    Definition(_SCHEMA_HEADER_DefineMsgType, ClassSuffix)                       \
    ClassSuffix##Type_Count                                                     \
};                                                                              \
                                                                                \
class RootClass##ClassSuffix : virtual public pinggy::SharedObject              \
{                                                                               \
public:                                                                         \
    RootClass##ClassSuffix                                                      \
        (t##ClassSuffix##Type                                                   \
            ClassSmallSuffix##Type=ClassSuffix##Type_Invalid):                  \
                ClassSmallSuffix##Type(ClassSmallSuffix##Type) {}               \
    virtual ~ RootClass##ClassSuffix () {}                                      \
    const t##ClassSuffix##Type ClassSmallSuffix##Type;                          \
};                                                                              \
DefineMakeSharedPtr(RootClass##ClassSuffix)                                     \
                                                                                \
void Inflate(DeserializerPtr, RootClass##ClassSuffix##Ptr &);                   \
void Deflate(SerializerPtr serializer, RootClass##ClassSuffix##Ptr);            \
                                                                                \
Definition(_SCHEMA_HEADER_DefineProtocolClass,(RootClass, ClassSuffix))         \

//=============================================================================

#define DECLARE_TRANSPORT_SERIALIZER_DESERIALIZER_PTR(cls, ...)                 \
    static void Deflate(SerializerPtr serializer, cls##Ptr objPtr);             \
    static void Inflate(DeserializerPtr deserializer, cls##Ptr &objPtr);        \


//=============================================================================
#define _SCHEMA_HEADER_DefineSendingFunction_(ClassName, RootClass,             \
                                            Suffix, ...)                        \
        virtual void Send##ClassName(ClassName##Suffix##Ptr);                   \

#define _SCHEMA_HEADER_DefineSendingFunction(x, vars, ...)                      \
    _SCHEMA_HEADER_StripParenAndExpand(_SCHEMA_HEADER_DefineSendingFunction_,   \
        x, _SCHEMA_HEADER_StripParen vars, __VA_ARGS__)

#define _SCHEMA_HEADER_EvHandlerHandlingFunction_(ClassName, HandlingClassName, \
    RootClass, Suffix, ...)                                                     \
        virtual void Handle##ClassName##Suffix(HandlingClassName##Ptr,          \
            ClassName##Suffix##Ptr) = 0;                                        \

#define _SCHEMA_HEADER_EvHandlerHandlingFunction(x, vars, ...)                  \
    _SCHEMA_HEADER_StripParenAndExpand(                                         \
        _SCHEMA_HEADER_EvHandlerHandlingFunction_, x,                           \
        _SCHEMA_HEADER_StripParen vars, __VA_ARGS__)

#define _SCHEMA_HEADER_EmptyEvHandlerHandlingFunction_(ClassName,               \
    HandlingClassName, RootClass, Suffix, ...)                                  \
        virtual void Handle##ClassName##Suffix(HandlingClassName##Ptr,          \
            ClassName##Suffix##Ptr) {}                                          \

#define _SCHEMA_HEADER_EmptyEvHandlerHandlingFunction(x, vars, ...)             \
    _SCHEMA_HEADER_StripParenAndExpand(                                         \
        _SCHEMA_HEADER_EmptyEvHandlerHandlingFunction_, x,                      \
        _SCHEMA_HEADER_StripParen vars, __VA_ARGS__)

#define DECLARE_HANDLING_CLASS(HandlingClassName, RootClass, ClassSuffix,       \
                                        ClassSmallSuffix, Definition)           \
DeclareClassWithSharedPtr(HandlingClassName);                                   \
abstract class HandlingClassName##EventHandler :                                \
        virtual public pinggy::SharedObject                                     \
{                                                                               \
public:                                                                         \
    virtual ~ HandlingClassName##EventHandler() {}                              \
    virtual void Handle##RootClass##ConnectionReset(HandlingClassName##Ptr) = 0;\
    virtual void Handle##RootClass##Stopped(HandlingClassName##Ptr) = 0;        \
    Definition(_SCHEMA_HEADER_EvHandlerHandlingFunction,                        \
        (HandlingClassName, RootClass, ClassSuffix))                            \
};                                                                              \
DeclareSharedPtr(HandlingClassName##EventHandler);                              \
abstract class HandlingClassName##EmptyEventHandler :                           \
        public HandlingClassName##EventHandler                                  \
{                                                                               \
public:                                                                         \
    virtual ~ HandlingClassName##EmptyEventHandler() {}                         \
    virtual void Handle##RootClass##ConnectionReset(HandlingClassName##Ptr) {}  \
    virtual void Handle##RootClass##Stopped(HandlingClassName##Ptr) {}          \
    Definition(_SCHEMA_HEADER_EmptyEvHandlerHandlingFunction,                   \
        (HandlingClassName, RootClass, ClassSuffix,))                           \
};                                                                              \
DeclareSharedPtr(HandlingClassName##EmptyEventHandler);                         \
class HandlingClassName : public TransportManagerEventHandler                   \
{                                                                               \
public:                                                                         \
    HandlingClassName(net::NetworkConnectionPtr netConn,                        \
        HandlingClassName##EventHandler##Ptr handlerPtr);                       \
    virtual ~ HandlingClassName();                                              \
    virtual bool Start(bool handshakeRequired=true);                            \
    virtual bool Stop();                                                        \
    virtual bool IsConnected() { return running; }                              \
    virtual bool Send##ClassSuffix(RootClass##ClassSuffix##Ptr                  \
        ClassSmallSuffix, bool queue = true);                                   \
    virtual void HandleConnectionReset(net::NetworkConnectionPtr netConn);      \
    virtual void HandleIncomingDeserialize(DeserializerPtr deserializer);       \
    virtual void HandleReadyToSendBuffer();                                     \
    virtual void HandleIncompleteHandshake();                                   \
    virtual net::NetworkConnectionPtr GetNetConn() { return netConn; }          \
    virtual pinggy::VoidPtr GetPtr() { return ptr; }                            \
private:                                                                        \
    net::NetworkConnectionPtr netConn;                                          \
    TransportManagerPtr transportManager;                                       \
    std::queue<RootClass##ClassSuffix##Ptr> sendQueue;                          \
    HandlingClassName##EventHandler##Ptr eventHandler;                          \
    pinggy::VoidPtr ptr;                                                        \
    bool running;                                                               \
};                                                                              \
DefineMakeSharedPtr(HandlingClassName)

#endif // __SRC_CPP_PROTOCOL_TRANSPORT_SCHEMAHEADERGENERATOR_HH__
