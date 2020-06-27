/***************************************************************************/
/*  Copyright (C) 2016-2020 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>
#include <strsafe.h>

#include <UtilsDevice/UtPepDevices.h>

#include <Devices/Includes/UtDevicePinDefs.h>

#include <Config/UtPepCtrl.h>

#include <Includes/UtMacros.h>

#include <Utils/UtHeap.h>

#include <Devices/Includes/UtGenericFuncs.inl>

static VOID UTPEPDEVICESAPI lFake8BitReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI lFake8BitProgramDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI lFake8BitVerifyDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);

static VOID UTPEPDEVICESAPI lFake16BitReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI lFake16BitProgramDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI lFake16BitVerifyDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);

static LPCWSTR l_pszFake8BitPinNames[] = {
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

static LPCWSTR l_pszFake16BitPinNames[] = {
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
    L"" };

DEVICES_BEGIN
    DEVICE_DATA_BEGIN(Fake8Bit)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(66)
        DEVICE_PINNAMES(l_pszFake8BitPinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
        DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0xFFFF)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_25VDC
            DEVICE_ROM_READ_FUNC(lFake8BitReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(lFake8BitProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lFake8BitVerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(Fake16Bit)
        DEVICE_EPROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(66)
        DEVICE_PINNAMES(l_pszFake16BitPinNames)
        DEVICE_ADAPTER_NONE
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
        DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0xFFFF)
            DEVICE_ROM_BITSPERVALUE(16)
            DEVICE_ROM_VPP_12dot75VDC
            DEVICE_ROM_READ_FUNC(lFake16BitReadDevice)
            DEVICE_ROM_PROGRAM_FUNC(lFake16BitProgramDevice)
            DEVICE_ROM_VERIFY_FUNC(lFake16BitVerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
DEVICES_END

/*
    Fake 8-Bit Device Functions
*/

static VOID UTPEPDEVICESAPI lFake8BitReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    BOOL bErrorOccurred = FALSE;
    ULONG ulAddress;

	nChipEnableNanoseconds;
	nOutputEnableNanoseconds;

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoRead);

    for (ulAddress = 0; ulAddress < ulDataLen; ++ulAddress)
    {
        pbyData[ulAddress] = 0xFF;

        if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc())
        {
            bErrorOccurred = TRUE;

            break;
        }

        pDeviceIOFuncs->pProgressDeviceIOFunc(ulAddress);
    }

    pDeviceIOFuncs->pEndDeviceIOFunc(bErrorOccurred, edoRead);
}

static VOID UTPEPDEVICESAPI lFake8BitProgramDevice(
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

static VOID UTPEPDEVICESAPI lFake8BitVerifyDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
    BOOL bErrorOccurred = FALSE;
    ULONG ulAddress;

	nChipEnableNanoseconds;
	nOutputEnableNanoseconds;

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoRead);

    for (ulAddress = 0; ulAddress < ulDataLen; ++ulAddress)
    {
        if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc())
        {
            bErrorOccurred = TRUE;

            break;
        }

        pDeviceIOFuncs->pVerifyByteErrorDeviceIOFunc(ulAddress,
                                                     pbyData[ulAddress],
                                                     0x00);

        pDeviceIOFuncs->pProgressDeviceIOFunc(ulAddress);
    }

    pDeviceIOFuncs->pEndDeviceIOFunc(bErrorOccurred, edoRead);
}

/*
    Fake 16-Bit Device Functions
*/

static VOID UTPEPDEVICESAPI lFake16BitReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    BOOL bErrorOccurred = FALSE;
    ULONG ulAddress;

	nChipEnableNanoseconds;
	nOutputEnableNanoseconds;

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoRead);

    for (ulAddress = 0; ulAddress < ulDataLen / sizeof(WORD); ++ulAddress)
    {
        *((LPWORD)pbyData + ulAddress) = 0xFFFF;

        if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc())
        {
            bErrorOccurred = TRUE;

            break;
        }

        pDeviceIOFuncs->pProgressDeviceIOFunc(ulAddress);
    }

    pDeviceIOFuncs->pEndDeviceIOFunc(bErrorOccurred, edoRead);
}

static VOID UTPEPDEVICESAPI lFake16BitProgramDevice(
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

static VOID UTPEPDEVICESAPI lFake16BitVerifyDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
    BOOL bErrorOccurred = FALSE;
    ULONG ulAddress;

	nChipEnableNanoseconds;
	nOutputEnableNanoseconds;

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoRead);

    for (ulAddress = 0; ulAddress < ulDataLen / sizeof(WORD); ++ulAddress)
    {
        if (FALSE == pDeviceIOFuncs->pContinueDeviceIOFunc())
        {
            bErrorOccurred = TRUE;

            break;
        }

        pDeviceIOFuncs->pVerifyWordErrorDeviceIOFunc(ulAddress,
                                                     *((LPWORD)pbyData + ulAddress),
                                                     0x0000);

        pDeviceIOFuncs->pProgressDeviceIOFunc(ulAddress);
    }

    pDeviceIOFuncs->pEndDeviceIOFunc(bErrorOccurred, edoRead);
}

/***************************************************************************/
/*  Copyright (C) 2016-2020 Kevin Eshbach                                  */
/***************************************************************************/
