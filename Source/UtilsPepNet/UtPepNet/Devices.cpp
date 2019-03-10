/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "Devices.h"

#include "UtilsDevice/UtPAL.h"

System::Boolean Pep::Programmer::Devices::Initialize(
  System::String^ sPluginPath)
{
    System::Boolean bResult = false;
    pin_ptr<const wchar_t> pszPluginPath = PtrToStringChars(sPluginPath);

    if (UtPepDevicesInitialize(pszPluginPath))
    {
        UtPALInitialize();

        s_PluginsList = gcnew System::Collections::Generic::List<Pep::Programmer::Plugin^>();
        s_DevicesList = gcnew System::Collections::Generic::List<Pep::Programmer::Device^>();

        bResult = true;
    }

    return bResult;
}

System::Boolean Pep::Programmer::Devices::Uninitialize()
{
    if (s_DevicesList != nullptr)
    {
        for each (Pep::Programmer::Device^ Device in s_DevicesList)
        {
            Device->Close();
        }

        s_DevicesList->Clear();

        delete s_DevicesList;

        s_DevicesList = nullptr;
    }

    if (s_PluginsList != nullptr)
    {
        s_PluginsList->Clear();

        delete s_PluginsList;

        s_PluginsList = nullptr;
    }

    UtPALUninitialize();

    return UtPepDevicesUninitialize() ? true : false;
}

System::Boolean Pep::Programmer::Devices::InitializePALFuseMap(
  array<System::Byte>^ byData)
{
    pin_ptr<System::Byte> pbyData = &byData[0];

    return UtPALClearFuseMap(pbyData, byData->Length) ? true : false;
}

void Pep::Programmer::Devices::InitPluginsList(void)
{
    Pep::Programmer::Plugin^ pPlugin;
    ULONG ulPluginCount;
    LPCWSTR pszName;
    WORD wProductMajorVersion, wProductMinorVersion;
    WORD wProductBuildVersion, wProductPrivateVersion;

    if (TRUE == UtPepDevicesGetPluginCount(&ulPluginCount))
    {
        for (ULONG ulIndex = 0; ulIndex < ulPluginCount; ++ulIndex)
        {
            if (TRUE == UtPepDevicesGetPluginName(ulIndex, &pszName) &&
                TRUE == UtPepDevicesGetPluginVersion(ulIndex, &wProductMajorVersion,
                                                     &wProductMinorVersion,
                                                     &wProductBuildVersion,
                                                     &wProductPrivateVersion))
            {
                pPlugin = gcnew Pep::Programmer::Plugin(pszName, wProductMajorVersion,
 													    wProductMinorVersion,
 													    wProductBuildVersion,
 													    wProductPrivateVersion);
      
                s_PluginsList->Add(pPlugin);
            }
        }
    }
}

void Pep::Programmer::Devices::InitDevicesList(void)
{
	Pep::Programmer::Device^ pDevice;
    ULONG ulDeviceCount;
    TDevice Device;

    if (TRUE == UtPepDevicesGetDeviceCount(&ulDeviceCount))
    {
        for (ULONG ulIndex = 0; ulIndex < ulDeviceCount; ++ulIndex)
        {
            if (TRUE == UtPepDevicesGetDevice(ulIndex, &Device))
            {
				pDevice = gcnew Pep::Programmer::Device(&Device);
      
                s_DevicesList->Add(pDevice);
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
