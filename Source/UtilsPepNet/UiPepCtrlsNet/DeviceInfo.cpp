/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2010-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "DeviceInfo.h"

#include <UtilsPep/UiPepCtrls.h>

#include <Includes/UtMacros.h>

#include <Includes/UiMacros.h>

#include <strsafe.h>

#pragma region "Constants"

#define CBorderWidth 1
#define CBorderHeight 1

#define CTotalDipSwitches 8

#pragma endregion

#pragma region "Local Functions"

static void lSetNameValue(
  HWND hWnd,
  System::String^ sValue,
  INT nMessage)
{
	pin_ptr<const wchar_t> pszName;

	if (false == System::String::IsNullOrWhiteSpace(sValue))
	{
		pszName = PtrToStringChars(sValue);

		if (FALSE == ::SendMessage(hWnd, nMessage, 0, (LPARAM)pszName))
		{
			throw gcnew System::Exception(L"Unable to change value");
		}
	}
	else
	{
		if (FALSE == ::SendMessage(hWnd, nMessage, 0, (LPARAM)NULL))
		{
			throw gcnew System::Exception(L"Unable to change value");
		}
	}
}

static System::String^ lGetNameValue(
  HWND hWnd,
  INT nMessage)
{
	INT nValueLen = 0;
	System::IntPtr pValue;
	System::String^ sValue;

	if (FALSE == ::SendMessage(hWnd, nMessage, (WPARAM)&nValueLen, NULL) ||
		nValueLen == 0)
	{
		return System::String::Empty;
	}

	try
	{
		pValue = System::Runtime::InteropServices::Marshal::AllocHGlobal(nValueLen * sizeof(WCHAR));
	}
	catch (System::OutOfMemoryException^)
	{
		return System::String::Empty;
	}

	if (TRUE == ::SendMessage(hWnd, nMessage, (WPARAM)&nValueLen, (LPARAM)(pValue.ToPointer())))
	{
		sValue = gcnew System::String((LPCWSTR)(pValue.ToPointer()));
	}
	else
	{
		sValue = System::String::Empty;
	}

	System::Runtime::InteropServices::Marshal::FreeHGlobal(pValue);

	return sValue;
}

static void lSetDipSwitchesValue(
  HWND hWnd,
  array<System::Boolean>^ DipSwitches)
{
	UINT8 nDipSwitches = 0;

	for (System::Int32 nIndex = 0; nIndex < DipSwitches->Length; ++nIndex)
	{
		if (DipSwitches[nIndex])
		{
			nDipSwitches |= (1 << nIndex);
		}
	}

	if (FALSE == ::SendMessage(hWnd, DIM_SETDIPSWITCHES, 0, (LPARAM)nDipSwitches))
	{
		throw gcnew System::Exception(L"Unable to set the dip switches");
	}
}

static array<System::Boolean>^ lGetDipSwitchesValue(
  HWND hWnd)
{
	UINT8 nDipSwitches = 0;
	array<System::Boolean>^ DipSwitches;

	if (FALSE == ::SendMessage(hWnd, DIM_GETDIPSWITCHES, 0, (LPARAM)&nDipSwitches))
	{
		throw gcnew System::Exception(L"Unable to get the dip switches");
	}

	DipSwitches = gcnew array<System::Boolean>(CTotalDipSwitches);

	for (System::Int32 nIndex = 0; nIndex < DipSwitches->Length; ++nIndex)
	{
		if (nDipSwitches & (1 << nIndex))
		{
			DipSwitches[nIndex] = true;
		}
		else
		{
			DipSwitches[nIndex] = false;
		}
	}

	return DipSwitches;
}

#pragma endregion

Pep::Forms::DeviceInfo::DeviceInfo() :
  m_hDeviceInfoCtrl(NULL)
{
    InitializeComponent();
}

Pep::Forms::DeviceInfo::DeviceInfo(
  System::ComponentModel::IContainer ^container) :
  m_hDeviceInfoCtrl(NULL)
{
	m_DeviceDipSwitches = gcnew array<System::Boolean>(CTotalDipSwitches);

	container->Add(this);

	InitializeComponent();
}

Pep::Forms::DeviceInfo::~DeviceInfo()
{
	if (components)
	{
		delete components;
	}
}

void Pep::Forms::DeviceInfo::BeginUpdate(void)
{
    if (m_hDeviceInfoCtrl)
    {
        ::SendMessage(m_hDeviceInfoCtrl, WM_SETREDRAW, FALSE, 0);
    }
}

void Pep::Forms::DeviceInfo::EndUpdate(void)
{
    if (m_hDeviceInfoCtrl)
    {
        ::SendMessage(m_hDeviceInfoCtrl, WM_SETREDRAW, TRUE, 0);
    }
}

void Pep::Forms::DeviceInfo::UpdateSystemColors(void)
{
	if (m_hDeviceInfoCtrl)
	{
		::SendMessage(m_hDeviceInfoCtrl, WM_SYSCOLORCHANGE, 0, 0);
	}
}

void Pep::Forms::DeviceInfo::OnHandleCreated(
  System::EventArgs^ e)
{
	HWND hWnd = (HWND)Handle.ToPointer();

	System::Windows::Forms::Control::OnHandleCreated(e);

	m_hDeviceInfoCtrl = ::CreateWindowExW(0, CUiDeviceInfoCtrlClass, NULL,
                                            WS_CHILD | (this->Visible ? WS_VISIBLE : 0),
		                                    0, 0, ClientRectangle.Width, ClientRectangle.Height,
		                                    hWnd, NULL, NULL, NULL);

	if (m_hDeviceInfoCtrl)
	{
		::SendMessage(m_hDeviceInfoCtrl, WM_SETREDRAW, FALSE, 0);

		lSetNameValue(m_hDeviceInfoCtrl, m_sDeviceName, DIM_SETNAME);
		lSetNameValue(m_hDeviceInfoCtrl, m_sDeviceAdapter, DIM_SETADAPTER);
		lSetDipSwitchesValue(m_hDeviceInfoCtrl, m_DeviceDipSwitches);
		lSetNameValue(m_hDeviceInfoCtrl, m_sDeviceSize, DIM_SETSIZE);
		lSetNameValue(m_hDeviceInfoCtrl, m_sDeviceVpp, DIM_SETVPP);
		lSetNameValue(m_hDeviceInfoCtrl, m_sDeviceBits, DIM_SETBITS);
		lSetNameValue(m_hDeviceInfoCtrl, m_sDeviceChipEnable, DIM_SETCHIPENABLE);
		lSetNameValue(m_hDeviceInfoCtrl, m_sDeviceOutputEnable, DIM_SETOUTPUTENABLE);

		::SendMessage(m_hDeviceInfoCtrl, WM_SETREDRAW, TRUE, 0);
	}

	UpdateFont(false);
}

void Pep::Forms::DeviceInfo::OnHandleDestroyed(
  System::EventArgs^ e)
{
	if (m_hDeviceInfoCtrl)
	{
		::DestroyWindow(m_hDeviceInfoCtrl);

		m_hDeviceInfoCtrl = NULL;
	}

	System::Windows::Forms::Control::OnHandleDestroyed(e);
}

void Pep::Forms::DeviceInfo::OnEnabledChanged(
  System::EventArgs^ e)
{
	System::Windows::Forms::Control::OnEnabledChanged(e);

	if (m_hDeviceInfoCtrl)
	{
		::EnableWindow(m_hDeviceInfoCtrl, this->Enabled ? TRUE : FALSE);
	}
}

void Pep::Forms::DeviceInfo::OnVisibleChanged(
  System::EventArgs^ e)
{
	System::Windows::Forms::Control::OnVisibleChanged(e);

	if (m_hDeviceInfoCtrl)
	{
		::ShowWindow(m_hDeviceInfoCtrl, this->Visible ? SW_SHOW : SW_HIDE);
	}
}

void Pep::Forms::DeviceInfo::OnFontChanged(
  System::EventArgs^ e)
{
	System::Windows::Forms::Control::OnFontChanged(e);

	UpdateFont(true);
}

void Pep::Forms::DeviceInfo::OnGotFocus(
  System::EventArgs^ e)
{
	System::Windows::Forms::Control::OnGotFocus(e);

	if (m_hDeviceInfoCtrl)
	{
		::SetFocus(m_hDeviceInfoCtrl);
	}
}

void Pep::Forms::DeviceInfo::OnPaint(
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

void Pep::Forms::DeviceInfo::OnResize(
  System::EventArgs^ e)
{
	System::Windows::Forms::Control::OnResize(e);

	if (m_hDeviceInfoCtrl)
	{
		::MoveWindow(m_hDeviceInfoCtrl, 0, 0, ClientRectangle.Width, ClientRectangle.Height, TRUE);
	}
}

void Pep::Forms::DeviceInfo::SetNameValue(
  System::String^ sName)
{
	if (this->DesignMode || m_hDeviceInfoCtrl == NULL)
	{
		m_sDeviceName = sName;

		return;
	}

	lSetNameValue(m_hDeviceInfoCtrl, sName, DIM_SETNAME);
}

System::String^ Pep::Forms::DeviceInfo::GetNameValue()
{
	if (this->DesignMode || m_hDeviceInfoCtrl == NULL)
	{
		return m_sDeviceName;
	}

	return lGetNameValue(m_hDeviceInfoCtrl, DIM_GETNAME);
}

void Pep::Forms::DeviceInfo::SetAdapterValue(
  System::String^ sAdapter)
{
	if (this->DesignMode || m_hDeviceInfoCtrl == NULL)
	{
		m_sDeviceAdapter = sAdapter;

		return;
	}

	lSetNameValue(m_hDeviceInfoCtrl, sAdapter, DIM_SETADAPTER);
}

System::String^ Pep::Forms::DeviceInfo::GetAdapterValue()
{
	if (this->DesignMode || m_hDeviceInfoCtrl == NULL)
	{
		return m_sDeviceAdapter;
	}

	return lGetNameValue(m_hDeviceInfoCtrl, DIM_GETADAPTER);
}

void Pep::Forms::DeviceInfo::SetDipSwitchesValue(
  array<System::Boolean>^ DipSwitches)
{
	UINT8 nDipSwitches = 0;

	if (DipSwitches == nullptr)
	{
		throw gcnew System::Exception(L"Null dip switches");
	}

	if (DipSwitches->Length != CTotalDipSwitches)
	{
		throw gcnew System::Exception(L"Invalid number of dip switches");
	}

	if (this->DesignMode || m_hDeviceInfoCtrl == NULL)
	{
		m_DeviceDipSwitches = DipSwitches;

		return;
	}

	lSetDipSwitchesValue(m_hDeviceInfoCtrl, DipSwitches);
}

array<System::Boolean>^ Pep::Forms::DeviceInfo::GetDipSwitchesValue()
{
	if (this->DesignMode || m_hDeviceInfoCtrl == NULL)
	{
		return m_DeviceDipSwitches;
	}

	return lGetDipSwitchesValue(m_hDeviceInfoCtrl);
}

void Pep::Forms::DeviceInfo::SetSizeValue(
  System::String^ sSize)
{
	if (this->DesignMode || m_hDeviceInfoCtrl == NULL)
	{
		m_sDeviceSize = sSize;

		return;
	}

	lSetNameValue(m_hDeviceInfoCtrl, sSize, DIM_SETSIZE);
}

System::String^ Pep::Forms::DeviceInfo::GetSizeValue()
{
	if (this->DesignMode || m_hDeviceInfoCtrl == NULL)
	{
		return m_sDeviceSize;
	}

	return lGetNameValue(m_hDeviceInfoCtrl, DIM_GETSIZE);
}

void Pep::Forms::DeviceInfo::SetVppValue(
  System::String^ sVpp)
{
	if (this->DesignMode || m_hDeviceInfoCtrl == NULL)
	{
		m_sDeviceVpp = sVpp;

		return;
	}

	lSetNameValue(m_hDeviceInfoCtrl, sVpp, DIM_SETVPP);
}

System::String^ Pep::Forms::DeviceInfo::GetVppValue()
{
	if (this->DesignMode || m_hDeviceInfoCtrl == NULL)
	{
		return m_sDeviceVpp;
	}

	return lGetNameValue(m_hDeviceInfoCtrl, DIM_GETVPP);
}

void Pep::Forms::DeviceInfo::SetBitsValue(
  System::String^ sBits)
{
	if (this->DesignMode || m_hDeviceInfoCtrl == NULL)
	{
		m_sDeviceBits = sBits;

		return;
	}

	lSetNameValue(m_hDeviceInfoCtrl, sBits, DIM_SETBITS);
}

System::String^ Pep::Forms::DeviceInfo::GetBitsValue()
{
	if (this->DesignMode || m_hDeviceInfoCtrl == NULL)
	{
		return m_sDeviceBits;
	}

	return lGetNameValue(m_hDeviceInfoCtrl, DIM_GETBITS);
}

void Pep::Forms::DeviceInfo::SetChipEnableValue(
  System::String^ sChipEnable)
{
	if (this->DesignMode || m_hDeviceInfoCtrl == NULL)
	{
		m_sDeviceChipEnable = sChipEnable;

		return;
	}

	lSetNameValue(m_hDeviceInfoCtrl, sChipEnable, DIM_SETCHIPENABLE);
}

System::String^ Pep::Forms::DeviceInfo::GetChipEnableValue()
{
	if (this->DesignMode || m_hDeviceInfoCtrl == NULL)
	{
		return m_sDeviceChipEnable;
	}

	return lGetNameValue(m_hDeviceInfoCtrl, DIM_GETCHIPENABLE);
}

void Pep::Forms::DeviceInfo::SetOutputEnableValue(
  System::String^ sOutputEnable)
{
	if (this->DesignMode || m_hDeviceInfoCtrl == NULL)
	{
		m_sDeviceOutputEnable = sOutputEnable;

		return;
	}

	lSetNameValue(m_hDeviceInfoCtrl, sOutputEnable, DIM_SETOUTPUTENABLE);
}

System::String^ Pep::Forms::DeviceInfo::GetOutputEnableValue()
{
	if (this->DesignMode || m_hDeviceInfoCtrl == NULL)
	{
		return m_sDeviceOutputEnable;
	}

	return lGetNameValue(m_hDeviceInfoCtrl, DIM_GETOUTPUTENABLE);
}

System::Drawing::Rectangle^ Pep::Forms::DeviceInfo::GetMinRectValue()
{
	RECT Rect;

	if (FALSE == ::SendMessage(m_hDeviceInfoCtrl, DIM_GETMINRECT, 0, (LPARAM)&Rect))
	{
		throw gcnew System::Exception(L"Unable to get the minimum rectangle");
	}

	return gcnew System::Drawing::Rectangle(Rect.left, Rect.top,
		                                    Rect.right - Rect.left,
		                                    Rect.bottom - Rect.top);
}

void Pep::Forms::DeviceInfo::UpdateFont(
  System::Boolean bRedraw)
{
	pin_ptr<const wchar_t> pszName = PtrToStringChars(Font->Name);
	LOGFONTW LogFont;
	HFONT hFont;

	if (m_hDeviceInfoCtrl)
	{
		LogFont.lfHeight = (LONG)MPtSizeToTwips(Font->SizeInPoints) * -1;
		LogFont.lfWidth = 0;
		LogFont.lfEscapement = 0;
		LogFont.lfOrientation = 0;
		LogFont.lfWeight = Font->Bold ? FW_BOLD : FW_NORMAL;
		LogFont.lfItalic = Font->Italic ? TRUE : FALSE;
		LogFont.lfUnderline = Font->Underline ? TRUE : FALSE;
		LogFont.lfStrikeOut = Font->Strikeout ? TRUE : FALSE;
		LogFont.lfCharSet = ANSI_CHARSET;
		LogFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		LogFont.lfQuality = PROOF_QUALITY;
		LogFont.lfPitchAndFamily = FF_DONTCARE;
		LogFont.lfOutPrecision = OUT_TT_PRECIS;

		::StringCchCopyW(LogFont.lfFaceName, MArrayLen(LogFont.lfFaceName), pszName);

		hFont = ::CreateFontIndirectW(&LogFont);

		::SendMessage(m_hDeviceInfoCtrl, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(bRedraw ? TRUE : FALSE, 0));
	}
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2010-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
