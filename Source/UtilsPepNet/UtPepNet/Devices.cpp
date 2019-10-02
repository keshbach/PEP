/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "Stdafx.h"

#include <UtilsDevice/UtPepDevices.h>
#include <UtilsDevice/UtPAL.h>

#include "Plugin.h"
#include "Device.h"

#include "Devices.h"

System::Boolean Pep::Programmer::Devices::InitializePALFuseMap(
  array<System::Byte>^ byData)
{
    pin_ptr<System::Byte> pbyData = &byData[0];

    return UtPALClearFuseMap(pbyData, byData->Length) ? true : false;
}

System::Collections::Generic::List<Pep::Programmer::Plugin^>^ Pep::Programmer::Devices::CreatePluginsList()
{
	System::Collections::Generic::List<Pep::Programmer::Plugin^>^ PluginList;
	Pep::Programmer::Plugin^ pPlugin;
	ULONG ulPluginCount;
	LPCWSTR pszName;
	WORD wProductMajorVersion, wProductMinorVersion;
	WORD wProductBuildVersion, wProductPrivateVersion;

	if (TRUE == UtPepDevicesGetPluginCount(&ulPluginCount))
	{
		PluginList = gcnew System::Collections::Generic::List<Pep::Programmer::Plugin^>(ulPluginCount);

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

				PluginList->Add(pPlugin);
			}
		}
	}
	else
	{
		PluginList = gcnew System::Collections::Generic::List<Pep::Programmer::Plugin^>(0);
	}

	return PluginList;
}

System::Collections::Generic::List<Pep::Programmer::Device^>^ Pep::Programmer::Devices::CreateDevicesList()
{
	System::Collections::Generic::List<Pep::Programmer::Device^>^ DeviceList;
	Pep::Programmer::Device^ pDevice;
	ULONG ulDeviceCount;
	TDevice Device;

	if (TRUE == UtPepDevicesGetDeviceCount(&ulDeviceCount))
	{
		DeviceList = gcnew System::Collections::Generic::List<Pep::Programmer::Device^>(ulDeviceCount);

		for (ULONG ulIndex = 0; ulIndex < ulDeviceCount; ++ulIndex)
		{
			if (TRUE == UtPepDevicesGetDevice(ulIndex, &Device))
			{
				pDevice = gcnew Pep::Programmer::Device(&Device);

				DeviceList->Add(pDevice);
			}
		}
	}
	else
	{
		DeviceList = gcnew System::Collections::Generic::List<Pep::Programmer::Device^>(0);
	}

	return DeviceList;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
