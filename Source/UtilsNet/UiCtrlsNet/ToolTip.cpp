/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2010-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ToolTip.h"

Common::Forms::ToolTip::ToolTip() :
  m_hToolTip(NULL)
{
	InitializeComponent();
}

Common::Forms::ToolTip::ToolTip(
  System::ComponentModel::IContainer^ container) :
  m_hToolTip(NULL)
{
	container->Add(this);

	InitializeComponent();
}

Common::Forms::ToolTip::~ToolTip()
{
    System::Diagnostics::Debug::Assert(m_hToolTip == NULL);

	if (components)
	{
		delete components;
	}
}

System::Boolean Common::Forms::ToolTip::CreateToolTip(
  System::Windows::Forms::Form^ ParentForm)
{
    HWND hParentWnd = (HWND)ParentForm->Handle.ToPointer();

    if (m_hToolTip != NULL)
    {
        return false;
    }

    m_hToolTip = ::CreateWindowExW(NULL, TOOLTIPS_CLASS, NULL,
                                   WS_POPUP | TTS_ALWAYSTIP | TTS_NOPREFIX,
                                   CW_USEDEFAULT, CW_USEDEFAULT,
                                   CW_USEDEFAULT, CW_USEDEFAULT,
                                   hParentWnd, NULL, NULL, NULL);

    ::SetWindowPos(m_hToolTip, HWND_TOPMOST, 0, 0, 0, 0,
                   SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    return true;
}

System::Boolean Common::Forms::ToolTip::DestroyToolTip(void)
{
    if (m_hToolTip == NULL)
    {
        return false;
    }

    ::DestroyWindow(m_hToolTip);

    m_hToolTip = NULL;

    return true;
}

void Common::Forms::ToolTip::InitializeComponent(void)
{
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2010-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
