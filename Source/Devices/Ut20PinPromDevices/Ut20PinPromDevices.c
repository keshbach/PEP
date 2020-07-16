/***************************************************************************/
/*  Copyright (C) 2009-2020 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>
#include <strsafe.h>

#include <UtilsDevice/UtPepDevices.h>

#include <Devices/Includes/UtDevicePinDefs.h>

#include <Config/UtPepCtrl.h>

#include <Includes/UtMacros.h>

#include <Utils/UtHeap.h>

#include <Devices/Includes/UtGenericFuncs.inl>

static VOID UTPEPDEVICESAPI l74LS471ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI l74LS471VerifyDevice(const TDeviceIOFuncs* pDeviceIOFuncs, UINT32 nChipEnableNanoseconds, UINT32 nOutputEnableNanoseconds, const LPBYTE pbyData, ULONG ulDataLen);

static LPCWSTR l_psz74S472PinNames[] = {
    CDevicePin_Address0,
    CDevicePin_Address1,
    CDevicePin_Address2,
    CDevicePin_Address3,
    CDevicePin_Address4,
    CDevicePin_Data0,
    CDevicePin_Data1,
    CDevicePin_Data2,
    CDevicePin_Data3,
    CDevicePin_Ground,
    CDevicePin_Data4,
    CDevicePin_Data5,
    CDevicePin_Data6,
    CDevicePin_Data7,
    CDevicePin_LowChipEnable,
    CDevicePin_Address5,
    CDevicePin_Address6,
    CDevicePin_Address7,
    CDevicePin_Address8,
    CDevicePin_Vcc};

static LPCWSTR l_psz74LS471PinNames[] = {
    CDevicePin_Address0,
    CDevicePin_Address1,
    CDevicePin_Address2,
    CDevicePin_Address3,
    CDevicePin_Address4,
    CDevicePin_Data0,
    CDevicePin_Data1,
    CDevicePin_Data2,
    CDevicePin_Data3,
    CDevicePin_Ground,
    CDevicePin_Data4,
    CDevicePin_Data5,
    CDevicePin_Data6,
    CDevicePin_Data7,
    CDevicePin_LowChipEnable1,
    CDevicePin_LowChipEnable2,
    CDevicePin_Address5,
    CDevicePin_Address6,
    CDevicePin_Address7,
    CDevicePin_Vcc};

DEVICES_BEGIN
    DEVICE_DATA_BEGIN(74S472)
        DEVICE_PROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(20)
        DEVICE_PINNAMES(l_psz74S472PinNames)
        DEVICE_ADAPTER("Part# KE74S472/74LS471")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS(60)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x200)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_NOTSUPPORTED
            DEVICE_ROM_READ_FUNC(lGenericReadDevicePinPulse4Mode)
            DEVICE_ROM_PROGRAM_FUNC_NONE
            DEVICE_ROM_VERIFY_FUNC(lGenericVerifyDevicePinPulse4Mode)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(74LS471)
        DEVICE_PROM
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(20)
        DEVICE_PINNAMES(l_psz74LS471PinNames)
        DEVICE_ADAPTER("Part# KE74S472/74LS471")
        DEVICE_MESSAGE_NONE
        DEVICE_DIPSWITCHES(TRUE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS(60)
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_ROM_DATA_BEGIN
            DEVICE_ROM_SIZE(0x100)
            DEVICE_ROM_BITSPERVALUE(8)
            DEVICE_ROM_VPP_NOTSUPPORTED
            DEVICE_ROM_READ_FUNC(l74LS471ReadDevice)
            DEVICE_ROM_PROGRAM_FUNC_NONE
            DEVICE_ROM_VERIFY_FUNC(l74LS471VerifyDevice)
        DEVICE_ROM_DATA_END
    DEVICE_DATA_END
DEVICES_END

static VOID UTPEPDEVICESAPI l74LS471ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    BOOL bErrorOccurred = FALSE;
    ULONG ulTmpBufferLen = ulDataLen / 32; // Do not change!
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
        ulTmpAddress = ulAddress & 0x1F;
        ulTmpAddress |= ((ulAddress << 1) & 0x01C0);

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

static VOID UTPEPDEVICESAPI l74LS471VerifyDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  UINT32 nChipEnableNanoseconds,
  UINT32 nOutputEnableNanoseconds,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
    BOOL bErrorOccurred = FALSE;
    ULONG ulTmpBufferLen = ulDataLen / 32; // Do not change!
    LPBYTE pbyTmpBuffer = (LPBYTE)UtAllocMem(ulTmpBufferLen);
    ULONG ulAddress, ulTmpAddress, ulIndex;

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
        ulTmpAddress = ulAddress & 0x1F;
        ulTmpAddress |= ((ulAddress << 1) & 0x01C0);

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

    pDeviceIOFuncs->pEndDeviceIOFunc(bErrorOccurred, edoRead);

    UtFreeMem(pbyTmpBuffer);
}

/***************************************************************************/
/*  Copyright (C) 2009-2020 Kevin Eshbach                                  */
/***************************************************************************/
