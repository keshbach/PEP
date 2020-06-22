/***************************************************************************/
/*  Copyright (C) 2007-2020 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>
#include <strsafe.h>

#include <UtilsDevice/UtPepDevices.h>

#include <Devices/Includes/UtDevicePinDefs.h>

#include <Config/UtPepCtrl.h>

#include <Includes/UtMacros.h>

#include <Utils/UtHeap.h>

static BOOL UTPEPDEVICESAPI lGenericInit(VOID);
static BOOL UTPEPDEVICESAPI lGenericUninit(VOID);

static LPCWSTR l_pszPinNames[] = {
    L"Prog/~CE",
    CDevicePin_HighChipEnable2,
    CDevicePin_Clock,
    L"RESET",
    CDevicePin_Vdd,
    L"READY",
    L"IO/~M",
    L"~IOR",
    L"~RD",
    L"~IOW",
    L"ALE",
    L"AD0",
    L"AD1",
    L"AD2",
    L"AD3",
    L"AD4",
    L"AD5",
    L"AD6",
    L"AD7",
    CDevicePin_Ground,
    CDevicePin_Address8,
    CDevicePin_Address9,
    CDevicePin_Address10,
    L"PA0",
    L"PA1",
    L"PA2",
    L"PA3",
    L"PA4",
    L"PA5",
    L"PA6",
    L"PA7",
    L"PB0",
    L"PB1",
    L"PB2",
    L"PB3",
    L"PB4",
    L"PB5",
    L"PB6",
    L"PB7",
    CDevicePin_Vcc};

DEVICES_BEGIN
    DEVICE_DATA_BEGIN(8755A)
        DEVICE_EPROMWithIO
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(40)
        DEVICE_PINNAMES(l_pszPinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_NOTSUPPORTED
            DEVICE_ROM_READ_FUNC(NULL)
            DEVICE_ROM_PROGRAM_FUNC_NONE
            DEVICE_ROM_VERIFY_FUNC(NULL)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
DEVICES_END

static BOOL UTPEPDEVICESAPI lGenericInit(VOID)
{
    return UtInitHeap() ? TRUE : FALSE;
}

static BOOL UTPEPDEVICESAPI lGenericUninit(VOID)
{
    return UtUninitHeap() ? TRUE : FALSE;
}

/***************************************************************************/
/*  Copyright (C) 2007-2020 Kevin Eshbach                                  */
/***************************************************************************/
