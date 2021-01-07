/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "MenuStrip.h"

#include "Includes/UtTemplates.h"

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

void Common::Forms::MenuStrip::UpdateToolStripItemsImageList(
  System::Windows::Forms::ImageList^ ImageList,
  System::Windows::Forms::ToolStripItemCollection^ ToolStripItemCollection)
{
    System::Boolean bSetImageList = true;
    System::Windows::Forms::ToolStripDropDownItem^ ToolStripDropDownItem;

    for each (System::Windows::Forms::ToolStripItem ^ ToolStripItem in ToolStripItemCollection)
    {
        if (bSetImageList)
        {
            if (ToolStripItem->Owner->ImageList != ImageList)
            {
                ToolStripItem->Owner->ImageList = ImageList;
            }

            bSetImageList = false;
        }

        if (IsInstance<System::Windows::Forms::ToolStripDropDownItem^>(ToolStripItem))
        {
            ToolStripDropDownItem = (System::Windows::Forms::ToolStripDropDownItem^)ToolStripItem;

            if (ToolStripDropDownItem->DropDownItems->Count > 0)
            {
                UpdateToolStripItemsImageList(ImageList, ToolStripDropDownItem->DropDownItems);
            }
        }
    }
}

void Common::Forms::MenuStrip::EnumToolStripItems(
  System::Windows::Forms::ToolStripItemCollection^ ToolStripItemCollection,
  System::Collections::Generic::List<TToolStripItemState^>^ ToolStripItemStateList)
{
	System::Windows::Forms::ToolStripDropDownItem^ ToolStripDropDownItem;
    TToolStripItemState^ ToolStripItemState;
    System::Boolean bAddItem;

    for each (System::Windows::Forms::ToolStripItem^ ToolStripItem in ToolStripItemCollection)
    {
        bAddItem = true;

		if (IsInstance<System::Windows::Forms::ToolStripDropDownItem^>(ToolStripItem))
		{
    		ToolStripDropDownItem = (System::Windows::Forms::ToolStripDropDownItem^)ToolStripItem;

            if (ToolStripDropDownItem->DropDownItems->Count > 0)
            {
                EnumToolStripItems(ToolStripDropDownItem->DropDownItems,
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
//  Copyright (C) 2009-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
