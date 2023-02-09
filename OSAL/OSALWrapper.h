#ifndef OSAL_WRAPPER_H
#define OSAL_WRAPPER_H
#ifdef LINUX
#include "Linux/LinuxWrapper.h"
#endif
#ifdef W550
#include "W550/W550Wrapper.h"
#endif
#ifdef XENOMAI
#include "Xenomai/XenomaiWrapper.h"
#endif
#ifdef XENOMAI
#include "INtime/INtimeWrapper.h"
#endif
#endif