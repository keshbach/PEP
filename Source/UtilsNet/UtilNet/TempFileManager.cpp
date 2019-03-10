/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2011-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "TempFileManager.h"

#include "File.h"

System::Boolean Common::IO::TempFileManager::Initialize(
  System::String^% sErrorMsg)
{
    sErrorMsg = L"";

	if (s_TempFilesStringColl != nullptr)
	{
		sErrorMsg = L"Temporary File Manager has already been initialized.";

		return false;
	}

    s_TempFilesStringColl = gcnew System::Collections::Specialized::StringCollection();

    return true;
}

System::Boolean Common::IO::TempFileManager::Uninitialize(
  System::String^% sErrorMsg)
{
    System::Boolean bResult = true;
    System::Text::StringBuilder sb;

	sErrorMsg = L"";

	if (s_TempFilesStringColl == nullptr)
	{
		sErrorMsg = L"Temporary File Manager was never initialized.";

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
            sb.Append(sFile);
            sb.Append(L"\n");

            bResult = false;
        }
    }

    if (!bResult)
    {
        sb.Insert(0, L"These temporary files could not be deleted.\n\n");
    }

    sErrorMsg = sb.ToString();

    delete s_TempFilesStringColl;

    s_TempFilesStringColl = nullptr;

    return bResult;
}

System::Boolean Common::IO::TempFileManager::CreateTempFile(
  System::String^ sExtension,
  System::String^% sTempFile,
  System::String^% sErrorMsg)
{
    System::Boolean bResult = false;

    sTempFile = L"";
    sErrorMsg = L"";

    try
    {
        sTempFile = Common::IO::File::GenerateTempFileName(sExtension);

        s_TempFilesStringColl->Add(sTempFile);

        delete sTempFile;

        bResult = true;
    }
    catch (System::Exception^)
    {
        sErrorMsg = L"Temporary file could not be created.";
    }

    return bResult;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2011-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
