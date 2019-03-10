/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

using namespace System;

namespace Common
{
	public ref class Rebaser abstract sealed
	{
    public:
        static System::Boolean GetFileImageBase(System::String^ sFile,
                                                System::UInt64% nImageBase,
                                                System::String^% sErrorMsg);
        static System::Boolean ReBaseFile(System::String^ sFile,
                                          System::String^ sSymbolPath,
                                          System::UInt64 nImageBase,
                                          System::String^% sErrorMsg);
	};
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
