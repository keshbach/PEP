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

#include <Devices/Includes/UtGenericFuncs.inl>

static VOID UTPEPDEVICESAPI l27020ProgramDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI l27040ProgramDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI l27080ProgramDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);

static LPCWSTR l_psz27C301PinNames[] = {
    CDevicePin_Vpp,
    CDevicePin_OutputEnable,
    CDevicePin_Address15,
    CDevicePin_Address12,
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
    CDevicePin_Address16,
    CDevicePin_Address11,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Address13,
    CDevicePin_Address14,
    CDevicePin_NotUsed,
    CDevicePin_LowProgram,
    CDevicePin_Vcc};

static LPCWSTR l_psz27C010PinNames[] = {
    CDevicePin_Vpp,
    CDevicePin_Address16,
    CDevicePin_Address15,
    CDevicePin_Address12,
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
    CDevicePin_Address11,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Address13,
    CDevicePin_Address14,
    CDevicePin_NotUsed,
    CDevicePin_LowProgram,
    CDevicePin_Vcc};

static LPCWSTR l_psz27C020PinNames[] = {
    CDevicePin_Vpp,
    CDevicePin_Address16,
    CDevicePin_Address15,
    CDevicePin_Address12,
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
    CDevicePin_Address11,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Address13,
    CDevicePin_Address14,
    CDevicePin_Address17,
    CDevicePin_LowProgram,
    CDevicePin_Vcc};

static LPCWSTR l_psz27C201PinNames[] = {
    CDevicePin_Vpp,
    CDevicePin_Address16,
    CDevicePin_Address15,
    CDevicePin_Address12,
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
    CDevicePin_Address11,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Address13,
    CDevicePin_Address14,
    CDevicePin_Address17,
    CDevicePin_NotUsed,
    CDevicePin_Vcc};

static LPCWSTR l_psz27C040PinNames[] = {
    CDevicePin_Vpp,
    CDevicePin_Address16,
    CDevicePin_Address15,
    CDevicePin_Address12,
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
    CDevicePin_Address11,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Address13,
    CDevicePin_Address14,
    CDevicePin_Address17,
    CDevicePin_Address18,
    CDevicePin_Vcc};

static LPCWSTR l_psz27C080PinNames[] = {
    CDevicePin_Address19,
    CDevicePin_Address16,
    CDevicePin_Address15,
    CDevicePin_Address12,
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
    CDevicePin_OutputEnable_Vpp,
    CDevicePin_Address11,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Address13,
    CDevicePin_Address14,
    CDevicePin_Address17,
    CDevicePin_Address18,
    CDevicePin_Vcc};

DEVICES_BEGIN
    DEVICE_DATA_BEGIN(27C301)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(32)
        DEVICE_PINNAMES(l_psz27C301PinNames)
        DEVICE_ADAPTER("Part# AMROM")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(200)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(200)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x20000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l27020ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C1000)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(32)
        DEVICE_PINNAMES(l_psz27C301PinNames)
        DEVICE_ADAPTER("Part# AMROM")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(200)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(70)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x20000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l27020ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(MSM271000)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(32)
        DEVICE_PINNAMES(l_psz27C010PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(200)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(70)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x20000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l27020ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(MX271000)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(32)
        DEVICE_PINNAMES(l_psz27C010PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(150)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(70)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x20000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l27020ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C010)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(32)
        DEVICE_PINNAMES(l_psz27C010PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(200)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(75)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x20000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l27020ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27HC010)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(32)
        DEVICE_PINNAMES(l_psz27C010PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(70)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(35)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l27020ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27LV010)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(32)
        DEVICE_PINNAMES(l_psz27C010PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(250)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(75)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l27020ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C1001)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(32)
        DEVICE_PINNAMES(l_psz27C010PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(200)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(75)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x20000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l27020ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C100)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(32)
        DEVICE_PINNAMES(l_psz27C301PinNames)
        DEVICE_ADAPTER("Part# AMROM")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(200)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(70)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x20000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l27020ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C101)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(32)
        DEVICE_PINNAMES(l_psz27C010PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(250)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(100)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x20000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l27020ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C020)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(32)
        DEVICE_PINNAMES(l_psz27C020PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(250)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(100)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x40000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l27020ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27LV020)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(32)
        DEVICE_PINNAMES(l_psz27C020PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(200)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(50)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x40000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l27020ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C201)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(32)
        DEVICE_PINNAMES(l_psz27C201PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(80)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(50)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x40000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l27020ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(MX27C2000)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(32)
        DEVICE_PINNAMES(l_psz27C020PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(150)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(65)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x40000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l27020ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C2001)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(32)
        DEVICE_PINNAMES(l_psz27C020PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(90)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(40)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x40000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l27020ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C040)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(32)
        DEVICE_PINNAMES(l_psz27C040PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(200)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(75)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x80000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l27040ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27LV040)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(32)
        DEVICE_PINNAMES(l_psz27C040PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(90)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(50)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x80000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l27040ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C401)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(32)
        DEVICE_PINNAMES(l_psz27C040PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(150)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(80)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x80000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l27040ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(MX27C4000)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(32)
        DEVICE_PINNAMES(l_psz27C040PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(150)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(65)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x80000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l27040ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C4001)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(32)
        DEVICE_PINNAMES(l_psz27C040PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(200)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(75)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x80000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l27040ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C080)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(32)
        DEVICE_PINNAMES(l_psz27C080PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(150)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(35)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x100000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l27080ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C801)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(32)
        DEVICE_PINNAMES(l_psz27C080PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(100)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(50)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x100000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l27080ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(MX27C80000)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(32)
        DEVICE_PINNAMES(l_psz27C080PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(150)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(65)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x100000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l27080ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C8001)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(32)
        DEVICE_PINNAMES(l_psz27C080PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(150)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(50)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x100000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l27080ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(57C1000)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(32)
        DEVICE_PINNAMES(l_psz27C010PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x20000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(NULL)
            DEVICE_ROM_PROGRAM_FUNC(NULL)
            DEVICE_ROM_VERIFY_FUNC(NULL)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(57C1001)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(32)
        DEVICE_PINNAMES(l_psz27C301PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x20000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(NULL)
            DEVICE_ROM_PROGRAM_FUNC(NULL)
            DEVICE_ROM_VERIFY_FUNC(NULL)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(57C4000)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(32)
        DEVICE_PINNAMES(l_psz27C040PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(150)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(70)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x80000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(NULL)
            DEVICE_ROM_PROGRAM_FUNC(NULL)
            DEVICE_ROM_VERIFY_FUNC(NULL)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
DEVICES_END

/*
   Generic Device Functions
*/

static VOID UTPEPDEVICESAPI l27020ProgramDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
    pDeviceIOFuncs;
	nChipEnableNanoseconds;
	nOutputEnableNanoseconds;
    pbyData;
    ulDataLen;
}

static VOID UTPEPDEVICESAPI l27040ProgramDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
    pDeviceIOFuncs;
	nChipEnableNanoseconds;
	nOutputEnableNanoseconds;
    pbyData;
    ulDataLen;
}

static VOID UTPEPDEVICESAPI l27080ProgramDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
    pDeviceIOFuncs;
	nChipEnableNanoseconds;
	nOutputEnableNanoseconds;
    pbyData;
    ulDataLen;
}

/***************************************************************************/
/*  Copyright (C) 2007-2020 Kevin Eshbach                                  */
/***************************************************************************/
