#pragma once
#include "INtimeOS.h"
namespace master
{
    namespace osal
    {
        typedef INtimeNIC OSAL_NIC;
        typedef INtimeThread OSAL_THREAD;
        typedef INtimeMutex OSAL_MUTEX;
        typedef INtimeSem OSAL_SEM;
        typedef INtimeHeap OSAL_HEAP;
        typedef INtimeTime OSAL_TIMER;
        // typedef INtimeIPC OSAL_IPC;
    }
}