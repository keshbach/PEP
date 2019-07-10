/***************************************************************************/
/*  Copyright (C) 2007-2019 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>

#include <strsafe.h>

#include <Config/UtPepCtrlCfg.h>

#include <Drivers/PepCtrlDefs.h>

#include <Includes/UtMacros.h>

#include <Utils/UtHeap.h>

#include <assert.h>

#pragma region "Constants"

#define CServiceManagerTotalRetries 6

#pragma endregion

extern HINSTANCE g_hInstance;

static BOOL l_bInitialize = TRUE;

#pragma region "Local Functions"

static VOID lWriteString1ParamMsg(
  _In_ LPCWSTR pszMsg,
  _In_ LPCWSTR pszValue,
  _In_ TUtPepCtrlCfgMsgFunc pMsgFunc)
{
    INT nTmpMsgLen = (lstrlenW(pszMsg) + lstrlenW(pszValue)) * sizeof(WCHAR);
    LPWSTR pszTmpMsg = (LPWSTR)UtAllocMem(nTmpMsgLen);

    StringCbPrintfW(pszTmpMsg, nTmpMsgLen, pszMsg, pszValue);

    pMsgFunc(pszTmpMsg);

    UtFreeMem(pszTmpMsg);
}

static VOID lWriteString2ParamsMsg(
  _In_ LPCWSTR pszMsg,
  _In_ LPCWSTR pszValue1,
  _In_ LPCWSTR pszValue2,
  _In_ TUtPepCtrlCfgMsgFunc pMsgFunc)
{
    INT nTmpMsgLen = (lstrlenW(pszMsg) + lstrlenW(pszValue1) + lstrlenW(pszValue2)) * sizeof(WCHAR);
    LPWSTR pszTmpMsg = (LPWSTR)UtAllocMem(nTmpMsgLen);

    StringCbPrintfW(pszTmpMsg, nTmpMsgLen, pszMsg, pszValue1, pszValue2);

    pMsgFunc(pszTmpMsg);

    UtFreeMem(pszTmpMsg);
}

static VOID lWriteHexMsg(
  _In_ LPCWSTR pszMsg,
  _In_ DWORD dwValue,
  _In_ TUtPepCtrlCfgMsgFunc pMsgFunc)
{
    INT nTmpMsgLen = (lstrlenW(pszMsg) + 11) * sizeof(WCHAR);
    LPWSTR pszTmpMsg = (LPWSTR)UtAllocMem(nTmpMsgLen);

    StringCbPrintf(pszTmpMsg, nTmpMsgLen, pszMsg, dwValue);

    pMsgFunc(pszTmpMsg);

    UtFreeMem(pszTmpMsg);
}

#pragma endregion

BOOL UTPEPCTRLCFGAPI UtPepCtrlCfgInitialize(VOID)
{
    if (l_bInitialize == FALSE)
    {
        return TRUE;
    }

    if (UtInitHeap() == FALSE)
    {
        return FALSE;
    }

    l_bInitialize = FALSE;

    return TRUE;
}

BOOL UTPEPCTRLCFGAPI UtPepCtrlCfgUninitialize(VOID)
{
    if (l_bInitialize == TRUE)
    {
        return FALSE;
    }

    l_bInitialize = TRUE;

    if (UtUninitHeap() == FALSE)
    {
        return FALSE;
    }

    return TRUE;
}

BOOL UTPEPCTRLCFGAPI UtPepCtrlCfgInstallDriver(
  _In_ LPCWSTR pszFile,
  _In_ TUtPepCtrlCfgMsgFunc pMsgFunc)
{
    BOOL bResult = FALSE;
    SC_HANDLE hManager;
    SC_HANDLE hService;

    if (l_bInitialize == TRUE)
    {
        if (pMsgFunc)
        {
            pMsgFunc(L"UtPepCtrlCfgInitialize has not been called.");
        }

        return FALSE;
    }

    if (pMsgFunc)
    {
        pMsgFunc(L"Opening the Service Control Manager");
    }

    hManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hManager)
    {
        if (pMsgFunc)
        {
            pMsgFunc(L"Service Control Manager opened.");
            pMsgFunc(L"Creating the driver's service.");
        }

        hService = CreateServiceW(hManager, CPepCtrlServiceName,
                                  CPepCtrlServiceDisplayName,
                                  SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
                                  SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
                                  pszFile, NULL, NULL, NULL, NULL, NULL);

        if (hService)
        {
            if (pMsgFunc)
            {
                pMsgFunc(L"Driver successfully installed.");
                pMsgFunc(L"Closing the driver service handle.");
            }

            if (CloseServiceHandle(hService))
            {
                if (pMsgFunc)
                {
                    pMsgFunc(L"Successfully closed the driver service handle.");
                }
            }
            else
            {
                if (pMsgFunc)
                {
                    lWriteHexMsg(L"Failed to close the driver service handle.  (Error Code: 0x%X)",
                                 GetLastError(), pMsgFunc);
                }
            }

            bResult = TRUE;
        }
        else
        {
            if (pMsgFunc)
            {
                lWriteHexMsg(L"Could not create the service.  (Error Code: 0x%X)",
                             GetLastError(), pMsgFunc);
            }
        }

        if (pMsgFunc)
        {
            pMsgFunc(L"Closing the Service Control Manager handle.");
        }

        if (CloseServiceHandle(hManager))
        {
            if (pMsgFunc)
            {
                pMsgFunc(L"Successfully closed the Service Control Manager handle.");
            }
        }
        else
        {
            if (pMsgFunc)
            {
                lWriteHexMsg(L"Failed to close the Service Control Manager handle.  (Error Code: 0x%X)",
                             GetLastError(), pMsgFunc);
            }
        }
    }
    else
    {
        if (pMsgFunc)
        {
            lWriteHexMsg(L"Could not open the Service Control Manager.  (Error Code: 0x%X)",
                         GetLastError(), pMsgFunc);
        }
    }

    return bResult;
}

BOOL UTPEPCTRLCFGAPI UtPepCtrlCfgUninstallDriver(
  _In_ TUtPepCtrlCfgMsgFunc pMsgFunc)
{
    BOOL bResult = FALSE;
    SC_HANDLE hManager;
    SC_HANDLE hService;

    if (l_bInitialize == TRUE)
    {
        if (pMsgFunc)
        {
            pMsgFunc(L"UtPepCtrlCfgInitialize has not been called.");
        }

        return FALSE;
    }

    if (pMsgFunc)
    {
        pMsgFunc(L"Attempting to stop the driver if it is running.");
    }

    if (!UtPepCtrlCfgStopDriver(pMsgFunc))
    {
        if (pMsgFunc)
        {
            pMsgFunc(L"Could not stop the driver from running.");
        }

        return FALSE;
    }

    if (pMsgFunc)
    {
        pMsgFunc(L"Driver stopped.");
    }

    if (pMsgFunc)
    {
        pMsgFunc(L"Opening the Service Control Manager");
    }

    hManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hManager == NULL)
    {
        if (pMsgFunc)
        {
            lWriteHexMsg(L"Could not open the Service Control Manager.  (Error Code: 0x%X)",
                         GetLastError(), pMsgFunc);
        }

        return FALSE;
    }

    if (pMsgFunc)
    {
        pMsgFunc(L"Service Control Manager opened.");
        pMsgFunc(L"Opening the driver service.");
    }

    hService = OpenServiceW(hManager, CPepCtrlServiceName, SERVICE_ALL_ACCESS);

    if (hService)
    {
        if (pMsgFunc)
        {
            pMsgFunc(L"Driver service opened.");
            pMsgFunc(L"Deleting the driver service.");
        }

        if (DeleteService(hService))
        {
            if (pMsgFunc)
            {
                pMsgFunc(L"Driver service deleted.");
            }

            bResult = TRUE;
        }
        else
        {
            if (pMsgFunc)
            {
                lWriteHexMsg(L"Could not delete the driver service.  (Error Code: 0x%X)",
                             GetLastError(), pMsgFunc);
            }
        }

        if (pMsgFunc)
        {
            pMsgFunc(L"Closing the driver service handle.");
        }

        if (CloseServiceHandle(hService))
        {
            if (pMsgFunc)
            {
                pMsgFunc(L"Successfully closed the driver service handle.");
            }
        }
        else
        {
            if (pMsgFunc)
            {
                lWriteHexMsg(L"Failed to close the driver service handle.  (Error Code: 0x%X)",
                             GetLastError(), pMsgFunc);
            }
        }
    }
    else
    {
        if (GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST)
        {
            if (pMsgFunc)
            {
                pMsgFunc(L"The driver's service does not exist.");
            }

            bResult = TRUE;
        }
        else
        {
            if (pMsgFunc)
            {
                lWriteHexMsg(L"Could not open the driver service.  (Error Code: 0x%X)",
                             GetLastError(), pMsgFunc);
            }
        }
    }

    if (pMsgFunc)
    {
        pMsgFunc(L"Closing the Service Control Manager handle.");
    }

    if (CloseServiceHandle(hManager))
    {
        if (pMsgFunc)
        {
            pMsgFunc(L"Successfully closed the Service Control Manager handle.");
        }
    }
    else
    {
        if (pMsgFunc)
        {
            lWriteHexMsg(L"Failed to close the Service Control Manager handle.  (Error Code: 0x%X)",
                         GetLastError(), pMsgFunc);
        }
    }

    return bResult;
}

BOOL UTPEPCTRLCFGAPI UtPepCtrlCfgStartDriver(
  _In_ TUtPepCtrlCfgMsgFunc pMsgFunc)
{
    BOOL bResult = FALSE;
    BOOL bQuitLoop = FALSE;
    INT nRetriesLeft = CServiceManagerTotalRetries;
    SC_HANDLE hManager;
    SC_HANDLE hService;
    SERVICE_STATUS ServiceStatus;

    if (l_bInitialize == TRUE)
    {
		if (pMsgFunc)
		{
			pMsgFunc(L"UtPepCtrlCfgInitialize has not been called.");
		}

		return FALSE;
    }

    if (pMsgFunc)
    {
        pMsgFunc(L"Opening the Service Control Manager.");
    }
    
    hManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hManager == NULL)
    {
        if (pMsgFunc)
        {
            lWriteHexMsg(L"Could not open the Service Control Manager.  (Error Code: 0x%X)",
                         GetLastError(), pMsgFunc);
        }

        return FALSE;
    }

    if (pMsgFunc)
    {
        pMsgFunc(L"Service Control Manager opened.");
        pMsgFunc(L"Opening the driver service.");
    }

    hService = OpenServiceW(hManager, CPepCtrlServiceName, SERVICE_ALL_ACCESS);

    if (hService)
    {
        if (pMsgFunc)
        {
            pMsgFunc(L"Querying the driver's service status.");
        }

        if (QueryServiceStatus(hService, &ServiceStatus))
        {
            if (pMsgFunc)
            {
                switch (ServiceStatus.dwCurrentState)
                {
                    case SERVICE_STOPPED:
                        pMsgFunc(L"The driver's service status is stopped.");
                        break;
                    case SERVICE_START_PENDING:
                        pMsgFunc(L"The driver's service status is start pending.");
                        break;
                    case SERVICE_STOP_PENDING:
                        pMsgFunc(L"The driver's service status is stop pending.");
                        break;
                    case SERVICE_RUNNING:
                        pMsgFunc(L"The driver's service status is running.");
                        break;
                    case SERVICE_CONTINUE_PENDING:
                        pMsgFunc(L"The driver's service status is continue pending.");
                        break;
                    case SERVICE_PAUSE_PENDING:
                        pMsgFunc(L"The driver's service status is pause pending.");
                        break;
                    case SERVICE_PAUSED:
                        pMsgFunc(L"The driver's service status is paused.");
                        break;
                    default:
                        lWriteHexMsg(L"Unknown driver service status.  (0x%X)",
                                     ServiceStatus.dwCurrentState, pMsgFunc);
                        break;
                }
            }

            if (ServiceStatus.dwCurrentState != SERVICE_RUNNING)
            {
				if (pMsgFunc)
				{
					pMsgFunc(L"The driver's service is not running.");
				}

                if (ServiceStatus.dwCurrentState != SERVICE_START_PENDING)
                {
                    if (pMsgFunc)
                    {
                        pMsgFunc(L"Starting the driver's service.");
                    }

                    if (!StartService(hService, 0, NULL))
                    {
                        if (pMsgFunc)
                        {
                            lWriteHexMsg(L"Could not start the driver's service.  (Error Code: 0x%X)",
                                         GetLastError(), pMsgFunc);
                        }

                        bQuitLoop = TRUE;
                    }
                }
                else
                {
                    bQuitLoop = TRUE;

                    if (pMsgFunc)
                    {
                        pMsgFunc(L"The driver's service is not starting.  Bailing out.");
                    }
                }

                while (!bQuitLoop)
                {
                    if (pMsgFunc)
                    {
                        pMsgFunc(L"Querying the driver's service status.");
                    }

                    if (QueryServiceStatus(hService, &ServiceStatus))
                    {
                        if (ServiceStatus.dwCurrentState != SERVICE_RUNNING)
                        {
                            if (pMsgFunc)
                            {
                                pMsgFunc(L"Service not running.  Pausing briefly before re-querying the driver's status.");
                            }

                            Sleep(500);

                            --nRetriesLeft;

                            if (nRetriesLeft == 0)
                            {
                                if (pMsgFunc)
                                {
                                    pMsgFunc(L"No service start retries left.");
                                }

                                bQuitLoop = TRUE;
                            }
                        }
                        else
                        {
                            if (pMsgFunc)
                            {
                                pMsgFunc(L"Driver running.");
                            }

                            bQuitLoop = TRUE;
                            bResult = TRUE;
                        }
                    }
                    else
                    {
                        bQuitLoop = TRUE;

                        if (pMsgFunc)
                        {
                            lWriteHexMsg(L"Failed to query the driver's service status.  (Error Code: 0x%X)",
                                         GetLastError(), pMsgFunc);
                        }
                    }
                }
            }
            else
            {
                bResult = TRUE;

                if (pMsgFunc)
                {
                    pMsgFunc(L"The driver service is running.");
                }
            }
        }
        else
        {
            if (pMsgFunc)
            {
                lWriteHexMsg(L"Failed to query the driver service status.  (0x%X)",
                             GetLastError(), pMsgFunc);
            }
        }

        if (pMsgFunc)
        {
            pMsgFunc(L"Closing the driver service handle.");
        }

        if (CloseServiceHandle(hService))
        {
            if (pMsgFunc)
            {
                pMsgFunc(L"Successfully closed the driver service handle.");
            }
        }
        else
        {
            if (pMsgFunc)
            {
                lWriteHexMsg(L"Failed to close the driver's service handle.  (Error Code: 0x%X)",
                             GetLastError(), pMsgFunc);
            }
        }
    }
    else
    {
        if (pMsgFunc)
        {
            lWriteHexMsg(L"Could not open the driver service.  (Error Code: 0x%X)",
                         GetLastError(), pMsgFunc);
        }
    }

    if (pMsgFunc)
    {
        pMsgFunc(L"Closing the Service Control Manager handle.");
    }

    if (CloseServiceHandle(hManager))
    {
        if (pMsgFunc)
        {
            pMsgFunc(L"Successfully closed the Service Control Manager handle.");
        }
    }
    else
    {
        if (pMsgFunc)
        {
            lWriteHexMsg(L"Failed to close the Service Control Manager handle.  (Error Code: 0x%X)",
                         GetLastError(), pMsgFunc);
        }
    }

    return bResult;
}

BOOL UTPEPCTRLCFGAPI UtPepCtrlCfgStopDriver(
  _In_ TUtPepCtrlCfgMsgFunc pMsgFunc)
{
    BOOL bResult = FALSE;
    BOOL bQuitLoop = FALSE;
    INT nRetriesLeft = CServiceManagerTotalRetries;
    SC_HANDLE hManager;
    SC_HANDLE hService;
    SERVICE_STATUS ServiceStatus;

    if (l_bInitialize == TRUE)
    {
		if (pMsgFunc)
		{
			pMsgFunc(L"UtPepCtrlCfgInitialize has not been called.");
		}

		return FALSE;
    }

    if (pMsgFunc)
    {
        pMsgFunc(L"Opening the Service Control Manager.");
    }

    hManager = OpenSCManagerW(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hManager == NULL)
    {
        if (pMsgFunc)
        {
            lWriteHexMsg(L"Could not open the Service Control Manager.  (Error Code: 0x%X)",
                         GetLastError(), pMsgFunc);
        }

        return FALSE;
    }

    if (pMsgFunc)
    {
        pMsgFunc(L"Service Control Manager opened.");
        pMsgFunc(L"Opening the driver service.");
    }

    hService = OpenServiceW(hManager, CPepCtrlServiceName, SERVICE_ALL_ACCESS);

    if (hService)
    {
        if (pMsgFunc)
        {
            pMsgFunc(L"Querying the driver's service status.");
        }

        if (QueryServiceStatus(hService, &ServiceStatus))
        {
            if (pMsgFunc)
            {
                switch (ServiceStatus.dwCurrentState)
                {
                    case SERVICE_STOPPED:
                        pMsgFunc(L"The driver's service status is stopped.");
                        break;
                    case SERVICE_START_PENDING:
                        pMsgFunc(L"The driver's service status is start pending.");
                        break;
                    case SERVICE_STOP_PENDING:
                        pMsgFunc(L"The driver's service status is stop pending.");
                        break;
                    case SERVICE_RUNNING:
                        pMsgFunc(L"The driver's service status is running.");
                        break;
                    case SERVICE_CONTINUE_PENDING:
                        pMsgFunc(L"The driver's service status is continue pending.");
                        break;
                    case SERVICE_PAUSE_PENDING:
                        pMsgFunc(L"The driver's service status is pause pending.");
                        break;
                    case SERVICE_PAUSED:
                        pMsgFunc(L"The driver's service status is paused.");
                        break;
                    default:
                        lWriteHexMsg(L"Unknown driver service status.  (0x%X)",
                                     ServiceStatus.dwCurrentState, pMsgFunc);
                        break;
                }
            }

            if (ServiceStatus.dwCurrentState != SERVICE_STOPPED)
            {
                if (ServiceStatus.dwCurrentState != SERVICE_STOP_PENDING)
                {
                    if (pMsgFunc)
                    {
                        pMsgFunc(L"Stopping the driver's service.");
                    }

                    if (!ControlService(hService, SERVICE_CONTROL_STOP, &ServiceStatus))
                    {
                        if (pMsgFunc)
                        {
                            lWriteHexMsg(L"Could not issue the stop command to the service.  (Error Code: 0x%X)",
                                         GetLastError(), pMsgFunc);
                        }
                    }
                }
                else
                {
                    bQuitLoop = TRUE;

                    if (pMsgFunc)
                    {
                        pMsgFunc(L"The driver's service is not stopping.  Bailing out.");
                    }
                }

                while (!bQuitLoop)
                {
                    if (pMsgFunc)
                    {
                        pMsgFunc(L"Querying the driver's service status.");
                    }

                    if (QueryServiceStatus(hService, &ServiceStatus))
                    {
                        if (ServiceStatus.dwCurrentState != SERVICE_STOPPED)
                        {
                            if (pMsgFunc)
                            {
                                pMsgFunc(L"Service not stopped.  Pausing briefly before re-querying the driver's status.");
                            }

                            Sleep(500);

                            --nRetriesLeft;

                            if (nRetriesLeft == 0)
                            {
                                if (pMsgFunc)
                                {
                                    pMsgFunc(L"No service stop retries left.");
                                }

                                bQuitLoop = TRUE;
                            }
                        }
                        else
                        {
                            if (pMsgFunc)
                            {
                                pMsgFunc(L"Driver stopped.");
                            }

                            bQuitLoop = TRUE;
                            bResult = TRUE;
                        }
                    }
                    else
                    {
                        if (pMsgFunc)
                        {
                            lWriteHexMsg(L"Could not query the service's status.  (Error Code: 0x%X)",
                                         GetLastError(), pMsgFunc);
                        }

                        bQuitLoop = TRUE;
                    }
                }
            }
            else
            {
                bResult = TRUE;

                if (pMsgFunc)
                {
                    pMsgFunc(L"The driver's service is already stopped.");
                }
            }
        }
        else
        {
            if (pMsgFunc)
            {
                lWriteHexMsg(L"Could not query the service's status.  (Error Code: 0x%X)",
                             GetLastError(), pMsgFunc);
            }
        }

        if (CloseServiceHandle(hService))
        {
            if (pMsgFunc)
            {
                pMsgFunc(L"Successfully closed the driver service handle.");
            }
        }
        else
        {
            if (pMsgFunc)
            {
                lWriteHexMsg(L"Could not close the driver service handle.  (Error Code: 0x%X)",
                             GetLastError(), pMsgFunc);
            }
        }
    }
    else
    {
        if (GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST)
        {
            if (pMsgFunc)
            {
                pMsgFunc(L"The driver's service does not exist.");
            }

            bResult = TRUE;
        }
        else
        {
            if (pMsgFunc)
            {
                lWriteHexMsg(L"Could not open the driver service.  (Error Code: 0x%X)",
                             GetLastError(), pMsgFunc);
            }
        }
    }

    if (pMsgFunc)
    {
        pMsgFunc(L"Closing the Service Control Manager handle.");
    }

    if (CloseServiceHandle(hManager))
    {
        if (pMsgFunc)
        {
            pMsgFunc(L"Successfully closed the Service Control Manager handle.");
        }
    }
    else
    {
        if (pMsgFunc)
        {
            lWriteHexMsg(L"Failed to close the Service Control Manager handle.  (Error Code: 0x%X)",
                         GetLastError(), pMsgFunc);
        }
    }

    return bResult;
}

/***************************************************************************/
/*  Copyright (C) 2007-2019 Kevin Eshbach                                  */
/***************************************************************************/
