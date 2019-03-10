/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "MessageBox.h"

#include "Application.h"
#include "MainForm.h"

System::Windows::Forms::DialogResult Common::Forms::MessageBox::Show(
  System::String^ sMsg,
  System::Windows::Forms::MessageBoxButtons Buttons,
  System::Windows::Forms::MessageBoxIcon Icon)
{
    return Show(sMsg, Buttons, Icon, System::Windows::Forms::MessageBoxDefaultButton::Button1);
}

System::Windows::Forms::DialogResult Common::Forms::MessageBox::Show(
  System::Windows::Forms::IWin32Window^ Owner,
  System::String^ sMsg,
  System::Windows::Forms::MessageBoxButtons Buttons,
  System::Windows::Forms::MessageBoxIcon Icon)
{
    return Show(Owner, sMsg, Buttons, Icon,
                System::Windows::Forms::MessageBoxDefaultButton::Button1);
}

System::Windows::Forms::DialogResult Common::Forms::MessageBox::Show(
  System::String^ sMsg,
  System::Windows::Forms::MessageBoxButtons Buttons,
  System::Windows::Forms::MessageBoxIcon Icon,
  System::Windows::Forms::MessageBoxDefaultButton DefButton)
{
    return System::Windows::Forms::MessageBox::Show(
        Common::Forms::Application::s_ApplicationContext->MainForm, sMsg,
        System::Windows::Forms::Application::ProductName,
        Buttons, Icon, DefButton);
}

System::Windows::Forms::DialogResult Common::Forms::MessageBox::Show(
  System::Windows::Forms::IWin32Window^ Owner,
  System::String^ sMsg,
  System::Windows::Forms::MessageBoxButtons Buttons,
  System::Windows::Forms::MessageBoxIcon Icon,
  System::Windows::Forms::MessageBoxDefaultButton DefButton)
{
    return System::Windows::Forms::MessageBox::Show(Owner, sMsg,
        System::Windows::Forms::Application::ProductName,
        Buttons, Icon, DefButton);
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
