/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

template<typename T>
public ref class ToolStripItemGroupImpl : public T
{
public:
    ToolStripItemGroupImpl()
    {
        m_GroupToolStripItemsList = gcnew System::Collections::Generic::List<TGroupToolStripItems^>();
        m_sActiveGroupName = L"";
    }

protected:
    ~ToolStripItemGroupImpl()
    {
        if (m_GroupToolStripItemsList != nullptr)
        {
            delete m_GroupToolStripItemsList;

            m_GroupToolStripItemsList = nullptr;
        }

        m_sActiveGroupName = nullptr;
    }

public:
    /// <summary>
    /// Indicates the group of ToolStripItem's that are active.
    /// </summary>

    [System::ComponentModel::Browsable(false),
        System::ComponentModel::EditorBrowsableAttribute(System::ComponentModel::EditorBrowsableState::Advanced)]
    property System::String^ ActiveGroup
    {
        System::String^ get()
        {
            return m_sActiveGroupName;
        }

        void set(System::String^ value)
        {
            ShowGroup(value);
        }
    }

public:
    /// <summary>
    /// Creates a new group and associates them with an array of ToolStripItems.
    /// </summary>

    void CreateGroup(System::String^ sGroupName,
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

    /// <summary>
    /// Destroys a group associated with any array of ToolStripItems.
    /// </summary>

    void DestroyGroup(
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

private:
    void ShowGroup(
        System::String^ sGroupName)
    {
        TGroupToolStripItems^ GroupToolItems = FindGroup(sGroupName);

        this->SuspendLayout();

        for each (System::Windows::Forms::ToolStripItem ^ ToolStripItem in this->Items)
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

        this->ResumeLayout();
    }

private:
    ref struct TGroupToolStripItems
    {
        System::String^ sGroupName;
        array<System::Windows::Forms::ToolStripItem^>^ ItemsArray;
    };

    TGroupToolStripItems^ FindGroup(
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

    System::Collections::Generic::List<TGroupToolStripItems^>^ m_GroupToolStripItemsList;
    System::String^ m_sActiveGroupName;
};

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
