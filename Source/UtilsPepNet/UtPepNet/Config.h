/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Pep
{
	namespace Programmer
	{
        public ref class Config abstract sealed
		{
        public:
            enum class EDeviceType
            {
                ParallelPort,
                USB
            };
        
        public:
			static property EDeviceType DeviceType
			{
				EDeviceType get()
				{
					return s_DeviceType;
				}
			}

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
			/// Has the PEP device been initialized.
			/// </summary>
			static property System::Boolean Initialized
			{
				System::Boolean get()
				{
					return s_bInitialized;
				}
			}

			/// <summary>
			/// Enable/disable the device change notification.
			/// </summary>

			static property System::Boolean DeviceChangeNotification
			{
				System::Boolean get()
				{
					return s_bDeviceChangeNotification;
				}

				void set(System::Boolean value)
				{
					s_bDeviceChangeNotification = value;
				}
			}

		public:
			/// <summary>
			/// Initializes the PEP device driver.
			/// </summary>
			/// <param name="DeviceType">The device type.</param>
			/// <param name="DeviceChange"></param>

			static void Initialize(EDeviceType DeviceType, Pep::Programmer::IDeviceChange^ DeviceChange);

			/// <summary>
			/// Uninitializes the PEP device driver.
			/// </summary>

			static void Uninitialize();

			/// <summary>
			/// Resets the PEP device driver.  (Use before executing a device operation.)
			/// </summary>

			static System::Boolean Reset();

            /// <summary>
            /// Retrieves the current device name.
            /// </summary>
            /// <param name="sDeviceName">The name of the device.</param>
            /// <returns>Returns true if successful.</returns>

            static System::Boolean GetDeviceName([System::Runtime::InteropServices::Out] System::String^% sDeviceName);

            /// <summary>
            /// Changes the current device name.
            /// </summary>
            /// <param name="sDeviceName">The name of the port device.</param>
            /// <returns>Returns true if successful.</returns>

            static System::Boolean SetDeviceName(System::String^ sDeviceName);

		internal:
			static property Pep::Programmer::IDeviceChange^ DeviceChange
			{
				Pep::Programmer::IDeviceChange^ get()
				{
					return s_pDeviceChange;
				}
			}

		private:
			static System::Boolean GetDevicePresent();

		private:
			static EDeviceType s_DeviceType = EDeviceType::USB;
			static Pep::Programmer::IDeviceChange^ s_pDeviceChange = nullptr;
			static System::Boolean s_bInitialized = false;
			static System::Boolean s_bDeviceChangeNotification = false;
		};
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
