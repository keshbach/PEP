/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "IDeviceChange.h"

namespace Pep
{
	namespace Programmer
	{
        public ref class Config sealed
		{
		public:
			/// <summary>
			/// Determines if the PEP device is present.
			/// </summary>

			static property System::Boolean DevicePresent
			{
				System::Boolean get()
				{
					return GetDevicePresent();
				}
			}

			/// <summary>
			/// Retrieves the type of port that the PEP device driver uses.
			/// </summary>

            static property System::String^ PortType
			{
				System::String^ get()
				{
					return GetPortType();
				}
			}

			/// <summary>
			/// Retrieves the name of the device that the PEP device driver uses.
			/// </summary>

			static property System::String^ PortDeviceName
			{
				System::String^ get()
				{
					return GetPortDeviceName();
				}
			}

		public:
			/// <summary>
			/// Initializes the PEP device driver.
			/// </summary>
			/// <returns>Returns true if successful.</returns>

			static System::Boolean Initialize(Pep::Programmer::IDeviceChange^ pDeviceChange);

			/// <summary>
			/// Uninitializes the PEP device driver.
			/// </summary>
			/// <returns>Returns true if successful.</returns>

			static System::Boolean Uninitialize();

			/// <summary>
			/// Resets the PEP device driver.  (Use before executing a device operation.)
			/// </summary>

			static System::Boolean Reset();

		internal:
			static Pep::Programmer::IDeviceChange^ GetDeviceChange();

		private:
			static System::Boolean GetDevicePresent();
			static System::String^ GetPortType();
			static System::String^ GetPortDeviceName();

		private:
			static Pep::Programmer::IDeviceChange^ s_pDeviceChange = nullptr;
		};
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
