/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2016-2016 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
	namespace Forms
	{
		public interface class IProcessMessage
		{
		public:
            virtual void ProcessKeyDown(System::Windows::Forms::Control^ control, System::Int32 nVirtKey, System::Int32 nData);
            virtual void ProcessSysKeyDown(System::Windows::Forms::Control^ control, System::Int32 nVirtKey, System::Int32 nData);
            virtual void ProcessLeftButtonDown(System::Windows::Forms::Control^ control, System::Int32 nXPos, System::Int32 nYPos);
            virtual void ProcessLeftButtonUp(System::Windows::Forms::Control^ control, System::Int32 nXPos, System::Int32 nYPos);
            virtual void ProcessMouseMove(System::Windows::Forms::Control^ control, System::Int32 nXPos, System::Int32 nYPos);
		};
	}
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2016-2016 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
