/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PepAppNet.h"

#using <System.Windows.Forms.dll>

#using "UiUtilNet.dll"
#using "UiPepCtrlsNet.dll"
#using "PepForms.dll"

Pep::Application::Startup::Startup()
{
}

System::UInt32 Pep::Application::Startup::Execute(
  System::String^ sPluginPath)
{
    Pep::Forms::MainForm^ AppForm;

    try
    {
        System::Windows::Forms::Application::EnableVisualStyles();
        System::Windows::Forms::Application::SetCompatibleTextRenderingDefault(false);

        Pep::Controls::Config::Initialize();

        AppForm = gcnew Pep::Forms::MainForm();

        AppForm->PlugPath = sPluginPath;

        System::Windows::Forms::Application::AddMessageFilter(AppForm);

        Common::Forms::Application::Run(AppForm);

        System::Windows::Forms::Application::RemoveMessageFilter(AppForm);

        Pep::Controls::Config::Uninitialize();
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

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
