/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "IDeviceChange.h"

#include "Config.h"

#include <Config/UtPepCtrl.h>

#include <Utils/UtHeap.h>

#include <assert.h>

#pragma region "Local Functions"

static VOID UTPEPCTRLAPI lPepCtrlDeviceChange(
  _In_ EUtPepCtrlDeviceChange DeviceChange)
{
    if (Pep::Programmer::Config::DeviceChangeNotification == false)
    {
        return;
    }

    while (Pep::Programmer::Config::Initialized == false)
    {
        // Wait until Pep::Programmer::Config::Initialize finishes in case
        // a device notification arrives early

        System::Threading::Thread::Sleep(50);
    }

    switch (DeviceChange)
    {
        case eUtPepCtrlDeviceArrived:
			Pep::Programmer::Config::DeviceChange->DeviceChange(Pep::Programmer::IDeviceChange::ENotification::Arrived);
			break;
        case eUtPepCtrlDeviceRemoved:
			Pep::Programmer::Config::DeviceChange->DeviceChange(Pep::Programmer::IDeviceChange::ENotification::Removed);
			break;
        default:
			System::Diagnostics::Debug::Assert(false, "Unknown device change");
            break;
    }
}

#pragma endregion

void Pep::Programmer::Config::Initialize(
  EDeviceType DeviceType,
  Pep::Programmer::IDeviceChange^ pDeviceChange)
{
    EUtPepCtrlDeviceType PepCtrlDeviceType;

    s_DeviceType = DeviceType;

    switch (DeviceType)
    {
        case EDeviceType::ParallelPort:
            PepCtrlDeviceType = eUtPepCtrlParallelPortDeviceType;
            break;
        case EDeviceType::USB:
            PepCtrlDeviceType = eUtPepCtrlUsbDeviceType;
            break;
        default:
            System::Diagnostics::Debug::Assert(false);
            return;
    }

	if (UtPepCtrlInitialize(PepCtrlDeviceType, &lPepCtrlDeviceChange))
    {
		s_pDeviceChange = pDeviceChange;
		s_bInitialized = true;
    }
}

void Pep::Programmer::Config::Uninitialize()
{
	if (s_bInitialized)
	{
        UtPepCtrlUninitialize();
 
        s_pDeviceChange = nullptr;
		s_bInitialized = false;
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

System::Boolean Pep::Programmer::Config::GetDeviceName(
  [System::Runtime::InteropServices::Out] System::String^% sDeviceName)
{
    LPWSTR pszDeviceName;
    INT nDeviceNameLen;

    sDeviceName = L"";

	if (!s_bInitialized)
	{
		return false;
	}

    if (!UtPepCtrlGetDeviceName(NULL, &nDeviceNameLen) ||
		!UtInitHeap())
    {
        return false;
    }

    pszDeviceName = (LPWSTR)UtAllocMem(nDeviceNameLen * sizeof(WCHAR));

    if (pszDeviceName == NULL)
    {
		UtUninitHeap();
		
		return false;
    }

    if (UtPepCtrlGetDeviceName(pszDeviceName, &nDeviceNameLen))
    {
        sDeviceName = gcnew System::String(pszDeviceName);
    }

    UtFreeMem(pszDeviceName);

	UtUninitHeap();

    return true;
}

System::Boolean Pep::Programmer::Config::SetDeviceName(
  System::String^ sDeviceName)
{
    pin_ptr<const wchar_t> pszDeviceName = PtrToStringChars(sDeviceName);

	if (!s_bInitialized)
	{
		return false;
	}

    return UtPepCtrlSetDeviceName(pszDeviceName) ? true : false;
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
//  Copyright (C) 2007-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
