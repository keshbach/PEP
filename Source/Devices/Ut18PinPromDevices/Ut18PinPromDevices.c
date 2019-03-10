/***************************************************************************/
/*  Copyright (C) 2007-2009 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>
#include <strsafe.h>

#include <UtilsDevice/UtPepDevices.h>

#include <Devices/Includes/UtDevicePinDefs.h>

#include <Config/UtPepCtrl.h>

#include <Includes/UtMacros.h>

#include <Utils/UtHeap.h>

#include <Devices/Includes/UtGenericFuncs.inl>

static LPCWSTR l_psz74S573PinNames[] = {
    CDevicePin_Address6,
    CDevicePin_Address5,
    CDevicePin_Address4,
    CDevicePin_Address3,
    CDevicePin_Address0,
    CDevicePin_Address1,
    CDevicePin_Address2,
    CDevicePin_LowChipEnable1,
    CDevicePin_Ground,
    CDevicePin_LowChipEnable2,
    CDevicePin_Data3,
    CDevicePin_Data2,
    CDevicePin_Data1,
    CDevicePin_Data0,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Address7,
    CDevicePin_Vcc};

static LPCWSTR l_psz82S125PinNames[] = {
    CDevicePin_Address6,
    CDevicePin_Address5,
    CDevicePin_Address4,
    CDevicePin_Address3,
    CDevicePin_Address0,
    CDevicePin_Address1,
    CDevicePin_Address2,
    CDevicePin_Address10,
    CDevicePin_Ground,
    CDevicePin_LowChipEnable,
    CDevicePin_Data3,
    CDevicePin_Data2,
    CDevicePin_Data1,
    CDevicePin_Data0,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Address7,
    CDevicePin_Vcc};

DEVICES_BEGIN
    DEVICE_DATA_BEGIN(74S573)
        DEVICE_PROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(18)
        DEVICE_PINNAMES(l_psz74S573PinNames)
        DEVICE_ADAPTER("Part# KE74S573/82S125")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
        DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0400)
            DEVICE_ROM_BITSPERVALUE(4)
            DEVICE_ROM_VPP_NOTSUPPORTED
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC_NONE
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(82S125)
        DEVICE_PROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(18)
        DEVICE_PINNAMES(l_psz82S125PinNames)
        DEVICE_ADAPTER("Part# KE74S573/82S125")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
        DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0800)
            DEVICE_ROM_BITSPERVALUE(4)
            DEVICE_ROM_VPP_NOTSUPPORTED
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC_NONE
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
DEVICES_END

/***************************************************************************/
/*  Copyright (C) 2007-2009 Kevin Eshbach                                  */
/***************************************************************************/
