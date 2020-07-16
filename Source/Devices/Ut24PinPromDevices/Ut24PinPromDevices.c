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

#define CGeneric_CE1_AddressLine 2048
#define CGeneric_CE2_AddressLine 4096

#define C82S183_CS3_AddressLine 2048
#define C82S183_Strobe_AddressLine 4096

#define C82S191_CE2_AddressLine 2048
#define C82S191_CE3_AddressLine 4096

#define C82S321_CE2_AddressLine 4096

#define C82S11x_CE2_AddressLine 512
#define C82S11x_Strobe_AddressLine 1024

static BOOL UTPEPDEVICESAPI lGenericInit(VOID);
static BOOL UTPEPDEVICESAPI lGenericUninit(VOID);

static VOID UTPEPDEVICESAPI lGenericReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI lGenericVerifyDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);

static VOID UTPEPDEVICESAPI l82S183ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI l82S183VerifyDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);

static VOID UTPEPDEVICESAPI l82S191ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI l82S191VerifyDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);

static VOID UTPEPDEVICESAPI l82S321ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI l82S321VerifyDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);

static VOID UTPEPDEVICESAPI l82HS641ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI l82HS641VerifyDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);

static VOID UTPEPDEVICESAPI lS114LatchedReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI lS114LatchedVerifyDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);

static VOID UTPEPDEVICESAPI lS114TransparentReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI lS114TransparentVerifyDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);

static VOID UTPEPDEVICESAPI lS115LatchedReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI lS115LatchedVerifyDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);

static VOID UTPEPDEVICESAPI lS115TransparentReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI lS115TransparentVerifyDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);

static LPCWSTR l_psz6336PinNames[] = {
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
    CDevicePin_HighChipEnable4,
    CDevicePin_HighChipEnable3,
    CDevicePin_LowChipEnable2,
    CDevicePin_LowChipEnable1,
    CDevicePin_NotUsed,
    CDevicePin_NotUsed,
    CDevicePin_Vcc};

static LPCWSTR l_psz74S474PinNames[] = {
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
    CDevicePin_HighChipEnable2,
    CDevicePin_HighChipEnable1,
    CDevicePin_LowChipSelect2,
    CDevicePin_LowChipSelect1,
    CDevicePin_NotUsed,
    CDevicePin_Address8,
    CDevicePin_Vcc};

static LPCWSTR l_pszMB7060PinNames[] = {
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
    CDevicePin_NotUsed,
    CDevicePin_NotUsed,
    CDevicePin_LowChipSelect,
    CDevicePin_NotUsed,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Vcc};

static LPCWSTR l_pszMB7131PinNames[] = {
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
    CDevicePin_HighChipEnable4,
    CDevicePin_HighChipEnable3,
    CDevicePin_LowChipEnable2,
    CDevicePin_LowChipEnable1,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Vcc};

static LPCWSTR l_psz82S114PinNames[] = {
    CDevicePin_Address3,
    CDevicePin_Address4,
    CDevicePin_NotUsed,
    CDevicePin_Address5,
    CDevicePin_Address6,
    CDevicePin_Address7,
    CDevicePin_Data0,
    CDevicePin_Data1,
    CDevicePin_Data2,
    CDevicePin_Data3,
    CDevicePin_HighFE2,
    CDevicePin_Ground,
    CDevicePin_HighFE1,
    CDevicePin_Data4,
    CDevicePin_Data5,
    CDevicePin_Data6,
    CDevicePin_Data7,
    CDevicePin_Strobe,
    CDevicePin_HighChipEnable2,
    CDevicePin_LowChipEnable1,
    CDevicePin_Address0,
    CDevicePin_Address1,
    CDevicePin_Address2,
    CDevicePin_Vcc};

static LPCWSTR l_psz82S115PinNames[] = {
    CDevicePin_Address3,
    CDevicePin_Address4,
    CDevicePin_Address5,
    CDevicePin_Address6,
    CDevicePin_Address7,
    CDevicePin_Address8,
    CDevicePin_Data0,
    CDevicePin_Data1,
    CDevicePin_Data2,
    CDevicePin_Data3,
    CDevicePin_HighFE2,
    CDevicePin_Ground,
    CDevicePin_HighFE1,
    CDevicePin_Data4,
    CDevicePin_Data5,
    CDevicePin_Data6,
    CDevicePin_Data7,
    CDevicePin_Strobe,
    CDevicePin_HighChipEnable2,
    CDevicePin_LowChipEnable1,
    CDevicePin_Address0,
    CDevicePin_Address1,
    CDevicePin_Address2,
    CDevicePin_Vcc};

static LPCWSTR l_psz82S183PinNames[] = {
    CDevicePin_Address7,
    CDevicePin_Address6,
    CDevicePin_Address5,
    CDevicePin_Address4,
    CDevicePin_Address3,
    CDevicePin_Address0,
    CDevicePin_Address1,
    CDevicePin_Address2,
    CDevicePin_Data0,
    CDevicePin_Data1,
    CDevicePin_Data2,
    CDevicePin_Ground,
    CDevicePin_Data3,
    CDevicePin_Data4,
    CDevicePin_Data5,
    CDevicePin_Data6,
    CDevicePin_Data7,
    CDevicePin_Strobe,
    CDevicePin_HighChipEnable3,
    CDevicePin_LowChipEnable2,
    CDevicePin_LowChipEnable1,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Vcc};

static LPCWSTR l_psz82S191PinNames[] = {
    CDevicePin_Address7,
    CDevicePin_Address6,
    CDevicePin_Address5,
    CDevicePin_Address4,
    CDevicePin_Address3,
    CDevicePin_Address0,
    CDevicePin_Address1,
    CDevicePin_Address2,
    CDevicePin_Data0,
    CDevicePin_Data1,
    CDevicePin_Data2,
    CDevicePin_Ground,
    CDevicePin_Data3,
    CDevicePin_Data4,
    CDevicePin_Data5,
    CDevicePin_Data6,
    CDevicePin_Data7,
    CDevicePin_HighChipEnable3,
    CDevicePin_HighChipEnable2,
    CDevicePin_LowChipEnable1,
    CDevicePin_Address10,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Vcc};

static LPCWSTR l_psz82S321PinNames[] = {
    CDevicePin_Address7,
    CDevicePin_Address6,
    CDevicePin_Address5,
    CDevicePin_Address4,
    CDevicePin_Address3,
    CDevicePin_Address0,
    CDevicePin_Address1,
    CDevicePin_Address2,
    CDevicePin_Data0,
    CDevicePin_Data1,
    CDevicePin_Data2,
    CDevicePin_Ground,
    CDevicePin_Data3,
    CDevicePin_Data4,
    CDevicePin_Data5,
    CDevicePin_Data6,
    CDevicePin_Data7,
    CDevicePin_HighChipEnable2,
    CDevicePin_Address11,
    CDevicePin_LowChipEnable1,
    CDevicePin_Address10,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Vcc};

static LPCWSTR l_psz82HS641PinNames[] = {
    CDevicePin_Address7,
    CDevicePin_Address6,
    CDevicePin_Address5,
    CDevicePin_Address4,
    CDevicePin_Address3,
    CDevicePin_Address0,
    CDevicePin_Address1,
    CDevicePin_Address2,
    CDevicePin_Data0,
    CDevicePin_Data1,
    CDevicePin_Data2,
    CDevicePin_Ground,
    CDevicePin_Data3,
    CDevicePin_Data4,
    CDevicePin_Data5,
    CDevicePin_Data6,
    CDevicePin_Data7,
    CDevicePin_Address12,
    CDevicePin_Address11,
    CDevicePin_LowChipEnable,
    CDevicePin_Address10,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Vcc};

DEVICES_BEGIN
    DEVICE_DATA_BEGIN(6336-1 (Untested))
        DEVICE_PROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz6336PinNames)
        DEVICE_ADAPTER("Part# KEPROM24")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS(60)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0100)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_NOTSUPPORTED
            DEVICE_ROM_READ_FUNC(lGenericReadDevice)
            DEVICE_ROM_PROGRAM_FUNC_NONE
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(74S474 (Untested))
        DEVICE_PROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz74S474PinNames)
        DEVICE_ADAPTER("Part# KEPROM24")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS(60)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0200)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_NOTSUPPORTED
            DEVICE_ROM_READ_FUNC(lGenericReadDevice)
            DEVICE_ROM_PROGRAM_FUNC_NONE
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(MB7060 (Untested))
        DEVICE_PROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_pszMB7060PinNames)
        DEVICE_ADAPTER("Part# KEPROM24")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS(70)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0400)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_NOTSUPPORTED
            DEVICE_ROM_READ_FUNC(lGenericReadDevice)
            DEVICE_ROM_PROGRAM_FUNC_NONE
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(MB7131 (Untested))
        DEVICE_PROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_pszMB7131PinNames)
        DEVICE_ADAPTER("Part# KEPROM24")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS(70)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0400)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_NOTSUPPORTED
            DEVICE_ROM_READ_FUNC(lGenericReadDevice)
            DEVICE_ROM_PROGRAM_FUNC_NONE
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(82S183 (Untested))
        DEVICE_PROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz82S183PinNames)
        DEVICE_ADAPTER("Part# KEPROM24")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS(60)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0400)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_NOTSUPPORTED
            DEVICE_ROM_READ_FUNC(l82S183ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC_NONE
            DEVICE_ROM_VERIFY_FUNC(l82S183VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(82S191 (Untested))
        DEVICE_PROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz82S191PinNames)
        DEVICE_ADAPTER("Part# KEPROM24")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS(80)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0800)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_NOTSUPPORTED
            DEVICE_ROM_READ_FUNC(l82S191ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC_NONE
            DEVICE_ROM_VERIFY_FUNC(l82S191VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(82S321 (Untested))
        DEVICE_PROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz82S321PinNames)
        DEVICE_ADAPTER("Part# KEPROM24")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS(80)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x1000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_NOTSUPPORTED
            DEVICE_ROM_READ_FUNC(l82S321ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC_NONE
            DEVICE_ROM_VERIFY_FUNC(l82S321VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(82HS641 (Untested))
        DEVICE_PROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz82HS641PinNames)
        DEVICE_ADAPTER("Part# KEPROM24")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, TRUE, TRUE, FALSE, TRUE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS(55)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x2000)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_NOTSUPPORTED
            DEVICE_ROM_READ_FUNC(l82HS641ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC_NONE
            DEVICE_ROM_VERIFY_FUNC(l82HS641VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(82S114 (Latched))
        DEVICE_PROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz82S114PinNames)
        DEVICE_ADAPTER("Part# KE82S11x")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS(60)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0100)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_NOTSUPPORTED
            DEVICE_ROM_READ_FUNC(lS114LatchedReadDevice)
            DEVICE_ROM_PROGRAM_FUNC_NONE
            DEVICE_ROM_VERIFY_FUNC(lS114LatchedVerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(82S114 (Transparent))
        DEVICE_PROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz82S114PinNames)
        DEVICE_ADAPTER("Part# KE82S11x")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS(60)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0100)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_NOTSUPPORTED
            DEVICE_ROM_READ_FUNC(lS114TransparentReadDevice)
            DEVICE_ROM_PROGRAM_FUNC_NONE
            DEVICE_ROM_VERIFY_FUNC(lS114TransparentVerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(82S115 (Latched))
        DEVICE_PROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz82S115PinNames)
        DEVICE_ADAPTER("Part# KE82S11x")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS(60)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0200)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_NOTSUPPORTED
            DEVICE_ROM_READ_FUNC(lS115LatchedReadDevice)
            DEVICE_ROM_PROGRAM_FUNC_NONE
            DEVICE_ROM_VERIFY_FUNC(lS115LatchedVerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(82S115 (Transparent))
        DEVICE_PROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_psz82S115PinNames)
        DEVICE_ADAPTER("Part# KE82S11x")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS(60)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x0200)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_NOTSUPPORTED
            DEVICE_ROM_READ_FUNC(lS115TransparentReadDevice)
            DEVICE_ROM_PROGRAM_FUNC_NONE
            DEVICE_ROM_VERIFY_FUNC(lS115TransparentVerifyDevice)
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

static VOID lGenericReadData(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  LPBYTE pbyData,
  ULONG ulDataLen,
  ULONG ulAddressMask)
{
    BOOL bErrorOccurred = FALSE;
    ULONG ulTmpBufferLen = ulDataLen / 32;
    ULONG ulAddress, ulTmpAddress;

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoRead);

    if (FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
        FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
        FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse4Mode) ||
        FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    for (ulAddress = 0; ulAddress < ulDataLen; ulAddress += ulTmpBufferLen)
    {
        ulTmpAddress = ulAddress | ulAddressMask;

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

static VOID lGenericVerifyData(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  const LPBYTE pbyData,
  ULONG ulDataLen,
  ULONG ulAddressMask)
{
    BOOL bErrorOccurred = FALSE;
    ULONG ulTmpBufferLen = ulDataLen / 32;
    LPBYTE pbyTmpBuffer = (LPBYTE)UtAllocMem(ulTmpBufferLen);
    ULONG ulAddress, ulTmpAddress, ulIndex;

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoVerify);

    if (FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
        FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
        FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse4Mode) ||
        FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    for (ulAddress = 0; ulAddress < ulDataLen; ulAddress += ulTmpBufferLen)
    {
        ulTmpAddress = ulAddress | ulAddressMask;

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

static VOID UTPEPDEVICESAPI lGenericReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    lGenericReadData(pDeviceIOFuncs, pbyData, ulDataLen,
                     CGeneric_CE1_AddressLine | CGeneric_CE2_AddressLine);
}

static VOID UTPEPDEVICESAPI lGenericVerifyDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
    lGenericVerifyData(pDeviceIOFuncs, pbyData, ulDataLen,
                       CGeneric_CE1_AddressLine | CGeneric_CE2_AddressLine);
}

/*
    82S183
*/

static VOID UTPEPDEVICESAPI l82S183ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    lGenericReadData(pDeviceIOFuncs, pbyData, ulDataLen,
                     C82S183_CS3_AddressLine | C82S183_Strobe_AddressLine);
}

static VOID UTPEPDEVICESAPI l82S183VerifyDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
    lGenericVerifyData(pDeviceIOFuncs, pbyData, ulDataLen,
                       C82S183_CS3_AddressLine | C82S183_Strobe_AddressLine);
}

/*
    82S191
*/

static VOID UTPEPDEVICESAPI l82S191ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    lGenericReadData(pDeviceIOFuncs, pbyData, ulDataLen,
                     C82S191_CE2_AddressLine | C82S191_CE3_AddressLine);
}

static VOID UTPEPDEVICESAPI l82S191VerifyDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
    lGenericVerifyData(pDeviceIOFuncs, pbyData, ulDataLen,
                       C82S191_CE2_AddressLine | C82S191_CE3_AddressLine);
}

/*
    82S321
*/

static VOID UTPEPDEVICESAPI l82S321ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    lGenericReadData(pDeviceIOFuncs, pbyData, ulDataLen,
                     C82S321_CE2_AddressLine);
}

static VOID UTPEPDEVICESAPI l82S321VerifyDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
    lGenericVerifyData(pDeviceIOFuncs, pbyData, ulDataLen,
                       C82S321_CE2_AddressLine);
}

/*
    82HS641
*/

static VOID UTPEPDEVICESAPI l82HS641ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    lGenericReadData(pDeviceIOFuncs, pbyData, ulDataLen, 0);
}

static VOID UTPEPDEVICESAPI l82HS641VerifyDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
    lGenericVerifyData(pDeviceIOFuncs, pbyData, ulDataLen, 0);
}

/*
   82S114 Latched Device Functions
*/

static BOOL lS114LatchedRead(
  ULONG ulAddress,
  LPBYTE pbyData)
{
    TUtPepCtrlReadUserData ReadUserData[4];
    ULONG ulTmpAddress;

    /*
     * The 82S114 has address lines A5 - A7 located at pins 4 - 6
     * while the 82S115 has address lines A5 - A7 located at pins 3 - 5.
     */

    ulTmpAddress = ulAddress & 0x1F;
    ulTmpAddress |= (ulAddress & 0xE0) << 1;

    ReadUserData[0].nAddress = ulTmpAddress;
    ReadUserData[0].OutputEnableMode = eUtPepCtrlIgnoreOE;
    ReadUserData[0].bPerformRead = FALSE;

    ulTmpAddress |= C82S11x_CE2_AddressLine;

    ReadUserData[1].nAddress = ulTmpAddress;
    ReadUserData[1].OutputEnableMode = eUtPepCtrlIgnoreOE;
    ReadUserData[1].bPerformRead = FALSE;

    ulTmpAddress |= C82S11x_Strobe_AddressLine;

    ReadUserData[2].nAddress = ulTmpAddress;
    ReadUserData[2].OutputEnableMode = eUtPepCtrlIgnoreOE;
    ReadUserData[2].bPerformRead = TRUE;

    /* Turn off CE2 and Strobe */

    ReadUserData[3].nAddress = 0;
    ReadUserData[3].OutputEnableMode = eUtPepCtrlIgnoreOE;
    ReadUserData[3].bPerformRead = FALSE;

    if (FALSE == UtPepCtrlReadUserData(ReadUserData, MArrayLen(ReadUserData),
                                       pbyData, 1))
    {
        return FALSE;
    }

    return TRUE;
}

static VOID UTPEPDEVICESAPI lS114LatchedReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    BOOL bErrorOccurred = FALSE;
    ULONG ulAddress;

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoRead);

    if (FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
        FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
        FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse4Mode) ||
        FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    for (ulAddress = 0; ulAddress < ulDataLen; ulAddress += 1)
    {
        if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc() ||
            FALSE == lS114LatchedRead(ulAddress, &pbyData[ulAddress]))
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

static VOID UTPEPDEVICESAPI lS114LatchedVerifyDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
    BOOL bErrorOccurred = FALSE;
    BYTE byTmpBuffer[1];
    ULONG ulAddress;

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoVerify);

    if (FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
        FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
        FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse4Mode) ||
        FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    for (ulAddress = 0; ulAddress < ulDataLen; ulAddress += 1)
    {
        if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc() ||
            FALSE == lS114LatchedRead(ulAddress, byTmpBuffer))
        {
            bErrorOccurred = TRUE;

            goto End;
        }

        if (pbyData[ulAddress] != byTmpBuffer[0])
        {
            pDeviceIOFuncs->pVerifyByteErrorDeviceIOFunc(ulAddress,
                                                         pbyData[ulAddress],
                                                         byTmpBuffer[0]);
        }

        pDeviceIOFuncs->pProgressDeviceIOFunc(ulAddress);
    }

End:
    UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode);

    pDeviceIOFuncs->pEndDeviceIOFunc(bErrorOccurred, edoVerify);
}

/*
   82S114 Transparent Device Functions
*/

static BOOL lS114TransparentRead(
  ULONG ulAddress,
  LPBYTE pbyData)
{
    TUtPepCtrlReadUserData ReadUserData[4];
    ULONG ulTmpAddress;

    /*
     * The 82S114 has address lines A5 - A7 located at pins 4 - 6
     * while the 82S115 has address lines A5 - A7 located at pins 3 - 5.
     */

    ulTmpAddress = ulAddress & 0x1F;
    ulTmpAddress |= (ulAddress & 0xE0) << 1;

    ReadUserData[0].nAddress = ulTmpAddress;
    ReadUserData[0].OutputEnableMode = eUtPepCtrlIgnoreOE;
    ReadUserData[0].bPerformRead = FALSE;

    ulTmpAddress |= C82S11x_Strobe_AddressLine;

    ReadUserData[1].nAddress = ulTmpAddress;
    ReadUserData[1].OutputEnableMode = eUtPepCtrlIgnoreOE;
    ReadUserData[1].bPerformRead = FALSE;
        
    ulTmpAddress |= C82S11x_CE2_AddressLine;

    ReadUserData[2].nAddress = ulTmpAddress;
    ReadUserData[2].OutputEnableMode = eUtPepCtrlIgnoreOE;
    ReadUserData[2].bPerformRead = TRUE;

    /* Turn off CE2 and Strobe */

    ReadUserData[3].nAddress = 0;
    ReadUserData[3].OutputEnableMode = eUtPepCtrlIgnoreOE;
    ReadUserData[3].bPerformRead = FALSE;

    if (FALSE == UtPepCtrlReadUserData(ReadUserData, MArrayLen(ReadUserData),
                                       pbyData, 1))
    {
        return FALSE;
    }

    return TRUE;
}

static VOID UTPEPDEVICESAPI lS114TransparentReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    BOOL bErrorOccurred = FALSE;
    ULONG ulAddress;

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoRead);

    if (FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
        FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
        FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse4Mode) ||
        FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    for (ulAddress = 0; ulAddress < ulDataLen; ulAddress += 1)
    {
        if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc() ||
            FALSE == lS114TransparentRead(ulAddress, &pbyData[ulAddress]))
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

static VOID UTPEPDEVICESAPI lS114TransparentVerifyDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
    BOOL bErrorOccurred = FALSE;
    BYTE byTmpBuffer[1];
    ULONG ulAddress;

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoVerify);

    if (FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
        FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
        FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse4Mode) ||
        FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    for (ulAddress = 0; ulAddress < ulDataLen; ulAddress += 1)
    {
        if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc() ||
            FALSE == lS114TransparentRead(ulAddress, byTmpBuffer))
        {
            bErrorOccurred = TRUE;

            goto End;
        }

        if (pbyData[ulAddress] != byTmpBuffer[0])
        {
            pDeviceIOFuncs->pVerifyByteErrorDeviceIOFunc(ulAddress,
                                                         pbyData[ulAddress],
                                                         byTmpBuffer[0]);
        }

        pDeviceIOFuncs->pProgressDeviceIOFunc(ulAddress);
    }

End:
    UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode);

    pDeviceIOFuncs->pEndDeviceIOFunc(bErrorOccurred, edoVerify);
}

/*
   82S115 Latched Device Functions
*/

static BOOL lS115LatchedRead(
  ULONG ulAddress,
  LPBYTE pbyData)
{
    TUtPepCtrlReadUserData ReadUserData[4];

    ReadUserData[0].nAddress = ulAddress;
    ReadUserData[0].OutputEnableMode = eUtPepCtrlIgnoreOE;
    ReadUserData[0].bPerformRead = FALSE;

    ulAddress |= C82S11x_CE2_AddressLine;

    ReadUserData[1].nAddress = ulAddress;
    ReadUserData[1].OutputEnableMode = eUtPepCtrlIgnoreOE;
    ReadUserData[1].bPerformRead = FALSE;
        
    ulAddress |= C82S11x_Strobe_AddressLine;

    ReadUserData[2].nAddress = ulAddress;
    ReadUserData[2].OutputEnableMode = eUtPepCtrlIgnoreOE;
    ReadUserData[2].bPerformRead = TRUE;

    /* Turn off CE2 and Strobe */

    ReadUserData[3].nAddress = 0;
    ReadUserData[3].OutputEnableMode = eUtPepCtrlIgnoreOE;
    ReadUserData[3].bPerformRead = FALSE;

    if (FALSE == UtPepCtrlReadUserData(ReadUserData, MArrayLen(ReadUserData),
                                       pbyData, 1))
    {
        return FALSE;
    }

    return TRUE;
}

static VOID UTPEPDEVICESAPI lS115LatchedReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    BOOL bErrorOccurred = FALSE;
    ULONG ulAddress;

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoRead);

    if (FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
        FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
        FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse4Mode) ||
        FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    for (ulAddress = 0; ulAddress < ulDataLen; ulAddress += 1)
    {
        if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc() ||
            FALSE == lS115LatchedRead(ulAddress, &pbyData[ulAddress]))
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

static VOID UTPEPDEVICESAPI lS115LatchedVerifyDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
    BOOL bErrorOccurred = FALSE;
    BYTE byTmpBuffer[1];
    ULONG ulAddress;

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoVerify);

    if (FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
        FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
        FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse4Mode) ||
        FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    for (ulAddress = 0; ulAddress < ulDataLen; ulAddress += 1)
    {
        if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc() ||
            FALSE == lS115LatchedRead(ulAddress, byTmpBuffer))
        {
            bErrorOccurred = TRUE;

            goto End;
        }

        if (pbyData[ulAddress] != byTmpBuffer[0])
        {
            pDeviceIOFuncs->pVerifyByteErrorDeviceIOFunc(ulAddress,
                                                         pbyData[ulAddress],
                                                         byTmpBuffer[0]);
        }

        pDeviceIOFuncs->pProgressDeviceIOFunc(ulAddress);
    }

End:
    UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode);

    pDeviceIOFuncs->pEndDeviceIOFunc(bErrorOccurred, edoVerify);
}

/*
   82S115 Transparent Device Functions
*/

static BOOL lS115TransparentRead(
  ULONG ulAddress,
  LPBYTE pbyData)
{
    TUtPepCtrlReadUserData ReadUserData[4];

    ReadUserData[0].nAddress = ulAddress;
    ReadUserData[0].OutputEnableMode = eUtPepCtrlIgnoreOE;
    ReadUserData[0].bPerformRead = FALSE;

    ulAddress |= C82S11x_Strobe_AddressLine;

    ReadUserData[1].nAddress = ulAddress;
    ReadUserData[1].OutputEnableMode = eUtPepCtrlIgnoreOE;
    ReadUserData[1].bPerformRead = FALSE;
        
    ulAddress |= C82S11x_CE2_AddressLine;

    ReadUserData[2].nAddress = ulAddress;
    ReadUserData[2].OutputEnableMode = eUtPepCtrlIgnoreOE;
    ReadUserData[2].bPerformRead = TRUE;

    /* Turn off CE2 and Strobe */

    ReadUserData[3].nAddress = 0;
    ReadUserData[3].OutputEnableMode = eUtPepCtrlIgnoreOE;
    ReadUserData[3].bPerformRead = FALSE;

    if (FALSE == UtPepCtrlReadUserData(ReadUserData, MArrayLen(ReadUserData),
                                       pbyData, 1))
    {
        return FALSE;
    }

    return TRUE;
}

static VOID UTPEPDEVICESAPI lS115TransparentReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    BOOL bErrorOccurred = FALSE;
    ULONG ulAddress;

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoRead);

    if (FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
        FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
        FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse4Mode) ||
        FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    for (ulAddress = 0; ulAddress < ulDataLen; ulAddress += 1)
    {
        if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc() ||
            FALSE == lS115TransparentRead(ulAddress, &pbyData[ulAddress]))
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

static VOID UTPEPDEVICESAPI lS115TransparentVerifyDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
    BOOL bErrorOccurred = FALSE;
    BYTE byTmpBuffer[1];
    ULONG ulAddress;

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoVerify);

    if (FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
        FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
        FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse4Mode) ||
        FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    for (ulAddress = 0; ulAddress < ulDataLen; ulAddress += 1)
    {
        if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc() ||
            FALSE == lS115TransparentRead(ulAddress, byTmpBuffer))
        {
            bErrorOccurred = TRUE;

            goto End;
        }

        if (pbyData[ulAddress] != byTmpBuffer[0])
        {
            pDeviceIOFuncs->pVerifyByteErrorDeviceIOFunc(ulAddress,
                                                         pbyData[ulAddress],
                                                         byTmpBuffer[0]);
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
