/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "Config.h"

#include <Config/UtPepCtrl.h>
#include <Config/UtPepCtrlCfg.h>

#include <Utils/UtHeap.h>

#include <assert.h>

#define CUtPepCtrlCfgDll L"UtPepCtrlCfg.dll"
#define CUtPepCtrlCfgInitializeFunc "UtPepCtrlCfgInitialize"
#define CUtPepCtrlCfgUninitializeFunc "UtPepCtrlCfgUninitialize"
#define CUtPepCtrlCfgGetPortTypeFunc "UtPepCtrlCfgGetPortType"
#define CUtPepCtrlCfgGetPortDeviceNameFunc "UtPepCtrlCfgGetPortDeviceName"

typedef BOOL (UTPEPCTRLCFGAPI *TUtPepCtrlCfgInitializeFunc)(VOID);
typedef BOOL (UTPEPCTRLCFGAPI *TUtPepCtrlCfgUninitializeFunc)(VOID);
typedef BOOL (UTPEPCTRLCFGAPI *TUtPepCtrlCfgGetPortTypeFunc)(EUtPepCtrlCfgPortType* pPortType);
typedef BOOL (UTPEPCTRLCFGAPI *TUtPepCtrlCfgGetPortDeviceName)(LPWSTR pszPortDeviceName, LPINT pnPortDeviceNameLen);

static VOID lManagedPepCtrlArrived(VOID)
{
    Pep::Programmer::Config::GetDeviceChange()->DeviceChange(Pep::Programmer::IDeviceChange::ENotification::Arrived);
}

static VOID lManagedPepCtrlRemoved(VOID)
{
	Pep::Programmer::Config::GetDeviceChange()->DeviceChange(Pep::Programmer::IDeviceChange::ENotification::Removed);
}

#pragma unmanaged

static VOID UTPEPCTRLAPI lUnmanagedPepCtrlDeviceChange(
  EUtPepCtrlDeviceChange DeviceChange)
{
    switch (DeviceChange)
    {
        case eUtPepCtrlDeviceArrived:
            lManagedPepCtrlArrived();
            break;
        case eUtPepCtrlDeviceRemoved:
            lManagedPepCtrlRemoved();
            break;
        default:
            assert(0);
            break;
    }
}

static BOOL lUnmanagedInitializePepCtrl(VOID)
{
    return UtPepCtrlInitialize(&lUnmanagedPepCtrlDeviceChange);
}

static BOOL lUnmanagedUninitializePepCtrl(VOID)
{
    return UtPepCtrlUninitialize();
}

static BOOL lUnmanagedIsDevicePresent(VOID)
{
    BOOL bPresent;

    if (UtPepCtrlIsDevicePresent(&bPresent))
    {
        return bPresent;
    }

    return FALSE;
}

static BOOL lUnmanagedReset(VOID)
{
    return UtPepCtrlReset() ? TRUE : FALSE;
}

static BOOL lUnmanagedGetPortType(
  EUtPepCtrlCfgPortType* pPortType)
{
    BOOL bResult = FALSE;
    HMODULE hModule = ::LoadLibrary(CUtPepCtrlCfgDll);
    TUtPepCtrlCfgInitializeFunc pUtPepCtrlCfgInitialize;
    TUtPepCtrlCfgUninitializeFunc pUtPepCtrlCfgUninitialize;
    TUtPepCtrlCfgGetPortTypeFunc pUtPepCtrlCfgGetPortType;

    if (hModule == NULL)
    {
        return FALSE;
    }

    pUtPepCtrlCfgInitialize = (TUtPepCtrlCfgInitializeFunc)::GetProcAddress(
                                  hModule,
                                  CUtPepCtrlCfgInitializeFunc);

    pUtPepCtrlCfgUninitialize = (TUtPepCtrlCfgUninitializeFunc)::GetProcAddress(
                                    hModule, CUtPepCtrlCfgUninitializeFunc);

    pUtPepCtrlCfgGetPortType = (TUtPepCtrlCfgGetPortTypeFunc)::GetProcAddress(
                                   hModule, CUtPepCtrlCfgGetPortTypeFunc);

    if (pUtPepCtrlCfgInitialize == NULL ||
        pUtPepCtrlCfgUninitialize == NULL ||
        pUtPepCtrlCfgGetPortType == NULL)
    {
        ::FreeLibrary(hModule);

        return FALSE;
    }

    if (pUtPepCtrlCfgInitialize() == TRUE)
    {
        bResult = pUtPepCtrlCfgGetPortType(pPortType);

        pUtPepCtrlCfgUninitialize();
    }

    ::FreeLibrary(hModule);

    return bResult;
}

static LPCWSTR lUnmanagedAllocPortDeviceName(VOID)
{
    HMODULE hModule = ::LoadLibrary(CUtPepCtrlCfgDll);
    TUtPepCtrlCfgInitializeFunc pUtPepCtrlCfgInitialize;
    TUtPepCtrlCfgUninitializeFunc pUtPepCtrlCfgUninitialize;
    TUtPepCtrlCfgGetPortDeviceName pUtPepCtrlCfgGetPortDeviceName;
    LPWSTR pszPortDeviceName;
    INT nPortDeviceNameLen;

    if (hModule == NULL)
    {
        return FALSE;
    }

    pUtPepCtrlCfgInitialize = (TUtPepCtrlCfgInitializeFunc)::GetProcAddress(
                                  hModule,
                                  CUtPepCtrlCfgInitializeFunc);

    pUtPepCtrlCfgUninitialize = (TUtPepCtrlCfgUninitializeFunc)::GetProcAddress(
                                    hModule, CUtPepCtrlCfgUninitializeFunc);

    pUtPepCtrlCfgGetPortDeviceName = (TUtPepCtrlCfgGetPortDeviceName)::GetProcAddress(
                                         hModule, CUtPepCtrlCfgGetPortDeviceNameFunc);

    if (pUtPepCtrlCfgInitialize == NULL ||
        pUtPepCtrlCfgUninitialize == NULL ||
        pUtPepCtrlCfgGetPortDeviceName == NULL)
    {
        ::FreeLibrary(hModule);

        return NULL;
    }

    pszPortDeviceName = NULL;

    if (pUtPepCtrlCfgInitialize())
    {
        if (TRUE == pUtPepCtrlCfgGetPortDeviceName(NULL, &nPortDeviceNameLen))
        {
            pszPortDeviceName = (LPWSTR)UtAllocMem(nPortDeviceNameLen);

            if (pszPortDeviceName != NULL)
            {
                if (FALSE == pUtPepCtrlCfgGetPortDeviceName(pszPortDeviceName, &nPortDeviceNameLen))
                {
                    UtFreeMem(pszPortDeviceName);

                    pszPortDeviceName = NULL;
                }
            }
        }

        pUtPepCtrlCfgUninitialize();
    }

    ::FreeLibrary(hModule);

    return pszPortDeviceName;
}

static VOID lUnmanagedFreePortDeviceName(
  LPCWSTR pszPortDeviceName)
{
    UtFreeMem((LPWSTR)pszPortDeviceName);
}

#pragma managed

System::Boolean Pep::Programmer::Config::Initialize(
  Pep::Programmer::IDeviceChange^ pDeviceChange)
{
    if (UtInitHeap() == FALSE)
    {
        return false;
    }

    s_pDeviceChange = pDeviceChange;

    if (lUnmanagedInitializePepCtrl() == FALSE)
    {
        UtUninitHeap();

        return false;
    }

    return true;
}

System::Boolean Pep::Programmer::Config::Uninitialize()
{
    UtUninitHeap();

    s_pDeviceChange = nullptr;

    return (lUnmanagedUninitializePepCtrl() == TRUE) ? true : false;
}

System::Boolean Pep::Programmer::Config::Reset()
{
    System::Boolean bResult;

    bResult = (lUnmanagedReset() == TRUE) ? true : false;

    return bResult;
}

Pep::Programmer::IDeviceChange^ Pep::Programmer::Config::GetDeviceChange()
{
    return s_pDeviceChange;
}

System::Boolean Pep::Programmer::Config::GetDevicePresent()
{
    System::Boolean bResult;

    bResult = (lUnmanagedIsDevicePresent() == TRUE) ? true : false;

    return bResult;
}

System::String^ Pep::Programmer::Config::GetPortType()
{
    System::String^ sPortType(L"Unknown");
    EUtPepCtrlCfgPortType CfgPortType;

    if (TRUE == lUnmanagedGetPortType(&CfgPortType))
    {
        switch (CfgPortType)
        {
            case eUtPepCtrlCfgParallelPortType:
                sPortType = L"Parallel";
                break;
            case eUtPepCtrlCfgUsbPrintPortType:
                sPortType = L"USB Print";
                break;
        }
    }

    return sPortType;
}

System::String^ Pep::Programmer::Config::GetPortDeviceName()
{
    System::String^ sPortDeviceName(L"");
    LPCWSTR pszPortDeviceName;

    pszPortDeviceName = lUnmanagedAllocPortDeviceName();

    if (pszPortDeviceName)
    {
        sPortDeviceName = gcnew System::String(pszPortDeviceName);

        lUnmanagedFreePortDeviceName(pszPortDeviceName);
    }

    return sPortDeviceName;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
