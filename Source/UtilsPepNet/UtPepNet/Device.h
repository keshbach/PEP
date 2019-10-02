/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Pep
{
	namespace Programmer
	{
		public ref class Device sealed
		{
		public:
			property System::String^ Name
			{
				System::String^ get()
				{
					return m_sName;
				}
			}

			property System::String^ DeviceType
			{
				System::String^ get()
				{
					return m_sDeviceType;
				}
			}

			property System::String^ DevicePackage
			{
				System::String^ get()
				{
					return m_sDevicePackage;
				}
			}

			property System::UInt32 PinCount
			{
				System::UInt32 get()
				{
					return m_nPinCount;
				}
			}

			property array<System::String^>^ PinNames
			{
				array<System::String^>^ get()
				{
					return m_sPinNames;
				}
			}

			property System::String^ PinNamesDiagram
			{
				System::String^ get()
				{
					return m_sPinNamesDiagram;
				}
			}

			property System::String^ Adapter
			{
				System::String^ get()
				{
					return m_sAdapter;
				}
			}

			property System::String^ Message
			{
				System::String^ get()
				{
					return m_sMessage;
				}
			}

			property array<System::Boolean>^ DipSwitches
			{
				array<System::Boolean>^ get()
				{
					return m_bDipSwitches;
				}
			}

			property System::Object^ DeviceData
			{
				System::Object^ get()
				{
					return m_pDeviceData;
				}
			}

		internal:
			Device(_In_ const TDevice* pDevice);
			~Device();

		private:
			System::String^ m_sName;
			System::String^ m_sDeviceType;
			System::String^ m_sDevicePackage;
			System::UInt32 m_nPinCount;
			array<System::String^>^ m_sPinNames;
			System::String^ m_sPinNamesDiagram;
			System::String^ m_sAdapter;
			System::String^ m_sMessage;
			array<System::Boolean>^ m_bDipSwitches;
			System::Object^ m_pDeviceData;
		};
	}
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
