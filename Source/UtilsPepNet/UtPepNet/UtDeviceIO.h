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
		private ref class UtDeviceIO sealed
		{
		internal:
			static TDeviceIOFuncs* GetDeviceIOFuncs(void);
			static Pep::Programmer::IDeviceIO^ GetCurrentDeviceIO(void);
			static void SetCurrentDeviceIO(Pep::Programmer::IDeviceIO^ pDeviceIO);

		private:
			static Pep::Programmer::IDeviceIO^ s_pDeviceIO = nullptr;
		};
	}
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
