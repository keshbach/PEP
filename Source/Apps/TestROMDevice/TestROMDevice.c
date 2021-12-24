/***************************************************************************/
/*  Copyright (C) 2006-2021 Kevin Eshbach                                  */
/***************************************************************************/

#include <stdio.h>
#include <conio.h>

#include <windows.h>
#include <shlwapi.h>

#include <Config/UtPepCtrl.h>

#include <UtilsDevice/UtPepDevices.h>

#include <Includes/UtMacros.h>

#include <Utils/UtHeapProcess.h>
#include <Utils/UtConsole.h>

#include <Apps/Includes/UtCommandLineParser.inl>

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
        case edoProgram:
            wprintf(L"Beginning the programming of a device with 0x%08x addresses.\n", ulTotalAddresses);
            break;
        case edoVerify:
            wprintf(L"Beginning the verify of a device with 0x%08x addresses.\n", ulTotalAddresses);
            break;
        default:
            wprintf(L"Unexpected device operation.\n");
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
        case edoProgram:
            wprintf(L"Ending the programming of a device.\n");
            break;
        case edoVerify:
            wprintf(L"Ending the verify of a device.\n");
            break;
        default:
            wprintf(L"Unexpected device operation.\n");
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

static int lReadDevice(
  LPCWSTR pszDevice,
  LPCWSTR pszFilename)
{
    int nResult = -1;
    LPBYTE pbyData = NULL;
    EUtPepCtrlDeviceType DeviceType = eUtPepCtrlUsbDeviceType;
    WCHAR cPath[MAX_PATH];
    TDevice Device;
    TDeviceIOFuncs DeviceIOFuncs;
    TROMData* pROMData;
    HANDLE hFile;
    DWORD dwBytesWritten;

    GetModuleFileName(GetModuleHandle(NULL), cPath, MArrayLen(cPath));

    PathRemoveFileSpec(cPath);

    wprintf(L"Calling UtPepDevicesInitialize with the path \"%s\".\n", cPath);

    if (FALSE == UtPepDevicesInitialize(cPath))
    {
        wprintf(L"Call to UtPepDevicesInitialize has failed.\n");

        return -1;
    }

    wprintf(L"Calling UtPepDevicesFindDevice to find the device \"%s\".\n", pszDevice);

    if (FALSE == UtPepDevicesFindDevice(pszDevice, edtEPROM, &Device) &&
        FALSE == UtPepDevicesFindDevice(pszDevice, edtEPROMWithIO, &Device) &&
        FALSE == UtPepDevicesFindDevice(pszDevice, edtPROM, &Device))
    {
        wprintf(L"Call to UtPepDevicesFindDevice has failed.\n");

        goto EndPepDevicesUninit;
    }

    pROMData = &Device.DeviceData.ROMData;

    wprintf(L"Checking if the device suppports initializing and uninitializing.\n");

    if (NULL == Device.DeviceData.pInitFunc ||
        NULL == Device.DeviceData.pUninitFuncs)
    {
        wprintf(L"Device does not support initializing and/or uninitializing.\n");

        goto EndPepDevicesUninit;
    }

    wprintf(L"Checking if the device supports reading.\n");

    if (NULL == pROMData->pReadDeviceFunc)
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

    wprintf(L"Press the \"Enter\" key to continue.\n");

    _getch();

    if (UtCommandLineParserGetUseParallelPortConfiguration())
    {
        wprintf(L"Using the parallel port device.\n");

        DeviceType = eUtPepCtrlParallelPortDeviceType;
    }
    else
    {
        wprintf(L"Using the USB device.\n");
    }

    wprintf(L"Calling UtPepCtrlInitialize\n");

    if (FALSE == UtPepCtrlInitialize(DeviceType, &lDeviceChangeFunc))
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

    wprintf(L"Please insert the device.\n");
    wprintf(L"Press the \"Enter\" key to continue.\n");

    _getch();

    wprintf(L"Initializing the device\n");

    if (FALSE == Device.DeviceData.pInitFunc())
    {
        wprintf(L"Initializing the device has failed.\n");

        goto EndPepCtrlUninit;
    }

    pbyData = (LPBYTE)UtAllocMem(pROMData->nSize);

    lInitDeviceIOFuncs(&DeviceIOFuncs);

    pROMData->pReadDeviceFunc(&DeviceIOFuncs,
                              Device.nChipEnableNanoseconds,
	                          Device.nOutputEnableNanoseconds,
		                      pbyData, pROMData->nSize);

    if (FALSE == l_bErrorOccurred)
    {
        hFile = CreateFile(pszFilename, GENERIC_WRITE, FILE_SHARE_READ, NULL,
                           CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile != INVALID_HANDLE_VALUE)
        {
            if (TRUE == WriteFile(hFile, pbyData, pROMData->nSize, &dwBytesWritten, NULL))
            {
                wprintf(L"Data written to the file \"%s\".\n", pszFilename);

                nResult = 0;
            }
            else
            {
                wprintf(L"Could not write to the file \"%s\".  (Error Code: 0x%08x)\n",
                        pszFilename, GetLastError());
            }

            CloseHandle(hFile);
        }
        else
        {
            wprintf(L"Could not create the file \"%s\".  (Error Code: 0x%08x)\n",
                    pszFilename, GetLastError());
        }
    }
    else
    {
        wprintf(L"Not saving the file because an error occurred.\n");
    }

    UtFreeMem(pbyData);

    wprintf(L"Uninitializing the device\n");

    if (FALSE == Device.DeviceData.pUninitFuncs())
    {
        wprintf(L"Uninitializing the device has failed.\n");
    }

EndPepCtrlUninit:
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

    return nResult;
}

static int lProgramDevice(
  LPCWSTR pszDevice,
  LPCWSTR pszFilename)
{
    int nResult = -1;
    LPBYTE pbyData = NULL;
    EUtPepCtrlDeviceType DeviceType = eUtPepCtrlUsbDeviceType;
    WCHAR cPath[MAX_PATH];
    TDevice Device;
    TDeviceIOFuncs DeviceIOFuncs;
    TROMData* pROMData;
    HANDLE hFile;
    DWORD dwBytesRead;
    LARGE_INTEGER FileSize;

    GetModuleFileName(GetModuleHandle(NULL), cPath, MArrayLen(cPath));

    PathRemoveFileSpec(cPath);

    wprintf(L"Calling UtPepDevicesInitialize with the path \"%s\".\n", cPath);

    if (FALSE == UtPepDevicesInitialize(cPath))
    {
        wprintf(L"Call to UtPepDevicesInitialize has failed.\n");

        return -1;
    }

    wprintf(L"Calling UtPepDevicesFindDevice to find the device \"%s\".\n", pszDevice);

    if (FALSE == UtPepDevicesFindDevice(pszDevice, edtEPROM, &Device) &&
        FALSE == UtPepDevicesFindDevice(pszDevice, edtEPROMWithIO, &Device) &&
        FALSE == UtPepDevicesFindDevice(pszDevice, edtPROM, &Device))
    {
        wprintf(L"Call to UtPepDevicesFindDevice has failed.\n");

        goto EndPepDevicesUninit;
    }

    pROMData = &Device.DeviceData.ROMData;

    wprintf(L"Checking if the device suppports initializing and uninitializing.\n");

    if (NULL == Device.DeviceData.pInitFunc ||
        NULL == Device.DeviceData.pUninitFuncs)
    {
        wprintf(L"Device does not support initializing and/or uninitializing.\n");

        goto EndPepDevicesUninit;
    }

    wprintf(L"Checking if the device supports programming.\n");

    if (NULL == pROMData->pProgramDeviceFunc)
    {
        wprintf(L"Device does not support programming.\n");

        goto EndPepDevicesUninit;
    }

    if (Device.pszAdapter)
    {
        wprintf(L"Adapter Required: %s\n", Device.pszAdapter);
    }

    wprintf(L"Dip Switches: ");

    UtConsolePrintDipSwitches(Device.bDipSwitches,
                              MArrayLen(Device.bDipSwitches));

    wprintf(L"Press the \"Enter\" key to continue.\n");

    _getch();

    if (UtCommandLineParserGetUseParallelPortConfiguration())
    {
        wprintf(L"Using the parallel port device.\n");

        DeviceType = eUtPepCtrlParallelPortDeviceType;
    }
    else
    {
        wprintf(L"Using the USB device.\n");
    }

    wprintf(L"Calling UtPepCtrlInitialize\n");

    if (FALSE == UtPepCtrlInitialize(DeviceType, &lDeviceChangeFunc))
    {
        wprintf(L"Call to UtPepCtrlInitialize has failed.\n");

        goto EndPepDevicesUninit;
    }

    wprintf(L"Initializing the device\n");

    if (FALSE == Device.DeviceData.pInitFunc())
    {
        wprintf(L"Initializing the device has failed.\n");

        goto EndPepCtrlUninit;
    }

    wprintf(L"Attempting to open the file \"%s\".\n", pszFilename);

    hFile = CreateFile(pszFilename, GENERIC_READ, FILE_SHARE_READ, NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        wprintf(L"Could not open the file.  (Error Code: 0x%08x).\n",
                GetLastError());

        goto EndDeviceUninit;
    }

    wprintf(L"Attempting to get the file size.\n");

    if (FALSE == GetFileSizeEx(hFile, &FileSize))
    {
        wprintf(L"Could not retrieve the file's size.  (Error Code: 0x%08x).\n",
                GetLastError());

        goto EndCloseHandle;
    }

    if (FileSize.QuadPart != pROMData->nSize)
    {
        wprintf(L"The file's size does not match that of the device.\n");

        goto EndCloseHandle;
    }

    pbyData = (LPBYTE)UtAllocMem(pROMData->nSize);

    wprintf(L"Attempting to read the file.\n");

    if (FALSE == ReadFile(hFile, pbyData, pROMData->nSize, &dwBytesRead, NULL))
    {
        wprintf(L"Could not read the file.  (Error Code: 0x%08x).\n",
                GetLastError());

        goto EndFreeMemory;
    }

    lInitDeviceIOFuncs(&DeviceIOFuncs);

    pROMData->pProgramDeviceFunc(&DeviceIOFuncs,
                                 Device.nChipEnableNanoseconds,
		                         Device.nOutputEnableNanoseconds,
                                 pbyData, pROMData->nSize);

    if (FALSE == l_bErrorOccurred)
    {
        nResult = 0;
    }
    else
    {
        wprintf(L"An error occurred during the programming.\n");
    }

EndFreeMemory:
    UtFreeMem(pbyData);
   
EndCloseHandle:
    CloseHandle(hFile);

EndDeviceUninit:
    wprintf(L"Uninitializing the device\n");

    if (FALSE == Device.DeviceData.pUninitFuncs())
    {
        wprintf(L"Uninitializing the device has failed.\n");
    }

EndPepCtrlUninit:
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

    return nResult;
}

static int lVerifyDevice(
  LPCWSTR pszDevice,
  LPCWSTR pszFilename)
{
    int nResult = -1;
    LPBYTE pbyData = NULL;
    EUtPepCtrlDeviceType DeviceType = eUtPepCtrlUsbDeviceType;
    WCHAR cPath[MAX_PATH];
    TDevice Device;
    TDeviceIOFuncs DeviceIOFuncs;
    TROMData* pROMData;
    HANDLE hFile;
    DWORD dwBytesRead;
    LARGE_INTEGER FileSize;

    GetModuleFileName(GetModuleHandle(NULL), cPath, MArrayLen(cPath));

    PathRemoveFileSpec(cPath);

    wprintf(L"Calling UtPepDevicesInitialize with the path \"%s\".\n", cPath);

    if (FALSE == UtPepDevicesInitialize(cPath))
    {
        wprintf(L"Call to UtPepDevicesInitialize has failed.\n");

        return -1;
    }

    wprintf(L"Calling UtPepDevicesFindDevice to find the device \"%s\".\n", pszDevice);

    if (FALSE == UtPepDevicesFindDevice(pszDevice, edtEPROM, &Device) &&
        FALSE == UtPepDevicesFindDevice(pszDevice, edtEPROMWithIO, &Device) &&
        FALSE == UtPepDevicesFindDevice(pszDevice, edtPROM, &Device))
    {
        wprintf(L"Call to UtPepDevicesFindDevice has failed.\n");

        goto EndPepDevicesUninit;
    }

    pROMData = &Device.DeviceData.ROMData;

    wprintf(L"Checking if the device suppports initializing and uninitializing.\n");

    if (NULL == Device.DeviceData.pInitFunc ||
        NULL == Device.DeviceData.pUninitFuncs)
    {
        wprintf(L"Device does not support initializing and/or uninitializing.\n");

        goto EndPepDevicesUninit;
    }

    wprintf(L"Checking if the device supports verifying.\n");

    if (NULL == pROMData->pVerifyDeviceFunc)
    {
        wprintf(L"Device does not support verifying.\n");

        goto EndPepDevicesUninit;
    }

    if (Device.pszAdapter)
    {
        wprintf(L"Adapter Required: %s\n", Device.pszAdapter);
    }

    wprintf(L"Dip Switches: ");

    UtConsolePrintDipSwitches(Device.bDipSwitches,
                              MArrayLen(Device.bDipSwitches));

    wprintf(L"Press the \"Enter\" key to continue.\n");

    _getch();

    if (UtCommandLineParserGetUseParallelPortConfiguration())
    {
        wprintf(L"Using the parallel port device.\n");

        DeviceType = eUtPepCtrlParallelPortDeviceType;
    }
    else
    {
        wprintf(L"Using the USB device.\n");
    }

    wprintf(L"Calling UtPepCtrlInitialize\n");

    if (FALSE == UtPepCtrlInitialize(DeviceType, &lDeviceChangeFunc))
    {
        wprintf(L"Call to UtPepCtrlInitialize has failed.\n");

        goto EndPepDevicesUninit;
    }

    wprintf(L"Initializing the device\n");

    if (FALSE == Device.DeviceData.pInitFunc())
    {
        wprintf(L"Initializing the device has failed.\n");

        goto EndPepCtrlUninit;
    }

    wprintf(L"Attempting to open the file \"%s\".\n", pszFilename);

    hFile = CreateFile(pszFilename, GENERIC_READ, FILE_SHARE_READ, NULL,
                       OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        wprintf(L"Could not open the file.  (Error Code: 0x%08x).\n",
                GetLastError());

        goto EndDeviceUninit;
    }

    wprintf(L"Attempting to get the file size.\n");

    if (FALSE == GetFileSizeEx(hFile, &FileSize))
    {
        wprintf(L"Could not retrieve the file's size.  (Error Code: 0x%08x).\n",
                GetLastError());

        goto EndCloseHandle;
    }

    if (FileSize.QuadPart != pROMData->nSize)
    {
        wprintf(L"The file's size does not match that of the device.\n");

        goto EndCloseHandle;
    }

    pbyData = (LPBYTE)UtAllocMem(pROMData->nSize);

    wprintf(L"Attempting to read the file.\n");

    if (FALSE == ReadFile(hFile, pbyData, pROMData->nSize, &dwBytesRead, NULL))
    {
        wprintf(L"Could not read the file.  (Error Code: 0x%08x).\n",
                GetLastError());

        goto EndFreeMemory;
    }

    lInitDeviceIOFuncs(&DeviceIOFuncs);

    pROMData->pVerifyDeviceFunc(&DeviceIOFuncs,
                                Device.nChipEnableNanoseconds,
                                Device.nOutputEnableNanoseconds,
                                pbyData, pROMData->nSize);

    if (FALSE == l_bErrorOccurred)
    {
        nResult = 0;
    }
    else
    {
        wprintf(L"An error occurred during the verification.\n");
    }

EndFreeMemory:
    UtFreeMem(pbyData);
   
EndCloseHandle:
    CloseHandle(hFile);

EndDeviceUninit:
    wprintf(L"Uninitializing the device\n");

    if (FALSE == Device.DeviceData.pUninitFuncs())
    {
        wprintf(L"Uninitializing the device has failed.\n");
    }

EndPepCtrlUninit:
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

    return nResult;
}

static int lDisplayHelp(void)
{
    wprintf(L"\n");

    UtConsolePrintAppVersion();

    wprintf(L"\n");
    wprintf(L"TestROMDevice [/read \"Device\" \"Output File\"]\n");
    wprintf(L"              [/program \"Device\" \"Input File\"]\n");
    wprintf(L"              [/verify \"Device\" \"Input File\"]\n");
    wprintf(L"\n");
    wprintf(L"    /read\n");
    wprintf(L"        \"Device\"      - Name of the device\n");
    wprintf(L"        \"Output File\" - Name of the file to save the data to\n");
    wprintf(L"    /program\n");
    wprintf(L"        \"Device\"      - Name of the device\n");
    wprintf(L"        \"Input File\"  - Name of the file to program the data from\n");
    wprintf(L"    /verify\n");
    wprintf(L"        \"Device\"      - Name of the device\n");
    wprintf(L"        \"Input File\"  - Name of the file to read the data from\n");
    wprintf(L"\n");
    wprintf(L"    Configuration\n");
    wprintf(L"    /parallelport     - Use the parallel port instead of USB\n");
    wprintf(L"\n");

    return -1;
}

int __cdecl wmain (int argc, WCHAR* argv[])
{
    int nResult, nTotalNewArgs;
    LPCWSTR* ppszNewArgs;

    HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

    SetConsoleCtrlHandler(lHandlerRoutine, TRUE);

    UtCommandLineParserInitialize(argc, argv);

    nTotalNewArgs = UtCommandLineParserGetTotalArguments();
    ppszNewArgs = UtCommandLineParserGetArguments();

    if (nTotalNewArgs == 4)
    {
        if (lstrcmpi(ppszNewArgs[1], L"/read") == 0)
        {
            nResult = lReadDevice(ppszNewArgs[2], ppszNewArgs[3]);
        }
        else if (lstrcmpi(ppszNewArgs[1], L"/program") == 0)
        {
            nResult = lProgramDevice(ppszNewArgs[2], ppszNewArgs[3]);
        }
        else if (lstrcmpi(ppszNewArgs[1], L"/verify") == 0)
        {
            nResult = lVerifyDevice(ppszNewArgs[2], ppszNewArgs[3]);
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

    UtCommandLineParserUninitialize();

    SetConsoleCtrlHandler(lHandlerRoutine, FALSE);

    return nResult;
}

/***************************************************************************/
/*  Copyright (C) 2006-2021 Kevin Eshbach                                  */
/***************************************************************************/
