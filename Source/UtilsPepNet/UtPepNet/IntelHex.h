/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2024 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Pep
{
	namespace Programmer
	{
		public ref class IntelHex sealed
		{
		public:
			static System::Boolean LoadIntelHexFile(System::String^ sFile, IntelHexData^% IntelHexData, System::String^% sErrorMessage);
			static System::Boolean IntelHexFromMemory(array<System::Byte>^ byData, IntelHexData^% IntelHexData, System::String^% sErrorMessage);
		};
	}
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2024 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
