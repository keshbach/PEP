/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2011-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
	namespace IO
	{
		public ref class TempFileManager sealed
		{
		public:
			static System::Boolean Initialize(System::String^% sErrorMsg);
			static System::Boolean Uninitialize(System::String^% sErrorMsg);
			static System::Boolean CreateTempFile(System::String^ sExtension,
												  System::String^% sTempFile,
												  System::String^% sErrorMsg);

		private:
			static System::Collections::Specialized::StringCollection^ s_TempFilesStringColl = nullptr;
		};
	}
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
