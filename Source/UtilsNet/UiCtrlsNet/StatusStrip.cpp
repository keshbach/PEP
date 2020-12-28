/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "StatusStrip.h"

Common::Forms::StatusStrip::StatusStrip()
{
	InitializeComponent();

    m_GroupToolStripItemsList = gcnew System::Collections::Generic::List<TGroupToolStripItems^>();
    m_sActiveGroupName = L"";
}

Common::Forms::StatusStrip::~StatusStrip()
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

void Common::Forms::StatusStrip::CreateGroup(
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

void Common::Forms::StatusStrip::DestroyGroup(
  System::String^ sGroupName)
{
    if (m_GroupToolStripItemsList != nullptr)
    {
        for each (TGroupToolStripItems^ GroupToolItems in m_GroupToolStripItemsList)
        {
            if (sGroupName->CompareTo(GroupToolItems->sGroupName) == 0)
            {
                m_GroupToolStripItemsList->Remove(GroupToolItems);

                delete GroupToolItems->ItemsArray;
                delete GroupToolItems;

                return;
            }
        }

        System::Diagnostics::Debug::Assert(false, L"Status Strip Group not found.");
    }
}

void Common::Forms::StatusStrip::ShowGroup(
  System::String^ sGroupName)
{
    TGroupToolStripItems^ GroupToolItems = FindGroup(sGroupName);

    SuspendLayout();

    for each (System::Windows::Forms::ToolStripItem^ ToolStripItem in Items)
    {
        ToolStripItem->Visible = false;
    }

    if (GroupToolItems != nullptr)
    {
        m_sActiveGroupName = sGroupName;

        for each (System::Windows::Forms::ToolStripItem^ ToolStripItem in GroupToolItems->ItemsArray)
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

Common::Forms::StatusStrip::TGroupToolStripItems^ Common::Forms::StatusStrip::FindGroup(
  System::String^ sGroupName)
{
    for each (TGroupToolStripItems^ GroupToolStripItems in m_GroupToolStripItemsList)
    {
        if (sGroupName->CompareTo(GroupToolStripItems->sGroupName) == 0)
        {
            return GroupToolStripItems;
        }
    }

    return nullptr;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
