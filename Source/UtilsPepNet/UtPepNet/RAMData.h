/////////////////////////////////////////////////////////////////////////////
//  Copyrigit (C) 2007-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Pep
{
	namespace Programmer
	{
		public ref class RAMData sealed
		{
		public:
			delegate void WriteDeviceDelegate(Pep::Programmer::IDeviceIO^ pDeviceIO, array<System::Byte>^ byData);
			delegate void VerifyDeviceDelegate(Pep::Programmer::IDeviceIO^ pDeviceIO, array<System::Byte>^ byData);

		public:
			property System::UInt32 Size
			{
				System::UInt32 get()
				{
					return m_nSize;
				}
			}

			property System::UInt32 BitsPerValue
			{
				System::UInt32 get()
				{
					return m_nBitsPerValue;
				}
			}

			property WriteDeviceDelegate^ WriteDeviceFunc
			{
				WriteDeviceDelegate^ get()
				{
					return m_WriteDeviceDelegate;
				}
			}

			property VerifyDeviceDelegate^ VerifyDeviceFunc
			{
				VerifyDeviceDelegate^ get()
				{
					return m_VerifyDeviceDelegate;
				}
			}

		internal:
			RAMData(_In_ const TRAMData* pRAMData,
                    _In_ TUtPepDevicesInitFunc pInitDeviceFunc,
                    _In_ TUtPepDevicesUninitFunc pUninitDeviceFunc);
			~RAMData();

		private:
			void WriteDevice(Pep::Programmer::IDeviceIO^ pDeviceIO, array<System::Byte>^ byData);
			void VerifyDevice(Pep::Programmer::IDeviceIO^ pDeviceIO, array<System::Byte>^ byData);

		private:
			System::UInt32 m_nSize;
			System::UInt32 m_nBitsPerValue;
			WriteDeviceDelegate^ m_WriteDeviceDelegate;
			VerifyDeviceDelegate^ m_VerifyDeviceDelegate;
			TUtPepDevicesWriteDeviceFunc m_pWriteDeviceFunc;
			TUtPepDevicesVerifyDeviceFunc m_pVerifyDeviceFunc;
			TUtPepDevicesInitFunc m_pInitDeviceFunc;
			TUtPepDevicesUninitFunc m_pUninitDeviceFunc;

		private:
			RAMData();
		};
	}
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
