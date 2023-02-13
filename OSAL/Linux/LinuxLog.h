#pragma once
#ifndef OSAL_LINUX_LOG_H
#define OSAL_LINUX_LOG_H
#include <stdio.h>
template <typename... Args>
inline void EcPrintf(const Args &...args)
{
    printf(args...);
}

#endif