/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ContextMenuStrip.h"
#include "EditContextMenuStrip.h" 

#include "Includes/UtTemplates.h"

#pragma region "Constants"

#define CContextMenuFullGroupName "Full"
#define CContextMenuReadOnlyGroupName "ReadOnly"

#pragma endregion

#pragma region "Local Functions"

static BOOL lIsKeyDown(
  INT nKeyCode)
{
    return (::GetKeyState(nKeyCode) & 0x8000) ? TRUE : FALSE;
}

static BOOL lIsContextMenuKeyDown(
  INT nKeyCode)
{
    if ((nKeyCode == VK_F10 && lIsKeyDown(VK_SHIFT)) ||
        nKeyCode == VK_APPS)
    {
        return TRUE;
    }

    return FALSE;
}

static BOOL lKeyCodeMatchesToolStripMenuItem(
  INT nKeyCode,
  System::Windows::Forms::ToolStripMenuItem^ ToolStripMenuItem)
{
    BOOL bShiftPressed, bControlPressed, bAltPressed;

    if (nKeyCode != (((INT)ToolStripMenuItem->ShortcutKeys) & 0xFF))
    {
        return FALSE;
    }

    bShiftPressed = lIsKeyDown(VK_SHIFT);
    bControlPressed = lIsKeyDown(VK_CONTROL);
    bAltPressed = lIsKeyDown(VK_MENU);

    if (((INT)ToolStripMenuItem->ShortcutKeys & (INT)System::Windows::Forms::Keys::Shift) != 0)
    {
        if (!bShiftPressed)
        {
            return FALSE;
        }
    }
    else
    {
        if (bShiftPressed)
        {
            return FALSE;
        }
    }

    if (((INT)ToolStripMenuItem->ShortcutKeys & (INT)System::Windows::Forms::Keys::Control) != 0)
    {
        if (!bControlPressed)
        {
            return FALSE;
        }
    }
    else
    {
        if (bControlPressed)
        {
            return FALSE;
        }
    }

    if (((INT)ToolStripMenuItem->ShortcutKeys & (INT)System::Windows::Forms::Keys::Alt) != 0)
    {
        if (!bAltPressed)
        {
            return FALSE;
        }
    }
    else
    {
        if (bAltPressed)
        {
            return FALSE;
        }
    }

    return TRUE;
}

static VOID lGenerateContextMenuMsg(
  HWND hEdit)
{
    RECT Rect;
    INT nXPos, nYPos;

    ::GetWindowRect(hEdit, &Rect);

    nXPos = Rect.left + ((Rect.right - Rect.left) / 2);
    nYPos = Rect.top + ((Rect.bottom - Rect.top) / 2);

    ::PostMessage(hEdit, WM_CONTEXTMENU, 0, MAKELPARAM(nXPos, nYPos));
}

#pragma endregion

#pragma region "Constructors"

Common::Forms::EditContextMenuStrip::EditContextMenuStrip()
{
    m_TextBox = nullptr;
    m_hEdit = NULL;
}

Common::Forms::EditContextMenuStrip::EditContextMenuStrip(
  System::Windows::Forms::TextBox^ TextBox) :
  m_TextBox(TextBox),
  m_hEdit(NULL)
{
    CreateContextMenu();
    CreateToolStripGroups();
}

Common::Forms::EditContextMenuStrip::EditContextMenuStrip(
  HWND hEdit) :
  m_TextBox(nullptr),
  m_hEdit(hEdit)
{
    CreateContextMenu();
    CreateToolStripGroups();
}

#pragma endregion

#pragma region "Destructor"

Common::Forms::EditContextMenuStrip::~EditContextMenuStrip()
{
    DestroyToolStripGroups();
    DestroyContextMenu();

    m_TextBox = nullptr;
}

#pragma endregion

void Common::Forms::EditContextMenuStrip::DisplayContextMenuStrip(
  LPPOINT pPoint)
{
    m_ContextMenuStrip->Show(pPoint->x, pPoint->y);
}

BOOL Common::Forms::EditContextMenuStrip::ProcessKeyDown(
  INT nKeyCode)
{
    return lIsContextMenuKeyDown(nKeyCode);
}

BOOL Common::Forms::EditContextMenuStrip::ProcessKeyUp(
  INT nKeyCode)
{
    System::Windows::Forms::ToolStripMenuItem^ ToolStripMenuItem;

    if (lIsContextMenuKeyDown(nKeyCode))
    {
        lGenerateContextMenuMsg(m_hEdit);

        return TRUE;
    }

    for each (System::Windows::Forms::ToolStripItem ^ ToolStripItem in m_ContextMenuStrip->Items)
    {
        if (IsInstance<System::Windows::Forms::ToolStripMenuItem^>(ToolStripItem))
        {
            ToolStripMenuItem = (System::Windows::Forms::ToolStripMenuItem^)ToolStripItem;

            if (lKeyCodeMatchesToolStripMenuItem(nKeyCode, ToolStripMenuItem))
            {
                ToolStripMenuItem->PerformClick();
                
                return TRUE;
            }
        }
    }

    return FALSE;
}

BOOL Common::Forms::EditContextMenuStrip::ProcessChar(
  INT nKeyCode)
{
    switch (nKeyCode)
    {
        case VK_ESCAPE:
        case VK_BACK:
        case VK_TAB:
        case VK_SPACE:
        case VK_DELETE:
            return FALSE;
    }

    if (nKeyCode >= TEXT('0'))
    {
        return FALSE;
    }

    return TRUE;
}

void Common::Forms::EditContextMenuStrip::CreateContextMenu()
{
    m_ContextMenuStrip = gcnew Common::Forms::ContextMenuStrip();

    m_ContextMenuStrip->ImageList = Common::Forms::ImageManager::ToolbarSmallImageList;

    m_ContextMenuStrip->Opening += gcnew System::ComponentModel::CancelEventHandler(this, &Common::Forms::EditContextMenuStrip::contextMenuStrip_Opening);

    m_ContextMenuStrip->SuspendLayout();

    // Undo

    m_ToolStripMenuItemUndo = gcnew System::Windows::Forms::ToolStripMenuItem();

    m_ToolStripMenuItemUndo->Name = "toolStripMenuItemUndo";
    m_ToolStripMenuItemUndo->ShortcutKeys = System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::Z;
    m_ToolStripMenuItemUndo->Size = System::Drawing::Size(136, 22);
    m_ToolStripMenuItemUndo->Text = "&Undo";
    m_ToolStripMenuItemUndo->ImageKey = Common::Forms::ToolbarImageKey::Undo;
    m_ToolStripMenuItemUndo->Click += gcnew System::EventHandler(this, &Common::Forms::EditContextMenuStrip::toolStripMenuItemUndo_Click);

    m_ContextMenuStrip->Items->Add(m_ToolStripMenuItemUndo);

    // Seperator1

    m_ToolStripSeparator1 = gcnew System::Windows::Forms::ToolStripSeparator();

    m_ContextMenuStrip->Items->Add(m_ToolStripSeparator1);

    // Cut

    m_ToolStripMenuItemCut = gcnew System::Windows::Forms::ToolStripMenuItem();

    m_ToolStripMenuItemCut->Name = "toolStripMenuItemCut";
    m_ToolStripMenuItemCut->ShortcutKeys = System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::X;
    m_ToolStripMenuItemCut->Size = System::Drawing::Size(136, 22);
    m_ToolStripMenuItemCut->Text = "Cu&t";
    m_ToolStripMenuItemCut->ImageKey = Common::Forms::ToolbarImageKey::Cut;
    m_ToolStripMenuItemCut->Click += gcnew System::EventHandler(this, &Common::Forms::EditContextMenuStrip::toolStripMenuItemCut_Click);

    m_ContextMenuStrip->Items->Add(m_ToolStripMenuItemCut);

    // Copy

    m_ToolStripMenuItemCopy = gcnew System::Windows::Forms::ToolStripMenuItem();

    m_ToolStripMenuItemCopy->Name = "toolStripMenuItemCopy";
    m_ToolStripMenuItemCopy->ShortcutKeys = System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::C;
    m_ToolStripMenuItemCopy->Size = System::Drawing::Size(136, 22);
    m_ToolStripMenuItemCopy->Text = "&Copy";
    m_ToolStripMenuItemCopy->ImageKey = Common::Forms::ToolbarImageKey::Copy;
    m_ToolStripMenuItemCopy->Click += gcnew System::EventHandler(this, &Common::Forms::EditContextMenuStrip::toolStripMenuItemCopy_Click);

    m_ContextMenuStrip->Items->Add(m_ToolStripMenuItemCopy);

    // Paste

    m_ToolStripMenuItemPaste = gcnew System::Windows::Forms::ToolStripMenuItem();

    m_ToolStripMenuItemPaste->Name = "toolStripMenuItemPaste";
    m_ToolStripMenuItemPaste->ShortcutKeys = System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::V;
    m_ToolStripMenuItemPaste->Size = System::Drawing::Size(136, 22);
    m_ToolStripMenuItemPaste->Text = "&Paste";
    m_ToolStripMenuItemPaste->ImageKey = Common::Forms::ToolbarImageKey::Paste;
    m_ToolStripMenuItemPaste->Click += gcnew System::EventHandler(this, &Common::Forms::EditContextMenuStrip::toolStripMenuItemPaste_Click);

    m_ContextMenuStrip->Items->Add(m_ToolStripMenuItemPaste);

    // Delete

    m_ToolStripMenuItemDelete = gcnew System::Windows::Forms::ToolStripMenuItem();

    m_ToolStripMenuItemDelete->Name = "toolStripMenuItemDelete";
    m_ToolStripMenuItemDelete->ShortcutKeys = System::Windows::Forms::Keys::None;
    m_ToolStripMenuItemDelete->Size = System::Drawing::Size(136, 22);
    m_ToolStripMenuItemDelete->Text = "&Delete";
    m_ToolStripMenuItemDelete->ImageKey = Common::Forms::ToolbarImageKey::Delete;
    m_ToolStripMenuItemDelete->Click += gcnew System::EventHandler(this, &Common::Forms::EditContextMenuStrip::toolStripMenuItemDelete_Click);

    m_ContextMenuStrip->Items->Add(m_ToolStripMenuItemDelete);

    // Seperator2

    m_ToolStripSeparator2 = gcnew System::Windows::Forms::ToolStripSeparator();

    m_ContextMenuStrip->Items->Add(m_ToolStripSeparator2);

    // Select all

    m_ToolStripMenuItemSelectAll = gcnew System::Windows::Forms::ToolStripMenuItem();

    m_ToolStripMenuItemSelectAll->Name = "toolStripMenuItemSelectAll";
    m_ToolStripMenuItemSelectAll->ShortcutKeys = System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::A;
    m_ToolStripMenuItemSelectAll->Size = System::Drawing::Size(136, 22);
    m_ToolStripMenuItemSelectAll->Text = "Select &All";
    m_ToolStripMenuItemSelectAll->ImageKey = Common::Forms::ToolbarImageKey::Select;
    m_ToolStripMenuItemSelectAll->Click += gcnew System::EventHandler(this, &Common::Forms::EditContextMenuStrip::toolStripMenuItemSelectAll_Click);

    m_ContextMenuStrip->Items->Add(m_ToolStripMenuItemSelectAll);

    m_ContextMenuStrip->ResumeLayout();

    if (m_TextBox != nullptr)
    {
        m_TextBox->ContextMenuStrip = m_ContextMenuStrip;
    }
}

void Common::Forms::EditContextMenuStrip::DestroyContextMenu()
{
    if (m_TextBox != nullptr)
    {
        m_TextBox->ContextMenuStrip = nullptr;
    }

    m_ContextMenuStrip->Items->Clear();

    m_ContextMenuStrip->Opening -= gcnew System::ComponentModel::CancelEventHandler(this, &Common::Forms::EditContextMenuStrip::contextMenuStrip_Opening);

    m_ToolStripMenuItemUndo->Click -= gcnew System::EventHandler(this, &Common::Forms::EditContextMenuStrip::toolStripMenuItemUndo_Click);
    m_ToolStripMenuItemCut->Click -= gcnew System::EventHandler(this, &Common::Forms::EditContextMenuStrip::toolStripMenuItemCut_Click);
    m_ToolStripMenuItemCopy->Click -= gcnew System::EventHandler(this, &Common::Forms::EditContextMenuStrip::toolStripMenuItemCopy_Click);
    m_ToolStripMenuItemPaste->Click -= gcnew System::EventHandler(this, &Common::Forms::EditContextMenuStrip::toolStripMenuItemPaste_Click);
    m_ToolStripMenuItemDelete->Click -= gcnew System::EventHandler(this, &Common::Forms::EditContextMenuStrip::toolStripMenuItemDelete_Click);
    m_ToolStripMenuItemSelectAll->Click -= gcnew System::EventHandler(this, &Common::Forms::EditContextMenuStrip::toolStripMenuItemSelectAll_Click);

    m_ToolStripMenuItemUndo = nullptr;
    m_ToolStripSeparator1 = nullptr;
    m_ToolStripMenuItemCut = nullptr;
    m_ToolStripMenuItemCopy = nullptr;
    m_ToolStripMenuItemPaste = nullptr;
    m_ToolStripMenuItemDelete = nullptr;
    m_ToolStripSeparator2 = nullptr;
    m_ToolStripMenuItemSelectAll = nullptr;

    m_ContextMenuStrip->ImageList = nullptr;

    m_ContextMenuStrip = nullptr;
}

void Common::Forms::EditContextMenuStrip::CreateToolStripGroups()
{
    array<System::Windows::Forms::ToolStripItem^>^ FullGroupItems = {
        m_ToolStripMenuItemUndo,
        m_ToolStripSeparator1,
        m_ToolStripMenuItemCut,
        m_ToolStripMenuItemCopy,
        m_ToolStripMenuItemPaste,
        m_ToolStripMenuItemDelete,
        m_ToolStripSeparator2,
        m_ToolStripMenuItemSelectAll};
    array<System::Windows::Forms::ToolStripItem^>^ ReadOnlyGroupItems = {
        m_ToolStripMenuItemCopy,
        m_ToolStripSeparator2,
        m_ToolStripMenuItemSelectAll };

    m_ContextMenuStrip->CreateGroup(CContextMenuFullGroupName, FullGroupItems);
    m_ContextMenuStrip->CreateGroup(CContextMenuReadOnlyGroupName, ReadOnlyGroupItems);
}

void Common::Forms::EditContextMenuStrip::DestroyToolStripGroups()
{
    m_ContextMenuStrip->DestroyGroup(CContextMenuFullGroupName);
    m_ContextMenuStrip->DestroyGroup(CContextMenuReadOnlyGroupName);
}

#pragma region "Context Menu Event Handlers"

void Common::Forms::EditContextMenuStrip::toolStripMenuItemUndo_Click(
  System::Object^ Object,
  System::EventArgs^ EventArgs)
{
    Object;
    EventArgs;

    if (m_TextBox != nullptr)
    {
        m_TextBox->Undo();
    }
    else if (m_hEdit)
    {
        ::SendMessage(m_hEdit, EM_UNDO, 0, 0);
    }
}

void Common::Forms::EditContextMenuStrip::toolStripMenuItemCut_Click(
  System::Object^ Object,
  System::EventArgs^ EventArgs)
{
    Object;
    EventArgs;

    if (m_TextBox != nullptr)
    {
        m_TextBox->Cut();
    }
    else if (m_hEdit)
    {
        ::SendMessage(m_hEdit, WM_CUT, 0, 0);
    }
}

void Common::Forms::EditContextMenuStrip::toolStripMenuItemCopy_Click(
  System::Object^ Object,
  System::EventArgs^ EventArgs)
{
    Object;
    EventArgs;

    if (m_TextBox != nullptr)
    {
        m_TextBox->Copy();
    }
    else if (m_hEdit)
    {
        ::SendMessage(m_hEdit, WM_COPY, 0, 0);
    }
}

void Common::Forms::EditContextMenuStrip::toolStripMenuItemPaste_Click(
  System::Object^ Object,
  System::EventArgs^ EventArgs)
{
    Object;
    EventArgs;

    if (m_TextBox != nullptr)
    {
        m_TextBox->Paste();
    }
    else if (m_hEdit)
    {
        ::SendMessage(m_hEdit, WM_PASTE, 0, 0);
    }
}

void Common::Forms::EditContextMenuStrip::toolStripMenuItemDelete_Click(
  System::Object^ Object,
  System::EventArgs^ EventArgs)
{
    Object;
    EventArgs;

    if (m_TextBox != nullptr)
    {
        m_TextBox->Text = m_TextBox->Text->Remove(m_TextBox->SelectionStart, m_TextBox->SelectionLength);
    }
    else if (m_hEdit)
    {
        ::SendMessage(m_hEdit, WM_CLEAR, 0, 0);
    }
}

void Common::Forms::EditContextMenuStrip::toolStripMenuItemSelectAll_Click(
    System::Object^ Object,
    System::EventArgs^ EventArgs)
{
    Object;
    EventArgs;

    if (m_TextBox != nullptr)
    {
        m_TextBox->SelectAll();
    }
    else if (m_hEdit)
    {
        ::SendMessage(m_hEdit, EM_SETSEL, 0, -1);
    }
}

void Common::Forms::EditContextMenuStrip::contextMenuStrip_Opening(
  System::Object^ Object,
  System::ComponentModel::CancelEventArgs^ EventArgs)
{
    DWORD dwStartPos, dwEndPos, dwSelectionLen, dwTextLen;

    Object;
    EventArgs;

    if (m_TextBox != nullptr)
    {
        if (m_TextBox->Focused == false)
        {
            m_TextBox->Focus();
        }

        m_ContextMenuStrip->ActiveGroup = m_TextBox->ReadOnly ? CContextMenuReadOnlyGroupName : CContextMenuFullGroupName;

        m_ToolStripMenuItemUndo->Enabled = m_TextBox->CanUndo;
        m_ToolStripMenuItemCut->Enabled = m_TextBox->SelectionLength > 0;
        m_ToolStripMenuItemCopy->Enabled = m_TextBox->SelectionLength > 0;
        m_ToolStripMenuItemDelete->Enabled = m_TextBox->SelectionLength > 0;
        m_ToolStripMenuItemSelectAll->Enabled = m_TextBox->TextLength > 0 && m_TextBox->SelectionLength != m_TextBox->TextLength;
    }
    else if (m_hEdit)
    {
        ::SendMessage(m_hEdit, EM_GETSEL, (WPARAM)&dwStartPos, (LPARAM)&dwEndPos);

        dwSelectionLen = dwEndPos - dwStartPos;
        dwTextLen = ::GetWindowTextLength(m_hEdit);

        if (::GetFocus() != m_hEdit)
        {
            ::SetFocus(m_hEdit);
        }

        m_ContextMenuStrip->ActiveGroup = (::GetWindowLongPtr(m_hEdit, GWL_STYLE) & ES_READONLY) ? CContextMenuReadOnlyGroupName : CContextMenuFullGroupName;

        m_ToolStripMenuItemUndo->Enabled = ::SendMessage(m_hEdit, EM_CANUNDO, 0, 0) ? true : false;
        m_ToolStripMenuItemCut->Enabled = dwSelectionLen ? true : false;
        m_ToolStripMenuItemCopy->Enabled = dwSelectionLen ? true : false;
        m_ToolStripMenuItemDelete->Enabled = dwSelectionLen ? true : false;
        m_ToolStripMenuItemSelectAll->Enabled = (dwTextLen > 0 && dwSelectionLen != dwTextLen) ? true : false;
    }

    m_ToolStripMenuItemPaste->Enabled = ::IsClipboardFormatAvailable(CF_TEXT);
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2021 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
