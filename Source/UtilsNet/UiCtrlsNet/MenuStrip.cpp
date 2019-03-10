/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "MenuStrip.h"

Common::Forms::MenuStrip::MenuStrip()
{
	InitializeComponent();

    m_ToolStripItemStateList = gcnew System::Collections::Generic::List<TToolStripItemState^>();
}

void Common::Forms::MenuStrip::SaveItemsEnableState(void)
{
    m_ToolStripItemStateList->Clear();

    EnumToolStripItems(Items, m_ToolStripItemStateList);
}

void Common::Forms::MenuStrip::RestoreItemsEnableState(void)
{
    for each (TToolStripItemState^ ToolStripItemState in m_ToolStripItemStateList)
    {
        ToolStripItemState->ToolStripItem->Enabled = ToolStripItemState->bEnabled;
    }

    m_ToolStripItemStateList->Clear();
}

void Common::Forms::MenuStrip::DisableAllItems(
  array<System::Windows::Forms::ToolStripItem^>^ ExcludedItemsArray)
{
    System::Collections::Generic::List<TToolStripItemState^>^ ToolStripItemStateList;

    ToolStripItemStateList = gcnew System::Collections::Generic::List<TToolStripItemState^>();

    EnumToolStripItems(Items, ToolStripItemStateList);

    for each (TToolStripItemState^ ToolStripItemState in ToolStripItemStateList)
    {
        if (false == IsToolStripItemInArray(ExcludedItemsArray,
                                            ToolStripItemState->ToolStripItem))
        {
            ToolStripItemState->ToolStripItem->Enabled = false;
        }
    }
}

Common::Forms::MenuStrip::~MenuStrip()
{
    delete m_ToolStripItemStateList;

    m_ToolStripItemStateList = nullptr;

	if (components)
	{
		delete components;
	}
}

void Common::Forms::MenuStrip::EnumToolStripItems(
  System::Windows::Forms::ToolStripItemCollection^ ToolStripItemCollection,
  System::Collections::Generic::List<TToolStripItemState^>^ ToolStripItemStateList)
{
    System::Windows::Forms::ToolStripMenuItem^ ToolStripMenuItem;
    TToolStripItemState^ ToolStripItemState;
    System::Boolean bAddItem;

    for each (System::Windows::Forms::ToolStripItem^ ToolStripItem in ToolStripItemCollection)
    {
        bAddItem = true;

        if (ToolStripItem->GetType() == System::Windows::Forms::ToolStripMenuItem::typeid)
        {
            ToolStripMenuItem = (System::Windows::Forms::ToolStripMenuItem^)ToolStripItem;

            if (ToolStripMenuItem->DropDownItems->Count > 0)
            {
                EnumToolStripItems(ToolStripMenuItem->DropDownItems,
                                   ToolStripItemStateList);

                bAddItem = false;
            }
        }

        if (bAddItem)
        {
            ToolStripItemState = gcnew TToolStripItemState;

            ToolStripItemState->ToolStripItem = ToolStripItem;
            ToolStripItemState->bEnabled = ToolStripItem->Enabled;

            ToolStripItemStateList->Add(ToolStripItemState);
        }
    }
}

System::Boolean Common::Forms::MenuStrip::IsToolStripItemInArray(
  array<System::Windows::Forms::ToolStripItem^>^ ToolStripItemsArray,
  System::Windows::Forms::ToolStripItem^ ToolStripItem)
{
    for each (System::Windows::Forms::ToolStripItem^ TmpToolStripItem in ToolStripItemsArray)
    {
        if (ToolStripItem == TmpToolStripItem)
        {
            return true;
        }
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
