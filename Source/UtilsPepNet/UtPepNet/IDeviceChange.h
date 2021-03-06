/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Pep
{
	namespace Programmer
	{
		public interface class IDeviceChange
		{
		public:
			enum class ENotification
			{
				Arrived,
				Removed
			};

		public:
			virtual void DeviceChange(ENotification DeviceChange);
		};
	}
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
