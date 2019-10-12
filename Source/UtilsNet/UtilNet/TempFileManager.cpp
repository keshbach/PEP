/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2011-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "TempFileManager.h"

#include "File.h"

System::Boolean Common::IO::TempFileManager::Initialize()
{
	if (s_TempFilesStringColl != nullptr)
	{
		return false;
	}

    s_TempFilesStringColl = gcnew System::Collections::Specialized::StringCollection();

    return true;
}

System::Boolean Common::IO::TempFileManager::Uninitialize()
{
    System::Boolean bResult = true;

	if (s_TempFilesStringColl == nullptr)
	{
		return false;
	}

    for each (System::String^ sFile in s_TempFilesStringColl)
    {
        try
        {
            System::IO::File::Delete(sFile);
        }
        catch (System::Exception^)
        {
            bResult = false;
        }
    }

    delete s_TempFilesStringColl;

    s_TempFilesStringColl = nullptr;

    return bResult;
}

System::Boolean Common::IO::TempFileManager::CreateTempFile(
  System::String^ sExtension,
  System::String^% sTempFile)
{
    System::Boolean bResult = false;

    sTempFile = L"";

    try
    {
        sTempFile = Common::IO::File::GenerateTempFileName(sExtension);

        s_TempFilesStringColl->Add(sTempFile);

        delete sTempFile;

        bResult = true;
    }
    catch (System::Exception^)
    {
    }

    return bResult;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2011-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
