/************************************************
 * Copyright(c) 2019 Sang Yang
 * 
 * Project:  Framework
 * FileName: Signal.h
 * Author:   shanyyang
 * Email:    tangminjie@163.com
 * Version: V1.0.0
 * Date: 2021-01-03 15:23
 * Description: 
 * Others:
 *************************************************/


#ifndef FRAMEWORK_INCLUDE_INFRA_SIGNAL_H
#define FRAMEWORK_INCLUDE_INFRA_SIGNAL_H

#include <cstdio>
#include "Infra/Function.h"
#include "Infra/Time/Time.h"
#include "Infra/Thread/Thread.h"
#include "Infra/Concurrence/Mutex.h"
#include "Infra/Concurrence/Guard.h"

namespace Uface {
namespace Infra {

//TSignal0
#define SIGNAL_NUMBER       0
#define SIGNAL_CLASS_TYPES  void
#define SIGNAL_TYPES_COMMA
#define SIGNAL_TYPE_ARGS    void
#define SIGNAL_ARGS
#include "Infra/SignalTemplate.h"
#undef  SIGNAL_NUMBER
#undef  SIGNAL_CLASS_TYPES
#undef	SIGNAL_TYPES_COMMA
#undef	SIGNAL_TYPE_ARGS
#undef	SIGNAL_ARGS

//TSignal1
#define SIGNAL_NUMBER 1
#define SIGNAL_CLASS_TYPES  typename T1
#define SIGNAL_TYPES_COMMA  , T1
#define SIGNAL_TYPE_ARGS    T1 a1
#define SIGNAL_ARGS         a1
#include "Infra/SignalTemplate.h"
#undef  SIGNAL_NUMBER
#undef  SIGNAL_CLASS_TYPES
#undef	SIGNAL_TYPES_COMMA
#undef	SIGNAL_TYPE_ARGS
#undef	SIGNAL_ARGS

//TSignal2
#define SIGNAL_NUMBER 2
#define SIGNAL_CLASS_TYPES  typename T1, typename T2
#define SIGNAL_TYPES_COMMA  , T1, T2
#define SIGNAL_TYPE_ARGS    T1 a1, T2 a2
#define SIGNAL_ARGS         a1, a2
#include "Infra/SignalTemplate.h"
#undef  SIGNAL_NUMBER
#undef  SIGNAL_CLASS_TYPES
#undef	SIGNAL_TYPES_COMMA
#undef	SIGNAL_TYPE_ARGS
#undef	SIGNAL_ARGS

//TSignal3
#define SIGNAL_NUMBER 3
#define SIGNAL_CLASS_TYPES  typename T1, typename T2, typename T3
#define SIGNAL_TYPES_COMMA  , T1, T2, T3
#define SIGNAL_TYPE_ARGS    T1 a1, T2 a2, T3 a3
#define SIGNAL_ARGS         a1, a2, a3
#include "Infra/SignalTemplate.h"
#undef  SIGNAL_NUMBER
#undef  SIGNAL_CLASS_TYPES
#undef	SIGNAL_TYPES_COMMA
#undef	SIGNAL_TYPE_ARGS
#undef	SIGNAL_ARGS

//TSignal4
#define SIGNAL_NUMBER 4
#define SIGNAL_CLASS_TYPES  typename T1, typename T2, typename T3, typename T4
#define SIGNAL_TYPES_COMMA  , T1, T2, T3, T4
#define SIGNAL_TYPE_ARGS    T1 a1, T2 a2, T3 a3, T4 a4
#define SIGNAL_ARGS         a1, a2, a3, a4
#include "Infra/SignalTemplate.h"
#undef  SIGNAL_NUMBER
#undef  SIGNAL_CLASS_TYPES
#undef	SIGNAL_TYPES_COMMA
#undef	SIGNAL_TYPE_ARGS
#undef	SIGNAL_ARGS

//TSignal5
#define SIGNAL_NUMBER 5
#define SIGNAL_CLASS_TYPES  typename T1, typename T2, typename T3, typename T4, typename T5
#define SIGNAL_TYPES_COMMA  , T1, T2, T3, T4, T5
#define SIGNAL_TYPE_ARGS    T1 a1, T2 a2, T3 a3, T4 a4, T5 a5
#define SIGNAL_ARGS         a1, a2, a3, a4, a5
#include "Infra/SignalTemplate.h"
#undef  SIGNAL_NUMBER
#undef  SIGNAL_CLASS_TYPES
#undef	SIGNAL_TYPES_COMMA
#undef	SIGNAL_TYPE_ARGS
#undef	SIGNAL_ARGS

//TSignal6
#define SIGNAL_NUMBER 6
#define SIGNAL_CLASS_TYPES  typename T1, typename T2, typename T3, typename T4, typename T5, typename T6
#define SIGNAL_TYPES_COMMA  , T1, T2, T3, T4, T5, T6
#define SIGNAL_TYPE_ARGS    T1 a1, T2 a2, T3 a3, T4 a4, T5 a5, T6 a6
#define SIGNAL_ARGS         a1, a2, a3, a4, a5, a6
#include "Infra/SignalTemplate.h"
#undef  SIGNAL_NUMBER
#undef  SIGNAL_CLASS_TYPES
#undef	SIGNAL_TYPES_COMMA
#undef	SIGNAL_TYPE_ARGS
#undef	SIGNAL_ARGS

} // namespace Infra
} // namespace Uface

#if defined(_MSC_VER)
#pragma warning (pop)
#endif

#endif //FRAMEWORK_INCLUDE_INFRA_SIGNAL_H
