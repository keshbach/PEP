/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2010-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "BufferViewer.h"

#include <UtilsPep/UiPepCtrls.h>

#pragma unmanaged

static HWND lUnmanagedCreateBufferViewerCtrl(
  HWND hParentWnd,
  INT nWidth,
  INT nHeight)
{
    return ::CreateWindowExW(0, CUiBufferViewerCtrlClass, NULL,
                             WS_CHILD | WS_VISIBLE, 0, 0, nWidth, nHeight,
                             hParentWnd, NULL, NULL, NULL);
}

#pragma managed

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

System::Void Pep::Forms::BufferViewer::BufferViewer_Load(
  System::Object^ sender,
  System::EventArgs^ e)
{
    HWND hWnd = (HWND)Handle.ToPointer();

    sender;
    e;

    m_hBufferViewerCtrl = lUnmanagedCreateBufferViewerCtrl(hWnd,
                                                           ClientRectangle.Width,
                                                           ClientRectangle.Height);

    if (m_byBuffer != nullptr)
    {
        UpdateBuffer();
    }

    UpdateDataOrganization();

    ResizeRedraw = true;
}

System::Void Pep::Forms::BufferViewer::BufferViewer_Resize(
  System::Object^ sender,
  System::EventArgs^ e)
{
    sender;
    e;

    ::MoveWindow(m_hBufferViewerCtrl, 0, 0,
                 ClientRectangle.Width, ClientRectangle.Height, TRUE);
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
    INT nDataOrganization;

    switch (m_DataOrganization)
    {
        case EDataOrganization::Ascii:
            nDataOrganization = CBufferViewerAsciiData;
            break;
        case EDataOrganization::Byte:
            nDataOrganization = CBufferViewerByteData;
            break;
        case EDataOrganization::WordBigEndian:
            nDataOrganization = CBufferViewerWordBigEndianData;
            break;
        case EDataOrganization::WordLittleEndian:
            nDataOrganization = CBufferViewerWordLittleEndianData;
            break;
        default:
            System::Diagnostics::Debug::Assert(false, "Undefined data organization");
            return false;
    }

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
//  Copyright (C) 2010-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
