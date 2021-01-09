/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ContextMenuStrip.h"

Common::Forms::ContextMenuStrip::ContextMenuStrip()
{
	InitializeComponent();

	m_GroupToolStripItemsList = gcnew System::Collections::Generic::List<TGroupToolStripItems^>();
	m_sActiveGroupName = L"";
}

Common::Forms::ContextMenuStrip::~ContextMenuStrip()
{
	if (m_GroupToolStripItemsList != nullptr)
	{
		delete m_GroupToolStripItemsList;

		m_GroupToolStripItemsList = nullptr;
	}

	m_sActiveGroupName = nullptr;

	if (components)
	{
		delete components;
	}
}

void Common::Forms::ContextMenuStrip::CreateGroup(
  System::String^ sGroupName,
  array<System::Windows::Forms::ToolStripItem^>^ ItemsArray)
{
    TGroupToolStripItems^ GroupToolItems = FindGroup(sGroupName);

    if (GroupToolItems == nullptr)
    {
        GroupToolItems = gcnew TGroupToolStripItems();

        GroupToolItems->sGroupName = sGroupName;

        m_GroupToolStripItemsList->Add(GroupToolItems);
    }

    GroupToolItems->ItemsArray = ItemsArray;
}

void Common::Forms::ContextMenuStrip::DestroyGroup(
  System::String^ sGroupName)
{
    if (m_GroupToolStripItemsList != nullptr)
    {
        for each (TGroupToolStripItems ^ GroupToolItems in m_GroupToolStripItemsList)
        {
            if (sGroupName->CompareTo(GroupToolItems->sGroupName) == 0)
            {
                m_GroupToolStripItemsList->Remove(GroupToolItems);

                delete GroupToolItems->ItemsArray;
                delete GroupToolItems;

                return;
            }
        }

        System::Diagnostics::Debug::Assert(false, L"Tool Strip Group not found.");
    }
}

void Common::Forms::ContextMenuStrip::ShowGroup(
  System::String^ sGroupName)
{
    TGroupToolStripItems^ GroupToolItems = FindGroup(sGroupName);

    SuspendLayout();

    for each (System::Windows::Forms::ToolStripItem ^ ToolStripItem in Items)
    {
        ToolStripItem->Visible = false;
    }

    if (GroupToolItems != nullptr)
    {
        m_sActiveGroupName = sGroupName;

        for each (System::Windows::Forms::ToolStripItem ^ ToolStripItem in GroupToolItems->ItemsArray)
        {
            ToolStripItem->Visible = true;
        }
    }
    else
    {
        m_sActiveGroupName = L"";
    }

    ResumeLayout();
}

Common::Forms::ContextMenuStrip::TGroupToolStripItems^ Common::Forms::ContextMenuStrip::FindGroup(
  System::String^ sGroupName)
{
    for each (TGroupToolStripItems ^ GroupToolStripItems in m_GroupToolStripItemsList)
    {
        if (sGroupName->CompareTo(GroupToolStripItems->sGroupName) == 0)
        {
            return GroupToolStripItems;
        }
    }

    return nullptr;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
