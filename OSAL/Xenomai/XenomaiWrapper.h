#pragma once
#ifndef XENOMAIWRAPPER
#define XENOMAIWRAPPER
#include "XenomaiOS.h"
namespace master
{
    namespace osal
    {
        typedef XenomaiNIC OSAL_NIC;
        typedef XenomaiThread OSAL_THREAD;
        typedef XenomaiMutex OSAL_MUTEX;
        typedef XenomaiSem OSAL_SEM;
        typedef XenomaiHeap OSAL_HEAP;
        typedef XenomaiTime OSAL_TIMER;
        typedef XenomaiIPC OSAL_IPC;
    }
}
#endif