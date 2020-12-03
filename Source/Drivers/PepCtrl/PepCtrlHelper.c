/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/

#include <Includes/UtCompiler.h>

#include <ntddk.h>

#include "PepCtrlHelper.h"
#include "PepCtrlPortData.h"

#include <Drivers/PepCtrlIOCTL.h>

#if !defined(NDEBUG)

#if defined(ALLOC_PRAGMA)
#pragma alloc_text (PAGE, PepCtrlHelperTranslateControlCode)
#pragma alloc_text (PAGE, PepCtrlHelperTranslateState)
#endif

_IRQL_requires_max_(PASSIVE_LEVEL)
PCSTR PepCtrlHelperTranslateControlCode(
  _In_ ULONG ulIOControlCode)
{
	PAGED_CODE()

	switch (ulIOControlCode)
	{
		case IOCTL_PEPCTRL_SET_PROGRAMMER_MODE:
			return "Set Programmer Mode";
		case IOCTL_PEPCTRL_SET_VCC_MODE:
			return "Set VCC Mode";
		case IOCTL_PEPCTRL_SET_PIN_PULSE_MODE:
			return "Set Pin Pulse Mode";
		case IOCTL_PEPCTRL_SET_VPP_MODE:
			return "Set VPP Mode";
		case IOCTL_PEPCTRL_SET_ADDRESS:
			return "Set Address";
		case IOCTL_PEPCTRL_SET_ADDRESS_WITH_DELAY:
			return "Set Address with Delay";
		case IOCTL_PEPCTRL_GET_DATA:
			return "Get Data";
		case IOCTL_PEPCTRL_SET_DATA:
			return "Set Data";
		case IOCTL_PEPCTRL_TRIGGER_PROGRAM:
			return "Trigger Program";
		case IOCTL_PEPCTRL_SET_OUTPUT_ENABLE:
			return "Set Output Enable";
		case IOCTL_PEPCTRL_GET_DEVICE_STATUS:
			return "Get Device Status";
		case IOCTL_PEPCTRL_DEVICE_NOTIFICATION:
			return "Device Notification";
		case IOCTL_PEPCTRL_GET_PORT_SETTINGS:
			return "Get Port Settings";
		case IOCTL_PEPCTRL_SET_PORT_SETTINGS:
			return "Set Port Settings";
		case IOCTL_PEPCTRL_SET_DELAY_SETTINGS:
			return "Set Delay Settings";
	}

	return "** Unknown Control Code **";
}

_IRQL_requires_max_(PASSIVE_LEVEL)
PCSTR PepCtrlHelperTranslateState(
  _In_ INT32 nState)
{
	PAGED_CODE()

	switch (nState)
	{
		case CPepCtrlStateRunning:
			return "Running";
		case CPepCtrlStateDeviceControl:
			return "Device Control";
		case CPepCtrlStateUnloading:
			return "Unloading";
		case CPepCtrlStateChangePortSettings:
			return "Change Port Settings";
		case CPepCtrlStateDeviceArrived:
			return "Device Arrived";
		case CPepCtrlStateDeviceRemoved:
			return "Device Removed";
	}

	return "** Unknown State **";
}

#endif

/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/
