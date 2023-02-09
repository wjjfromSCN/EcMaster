#pragma once
#ifndef W550OSALACHIEVE
#define W550OSALACHIEVE
#include "../Xenomai/XenomaiOS.h"


#include <string>
class W550NIC : public McNicInterface
{
private:
    typedef INT32 SOCKET;
    std::string m_NicName;
    INT32 InitSockets(std::string strIfName, BYTE *HwAddress);

public:
    XenomaiNIC(/* args */);
    ~XenomaiNIC();
    virtual INT32 Open(const char *NicName, BYTE *HwAddress) override;
    virtual INT32 Close() override;
    virtual INT32 SendFrame(void *pData, INT32 Len, INT32 Flag) override;
    virtual INT32 RecvFrame(void *pData, INT32 Len, INT32 Flag) override;
};


#endif
