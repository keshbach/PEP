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

#define CDataBits0To7 0
#define CDataBits8To15 1

#define MSelectAddressAndDataBits(address, dataBits) \
    (((ULONG)address << 1) | ((ULONG)dataBits & 0x01))

static BOOL UTPEPDEVICESAPI lGenericInit(VOID);
static BOOL UTPEPDEVICESAPI lGenericUninit(VOID);

static VOID UTPEPDEVICESAPI l27Cx02ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI l27Cx02ProgramDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI l27Cx02VerifyDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);

static LPCWSTR l_psz27C1024PinNames[] = {
    CDevicePin_Vpp,
    CDevicePin_LowChipEnable,
    CDevicePin_Data15,
    CDevicePin_Data14,
    CDevicePin_Data13,
    CDevicePin_Data12,
    CDevicePin_Data11,
    CDevicePin_Data10,
    CDevicePin_Data9,
    CDevicePin_Data8,
    CDevicePin_Ground,
    CDevicePin_Data7,
    CDevicePin_Data6,
    CDevicePin_Data5,
    CDevicePin_Data4,
    CDevicePin_Data3,
    CDevicePin_Data2,
    CDevicePin_Data1,
    CDevicePin_Data0,
    CDevicePin_OutputEnable,
    CDevicePin_Address0,
    CDevicePin_Address1,
    CDevicePin_Address2,
    CDevicePin_Address3,
    CDevicePin_Address4,
    CDevicePin_Address5,
    CDevicePin_Address6,
    CDevicePin_Address7,
    CDevicePin_Address8,
    CDevicePin_Ground,
    CDevicePin_Address9,
    CDevicePin_Address10,
    CDevicePin_Address11,
    CDevicePin_Address12,
    CDevicePin_Address13,
    CDevicePin_Address14,
    CDevicePin_Address15,
    CDevicePin_NotUsed,
    CDevicePin_LowProgram,
    CDevicePin_Vcc};

static LPCWSTR l_psz27C2048PinNames[] = {
    CDevicePin_Vpp,
    CDevicePin_LowChipEnable,
    CDevicePin_Data15,
    CDevicePin_Data14,
    CDevicePin_Data13,
    CDevicePin_Data12,
    CDevicePin_Data11,
    CDevicePin_Data10,
    CDevicePin_Data9,
    CDevicePin_Data8,
    CDevicePin_Ground,
    CDevicePin_Data7,
    CDevicePin_Data6,
    CDevicePin_Data5,
    CDevicePin_Data4,
    CDevicePin_Data3,
    CDevicePin_Data2,
    CDevicePin_Data1,
    CDevicePin_Data0,
    CDevicePin_OutputEnable,
    CDevicePin_Address0,
    CDevicePin_Address1,
    CDevicePin_Address2,
    CDevicePin_Address3,
    CDevicePin_Address4,
    CDevicePin_Address5,
    CDevicePin_Address6,
    CDevicePin_Address7,
    CDevicePin_Address8,
    CDevicePin_Ground,
    CDevicePin_Address9,
    CDevicePin_Address10,
    CDevicePin_Address11,
    CDevicePin_Address12,
    CDevicePin_Address13,
    CDevicePin_Address14,
    CDevicePin_Address15,
    CDevicePin_Address16,
    CDevicePin_LowProgram,
    CDevicePin_Vcc};

static LPCWSTR l_psz27C4096PinNames[] = {
    CDevicePin_Vpp,
    CDevicePin_LowChipEnable,
    CDevicePin_Data15,
    CDevicePin_Data14,
    CDevicePin_Data13,
    CDevicePin_Data12,
    CDevicePin_Data11,
    CDevicePin_Data10,
    CDevicePin_Data9,
    CDevicePin_Data8,
    CDevicePin_Ground,
    CDevicePin_Data7,
    CDevicePin_Data6,
    CDevicePin_Data5,
    CDevicePin_Data4,
    CDevicePin_Data3,
    CDevicePin_Data2,
    CDevicePin_Data1,
    CDevicePin_Data0,
    CDevicePin_OutputEnable,
    CDevicePin_Address0,
    CDevicePin_Address1,
    CDevicePin_Address2,
    CDevicePin_Address3,
    CDevicePin_Address4,
    CDevicePin_Address5,
    CDevicePin_Address6,
    CDevicePin_Address7,
    CDevicePin_Address8,
    CDevicePin_Ground,
    CDevicePin_Address9,
    CDevicePin_Address10,
    CDevicePin_Address11,
    CDevicePin_Address12,
    CDevicePin_Address13,
    CDevicePin_Address14,
    CDevicePin_Address15,
    CDevicePin_Address16,
    CDevicePin_Address17,
    CDevicePin_Vcc};

static LPCWSTR l_pszMX27C1100PinNames[] = {
    CDevicePin_NotUsed,
    CDevicePin_Address7,
    CDevicePin_Address6,
    CDevicePin_Address5,
    CDevicePin_Address4,
    CDevicePin_Address3,
    CDevicePin_Address2,
    CDevicePin_Address1,
    CDevicePin_Address0,
    CDevicePin_LowChipEnable,
    CDevicePin_Ground,
    CDevicePin_OutputEnable,
    CDevicePin_Data0,
    CDevicePin_Data8,
    CDevicePin_Data1,
    CDevicePin_Data9,
    CDevicePin_Data2,
    CDevicePin_Data10,
    CDevicePin_Data3,
    CDevicePin_Data11,
    CDevicePin_Vcc,
    CDevicePin_Data4,
    CDevicePin_Data12,
    CDevicePin_Data5,
    CDevicePin_Data13,
    CDevicePin_Data6,
    CDevicePin_Data14,
    CDevicePin_Data7,
    CDevicePin_Address15_A1,
    CDevicePin_Ground,
    CDevicePin_NotUsed,
    CDevicePin_Address16,
    CDevicePin_Address15,
    CDevicePin_Address14,
    CDevicePin_Address13,
    CDevicePin_Address12,
    CDevicePin_Address11,
    CDevicePin_Address10,
    CDevicePin_Address9,
    CDevicePin_Address8};

static LPCWSTR l_psz27C4000PinNames[] = {
    CDevicePin_Address17,
    CDevicePin_Address7,
    CDevicePin_Address6,
    CDevicePin_Address5,
    CDevicePin_Address4,
    CDevicePin_Address3,
    CDevicePin_Address2,
    CDevicePin_Address1,
    CDevicePin_Address0,
    CDevicePin_LowChipEnable,
    CDevicePin_Ground,
    CDevicePin_OutputEnable,
    CDevicePin_Data0,
    CDevicePin_Data8,
    CDevicePin_Data1,
    CDevicePin_Data9,
    CDevicePin_Data2,
    CDevicePin_Data10,
    CDevicePin_Data3,
    CDevicePin_Data11,
    CDevicePin_Vcc,
    CDevicePin_Data4,
    CDevicePin_Data12,
    CDevicePin_Data5,
    CDevicePin_Data13,
    CDevicePin_Data6,
    CDevicePin_Data14,
    CDevicePin_Data7,
    CDevicePin_Data15_A1,
    CDevicePin_Ground,
    CDevicePin_Byte_Vpp,
    CDevicePin_Address16,
    CDevicePin_Address15,
    CDevicePin_Address14,
    CDevicePin_Address13,
    CDevicePin_Address12,
    CDevicePin_Address11,
    CDevicePin_Address10,
    CDevicePin_Address9,
    CDevicePin_Address8};

static LPCWSTR l_pszPinNames[] = {
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
    DEVICE_DATA_BEGIN(27C102)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(40)
        DEVICE_PINNAMES(l_psz27C1024PinNames)
        DEVICE_ADAPTER("Part# A40PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x20000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx02ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx02ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx02VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C516)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(40)
        DEVICE_PINNAMES(l_pszPinNames)
        DEVICE_ADAPTER("Part# A40PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x10000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx02ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx02ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx02VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C1024)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(40)
        DEVICE_PINNAMES(l_psz27C1024PinNames)
        DEVICE_ADAPTER("Part# A40PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x20000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx02ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx02ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx02VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(57C1024)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(40)
        DEVICE_PINNAMES(l_pszPinNames)
        DEVICE_ADAPTER("Part# A40PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x20000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx02ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx02ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx02VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(57H1025)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(40)
        DEVICE_PINNAMES(l_psz27C1024PinNames)
        DEVICE_ADAPTER("Part# A40PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x20000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx02ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx02ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx02VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C1616)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(40)
        DEVICE_PINNAMES(l_pszPinNames)
        DEVICE_ADAPTER("Part# A40PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x8000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx02ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx02ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx02VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C210)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(40)
        DEVICE_PINNAMES(l_psz27C1024PinNames)
        DEVICE_ADAPTER("Part# A40PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x20000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx02ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx02ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx02VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C2048)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(40)
        DEVICE_PINNAMES(l_psz27C2048PinNames)
        DEVICE_ADAPTER("Part# A40PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x40000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx02ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx02ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx02VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C202)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(40)
        DEVICE_PINNAMES(l_psz27C2048PinNames)
        DEVICE_ADAPTER("Part# A40PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x40000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx02ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx02ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx02VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C220)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(40)
        DEVICE_PINNAMES(l_psz27C2048PinNames)
        DEVICE_ADAPTER("Part# A40PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x40000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx02ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx02ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx02VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C4096)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(40)
        DEVICE_PINNAMES(l_psz27C4096PinNames)
        DEVICE_ADAPTER("Part# A40PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x80000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx02ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx02ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx02VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C240)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(40)
        DEVICE_PINNAMES(l_psz27C4096PinNames)
        DEVICE_ADAPTER("Part# A40PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x80000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx02ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx02ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx02VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C4002)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(40)
        DEVICE_PINNAMES(l_psz27C4096PinNames)
        DEVICE_ADAPTER("Part# A40PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x80000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx02ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx02ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx02VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C402)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(40)
        DEVICE_PINNAMES(l_psz27C4096PinNames)
        DEVICE_ADAPTER("Part# A40PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x80000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx02ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx02ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx02VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(MX27C1100)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(40)
        DEVICE_PINNAMES(l_pszMX27C1100PinNames)
        DEVICE_ADAPTER("Part# A42PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x20000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx02ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx02ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx02VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(MX27C2100)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(40)
        DEVICE_PINNAMES(l_psz27C1024PinNames)
        DEVICE_ADAPTER("Part# A40PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x20000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx02ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx02ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx02VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(HN27C4000)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(40)
        DEVICE_PINNAMES(l_psz27C4000PinNames)
        DEVICE_ADAPTER("Part# A42PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x80000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx02ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx02ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx02VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(MX27C4100)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(40)
        DEVICE_PINNAMES(l_psz27C4000PinNames)
        DEVICE_ADAPTER("Part# A42PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x80000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx02ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx02ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx02VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(MX27C4111)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(40)
        DEVICE_PINNAMES(l_psz27C4000PinNames)
        DEVICE_ADAPTER("Part# A42PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x80000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx02ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx02ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx02VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(uPD27C4000)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(40)
        DEVICE_PINNAMES(l_psz27C4000PinNames)
        DEVICE_ADAPTER("Part# A42PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x80000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx02ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx02ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx02VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(NEC27C4040)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(40)
        DEVICE_PINNAMES(l_pszPinNames)
        DEVICE_ADAPTER("Part# A42PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x80000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(NULL)
            DEVICE_ROM_PROGRAM_FUNC(NULL)
            DEVICE_ROM_VERIFY_FUNC(NULL)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(TC574200)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(40)
        DEVICE_PINNAMES(l_psz27C4000PinNames)
        DEVICE_ADAPTER("Part# A42PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x80000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx02ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx02ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx02VerifyDevice)
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

/*
   27Cx02 Device Functions
*/

static VOID UTPEPDEVICESAPI l27Cx02ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    BOOL bErrorOccurred = FALSE;
    ULONG ulTmpBufferLen = (ulDataLen / sizeof(WORD)) / 32;
    ULONG ulAddress, ulTmpAddress, ulIndex, ulTmpIndex;
    TUtPepCtrlReadUserData* pReadUserData;

    pReadUserData = (TUtPepCtrlReadUserData*)UtAllocMem(
                        (sizeof(TUtPepCtrlReadUserData) * 2) * ulTmpBufferLen);

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen / sizeof(WORD), edoRead);

	if (FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
        FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse4Mode) ||
        FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    for (ulAddress = 0; ulAddress < ulDataLen / sizeof(WORD);
         ulAddress += ulTmpBufferLen)
    {
        ulTmpIndex = 0;

        for (ulIndex = 0; ulIndex < ulTmpBufferLen; ++ulIndex)
        {
            ulTmpAddress = ulAddress + ulIndex;

            pReadUserData[ulTmpIndex].nAddress = MSelectAddressAndDataBits(
                                                     ulTmpAddress, CDataBits0To7);
            pReadUserData[ulTmpIndex].OutputEnableMode = eUtPepCtrlIgnoreOE;
            pReadUserData[ulTmpIndex].bPerformRead = TRUE;

            ++ulTmpIndex;

            ulTmpAddress = ulAddress + ulIndex;

            pReadUserData[ulTmpIndex].nAddress = MSelectAddressAndDataBits(
                                                     ulTmpAddress,
                                                     CDataBits8To15);
            pReadUserData[ulTmpIndex].OutputEnableMode = eUtPepCtrlIgnoreOE;
            pReadUserData[ulTmpIndex].bPerformRead = TRUE;

            ++ulTmpIndex;
        }

        if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc() ||
            FALSE == UtPepCtrlReadUserData(pReadUserData, ulTmpBufferLen * 2,
                                           pbyData + (ulAddress * 2),
                                           ulTmpBufferLen * 2))
        {
            bErrorOccurred = TRUE;

            goto End;
        }

        pDeviceIOFuncs->pProgressDeviceIOFunc(ulAddress);
    }

End:
    UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode);

    pDeviceIOFuncs->pEndDeviceIOFunc(bErrorOccurred, edoRead);

    UtFreeMem(pReadUserData);
}

static VOID UTPEPDEVICESAPI l27Cx02ProgramDevice(
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

static VOID UTPEPDEVICESAPI l27Cx02VerifyDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
    BOOL bErrorOccurred = FALSE;
    ULONG ulTmpBufferLen = (ulDataLen / sizeof(WORD)) / 32;
    ULONG ulAddress, ulTmpAddress, ulIndex, ulTmpIndex;
    TUtPepCtrlReadUserData* pReadUserData;
    LPWORD pwTmpData;

    pReadUserData = (TUtPepCtrlReadUserData*)UtAllocMem(
                        (sizeof(TUtPepCtrlReadUserData) * 2) * ulTmpBufferLen);

    pwTmpData = (LPWORD)UtAllocMem(ulTmpBufferLen * sizeof(WORD));

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen / sizeof(WORD), edoVerify);

    if (FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
        FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
        FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse4Mode) ||
        FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    for (ulAddress = 0; ulAddress < ulDataLen / sizeof(WORD);
         ulAddress += ulTmpBufferLen)
    {
        ulTmpIndex = 0;

        for (ulIndex = 0; ulIndex < ulTmpBufferLen; ++ulIndex)
        {
            ulTmpAddress = ulAddress + ulIndex;

            pReadUserData[ulTmpIndex].nAddress = MSelectAddressAndDataBits(
                                                     ulTmpAddress,
                                                     CDataBits0To7);
            pReadUserData[ulTmpIndex].OutputEnableMode = eUtPepCtrlIgnoreOE;
            pReadUserData[ulTmpIndex].bPerformRead = TRUE;

            ++ulTmpIndex;

            ulTmpAddress = ulAddress + ulIndex;

            pReadUserData[ulTmpIndex].nAddress = MSelectAddressAndDataBits(
                                                     ulTmpAddress,
                                                     CDataBits8To15);
            pReadUserData[ulTmpIndex].OutputEnableMode = eUtPepCtrlIgnoreOE;
            pReadUserData[ulTmpIndex].bPerformRead = TRUE;

            ++ulTmpIndex;
        }

        if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc() ||
            FALSE == UtPepCtrlReadUserData(pReadUserData, ulTmpBufferLen * 2,
                                           (LPBYTE)pwTmpData,
                                           ulTmpBufferLen * sizeof(WORD)))
        {
            bErrorOccurred = TRUE;

            goto End;
        }

        for (ulIndex = 0; ulIndex < ulTmpBufferLen; ++ulIndex)
        {
            if (*(pwTmpData + ulIndex) != *((LPDWORD)pbyData + ulIndex + ulAddress))
            {
                pDeviceIOFuncs->pVerifyWordErrorDeviceIOFunc(
                    ulAddress + ulIndex,
                    *((LPWORD)pbyData + ulIndex + ulAddress),
                    *(pwTmpData + ulIndex));
            }
        }

        if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc())
        {
            bErrorOccurred = TRUE;

            goto End;
        }

        pDeviceIOFuncs->pProgressDeviceIOFunc(ulAddress);
    }

End:
    UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode);

    pDeviceIOFuncs->pEndDeviceIOFunc(bErrorOccurred, edoVerify);

    UtFreeMem(pwTmpData);
    UtFreeMem(pReadUserData);
}

/***************************************************************************/
/*  Copyright (C) 2007-2020 Kevin Eshbach                                  */
/***************************************************************************/
