/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Pep
{
	namespace Programmer
	{
        public ref class Config abstract sealed
		{
        public:
            enum class EPortType
            {
                None,
                Parallel,
                USBPrint
            };
        
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

            /// <summary>
            /// Retrieves the current port configuration.
            /// </summary>
            /// <param name="PortType">The type of port.</param>
            /// <param name="sPortDeviceName">The name of the port device.</param>
            /// <returns>Returns true if successful.</returns>

            static System::Boolean GetPortConfig([System::Runtime::InteropServices::Out] EPortType% PortType, [System::Runtime::InteropServices::Out] System::String^% sPortDeviceName);

            /// <summary>
            /// Changes the current port configuration.
            /// </summary>
            /// <param name="PortType">The type of port.</param>
            /// <param name="sPortDeviceName">The name of the port device.</param>
            /// <returns>Returns true if successful.</returns>

            static System::Boolean SetPortConfig(EPortType PortType, System::String^ sPortDeviceName);

		internal:
			static Pep::Programmer::IDeviceChange^ GetDeviceChange();

		private:
			static System::Boolean GetDevicePresent();

		private:
			static Pep::Programmer::IDeviceChange^ s_pDeviceChange = nullptr;
		};
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
