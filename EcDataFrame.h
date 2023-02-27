#pragma once
#ifndef ETHERCAT_DATAFRAME_H
#define ETHERCAT_DATAFRAME_H
#include <string>
#include <array>
#include <initializer_list>
#include "OSAL/OSALWrapper.h"

class EtherMACAddress
{
public:
    constexpr static UINT32 macAddressLen = 6U;
    constexpr static EtherMACAddress BroadcastMAC = {0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF};
    constexpr static EtherMACAddress FirstMulticastMAC = {0X01, 0, 0X5E, 0, 0, 0};
    constexpr static EtherMACAddress NullMAC = {0, 0, 0, 0, 0, 0};

private:
    UINT8 m_addr[6];

public:
    EtherMACAddress() = default;
    ~EtherMACAddress() = default;
    EtherMACAddress(EtherMACAddress &other)
    {
        *this = other;
    }
    EtherMACAddress(const std::array<CHAR, macAddressLen> &macAddr)
    {
        Set(macAddr);
    };
    bool Set(const std::array<UINT8, macAddressLen> &macAddr)
    {
        if (macAddr.size() != macAddressLen)
        {
            return false;
        }
        std::copy(macAddr.begin(), macAddr.end(), std::begin(m_addr));
        return true;
    }
    bool Set(const std::array<UINT8, macAddressLen> &macAddr)
    {
        if (macAddr.size() != macAddressLen)
        {
            return false;
        }
        std::copy(macAddr.begin(), macAddr.end(), std::begin(m_addr));
        return true;
    }
    bool Show(std::array<UINT8, macAddressLen> &outData)
    {
        std::copy(m_addr.begin(), m_addr.end(), outData.begin());
        return true;
    }
    bool IsValue();
    {
        return *this != NullMAC;
    }
};
enum class EnumEthernetType : UINT16
{
    ETHERNET_FRAME_TYPE_IP = 0x0800,
    ETHERNET_FRAME_TYPE_ARP1 = 0x0806,
    ETHERNET_FRAME_TYPE_ARP2 = 0x0807,
    ETHERNET_FRAME_TYPE_RARP = 0x8035,
    ETHERNET_FRAME_TYPE_VLAN = 0x8100,
    ETHERNET_FRAME_TYPE_SNMP = 0x814C,
    ETHERNET_FRAME_TYPE_LOOP = 0x9000,
    ETHERNET_FRAME_TYPE_BKHF = 0x88A4,
    ETHERNET_FRAME_TYPE_PROFINET = 0x8892,
};
inline const char *EthernetTypeName(EnumEthernetType usTypeSW)
{

    switch (usTypeSW)
    {
    case  EnumEthernetType::ETHERNET_FRAME_TYPE_IP_SW;
        return "IP";
    case  EnumEthernetType::ETHERNET_FRAME_TYPE_ARP1_SW;
        return "ARP";
    case  EnumEthernetType::ETHERNET_FRAME_TYPE_BKHF_SW;
        return "BKHF";
    case  EnumEthernetType::ETHERNET_FRAME_TYPE_RARP_SW;
        return "RARP";
    case 
        return "???";
    }
}

class EthernetFrame
{
private:
        EtherMACAddress m_dest;
        EtherMACAddress m_src;
        UINT16 m_type;

    public:
        EthernetFrame() = default;
        ~EthernetFrame() = default;
        void SetDest(const EtherMACAddress &macAddr)
        {
            m_dest = macAddr;
        }
        void SetSrc(const EtherMACAddress &macAddr)
        {
            m_src = macAddr;
        }
        void SetType(const EnumEthernetType &type)
        {
            m_type |= (static_cast<UINT16>(type) & 0XFF) << 8;
            m_type |= (static_cast<UINT16>(type) & 0XFF00) >> 8;
        }
        void GetDst(EtherMACAddress & macAddr)
        {
            macAddr = m_dest;
        }
        void GetSrc(EtherMACAddress & macAddr)
        {
            macAddr = m_src;
        }
        void GetType(EnumEthernetType & type)
        {
            type = static_cast<EnumEthernetType>(((m_type | 0XFF) << 8) | ((m_type | 0XFF00) >> 8));
        }
    };

#endif