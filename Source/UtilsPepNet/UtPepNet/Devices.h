/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Plugin.h"
#include "Device.h"

namespace Pep
{
	namespace Programmer
	{
		public ref class Devices abstract sealed
		{
		public:
			/// <summary>
			/// Retrieves all of the available PEP Plugins.
			/// </summary>

            static property System::Collections::Generic::List<Pep::Programmer::Plugin^>^ PluginsList
			{
                System::Collections::Generic::List<Pep::Programmer::Plugin^>^ get()
				{
                    System::Collections::Generic::List<Pep::Programmer::Plugin^>^ pPluginsList = gcnew System::Collections::Generic::List<Pep::Programmer::Plugin^>();

					if (s_PluginsList == nullptr)
					{
						return nullptr;
					}

					if (s_PluginsList->Count == 0)
					{
						InitPluginsList();
					}

					for each (Pep::Programmer::Plugin^ pPlugin in s_PluginsList)
					{
						pPluginsList->Add(pPlugin);
					}

					return pPluginsList;
				}
			}

			/// <summary>
			/// Retrieves all of the available PEP Devices.
			/// </summary>

            static property System::Collections::Generic::List<Pep::Programmer::Device^>^ DevicesList
			{
                System::Collections::Generic::List<Pep::Programmer::Device^>^ get()
				{
                    System::Collections::Generic::List<Pep::Programmer::Device^>^ pDevicesList = gcnew System::Collections::Generic::List<Pep::Programmer::Device^>();

					if (s_DevicesList == nullptr)
					{
						return nullptr;
					}

					if (s_DevicesList->Count == 0)
					{
						InitDevicesList();
					}

					for each (Pep::Programmer::Device^ pDevice in s_DevicesList)
					{
						pDevicesList->Add(pDevice);
					}

					return pDevicesList;
				}
			}

		public:
			/// <summary>
			/// Initializes the PEP Device plugins.
			/// </summary>
			/// <param name="sPluginPath">Fully qualified path to search for PEP Devices.</param>
			/// <returns>Returns true if successful.</returns>

			static System::Boolean Initialize(System::String^ sPluginPath);

			/// <summary>
			/// Uninitializes the PEP Device plugins.
			/// </summary>
			/// <returns>Returns true if successful.</returns>

			static System::Boolean Uninitialize();

			/// <summary>
			/// Initialized the fuse map data of a PAL.
			/// </summary>
			/// <returns>Returns true if successful.</returns>

			static System::Boolean InitializePALFuseMap(array<System::Byte>^ byData);

		private:
			static void InitPluginsList(void);
			static void InitDevicesList(void);

		private:
            static System::Collections::Generic::List<Pep::Programmer::Plugin^>^ s_PluginsList = nullptr;
            static System::Collections::Generic::List<Pep::Programmer::Device^>^ s_DevicesList = nullptr;
        };
	}
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
