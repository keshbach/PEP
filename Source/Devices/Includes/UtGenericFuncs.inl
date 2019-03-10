/***************************************************************************/
/*  Copyright (C) 2007-2013 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(CGenericBytesPerOperation)
#define CGenericBytesPerOperation 32
#endif

static VOID lGenericReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  LPBYTE pbyData,
  ULONG ulDataLen,
  EUtPepCtrlPinPulseMode PinPulseMode)
{
    BOOL bErrorOccurred = FALSE;
    ULONG ulTmpBufferLen = ulDataLen / CGenericBytesPerOperation;
    ULONG ulAddress;

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoRead);

    if (FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
        FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
        FALSE == UtPepCtrlSetPinPulseMode(PinPulseMode) ||
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

    pDeviceIOFuncs->pProgressDeviceIOFunc(ulDataLen);

End:
    UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode);

    pDeviceIOFuncs->pEndDeviceIOFunc(bErrorOccurred, edoRead);
}

static VOID lGenericVerifyDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  const LPBYTE pbyData,
  ULONG ulDataLen,
  EUtPepCtrlPinPulseMode PinPulseMode)
{
    BOOL bErrorOccurred = FALSE;
    ULONG ulTmpBufferLen = ulDataLen / CGenericBytesPerOperation;
    LPBYTE pbyTmpBuffer = (LPBYTE)UtAllocMem(ulTmpBufferLen);
    ULONG ulAddress, ulIndex;

    pDeviceIOFuncs->pBeginDeviceIOFunc(ulDataLen, edoVerify);

    if (FALSE == UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode) ||
        FALSE == UtPepCtrlSetVccMode(eUtPepCtrl5VDCMode) ||
        FALSE == UtPepCtrlSetPinPulseMode(PinPulseMode) ||
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

    pDeviceIOFuncs->pProgressDeviceIOFunc(ulDataLen);

End:
    UtPepCtrlSetProgrammerMode(eUtPepCtrlProgrammerNoneMode);

    pDeviceIOFuncs->pEndDeviceIOFunc(bErrorOccurred, edoVerify);

    UtFreeMem(pbyTmpBuffer);
}

static BOOL UTPEPDEVICESAPI lGenericInit(VOID)
{
    return UtInitHeap() ? TRUE : FALSE;
}

static BOOL UTPEPDEVICESAPI lGenericUninit(VOID)
{
    return UtUninitHeap() ? TRUE : FALSE;
}

static VOID UTPEPDEVICESAPI lGenericReadDevicePinPulse4Mode(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    lGenericReadDevice(pDeviceIOFuncs, pbyData, ulDataLen, eUtPepCtrlPinPulse4Mode);
}

static VOID UTPEPDEVICESAPI lGenericVerifyDevicePinPulse4Mode(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  const LPBYTE pbyData,
  ULONG ulDataLen)
{
    lGenericVerifyDevice(pDeviceIOFuncs, pbyData, ulDataLen, eUtPepCtrlPinPulse4Mode);
}

/***************************************************************************/
/*  Copyright (C) 2007-2013 Kevin Eshbach                                  */
/***************************************************************************/
