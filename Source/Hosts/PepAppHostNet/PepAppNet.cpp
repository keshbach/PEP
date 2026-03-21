/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PepAppNet.h"

#using <System.Windows.Forms.dll>

#using "UtilNet.dll"
#using "UiUtilNet.dll"
#using "UtPepNet.dll"
#using "PepForms.dll"

#pragma region "Constants"

#define CRegistryKey L"Software\\Kevin Eshbach\\PepApp\\3.00"

#define CFormLocationsName L"FormLocations"

#pragma endregion

static System::Boolean lReset(void)
{
    System::Boolean bResult = false;
    Microsoft::Win32::RegistryKey^ RegKey = Common::Registry::OpenCurrentUserRegKey(CRegistryKey, true);

    if (RegKey == nullptr)
    {
        return true;
    }

    try
    {
        for each(System::String ^ sSubKeyName in RegKey->GetSubKeyNames())
        {
            RegKey->DeleteSubKeyTree(sSubKeyName);
        }

        bResult = true;
    }
    finally
    {
        RegKey->Close();
    }

    return bResult;
}

Pep::Application::Startup::Startup()
{
}

System::UInt32 Pep::Application::Startup::Execute(
  System::Boolean bUseParallelPort,
  System::Boolean bReset)
{
	System::String^ sFormLocationsRegistryKey = System::String::Format(L"{0}\\{1}", CRegistryKey, CFormLocationsName);
    Pep::Programmer::Config::EDeviceType DeviceType = Pep::Programmer::Config::EDeviceType::USB;
    Pep::Forms::MainForm^ AppForm;

    if (bUseParallelPort)
    {
        DeviceType = Pep::Programmer::Config::EDeviceType::ParallelPort;
    }

    if (bReset)
    {
        return lReset() ? 0 : 1;
    }

	if (!Common::IO::TempFileManager::Initialize())
	{
	}

    try
    {
        System::Windows::Forms::Application::EnableVisualStyles();
        System::Windows::Forms::Application::SetCompatibleTextRenderingDefault(false);

        AppForm = gcnew Pep::Forms::MainForm(CRegistryKey, sFormLocationsRegistryKey);

		gcnew Common::Forms::FormLocation(AppForm, sFormLocationsRegistryKey);

		Pep::Programmer::Config::Initialize(DeviceType, AppForm);

        Common::Forms::Application::Run(AppForm);

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
//  Copyright (C) 2019-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
