/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PepAppNet.h"

#using <System.Windows.Forms.dll>

#using "UtilNet.dll"
#using "UiUtilNet.dll"
#using "UtPepNet.dll"
#using "PepForms.dll"

Pep::Application::Startup::Startup()
{
}

System::UInt32 Pep::Application::Startup::Execute()
{
    Pep::Forms::MainForm^ AppForm;

	if (!Common::IO::TempFileManager::Initialize())
	{
	}

    try
    {
        System::Windows::Forms::Application::EnableVisualStyles();
        System::Windows::Forms::Application::SetCompatibleTextRenderingDefault(false);

        AppForm = gcnew Pep::Forms::MainForm();

		Pep::Programmer::Config::Initialize(AppForm);

        System::Windows::Forms::Application::AddMessageFilter(AppForm);

        Common::Forms::Application::Run(AppForm);

        System::Windows::Forms::Application::RemoveMessageFilter(AppForm);

		Pep::Programmer::Config::Uninitialize();
	}
    catch (System::Exception^ exception)
    {
        System::String^ sMsg;

        sMsg = System::String::Format(L"Unhandled exception.  ({0})\n\nThe application will now automatically close to prevent data loss.", 
                                      exception->Message);

        System::Windows::Forms::MessageBox::Show(sMsg,
            System::Windows::Forms::Application::ProductName,
            System::Windows::Forms::MessageBoxButtons::OK,
            System::Windows::Forms::MessageBoxIcon::Error);
    }

	if (!Common::IO::TempFileManager::Uninitialize())
	{
	}

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
