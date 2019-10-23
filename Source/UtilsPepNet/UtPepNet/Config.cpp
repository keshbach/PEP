/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "IDeviceChange.h"

#include "Config.h"

#include <Config/UtPepCtrl.h>

#include <Utils/UtHeap.h>

#include <assert.h>

#pragma region "Local Functions"

static VOID UTPEPCTRLAPI lPepCtrlDeviceChange(
  EUtPepCtrlDeviceChange DeviceChange)
{
    switch (DeviceChange)
    {
        case eUtPepCtrlDeviceArrived:
			Pep::Programmer::Config::GetDeviceChange()->DeviceChange(Pep::Programmer::IDeviceChange::ENotification::Arrived);
			break;
        case eUtPepCtrlDeviceRemoved:
			Pep::Programmer::Config::GetDeviceChange()->DeviceChange(Pep::Programmer::IDeviceChange::ENotification::Removed);
			break;
        default:
			System::Diagnostics::Debug::Assert(false, "Unknown device change");
            break;
    }
}

#pragma endregion

void Pep::Programmer::Config::Initialize(
  Pep::Programmer::IDeviceChange^ pDeviceChange)
{
	if (UtPepCtrlInitialize(&lPepCtrlDeviceChange))
    {
		s_pDeviceChange = pDeviceChange;
		s_bInitialized = true;
    }
}

void Pep::Programmer::Config::Uninitialize()
{
	if (s_bInitialized)
	{
		s_pDeviceChange = nullptr;
		s_bInitialized = false;

		UtPepCtrlUninitialize();
	}
}

System::Boolean Pep::Programmer::Config::Reset()
{
	if (s_bInitialized)
	{
		return UtPepCtrlReset() ? true : false;
	}

	return false;
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

	if (!s_bInitialized)
	{
		return false;
	}

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

	if (!s_bInitialized)
	{
		return false;
	}

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
    System::Boolean bResult = false;
	BOOL bPresent;

	if (s_bInitialized)
	{
		if (UtPepCtrlIsDevicePresent(&bPresent))
		{
			return bPresent ? true : false;
		}
	}

    return bResult;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
