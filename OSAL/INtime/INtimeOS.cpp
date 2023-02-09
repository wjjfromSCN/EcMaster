#include "INtimeOS.h"

INtimeNIC::INtimeNIC() : Txs(0), Rxs(0),
                         N_TX_BUFS(3), N_RX_BUFS(256),
                         TX_BUF_SIZE(1500), RX_BUF_SIZE(1536),
                         Now(0), t(0), Has64BitDMA(0)
{
}
INtimeNIC::~INtimeNIC()
{
}
INT32 INtimeNIC::Open(const char *NicName, BYTE *HwAddress)
{
    Status = hpeOpen(NicName, SPEED_100 | DUPLEX_FULL | DontWaitForLink, Interrupts, &NicHandle);
    if (Status != E_OK)
    {
        printf("hpeOpen failed with status %04x\n", Status);
        return Status;
    }
    InitNic();
    memcpy(HwAddress, MacAddress, 6);
    return E_OK;
}
INT32 INtimeNIC::Close()
{
    if (Txs != NULL)
    {
        FreeTxBufferSet(Txs);
        printf("Free TxBufferSet!");
    }
    if (Rxs != NULL)
    {
        hpeFreeReceiveBufferSet(NicHandle, &Rxs);
        printf("Free RxBufferSet!");
    }
    Status = hpeClose(NicHandle);
    printf("Closing the Nic...\n");
    return Status;
}
INT32 INtimeNIC::SendFrame(void *pData, INT32 Len, INT32 Flag)
{
    BYTE *payload = (BYTE *)Txs->buffers[0].fragments[0].ptr;
    memcpy(payload, (BYTE *)pData, Len);
    Txs->buffer_count = 1;

    if ((Status = hpeAttachTransmitBufferSet(NicHandle, Txs)) != E_OK)
    {
        printf("Failed to attach transmit buffer set: %04x\n", Status);
        Close();
        return Status;
    }
    if ((Status = hpeStartTransmitter(NicHandle)) != E_OK)
    {
        printf("TX: Failed to start transmitter: %04x\n", Status);
        Close();
        return Status;
    }
    if (Interrupts & OUTPUT_INTERRUPT)
    {
        for (INT32 j = 0; j < (INT32)Txs->buffer_count; j++)
        {
            if ((Status = hpeWaitForTransmitComplete(NicHandle, INFINITE)) != E_OK)
            {
                printf("TX: hpeWaitForTransmitComplete failed: %04x\n", Status);
                Close();
                return Status;
            }
        }
    }
    else
    {
        while (hpeGetTransmitterState(NicHandle, &Txstate) == E_OK && Txstate == HPE_TXBUSY)
        {
        }
    }
}
INT32 INtimeNIC::RecvFrame(void *pData, INT32 Len, INT32 Flag)
{
    HPEBUFFER *rxbuf;
    if (Interrupts & INPUT_INTERRUPT)
    {
        if ((Status = hpeWaitForReceiveComplete(NicHandle, INFINITE)) == E_TIME)
        {
            printf("RX: (hpeWaitForReceiveComplete timed out)\n");
            return Status;
        }
        else if (Status != E_OK)
        {
            printf("RX: hpeWaitForReceiveComplete failed: %04x\n", Status);
            return Status;
        }
    }
    else
    {
        RtSleepEx(10);
    }
    while ((Status = hpeGetReceiveBuffer(NicHandle, &rxbuf)) == E_OK)
    {
        if (rxbuf->used == 0)
        {
            continue;
        }
        if (rxbuf->used >= 0x600)
        {
            printf("RX: Invalid receive buffer: len=%x\n", rxbuf->used);
            continue;
        }
        memcpy((BYTE *)pData, (BYTE *)rxbuf->ptr, Len);
        if (Interrupts & INPUT_INTERRUPT)
        {
            break;
        }
    }

    return rxbuf->used;
}
VOID INtimeNIC::InitNic()
{
    AdjustNicAbility();
    time(&Now);
    do
    {
        if ((Status = hpeGetMediaStatus(NicHandle, &Mstat)) != E_OK)
        {
            printf("hpeGetMediaStatus failed with status %04x\n", Status);
            Close();
            return;
        }
        if (Mstat.media_speed == SPEED_NONE)
        {
            RtSleepEx(1000);
            time(&t);
            printf("Waiting for the link...\n");
        }
    } while (Mstat.media_speed == SPEED_NONE && t < (Now + 10));

    if (Mstat.media_speed != SPEED_100 || Mstat.media_duplex != DUPLEX_FULL)
    {
        printf("Link is not connected as requested: speed=%u, duplex=%u\n", Mstat.media_speed, Mstat.media_duplex);
        Close();
        return;
    }
    if ((Status = hpeGetMacAddress(NicHandle, MacAddress)) != E_OK)
    {
        printf("hpeGetMacAddress failed with status %04x\n", Status);
        Close();
        return;
    }

    if (AllocRxBufferSet(N_RX_BUFS, RX_BUF_SIZE) != E_OK)
    {
        printf("Unable to allocate the RX buffer set\n");
        Close();
        return;
    }
    if ((Status = hpeAttachReceiveBufferSet(NicHandle, Rxs)) != E_OK)
    {
        printf("Failed to attach receive buffer set: %04x\n", Status);
        Close();
        return;
    }

    AllocTxBufferSet(N_TX_BUFS);
    if (Txs == NULL)
    {
        printf("Unable to allocate the TX buffer set\n");
        Close();
        return;
    }
}
VOID INtimeNIC::AdjustNicAbility()
{
    Status = hpeGetInterfaceInfo(NicHandle, HPE_INFO_INTERFACE_CAPABILITIES, (LPVOID *)&HpeCaps, sizeof(HpeCaps));
    if (Status != E_OK)
    {
        Close();
        printf("hpeGetInterfaceInfo(HPE_INFO_INTERFACE_CAPABILITIES) failed with status %04x\n", Status);
        return;
    }
    Status = hpeGetInterfaceInfo(NicHandle, HPE_INFO_INTERFACE_PARAMETERS, (LPVOID *)&HpeParams, sizeof(HpeParams));
    if (Status != E_OK)
    {
        Close();
        printf("hpeGetInterfaceInfo(HPE_INFO_INTERFACE_PARAMETERS) failed with status %04x\n", Status);
        return;
    }
    Status = hpeGetInterfaceInfo(NicHandle, HPE_INFO_INTERFACE_PCIBUS, (LPVOID *)&HpePci, sizeof(HpePci));
    if (Status != E_OK)
    {
        Close();
        printf("hpeGetInterfaceInfo(HPE_INFO_INTERFACE_PCIBUS) failed with status %04x\n", Status);
        return;
    }

    Has64BitDMA = HpeCaps.capabilities & HPE_CAP_64BIT_DMA ? 1 : 0;
    if (N_RX_BUFS < HpeParams.min_receive_buffers)
    {
        N_RX_BUFS = HpeParams.min_receive_buffers;
    }
    if (N_TX_BUFS < HpeParams.min_transmit_buffers)
    {
        N_TX_BUFS = HpeParams.min_transmit_buffers;
    }
    if (RX_BUF_SIZE < HpeParams.min_receive_bufsize)
    {
        RX_BUF_SIZE = HpeParams.min_receive_bufsize;
    }
}
INT32 INtimeNIC::AllocRxBufferSet(UINT N, ULONG RxBufSize)
{
    Status = hpeAllocateReceiveBufferSet(NicHandle, &Rxs, N, RxBufSize); // N * RxBufSize
    return Status;
}
VOID INtimeNIC::AllocTxBufferSet(UINT n)
{
    // AllocateTxBufferSet: allocate n buffers of tx buf size each
    Txs = (HPETXBUFFERSET *)malloc(sizeof(HPETXBUFFER) * n + sizeof(HPETXBUFFERSET));
    if (!Txs)
    {
        Txs = NULL;
        return;
    }
    memset(Txs, 0, sizeof(HPETXBUFFER) * n + sizeof(HPETXBUFFERSET));
    Txs->buffer_count = n;

    for (UINT i = 0; i < n; i++)
    {
        Txs->buffers[i].fragment_count = 1;

        for (UINT frag = 0; frag < 1; frag++)
        {
            if (Has64BitDMA)
            {
                Txs->buffers[i].fragments[frag].ptr = AllocateRtMemory(TX_BUF_SIZE);
            }
            else
            {
                Txs->buffers[i].fragments[frag].ptr = AllocateRtMemoryEx(TX_BUF_SIZE, 0, 0xffffffff, 0);
            }
            if (!Txs->buffers[i].fragments[frag].ptr)
            {
                printf("Failed to allocate TX buffer\n");
                Txs = NULL;
                return;
            }
            Txs->buffers[i].fragments[frag].size = TX_BUF_SIZE;
        }
    }
}
VOID INtimeNIC::FreeTxBufferSet(HPETXBUFFERSET *Txs)
{
    if (Txs != NULL)
    {
        for (INT32 i = 0; i < (INT32)Txs->buffer_count; i++)
        {
            for (INT32 j = 0; j < (INT32)Txs->buffers[i].fragment_count; j++)
            {
                FreeRtMemory(Txs->buffers[i].fragments[j].ptr);
            }
        }
        free(Txs);
    }
}
BYTE *INtimeNIC::GetMacAddress()
{
    return MacAddress;
}

INtimeThread::INtimeThread() : StackSize(0x2000), rx(0)
{
}
INtimeThread::~INtimeThread()
{
    DeleteRtThread(rx);
}
INT32 INtimeThread::Start(void (*Task)(void *), INT32 Priority, void *Context, const char *ThreadName)
{
    if ((rx = CreateRtThread(Priority, (LPPROC)Task, StackSize, Context)) == BAD_RTHANDLE)
    {
        printf("Failed to create the RX thread: %04x\n", GetLastRtError()); //返回由失败的调用线程进行的最后一次系统调用的状态
        return GetLastRtError();
    }
    this->ThreadName = const_cast<char *>(ThreadName);
    return E_OK;
}
INT32 INtimeThread::Join()
{
    return E_OK;
}

INtimeMutex::INtimeMutex()
{
    RxWaitSem = CreateRtSemaphore(1, 1, FIFO_QUEUING);
}
INtimeMutex::~INtimeMutex()
{
    DeleteRtSemaphore(RxWaitSem);
}
VOID INtimeMutex::Lock()
{
    WaitForRtSemaphore(RxWaitSem, 1, WAIT_FOREVER);
}
VOID INtimeMutex::UnLock()
{
    ReleaseRtSemaphore(RxWaitSem, 1);
}

INtimeSem::INtimeSem()
{
    RxWaitSem = CreateRtSemaphore(1, 1, FIFO_QUEUING);
}
INtimeSem::~INtimeSem()
{
    DeleteRtSemaphore(RxWaitSem);
}
VOID INtimeSem::Pend()
{
    WaitForRtSemaphore(RxWaitSem, 1, WAIT_FOREVER);
}
VOID INtimeSem::Signal()
{
    ReleaseRtSemaphore(RxWaitSem, 1);
}
VOID INtimeSem::BroadCast()
{
}

INtimeHeap::INtimeHeap()
{
}
INtimeHeap::~INtimeHeap()
{
}
VOID *INtimeHeap::Alloc(INT32 Size)
{
    VOID *ptr;
    ptr = AllocateRtMemory(Size);
    if (ptr == NULL)
    {
        printf("AllocateRtMemory() faild.");
        return NULL;
    }
    return ptr;
}
VOID INtimeHeap::Free(void *pData)
{
    INT32 Res = FreeRtMemory(pData);
    if (Res == -1)
    {
        printf("FreeRtMemory() faild.");
    }
}

INtimeTime::INtimeTime()
{
}
INtimeTime::~INtimeTime()
{
}
UINT64 INtimeTime::GetTime()
{
    GetRtSystemTimeAsTimeValue(&TimeValue);
    UINT64 Res = (TimeValue.qwSeconds * 1000000000 + TimeValue.dwMicroseconds * 1000);
    return Res;
}
UINT64 INtimeTime::TaskSleepUntill(UINT64 TargetTime)
{
	UINT64 Time = TargetTime - GetTime();
	DelayInMicroseconds(Time / 1000);
	return E_OK;
}
void INtimeTime::TaskSleep(UINT64 Time)
{
    DelayInMicroseconds(Time/1000);
}

void INtimeTime::DelayInMicroseconds(UINT64 us)
{
	static LARGE_INTEGER freq = { 0, 0 };
	LARGE_INTEGER start;
	LARGE_INTEGER delay;
	LARGE_INTEGER now;
	volatile INT32 cycles = 0;

	if (freq.QuadPart == 0)
	{
		QueryPerformanceFrequency(&freq);
	}

	delay.QuadPart = us * freq.QuadPart / 1000000;

	QueryPerformanceCounter(&start);
	do
	{
		QueryPerformanceCounter(&now);
		// do busy wait here
		++cycles;
	} while (now.QuadPart - start.QuadPart < delay.QuadPart);
}