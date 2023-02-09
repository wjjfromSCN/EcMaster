#include"W550OS.h"
#include"wiznet_drv.h"
INT32 W550NIC::InitSockets(std::string strIfName, BYTE *HwAddress)
{
	m_strIfName = strIfName;
	INT32 rVal;
	const uint16_t priMAC[3] = {0x0101, 0x0101, 0x0101};
	wiznet_hw_config(8, 0, 0);
	rVal = wiznet_macraw_init((uint8_t *)priMAC);
	if (rVal != 0)
		return 1;
	EC_INFO("CEcRawScoketDevice::using ethernet controller W5500 ,ret:{}\n", rVal);
	m_rcvSocket = m_sndSocket = 1;
}
W550NIC::W550NIC(/* args */)
{
}
W550NIC::~W550NIC()
{
	if (m_sndSocket == -1)
		Close();
}
INT32 W550NIC::Open(const char *NicName, BYTE *HwAddress)
{
	return InitSockets(NicName, HwAddress);
}
INT32 W550NIC::Close()
{
	close(m_sndSocket);
	m_sndSocket = -1;
	return 0;
}
INT32 XenomaiNIC::SendFrame(void *pData, INT32 Len, INT32 Flag)
{
	return 	 wiznet_macraw_send((char *)pData, Len);
}
INT32 XenomaiNIC::RecvFrame(void *pData, INT32 Len, INT32 Flag)
{
	return wiznet_macraw_recv(m_RecvBuff, co_cbUdpBuf);
}