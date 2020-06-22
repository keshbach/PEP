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

#define CAddressLine22 0x400000

/*
    Address Data Macro

    A8 - Address value
    A9 - Enable address data 
*/

#define CAddressDataOff 0
#define CAddressDataOn 1

#define CAddressDataEnable 1
#define CAddressDataDisable 0

#define MAddress7To20Data(addressBit, enable) \
    ((((ULONG)addressBit & 0x01) << 9) | (((ULONG)enable & 0x01) << 10))

#define MSelectAddressAndDataBits(address, selectDataBits) \
    ((((ULONG)address & 0x7F) << 1) | ((ULONG)selectDataBits & 0x01))

#define CAddress7To20 0xFFF80

#define CTotalUpperAddressPins 16

static BOOL UTPEPDEVICESAPI lGenericInit(VOID);
static BOOL UTPEPDEVICESAPI lGenericUninit(VOID);

static VOID UTPEPDEVICESAPI l27Cx00ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI l27Cx00ProgramDevice(const TDeviceIOFuncs* pDeviceIOFuncs, const LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI l27Cx00VerifyDevice(const TDeviceIOFuncs* pDeviceIOFuncs, const LPBYTE pbyData, ULONG ulDataLen);

static LPCWSTR l_psz27C8000PinNames[] = {
    CDevicePin_Address18,
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
    CDevicePin_Address8,
    CDevicePin_NotUsed};

static LPCWSTR l_psz27C160PinNames[] = {
    CDevicePin_Address18,
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
    CDevicePin_Address8,
    CDevicePin_Address19};

static LPCWSTR l_psz27C322PinNames[] = {
    CDevicePin_Address18,
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
    CDevicePin_OutputEnable_Vpp,
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
    CDevicePin_Data15,
    CDevicePin_Ground,
    CDevicePin_Address20,
    CDevicePin_Address16,
    CDevicePin_Address15,
    CDevicePin_Address14,
    CDevicePin_Address13,
    CDevicePin_Address12,
    CDevicePin_Address11,
    CDevicePin_Address10,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Address19};

static LPCWSTR l_psz23C640PinNames[] = {
    CDevicePin_Address18,
    CDevicePin_Address17,
    CDevicePin_Address7,
    CDevicePin_Address6,
    CDevicePin_Address5,
    CDevicePin_Address4,
    CDevicePin_Address3,
    CDevicePin_Address2,
    CDevicePin_Address1,
    CDevicePin_Address0,
    CDevicePin_Address21,
    CDevicePin_Ground,
    CDevicePin_OutputEnable_Vpp,
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
    CDevicePin_Data15,
    CDevicePin_Ground,
    CDevicePin_Address20,
    CDevicePin_Address16,
    CDevicePin_Address15,
    CDevicePin_Address14,
    CDevicePin_Address13,
    CDevicePin_Address12,
    CDevicePin_Address11,
    CDevicePin_Address10,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Address19};

static LPCWSTR l_psz23C6410PinNames[] = {
    CDevicePin_Address18,
    CDevicePin_Address17,
    CDevicePin_Address7,
    CDevicePin_Address6,
    CDevicePin_Address5,
    CDevicePin_Address4,
    CDevicePin_Address3,
    CDevicePin_Address2,
    CDevicePin_Address1,
    CDevicePin_Address0,
    CDevicePin_Address21,
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
    CDevicePin_Data15,
    CDevicePin_Ground,
    CDevicePin_Address20,
    CDevicePin_Address16,
    CDevicePin_Address15,
    CDevicePin_Address14,
    CDevicePin_Address13,
    CDevicePin_Address12,
    CDevicePin_Address11,
    CDevicePin_Address10,
    CDevicePin_Address9,
    CDevicePin_Address8,
    CDevicePin_Address19};

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
    L"",
    L"",
    L""};

DEVICES_BEGIN
    DEVICE_DATA_BEGIN(27C800)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(42)
        DEVICE_PINNAMES(l_psz27C8000PinNames)
        DEVICE_ADAPTER("Part# A42PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x100000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx00ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx00ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx00VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(MSM27C802)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(42)
        DEVICE_PINNAMES(l_psz27C8000PinNames)
        DEVICE_ADAPTER("Part# A42PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x100000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx00ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx00ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx00VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(MSM27C822)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(42)
        DEVICE_PINNAMES(l_pszPinNames)
        DEVICE_ADAPTER("Part# A42PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x100000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx00ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx00ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx00VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(uPDC27C8000?)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(42)
        DEVICE_PINNAMES(l_psz27C8000PinNames)
        DEVICE_ADAPTER("Part# A42PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x100000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx00ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx00ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx00VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(uPD27C8100?)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(42)
        DEVICE_PINNAMES(l_psz27C8000PinNames)
        DEVICE_ADAPTER("Part# A42PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x100000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx00ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx00ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx00VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(MX27C8111)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(42)
        DEVICE_PINNAMES(l_psz27C8000PinNames)
        DEVICE_ADAPTER("Part# A42PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x100000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx00ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx00ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx00VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(AT27C8192)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(42)
        DEVICE_PINNAMES(l_psz27C8000PinNames)
        DEVICE_ADAPTER("Part# A42PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x100000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx00ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx00ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx00VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C160)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(42)
        DEVICE_PINNAMES(l_psz27C160PinNames)
        DEVICE_ADAPTER("Part# A42PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x200000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx00ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx00ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx00VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(27C322)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(42)
        DEVICE_PINNAMES(l_psz27C322PinNames)
        DEVICE_ADAPTER("Part# A42PE")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x400000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(l27Cx00ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(l27Cx00ProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(l27Cx00VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(23C640 (Mask ROM))
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(42)
        DEVICE_PINNAMES(l_psz23C640PinNames)
        DEVICE_ADAPTER("Part# A42PE")
        DEVICE_MESSAGE("Set switch to 32 Meg - A21 to Pin 11")
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x800000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12VDC
            DEVICE_ROM_READ_FUNC(l27Cx00ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC_NONE
            DEVICE_ROM_VERIFY_FUNC(l27Cx00VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(MX23C6410)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(42)
        DEVICE_PINNAMES(l_psz23C6410PinNames)
        DEVICE_ADAPTER("Part# A42PE")
        DEVICE_MESSAGE("Set switch to 32 Meg - A21 to Pin 11")
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, FALSE, FALSE, TRUE, TRUE, TRUE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x800000)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12VDC
            DEVICE_ROM_READ_FUNC(l27Cx00ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC_NONE
            DEVICE_ROM_VERIFY_FUNC(l27Cx00VerifyDevice)
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

static BOOL lSetAddressLines7To20(
  ULONG ulAddress)
{
    TUtPepCtrlReadUserData ReadUserData[CTotalUpperAddressPins * 2];
    ULONG ulTmpAddress, ulIndex;
    BYTE byData;

    ulTmpAddress = ulAddress;

    for (ulIndex = 0; ulIndex < CTotalUpperAddressPins; ++ulIndex)
    {
        byData = (ulTmpAddress & CAddressLine22) ? CAddressDataOn : CAddressDataOff;

        ReadUserData[ulIndex * 2].nAddress = MAddress7To20Data(
                                                 byData, CAddressDataDisable);
        ReadUserData[ulIndex * 2].OutputEnableMode = eUtPepCtrlIgnoreOE;
        ReadUserData[ulIndex * 2].bPerformRead = FALSE;

        ReadUserData[(ulIndex * 2) + 1].nAddress = MAddress7To20Data(
                                                       byData, CAddressDataEnable);
        ReadUserData[(ulIndex * 2) + 1].OutputEnableMode = eUtPepCtrlIgnoreOE;
        ReadUserData[(ulIndex * 2) + 1].bPerformRead = FALSE;

        ulTmpAddress = ulTmpAddress >> 1;
    }

    if (!UtPepCtrlReadUserData(ReadUserData,
                               sizeof(ReadUserData) / sizeof(ReadUserData[0]),
                               NULL, 0))
    {
        return FALSE;
    }

    return TRUE;
}

static BOOL lReadAddressData(
  ULONG ulAddress,
  LPWORD pwData,
  ULONG ulDataLen)
{
    TUtPepCtrlReadUserData ReadUserData[2];
    ULONG ulTmpAddress, ulLastAddress, ulIndex;
    LPWORD pwTmpData;

    ulTmpAddress = ulAddress;
    ulLastAddress = 0;
    pwTmpData = pwData;

    for (ulIndex = 0; ulIndex < ulDataLen; ++ulIndex)
    {
        if ((ulTmpAddress & CAddress7To20) != (ulLastAddress & CAddress7To20))
        {
            if (FALSE == lSetAddressLines7To20(ulTmpAddress))
            {
                return FALSE;
            }
        }

        ReadUserData[0].nAddress = MSelectAddressAndDataBits(ulTmpAddress,
                                                             CDataBits0To7);
        ReadUserData[0].OutputEnableMode = eUtPepCtrlIgnoreOE;
        ReadUserData[0].bPerformRead = TRUE;

        ReadUserData[1].nAddress = MSelectAddressAndDataBits(ulTmpAddress,
                                                             CDataBits8To15);
        ReadUserData[1].OutputEnableMode = eUtPepCtrlIgnoreOE;
        ReadUserData[1].bPerformRead = TRUE;

        if (!UtPepCtrlReadUserData(ReadUserData,
                                   sizeof(ReadUserData) / sizeof(ReadUserData[0]),
                                   (LPBYTE)pwTmpData, sizeof(WORD)))
        {
            return FALSE;
        }

        ulLastAddress = ulTmpAddress;

        ++ulTmpAddress;
        ++pwTmpData;
    }

    return TRUE;
}

/*
   27Cx00 Device Functions
*/

static VOID UTPEPDEVICESAPI l27Cx00ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    BOOL bErrorOccurred = FALSE;
    ULONG ulTmpBufferLen = (ulDataLen / sizeof(WORD)) / 64;
    ULONG ulAddress;

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen / sizeof(WORD), edoRead);

    if (FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
        FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
        FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse4Mode) ||
        FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    if (FALSE == lSetAddressLines7To20(0))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    for (ulAddress = 0; ulAddress < ulDataLen / sizeof(WORD);
         ulAddress += ulTmpBufferLen)
    {
        if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc() ||
            FALSE == lReadAddressData(ulAddress, (LPWORD)pbyData + ulAddress,
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

static VOID UTPEPDEVICESAPI l27Cx00ProgramDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
    pDeviceIOFuncs;
    pbyData;
    ulDataLen;
}

static VOID UTPEPDEVICESAPI l27Cx00VerifyDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
    BOOL bErrorOccurred = FALSE;
    ULONG ulTmpBufferLen = ulDataLen / 8192;
    LPWORD pwTmpBuffer = (LPWORD)UtAllocMem(ulTmpBufferLen * sizeof(WORD));
    ULONG ulAddress, ulIndex;

    pbyData;
    ulDataLen;

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen / 2, edoVerify);

    if (FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
        FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
        FALSE == UtPepCtrlSetPinPulseMode(eUtPepCtrlPinPulse4Mode) ||
        FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerReadMode))
    {
        bErrorOccurred = TRUE;

        goto End;
    }

    for (ulAddress = 0; ulAddress < ulDataLen / 2; ulAddress += ulTmpBufferLen)
    {
        if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc()/* ||
            FALSE == UtPepCtrlReadWordData(ulAddress, pwTmpBuffer,
                                           ulTmpBufferLen)*/)
        {
            bErrorOccurred = TRUE;

            goto End;
        }

        for (ulIndex = 0; ulIndex < ulTmpBufferLen; ++ulIndex)
        {
            if (((LPWORD)pbyData)[ulAddress + ulIndex] != pwTmpBuffer[ulIndex])
            {
                pDeviceIOFuncs->pVerifyWordErrorDeviceIOFunc(
                    ulAddress + ulIndex,
                    ((LPWORD)pbyData)[ulAddress + ulIndex],
                    pwTmpBuffer[ulIndex]);
            }
        }

        pDeviceIOFuncs->pProgressDeviceIOFunc(ulAddress);
    }

End:
    UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode);

    pDeviceIOFuncs->pEndDeviceIOFunc(bErrorOccurred, edoVerify);

    UtFreeMem(pwTmpBuffer);
}

/***************************************************************************/
/*  Copyright (C) 2007-2020 Kevin Eshbach                                  */
/***************************************************************************/
