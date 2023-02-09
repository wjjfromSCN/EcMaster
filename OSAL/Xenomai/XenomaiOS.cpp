#include "XenomaiOS.h"
#include <rtdm/ipc.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netpacket/packet.h>
#include <iostream>
#include "rtnet.h"
INT32 XenomaiNIC::InitSockets(std::string strIfName, BYTE *HwAddress)
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
	if (isUnderRtnet == true)
	{
		EC_INFO("using rtnet nic driver");
		timeout.tv_sec = 0;
		timeout.tv_usec = 10; // TODO: DEPENDS ON PERFORMANCE OF SLAVES
							  //#if (defined RTNET_RTIOC_TIMEOUT) && (defined RTNET_RTIOC_EXTPOOL)
		r = ioctl(psock, RTNET_RTIOC_TIMEOUT, &timeout);
		if (r < 0)
		{
			EC_ERROR("Adapter_m::setting socket timeout failed:%d, %s", r, strerror(-r));
		}
		INT32 sizeofpool = 4096;
		r = ioctl(psock, RTNET_RTIOC_EXTPOOL, &sizeofpool);
		if (r < 0)
		{
			EC_ERROR("Adapter_m::setting socket timeout failed:%d, %s", r, strerror(-r));
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

		
	}
	return r;
}
XenomaiNIC::XenomaiNIC(/* args */)
{
}
XenomaiNIC::~XenomaiNIC()
{
	if (m_sndSocket != -1)
		Close();
}
INT32 XenomaiNIC::Open(const char *NicName, BYTE *HwAddress)
{
	return InitSockets(NicName, HwAddress);
}
INT32 XenomaiNIC::Close()
{
	close(m_sndSocket);
	m_sndSocket = -1;
	return 0;
}
INT32 XenomaiNIC::SendFrame(void *pData, INT32 Len, INT32 Flag)
{
	
	return send(m_sndSocket, (char *)pData, Len, Flag);
}
INT32 XenomaiNIC::RecvFrame(void *pData, INT32 Len, INT32 Flag)
{
	return recv(m_rcvSocket, pData, Len, Flag);
}

XenomaiThread::XenomaiThread()
	: NeedJoinFlag(false)
{
}
XenomaiThread::~XenomaiThread()
{

	Join();
}
INT32 XenomaiThread::Start(void (*Task)(void *), INT32 Priority, void *Context, const char *ThreadName)
{
	m_Task = Task;
	m_Priority = Priority;
	m_ThreadName = ThreadName;
	m_Context = Context;
	INT32 err = rt_task_create(&m_TaskHandle, m_ThreadName.c_str(), 8192, m_Priority, T_JOINABLE );
	if (err)
		return err + 1000;
	else
	{
		cpu_set_t cpus;
		CPU_ZERO(&cpus);
		CPU_SET(1, &cpus); //将线程限制在指定的cpu2上运行
		CPU_SET(2, &cpus); //将线程限制在指定的cpu2上运行
		CPU_SET(3, &cpus); //将线程限制在指定的cpu2上运行
		if (rt_task_set_affinity(&m_TaskHandle, &cpus))
		{
			EC_WARN("Set Task %s affinity error.", m_ThreadName.c_str());
		}
		err = rt_task_start(&m_TaskHandle, m_Task, m_Context);
		if (err)
			return err + 2000;
	}
}
INT32 XenomaiThread::Join()
{
	if (!NeedJoinFlag) return 1;
	INT32 nErr = rt_task_unblock(&m_TaskHandle);
	nErr = rt_task_join(&m_TaskHandle);
	nErr = rt_task_delete(&m_TaskHandle);
	NeedJoinFlag = false;
	return nErr;
}
VOID XenomaiThread::PrintInfo()
{
	RT_TASK_INFO Info;
	if(rt_task_inquire(&m_TaskHandle,&Info)==0)
	{
		EC_INFO("Task name %s prio :%d.\n\t \
					stat %u.",m_ThreadName.c_str(),Info.stat.status);

	}
	else 
	{
		EC_WARN("Inquire xenomai task %s fail!",m_ThreadName.c_str());
	}


}

XenomaiMutex::XenomaiMutex()
{
	rt_mutex_create(&m_csCrit, nullptr);
}
XenomaiMutex::~XenomaiMutex()
{
	rt_mutex_release(&m_csCrit);
	rt_mutex_delete(&m_csCrit);
}
void XenomaiMutex::Lock()
{
	rt_mutex_acquire(&m_csCrit, TM_INFINITE);
}
void XenomaiMutex::UnLock()
{
	rt_mutex_release(&m_csCrit);
}

XenomaiSem::XenomaiSem(const char *SemName)
{
	if (rt_sem_create(&m_SemHandle, SemName, 0, 0))
	{
		EC_ERROR("Sem initilized failed");
	}
}
XenomaiSem::~XenomaiSem()
{
	if (rt_sem_delete(&m_SemHandle))
	{
		EC_ERROR("Sem delete failed");
	}
}
VOID XenomaiSem::Pend()
{
	INT32 ret=rt_sem_p(&m_SemHandle,TM_INFINITE);
	if (ret)
	{
		EC_ERROR("Sem wait failed,error code %d.",ret);
	}
}
VOID XenomaiSem::Signal()
{
	INT32 ret=rt_sem_v(&m_SemHandle);
	if (ret)
	{
		EC_ERROR("Sem post failed,error code %d.",ret);
	}
}
VOID XenomaiSem::BroadCast()
{
	if (rt_sem_broadcast(&m_SemHandle))
	{
		EC_ERROR("Sem broadcast failed");
	}
}
XenomaiHeap::XenomaiHeap(const char *name, INT32 size)
{

	if (rt_heap_create(&heapPtr, name, size, H_PRIO))
	{
		EC_ERROR("Xenomai Heap create error!.");
	}
}
XenomaiHeap::~XenomaiHeap()
{
	RT_HEAP_INFO HeapInfo;
	if (rt_heap_inquire(&heapPtr, &HeapInfo))
		EC_ERROR("Unvaild heap descriptor.\n");
	else
	{
		EC_TRACE("Number of tasks waiting for available memory: %d.\n", HeapInfo.nwaiters);
		EC_TRACE("Creation mode flags as given to create :%d.\n", HeapInfo.mode);
		EC_TRACE("Heap Size: %d,used %d ,free %d.\n", HeapInfo.heapsize, HeapInfo.usedmem, HeapInfo.usablemem);
		EC_TRACE("heap name %s.\n", HeapInfo.name);
	}
	if (rt_heap_delete(&heapPtr))
	{
		EC_ERROR("Xenomai Heap delete error!");
	}
	else
	{
		EC_INFO("Xenomai Heap delete successfully!");
	}
}
VOID *XenomaiHeap::Alloc(INT32 Size)
{
	void *ptr;
	INT32 err = rt_heap_alloc(&heapPtr, Size, TM_INFINITE, &ptr);
	if (err)
	{
		EC_ERROR("Xenomai Heap alloc error!code : %d.", err);
		ptr = nullptr;
	}
	return ptr;
}
VOID XenomaiHeap::Free(void *pData)
{
	if (rt_heap_free(&heapPtr, pData))
	{
		EC_ERROR("Xenomai Heap free error!");
	}
}

XenomaiTime::XenomaiTime()
{
}
XenomaiTime::~XenomaiTime()
{
}
UINT64 XenomaiTime::GetTime()
{
	UINT64 Res = rt_timer_ticks2ns(rt_timer_read());
	return Res;
}
UINT64 XenomaiTime::TaskSleepUntill(UINT64 TargetTime)
{
	return rt_task_sleep_until(rt_timer_ns2ticks(TargetTime));
}
void XenomaiTime::TaskSleep(UINT64 Time)
{
	
	rt_task_sleep(rt_timer_ns2ticks(Time));
}
XenomaiIPC::XenomaiIPC(std::string EndPortDefine, UINT32 PortNum, UINT32 TargetPortNum)
	: EndPortDescritption(EndPortDefine), EndPortNum(PortNum),
	  IDDPPortSocket(INVALID_SOCKET_DESCRIPT), TargetEndPortNum(TargetPortNum), CloseFlag(true)
{
	EndPortInit();
}

INT32 XenomaiIPC::EndPortInit()
{
	if (IDDPPortSocket != INVALID_SOCKET_DESCRIPT || CloseFlag != true)
		return -1;
	struct sockaddr_ipc PortAddr;
	INT32 ret, s, len;
	s = socket(AF_RTIPC, SOCK_DGRAM, IPCPROTO_IDDP);
	if (s < 0)
		EC_TRACE("%d::Master IDDP Send Socket cteate error!error code :%d.", EndPortDescritption, s);
	size_t poolsize = 32768;
	ret = setsockopt(s, SOL_IDDP, IDDP_POOLSZ, &poolsize, sizeof(poolsize));
	if (ret)
		EC_TRACE("%d::IDDP Kernel set pool size fail.code:%d.", EndPortDescritption, errno);
	PortAddr.sipc_family = AF_RTIPC;
	PortAddr.sipc_port = EndPortNum;
	ret = bind(s, (struct sockaddr *)&PortAddr, sizeof(PortAddr));
	if (ret)
		EC_TRACE("%d::Master IDDP Send Socket bind error!error code :%d.", EndPortDescritption, ret);
	if (s > 0)
	{

		IDDPPortSocket = s;
		EC_TRACE("%d::IDDP create succefully,this port:%d,target port:%d.!", IDDPPortSocket, EndPortNum, TargetEndPortNum);
		CloseFlag = false;
		return 0;
	}
	return -1;
}
INT32 XenomaiIPC::EndPortClose()
{
	close(EndPortNum);
	IDDPPortSocket = INVALID_SOCKET_DESCRIPT;
	CloseFlag = true;
	return 0;
}
INT32 XenomaiIPC::EndPortSend(VOID * pData, UINT32 len, INT32 Flag)
{
	INT32 ret = -1;
	if (IDDPPortSocket == INVALID_SOCKET_DESCRIPT || CloseFlag == true)
		return -1;
	struct sockaddr_ipc TargetAddr;
	TargetAddr.sipc_family = AF_RTIPC;
	TargetAddr.sipc_port = TargetEndPortNum;
	ret = sendto(IDDPPortSocket, pData, len, Flag, (struct sockaddr *)&TargetAddr, sizeof(TargetAddr));
	return ret;
}
INT32 XenomaiIPC::EndPortRecv(VOID * RecvBuffer, UINT32 RecvLen, INT32 Flag)
{
	INT32 ret = -1;
	if (IDDPPortSocket == INVALID_SOCKET_DESCRIPT || CloseFlag == true)
		return -1;
	struct sockaddr_ipc RecvAddr;
	socklen_t addlen = sizeof(RecvAddr);
	ret = recvfrom(IDDPPortSocket, RecvBuffer, RecvLen, Flag, (struct sockaddr *)&RecvAddr, &addlen);
	return ret;
}
XenomaiIPC::~XenomaiIPC()
{
	if (IDDPPortSocket != INVALID_SOCKET_DESCRIPT && CloseFlag == false)
		EndPortClose();
}