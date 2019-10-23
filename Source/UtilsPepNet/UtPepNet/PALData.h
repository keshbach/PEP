/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

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
			PALData(_In_ const TPALData* pPALData,
                    _In_ TUtPepDevicesInitFunc pInitDeviceFunc,
                    _In_ TUtPepDevicesUninitFunc pUninitDeviceFunc,
                    _In_ LPCWSTR pszDeviceName,
                    _In_ UINT nPinCount);
			~PALData();

        public:
			System::Boolean WriteJEDFile(System::String^ sFile, array<System::Byte>^ byData);
			System::Boolean WriteJEDText(array<System::Byte>^ byData, System::String^% sText);

		private:
			void ReadDevice(_In_ Pep::Programmer::IDeviceIO^ pDeviceIO, _In_ array<System::Byte>^ byData, _In_ array<PinConfig^>^ PinConfigArray);

			void InitPALData(_In_ TPALData* pPALData);

		private:
			System::UInt32 m_nFuseMapSize;
			array<Pep::Programmer::PinConfigValues^>^ m_PinConfigValuesArray;
			ReadDeviceDelegate^ m_ReadDeviceDelegate;
			TUtPepDevicesInitFunc m_pInitDeviceFunc;
			TUtPepDevicesUninitFunc m_pUninitDeviceFunc;

			// TPALData members

			TDevicePinConfigValues* m_pDevicePinConfigValues;
			UINT m_nDevicePinConfigValuesCount;
			TDevicePinFuseRows* m_pDevicePinFuseRows;
			UINT m_nDevicePinFuseRowsCount;
			TDevicePinFuseColumns* m_pDevicePinFuseColumns;
			UINT m_nDevicePinFuseColumnsCount;
			TUtPepDevicesGetFuseMapSizeFunc m_pGetFuseMapSizeFunc;
			TUtPepDevicesConfigFuseMapFunc m_pConfigFuseMapFunc;
			TUtPepDevicesSetDevicePinConfigsFunc m_pSetDevicePinConfigsFunc;
			TUtPepDevicesAllocFuseMapTextFunc m_pAllocFuseMapTextFunc;
			TUtPepDevicesFreeFuseMapTextFunc m_pFreeFuseMapTextFunc;
			TUtPepDevicesAllocSampleFuseMapTextFunc m_pAllocSampleFuseMapTextFunc;
			TUtPepDevicesFreeSampleFuseMapTextFunc m_pFreeSampleFuseMapTextFunc;
			TUtPepDevicesReadDeviceFunc m_pReadDeviceFunc;

			LPCWSTR m_pszDeviceName;
			UINT m_nPinCount;
		};
	}
};

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
