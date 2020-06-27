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

#define C2716VppPin 2048

static VOID UTPEPDEVICESAPI lGenericProgramDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);

static VOID UTPEPDEVICESAPI l1702ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI l1702ProgramDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI l1702VerifyDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);

static VOID UTPEPDEVICESAPI l2708ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI l2708ProgramDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI l2708VerifyDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);

static VOID UTPEPDEVICESAPI l2716ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI l2716ProgramDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI l2716VerifyDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);

static VOID UTPEPDEVICESAPI lTMS2716ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI lTMS2716ProgramDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI lTMS2716VerifyDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);

static LPCWSTR l_psz1702PinNames[] = {
    CDevicePin_Address2,
    CDevicePin_Address1,
    CDevicePin_Address0,
    CDevicePin_Data0,
    CDevicePin_Data1,
    CDevicePin_Data2,
    CDevicePin_Data3,
    CDevicePin_Data4,
    CDevicePin_Data5,
    CDevicePin_Data6,
    CDevicePin_Data7,
    CDevicePin_Vcc,
    CDevicePin_HighProgram,
    CDevicePin_LowChipSelect,
    CDevicePin_Vbb,
    CDevicePin_Vgg,
    CDevicePin_Address7,
    CDevicePin_Address6,
    CDevicePin_Address5,
    CDevicePin_Address4,
    CDevicePin_Address3,
    CDevicePin_Vcc,
    CDevicePin_Vcc,
    CDevicePin_Vdd};

static LPCWSTR l_psz2708PinNames[] = {
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
    CDevicePin_HighProgram,
    CDevicePin_Vdd,
    CDevicePin_LowChipSelect,
    CDevicePin_Vbb,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Vcc};

static LPCWSTR l_pszTMS2716PinNames[] = {
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
    CDevicePin_LowChipSelect,
    CDevicePin_Vdd,
    CDevicePin_Address10,
    CDevicePin_Vbb,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Vcc};

static LPCWSTR l_psz2716PinNames[] = {
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
    CDevicePin_Vpp,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Vcc};

static LPCWSTR l_psz2532PinNames[] = {
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
    CDevicePin_LowChipEnable,
    CDevicePin_Vpp,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Vcc};

static LPCWSTR l_psz2732PinNames[] = {
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
    CDevicePin_Vcc};

static LPCWSTR l_pszMMx203PinNames[] = {
    CDevicePin_Address2,
    CDevicePin_Address1,
    CDevicePin_Address0,
    CDevicePin_Data0,
    CDevicePin_Data1,
    CDevicePin_Data2,
    CDevicePin_Data3,
    CDevicePin_Data4,
    CDevicePin_Data5,
    CDevicePin_Data6,
    CDevicePin_Data7,
    L"",
    CDevicePin_Address8,
    CDevicePin_LowChipSelect,
    L"",
    L"",
    CDevicePin_Address7,
    CDevicePin_Address6,
    CDevicePin_Address5,
    CDevicePin_Address4,
    CDevicePin_Address3,
    CDevicePin_HighProgram,
    L"",
    L""};

static LPCWSTR l_psz57C43PinNames[] = {
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
    CDevicePin_HighChipSelect2,
    CDevicePin_Address11,
    CDevicePin_ChipSelect1_Vpp,
    CDevicePin_Address10,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Vcc};

static LPCWSTR l_psz2758PinNames[] = {
    L"",
    L"",
    L"",
    L"",
    L"",
    L"",
    L"",
    L"",
    L"",
    L"",
    L"",
    L"",
    L"",
    L"",
    L"",
    L"",
    L"",
    L"",
    L"",
    L"",
    L"",
    L"",
    L"",
    L""};

DEVICES_BEGIN
    DEVICE_DATA_BEGIN(1702)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz1702PinNames)
        DEVICE_ADAPTER("Part #A1702")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0100)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_26VDC
            DEVICE_ROM_READ_FUNC(l1702ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l1702ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l1702VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(MM4203)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_pszMMx203PinNames)
        DEVICE_ADAPTER("Contact Factory")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0100)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_26VDC
            DEVICE_ROM_READ_FUNC(NULL)
            DEVICE_ROM_PROGRAM_FUNC(NULL)
            DEVICE_ROM_VERIFY_FUNC(NULL)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(MM5203)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_pszMMx203PinNames)
        DEVICE_ADAPTER("Contact Factory")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0100)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_26VDC
            DEVICE_ROM_READ_FUNC(NULL)
            DEVICE_ROM_PROGRAM_FUNC(NULL)
            DEVICE_ROM_VERIFY_FUNC(NULL)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(2708)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz2708PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0400)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_26VDC
            DEVICE_ROM_READ_FUNC(l2708ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l2708ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l2708VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(MB8518)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz2708PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0400)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_26VDC
            DEVICE_ROM_READ_FUNC(l2708ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l2708ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l2708VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(TMS2716)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_pszTMS2716PinNames)
        DEVICE_ADAPTER("Part #ATMS2716")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0800)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_26VDC
            DEVICE_ROM_READ_FUNC(lTMS2716ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(lTMS2716ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lTMS2716VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(2758)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz2758PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0400)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_25VDC
            DEVICE_ROM_READ_FUNC(NULL)
            DEVICE_ROM_PROGRAM_FUNC(NULL)
            DEVICE_ROM_VERIFY_FUNC(NULL)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(2716)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz2716PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0800)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_25VDC
            DEVICE_ROM_READ_FUNC(l2716ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l2716ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l2716VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(2516)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz2716PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0800)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_25VDC
            DEVICE_ROM_READ_FUNC(l2716ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l2716ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l2716VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(2716B)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz2716PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0800)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l2716ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l2716ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l2716VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C16)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz2716PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0800)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_25VDC
            DEVICE_ROM_READ_FUNC(l2716ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l2716ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l2716VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C16B)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz2716PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0800)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l2716ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l2716ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l2716VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(2532)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz2532PinNames)
        DEVICE_ADAPTER("Part# KE2532")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x1000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_25VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(lGenericProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(2532A)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz2532PinNames)
        DEVICE_ADAPTER("Part# KE2532")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x1000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_21VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(lGenericProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(2732)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz2732PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x1000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_25VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(lGenericProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(2732A)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz2732PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x1000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_21VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(lGenericProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C32)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz2732PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x1000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_21VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(lGenericProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(2732B)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz2732PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x1000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(lGenericProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C32B)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz2732PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x1000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC(lGenericProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
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

/*
    1702 Device Functions
*/

static VOID UTPEPDEVICESAPI l1702ReadDevice(
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

static VOID UTPEPDEVICESAPI l1702ProgramDevice(
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

static VOID UTPEPDEVICESAPI l1702VerifyDevice(
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

/*
    2708 Device Functions
*/

static VOID UTPEPDEVICESAPI l2708ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    BOOL bErrorOccurred = FALSE;
    ULONG ulTmpBufferLen = ulDataLen / 32;
    ULONG ulAddress;

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoRead);

    if (FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
        FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
        FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse1Mode) ||
        FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    for (ulAddress = 0; ulAddress < ulDataLen; ulAddress += ulTmpBufferLen)
    {
        if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc() ||
            FALSE == UtPepCtrlReadData(ulAddress, pbyData + ulAddress,
                                       ulTmpBufferLen))
        {
            bErrorOccurred = TRUE;

            goto End;
        }

        pDeviceIOFuncs->pProgressDeviceIOFunc(ulAddress);
    }

End:
    UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode);

    pDeviceIOFuncs->pEndDeviceIOFunc(bErrorOccurred, edoRead);
}

static VOID UTPEPDEVICESAPI l2708ProgramDevice(
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

static VOID UTPEPDEVICESAPI l2708VerifyDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds, 
  UINT32 nOutputEnableNanoseconds,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
    BOOL bErrorOccurred = FALSE;
    ULONG ulTmpBufferLen = ulDataLen / 32;
    LPBYTE pbyTmpBuffer = (LPBYTE)UtAllocMem(ulTmpBufferLen);
    ULONG ulAddress, ulIndex;

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoVerify);

    if (FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
        FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
        FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse1Mode) ||
        FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    for (ulAddress = 0; ulAddress < ulDataLen; ulAddress += ulTmpBufferLen)
    {
        if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc() ||
            FALSE == UtPepCtrlReadData(ulAddress, pbyTmpBuffer,
                                       ulTmpBufferLen))
        {
            bErrorOccurred = TRUE;

            goto End;
        }

        for (ulIndex = 0; ulIndex < ulTmpBufferLen; ++ulIndex)
        {
            if (pbyData[ulAddress + ulIndex] != pbyTmpBuffer[ulIndex])
            {
                pDeviceIOFuncs->pVerifyByteErrorDeviceIOFunc(ulAddress + ulIndex,
                                                             pbyData[ulAddress + ulIndex],
                                                             pbyTmpBuffer[ulIndex]);
            }
        }

        pDeviceIOFuncs->pProgressDeviceIOFunc(ulAddress);
    }

End:
    UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode);

    pDeviceIOFuncs->pEndDeviceIOFunc(bErrorOccurred, edoVerify);

    UtFreeMem(pbyTmpBuffer);
}

/*
    2716 Device Functions
*/

static VOID UTPEPDEVICESAPI l2716ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    BOOL bErrorOccurred = FALSE;
    ULONG ulAddress;
    TUtPepCtrlReadUserData ReadUserData[3];

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoRead);

    if (FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
        FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
        FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse4Mode) ||
        FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    for (ulAddress = 0; ulAddress < ulDataLen; ++ulAddress)
    {
        ReadUserData[0].nAddress = ulAddress | C2716VppPin;
        ReadUserData[0].OutputEnableMode = eUtPepCtrlDisableOE;
        ReadUserData[0].bPerformRead = FALSE;

        ReadUserData[1].nAddress = ulAddress | C2716VppPin;
        ReadUserData[1].OutputEnableMode = eUtPepCtrlEnableOE;
        ReadUserData[1].bPerformRead = TRUE;

        ReadUserData[2].nAddress = ulAddress | C2716VppPin;
        ReadUserData[2].OutputEnableMode = eUtPepCtrlDisableOE;
        ReadUserData[2].bPerformRead = FALSE;

        if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc() ||
            FALSE == UtPepCtrlReadUserData(ReadUserData, 3, pbyData + ulAddress, 1))
        {
            bErrorOccurred = TRUE;

            goto End;
        }

        pDeviceIOFuncs->pProgressDeviceIOFunc(ulAddress);
    }

End:
    /* Turn Vpp off before Vcc is turned off. */
    ReadUserData[0].nAddress = 0;
    ReadUserData[0].OutputEnableMode = eUtPepCtrlDisableOE;
    ReadUserData[0].bPerformRead = FALSE;

    UtPepCtrlReadUserData(ReadUserData, 1, NULL, 0);

    UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode);

    pDeviceIOFuncs->pEndDeviceIOFunc(bErrorOccurred, edoRead);
}

static VOID UTPEPDEVICESAPI l2716ProgramDevice(
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

static VOID UTPEPDEVICESAPI l2716VerifyDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
    BOOL bErrorOccurred = FALSE;
    ULONG ulAddress;
    TUtPepCtrlReadUserData ReadUserData[3];
    BYTE byData;

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoRead);

    if (FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
        FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
        FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse4Mode) ||
        FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    for (ulAddress = 0; ulAddress < ulDataLen; ++ulAddress)
    {
        ReadUserData[0].nAddress = ulAddress | C2716VppPin;
        ReadUserData[0].OutputEnableMode = eUtPepCtrlDisableOE;
        ReadUserData[0].bPerformRead = FALSE;

        ReadUserData[1].nAddress = ulAddress | C2716VppPin;
        ReadUserData[1].OutputEnableMode = eUtPepCtrlEnableOE;
        ReadUserData[1].bPerformRead = TRUE;

        ReadUserData[2].nAddress = ulAddress | C2716VppPin;
        ReadUserData[2].OutputEnableMode = eUtPepCtrlDisableOE;
        ReadUserData[2].bPerformRead = FALSE;

        if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc() ||
            FALSE == UtPepCtrlReadUserData(ReadUserData, 3, &byData, 1))
        {
            bErrorOccurred = TRUE;

            goto End;
        }

        if (pbyData[ulAddress] != byData)
        {
            pDeviceIOFuncs->pVerifyByteErrorDeviceIOFunc(ulAddress,
                                                         pbyData[ulAddress],
                                                         byData);
        }

        pDeviceIOFuncs->pProgressDeviceIOFunc(ulAddress);
    }

End:
    /* Turn Vpp off before Vcc is turned off. */
    ReadUserData[0].nAddress = 0;
    ReadUserData[0].OutputEnableMode = eUtPepCtrlDisableOE;
    ReadUserData[0].bPerformRead = FALSE;

    UtPepCtrlReadUserData(ReadUserData, 1, NULL, 0);

    UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode);

    pDeviceIOFuncs->pEndDeviceIOFunc(bErrorOccurred, edoRead);
}

/*
   TMS2716 Device Functions
*/

static VOID UTPEPDEVICESAPI lTMS2716ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    BOOL bErrorOccurred = FALSE;
    ULONG ulTmpBufferLen = ulDataLen / 32;
    ULONG ulAddress, ulTmpAddress;

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoRead);

    if (FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
        FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
        FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse1Mode) ||
        FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    for (ulAddress = 0; ulAddress < ulDataLen; ulAddress += ulTmpBufferLen)
    {
        ulTmpAddress = (ulAddress & 0x0400) << 2;
        ulTmpAddress |= (ulAddress & 0x03FF);

        if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc() ||
            FALSE == UtPepCtrlReadData(ulTmpAddress, pbyData + ulAddress,
                                       ulTmpBufferLen))
        {
            bErrorOccurred = TRUE;

            goto End;
        }

        pDeviceIOFuncs->pProgressDeviceIOFunc(ulAddress);
    }

End:
    UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode);

    pDeviceIOFuncs->pEndDeviceIOFunc(bErrorOccurred, edoRead);
}

static VOID UTPEPDEVICESAPI lTMS2716ProgramDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
    pDeviceIOFuncs;
    pbyData;
    ulDataLen;
}

static VOID UTPEPDEVICESAPI lTMS2716VerifyDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
    BOOL bErrorOccurred = FALSE;
    ULONG ulTmpBufferLen = ulDataLen / 32;
    LPBYTE pbyTmpBuffer = (LPBYTE)UtAllocMem(ulTmpBufferLen);
    ULONG ulAddress, ulTmpAddress, ulIndex;

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoVerify);

    if (FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
        FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
        FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse1Mode) ||
        FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    for (ulAddress = 0; ulAddress < ulDataLen; ulAddress += ulTmpBufferLen)
    {
        ulTmpAddress = (ulAddress & 0x0400) << 2;
        ulTmpAddress |= (ulAddress & 0x03FF);

        if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc() ||
            FALSE == UtPepCtrlReadData(ulTmpAddress, pbyTmpBuffer,
                                       ulTmpBufferLen))
        {
            bErrorOccurred = TRUE;

            goto End;
        }

        for (ulIndex = 0; ulIndex < ulTmpBufferLen; ++ulIndex)
        {
            if (pbyData[ulAddress + ulIndex] != pbyTmpBuffer[ulIndex])
            {
                pDeviceIOFuncs->pVerifyByteErrorDeviceIOFunc(ulAddress + ulIndex,
                                                             pbyData[ulAddress + ulIndex],
                                                             pbyTmpBuffer[ulIndex]);
            }
        }

        pDeviceIOFuncs->pProgressDeviceIOFunc(ulAddress);
    }

End:
    UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode);

    pDeviceIOFuncs->pEndDeviceIOFunc(bErrorOccurred, edoVerify);

    UtFreeMem(pbyTmpBuffer);
}

/***************************************************************************/
/*  Copyright (C) 2007-2020 Kevin Eshbach                                  */
/***************************************************************************/
