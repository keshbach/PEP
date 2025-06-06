/***************************************************************************/
/*  Copyright (C) 2007-2020 Kevin Eshbach                                  */
/***************************************************************************/

#include <Includes/UtCompiler.h>

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
static VOID UTPEPDEVICESAPI l2716ProgramDeviceWith25VDC(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI l2716ProgramDeviceWith12VDC(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);
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
	CDevicePin_Vpp,
    CDevicePin_Vdd,
	CDevicePin_LowChipSelect_ProgramEnable,
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
	CDevicePin_LowChipSelect_Vpp,
    CDevicePin_Vdd,
    CDevicePin_Address10,
    CDevicePin_Vbb,
    CDevicePin_Address9,
    CDevicePin_Address8,
	CDevicePin_Vcc_ProgramEnable};

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
    CDevicePin_LowChipEnable_Vpp,
    CDevicePin_Address10,
    CDevicePin_LowOutputEnable,
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
    CDevicePin_LowOutputEnable_Vpp,
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
    CDevicePin_LowChipSelect1_Vpp,
    CDevicePin_Address10,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Vcc};

static LPCWSTR l_psz2758PinNames[] = {
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
	CDevicePin_LowChipEnable_Vpp,
	L"AR",
	CDevicePin_LowOutputEnable,
	CDevicePin_Vpp,
	CDevicePin_Address9,
	CDevicePin_Address8,
	CDevicePin_Vcc };

static LPCWSTR l_psz2364PinNames[] = {
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
    CDevicePin_LowChipSelect,
    CDevicePin_Address12,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Vcc };

DEVICES_BEGIN
    DEVICE_DATA_BEGIN(1702)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz1702PinNames)
        DEVICE_ADAPTER("Part #A1702")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(900)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(100)
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
		DEVICE_CHIP_ENABLE_NANO_SECS(120)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(450)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0400)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_25VDC
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
		DEVICE_CHIP_ENABLE_NANO_SECS(120)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(450)
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
		DEVICE_CHIP_ENABLE_NANO_SECS(450)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(450)
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
		DEVICE_CHIP_ENABLE_NANO_SECS(450)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(150)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0800)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_25VDC
            DEVICE_ROM_READ_FUNC(l2716ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l2716ProgramDeviceWith25VDC)
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
		DEVICE_CHIP_ENABLE_NANO_SECS(350)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(150)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0800)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l2716ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l2716ProgramDeviceWith12VDC)
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
		DEVICE_CHIP_ENABLE_NANO_SECS(550)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(120)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0800)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_25VDC
            DEVICE_ROM_READ_FUNC(l2716ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l2716ProgramDeviceWith25VDC)
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
		DEVICE_CHIP_ENABLE_NANO_SECS(450)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(120)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0800)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l2716ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l2716ProgramDeviceWith12VDC)
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
		DEVICE_CHIP_ENABLE_NANO_SECS(450)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(150)
		DEVICE_INIT_FUNC(lGenericInit)
		DEVICE_UNINIT_FUNC(lGenericUninit)
		DEVICE_ROM_DATA_BEGIN
			DEVICE_ROM_SIZE(0x0800)
			DEVICE_ROM_BITSPERVALUE(8)
			DEVICE_ROM_VPP_25VDC
			DEVICE_ROM_READ_FUNC(l2716ReadDevice)
			DEVICE_ROM_PROGRAM_FUNC(l2716ProgramDeviceWith25VDC)
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
		DEVICE_CHIP_ENABLE_NANO_SECS(450)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(150)
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
		DEVICE_CHIP_ENABLE_NANO_SECS(350)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(150)
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
		DEVICE_CHIP_ENABLE_NANO_SECS(450)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(200)
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
		DEVICE_CHIP_ENABLE_NANO_SECS(300)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(100)
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
		DEVICE_CHIP_ENABLE_NANO_SECS(300)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(100)
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
	DEVICE_DATA_BEGIN(27C32)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz2732PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(450)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(150)
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
    DEVICE_DATA_BEGIN(27C32B)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz2732PinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS(350)
		DEVICE_OUTPUT_ENABLE_NANO_SECS(150)
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
    DEVICE_DATA_BEGIN(2364)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz2364PinNames)
        DEVICE_ADAPTER("Part# KE2364")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
        DEVICE_CHIP_ENABLE_NANO_SECS(450)
        DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
        DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x2000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_NOTSUPPORTED
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC_NONE
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
	// uses 25VPP for programming
	// uses single 50 ms pulse to program

    pDeviceIOFuncs;
	nChipEnableNanoseconds;
	nOutputEnableNanoseconds;
    pbyData;

	pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoProgram);

	pDeviceIOFuncs->pEndDeviceIOFunc(TRUE, edoProgram);
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

	pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoRead);

	pDeviceIOFuncs->pEndDeviceIOFunc(TRUE, edoRead);
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

	pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoProgram);

	pDeviceIOFuncs->pEndDeviceIOFunc(TRUE, edoProgram);
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

	pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoVerify);

	pDeviceIOFuncs->pEndDeviceIOFunc(TRUE, edoVerify);
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
	ULONG ulAddress;
	TUtPepCtrlReadUserData ReadUserData[3];

	nOutputEnableNanoseconds;

	pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoRead);

	// Since the Chip Enable pin of the 2708 is mapped to the Output Enable pin flip the 
	// delay parameters and just wait for the chip to power up before attempting a read.

	if (FALSE == UtPepCtrlSetDelaySettings(0, nChipEnableNanoseconds) ||
        FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
		FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
		FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse1Mode) ||
		FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode))
	{
		bErrorOccurred = TRUE;

		goto End;
	}

	for (ulAddress = 0; ulAddress < ulDataLen; ++ulAddress)
	{
		ReadUserData[0].nAddress = ulAddress;
		ReadUserData[0].OutputEnableMode = eUtPepCtrlDisableOE;
		ReadUserData[0].bPerformRead = FALSE;

		ReadUserData[1].nAddress = ulAddress;
		ReadUserData[1].OutputEnableMode = eUtPepCtrlEnableOE;
		ReadUserData[1].bPerformRead = TRUE;

		ReadUserData[2].nAddress = ulAddress;
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
	BOOL bErrorOccurred = FALSE;
	ULONG ulTmpBufferLen = ulDataLen / 32;
	ULONG ulAddress;

	nChipEnableNanoseconds;
	nOutputEnableNanoseconds;
    pbyData;
    ulDataLen;

	pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoProgram);

	// Since the Chip Enable pin of the 2708 is mapped to the Output Enable pin flip the 
	// delay parameters and just wait for the chip to power up before attempting a read.

	if (FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
		FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
		FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse1Mode) ||
		FALSE == UtPepCtrlSetVppMode(eUtPepCtrl25VDCVppMode) ||
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

static VOID UTPEPDEVICESAPI l2708VerifyDevice(
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

	nOutputEnableNanoseconds;

	pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoVerify);

	// Since the Chip Enable pin of the 2708 is mapped to the Output Enable pin flip the 
	// delay parameters and just wait for the chip to power up before attempting a read.

	if (FALSE == UtPepCtrlSetDelaySettings(0, nChipEnableNanoseconds) ||
		FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
		FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
		FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse1Mode) ||
		FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode))
	{
		bErrorOccurred = TRUE;

		goto End;
	}

	for (ulAddress = 0; ulAddress < ulDataLen; ++ulAddress)
	{
		ReadUserData[0].nAddress = ulAddress;
		ReadUserData[0].OutputEnableMode = eUtPepCtrlDisableOE;
		ReadUserData[0].bPerformRead = FALSE;

		ReadUserData[1].nAddress = ulAddress;
		ReadUserData[1].OutputEnableMode = eUtPepCtrlEnableOE;
		ReadUserData[1].bPerformRead = TRUE;

		ReadUserData[2].nAddress = ulAddress;
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
	UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode);

	pDeviceIOFuncs->pEndDeviceIOFunc(bErrorOccurred, edoVerify);
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

    if (FALSE == UtPepCtrlSetDelaySettings(nChipEnableNanoseconds, nOutputEnableNanoseconds) ||
		FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
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

static VOID UTPEPDEVICESAPI l2716ProgramDeviceWith25VDC(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds, 
  UINT32 nOutputEnableNanoseconds,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
	BOOL bErrorOccurred = FALSE;
	ULONG ulTmpBufferLen = ulDataLen / 32;
	ULONG ulAddress;

	nChipEnableNanoseconds;
	nOutputEnableNanoseconds;

	pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoProgram);

	if (FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
		FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
		FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse2Mode) ||
		FALSE == UtPepCtrlSetVppMode(eUtPepCtrl25VDCVppMode) ||
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

	pDeviceIOFuncs->pEndDeviceIOFunc(TRUE, edoProgram);
}

static VOID UTPEPDEVICESAPI l2716ProgramDeviceWith12VDC(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
	BOOL bErrorOccurred = FALSE;
	ULONG ulTmpBufferLen = ulDataLen / 32;
	ULONG ulAddress;

	nChipEnableNanoseconds;
	nOutputEnableNanoseconds;

	pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoProgram);

	if (FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
		FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
		FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse2Mode) ||
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

	pDeviceIOFuncs->pEndDeviceIOFunc(TRUE, edoProgram);
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

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoVerify);

    if (FALSE == UtPepCtrlSetDelaySettings(nChipEnableNanoseconds, nOutputEnableNanoseconds) ||
		FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
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

    pDeviceIOFuncs->pEndDeviceIOFunc(bErrorOccurred, edoVerify);
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
    ULONG ulAddress, ulTmpAddress;
	TUtPepCtrlReadUserDataWithDelay ReadUserDataWithDelay[1];

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoRead);

    if (FALSE == UtPepCtrlSetDelaySettings(nChipEnableNanoseconds, nOutputEnableNanoseconds) ||
		FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
        FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
        FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse1Mode) ||
        FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

	for (ulAddress = 0; ulAddress < ulDataLen; ++ulAddress)
    {
        ulTmpAddress = (ulAddress & 0x0400) << 2;
        ulTmpAddress |= (ulAddress & 0x03FF);

		ReadUserDataWithDelay[0].nAddress = ulTmpAddress;
		ReadUserDataWithDelay[0].bPerformRead = TRUE;
		ReadUserDataWithDelay[0].nDelayNanoSeconds = nOutputEnableNanoseconds;

		if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc() ||
			FALSE == UtPepCtrlReadUserDataWithDelay(ReadUserDataWithDelay, 1, pbyData + ulAddress, 1))
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
	nChipEnableNanoseconds;
	nOutputEnableNanoseconds;
    pbyData;

	// Note: pull OE pin low to send +12VDC to the Vcc pin

	pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoProgram);

	pDeviceIOFuncs->pEndDeviceIOFunc(TRUE, edoProgram);
}

static VOID UTPEPDEVICESAPI lTMS2716VerifyDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
    BOOL bErrorOccurred = FALSE;
    ULONG ulAddress, ulTmpAddress;
	TUtPepCtrlReadUserDataWithDelay ReadUserDataWithDelay[1];
	BYTE byData;

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoVerify);

    if (FALSE == UtPepCtrlSetDelaySettings(nChipEnableNanoseconds, nOutputEnableNanoseconds) ||
		FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
        FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
        FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse1Mode) ||
        FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

	for (ulAddress = 0; ulAddress < ulDataLen; ++ulAddress)
	{
		ulTmpAddress = (ulAddress & 0x0400) << 2;
        ulTmpAddress |= (ulAddress & 0x03FF);

		ReadUserDataWithDelay[0].nAddress = ulTmpAddress;
		ReadUserDataWithDelay[0].bPerformRead = TRUE;
		ReadUserDataWithDelay[0].nDelayNanoSeconds = nOutputEnableNanoseconds;

		if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc() ||
			FALSE == UtPepCtrlReadUserDataWithDelay(ReadUserDataWithDelay, 1, &byData, 1))
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
    UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode);

    pDeviceIOFuncs->pEndDeviceIOFunc(bErrorOccurred, edoVerify);
}

/***************************************************************************/
/*  Copyright (C) 2007-2020 Kevin Eshbach                                  */
/***************************************************************************/
