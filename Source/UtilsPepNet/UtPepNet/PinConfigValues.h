/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <UtilsDevice/UtPepDevices.h>

namespace Pep
{
	namespace Programmer
	{
		public ref class PinConfigValues sealed
		{
		public:
			property System::UInt32 Pin
			{
				System::UInt32 get()
				{
					return m_nPin;
				}
			}

			property array<System::String^>^ Types
			{
				array<System::String^>^ get()
				{
					return m_TypesArray;
				}
			}

			property array<System::String^>^ Polarities
			{
				array<System::String^>^ get()
				{
					return m_PolaritiesArray;
				}
			}

		internal:
			PinConfigValues(const TDevicePinConfigValues* pDevicePinConfigValues);
			~PinConfigValues();

		private:
			System::UInt32 m_nPin;
			array<System::String^>^ m_TypesArray;
			array<System::String^>^ m_PolaritiesArray;

		private:
			PinConfigValues();
		};
	}
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
