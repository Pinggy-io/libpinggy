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


#ifndef APP_FOREACH_MACROS_H_
#define APP_FOREACH_MACROS_H_


#ifdef __cplusplus
extern "C" {
#endif


#define APP_EXPAND(_x) _x
#define APP_MACRO_PASTE(_x,_y) _x##_y

#ifndef __WINDOWS_OS__
/*
  Reduced no of argument from 127 to 60 because windows does not support
  more than 127 arguments in macro
*/
#define PP_ARG_N( \
		 _1,  _2,  _3,  _4,  _5,  _6,  _7,  _8,  _9, _10, \
		_11, _12, _13, _14, _15, _16, _17, _18, _19, _20, \
		_21, _22, _23, _24, _25, _26, _27, _28, _29, _30, \
		_31, _32, _33, _34, _35, _36, _37, _38, _39, _40, \
		_41, _42, _43, _44, _45, _46, _47, _48, _49, _50, \
		_51, _52, _53, _54, _55, _56, _57, _58, _59, _60, \
		N, ...) N
/* Note 60 is removed */
#define PP_RSEQ_N()                                        \
		59, 58, 57, 56, 55, 54, 53, 52, 51, 50, \
		49, 48, 47, 46, 45, 44, 43, 42, 41, 40, \
		39, 38, 37, 36, 35, 34, 33, 32, 31, 30, \
		29, 28, 27, 26, 25, 24, 23, 22, 21, 20, \
		19, 18, 17, 16, 15, 14, 13, 12, 11, 10, \
		9, 8, 7, 6, 5, 4, 3, 2, 1, 0


#define PP_NARG_(...)    PP_ARG_N(__VA_ARGS__)

/* Note dummy first argument _ and ##__VA_ARGS__ instead of __VA_ARGS__ */
#define PP_NARG(...)     PP_NARG_(_, ##__VA_ARGS__, PP_RSEQ_N())


//Macros to iterate over elipses
#define APP_MACRO_FOR_EACH_0(...) YOU_CAN_NOT_HAVE_0_ARGUMENTS
#define APP_MACRO_FOR_EACH_1(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fnl1(_x) _fnl2(_x)
#define APP_MACRO_FOR_EACH_2(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_1(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_3(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_2(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_4(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_3(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_5(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_4(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_6(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_5(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_7(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_6(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_8(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_7(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_9(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_8(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_10(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_9(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_11(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_10(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_12(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_11(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_13(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_12(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_14(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_13(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_15(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_14(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_16(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_15(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_17(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_16(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_18(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_17(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_19(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_18(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_20(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_19(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_21(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_20(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_22(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_21(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_23(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_22(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_24(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_23(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_25(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_24(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_26(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_25(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_27(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_26(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_28(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_27(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_29(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_28(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_30(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_29(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_31(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_30(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_32(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_31(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_33(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_32(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_34(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_33(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_35(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_34(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_36(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_35(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_37(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_36(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_38(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_37(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_39(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_38(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_40(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_39(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_41(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_40(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_42(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_41(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_43(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_42(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_44(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_43(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_45(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_44(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_46(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_45(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_47(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_46(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_48(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_47(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_49(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_48(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_50(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_49(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_51(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_50(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_52(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_51(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_53(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_52(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_54(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_53(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_55(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_54(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_56(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_55(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_57(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_56(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_58(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_57(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_59(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_58(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)
#define APP_MACRO_FOR_EACH_60(_fn1, _fnl1, _fn2, _fnl2, _x, ...) _fn1(_x) APP_MACRO_FOR_EACH_59(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__) _fn2(_x)

#define APP_MACRO_DUMMY(_x)
#define APP_MACRO_FOR_EACH_(_fn1, _fnl1, _fn2, _fnl2, N, ...) APP_MACRO_FOR_EACH_##N(_fn1, _fnl1, _fn2, _fnl2, __VA_ARGS__)
#define APP_MACRO_FOR_EACH_N(...) APP_MACRO_FOR_EACH_(__VA_ARGS__)
#define APP_MACRO_FOR_EACH(_fn1, _fn2, ...) APP_MACRO_FOR_EACH_N(_fn1, _fn1, _fn2, _fn2, PP_NARG(__VA_ARGS__), __VA_ARGS__)
#define APP_MACRO_FOR_EACH_FORNT(_fn, ...) APP_MACRO_FOR_EACH(_fn, APP_MACRO_DUMMY, __VA_ARGS__)
#define APP_MACRO_FOR_EACH_BACK(_fn, ...)  APP_MACRO_FOR_EACH(APP_MACRO_DUMMY, _fn1, __VA_ARGS__)
#define APP_MACRO_FOR_EACH_LASTFUNC(_fn1, _fnl1, _fn2, _fnl2, ...) APP_MACRO_FOR_EACH_N(_fn1, _fnl1, _fn2, _fnl2, PP_NARG(__VA_ARGS__), __VA_ARGS__)

//============================================

//Macros to iterate over elipses
#define APP_MACRO_FOR_EACH_WITH_ARG_0(...) YOU_CAN_NOT_HAVE_0_ARGUMENTS
#define APP_MACRO_FOR_EACH_WITH_ARG_1( _fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fnl1(_arg1, _x) _fnl2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_2( _fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_1( _fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_3( _fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_2( _fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_4( _fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_3( _fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_5( _fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_4( _fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_6( _fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_5( _fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_7( _fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_6( _fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_8( _fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_7( _fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_9( _fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_8( _fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_10(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_9( _fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_11(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_10(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_12(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_11(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_13(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_12(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_14(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_13(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_15(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_14(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_16(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_15(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_17(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_16(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_18(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_17(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_19(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_18(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_20(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_19(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_21(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_20(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_22(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_21(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_23(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_22(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_24(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_23(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_25(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_24(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_26(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_25(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_27(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_26(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_28(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_27(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_29(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_28(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_30(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_29(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_31(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_30(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_32(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_31(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_33(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_32(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_34(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_33(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_35(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_34(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_36(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_35(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_37(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_36(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_38(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_37(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_39(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_38(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_40(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_39(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_41(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_40(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_42(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_41(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_43(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_42(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_44(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_43(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_45(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_44(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_46(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_45(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_47(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_46(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_48(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_47(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_49(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_48(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_50(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_49(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_51(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_50(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_52(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_51(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_53(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_52(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_54(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_53(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_55(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_54(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_56(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_55(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_57(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_56(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_58(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_57(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_59(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_58(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG_60(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, _x, ...) _fn1(_arg1, _x) APP_MACRO_FOR_EACH_WITH_ARG_59(_fn1, _fnl1, _fn2, _fnl2, _arg1, _arg2, __VA_ARGS__) _fn2(_arg2, _x)

#define APP_MACRO_WITH_ARG_DUMMY(_x, _y)
#define APP_MACRO_FOR_EACH_WITH_ARG_(_fn1, _fnl1, _fn2, _fnl2, _a1, _a2, N, ...) APP_MACRO_FOR_EACH_WITH_ARG_##N(_fn1, _fnl1, _fn2, _fnl2, _a1, _a2, __VA_ARGS__)
#define APP_MACRO_FOR_EACH_WITH_ARG_N(...) APP_MACRO_FOR_EACH_WITH_ARG_(__VA_ARGS__)
#define APP_MACRO_FOR_EACH_WITH_ARG(_fn1, _fn2, _a1, _a2, ...) APP_MACRO_FOR_EACH_WITH_ARG_N(_fn1, _fn1, _fn2, _fn2, _a1, _a2, PP_NARG(__VA_ARGS__), __VA_ARGS__)
#define APP_MACRO_FOR_EACH_WITH_ARG_FORNT(_fn, _a, ...) APP_MACRO_FOR_EACH_WITH_ARG(_fn, APP_MACRO_WITH_ARG_DUMMY, _a, _a, __VA_ARGS__)
#define APP_MACRO_FOR_EACH_WITH_ARG_BACK(_fn, _a, ...)  APP_MACRO_FOR_EACH_WITH_ARG(APP_MACRO_WITH_ARG_DUMMY, _fn1, _a, _a, __VA_ARGS__)
#define APP_MACRO_FOR_EACH_WITH_ARG_LASTFUNC(_fn1, _fnl1, _fn2, _fnl2, _a1, _a2, ...) APP_MACRO_FOR_EACH_WITH_ARG_N(_fn1, _fnl1, _fn2, _fnl2, _a1, _a2, PP_NARG(__VA_ARGS__), __VA_ARGS__)


#else

#define APP_FOR_EACH_1(startFunc, endFunc, singleStartFunc, singleEndFunc, x) singleStartFunc(x) singleEndFunc(x)
#define APP_FOR_EACH_2(startFunc, endFunc, singleStartFunc, singleEndFunc, x, ...) startFunc(x) APP_EXPAND(APP_FOR_EACH_1(startFunc, endFunc, singleStartFunc, singleEndFunc, __VA_ARGS__)) endFunc(x)
#define APP_FOR_EACH_3(startFunc, endFunc, singleStartFunc, singleEndFunc, x, ...) startFunc(x) APP_EXPAND(APP_FOR_EACH_2(startFunc, endFunc, singleStartFunc, singleEndFunc, __VA_ARGS__)) endFunc(x)
#define APP_FOR_EACH_4(startFunc, endFunc, singleStartFunc, singleEndFunc, x, ...) startFunc(x) APP_EXPAND(APP_FOR_EACH_3(startFunc, endFunc, singleStartFunc, singleEndFunc, __VA_ARGS__)) endFunc(x)
#define APP_FOR_EACH_5(startFunc, endFunc, singleStartFunc, singleEndFunc, x, ...) startFunc(x) APP_EXPAND(APP_FOR_EACH_4(startFunc, endFunc, singleStartFunc, singleEndFunc, __VA_ARGS__)) endFunc(x)
#define APP_FOR_EACH_6(startFunc, endFunc, singleStartFunc, singleEndFunc, x, ...) startFunc(x) APP_EXPAND(APP_FOR_EACH_5(startFunc, endFunc, singleStartFunc, singleEndFunc, __VA_ARGS__)) endFunc(x)
#define APP_FOR_EACH_7(startFunc, endFunc, singleStartFunc, singleEndFunc, x, ...) startFunc(x) APP_EXPAND(APP_FOR_EACH_6(startFunc, endFunc, singleStartFunc, singleEndFunc, __VA_ARGS__)) endFunc(x)
#define APP_FOR_EACH_8(startFunc, endFunc, singleStartFunc, singleEndFunc, x, ...) startFunc(x) APP_EXPAND(APP_FOR_EACH_7(startFunc, endFunc, singleStartFunc, singleEndFunc, __VA_ARGS__)) endFunc(x)
#define APP_FOR_EACH_9(startFunc, endFunc, singleStartFunc, singleEndFunc, x, ...) startFunc(x) APP_EXPAND(APP_FOR_EACH_8(startFunc, endFunc, singleStartFunc, singleEndFunc, __VA_ARGS__)) endFunc(x)
#define APP_FOR_EACH_10(startFunc, endFunc, singleStartFunc, singleEndFunc, x, ...) startFunc(x) APP_EXPAND(APP_FOR_EACH_9(startFunc, endFunc, singleStartFunc, singleEndFunc, __VA_ARGS__)) endFunc(x)
#define APP_FOR_EACH_11(startFunc, endFunc, singleStartFunc, singleEndFunc, x, ...) startFunc(x) APP_EXPAND(APP_FOR_EACH_10(startFunc, endFunc, singleStartFunc, singleEndFunc, __VA_ARGS__)) endFunc(x)
#define APP_FOR_EACH_12(startFunc, endFunc, singleStartFunc, singleEndFunc, x, ...) startFunc(x) APP_EXPAND(APP_FOR_EACH_11(startFunc, endFunc, singleStartFunc, singleEndFunc, __VA_ARGS__)) endFunc(x)
#define APP_FOR_EACH_13(startFunc, endFunc, singleStartFunc, singleEndFunc, x, ...) startFunc(x) APP_EXPAND(APP_FOR_EACH_12(startFunc, endFunc, singleStartFunc, singleEndFunc, __VA_ARGS__)) endFunc(x)
#define APP_FOR_EACH_14(startFunc, endFunc, singleStartFunc, singleEndFunc, x, ...) startFunc(x) APP_EXPAND(APP_FOR_EACH_13(startFunc, endFunc, singleStartFunc, singleEndFunc, __VA_ARGS__)) endFunc(x)
#define APP_FOR_EACH_15(startFunc, endFunc, singleStartFunc, singleEndFunc, x, ...) startFunc(x) APP_EXPAND(APP_FOR_EACH_14(startFunc, endFunc, singleStartFunc, singleEndFunc, __VA_ARGS__)) endFunc(x)
#define APP_FOR_EACH_16(startFunc, endFunc, singleStartFunc, singleEndFunc, x, ...) startFunc(x) APP_EXPAND(APP_FOR_EACH_15(startFunc, endFunc, singleStartFunc, singleEndFunc, __VA_ARGS__)) endFunc(x)
#define APP_FOR_EACH_17(startFunc, endFunc, singleStartFunc, singleEndFunc, x, ...) startFunc(x) APP_EXPAND(APP_FOR_EACH_16(startFunc, endFunc, singleStartFunc, singleEndFunc, __VA_ARGS__)) endFunc(x)
#define APP_FOR_EACH_18(startFunc, endFunc, singleStartFunc, singleEndFunc, x, ...) startFunc(x) APP_EXPAND(APP_FOR_EACH_17(startFunc, endFunc, singleStartFunc, singleEndFunc, __VA_ARGS__)) endFunc(x)
#define APP_FOR_EACH_19(startFunc, endFunc, singleStartFunc, singleEndFunc, x, ...) startFunc(x) APP_EXPAND(APP_FOR_EACH_18(startFunc, endFunc, singleStartFunc, singleEndFunc, __VA_ARGS__)) endFunc(x)
#define APP_FOR_EACH_20(startFunc, endFunc, singleStartFunc, singleEndFunc, x, ...) startFunc(x) APP_EXPAND(APP_FOR_EACH_19(startFunc, endFunc, singleStartFunc, singleEndFunc, __VA_ARGS__)) endFunc(x)

#define APP_GET_FOR_EACH_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,NAME,...) NAME

#define APP_MACRO_FOR_EACH_LASTFUNC(startFunc, singleStartFunc, endFunc, singleEndFunc, ...) \
  APP_EXPAND(APP_GET_FOR_EACH_MACRO(__VA_ARGS__,APP_FOR_EACH_20,APP_FOR_EACH_19,APP_FOR_EACH_18,APP_FOR_EACH_17,APP_FOR_EACH_16,APP_FOR_EACH_15,APP_FOR_EACH_14,APP_FOR_EACH_13,APP_FOR_EACH_12,APP_FOR_EACH_11,APP_FOR_EACH_10,APP_FOR_EACH_9,APP_FOR_EACH_8,APP_FOR_EACH_7,APP_FOR_EACH_6,APP_FOR_EACH_5,APP_FOR_EACH_4,APP_FOR_EACH_3,APP_FOR_EACH_2,APP_FOR_EACH_1)(startFunc, endFunc, singleStartFunc, singleEndFunc, __VA_ARGS__))


#define APP_MACRO_DUMMY(_x)
#define APP_MACRO_FOR_EACH(_fn1, _fn2, ...) APP_MACRO_FOR_EACH_LASTFUNC(_fn1, _fn1, _fn2, _fn2, __VA_ARGS__)
#define APP_MACRO_FOR_EACH_FORNT(_fn, ...) APP_MACRO_FOR_EACH(_fn, APP_MACRO_DUMMY, __VA_ARGS__)
#define APP_MACRO_FOR_EACH_BACK(_fn, ...)  APP_MACRO_FOR_EACH(APP_MACRO_DUMMY, _fn1, __VA_ARGS__)


//===================

#define APP_FOR_EACH_WITH_ARG_1( startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, x) singleStartFunc(a1, x) singleEndFunc(a2, x)
#define APP_FOR_EACH_WITH_ARG_2( startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, x, ...) startFunc(a1, x) APP_EXPAND(APP_FOR_EACH_WITH_ARG_1( startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, __VA_ARGS__)) endFunc(a2, x)
#define APP_FOR_EACH_WITH_ARG_3( startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, x, ...) startFunc(a1, x) APP_EXPAND(APP_FOR_EACH_WITH_ARG_2( startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, __VA_ARGS__)) endFunc(a2, x)
#define APP_FOR_EACH_WITH_ARG_4( startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, x, ...) startFunc(a1, x) APP_EXPAND(APP_FOR_EACH_WITH_ARG_3( startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, __VA_ARGS__)) endFunc(a2, x)
#define APP_FOR_EACH_WITH_ARG_5( startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, x, ...) startFunc(a1, x) APP_EXPAND(APP_FOR_EACH_WITH_ARG_4( startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, __VA_ARGS__)) endFunc(a2, x)
#define APP_FOR_EACH_WITH_ARG_6( startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, x, ...) startFunc(a1, x) APP_EXPAND(APP_FOR_EACH_WITH_ARG_5( startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, __VA_ARGS__)) endFunc(a2, x)
#define APP_FOR_EACH_WITH_ARG_7( startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, x, ...) startFunc(a1, x) APP_EXPAND(APP_FOR_EACH_WITH_ARG_6( startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, __VA_ARGS__)) endFunc(a2, x)
#define APP_FOR_EACH_WITH_ARG_8( startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, x, ...) startFunc(a1, x) APP_EXPAND(APP_FOR_EACH_WITH_ARG_7( startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, __VA_ARGS__)) endFunc(a2, x)
#define APP_FOR_EACH_WITH_ARG_9( startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, x, ...) startFunc(a1, x) APP_EXPAND(APP_FOR_EACH_WITH_ARG_8( startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, __VA_ARGS__)) endFunc(a2, x)
#define APP_FOR_EACH_WITH_ARG_10(startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, x, ...) startFunc(a1, x) APP_EXPAND(APP_FOR_EACH_WITH_ARG_9( startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, __VA_ARGS__)) endFunc(a2, x)
#define APP_FOR_EACH_WITH_ARG_11(startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, x, ...) startFunc(a1, x) APP_EXPAND(APP_FOR_EACH_WITH_ARG_10(startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, __VA_ARGS__)) endFunc(a2, x)
#define APP_FOR_EACH_WITH_ARG_12(startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, x, ...) startFunc(a1, x) APP_EXPAND(APP_FOR_EACH_WITH_ARG_11(startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, __VA_ARGS__)) endFunc(a2, x)
#define APP_FOR_EACH_WITH_ARG_13(startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, x, ...) startFunc(a1, x) APP_EXPAND(APP_FOR_EACH_WITH_ARG_12(startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, __VA_ARGS__)) endFunc(a2, x)
#define APP_FOR_EACH_WITH_ARG_14(startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, x, ...) startFunc(a1, x) APP_EXPAND(APP_FOR_EACH_WITH_ARG_13(startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, __VA_ARGS__)) endFunc(a2, x)
#define APP_FOR_EACH_WITH_ARG_15(startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, x, ...) startFunc(a1, x) APP_EXPAND(APP_FOR_EACH_WITH_ARG_14(startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, __VA_ARGS__)) endFunc(a2, x)
#define APP_FOR_EACH_WITH_ARG_16(startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, x, ...) startFunc(a1, x) APP_EXPAND(APP_FOR_EACH_WITH_ARG_15(startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, __VA_ARGS__)) endFunc(a2, x)
#define APP_FOR_EACH_WITH_ARG_17(startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, x, ...) startFunc(a1, x) APP_EXPAND(APP_FOR_EACH_WITH_ARG_16(startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, __VA_ARGS__)) endFunc(a2, x)
#define APP_FOR_EACH_WITH_ARG_18(startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, x, ...) startFunc(a1, x) APP_EXPAND(APP_FOR_EACH_WITH_ARG_17(startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, __VA_ARGS__)) endFunc(a2, x)
#define APP_FOR_EACH_WITH_ARG_19(startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, x, ...) startFunc(a1, x) APP_EXPAND(APP_FOR_EACH_WITH_ARG_18(startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, __VA_ARGS__)) endFunc(a2, x)
#define APP_FOR_EACH_WITH_ARG_20(startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, x, ...) startFunc(a1, x) APP_EXPAND(APP_FOR_EACH_WITH_ARG_19(startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, __VA_ARGS__)) endFunc(a2, x)

#define APP_GET_FOR_EACH_WITH_ARG_MACRO(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,NAME,...) NAME

#define APP_MACRO_FOR_EACH_WITH_ARG_LASTFUNC(startFunc, singleStartFunc, endFunc, singleEndFunc, a1, a2, ...) \
  APP_EXPAND(APP_GET_FOR_EACH_WITH_ARG_MACRO(__VA_ARGS__,APP_FOR_EACH_WITH_ARG_20,APP_FOR_EACH_WITH_ARG_19,APP_FOR_EACH_WITH_ARG_18,APP_FOR_EACH_WITH_ARG_17,APP_FOR_EACH_WITH_ARG_16,APP_FOR_EACH_WITH_ARG_15,APP_FOR_EACH_WITH_ARG_14,APP_FOR_EACH_WITH_ARG_13,APP_FOR_EACH_WITH_ARG_12,APP_FOR_EACH_WITH_ARG_11,APP_FOR_EACH_WITH_ARG_10,APP_FOR_EACH_WITH_ARG_9,APP_FOR_EACH_WITH_ARG_8,APP_FOR_EACH_WITH_ARG_7,APP_FOR_EACH_WITH_ARG_6,APP_FOR_EACH_WITH_ARG_5,APP_FOR_EACH_WITH_ARG_4,APP_FOR_EACH_WITH_ARG_3,APP_FOR_EACH_WITH_ARG_2,APP_FOR_EACH_WITH_ARG_1)(startFunc, endFunc, singleStartFunc, singleEndFunc, a1, a2, __VA_ARGS__))


#define APP_MACRO_DUMMY_WITH_ARG(_a, _x)
#define APP_MACRO_FOR_EACH_WITH_ARG(_fn1, _fn2, _a1, _a2, ...) APP_MACRO_FOR_EACH_WITH_ARG_LASTFUNC(_fn1, _fn1, _fn2, _fn2, _a1, _a2, __VA_ARGS__)
#define APP_MACRO_FOR_EACH_WITH_ARG_FORNT(_fn, _a, ...) APP_MACRO_FOR_EACH_WITH_ARG(_fn, APP_MACRO_DUMMY_WITH_ARG, _a, _a, __VA_ARGS__)
#define APP_MACRO_FOR_EACH_WITH_ARG_BACK(_fn, _a, ...)  APP_MACRO_FOR_EACH_WITH_ARG(APP_MACRO_DUMMY_WITH_ARG, _fn1, _a, _a, __VA_ARGS__)

#endif //__WINDOWS_OS__

#ifdef __cplusplus
}
#endif

#endif /* APP_FOREACH_MACROS_H_ */
