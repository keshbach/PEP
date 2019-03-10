/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "UtContextMenuStrip.h"

void Common::Forms::UtContextMenuStrip::SaveShortcutKeys(
  System::Windows::Forms::ContextMenuStrip^ ContextMenuStrip)
{
    System::Diagnostics::Debug::Assert(s_ToolStripMenuItemShortcutKeysList == nullptr);

    s_ToolStripMenuItemShortcutKeysList = gcnew System::Collections::Generic::List<TToolStripMenuItemShortcutKeys^>();

    EnumToolStripItems(ContextMenuStrip->Items);
}

void Common::Forms::UtContextMenuStrip::RestoreShortcutKeys(void)
{
    System::Diagnostics::Debug::Assert(s_ToolStripMenuItemShortcutKeysList != nullptr);

    for each (TToolStripMenuItemShortcutKeys^ ToolStripMenuItemShortcutKeys in s_ToolStripMenuItemShortcutKeysList)
    {
        ToolStripMenuItemShortcutKeys->ToolStripMenuItem->ShortcutKeys = ToolStripMenuItemShortcutKeys->ShortcutKeys;
    }

    delete s_ToolStripMenuItemShortcutKeysList;

    s_ToolStripMenuItemShortcutKeysList = nullptr;
}

void Common::Forms::UtContextMenuStrip::ClearShortcutKeys(void)
{
    System::Diagnostics::Debug::Assert(s_ToolStripMenuItemShortcutKeysList != nullptr);

    for each (TToolStripMenuItemShortcutKeys^ ToolStripMenuItemShortcutKeys in s_ToolStripMenuItemShortcutKeysList)
    {
        ToolStripMenuItemShortcutKeys->ToolStripMenuItem->ShortcutKeys = System::Windows::Forms::Keys::None;
    }
}

void Common::Forms::UtContextMenuStrip::EnumToolStripItems(
  System::Windows::Forms::ToolStripItemCollection^ ToolStripItemCollection)
{
    System::Windows::Forms::ToolStripMenuItem^ ToolStripMenuItem;
    TToolStripMenuItemShortcutKeys^ ToolStripMenuItemShortcutKeys;

    for each (System::Windows::Forms::ToolStripItem^ ToolStripItem in ToolStripItemCollection)
    {
        if (ToolStripItem->GetType() == System::Windows::Forms::ToolStripMenuItem::typeid)
        {
            ToolStripMenuItem = (System::Windows::Forms::ToolStripMenuItem^)ToolStripItem;

            ToolStripMenuItemShortcutKeys = gcnew TToolStripMenuItemShortcutKeys;

            ToolStripMenuItemShortcutKeys->ToolStripMenuItem = ToolStripMenuItem;
            ToolStripMenuItemShortcutKeys->ShortcutKeys = ToolStripMenuItem->ShortcutKeys;

            s_ToolStripMenuItemShortcutKeysList->Add(ToolStripMenuItemShortcutKeys);

            if (ToolStripMenuItem->DropDownItems->Count > 0)
            {
                EnumToolStripItems(ToolStripMenuItem->DropDownItems);
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
