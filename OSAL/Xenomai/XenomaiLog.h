#pragma once 
#ifndef OSAL_XENOMAI_LOG_H
#define OSAL_XENOMAI_LOG_H

template <typename... Args>
inline void EcPrintf(const Args &...args)
{
    rt_printf(args...);
}

#endif