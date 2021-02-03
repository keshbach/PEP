/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ContextMenuStrip.h"
#include "EditContextMenuStrip.h"
#include "MaskedTextBox.h" 

#define CContextMenuFullGroupName "Full"
#define CContextMenuReadOnlyGroupName "ReadOnly"

Common::Forms::MaskedTextBox::MaskedTextBox()
{
	InitializeComponent();
}

Common::Forms::MaskedTextBox::~MaskedTextBox()
{
	if (components)
	{
		delete components;
	}
}

void Common::Forms::MaskedTextBox::WndProc(
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

    System::Windows::Forms::MaskedTextBox::WndProc(Message);
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
