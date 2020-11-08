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

static VOID UTPEPDEVICESAPI lGenericProgramDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);

static VOID UTPEPDEVICESAPI l2764NMOSProgramDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI l2764CMOSProgramDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);

static VOID UTPEPDEVICESAPI l27513ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI l27513ProgramDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI l27513VerifyDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);

static VOID UTPEPDEVICESAPI l27C011ProgramDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);

static LPCWSTR l_pszMB831000PinNames[] = {
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
    CDevicePin_Vcc};

static LPCWSTR l_psz2764PinNames[] = {
    CDevicePin_Vpp,
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
    CDevicePin_LowOutputEnable,
    CDevicePin_Address11,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_NotUsed,
    CDevicePin_LowProgram,
    CDevicePin_Vcc};

static LPCWSTR l_psz2564PinNames[] = {
    CDevicePin_Vpp,
    CDevicePin_LowChipSelect1,
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
    CDevicePin_Address11,
    CDevicePin_Address10,
    CDevicePin_LowProgram,
    CDevicePin_Address12,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Vcc,
    CDevicePin_LowChipSelect2,
    CDevicePin_Vcc};

static LPCWSTR l_psz27128PinNames[] = {
    CDevicePin_Vpp,
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
    CDevicePin_LowOutputEnable,
    CDevicePin_Address11,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Address13,
    CDevicePin_LowProgram,
    CDevicePin_Vcc};

static LPCWSTR l_psz27256PinNames[] = {
    CDevicePin_Vpp,
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
    CDevicePin_LowOutputEnable,
    CDevicePin_Address11,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Address13,
    CDevicePin_Address14,
    CDevicePin_Vcc};

static LPCWSTR l_psz27512PinNames[] = {
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
    CDevicePin_LowOutputEnable_Vpp,
    CDevicePin_Address11,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Address13,
    CDevicePin_Address14,
    CDevicePin_Vcc};

static LPCWSTR l_psz27513PinNames[] = {
    CDevicePin_LowReset,
    CDevicePin_Address12,
    CDevicePin_Address7,
    CDevicePin_Address6,
    CDevicePin_Address5,
    CDevicePin_Address4,
    CDevicePin_Address3,
    CDevicePin_Address2,
    CDevicePin_Address1,
    CDevicePin_Address0,
    CDevicePin_DataInOut0,
    CDevicePin_DataInOut1,
    CDevicePin_Data2,
    CDevicePin_Ground,
    CDevicePin_Data3,
    CDevicePin_Data4,
    CDevicePin_Data5,
    CDevicePin_Data6,
    CDevicePin_Data7,
    CDevicePin_LowChipEnable,
    CDevicePin_Address10,
    CDevicePin_LowOutputEnable,
    CDevicePin_Address11,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Address13,
    CDevicePin_LowWriteEnable,
    CDevicePin_Vcc};

static LPCWSTR l_psz27C011PinNames[] = {
    CDevicePin_LowReset_Vpp,
    CDevicePin_Address12,
    CDevicePin_Address7,
    CDevicePin_Address6,
    CDevicePin_Address5,
    CDevicePin_Address4,
    CDevicePin_Address3,
    CDevicePin_Address2,
    CDevicePin_Address1,
    CDevicePin_Address0,
    CDevicePin_DataInOut0,
    CDevicePin_DataInOut1,
    CDevicePin_DataInOut2,
    CDevicePin_Ground,
    CDevicePin_Data3,
    CDevicePin_Data4,
    CDevicePin_Data5,
    CDevicePin_Data6,
    CDevicePin_Data7,
    CDevicePin_LowChipEnable,
    CDevicePin_Address10,
    CDevicePin_LowOutputEnable,
    CDevicePin_Address11,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Address13,
    CDevicePin_LowWriteEnable_LowProgram,
    CDevicePin_Vcc};

DEVICES_BEGIN
    DEVICE_DATA_BEGIN(MB831000)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_pszMB831000PinNames)
        DEVICE_ADAPTER("Part# KE27301 28 Pins")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x20000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_NOTSUPPORTED
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC_NONE
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(2764)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz2764PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(300)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(120)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x2000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_21VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l2764NMOSProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(2764A)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz2764PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(250)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(100)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x2000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l2764CMOSProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C64)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz2764PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(250)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(100)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x2000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l2764CMOSProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27LV64)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz2764PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x2000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l2764CMOSProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(HN27C64)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz2764PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x2000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_21VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l2764NMOSProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(MBM27C64)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz2764PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x2000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_21VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l2764NMOSProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(87C64)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz2764PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x2000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(l2764CMOSProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(2564)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz2564PinNames)
        DEVICE_ADAPTER("Part# KE2564")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(450)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(120)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x2000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_25VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(lGenericProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(HN27128)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz27128PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x4000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_21VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(lGenericProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27128)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz27128PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(450)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(150)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x4000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_21VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(lGenericProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27128A)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz27128PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(300)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(120)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x4000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(lGenericProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C128)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz27128PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(250)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(100)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x4000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(lGenericProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27CP128)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz27128PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x4000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(lGenericProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27LV128)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz27128PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x4000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(lGenericProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(MBM27C128)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz27128PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x4000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_21VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(lGenericProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27256)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz27256PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(250)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(100)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x8000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(lGenericProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(87C257)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz27256PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(150)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(60)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x8000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(lGenericProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C256)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz27256PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(250)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(150)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x8000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(lGenericProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27LV256)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz27256PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x8000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(lGenericProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(MBM27C256)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz27256PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(450)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(150)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x8000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_21VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(lGenericProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(MBM27C256H)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz27256PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x8000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(lGenericProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(57256)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz27256PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(200)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(70)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x8000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(NULL)
            DEVICE_ROM_PROGRAM_FUNC(NULL)
            DEVICE_ROM_VERIFY_FUNC(NULL)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27512)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz27512PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(300)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(120)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x10000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(lGenericProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C512)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz27512PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(150)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(60)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x10000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(lGenericProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(WE27E512)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz27512PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, TRUE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x10000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(lGenericProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27LV512)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz27512PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(300)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(125)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x10000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(lGenericProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(57512)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz27512PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(200)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(70)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x10000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(lGenericProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C011)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz27C011PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(200)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(70)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x20000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27513ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27C011ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27513VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27513)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz27513PinNames)
        DEVICE_ADAPTER("Part# A27513")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(250)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(100)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x10000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27513ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27513ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27513VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C513)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(28)
        DEVICE_PINNAMES(l_psz27513PinNames)
        DEVICE_ADAPTER("Part# A27513")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, TRUE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(250)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(100)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x10000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27513ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27513ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27513VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
DEVICES_END

static VOID UTPEPDEVICESAPI lGenericProgramDevice(
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

static VOID UTPEPDEVICESAPI l2764NMOSProgramDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
    BOOL bErrorOccurred = FALSE;
    ULONG ulTmpBufferLen = ulDataLen / CGenericBytesPerOperation;
    ULONG ulAddress;

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoProgram);

    if (FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
        FALSE == UtPepCtrlSetVccMode(eUtPepCtrl625VDCMode) ||
        FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse4Mode) ||
        FALSE == UtPepCtrlSetVppMode(eUtPepCtrl21VDCVppMode) ||
        FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerWriteMode))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    for (ulAddress = 0; ulAddress < ulDataLen; ulAddress += ulTmpBufferLen)
    {
        if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc() ||
            FALSE == UtPepCtrlProgramData(ulAddress, pbyData + ulAddress,
                                          ulTmpBufferLen))
        {
            bErrorOccurred = TRUE;

            goto End;
        }

        pDeviceIOFuncs->pProgressDeviceIOFunc(ulAddress);
    }

    pDeviceIOFuncs->pProgressDeviceIOFunc(ulDataLen);

End:
    UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode);

    pDeviceIOFuncs->pEndDeviceIOFunc(bErrorOccurred, edoProgram);
}

static VOID UTPEPDEVICESAPI l2764CMOSProgramDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
    BOOL bErrorOccurred = FALSE;
    ULONG ulTmpBufferLen = ulDataLen / CGenericBytesPerOperation;
    ULONG ulAddress;

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoWrite);

    if (FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
        FALSE == UtPepCtrlSetVccMode(eUtPepCtrl625VDCMode) ||
        FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse4Mode) ||
        FALSE == UtPepCtrlSetVppMode(eUtPepCtrl12VDCVppMode) ||
        FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerWriteMode))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    for (ulAddress = 0; ulAddress < ulDataLen; ulAddress += ulTmpBufferLen)
    {
        if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc() ||
            FALSE == UtPepCtrlProgramData(ulAddress, pbyData + ulAddress,
                                          ulTmpBufferLen))
        {
            bErrorOccurred = TRUE;

            goto End;
        }

        pDeviceIOFuncs->pProgressDeviceIOFunc(ulAddress);
    }

    pDeviceIOFuncs->pProgressDeviceIOFunc(ulDataLen);

End:
    UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode);

    pDeviceIOFuncs->pEndDeviceIOFunc(bErrorOccurred, edoWrite);
}

/*
    27513
*/

static VOID UTPEPDEVICESAPI l27513ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    pDeviceIOFuncs;
	nChipEnableNanoseconds;
	nOutputEnableNanoseconds;
    pbyData;
    ulDataLen;
}

static VOID UTPEPDEVICESAPI l27513ProgramDevice(
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

static VOID UTPEPDEVICESAPI l27513VerifyDevice(
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

static VOID UTPEPDEVICESAPI l27C011ProgramDevice(
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
