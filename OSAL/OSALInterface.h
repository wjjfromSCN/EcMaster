#pragma once
#ifndef OSFACTORY
#define OSFACTORY
#include <functional>
#include "OSALType.h"
namespace master
{
    namespace osal
    {
        struct McNicInterface
        {
        public:
            McNicInterface() = default;
            virtual ~McNicInterface() = default;
            virtual INT32 Open(const char *NicName, BYTE *HwAddress) = 0; // hqaddress size is 6
            virtual INT32 Close() = 0;
            virtual INT32 SendFrame(void *pData, INT32 Len, INT32 Flag) = 0;
            virtual INT32 RecvFrame(void *pData, INT32 Len, INT32 Flag) = 0;
        };
        struct McThreadInterface
        {
            McThreadInterface(){};
            virtual ~McThreadInterface() = default;
            virtual INT32 Start(void (*Task)(void *), INT32 Priority, void *Context, const char *ThreadName) = 0;
            virtual INT32 Join() = 0;
        };
        struct McMutexInterface
        {
            McMutexInterface() = default;
            virtual ~McMutexInterface() = default;
            virtual void Lock() = 0;
            virtual void UnLock() = 0;
        };
        struct McSemInterface
        {
            McSemInterface() = default;
            virtual ~McSemInterface() = default;
            virtual VOID Pend() = 0;   // get resource
            virtual VOID Signal() = 0; // release resource
            virtual VOID BroadCast() = 0;
        };
        struct McHeapInterface
        {
            McHeapInterface() = default;
            virtual ~McHeapInterface() = default;
            virtual VOID *Alloc(INT32 Size) = 0;
            virtual VOID Free(void *pData) = 0;
        };
        struct McTimeInterface
        {
            McTimeInterface() = default;
            virtual ~McTimeInterface() = default;
            virtual UINT64 GetTime() = 0;                          // ns
            virtual UINT64 TaskSleepUntill(UINT64 TargetTime) = 0; // ns
            virtual void TaskSleep(UINT64 Time) = 0;
        };
        struct McIPCInterface
        {
        public:
            McIPCInterface() = default;
            virtual ~McIPCInterface() = default;
            virtual INT32 EndPortInit() = 0;
            virtual INT32 EndPortClose() = 0;
            virtual INT32 EndPortSend(VOID *pData, UINT32 len, INT32 Flag) = 0;
            virtual INT32 EndPortRecv(VOID *RecvBuffer, UINT32 RecvLen, INT32 Flag) = 0;
        };
        struct LOGInterface
        {
        public:
            VOID Trace();
            VOID Info();
            VOID Debug();
            VOID Warn()
            {
                
            }
            template <typename T>
            VOID RegLogFun(std::function<void(T &&t)> &func)
            {
                printFunc = func;
            }

        private:
            template <typename T>
            std::function<void(T &&t)> printFunc;
        };

    }
}