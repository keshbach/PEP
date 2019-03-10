/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Pep
{
	namespace Programmer
	{
		public interface class IDeviceIO
		{
		public:
			enum class EDeviceOperation
			{
				Read,
				Write,
				Program,
				Verify,
				Unknown
			};

		public:
			virtual void BeginDeviceIO(System::UInt32 nTotalAddresses, EDeviceOperation DeviceOperation);
			virtual void EndDeviceIO(System::Boolean bErrorOccurred, EDeviceOperation DeviceOperation);
			virtual void ProgressDeviceIO(System::UInt32 nAddress);
			virtual void VerifyByteErrorDeviceIO(System::UInt32 nAddress, System::Byte byExpected, System::Byte byFound);
			virtual void VerifyWordErrorDeviceIO(System::UInt32 nAddress, System::UInt16 wExpected, System::UInt16 wFound);
			virtual System::Boolean ContinueDeviceIO(void);
			virtual void ShowMessageDeviceIO(System::String^ sMessage);
			virtual void LogMessageDeviceIO(System::String^ sMessage);
		};
	}
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
