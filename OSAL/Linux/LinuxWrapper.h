#pragma once
#ifndef LinuxWRAPPER
#define LinuxWRAPPER
#include "LinuxOS.h"

typedef LinuxNIC OSAL_NIC;
typedef LinuxThread OSAL_THREAD;
typedef LinuxMutex OSAL_MUTEX;
typedef LinuxSem OSAL_SEM;
typedef LinuxHeap OSAL_HEAP;
typedef LinuxTime OSAL_TIMER;
typedef LinuxIPC OSAL_IPC;
#endif