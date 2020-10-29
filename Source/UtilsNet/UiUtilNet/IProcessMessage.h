/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2016-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
	namespace Forms
	{
		private interface class IProcessMessage
		{
		public:
            virtual void ProcessKeyDown(System::Windows::Forms::Control^ control, System::Int32 nVirtKey, System::Int32 nData);
			virtual void ProcessKeyUp(System::Windows::Forms::Control^ control, System::Int32 nVirtKey, System::Int32 nData);
			virtual void ProcessMouseMove(System::Windows::Forms::Control^ control, System::Int32 nXPos, System::Int32 nYPos);
		};
	}
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2016-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
