/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

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
					return CreatePluginsList();
				}
			}

			/// <summary>
			/// Retrieves all of the available PEP Devices.
			/// </summary>

            static property System::Collections::Generic::List<Pep::Programmer::Device^>^ DevicesList
			{
                System::Collections::Generic::List<Pep::Programmer::Device^>^ get()
				{
					return CreateDevicesList();
				}
			}

		public:
			/// <summary>
			/// Initialize the fuse map data of a PAL.
			/// </summary>
			/// <returns>Returns true if successful.</returns>

			static System::Boolean InitializePALFuseMap(array<System::Byte>^ byData);

		private:
			static System::Collections::Generic::List<Pep::Programmer::Plugin^>^ CreatePluginsList();
			static System::Collections::Generic::List<Pep::Programmer::Device^>^ CreateDevicesList();
        };
	}
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
