#pragma once
#ifndef XENOMAI_W550_WRAPPER_H
#define XENOMAI_W550_WRAPPER_H
#include "../Xenomai/XenomaiOS.h"
#include "W550OS.h"

typedef W550NIC OSAL_NIC;
typedef XenomaiThread OSAL_THREAD;
typedef XenomaiMutex OSAL_MUTEX;
typedef XenomaiSem OSAL_SEM;
typedef XenomaiHeap OSAL_HEAP;
typedef XenomaiTime OSAL_TIMER;
typedef XenomaiIPC OSAL_IPC;
#endif