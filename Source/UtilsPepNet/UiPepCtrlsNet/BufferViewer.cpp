/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2010-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BufferViewer.h"

#include <UtilsPep/UiPepCtrls.h>

#pragma region "Constants"

#define CBorderWidth 1
#define CBorderHeight 1

#pragma endregion

#pragma region "Local Functions"

static INT lTranslateUpdateDataOrganization(
  Pep::Forms::BufferViewer::EDataOrganization DataOrganization)
{
    switch (DataOrganization)
    {
        case Pep::Forms::BufferViewer::EDataOrganization::Ascii:
            return CBufferViewerAsciiData;
        case Pep::Forms::BufferViewer::EDataOrganization::Byte:
            return CBufferViewerByteData;
        case Pep::Forms::BufferViewer::EDataOrganization::WordBigEndian:
            return CBufferViewerWordBigEndianData;
        case Pep::Forms::BufferViewer::EDataOrganization::WordLittleEndian:
            return CBufferViewerWordLittleEndianData;
	}

	System::Diagnostics::Debug::Assert(false, "Undefined data organization");

	return CBufferViewerByteData;
}

#pragma endregion

Pep::Forms::BufferViewer::BufferViewer() :
  m_DataOrganization(Pep::Forms::BufferViewer::EDataOrganization::Byte),
  m_byBuffer(nullptr),
  m_nFontPtSize(CBufferViewerDefPtSize),
  m_hBufferViewerCtrl(NULL)
{
    InitializeComponent();
}

Pep::Forms::BufferViewer::BufferViewer(
  System::ComponentModel::IContainer ^container) :
  m_DataOrganization(Pep::Forms::BufferViewer::EDataOrganization::Byte),
  m_byBuffer(nullptr),
  m_nFontPtSize(10),
  m_hBufferViewerCtrl(NULL)
{
	container->Add(this);

	InitializeComponent();
}

Pep::Forms::BufferViewer::~BufferViewer()
{
	if (components)
	{
		delete components;
	}

    m_byBuffer = nullptr;
}

void Pep::Forms::BufferViewer::BeginUpdate(void)
{
    if (m_hBufferViewerCtrl)
    {
        ::SendMessage(m_hBufferViewerCtrl, WM_SETREDRAW, FALSE, 0);
    }
}

void Pep::Forms::BufferViewer::EndUpdate(void)
{
    if (m_hBufferViewerCtrl)
    {
        ::SendMessage(m_hBufferViewerCtrl, WM_SETREDRAW, TRUE, 0);
    }
}

void Pep::Forms::BufferViewer::OnHandleCreated(
  System::EventArgs^ e)
{
	HWND hWnd = (HWND)Handle.ToPointer();

	System::Windows::Forms::Control::OnHandleCreated(e);

	m_hBufferViewerCtrl = ::CreateWindowExW(0, CUiBufferViewerCtrlClass, NULL,
                                            WS_CHILD | WS_BORDER | (this->Visible ? WS_VISIBLE : 0),
		                                    0, 0, ClientRectangle.Width, ClientRectangle.Height,
		                                    hWnd, NULL, NULL, NULL);

	if (m_byBuffer != nullptr)
	{
		UpdateBuffer();
	}

	UpdateDataOrganization();

	this->ResizeRedraw = true;
}

void Pep::Forms::BufferViewer::OnHandleDestroyed(
  System::EventArgs^ e)
{
	if (m_hBufferViewerCtrl)
	{
		::DestroyWindow(m_hBufferViewerCtrl);

		m_hBufferViewerCtrl = NULL;
	}

	System::Windows::Forms::Control::OnHandleDestroyed(e);
}

void Pep::Forms::BufferViewer::OnEnabledChanged(
  System::EventArgs^ e)
{
	System::Windows::Forms::Control::OnEnabledChanged(e);

	if (m_hBufferViewerCtrl)
	{
		::EnableWindow(m_hBufferViewerCtrl, this->Enabled ? TRUE : FALSE);
	}
}

void Pep::Forms::BufferViewer::OnVisibleChanged(
  System::EventArgs^ e)
{
	System::Windows::Forms::Control::OnVisibleChanged(e);

	if (m_hBufferViewerCtrl)
	{
		::ShowWindow(m_hBufferViewerCtrl, this->Visible ? SW_SHOW : SW_HIDE);
	}
}

void Pep::Forms::BufferViewer::OnGotFocus(
  System::EventArgs^ e)
{
	System::Windows::Forms::Control::OnGotFocus(e);

	if (m_hBufferViewerCtrl)
	{
		::SetFocus(m_hBufferViewerCtrl);
	}
}

void Pep::Forms::BufferViewer::OnPaint(
  System::Windows::Forms::PaintEventArgs^ e)
{
	System::Windows::Forms::Control::OnPaint(e);

	if (this->DesignMode)
	{
		e->Graphics->FillRectangle(System::Drawing::SystemBrushes::WindowFrame,
                                   0, 0, ClientSize.Width, ClientSize.Height);

		e->Graphics->FillRectangle(System::Drawing::SystemBrushes::Window,
                                   CBorderWidth, CBorderHeight,
                                   ClientSize.Width - (CBorderWidth * 2),
                                   ClientSize.Height - (CBorderHeight * 2));

		e->Graphics->DrawString(Name, Font, System::Drawing::SystemBrushes::WindowText,
                                CBorderWidth * 2, CBorderHeight * 2);
	}
}

void Pep::Forms::BufferViewer::OnResize(
  System::EventArgs^ e)
{
	System::Windows::Forms::Control::OnResize(e);

	if (m_hBufferViewerCtrl)
	{
		::MoveWindow(m_hBufferViewerCtrl, 0, 0, ClientRectangle.Width, ClientRectangle.Height, TRUE);
	}
}

System::Boolean Pep::Forms::BufferViewer::UpdateBuffer(void)
{
    System::Boolean bResult = true;
    pin_ptr<System::Byte> pbyData = &m_byBuffer[0];
    TUiBufferViewerMem BufferViewerMem;

    BufferViewerMem.pbyBuffer = pbyData;
    BufferViewerMem.nBufferLen = m_byBuffer->Length;

    if (m_hBufferViewerCtrl)
    {
        bResult = ::SendMessage(m_hBufferViewerCtrl, BVM_SETBUFFER, 0,
                                (LPARAM)&BufferViewerMem) ? true : false;

    }

    return bResult;
}

System::Boolean Pep::Forms::BufferViewer::UpdateDataOrganization(void)
{
    System::Boolean bResult = true;
	INT nDataOrganization = lTranslateUpdateDataOrganization(m_DataOrganization);

    if (m_hBufferViewerCtrl)
    {
        bResult = ::SendMessage(m_hBufferViewerCtrl, BVM_SETDATAORGANIZATION, 0,
                                (LPARAM)nDataOrganization) ? true : false;
    }

    return bResult;
}

System::Boolean Pep::Forms::BufferViewer::UpdateFontPtSize(void)
{
    System::Boolean bResult = true;

    if (m_hBufferViewerCtrl)
    {
        bResult = ::SendMessage(m_hBufferViewerCtrl, BVM_SETFONTPTSIZE,
                                0, m_nFontPtSize) ? true : false;
    }

    return bResult;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2010-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
