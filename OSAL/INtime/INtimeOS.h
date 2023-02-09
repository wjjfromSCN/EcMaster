#pragma once
#ifndef INTIMEOSALACHIEVE
#define INTIMEOSALACHIEVE
#include <windows.h>
#include <iwin32.h>
#include <rt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/endian.h>
#include <ctype.h>
#include <hpeif2.h>
#include "../OSFactory.h"

class INtimeNIC : public McNicInterface
{
private:
    HPEHANDLE NicHandle;
    HPESTATUS Status;
    HPEMEDIASTATUS Mstat;
    DWORD Txstate;

    const INT32 Interrupts = ALL_INTERRUPTS;
    const INT32 DontWaitForLink = 0;
    const static INT32 ADDR_LEN = 6;

    INT32 Has64BitDMA;
    time_t Now, t;
    HPETXBUFFERSET *Txs;
    HPERXBUFFERSET *Rxs;
    ULONG N_TX_BUFS;
    ULONG N_RX_BUFS;
    ULONG TX_BUF_SIZE;
    ULONG RX_BUF_SIZE;

    HPE_INTERFACE_CAPABILITIES HpeCaps;
    HPE_INTERFACE_PARAMETERS HpeParams;
    HPE_INTERFACE_PCIBUS HpePci;

    BYTE MacAddress[ADDR_LEN];

public:
    INtimeNIC();
    ~INtimeNIC();
    virtual INT32 Open(const char *NicName, BYTE *HwAddress) override;
    virtual INT32 Close() override;
    virtual INT32 SendFrame(void *pData, INT32 Len, INT32 Flag) override;
    virtual INT32 RecvFrame(void *pData, INT32 Len, INT32 Flag) override;

private:
    VOID InitNic();
    VOID AdjustNicAbility();
    INT32 AllocRxBufferSet(UINT N, ULONG RxBufSize);
    VOID AllocTxBufferSet(UINT n);
    VOID FreeTxBufferSet(HPETXBUFFERSET *txs);
    BYTE *GetMacAddress();
};

class INtimeThread : public McThreadInterface
{
private:
    CHAR *ThreadName;
    UINT32 StackSize;
    RTHANDLE rx;

public:
    INtimeThread();
    ~INtimeThread();
    virtual INT32 Start(void (*Task)(void *), INT32 Priority, void *Context, const char *ThreadName) override;
    virtual INT32 Join() override;
};

class INtimeMutex : public McMutexInterface
{
private:
    HANDLE Mutex;
    RTHANDLE RxWaitSem;
public:
    INtimeMutex();
    ~INtimeMutex();
    virtual VOID Lock() override;
    virtual VOID UnLock() override;
};

class INtimeSem : public McSemInterface
{
private:
    RTHANDLE RxWaitSem;

public:
    INtimeSem();
    ~INtimeSem();
    virtual VOID Pend() override;
    virtual VOID Signal() override;
    virtual VOID BroadCast() override;
};

class INtimeHeap : public McHeapInterface
{
private:
public:
    INtimeHeap();
    ~INtimeHeap();
    virtual VOID *Alloc(INT32 Size) override;
    virtual VOID Free(void *pData) override;
};

class INtimeTime : public McTimeInterface
{
private:
    TIMEVALUE TimeValue;

public:
    INtimeTime();
    ~INtimeTime();
    virtual UINT64 GetTime() override;
    virtual UINT64 TaskSleepUntill(UINT64 TargetTime) override;
    virtual void TaskSleep(UINT64 Time) override;
    void DelayInMicroseconds(UINT64 us);
};
#endif