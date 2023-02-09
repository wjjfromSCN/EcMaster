#pragma once
#ifndef XENOMAIOSALACHIEVE
#define XENOMAIOSALACHIEVE
#include "../OSFactory.h"
#include <alchemy/task.h>
#include <alchemy/sem.h>
#include <alchemy/mutex.h>
#include <alchemy/heap.h>
#include <string>
template <typename... Args>
inline void RtPrintf(const Args &...args)
{
    rt_printf(args...);
}

#define __DEBUG //日志模块总开关，注释掉将关闭日志输出

#ifdef __DEBUG
#define DEBUG(format, ...) RtPrintf(format, ##__VA_ARGS__)
#else
#define DEBUG(format, ...)
#endif

//定义日志级别
enum LOG_LEVEL
{

    LOG_LEVEL_TRACE,
    LOG_LEVEL_INFO,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERR,
    LOG_LEVEL_OFF
};
const LOG_LEVEL level = LOG_LEVEL_INFO;
#define EC_TRACE(format, ...)                                               \
    do                                                                      \
    {                                                                       \
        if (level <= LOG_LEVEL_TRACE)                                       \
            DEBUG("\n->TRACE   @ FUNC:%s FILE:%s LINE:%d \n" format "\n\t", \
                  __func__, __FILE__, __LINE__, ##__VA_ARGS__);             \
    } while (0)
#define EC_ERROR(format, ...)                                               \
    do                                                                      \
    {                                                                       \
        if (level <= LOG_LEVEL_ERR)                                         \
            DEBUG("\n->ERROR   @ FUNC:%s FILE:%s LINE:%d \n" format "\n\t", \
                  __func__, __FILE__, __LINE__, ##__VA_ARGS__);             \
    } while (0)

#define EC_WARN(format, ...)                                                      \
    do                                                                            \
    {                                                                             \
        if (level <= LOG_LEVEL_WARN)                                              \
            DEBUG("\n->WARN  @ FUNC:%s \n" format "\n", __func__, ##__VA_ARGS__); \
    } while (0)

#define EC_INFO(format, ...)                                  \
    do                                                        \
    {                                                         \
        if (level <= LOG_LEVEL_INFO)                          \
            DEBUG("\n->INFO  \n" format "\n", ##__VA_ARGS__); \
    } while (0)

#define EC_DEBUG(format, ...)                                 \
    do                                                        \
    {                                                         \
        if (level <= LOG_LEVEL_DEBUG)                         \
            DEBUG("\n->DEBUG \n" format "\n", ##__VA_ARGS__); \
    } while (0)

class XenomaiNIC : public McNicInterface
{
private:
    typedef INT32 SOCKET;
    std::string m_NicName;
    INT32 InitSockets(std::string strIfName, BYTE *HwAddress);
    SOCKET m_rcvSocket;
    SOCKET m_sndSocket;

public:
    XenomaiNIC(/* args */);
    ~XenomaiNIC();
    virtual INT32 Open(const char *NicName, BYTE *HwAddress) override;
    virtual INT32 Close() override;
    virtual INT32 SendFrame(void *pData, INT32 Len, INT32 Flag) override;
    virtual INT32 RecvFrame(void *pData, INT32 Len, INT32 Flag) override;
};

class XenomaiThread : public McThreadInterface
{
    typedef RT_TASK ThreadHandle;
    ThreadHandle m_TaskHandle;
    void (*m_Task)(void *);
    INT32 m_Priority;
    std::string m_ThreadName;
    void *m_Context;
    BOOL NeedJoinFlag;

public:
    XenomaiThread();
    virtual ~XenomaiThread();
    virtual INT32 Start(void (*Task)(void *), INT32 Priority, void *Context, const char *ThreadName) override;
    virtual INT32 Join() override;
    VOID PrintInfo();
};
class XenomaiMutex : public McMutexInterface
{
    RT_MUTEX m_csCrit;

public:
    XenomaiMutex();
    virtual ~XenomaiMutex();
    virtual void Lock() override;
    virtual void UnLock() override;
};
class XenomaiSem : public McSemInterface
{
    typedef RT_SEM SemHandle;
    SemHandle m_SemHandle;

public:
    XenomaiSem(const char *SemName = nullptr);
    virtual ~XenomaiSem();
    virtual VOID Pend() override;   // get resource
    virtual VOID Signal() override; // release resource
    virtual VOID BroadCast() override;
};
class XenomaiHeap : public McHeapInterface
{
    RT_HEAP heapPtr;
    std::string m_name;

public:
    XenomaiHeap(const char *name, INT32 size);
    virtual ~XenomaiHeap();
    VOID *Alloc(INT32 Size) override;
    VOID Free(void *pData) override;
};

struct XenomaiTime : public McTimeInterface
{
public:
    XenomaiTime();
    ~XenomaiTime();
    UINT64 GetTime() override;                          // ns
    UINT64 TaskSleepUntill(UINT64 TargetTime) override; // ns
    void TaskSleep(UINT64 Time) override;
};
struct XenomaiIPC : public McIPCInterface
{
    const INT32 INVALID_SOCKET_DESCRIPT = -1;
    std::string EndPortDescritption;
    UINT32 EndPortNum;
    INT32 IDDPPortSocket;


    INT32 TargetEndPortNum;
    bool CloseFlag;
public:
    XenomaiIPC(std::string EndPortDefine, UINT32 PortNum,UINT32 TargetPortNum);

    virtual INT32 EndPortInit() override;
    virtual INT32 EndPortClose() override;
    virtual INT32 EndPortSend(VOID * pData, UINT32 len, INT32 Flag) override;
    virtual INT32 EndPortRecv(VOID * RecvBuffer, UINT32 RecvLen, INT32 Flag) override;
    virtual ~XenomaiIPC();
};

#endif
