#include "LinuxOS.h"
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netpacket/packet.h>
#include <string.h>
#include <unistd.h>
LinuxNIC::LinuxNIC(/* args */)
{
}
LinuxNIC::~LinuxNIC()
{
}
INT32 LinuxNIC::InitSockets(std::string strIfName, BYTE *HwAddress)
{
    m_NicName = strIfName;
    INT32 psock;
    INT32 i;
    INT32 r, ifindex;
    SOCKET sndSocket, rcvSocket;
    struct timeval timeout;

    struct ifreq ifr;
    struct sockaddr_ll sll;
    psock = sndSocket = rcvSocket = socket(PF_PACKET, SOCK_RAW, htons(0x88a4));
    if (psock < 0)
    {
        EC_ERROR("Adapter_m::create raw socket failed:%s!", strerror(errno));
        return errno;
    }
    else
    {
        EC_INFO("Adapter_m::create raw socket success:%d", psock);
    }
    strcpy(ifr.ifr_name, m_NicName.c_str());
    r = ioctl(psock, SIOCGIFINDEX, &ifr);
    if (r < 0)
    {
        EC_ERROR("Adapter_m::getting socket index failed:%d", strerror(errno));
        close(sndSocket);
        return r;
    }
    bool isUnderRtnet = false;
    if (!strncmp(m_NicName.c_str(), "rt", 2))
    {
        isUnderRtnet = true;
    }
    if (isUnderRtnet != true)
    {
#ifdef RPIW5500
        timeout.tv_sec = 0;
        timeout.tv_usec = 100; // TODO: DEPENDS ON PERFORMANCE OF NetIF
#else
        timeout.tv_sec = 0;
        timeout.tv_usec = 50; // TODO: DEPENDS ON PERFORMANCE OF NetIF
#endif
        r = setsockopt(psock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        r = setsockopt(psock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
        i = 1;
        r = setsockopt(psock, SOL_SOCKET, SO_DONTROUTE, &i, sizeof(i));
        INT32 sizeofpool = 4096;
        r = setsockopt(psock, SOL_SOCKET, SO_RCVBUF, &sizeofpool, sizeof(sizeofpool));
        if (r < 0)
        {
            EC_ERROR("Adapter_m::setting socket recvbuff failed:%d,%s", r, strerror(-r));
        }
        r = setsockopt(psock, SOL_SOCKET, SO_SNDBUF, &sizeofpool, sizeof(sizeofpool));
        if (r < 0)
        {
            EC_ERROR("Adapter_m::setting socket recvbuff failed::%d,%s", r, strerror(-r));
        }
    }
    ifindex = ifr.ifr_ifindex;
    strcpy(ifr.ifr_name, m_NicName.c_str());
    ifr.ifr_flags = 0;
    /* reset flags of NIC interface */
    r = ioctl(psock, SIOCGIFFLAGS, &ifr);
    /* set flags of NIC interface, here promiscuous and broadcast */
    ifr.ifr_flags = ifr.ifr_flags || IFF_PROMISC || IFF_BROADCAST;
    r = ioctl(psock, SIOCGIFFLAGS, &ifr);
    /* bind socket to protocol, in this case RAW EtherCAT */
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = ifindex;
    sll.sll_protocol = htons(0x88a4);
    r = bind(psock, (struct sockaddr *)&sll, sizeof(sll));
    if (r >= 0)
    {
        m_rcvSocket = rcvSocket;
        m_sndSocket = sndSocket;
    }
    else
    {
        close(sndSocket);
    }
    r = ioctl(psock, SIOCGIFHWADDR, &ifr);
    if (!r)
    {
        char *hw = ifr.ifr_hwaddr.sa_data;
#ifdef XENOMAI2 // TI-CPSW RTnet uses random mac id, try to correct it
        hw[0] = 0x00;
        hw[1] = 0x12;
        hw[2] = 0x37;
#endif
        memcpy(HwAddress, hw, 6);

        EC_INFO("Adapter_m::opening ethernet interface %s with MAC id: %x:%x:%x:%x:%x:%x.", strIfName.c_str(), (INT32)hw[0], (INT32)hw[1], (INT32)hw[2], (INT32)hw[3], (INT32)hw[4], (INT32)hw[5]);
    }
    return r;
}

INT32 LinuxNIC::Open(const char *NicName, BYTE *HwAddress)
{
    return InitSockets(NicName, HwAddress);
}
INT32 LinuxNIC::Close()
{
    close(m_sndSocket);
    m_sndSocket = -1;
    return 0;
}
INT32 LinuxNIC::SendFrame(void *pData, INT32 Len, INT32 Flag)
{
    return send(m_sndSocket, (char *)pData, Len, Flag);
}
INT32 LinuxNIC::RecvFrame(void *pData, INT32 Len, INT32 Flag)
{
    return recv(m_rcvSocket, pData, Len, Flag);
}

LinuxThread::LinuxThread()
    : NeedJoinFlag(false)
{
}
LinuxThread::~LinuxThread()
{
    if (NeedJoinFlag)
        Join();
}
INT32 LinuxThread::Start(void (*Task)(void *), INT32 Priority, void *Context, const char *ThreadName)
{
    m_Task = Task;
    m_Priority = Priority;
    m_ThreadName = ThreadName;
    m_Context = Context;
    m_TaskHandle = std::thread{Task, m_Context};
    NeedJoinFlag = true;
    return 0;
}
INT32 LinuxThread::Join()
{
    m_TaskHandle.join();
    NeedJoinFlag = false;
    return 0;
}

LinuxMutex::LinuxMutex()
{
}
LinuxMutex::~LinuxMutex()
{
}
void LinuxMutex::Lock()
{
    m_csCrit.lock();
}
void LinuxMutex::UnLock()
{
    m_csCrit.unlock();
}

LinuxSem::LinuxSem(const char *SemName)
{
    if (sem_init(&m_SemHandle, 0, 0))
    {
        EC_ERROR("Sem initilized failed");
    }
}
LinuxSem::~LinuxSem()
{
    if (sem_destroy(&m_SemHandle))
    {
        EC_ERROR("Sem delete failed");
    }
}
VOID LinuxSem::Pend()
{
    if (sem_wait(&m_SemHandle))
    {
        EC_ERROR("Sem wait failed");
    }
}
VOID LinuxSem::Signal()
{
    if (sem_post(&m_SemHandle))
    {
        EC_ERROR("Sem post failed");
    }
}
VOID LinuxSem::BroadCast()
{
    // if (sem_broadcast_np(&m_SemHandle))
    // {
    //     EC_ERROR("Sem wait failed");
    // }
}
LinuxHeap::LinuxHeap(const char *name, INT32 size)
    : HeapName(name), HeapSize(size)
{
}
LinuxHeap::~LinuxHeap()
{
}
VOID *LinuxHeap::Alloc(INT32 Size)
{
    return malloc(Size);
}
VOID LinuxHeap::Free(void *pData)
{
    free(pData);
}

LinuxTime::LinuxTime()
{

    timespec Time;
    clock_gettime(CLOCK_MONOTONIC, &Time);
    StartTime = Time.tv_sec * 1000 + Time.tv_nsec / 1000000;
}
LinuxTime::~LinuxTime()
{
}
UINT64 LinuxTime::GetTime()
{
    timespec Time;
    clock_gettime(CLOCK_MONOTONIC, &Time);
    return Time.tv_sec * 1000000000 + Time.tv_nsec - StartTime;
}
UINT64 LinuxTime::TaskSleepUntill(UINT64 TargetTime)
{
    usleep((TargetTime - StartTime) / 1000);
}
void LinuxTime::TaskSleep(UINT64 Time)
{
    usleep(Time / 1000);
}
LinuxIPC::LinuxIPC(std::string EndPortDefine, UINT32 PortNum, UINT32 TargetPortNum)
    : EndPortDescritption(EndPortDefine), EndPortNum(PortNum), TargetEndPortNum(TargetPortNum)
{
}

INT32 LinuxIPC::EndPortInit()
{
    return 0;
}
INT32 LinuxIPC::EndPortClose()
{
    return 0;
}
INT32 LinuxIPC::EndPortSend(VOID * pData, UINT32 len, INT32 Flag)
{
    return 0;
}
INT32 LinuxIPC::EndPortRecv(VOID * RecvBuffer, UINT32 RecvLen, INT32 Flag)
{
    return 0;
}
LinuxIPC::~LinuxIPC()
{
}