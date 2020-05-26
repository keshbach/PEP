/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2020-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "ICheckedListBoxItemChange.h"

#include "CheckedListBoxEnums.h"
#include "CheckStateChangeEventArgs.h"
#include "CheckedListBoxItem.h"
#include "ICheckedListBoxList.h"
#include "CheckedListBoxItemCollectionEditor.h"
#include "CheckedListBoxItemCollection.h"

#include "CheckedListBox.h"

#include <UtilsPep/UiPepCtrls.h>

#include <Includes/UtMacros.h>

#include <Includes/UiMacros.h>

#include <strsafe.h>

#pragma region "Constants"

#define CBorderWidth 1
#define CBorderHeight 1

#pragma endregion

#pragma region "Local Functions"

static DWORD lTranslateCheckState(
  Pep::Forms::ECheckState CheckState)
{
	switch (CheckState)
	{
    	case Pep::Forms::ECheckState::Unchecked:
	    	return CCheckedListBoxUncheckedState;
		case Pep::Forms::ECheckState::Checked:
			return CCheckedListBoxCheckedState;
		case Pep::Forms::ECheckState::Indeterminate:
			return CCheckedListBoxIndeterminateState;
	}

	throw gcnew System::Exception("Unknown check state");
}

static Pep::Forms::ECheckState lTranslateCheckState(
  _In_ DWORD dwCheckState)
{
	if (dwCheckState & CCheckedListBoxUncheckedState)
	{
		return Pep::Forms::ECheckState::Unchecked;
	}
	else if (dwCheckState & CCheckedListBoxCheckedState)
	{
		return Pep::Forms::ECheckState::Checked;
	}
	else if (dwCheckState & CCheckedListBoxIndeterminateState)
	{
		return Pep::Forms::ECheckState::Indeterminate;
	}

	throw gcnew System::Exception("Unknown check state");
}

#pragma endregion

Pep::Forms::CheckedListBox::CheckedListBox() :
  m_hCheckedListBoxCtrl(NULL)
{
    InitializeComponent();
}

Pep::Forms::CheckedListBox::CheckedListBox(
  System::ComponentModel::IContainer ^container) :
  m_hCheckedListBoxCtrl(NULL)
{
	container->Add(this);

	InitializeComponent();
}

Pep::Forms::CheckedListBox::~CheckedListBox()
{
	if (components)
	{
		delete components;
	}

	if (m_CheckedListBoxItemCollection)
	{
		m_CheckedListBoxItemCollection->Close();

		m_CheckedListBoxItemCollection = nullptr;
	}
}

void Pep::Forms::CheckedListBox::BeginUpdate(void)
{
    if (m_hCheckedListBoxCtrl)
    {
        ::SendMessage(m_hCheckedListBoxCtrl, WM_SETREDRAW, FALSE, 0);
    }
}

void Pep::Forms::CheckedListBox::EndUpdate(void)
{
    if (m_hCheckedListBoxCtrl)
    {
        ::SendMessage(m_hCheckedListBoxCtrl, WM_SETREDRAW, TRUE, 0);
    }
}

#pragma region Pep::Forms::ICheckedListBoxList

void Pep::Forms::CheckedListBox::Add(
  CheckedListBoxItem^ CheckedListBoxItem)
{
	pin_ptr<const wchar_t> pszName;

	if (m_hCheckedListBoxCtrl)
	{
		pszName = PtrToStringChars(CheckedListBoxItem->Name);

		if (::SendMessage(m_hCheckedListBoxCtrl, CLBM_ADDITEM, 0, (LPARAM)pszName))
		{
			::SendMessage(m_hCheckedListBoxCtrl, CLBM_SETCHECKSTATE, m_CheckedListBoxItemCollection->Count,
				          lTranslateCheckState(CheckedListBoxItem->CheckState));

			CheckedListBoxItem->CheckedListBoxItemChange = this;

			m_CheckedListBoxItemList->Add(CheckedListBoxItem);
		}
	}
	else
	{
		CheckedListBoxItem->CheckedListBoxItemChange = this;

		m_CheckedListBoxItemList->Add(CheckedListBoxItem);
	}
}

void Pep::Forms::CheckedListBox::Clear()
{
	if (m_hCheckedListBoxCtrl)
	{
		::SendMessage(m_hCheckedListBoxCtrl, CLBM_DELETEALLITEMS, 0, 0);
	}

	for each (CheckedListBoxItem^ CheckedListBoxItem in m_CheckedListBoxItemList)
	{
		CheckedListBoxItem->CheckedListBoxItemChange = nullptr;
	}

	m_CheckedListBoxItemList->Clear();
}

void Pep::Forms::CheckedListBox::CopyTo(
  array<CheckedListBoxItem^>^ CheckedListBoxItemArray,
  int nIndex)
{
	pin_ptr<const wchar_t> pszName;

	if (CheckedListBoxItemArray == nullptr)
	{
		throw gcnew System::ArgumentNullException();
	}

	if (nIndex < 0 || nIndex > m_CheckedListBoxItemList->Count)
	{
		throw gcnew System::ArgumentOutOfRangeException();
	}

	if (nIndex == m_CheckedListBoxItemList->Count)
	{
		for (int nArrayIndex = 0; nArrayIndex < m_CheckedListBoxItemList->Count; ++nArrayIndex)
		{
			Add(CheckedListBoxItemArray[nArrayIndex]);
		}
	}
	else
	{
		for (int nArrayIndex = 0; nArrayIndex < CheckedListBoxItemArray->Length; ++nArrayIndex)
		{
			if (m_hCheckedListBoxCtrl)
			{
				pszName = PtrToStringChars(CheckedListBoxItemArray[nArrayIndex]->Name);

				if (::SendMessage(m_hCheckedListBoxCtrl, CLBM_INSERTITEM, nIndex + nArrayIndex, (LPARAM)pszName))
				{
					::SendMessage(m_hCheckedListBoxCtrl, CLBM_SETCHECKSTATE, m_CheckedListBoxItemCollection->Count,
                                  lTranslateCheckState(CheckedListBoxItemArray[nArrayIndex]->CheckState));

					CheckedListBoxItemArray[nArrayIndex]->CheckedListBoxItemChange = this;

					m_CheckedListBoxItemList->Insert(nIndex + nArrayIndex, CheckedListBoxItemArray[nArrayIndex]);
				}
			}
			else
			{
				CheckedListBoxItemArray[nArrayIndex]->CheckedListBoxItemChange = this;

				m_CheckedListBoxItemList->Insert(nIndex + nArrayIndex, CheckedListBoxItemArray[nArrayIndex]);
			}
		}
	}
}

bool Pep::Forms::CheckedListBox::Remove(
  int nIndex)
{
	System::Collections::Generic::IEnumerator<CheckedListBoxItem^>^ Enumerator;

	if (nIndex < 0 || nIndex > m_CheckedListBoxItemList->Count)
	{
		return false;
	}

	if (m_hCheckedListBoxCtrl)
	{
		::SendMessage(m_hCheckedListBoxCtrl, CLBM_DELETEITEM, nIndex, 0);
	}

	Enumerator = m_CheckedListBoxItemList->GetEnumerator();

	for (int nTmpIndex = 0; nTmpIndex < nIndex; ++nTmpIndex)
	{
		Enumerator->MoveNext();
	}

	Enumerator->Current->CheckedListBoxItemChange = nullptr;

	m_CheckedListBoxItemList->RemoveAt(nIndex);

	return true;
}

#pragma endregion

#pragma region Pep::Forms::ICheckedListBoxItemChange

void Pep::Forms::CheckedListBox::OnNameChange(
  System::Object^ CheckedListBoxItem)
{
	pin_ptr<const wchar_t> pszName;

	if (m_hCheckedListBoxCtrl)
	{
		for (int nIndex = 0; nIndex < m_CheckedListBoxItemCollection->Count; ++nIndex)
		{
			if (m_CheckedListBoxItemCollection[nIndex] == CheckedListBoxItem)
			{
				pszName = PtrToStringChars(m_CheckedListBoxItemCollection[nIndex]->Name);

				::SendMessage(m_hCheckedListBoxCtrl, CLBM_SETCHECKSTATE, nIndex,
					          (LPARAM)pszName);

				return;
			}
		}
	}
}

void Pep::Forms::CheckedListBox::OnCheckStateChange(
  System::Object^ CheckedListBoxItem)
{
	if (m_hCheckedListBoxCtrl)
	{
    	for (int nIndex = 0; nIndex < m_CheckedListBoxItemCollection->Count; ++nIndex)
        {
		    if (m_CheckedListBoxItemCollection[nIndex] == CheckedListBoxItem)
		    {
		    	::SendMessage(m_hCheckedListBoxCtrl, CLBM_SETCHECKSTATE, nIndex,
				              lTranslateCheckState(m_CheckedListBoxItemCollection[nIndex]->CheckState));

			    return;
		    }
	    }
	}
}

#pragma endregion

void Pep::Forms::CheckedListBox::OnHandleCreated(
  System::EventArgs^ e)
{
	HWND hWnd = (HWND)Handle.ToPointer();

	System::Windows::Forms::Control::OnHandleCreated(e);

	m_hCheckedListBoxCtrl = ::CreateWindowExW(0, CUiCheckedListBoxCtrlClass, NULL,
                                              WS_CHILD | (this->Visible ? WS_VISIBLE : 0),
                                              0, 0, ClientRectangle.Width, ClientRectangle.Height,
		                                      hWnd, NULL, NULL, NULL);

	UpdateFont(false);

	UpdateItems();
}

void Pep::Forms::CheckedListBox::OnHandleDestroyed(
  System::EventArgs^ e)
{
	if (m_hCheckedListBoxCtrl)
	{
		::DestroyWindow(m_hCheckedListBoxCtrl);

		m_hCheckedListBoxCtrl = NULL;
	}

	System::Windows::Forms::Control::OnHandleDestroyed(e);
}

void Pep::Forms::CheckedListBox::OnEnabledChanged(
  System::EventArgs^ e)
{
	System::Windows::Forms::Control::OnEnabledChanged(e);

	if (m_hCheckedListBoxCtrl)
	{
		::EnableWindow(m_hCheckedListBoxCtrl, this->Enabled ? TRUE : FALSE);
	}
}

void Pep::Forms::CheckedListBox::OnVisibleChanged(
  System::EventArgs^ e)
{
	System::Windows::Forms::Control::OnVisibleChanged(e);

	if (m_hCheckedListBoxCtrl)
	{
		::ShowWindow(m_hCheckedListBoxCtrl, this->Visible ? SW_SHOW : SW_HIDE);
	}
}

void Pep::Forms::CheckedListBox::OnFontChanged(
  System::EventArgs^ e)
{
	System::Windows::Forms::Control::OnFontChanged(e);

	UpdateFont(true);
}

void Pep::Forms::CheckedListBox::OnGotFocus(
  System::EventArgs^ e)
{
	System::Windows::Forms::Control::OnGotFocus(e);

	if (m_hCheckedListBoxCtrl)
	{
		::SetFocus(m_hCheckedListBoxCtrl);
	}
}

void Pep::Forms::CheckedListBox::OnPaint(
  System::Windows::Forms::PaintEventArgs^ e)
{
	System::Windows::Forms::Control::OnPaint(e);

	e->Graphics->FillRectangle(System::Drawing::SystemBrushes::WindowFrame,
		                       0, 0, ClientSize.Width, ClientSize.Height);

	e->Graphics->FillRectangle(System::Drawing::SystemBrushes::Window,
                               CBorderWidth, CBorderHeight,
		                       ClientSize.Width - (CBorderWidth * 2),
		                       ClientSize.Height - (CBorderHeight * 2));

	e->Graphics->DrawString(Name, Font, System::Drawing::SystemBrushes::WindowText,
		                    CBorderWidth * 2, CBorderHeight * 2);
}

void Pep::Forms::CheckedListBox::OnResize(
  System::EventArgs^ e)
{
	System::Windows::Forms::Control::OnResize(e);

	if (m_hCheckedListBoxCtrl)
	{
		::MoveWindow(m_hCheckedListBoxCtrl, 0, 0, ClientRectangle.Width, ClientRectangle.Height, TRUE);
	}
}

void Pep::Forms::CheckedListBox::WndProc(
  System::Windows::Forms::Message% msg)
{
	LPNMHDR pHdr;
	TUiCheckedListBoxCtrlNMStateChange* pHdrStateChange;
	ECheckState CheckState;

	if (msg.Msg == WM_NOTIFY)
	{
		pHdr = (LPNMHDR)msg.LParam.ToPointer();

		if (pHdr->hwndFrom == m_hCheckedListBoxCtrl && pHdr->code == CLBNM_STATECHANGE)
		{
			pHdrStateChange = (TUiCheckedListBoxCtrlNMStateChange*)pHdr;

			CheckState = lTranslateCheckState(pHdrStateChange->dwNewState);

			m_CheckedListBoxItemCollection[pHdrStateChange->nIndex]->CheckState = CheckState;

			CheckStateChange(this, gcnew CheckStateChangedEventArgs(pHdrStateChange->nIndex, CheckState));
		}
	}

	System::Windows::Forms::Control::WndProc(msg);
}

bool Pep::Forms::CheckedListBox::PreProcessMessage(
  System::Windows::Forms::Message% msg)
{
	if ((msg.Msg == WM_KEYDOWN || msg.Msg == WM_KEYUP) && msg.WParam.ToInt32() != VK_TAB)
	{
		if (m_hCheckedListBoxCtrl)
		{
			::SendMessage((HWND)msg.HWnd.ToPointer(), msg.Msg,
                          (WPARAM)msg.WParam.ToPointer(),
                          (LPARAM)msg.LParam.ToPointer());
		}

		return true;
	}

	return System::Windows::Forms::Control::PreProcessMessage(msg);
}

System::Boolean Pep::Forms::CheckedListBox::UpdateItems()
{
	pin_ptr<const wchar_t> pszName;

	if (m_hCheckedListBoxCtrl)
	{
		if (FALSE == ::SendMessage(m_hCheckedListBoxCtrl, CLBM_DELETEALLITEMS, 0, 0))
		{
			return false;
		}

		for (int nIndex = 0; nIndex < m_CheckedListBoxItemCollection->Count; ++nIndex)
		{
			pszName = PtrToStringChars(m_CheckedListBoxItemCollection[nIndex]->Name);

			if (::SendMessage(m_hCheckedListBoxCtrl, CLBM_ADDITEM, 0, (LPARAM)pszName))
			{
				::SendMessage(m_hCheckedListBoxCtrl, CLBM_SETCHECKSTATE, nIndex,
					          lTranslateCheckState(m_CheckedListBoxItemCollection[nIndex]->CheckState));
			}
		}
	}

	return true;
}

void Pep::Forms::CheckedListBox::UpdateFont(
  System::Boolean bRedraw)
{
	pin_ptr<const wchar_t> pszName = PtrToStringChars(Font->Name);
	LOGFONTW LogFont;
	HFONT hFont;

	if (m_hCheckedListBoxCtrl)
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

		::SendMessage(m_hCheckedListBoxCtrl, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(bRedraw ? TRUE : FALSE, 0));
	}
}

System::Int32 Pep::Forms::CheckedListBox::GetMinWidth()
{
	INT nMinWidth;

	if (m_hCheckedListBoxCtrl)
	{
		if (::SendMessage(m_hCheckedListBoxCtrl, CLBM_GETMINWIDTH, 0, (LPARAM)&nMinWidth))
		{
			return nMinWidth + (CBorderWidth * 2);
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2020-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
