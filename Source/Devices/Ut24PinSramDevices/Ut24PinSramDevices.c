/***************************************************************************/
/*  Copyright (C) 2008-2020 Kevin Eshbach                                  */
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

static VOID UTPEPDEVICESAPI l6116WriteDevice(const TDeviceIOFuncs* pDeviceIOFuncs, const LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI l6116VerifyDevice(const TDeviceIOFuncs* pDeviceIOFuncs, const LPBYTE pbyData, ULONG ulDataLen);

static LPCWSTR l_psz4118PinNames[] = {
    CDevicePin_Address7,
    CDevicePin_Address6,
    CDevicePin_Address5,
    CDevicePin_Address4,
    CDevicePin_Address3,
    CDevicePin_Address2,
    CDevicePin_Address1,
    CDevicePin_Address0,
    CDevicePin_Data0,
    CDevicePin_Data1,
    CDevicePin_Data2,
    CDevicePin_Ground,
    CDevicePin_Data3,
    CDevicePin_Data4,
    CDevicePin_Data5,
    CDevicePin_Data6,
    CDevicePin_Data7,
    CDevicePin_LowChipEnable1,
    CDevicePin_NotUsed,
    CDevicePin_OutputEnable,
    CDevicePin_WriteEnable,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Vcc};

static LPCWSTR l_psz6116PinNames[] = {
    CDevicePin_Address7,
    CDevicePin_Address6,
    CDevicePin_Address5,
    CDevicePin_Address4,
    CDevicePin_Address3,
    CDevicePin_Address2,
    CDevicePin_Address1,
    CDevicePin_Address0,
    CDevicePin_Data0,
    CDevicePin_Data1,
    CDevicePin_Data2,
    CDevicePin_Ground,
    CDevicePin_Data3,
    CDevicePin_Data4,
    CDevicePin_Data5,
    CDevicePin_Data6,
    CDevicePin_Data7,
    CDevicePin_LowChipEnable,
    CDevicePin_Address10,
    CDevicePin_OutputEnable,
    CDevicePin_WriteEnable,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Vcc};

DEVICES_BEGIN
    DEVICE_DATA_BEGIN(MK4118)
        DEVICE_SRAM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz4118PinNames)
        DEVICE_ADAPTER("Part #6116")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_RAM_DATA_BEGIN
            DEVICE_RAM_SIZE(0x0400)
            DEVICE_RAM_BITSPERVALUE(8)
            DEVICE_RAM_WRITE_FUNC(l6116WriteDevice)
            DEVICE_RAM_VERIFY_FUNC(l6116VerifyDevice)
        DEVICE_RAM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(6116)
        DEVICE_SRAM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz6116PinNames)
        DEVICE_ADAPTER("Part #6116")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_RAM_DATA_BEGIN
            DEVICE_RAM_SIZE(0x0800)
            DEVICE_RAM_BITSPERVALUE(8)
            DEVICE_RAM_WRITE_FUNC(l6116WriteDevice)
            DEVICE_RAM_VERIFY_FUNC(l6116VerifyDevice)
        DEVICE_RAM_DATA_END
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

/*
   6116 Device Functions
*/

static VOID UTPEPDEVICESAPI l6116WriteDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
    pDeviceIOFuncs;
    pbyData;
    ulDataLen;
}

static VOID UTPEPDEVICESAPI l6116VerifyDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
    pDeviceIOFuncs;
    pbyData;
    ulDataLen;
}

/***************************************************************************/
/*  Copyright (C) 2008-2020 Kevin Eshbach                                  */
/***************************************************************************/
