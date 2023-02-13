#pragma once 
#ifndef OSAL_LOG_H
#define OSAL_LOG_H

#ifdef LINUX
#include "Linux/LinuxLog.h"
#endif
#ifdef W550
#include "Xenomai/XenomaiLog.h"
#endif
#ifdef XENOMAI
#include "Xenomai/XenomaiLog.h"
#endif
#ifdef INTIME
#include "INtime/INtimeWrapper.h"
#endif
#define __DEBUG //日志模块总开关，注释掉将关闭日志输出

#ifdef __DEBUG
#define DEBUG(format, ...) EcPrintf(format, ##__VA_ARGS__)
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
#endif