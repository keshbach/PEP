/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2025 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "TextBoxMessagesTextSelectedEventArgs.h"

#include "TextBoxMessages.h" 

static BOOL lIsTextSelected(
  HWND hWnd)
{
    DWORD dwStartPosition, dwEndPosition;

    ::SendMessage(hWnd, EM_GETSEL, (WPARAM)&dwStartPosition, (LPARAM)&dwEndPosition);

    return dwStartPosition == dwEndPosition ? FALSE : TRUE;
}

Common::Forms::TextBoxMessages::TextBoxMessages()
{
	InitializeComponent();

    m_bTextSelected = false;
}

Common::Forms::TextBoxMessages::~TextBoxMessages()
{
	if (components)
	{
		delete components;
	}
}

void Common::Forms::TextBoxMessages::WndProc(
  System::Windows::Forms::Message% Message)
{
    System::Windows::Forms::TextBox::WndProc(Message);

    switch (Message.Msg)
    {
        case EM_SETSEL:
        case WM_KEYDOWN:
        case WM_MOUSEMOVE:
            UpdateTextSelected(lIsTextSelected((HWND)Handle.ToPointer()) ? true : false);
            break;
    }
}

void Common::Forms::TextBoxMessages::OnTextSelected(
  Common::Forms::TextBoxMessagesTextSelectedEventArgs^ e)
{
    TextSelected(this, e);
}

void Common::Forms::TextBoxMessages::UpdateTextSelected(
  System::Boolean bTextSelected)
{
    if (bTextSelected != m_bTextSelected)
    {
        m_bTextSelected = bTextSelected;

        OnTextSelected(gcnew Common::Forms::TextBoxMessagesTextSelectedEventArgs(bTextSelected));
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2025 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
