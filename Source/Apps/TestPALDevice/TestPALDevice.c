/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/

#include <stdio.h>
#include <conio.h>
#include <assert.h>

#include <windows.h>
#include <shlwapi.h>

#include <Config/UtPepCtrl.h>

#include <UtilsDevice/UtPepDevices.h>

#include <Includes/UtMacros.h>

#include <Utils/UtHeapProcess.h>
#include <Utils/UtConsole.h>

#include <UtilsDevice/UtPAL.h>

static BOOL l_bQuitDeviceOperation = FALSE;
static BOOL l_bErrorOccurred = FALSE;
static ULONG l_ulTotalAddressesDeviceIO = 0;

static BOOL WINAPI lHandlerRoutine(
  DWORD dwCtrlType)
{
    wprintf(L"Console ctrl handler invoked.\n");

    switch (dwCtrlType)
    {
        case CTRL_C_EVENT:
        case CTRL_BREAK_EVENT:
            break;
        case CTRL_CLOSE_EVENT:
            break;
    }

    l_bQuitDeviceOperation = TRUE;

    return TRUE;
}

static VOID UTPEPCTRLAPI lDeviceChangeFunc(
  EUtPepCtrlDeviceChange DeviceChange)
{
    switch (DeviceChange)
    {
        case eUtPepCtrlDeviceArrived:
            wprintf(L"TestPepCtrl: ** Device arrived **\n");
            break;
        case eUtPepCtrlDeviceRemoved:
            wprintf(L"TestPepCtrl: ** Device removed **\n");
            break;
        default:
            wprintf(L"TestPepCtrl: ** Unknown device change **\n");
            break;
    }
}

static VOID UTPEPDEVICESCALLBACK lBeginDeviceIO(
  ULONG ulTotalAddresses,
  EDeviceOperation DeviceOperation)
{
    l_ulTotalAddressesDeviceIO = ulTotalAddresses;

    switch (DeviceOperation)
    {
        case edoRead:
            wprintf(L"Beginning the read of a device with 0x%08x addresses.\n", ulTotalAddresses);
            break;
        case edoWrite:
            wprintf(L"Beginning the write of a device with 0x%08x addresses.\n", ulTotalAddresses);
            break;
        case edoProgram:
            wprintf(L"Beginning the programming of a device with 0x%08x addresses.\n", ulTotalAddresses);
            break;
        case edoVerify:
            wprintf(L"Beginning the verify of a device with 0x%08x addresses.\n", ulTotalAddresses);
            break;
    }
}

static VOID UTPEPDEVICESCALLBACK lEndDeviceIO(
  BOOL bErrorOccurred,
  EDeviceOperation DeviceOperation)
{
    switch (DeviceOperation)
    {
        case edoRead:
            wprintf(L"Ending the read of a device.\n");
            break;
        case edoWrite:
            wprintf(L"Ending the write of a device.\n");
            break;
        case edoProgram:
            wprintf(L"Ending the programming of a device.\n");
            break;
        case edoVerify:
            wprintf(L"Ending the verify of a device.\n");
            break;
    }

    if (bErrorOccurred)
    {
        wprintf(L"An error occurred.\n");
    }

    l_bErrorOccurred = bErrorOccurred;
}

static VOID UTPEPDEVICESCALLBACK lProgressDeviceIO(
  ULONG ulAddress)
{
    if (ulAddress > l_ulTotalAddressesDeviceIO)
    {
        wprintf(L"ERROR!  Progress device initialized with 0x%08x total "
                L"addresses, but the address 0x%08x was received.\n",
                l_ulTotalAddressesDeviceIO, ulAddress);

        wprintf(L"Press the \"Enter\" key to continue.\n");

        _getch();
    }

    wprintf(L"At address 0x%08x.\n", ulAddress);
}

static VOID UTPEPDEVICESCALLBACK lVerifyByteErrorDeviceIO(
  ULONG ulAddress,
  BYTE byExpected,
  BYTE byFound)
{
    if (ulAddress > l_ulTotalAddressesDeviceIO)
    {
        wprintf(L"ERROR!  Progress device initialized with 0x%08x total "
                L"addresses, but the address 0x%08x was received.\n",
                l_ulTotalAddressesDeviceIO, ulAddress);

        wprintf(L"Press the \"Enter\" key to continue.\n");

        _getch();
    }

    wprintf(L"Byte verify failed at address 0x%08x.\n", ulAddress);
    wprintf(L"Expected: 0x%02x, Found: 0x%02x\n", byExpected, byFound);
}

static VOID UTPEPDEVICESCALLBACK lVerifyWordErrorDeviceIO(
  ULONG ulAddress,
  WORD wExpected,
  WORD wFound)
{
    if (ulAddress > l_ulTotalAddressesDeviceIO)
    {
        wprintf(L"ERROR!  Progress device initialized with 0x%08x total "
                L"addresses, but the address 0x%08x was received.\n",
                l_ulTotalAddressesDeviceIO, ulAddress);

        wprintf(L"Press the \"Enter\" key to continue.\n");

        _getch();
    }

    wprintf(L"Word verify failed at address 0x%08x.\n", ulAddress);
    wprintf(L"Expected: 0x%04x, Found: 0x%04x\n", wExpected, wFound);
}

static BOOL UTPEPDEVICESCALLBACK lContinueDeviceIO(VOID)
{
    return (l_bQuitDeviceOperation == FALSE) ? TRUE : FALSE;
}

static VOID UTPEPDEVICESCALLBACK lShowMessageDeviceIO(
  LPCWSTR pszMessage)
{
    wprintf(L"Device Message:\n");
    wprintf(L"\n");
    wprintf(pszMessage);
    wprintf(L"\n");
    wprintf(L"\n");
    wprintf(L"Press the \"Enter\" key to continue.\n");

    _getch();
}

static VOID UTPEPDEVICESCALLBACK lLogMessageDeviceIO(
  LPCWSTR pszMessage)
{
    wprintf(L"Device Message to Log:\n");
    wprintf(L"\n");
    wprintf(pszMessage);
    wprintf(L"\n");
    wprintf(L"\n");
}

static void lInitDeviceIOFuncs(
  TDeviceIOFuncs* pDeviceIOFuncs)
{
    pDeviceIOFuncs->pBeginDeviceIOFunc = lBeginDeviceIO;
    pDeviceIOFuncs->pEndDeviceIOFunc = lEndDeviceIO;
    pDeviceIOFuncs->pProgressDeviceIOFunc = lProgressDeviceIO;
    pDeviceIOFuncs->pVerifyByteErrorDeviceIOFunc = lVerifyByteErrorDeviceIO;
    pDeviceIOFuncs->pVerifyWordErrorDeviceIOFunc = lVerifyWordErrorDeviceIO;
    pDeviceIOFuncs->pContinueDeviceIOFunc = lContinueDeviceIO;
    pDeviceIOFuncs->pShowMessageDeviceIOFunc = lShowMessageDeviceIO;
    pDeviceIOFuncs->pLogMessageDeviceIOFunc = lLogMessageDeviceIO;
}

static INT lParseNumber(
  LPCWSTR pszNumber)
{
    INT nNumberLen = lstrlenW(pszNumber);
    INT nNumber = 0;
    INT nMultiplier = 1;
    INT nNumberPos;

    for (nNumberPos = 1; nNumberPos < nNumberLen; ++nNumberPos)
    {
        nMultiplier *= 10;
    }

    for (nNumberPos = 0; nNumberPos < nNumberLen; ++nNumberPos)
    {
        switch (pszNumber[nNumberPos])
        {
            case L'0':
            case L'1':
            case L'2':
            case L'3':
            case L'4':
            case L'5':
            case L'6':
            case L'7':
            case L'8':
            case L'9':
                nNumber += (nMultiplier * (pszNumber[nNumberPos] - L'0'));
                break;
            default:
                return -1;
        }

        nMultiplier /= 10;
    }

    return nNumber;
}

static BOOL lAllocDevicePinConfigs(
  LPCWSTR pszArgs[],
  UINT nTotalArgs,
  TDevicePinConfig** ppDevicePinConfigs,
  UINT* pnTotalDevicePinConfigs)
{
    TDevicePinConfig* pTmpDevicePinConfigs;
    TDevicePinConfig* pCurDevicePinConfig;
    UINT nArgIndex;
    INT nPin;
  
    *ppDevicePinConfigs = NULL;
    *pnTotalDevicePinConfigs = 0;

    nArgIndex = 0;

    while (nArgIndex < nTotalArgs)
    {
        nPin = lParseNumber(pszArgs[nArgIndex]);

        if (nPin == -1)
        {
            return TRUE;
        }

        ++nArgIndex;

        if (nArgIndex == nTotalArgs)
        {
            return TRUE;
        }

        if (*ppDevicePinConfigs != NULL)
        {
            pTmpDevicePinConfigs = *ppDevicePinConfigs;

            *ppDevicePinConfigs = (TDevicePinConfig*)UtReAllocMem(
                *ppDevicePinConfigs,
                sizeof(TDevicePinConfig) * (*pnTotalDevicePinConfigs + 1));

            if (*ppDevicePinConfigs == NULL)
            {
                UtFreeMem(pTmpDevicePinConfigs);

                *pnTotalDevicePinConfigs = 0;

                return FALSE;
            }
        }
        else
        {
            *ppDevicePinConfigs = (TDevicePinConfig*)UtAllocMem(sizeof(TDevicePinConfig));

            if (*ppDevicePinConfigs == NULL)
            {
                return FALSE;
            }
        }

        pCurDevicePinConfig = *ppDevicePinConfigs + *pnTotalDevicePinConfigs;

        pCurDevicePinConfig->nPin = nPin;
        pCurDevicePinConfig->pszType = pszArgs[nArgIndex];
        pCurDevicePinConfig->pszPolarity = NULL;

        *pnTotalDevicePinConfigs += 1;

        ++nArgIndex;

        if (nArgIndex < nTotalArgs)
        {
            nPin = lParseNumber(pszArgs[nArgIndex]);

            if (nPin == -1)
            {
                pCurDevicePinConfig->pszPolarity = pszArgs[nArgIndex];

                ++nArgIndex;
            }
        }
    }

    return TRUE;
}

static void lFreeDevicePinConfigs(
  TDevicePinConfig* pDevicePinConfigs)
{
    UtFreeMem(pDevicePinConfigs);
}

static VOID lValidateDevicePinConfigs(
  const TPALData* pPALData,
  TDevicePinConfig* pDevicePinConfigs,
  UINT nTotalDevicePinConfigs)
{
    TPALPinDefined* pPALPinDefined;
    UINT nIndex;

    wprintf(L"Validating the device pin configurations.\n");

    pPALPinDefined = (TPALPinDefined*)UtAllocMem(
        sizeof(TPALPinDefined) * nTotalDevicePinConfigs);

    if (pPALPinDefined == NULL)
    {
        wprintf(L"Could not allocate memory for the TPALPinDefined structures.\n");

        return;
    }

    wprintf(L"Calling UtPALDevicePinConfigToPinDefined.\n");

    if (UtPALDevicePinConfigToPinDefined(pPALData, pDevicePinConfigs,
                                         nTotalDevicePinConfigs,
                                         pPALPinDefined, nTotalDevicePinConfigs))
    {
        for (nIndex = 0; nIndex < nTotalDevicePinConfigs; ++nIndex)
        {
            if (pPALPinDefined[nIndex].bDefined == FALSE)
            {
                wprintf(L"Warning: Pin %d is not defined.\n",
                        pPALPinDefined[nIndex].nPin);
            }
        }
    }
    else
    {
        wprintf(L"Call to UtPALDevicePinConfigToPinDefined has failed.\n");
    }

    UtFreeMem(pPALPinDefined);
}

static int lReadDevice(
  LPCWSTR pszDevice,
  LPCWSTR pszFilename,
  LPCWSTR pszArgs[],
  UINT nTotalArgs)
{
    int nResult = -1;
    LPBYTE pbyData = NULL;
    TDevicePinConfig* pDevicePinConfigs = NULL;
    BOOL bDeviceUninitialized = FALSE;
    WCHAR cPath[MAX_PATH];
    TDevice Device;
    TDeviceIOFuncs DeviceIOFuncs;
    TPALData* pPALData;
    UINT nTotalDevicePinConfigs;
    ULONG ulDataLen;

	if (FALSE == UtPALInitialize())
	{
		wprintf(L"Call to UtPALInitialize has failed.\n");

		return -1;
	}

    lInitDeviceIOFuncs(&DeviceIOFuncs);

    GetModuleFileNameW(GetModuleHandle(NULL), cPath, MArrayLen(cPath));

    PathRemoveFileSpecW(cPath);

    wprintf(L"Calling UtPepDevicesInitialize with the path \"%s\".\n", cPath);

    if (FALSE == UtPepDevicesInitialize(cPath))
    {
        wprintf(L"Call to UtPepDevicesInitialize has failed.\n");

		goto EndPALUninitialize;
    }

    wprintf(L"Calling UtPepDevicesFindDevice for the device \"%s\".\n", pszDevice);

    if (FALSE == UtPepDevicesFindDevice(pszDevice, edtPAL, &Device))
    {
        wprintf(L"Call to UtPepDevicesFindDevice has failed.\n");

        goto EndPepDevicesUninit;
    }

    pPALData = &Device.DeviceData.PALData;

    wprintf(L"Checking if the device suppports initializing and uninitializing.\n");

    if (NULL == Device.DeviceData.pInitFunc ||
        NULL == Device.DeviceData.pUninitFuncs)
    {
        wprintf(L"Device does not support initializing and/or uninitializing.\n");

        goto EndPepDevicesUninit;
    }

    wprintf(L"Checking if the device supports configurable pin configurations.\n");

    if (NULL == pPALData->pSetDevicePinConfigsFunc)
    {
        wprintf(L"Device does not support configurable pin configurations.\n");

        goto EndPepDevicesUninit;
    }

    wprintf(L"Checking if the device supports reading.\n");

    if (NULL == pPALData->pReadDeviceFunc)
    {
        wprintf(L"Device does not support reading.\n");

        goto EndPepDevicesUninit;
    }

    if (Device.pszAdapter)
    {
        wprintf(L"Adapter Required: %s\n", Device.pszAdapter);
    }

    wprintf(L"Dip Switches: ");

    UtConsolePrintDipSwitches(Device.bDipSwitches,
                              MArrayLen(Device.bDipSwitches));

    if (Device.pszMessage)
    {
        wprintf(L"Device Message: %s\n", Device.pszMessage);
    }

    wprintf(L"Press the \"Enter\" key to continue.\n");

    _getch();

    wprintf(L"Calling UtPepCtrlInitialize\n");

    if (FALSE == UtPepCtrlInitialize(&lDeviceChangeFunc))
    {
        wprintf(L"Call to UtPepCtrlInitialize has failed.\n");

        goto EndPepDevicesUninit;
    }

    wprintf(L"Calling UtPepCtrlReset\n");

    if (FALSE == UtPepCtrlReset())
    {
        wprintf(L"Call to UtPepCtrlReset has failed.\n");

        goto EndPepCtrlUninit;
    }

    wprintf(L"Initializing the device\n");

    if (FALSE == Device.DeviceData.pInitFunc())
    {
        wprintf(L"Initializing the device has failed.\n");

        goto EndPepCtrlUninit;
    }

    wprintf(L"Converting the device pin command line arguments.\n");

    if (FALSE == lAllocDevicePinConfigs(pszArgs, nTotalArgs,
                                        &pDevicePinConfigs,
                                        &nTotalDevicePinConfigs))
    {
        wprintf(L"Converting the device pin command line arguments has failed.\n");

        goto EndDeviceUninit;
    }

    lValidateDevicePinConfigs(pPALData, pDevicePinConfigs, nTotalDevicePinConfigs);

    wprintf(L"Setting the device pin configurations.\n");

    if (FALSE == pPALData->pSetDevicePinConfigsFunc(pDevicePinConfigs,
                                                    nTotalDevicePinConfigs))
    {
        wprintf(L"Could not set the device pin configurations.\n");

        goto EndDeviceUninit;
    }

    wprintf(L"Calculating the size of the fuse map in bytes.\n");

    if (FALSE == UtPALGetFuseMapSize(pPALData, &ulDataLen))
    {
        goto EndDeviceUninit;
    }
    
    wprintf(L"Allocating memory for the fuse map.\n");

    pbyData = (LPBYTE)UtAllocMem(ulDataLen);

    if (pbyData == NULL)
    {
        wprintf(L"Could not allocating memory for the fuse map.\n");

        goto EndDeviceUninit;
    }

    wprintf(L"Reading the device.\n");

    pPALData->pReadDeviceFunc(&DeviceIOFuncs, pbyData, ulDataLen);

    wprintf(L"Uninitializing the device\n");

    if (FALSE == Device.DeviceData.pUninitFuncs())
    {
        wprintf(L"Uninitializing the device has failed.\n");
    }

    bDeviceUninitialized = TRUE;

    if (FALSE == l_bErrorOccurred)
    {
        wprintf(L"Writing the data file \"%s\".\n", pszFilename);

        if (UtPALWriteFuseMapToJEDFile(pPALData, pbyData, ulDataLen,
                                       Device.pszName, Device.nPinCount,
                                       pszFilename))
        {
            wprintf(L"Data written to the file.\n");

            nResult = 0;
        }
        else
        {
            wprintf(L"Could not write the data to the file.\n");
        }
    }

    UtFreeMem(pbyData);

EndDeviceUninit:
    if (bDeviceUninitialized == FALSE)
    {
        wprintf(L"Uninitializing the device\n");

        if (FALSE == Device.DeviceData.pUninitFuncs())
        {
            wprintf(L"Uninitializing the device has failed.\n");
        }
    }

EndPepCtrlUninit:
    if (pDevicePinConfigs)
    {
        wprintf(L"Freeing the device pins configured from the command line\n");

        lFreeDevicePinConfigs(pDevicePinConfigs);
    }

    wprintf(L"Calling UtPepCtrlUninitialize\n");

    if (FALSE == UtPepCtrlUninitialize())
    {
        wprintf(L"Call to UtPepCtrlUninitialize has failed.\n");
    }

EndPepDevicesUninit:
    wprintf(L"Calling UtPepDevicesUninitialize\n");

    if (FALSE == UtPepDevicesUninitialize())
    {
        wprintf(L"Call to UtPepDevicesUninitialize has failed.\n");
    }

EndPALUninitialize:
	if (FALSE == UtPALUninitialize())
	{
		wprintf(L"Call to UtPALUninitialize has failed.\n");
	}

    return nResult;
}

static int lDisplayHelp(void)
{
    wprintf(L"\n");

    UtConsolePrintAppVersion();

    wprintf(L"\n");
    wprintf(L"Espresso Logic Minimizer\n");
    wprintf(L"Copyright (c) 1988, 1989, Regents of the University of California.\n");
    wprintf(L"All rights reserved.\n");
    wprintf(L"\n");
    wprintf(L"TestPALDevice [/read \"Device\" \"Output File\"\n");
    wprintf(L"               \"Pin Number\" \"Pin Value\" \"Polarity Value\" ...\n");
    wprintf(L"               (\"Combinatorial\" | \"Registered\")]\n");
    wprintf(L"\n");
    wprintf(L"    /read\n");
    wprintf(L"        \"Device\"         - Name of the device\n");
    wprintf(L"        \"Output File\"    - Name of the JED file to save the data to\n");
    wprintf(L"        \"Pin Number\"     - Number of the pin (Example: 9)\n");
    wprintf(L"        \"Pin Value\"      - Value of the pin (Example: \"Input\")\n");
    wprintf(L"        \"Polarity Value\" - Polarity of the pin (Example: \"Active Low\")\n");
    wprintf(L"        \"Combinatorial\"  - Device is combinatorial sub-type\n");
    wprintf(L"        \"Registered\"     - Device is registered is sub-type\n");
    wprintf(L"        \"Active Low\"     - Device has active low output polarity\n");
    wprintf(L"        \"Active High\"    - Device has active high output polarity\n");
    wprintf(L"\n");

    return -1;
}

int __cdecl wmain (int argc, WCHAR* argv[])
{
    int nResult;

    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    SetConsoleCtrlHandler(lHandlerRoutine, TRUE);

    if (argc >= 4)
    {
        if (lstrcmpi(argv[1], L"/read") == 0)
        {
            nResult = lReadDevice(argv[2], argv[3], &argv[4], argc - 4);
        }
        else
        {
            nResult = lDisplayHelp();
        }
    }
    else
    {
        nResult = lDisplayHelp();
    }

    SetConsoleCtrlHandler(lHandlerRoutine, FALSE);

    return nResult;
}

/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/
