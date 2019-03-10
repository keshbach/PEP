/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <UtilsDevice/UtPepDevices.h>

#include "IDeviceIO.h"

#include "PinConfigValues.h"
#include "PinConfig.h"

namespace Pep
{
	namespace Programmer
	{
		public ref class PALData sealed
		{
		public:
			delegate void ReadDeviceDelegate(Pep::Programmer::IDeviceIO^ pDeviceIO, array<System::Byte>^ byData, array<Pep::Programmer::PinConfig^>^ PinConfigArray);

		public:
			property System::UInt32 FuseMapSize
			{
				System::UInt32 get()
				{
					return m_nFuseMapSize;
				}
			}

			property array<Pep::Programmer::PinConfigValues^>^ PinConfigValues
			{
				array<Pep::Programmer::PinConfigValues^>^ get()
				{
					return m_PinConfigValuesArray;
				}
			}

			property ReadDeviceDelegate^ ReadDeviceFunc
			{
				ReadDeviceDelegate^ get()
				{
					return m_ReadDeviceDelegate;
				}
			}

		internal:
			PALData(const TPALData* pPALData,
                    TUtPepDevicesInitFunc pInitDeviceFunc,
                    TUtPepDevicesUninitFunc pUninitDeviceFunc,
                    LPCWSTR pszDeviceName,
                    UINT nPinCount);
			~PALData();
			!PALData();

            void Close();

        public:
			System::Boolean WriteJEDFile(System::String^ sFile, array<System::Byte>^ byData);
			System::Boolean WriteJEDText(array<System::Byte>^ byData, System::String^% sText);

		private:
			void ReadDevice(Pep::Programmer::IDeviceIO^ pDeviceIO, array<System::Byte>^ byData, array<PinConfig^>^ PinConfigArray);

		private:
			System::UInt32 m_nFuseMapSize;
			array<Pep::Programmer::PinConfigValues^>^ m_PinConfigValuesArray;
			ReadDeviceDelegate^ m_ReadDeviceDelegate;
			TUtPepDevicesSetDevicePinConfigsFunc m_pSetDevicePinConfigsFunc;
			TUtPepDevicesReadDeviceFunc m_pReadDeviceFunc;
			TUtPepDevicesInitFunc m_pInitDeviceFunc;
			TUtPepDevicesUninitFunc m_pUninitDeviceFunc;
			TPALData* m_pPALData;
			LPCWSTR m_pszDeviceName;
			UINT m_nPinCount;
		};
	}
};

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
