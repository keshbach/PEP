/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2018-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "MenuItemHelp.h"

#include "MenuItemHelpList.h"

#pragma region Constants
#define CSerializationCountName L"Count"
#define CSerializationItemName L"Item{0}"
#pragma endregion

#pragma region Constructors

Common::Forms::MenuItemHelpList::MenuItemHelpList()
{
    m_MenuItemHelpList = gcnew System::Collections::Generic::List<MenuItemHelp^>();
}

Common::Forms::MenuItemHelpList::MenuItemHelpList(
  System::Runtime::Serialization::SerializationInfo^ info,
  System::Runtime::Serialization::StreamingContext context)
{
    int nCount;

    context;

    if (info == nullptr)
    {
        throw gcnew System::ArgumentNullException("info");
    }

    m_MenuItemHelpList = gcnew System::Collections::Generic::List<MenuItemHelp^>();

    nCount = info->GetInt32(CSerializationCountName);

    m_MenuItemHelpList->Capacity = nCount;

    for (int nIndex = 0; nIndex < nCount; ++nIndex)
    {
        m_MenuItemHelpList[nIndex] = (MenuItemHelp^)info->GetValue(System::String::Format(CSerializationItemName, nIndex), MenuItemHelp::typeid);
    }
}

#pragma endregion

#pragma region Deconstructor

Common::Forms::MenuItemHelpList::~MenuItemHelpList()
{
    m_MenuItemHelpList->Clear();

    m_MenuItemHelpList = nullptr;
}

#pragma endregion

#pragma region System.Collections.Generic.ICollection

void Common::Forms::MenuItemHelpList::Add(
  MenuItemHelp^ item)
{
    if (!m_MenuItemHelpList->Contains(item))
    {
        m_MenuItemHelpList->Add(item);
    }
}

void Common::Forms::MenuItemHelpList::Clear()
{
    m_MenuItemHelpList->Clear();
}

bool Common::Forms::MenuItemHelpList::Contains(
  MenuItemHelp^ item)
{
    return m_MenuItemHelpList->Contains(item);
}

void Common::Forms::MenuItemHelpList::CopyTo(
  array<MenuItemHelp^>^ array,
  int arrayIndex)
{
    m_MenuItemHelpList->CopyTo(array, arrayIndex);
}

bool Common::Forms::MenuItemHelpList::Remove(
  MenuItemHelp^ item)
{
    return m_MenuItemHelpList->Remove(item);
}

#pragma endregion

#pragma region System.Collections.Generic.IEnumerable

System::Collections::IEnumerator^ Common::Forms::MenuItemHelpList::GetEnumerator()
{
    return m_MenuItemHelpList->GetEnumerator();
}

System::Collections::Generic::IEnumerator<Common::Forms::MenuItemHelp^>^ Common::Forms::MenuItemHelpList::GetEnumeratorGeneric()
{
    return m_MenuItemHelpList->GetEnumerator();
}

#pragma endregion

#pragma region System::Collections::Generic::IList

int Common::Forms::MenuItemHelpList::IndexOf(MenuItemHelp^ item)
{
    return m_MenuItemHelpList->IndexOf(item);
}

void Common::Forms::MenuItemHelpList::Insert(int index, MenuItemHelp^ item)
{
    if (!m_MenuItemHelpList->Contains(item))
    {
        m_MenuItemHelpList->Insert(index, item);
    }
}

void Common::Forms::MenuItemHelpList::RemoveAt(int index)
{
    m_MenuItemHelpList->RemoveAt(index);
}

#pragma endregion

#pragma region System::Runtime::Serialization::ISerializable

void Common::Forms::MenuItemHelpList::GetObjectData(
  System::Runtime::Serialization::SerializationInfo^ info,
  System::Runtime::Serialization::StreamingContext context)
{
    context;

    if (info == nullptr)
    {
        throw gcnew System::ArgumentNullException("info");
    }

    info->AddValue(CSerializationCountName, m_MenuItemHelpList->Count);
    
    for (int nIndex = 0; nIndex < m_MenuItemHelpList->Count; ++nIndex)
    {
        info->AddValue(System::String::Format(CSerializationItemName, nIndex), m_MenuItemHelpList[nIndex]);
    }
}

#pragma endregion

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2018-2018 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
