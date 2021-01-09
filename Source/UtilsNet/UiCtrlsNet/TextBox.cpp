/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ContextMenuStrip.h"
#include "TextBox.h" 

#define CContextMenuFullGroupName "Full"
#define CContextMenuReadOnlyGroupName "ReadOnly"

Common::Forms::TextBox::TextBox()
{
	InitializeComponent();
}

Common::Forms::TextBox::~TextBox()
{
	if (components)
	{
		delete components;
	}
}

void Common::Forms::TextBox::WndProc(
  System::Windows::Forms::Message% Message)
{
    switch (Message.Msg)
    {
        case WM_CREATE:
            CreateContextMenu();
            CreateToolStripGroups();
            break;
        case WM_DESTROY:
            DestroyToolStripGroups();
            DestroyContextMenu();
            break;
    }

    System::Windows::Forms::TextBox::WndProc(Message);
}

void Common::Forms::TextBox::CreateContextMenu()
{
    m_ContextMenuStrip = gcnew Common::Forms::ContextMenuStrip();

    m_ContextMenuStrip->ImageList = Common::Forms::ImageManager::ToolbarSmallImageList;

    m_ContextMenuStrip->Opening += gcnew System::ComponentModel::CancelEventHandler(this, &Common::Forms::TextBox::contextMenuStrip_Opening);

    m_ContextMenuStrip->SuspendLayout();

    // Undo

    m_ToolStripMenuItemUndo = gcnew System::Windows::Forms::ToolStripMenuItem();

    m_ToolStripMenuItemUndo->Name = "toolStripMenuItemUndo";
    m_ToolStripMenuItemUndo->ShortcutKeys = System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::Z;
    m_ToolStripMenuItemUndo->Size = System::Drawing::Size(136, 22);
    m_ToolStripMenuItemUndo->Text = "&Undo";
    m_ToolStripMenuItemUndo->ImageKey = Common::Forms::ToolbarImageKey::Undo;
    m_ToolStripMenuItemUndo->Click += gcnew System::EventHandler(this, &Common::Forms::TextBox::toolStripMenuItemUndo_Click);

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
    m_ToolStripMenuItemCut->Click += gcnew System::EventHandler(this, &Common::Forms::TextBox::toolStripMenuItemCut_Click);

    m_ContextMenuStrip->Items->Add(m_ToolStripMenuItemCut);

    // Copy

    m_ToolStripMenuItemCopy = gcnew System::Windows::Forms::ToolStripMenuItem();

    m_ToolStripMenuItemCopy->Name = "toolStripMenuItemCopy";
    m_ToolStripMenuItemCopy->ShortcutKeys = System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::C;
    m_ToolStripMenuItemCopy->Size = System::Drawing::Size(136, 22);
    m_ToolStripMenuItemCopy->Text = "&Copy";
    m_ToolStripMenuItemCopy->ImageKey = Common::Forms::ToolbarImageKey::Copy;
    m_ToolStripMenuItemCopy->Click += gcnew System::EventHandler(this, &Common::Forms::TextBox::toolStripMenuItemCopy_Click);

    m_ContextMenuStrip->Items->Add(m_ToolStripMenuItemCopy);

    // Paste

    m_ToolStripMenuItemPaste = gcnew System::Windows::Forms::ToolStripMenuItem();

    m_ToolStripMenuItemPaste->Name = "toolStripMenuItemPaste";
    m_ToolStripMenuItemPaste->ShortcutKeys = System::Windows::Forms::Keys::Control | System::Windows::Forms::Keys::V;
    m_ToolStripMenuItemPaste->Size = System::Drawing::Size(136, 22);
    m_ToolStripMenuItemPaste->Text = "&Paste";
    m_ToolStripMenuItemPaste->ImageKey = Common::Forms::ToolbarImageKey::Paste;
    m_ToolStripMenuItemPaste->Click += gcnew System::EventHandler(this, &Common::Forms::TextBox::toolStripMenuItemPaste_Click);

    m_ContextMenuStrip->Items->Add(m_ToolStripMenuItemPaste);

    // Delete

    m_ToolStripMenuItemDelete = gcnew System::Windows::Forms::ToolStripMenuItem();

    m_ToolStripMenuItemDelete->Name = "toolStripMenuItemDelete";
    m_ToolStripMenuItemDelete->ShortcutKeys = System::Windows::Forms::Keys::None;
    m_ToolStripMenuItemDelete->Size = System::Drawing::Size(136, 22);
    m_ToolStripMenuItemDelete->Text = "&Delete";
    m_ToolStripMenuItemDelete->ImageKey = Common::Forms::ToolbarImageKey::Delete;
    m_ToolStripMenuItemDelete->Click += gcnew System::EventHandler(this, &Common::Forms::TextBox::toolStripMenuItemDelete_Click);

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
    m_ToolStripMenuItemSelectAll->Click += gcnew System::EventHandler(this, &Common::Forms::TextBox::toolStripMenuItemSelectAll_Click);

    m_ContextMenuStrip->Items->Add(m_ToolStripMenuItemSelectAll);

    m_ContextMenuStrip->ResumeLayout();

    this->ContextMenuStrip = m_ContextMenuStrip;
}

void Common::Forms::TextBox::DestroyContextMenu()
{
    this->ContextMenuStrip = nullptr;

    m_ContextMenuStrip->Items->Clear();

    m_ContextMenuStrip->Opening -= gcnew System::ComponentModel::CancelEventHandler(this, &Common::Forms::TextBox::contextMenuStrip_Opening);

    m_ToolStripMenuItemUndo->Click -= gcnew System::EventHandler(this, &Common::Forms::TextBox::toolStripMenuItemUndo_Click);
    m_ToolStripMenuItemCut->Click -= gcnew System::EventHandler(this, &Common::Forms::TextBox::toolStripMenuItemCut_Click);
    m_ToolStripMenuItemCopy->Click -= gcnew System::EventHandler(this, &Common::Forms::TextBox::toolStripMenuItemCopy_Click);
    m_ToolStripMenuItemPaste->Click -= gcnew System::EventHandler(this, &Common::Forms::TextBox::toolStripMenuItemPaste_Click);
    m_ToolStripMenuItemDelete->Click -= gcnew System::EventHandler(this, &Common::Forms::TextBox::toolStripMenuItemDelete_Click);
    m_ToolStripMenuItemSelectAll->Click -= gcnew System::EventHandler(this, &Common::Forms::TextBox::toolStripMenuItemSelectAll_Click);

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

void Common::Forms::TextBox::CreateToolStripGroups()
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

void Common::Forms::TextBox::DestroyToolStripGroups()
{
    m_ContextMenuStrip->DestroyGroup(CContextMenuFullGroupName);
    m_ContextMenuStrip->DestroyGroup(CContextMenuReadOnlyGroupName);
}

#pragma region "Context Menu Event Handlers"

void Common::Forms::TextBox::toolStripMenuItemUndo_Click(
  System::Object^ Object,
  System::EventArgs^ EventArgs)
{
    Object;
    EventArgs;

    this->Undo();
}

void Common::Forms::TextBox::toolStripMenuItemCut_Click(
  System::Object^ Object,
  System::EventArgs^ EventArgs)
{
    Object;
    EventArgs;

    this->Cut();
}

void Common::Forms::TextBox::toolStripMenuItemCopy_Click(
  System::Object^ Object,
  System::EventArgs^ EventArgs)
{
    Object;
    EventArgs;

    this->Copy();
}

void Common::Forms::TextBox::toolStripMenuItemPaste_Click(
  System::Object^ Object,
  System::EventArgs^ EventArgs)
{
    Object;
    EventArgs;

    this->Paste();
}

void Common::Forms::TextBox::toolStripMenuItemDelete_Click(
  System::Object^ Object,
  System::EventArgs^ EventArgs)
{
    Object;
    EventArgs;

    this->Text = this->Text->Remove(this->SelectionStart, this->SelectionLength);
}

void Common::Forms::TextBox::toolStripMenuItemSelectAll_Click(
  System::Object^ Object,
  System::EventArgs^ EventArgs)
{
    Object;
    EventArgs;

    this->SelectAll();
}

void Common::Forms::TextBox::contextMenuStrip_Opening(
  System::Object^ Object,
  System::ComponentModel::CancelEventArgs^ EventArgs)
{
    Object;
    EventArgs;

    if (this->Focused == false)
    {
        this->Focus();
    }

    m_ContextMenuStrip->ActiveGroup = this->ReadOnly ? CContextMenuReadOnlyGroupName : CContextMenuFullGroupName;

    m_ToolStripMenuItemUndo->Enabled = this->CanUndo;
    m_ToolStripMenuItemCut->Enabled = this->SelectionLength > 0;
    m_ToolStripMenuItemCopy->Enabled = this->SelectionLength > 0;
    m_ToolStripMenuItemPaste->Enabled = ::IsClipboardFormatAvailable(CF_TEXT);
    m_ToolStripMenuItemDelete->Enabled = this->SelectionLength > 0;
    m_ToolStripMenuItemSelectAll->Enabled = this->TextLength > 0 && this->SelectionLength != this->TextLength;
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
