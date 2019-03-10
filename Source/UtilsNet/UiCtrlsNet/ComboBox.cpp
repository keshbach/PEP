/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ComboBox.h"

Common::Forms::ComboBox::ComboBox() :
  m_hTooltip(NULL)
{
	InitializeComponent();
}

Common::Forms::ComboBox::~ComboBox()
{
	if (components)
	{
		delete components;
	}
}

void Common::Forms::ComboBox::AutosizeDropDown(void)
{
    System::Drawing::Size MaxSize(0, 0);
    System::Drawing::Size^ TmpSize;

    for each (System::String^ sText in Items)
    {
        TmpSize = Common::Forms::Utility::CalcStringSize(sText, Font);

        if (TmpSize->Width > MaxSize.Width)
        {
            MaxSize.Width = TmpSize->Width;
        }
    }

    if (Items->Count > MaxDropDownItems)
    {
        MaxSize.Width += ::GetSystemMetrics(SM_CXVSCROLL);
        MaxSize.Width += (::GetSystemMetrics(SM_CXBORDER) * 2);
    }

    DropDownWidth = MaxSize.Width;
}

void Common::Forms::ComboBox::WndProc(
  System::Windows::Forms::Message% Message)
{
    System::Windows::Forms::ComboBox::WndProc(Message);
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
