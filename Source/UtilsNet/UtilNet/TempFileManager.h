/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2011-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Common
{
	namespace IO
	{
		public ref class TempFileManager sealed
		{
		public:
			static System::Boolean Initialize();
			static System::Boolean Uninitialize();
			static System::Boolean CreateTempFile(System::String^ sExtension,
												  System::String^% sTempFile);

		private:
			static System::Collections::Specialized::StringCollection^ s_TempFilesStringColl = nullptr;
		};
	}
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
