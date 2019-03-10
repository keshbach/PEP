/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ComboBox.h"
#include "ListViewComboBox.h"

Common::Forms::ListViewComboBox::ListViewComboBox()
{
	InitializeComponent();
}

Common::Forms::ListViewComboBox::~ListViewComboBox()
{
	if (components)
	{
		delete components;
	}
}

void Common::Forms::ListViewComboBox::WndProc(
  System::Windows::Forms::Message% Message)
{
    switch (Message.Msg)
    {
        case WM_SETFOCUS:
            HandleSetFocus(Message);
            break;
        case WM_KILLFOCUS:
            HandleKillFocus(Message);
            break;
        case WM_KEYDOWN:
            HandleKeydown(Message);
            break;
		case WM_GETDLGCODE:
			HandleGetDlgCode(Message);
			break;
        default:
            Common::Forms::ComboBox::WndProc(Message);
            break;
    }
}

void Common::Forms::ListViewComboBox::HandleSetFocus(
  System::Windows::Forms::Message% Message)
{
    System::EventArgs^ EventArg = gcnew System::EventArgs();

    Common::Forms::ComboBox::WndProc(Message);

    ListViewComboBoxSetFocus(this, EventArg);
}

void Common::Forms::ListViewComboBox::HandleKillFocus(
  System::Windows::Forms::Message% Message)
{
    System::EventArgs^ EventArg = gcnew System::EventArgs();

    Common::Forms::ComboBox::WndProc(Message);

    ListViewComboBoxKillFocus(this, EventArg);
}

void Common::Forms::ListViewComboBox::HandleKeydown(
  System::Windows::Forms::Message% Message)
{
    WPARAM wParam = (WPARAM)Message.WParam.ToPointer();
    ListViewComboBoxKeydownEventArgs^ ListViewComboBoxKeydownEventArg;

	switch (wParam)
	{
		case VK_ESCAPE:
            ListViewComboBoxKeydownEventArg = gcnew ListViewComboBoxKeydownEventArgs(System::Windows::Forms::Keys::Escape);

            ListViewComboBoxKeydown(this, ListViewComboBoxKeydownEventArg);
			break;
		case VK_RETURN:
            ListViewComboBoxKeydownEventArg = gcnew ListViewComboBoxKeydownEventArgs(System::Windows::Forms::Keys::Enter);

            ListViewComboBoxKeydown(this, ListViewComboBoxKeydownEventArg);
			break;
        case VK_TAB:
            ListViewComboBoxKeydownEventArg = gcnew ListViewComboBoxKeydownEventArgs(System::Windows::Forms::Keys::Tab);

            ListViewComboBoxKeydown(this, ListViewComboBoxKeydownEventArg);
            break;
        default:
    	    Common::Forms::ComboBox::WndProc(Message);
            break;
	}
}

void Common::Forms::ListViewComboBox::HandleGetDlgCode(
  System::Windows::Forms::Message% Message)
{
    Message.Result = (System::IntPtr)DLGC_WANTALLKEYS;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
