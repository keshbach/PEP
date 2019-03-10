/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ListViewItemSequentialSorter.h"
#include "ListViewItemGroupSorter.h"
#include "ListViewItemGroupSequentialSorter.h"

#include "ListViewComboBoxEditEventArgs.h"

#include "ComboBox.h"

#include "ListViewComboBox.h"

#include "ListView.h"
#include "UtContextMenuStrip.h"

#define CEditWndProcPropName L"KE_EditWndProc"

#define LVM_LABELEDITCHAR (LVM_FIRST - 1)

#define CDragDropScrollHeight 8

#pragma unmanaged

static LRESULT CALLBACK lUnmanagedEditWindowProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam);

static BOOL lUnmanagedIsEditControl(
  HWND hWnd)
{
    WCHAR cClassName[10];

    ::GetClassNameW(hWnd, cClassName, sizeof(cClassName) / sizeof(cClassName[0]));

    return (::lstrcmpiW(cClassName, L"edit") == 0) ? TRUE : FALSE;
}

static VOID lUnmanagedSubclassEditControl(
  HWND hWnd)
{
    WNDPROC pWndProc = (WNDPROC)::GetWindowLongPtr(hWnd, GWLP_WNDPROC);

    ::SetPropW(hWnd, CEditWndProcPropName, pWndProc);

    ::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)lUnmanagedEditWindowProc);
}

static VOID lUnmanagedUnsubclassEditControl(
  HWND hWnd)
{
    WNDPROC pPrevWndProc = (WNDPROC)::GetPropW(hWnd, CEditWndProcPropName);

    ::RemovePropW(hWnd, CEditWndProcPropName);

    ::SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)pPrevWndProc);
}

static BOOL lUnmanagedEditProcessCharMsg(
  HWND hWnd,
  INT nKeyCode)
{
    return (BOOL)::SendMessage(::GetParent(hWnd), LVM_LABELEDITCHAR, nKeyCode, 0);
}

static LRESULT CALLBACK lUnmanagedEditWindowProc(
  HWND hWnd,
  UINT nMsg,
  WPARAM wParam,
  LPARAM lParam)
{
    WNDPROC pPrevWndProc = (WNDPROC)::GetPropW(hWnd, CEditWndProcPropName);

    switch (nMsg)
    {
        case WM_CHAR:
            if (lUnmanagedEditProcessCharMsg(hWnd, (INT)wParam))
            {
                return 0;
            }
            break;
        case WM_DESTROY:
            lUnmanagedUnsubclassEditControl(hWnd);
            break;
    }

    return ::CallWindowProc(pPrevWndProc, hWnd, nMsg, wParam, lParam);
}

#pragma managed

Common::Forms::ListView::ListView() :
  m_SortOrder(Common::Forms::ListView::ESortOrder::None),
  m_LabelEditor(Common::Forms::ListView::ELabelEditor::Label),
  m_ComboBoxItems(nullptr),
  m_SortArrow(Common::Forms::ListView::ESortArrow::None),
  m_nRow(0),
  m_nColumn(0),
  m_bApplyListViewComboBoxChanges(true),
  m_bIgnoreEndListViewComboBoxEdit(false)
{
	InitializeComponent();

    m_ListViewComboBox = gcnew Common::Forms::ListViewComboBox();

	m_ListViewComboBox->Visible = false;
	m_ListViewComboBox->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
    m_ListViewComboBox->AutoCompleteMode = System::Windows::Forms::AutoCompleteMode::None;
	m_ListViewComboBox->DropDownHeight = 100;

    m_ListViewComboBox->ListViewComboBoxSetFocus += gcnew Common::Forms::ListViewComboBox::ListViewComboBoxSetFocusHandler(this, &Common::Forms::ListView::ListViewComboBox_SetFocus);
    m_ListViewComboBox->ListViewComboBoxKillFocus += gcnew Common::Forms::ListViewComboBox::ListViewComboBoxKillFocusHandler(this, &Common::Forms::ListView::ListViewComboBox_KillFocus);
    m_ListViewComboBox->ListViewComboBoxKeydown += gcnew Common::Forms::ListViewComboBox::ListViewComboBoxKeydownHandler(this, &Common::Forms::ListView::ListViewComboBox_Keydown);
}

Common::Forms::ListView::~ListView()
{
    if (components)
    {
        delete components;
    }

    if (m_ListViewComboBox != nullptr)
    {
        delete m_ListViewComboBox;

        m_ListViewComboBox = nullptr;
    }

    delete m_ComboBoxItems;

    m_ComboBoxItems = nullptr;
}

System::Windows::Forms::ListViewHitTestInfo^ Common::Forms::ListView::HitTest(
  System::Drawing::Point point)
{
    HWND hWnd = (HWND)Handle.ToPointer();
    HWND hHeader;
    POINT Point;
    RECT Rect;

    if (View != System::Windows::Forms::View::Details ||
        HeaderStyle == System::Windows::Forms::ColumnHeaderStyle::None)
    {
        return System::Windows::Forms::ListView::HitTest(point);
    }

    hHeader = ListView_GetHeader(hWnd);

    Point.x = point.X;
    Point.y = point.Y;

    ::ClientToScreen(hWnd, &Point);

    ::GetWindowRect(hHeader, &Rect);

    if (::PtInRect(&Rect, Point) == FALSE)
    {
        return System::Windows::Forms::ListView::HitTest(point);
    }

    return gcnew System::Windows::Forms::ListViewHitTestInfo(nullptr, nullptr,
                                                             System::Windows::Forms::ListViewHitTestLocations::None);
}

System::Windows::Forms::ListViewHitTestInfo^ Common::Forms::ListView::HitTest(
  System::Int32 x,
  System::Int32 y)
{
    System::Drawing::Point Point(x, y);

    return HitTest(Point);
}

void Common::Forms::ListView::AutosizeColumns(void)
{
    for (System::Int32 nColumn = 0; nColumn < Columns->Count; ++nColumn)
    {
        AutosizeColumn(nColumn);
    }
}

void Common::Forms::ListView::AutosizeColumn(
  System::Int32 nColumn)
{
    HWND hWnd = (HWND)Handle.ToPointer();
    System::Int32 nWidth = 0;
    System::Int32 nTmpWidth;
    System::Drawing::Size^ TmpSize;
    pin_ptr<const wchar_t> pszText;
    System::Windows::Forms::ListViewItem::ListViewSubItemCollection^ pListViewSubItems;

    if (nColumn < 0 || nColumn >= Columns->Count)
    {
        throw gcnew System::ArgumentOutOfRangeException();
    }

    if (HeaderStyle != System::Windows::Forms::ColumnHeaderStyle::None)
    {
        pszText = PtrToStringChars(Columns[nColumn]->Text);

        nWidth = ListView_GetStringWidth(hWnd, pszText) + 16;
    }

    for (System::Int32 nIndex = 0; nIndex < Items->Count; ++nIndex)
    {
        pListViewSubItems = Items[nIndex]->SubItems;

        if (nColumn < pListViewSubItems->Count)
        {
            TmpSize = Common::Forms::Utility::CalcStringSize(pListViewSubItems[nColumn]->Text,
                                                             pListViewSubItems[nColumn]->Font);

            nTmpWidth = TmpSize->Width;
        }
        else
        {
            nTmpWidth = 0;
        }

        nTmpWidth += 12;

        if (CheckBoxes == true)
        {
            nTmpWidth += 10;
        }

        if (Columns[nColumn]->DisplayIndex == 0)
        {
            if (Items[nIndex]->ImageKey != gcnew System::String(L"") ||
                Items[nIndex]->ImageIndex != -1)
            {
                nTmpWidth += Items[nIndex]->ImageList->ImageSize.Width;
            }
        }

        if (nTmpWidth > nWidth)
        {
            nWidth = nTmpWidth;
        }
    }

    Columns[nColumn]->Width = nWidth;
}

void Common::Forms::ListView::ChangeLabelEditText(
  System::String^ sText)
{
    HWND hText = ListView_GetEditControl((HWND)Handle.ToPointer());
    pin_ptr<const wchar_t> pszText = PtrToStringChars(sText);

    if (hText)
    {
        ::SetWindowTextW(hText, pszText);
    }
    else
    {
        throw gcnew System::Exception(L"Label edit control has not been created.");
    }
}

void Common::Forms::ListView::ChangeLabelEditSelection(
  System::Int32 nStart,
  System::Int32 nEnd)
{
    HWND hText = ListView_GetEditControl((HWND)Handle.ToPointer());

    if (hText)
    {
        ::SendMessage(hText, EM_SETSEL, (WPARAM)nStart, (LPARAM)nEnd);
    }
    else
    {
        throw gcnew System::Exception(L"Label edit control has not been created.");
    }

}

void Common::Forms::ListView::ScrollUp(void)
{
    HWND hWnd = (HWND)Handle.ToPointer();
    SCROLLINFO ScrollInfo;

    ScrollInfo.cbSize = sizeof(ScrollInfo);
    ScrollInfo.fMask = SIF_POS | SIF_RANGE;

    ::GetScrollInfo(hWnd, SB_VERT, &ScrollInfo);

    if (ScrollInfo.nPos > ScrollInfo.nMin)
    {
        ScrollInfo.fMask = SIF_POS;

        --ScrollInfo.nPos;

        ::SetScrollInfo(hWnd, SB_VERT, &ScrollInfo, TRUE);

        ::SendMessage(hWnd, WM_VSCROLL, MAKEWPARAM(TB_LINEUP, 0), 0);
    }
}

void Common::Forms::ListView::ScrollDown(void)
{
    HWND hWnd = (HWND)Handle.ToPointer();
    SCROLLINFO ScrollInfo;

    ScrollInfo.cbSize = sizeof(ScrollInfo);
    ScrollInfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;

    ::GetScrollInfo(hWnd, SB_VERT, &ScrollInfo);

    if (ScrollInfo.nPos + (INT)ScrollInfo.nPage <= ScrollInfo.nMax)
    {
        ScrollInfo.fMask = SIF_POS;

        ++ScrollInfo.nPos;

        ::SetScrollInfo(hWnd, SB_VERT, &ScrollInfo, TRUE);

        ::SendMessage(hWnd, WM_VSCROLL, MAKEWPARAM(TB_LINEDOWN, 0), 0);
    }
}

Common::Forms::ListView::EDragDropScrollHitTest Common::Forms::ListView::DragDropScrollHitTest(
  System::Drawing::Point point)
{
    HWND hWnd = (HWND)Handle.ToPointer();
    System::Drawing::Rectangle^ TopRect;
    System::Drawing::Rectangle^ BottomRect;
    HWND hHeader;
    RECT Rect;
    INT nHeaderHeight;

    if (HeaderStyle != System::Windows::Forms::ColumnHeaderStyle::None)
    {
        hHeader = ListView_GetHeader(hWnd);

        ::GetWindowRect(hHeader, &Rect);

        nHeaderHeight = Rect.bottom - Rect.top;
    }
    else
    {
        nHeaderHeight = 0;
    }

    TopRect = gcnew System::Drawing::Rectangle(0, nHeaderHeight,
                                               ClientSize.Width, CDragDropScrollHeight);
    BottomRect = gcnew System::Drawing::Rectangle(0, ClientSize.Height - CDragDropScrollHeight,
                                                  ClientSize.Width, CDragDropScrollHeight);

    if (TopRect->Contains(point))
    {
        return EDragDropScrollHitTest::UpArea;
    }

    if (BottomRect->Contains(point))
    {
        return EDragDropScrollHitTest::DownArea;
    }

    return EDragDropScrollHitTest::None;
}

void Common::Forms::ListView::WndProc(
  System::Windows::Forms::Message% Message)
{
    switch (Message.Msg)
    {
        case WM_CREATE:
            HandleCreate(Message);
            break;
        case WM_LBUTTONDBLCLK:
            HandleLButtonDblClk(Message);
            break;
        case WM_RBUTTONDBLCLK: // Ignore message
            break;
        case WM_LBUTTONDOWN:
            HandleLButtonDown(Message);
            break;
        case WM_RBUTTONDOWN:
            HandleRButtonDown(Message);
            break;
        case WM_NOTIFY:
            HandleNotify(Message);
            break;
        case WM_PARENTNOTIFY:
            HandleParentNotify(Message);
            break;
        case LVM_LABELEDITCHAR:
            HandleLabelEditChar(Message);
            break;
        default:
            System::Windows::Forms::ListView::WndProc(Message);
            break;
    }
}

void Common::Forms::ListView::HandleCreate(
    System::Windows::Forms::Message% Message)
{
    System::Windows::Forms::ListView::WndProc(Message);

    RefreshSortArrow();
}

void Common::Forms::ListView::HandleLButtonDblClk(
  System::Windows::Forms::Message% Message)
{
    HWND hWnd = (HWND)Message.HWnd.ToPointer();
    LPARAM lParam = (LPARAM)Message.LParam.ToPointer();
    LVHITTESTINFO HitTestInfo;

    HitTestInfo.pt.x = GET_X_LPARAM(lParam);
    HitTestInfo.pt.y = GET_Y_LPARAM(lParam);

	m_ListViewComboBox->Visible = false;

    if (m_LabelEditor == Common::Forms::ListView::ELabelEditor::ComboBox &&
        ListView_SubItemHitTest(hWnd, &HitTestInfo) != -1)
    {
        BeginListViewComboBoxEdit(HitTestInfo.iItem, HitTestInfo.iSubItem);
    }
	else
	{
	    System::Windows::Forms::ListView::WndProc(Message);
	}
}

void Common::Forms::ListView::HandleLButtonDown(
  System::Windows::Forms::Message% Message)
{
    HWND hWnd = (HWND)Message.HWnd.ToPointer();
    LPARAM lParam = (LPARAM)Message.LParam.ToPointer();
    LVHITTESTINFO HitTestInfo;
    HWND hEdit;

    hEdit = ListView_GetEditControl(hWnd);

    if (hEdit)
    {
        ::SendMessage(hEdit, WM_CANCELMODE, 0, 0);
    }
	else
	{
		EndListViewComboBoxEdit();
	}

    HitTestInfo.pt.x = GET_X_LPARAM(lParam);
    HitTestInfo.pt.y = GET_Y_LPARAM(lParam);

    ListView_HitTest(hWnd, &HitTestInfo);

    if (HitTestInfo.iItem != -1)
    {
        System::Windows::Forms::ListView::WndProc(Message);
    }
    else
    {
        if (::GetFocus() != hWnd)
        {
            ::SetFocus(hWnd);

            // Default select the first item if nothing has been selected yet.

            if (ListView_GetNextItem(hWnd, -1, LVNI_SELECTED) == -1)
            {
                ListView_SetItemState(hWnd, 0,
                                      LVIS_FOCUSED | LVIS_SELECTED,
                                      LVIS_FOCUSED | LVIS_SELECTED);
            }
        }
    }
}

void Common::Forms::ListView::HandleRButtonDown(
  System::Windows::Forms::Message% Message)
{
    HWND hWnd = (HWND)Message.HWnd.ToPointer();
    LPARAM lParam = (LPARAM)Message.LParam.ToPointer();
    LVHITTESTINFO HitTestInfo;

    HitTestInfo.pt.x = GET_X_LPARAM(lParam);
    HitTestInfo.pt.y = GET_Y_LPARAM(lParam);

    if (ListView_HitTest(hWnd, &HitTestInfo) != -1)
    {
        System::Windows::Forms::ListView::WndProc(Message);
    }
    else
    {
        if (::GetFocus() != hWnd)
        {
            ::SetFocus(hWnd);
        }
    }
}

void Common::Forms::ListView::HandleNotify(
  System::Windows::Forms::Message% Message)
{
    LPNMHDR pHeader = (LPNMHDR)Message.LParam.ToPointer();

    switch (pHeader->code)
    {
        case HDN_BEGINTRACK:
        case HDN_ENDDRAG:
        	if (m_ListViewComboBox->Visible == true)
            {
                Message.Result = (System::IntPtr)TRUE;
            }
            else
            {
                System::Windows::Forms::ListView::WndProc(Message);
            }
            break;
        default:
            System::Windows::Forms::ListView::WndProc(Message);
            break;
    }
}

void Common::Forms::ListView::HandleParentNotify(
  System::Windows::Forms::Message% Message)
{
    if (WM_CREATE == LOWORD(Message.WParam.ToPointer()))
    {
        if (lUnmanagedIsEditControl((HWND)Message.LParam.ToPointer()))
        {
            if (ContextMenuStrip != nullptr)
            {
                Common::Forms::UtContextMenuStrip::SaveShortcutKeys(ContextMenuStrip);
                Common::Forms::UtContextMenuStrip::ClearShortcutKeys();
            }

            lUnmanagedSubclassEditControl((HWND)Message.LParam.ToPointer());
        }
    }
    else if (WM_DESTROY == LOWORD(Message.WParam.ToPointer()))
    {
        if (lUnmanagedIsEditControl((HWND)Message.LParam.ToPointer()))
        {
            if (ContextMenuStrip != nullptr)
            {
                Common::Forms::UtContextMenuStrip::RestoreShortcutKeys();
            }
        }
    }

    System::Windows::Forms::ListView::WndProc(Message);
}

void Common::Forms::ListView::HandleLabelEditChar(
  System::Windows::Forms::Message% Message)
{
    System::Windows::Forms::KeyPressEventArgs^ EventArgs;

    EventArgs = gcnew System::Windows::Forms::KeyPressEventArgs((WCHAR)(WPARAM)Message.WParam.ToPointer());

    OnKeyPressLabelEdit(EventArgs);

    Message.Result = EventArgs->Handled ? (System::IntPtr)TRUE : (System::IntPtr)FALSE;
}

void Common::Forms::ListView::BeginListViewComboBoxEdit(
  System::Int32 nRow,
  System::Int32 nColumn)
{
    HWND hWnd = (HWND)Handle.ToPointer();
    RECT Rect, TmpRect;
	System::String^ ItemText;
    ListViewComboBoxEditEventArgs^ ListViewComboBoxEditEventArg;

    m_nRow = nRow + 1;
    m_nColumn = nColumn + 1;

	if (nColumn == 0)
	{
        ItemText = Items[nRow]->Text;
    }
    else
    {
        ItemText = Items[nRow]->SubItems[nColumn]->Text;
    }

    ListViewComboBoxEditEventArg = gcnew ListViewComboBoxEditEventArgs(m_nRow,
                                                                       m_nColumn,
                                                                       ItemText);

    OnBeforeComboBoxEdit(ListViewComboBoxEditEventArg);

    if (ListViewComboBoxEditEventArg->CancelEdit == false)
	{
        ListView_EnsureVisible(hWnd, nRow, FALSE);

        if (nColumn == 0)
        {
            ListView_GetItemRect(hWnd, nRow, &TmpRect, LVIR_BOUNDS);
            ListView_GetSubItemRect(hWnd, nRow, 1, LVIR_BOUNDS, &Rect);
            
            Rect.right = Rect.left - 1;
            Rect.left = TmpRect.left;
        }
        else
        {
            ListView_GetSubItemRect(hWnd, nRow, nColumn, LVIR_BOUNDS, &Rect);
        }

        // Add sub-items if they are not present to avoid an exception being
        // thrown about a sub-item not existing.

        while (nColumn >= Items[nRow]->SubItems->Count)
        {
            Items[nRow]->SubItems->Add(L"");
        }

        m_ListViewComboBox->Parent = this;

		m_ListViewComboBox->Location = System::Drawing::Point(Rect.left, Rect.top);
		m_ListViewComboBox->Size = System::Drawing::Size(Rect.right - Rect.left, 200);
        m_ListViewComboBox->SelectedItem = gcnew System::String(L"");

		m_ListViewComboBox->Items->Clear();

        for each (System::String^ sValue in m_ComboBoxItems)
        {
			m_ListViewComboBox->Items->Add(sValue);

			if (ItemText == sValue)
			{
				m_ListViewComboBox->SelectedItem = sValue;
			}
        }

        m_bApplyListViewComboBoxChanges = true;

        ::EnableScrollBar(hWnd, SB_BOTH, ESB_DISABLE_BOTH);

        m_ListViewComboBox->AutosizeDropDown();

		m_ListViewComboBox->Visible = true;

        m_ListViewComboBox->Focus();
    }
}

void Common::Forms::ListView::EndListViewComboBoxEdit(void)
{
    HWND hWnd = (HWND)Handle.ToPointer();
	System::String^ ComboBoxText;
    ListViewComboBoxEditEventArgs^ ListViewComboBoxEditEventArg;

	if (m_ListViewComboBox->Visible == true &&
        m_bIgnoreEndListViewComboBoxEdit == false)
	{
        m_bIgnoreEndListViewComboBoxEdit = true;

        if (m_bApplyListViewComboBoxChanges == false)
        {
		    ListViewComboBoxEditEventArg->CancelEdit = true;
        }

        ComboBoxText = (System::String^)m_ListViewComboBox->SelectedItem;

        ListViewComboBoxEditEventArg = gcnew ListViewComboBoxEditEventArgs(m_nRow,
                                                                           m_nColumn,
                                                                           ComboBoxText);

        OnAfterComboBoxEdit(ListViewComboBoxEditEventArg);

		m_ListViewComboBox->Visible = false;

        ::EnableScrollBar(hWnd, SB_BOTH, ESB_ENABLE_BOTH);

		if (ListViewComboBoxEditEventArg->CancelEdit == false)
		{
			if (ComboBoxText != nullptr)
			{
				if (m_nColumn == 1)
				{
					Items[m_nRow - 1]->Text = ComboBoxText;
				}
				else
				{
					Items[m_nRow - 1]->SubItems[m_nColumn - 1]->Text = ComboBoxText;
				}
			}
		}

        m_bIgnoreEndListViewComboBoxEdit = false;
	}
}

void Common::Forms::ListView::RefreshSortArrow(void)
{
    HWND hWnd = (HWND)Handle.ToPointer();
    HDITEM HeaderItem;
    HWND hHeaderCtrl;

    hHeaderCtrl = ListView_GetHeader(hWnd);

    if (hHeaderCtrl)
    {
        HeaderItem.mask = HDI_FORMAT;

        Header_GetItem(hHeaderCtrl, 0, &HeaderItem);

        HeaderItem.fmt &= ~HDF_SORTDOWN;
        HeaderItem.fmt &= ~HDF_SORTUP;

        switch (m_SortArrow)
        {
            case Common::Forms::ListView::ESortArrow::None:
                break;
            case Common::Forms::ListView::ESortArrow::Up:
                HeaderItem.fmt |= HDF_SORTUP;
                break;
            case Common::Forms::ListView::ESortArrow::Down:
                HeaderItem.fmt |= HDF_SORTDOWN;
                break;
            default:
                System::Diagnostics::Debug::Assert(false, L"Unknown sort arrow.");
                break;
        }

        Header_SetItem(hHeaderCtrl, 0, &HeaderItem);
    }
}

void Common::Forms::ListView::ListViewComboBox_SetFocus(
  System::Object^ sender,
  System::EventArgs^ e)
{
    sender;
    e;
}

void Common::Forms::ListView::ListViewComboBox_KillFocus(
  System::Object^ sender,
  System::EventArgs^ e)
{
    sender;
    e;

    EndListViewComboBoxEdit();
}

void Common::Forms::ListView::ListViewComboBox_Keydown(
  System::Object^ sender,
  Common::Forms::ListViewComboBox::ListViewComboBoxKeydownEventArgs^ e)
{
    sender;

    switch (e->Key)
    {
        case System::Windows::Forms::Keys::Enter:
            m_bApplyListViewComboBoxChanges = true;

            Focus();
            break;
        case System::Windows::Forms::Keys::Escape:
            m_bApplyListViewComboBoxChanges = false;

            Focus();
            break;
        case System::Windows::Forms::Keys::Tab:
            break;
    }
}

void Common::Forms::ListView::OnBeforeComboBoxEdit(
  ListViewComboBoxEditEventArgs^ e)
{
    BeforeComboBoxEdit(m_ListViewComboBox, e);
}

void Common::Forms::ListView::OnAfterComboBoxEdit(
  ListViewComboBoxEditEventArgs^ e)
{
    AfterComboBoxEdit(m_ListViewComboBox, e);
}

void Common::Forms::ListView::OnKeyPressLabelEdit(
  System::Windows::Forms::KeyPressEventArgs^ e)
{
    KeyPressLabelEdit(this, e);
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
