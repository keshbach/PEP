/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "TreeNodeAscendingSorter.h"
#include "TreeNodeDescendingSorter.h"

#include "TreeView.h"
#include "UtContextMenuStrip.h"

#define CEditWndProcPropName L"KE_EditWndProc"

#define TVM_LABELEDITCHAR (TV_FIRST - 1)

#define CDragDropScrollHeight 8

private struct _TREEITEM {}; // fix for linker warning LNK4248

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
    return (BOOL)::SendMessage(::GetParent(hWnd), TVM_LABELEDITCHAR, nKeyCode, 0);
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

Common::Forms::TreeView::TreeView() :
  m_SortOrder(Common::Forms::TreeView::ESortOrder::None)
{
	InitializeComponent();
}

Common::Forms::TreeView::~TreeView()
{
	if (components)
	{
		delete components;
	}
}

void Common::Forms::TreeView::Sort(void)
{
    System::Windows::Forms::TreeNode^ Node;

    if (m_SortOrder != Common::Forms::TreeView::ESortOrder::None)
    {
        Node = SelectedNode;

        dynamic_cast<System::Windows::Forms::TreeView^>(this)->Sort();

        SelectedNode = Node;

        if (Node != nullptr)
        {
            Node->EnsureVisible();
        }
    }
}

void Common::Forms::TreeView::ChangeLabelEditText(
  System::String^ sText)
{
    HWND hText = TreeView_GetEditControl((HWND)Handle.ToPointer());
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

void Common::Forms::TreeView::ChangeLabelEditSelection(
    System::Int32 nStart,
    System::Int32 nEnd)
{
    HWND hText = TreeView_GetEditControl((HWND)Handle.ToPointer());

    if (hText)
    {
        ::SendMessage(hText, EM_SETSEL, (WPARAM)nStart, (LPARAM)nEnd);
    }
    else
    {
        throw gcnew System::Exception(L"Label edit control has not been created.");
    }

}

void Common::Forms::TreeView::ScrollUp(void)
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

void Common::Forms::TreeView::ScrollDown(void)
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

Common::Forms::TreeView::EDragDropScrollHitTest Common::Forms::TreeView::DragDropScrollHitTest(
    System::Drawing::Point point)
{
    System::Drawing::Rectangle^ TopRect;
    System::Drawing::Rectangle^ BottomRect;

    TopRect = gcnew System::Drawing::Rectangle(0, 0, ClientSize.Width,
                                               CDragDropScrollHeight);
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

System::Windows::Forms::TreeNode^ Common::Forms::TreeView::FindTreeNode(
  System::Windows::Forms::TreeNodeCollection^ NodeColl,
  System::String^ sText,
  System::Boolean bIgnoreCase)
{
    for each (System::Windows::Forms::TreeNode^ TreeNode in NodeColl)
    {
        if (0 == System::String::Compare(TreeNode->Text, sText, bIgnoreCase))
        {
            return TreeNode;
        }
    }

    return nullptr;
}

System::Boolean Common::Forms::TreeView::IsTreeNodeChild(
  System::Windows::Forms::TreeNode^ RootNode,
  System::Windows::Forms::TreeNode^ Node)
{
    for each (System::Windows::Forms::TreeNode^ TreeNode in RootNode->Nodes)
    {
        if (TreeNode == Node || IsTreeNodeChild(TreeNode, Node))
        {
            return true;
        }
    }

    return false;
}

void Common::Forms::TreeView::WndProc(
  System::Windows::Forms::Message% Message)
{
    switch (Message.Msg)
    {
        case WM_RBUTTONDBLCLK: // Ignore message
            break;
        case WM_RBUTTONDOWN:
            HandleRButtonDown(Message);
            break;
        case WM_PARENTNOTIFY:
            HandleParentNotify(Message);
            break;
        case TVM_LABELEDITCHAR:
            HandleLabelEditChar(Message);
            break;
        default:
            System::Windows::Forms::TreeView::WndProc(Message);
            break;
    }
}

void Common::Forms::TreeView::HandleRButtonDown(
  System::Windows::Forms::Message% Message)
{
    HWND hWnd = (HWND)Message.HWnd.ToPointer();
    LPARAM lParam = (LPARAM)Message.LParam.ToPointer();
    TVHITTESTINFO HitTestInfo;

    HitTestInfo.pt.x = GET_X_LPARAM(lParam); 
    HitTestInfo.pt.y = GET_Y_LPARAM(lParam); 

    if (TreeView_HitTest(hWnd, &HitTestInfo))
    {
        TreeView_SelectItem(hWnd, HitTestInfo.hItem);

        System::Windows::Forms::TreeView::WndProc(Message);
    }
}

void Common::Forms::TreeView::HandleParentNotify(
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

    System::Windows::Forms::TreeView::WndProc(Message);
}

void Common::Forms::TreeView::HandleLabelEditChar(
  System::Windows::Forms::Message% Message)
{
    System::Windows::Forms::KeyPressEventArgs^ EventArgs;

    EventArgs = gcnew System::Windows::Forms::KeyPressEventArgs((WCHAR)(WPARAM)Message.WParam.ToPointer());

    OnKeyPressLabelEdit(EventArgs);

    Message.Result = EventArgs->Handled ? (System::IntPtr)TRUE : (System::IntPtr)FALSE;
}

void Common::Forms::TreeView::OnKeyPressLabelEdit(
  System::Windows::Forms::KeyPressEventArgs^ e)
{
    KeyPressLabelEdit(this, e);
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
