/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ITextBoxClipboard.h"
#include "ContextMenuStrip.h"
#include "EditContextMenuStrip.h"
#include "TextBox.h" 

Common::Forms::TextBox::TextBox()
{
	InitializeComponent();
}

Common::Forms::TextBox::~TextBox()
{
	if (components)
	{
		delete components;
	}
}

void Common::Forms::TextBox::WndProc(
  System::Windows::Forms::Message% Message)
{
    switch (Message.Msg)
    {
        case WM_CREATE:
            m_EditContextMenuStrip = gcnew Common::Forms::EditContextMenuStrip(this);
            break;
        case WM_DESTROY:
            delete m_EditContextMenuStrip;

            m_EditContextMenuStrip = nullptr;
            break;
    }

    System::Windows::Forms::TextBox::WndProc(Message);
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
