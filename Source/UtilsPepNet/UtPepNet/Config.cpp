/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "IDeviceChange.h"

#include "Config.h"

#include <Config/UtPepCtrl.h>

#include <Utils/UtHeap.h>

#include <assert.h>

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

#pragma managed

System::Boolean Pep::Programmer::Config::Initialize(
  Pep::Programmer::IDeviceChange^ pDeviceChange)
{
    s_pDeviceChange = pDeviceChange;

    if (lUnmanagedInitializePepCtrl() == FALSE)
    {
        return false;
    }

    return true;
}

System::Boolean Pep::Programmer::Config::Uninitialize()
{
    s_pDeviceChange = nullptr;

    return (lUnmanagedUninitializePepCtrl() == TRUE) ? true : false;
}

System::Boolean Pep::Programmer::Config::Reset()
{
    return UtPepCtrlReset() ? true : false;
}

System::Boolean Pep::Programmer::Config::GetPortConfig(
  [System::Runtime::InteropServices::Out] EPortType% PortType,
  [System::Runtime::InteropServices::Out] System::String^% sPortDeviceName)
{
    EUtPepCtrlPortType PepCtrlPortType;
    LPWSTR pszPortDeviceName;
    INT nPortDeviceNameLen;

    PortType = Config::EPortType::None;
    sPortDeviceName = L"";

    if (!UtPepCtrlGetPortType(&PepCtrlPortType) ||
        !UtPepCtrlGetPortDeviceName(NULL, &nPortDeviceNameLen) ||
		!UtInitHeap())
    {
        return false;
    }

    pszPortDeviceName = (LPWSTR)UtAllocMem(nPortDeviceNameLen * sizeof(WCHAR));

    if (pszPortDeviceName == NULL)
    {
		UtUninitHeap();
		
		return false;
    }

    if (UtPepCtrlGetPortDeviceName(pszPortDeviceName, &nPortDeviceNameLen))
    {
        sPortDeviceName = gcnew System::String(pszPortDeviceName);
    }

    UtFreeMem(pszPortDeviceName);

	UtUninitHeap();

    switch (PepCtrlPortType)
    {
        case eUtPepCtrlNonePortType:
            PortType = Config::EPortType::None;
            break;
        case eUtPepCtrlParallelPortType:
            PortType = Config::EPortType::Parallel;
            break;
        case eUtPepCtrlUsbPrintPortType:
            PortType = Config::EPortType::USBPrint;
            break;
     }

    return true;
}

System::Boolean Pep::Programmer::Config::SetPortConfig(
  EPortType PortType,
  System::String^ sPortDeviceName)
{
    pin_ptr<const wchar_t> pszPortDeviceName = PtrToStringChars(sPortDeviceName);
    EUtPepCtrlPortType PepCtrlPortType;

    switch (PortType)
    {
        case Config::EPortType::None:
            PepCtrlPortType = eUtPepCtrlNonePortType;
            break;
        case Config::EPortType::Parallel:
            PepCtrlPortType = eUtPepCtrlParallelPortType;
            break;
        case Config::EPortType::USBPrint:
            PepCtrlPortType = eUtPepCtrlUsbPrintPortType;
            break;
        default:
            return false;
    }

    return UtPepCtrlSetPortSettings(PepCtrlPortType, pszPortDeviceName) ? true : false;
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

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
