/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <UtilsDevice/UtPepDevices.h>

#include "IDeviceIO.h"

namespace Pep
{
	namespace Programmer
	{
		public ref class ROMData sealed
		{
		public:
			delegate void ReadDeviceDelegate(Pep::Programmer::IDeviceIO^ pDeviceIO, array<System::Byte>^ byData);
			delegate void ProgramDeviceDelegate(Pep::Programmer::IDeviceIO^ pDeviceIO, array<System::Byte>^ byData);
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

			property System::String^ DeviceVpp
			{
				System::String^ get()
				{
					return m_sDeviceVpp;
				}
			}

			property ReadDeviceDelegate^ ReadDeviceFunc
			{
				ReadDeviceDelegate^ get()
				{
					return m_ReadDeviceDelegate;
				}
			}

			property ProgramDeviceDelegate^ ProgramDeviceFunc
			{
				ProgramDeviceDelegate^ get()
				{
					return m_ProgramDeviceDelegate;
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
			ROMData(const TROMData* pROMData,
				TUtPepDevicesInitFunc pInitDeviceFunc,
				TUtPepDevicesUninitFunc pUninitDeviceFunc);
			~ROMData();

		private:
			void ReadDevice(Pep::Programmer::IDeviceIO^ pDeviceIO, array<System::Byte>^ byData);
			void ProgramDevice(Pep::Programmer::IDeviceIO^ pDeviceIO, array<System::Byte>^ byData);
			void VerifyDevice(Pep::Programmer::IDeviceIO^ pDeviceIO, array<System::Byte>^ byData);

		private:
			System::UInt32 m_nSize;
			System::UInt32 m_nBitsPerValue;
			System::String^ m_sDeviceVpp;
			ReadDeviceDelegate^ m_ReadDeviceDelegate;
			ProgramDeviceDelegate^ m_ProgramDeviceDelegate;
			VerifyDeviceDelegate^ m_VerifyDeviceDelegate;
			TUtPepDevicesReadDeviceFunc m_pReadDeviceFunc;
			TUtPepDevicesProgramDeviceFunc m_pProgramDeviceFunc;
			TUtPepDevicesVerifyDeviceFunc m_pVerifyDeviceFunc;
			TUtPepDevicesInitFunc m_pInitDeviceFunc;
			TUtPepDevicesUninitFunc m_pUninitDeviceFunc;

		private:
			ROMData();
		};
	}
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
