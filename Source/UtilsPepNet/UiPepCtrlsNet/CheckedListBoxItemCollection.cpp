/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2020-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "ICheckedListBoxItemChange.h"

#include "CheckedListBoxEnums.h"
#include "CheckedListBoxItem.h"
#include "ICheckedListBoxList.h"
#include "CheckedListBoxItemCollectionEditor.h"

#include "CheckedListBoxItemCollection.h"

#include "CheckedListBoxItemListEnumerator.h"

#pragma region Constants

#define CSerializationCountName L"Count"
#define CSerializationItemName L"Item{0}"

#pragma endregion

#pragma region Constructor

Pep::Forms::CheckedListBoxItemCollection::CheckedListBoxItemCollection()
{
	m_CheckedListBoxList = nullptr;
	m_SyncObject = gcnew System::Object();
}

Pep::Forms::CheckedListBoxItemCollection::CheckedListBoxItemCollection(
  ICheckedListBoxList^ CheckedListBoxList)
{
    m_CheckedListBoxList = CheckedListBoxList;
	m_SyncObject = gcnew System::Object();
}

#pragma endregion

#pragma region Deconstructor

Pep::Forms::CheckedListBoxItemCollection::~CheckedListBoxItemCollection()
{
	if (m_CheckedListBoxList != nullptr)
	{
		m_CheckedListBoxList->Clear();

		m_CheckedListBoxList = nullptr;
	}

	m_SyncObject = nullptr;
}

#pragma endregion

void Pep::Forms::CheckedListBoxItemCollection::Close()
{
    m_CheckedListBoxList = nullptr;
}

#pragma region System::Collections::IList

void Pep::Forms::CheckedListBoxItemCollection::CopyTo(
  System::Array^ CheckedListBoxItemArray,
  int nIndex)
{
	System::Int32 nCount = 0;
	array<CheckedListBoxItem^>^ NewCheckedListBoxItemArray;
	System::Int32 nNewIndex;

	if (CheckedListBoxItemArray == nullptr)
	{
		throw gcnew System::Exception("Array is null");
	}

	for each (System::Object^ obj in CheckedListBoxItemArray)
	{
		if (obj != nullptr && obj->GetType() == CheckedListBoxItem::typeid)
		{
			++nCount;
		}
		else
		{
			throw gcnew System::Exception("Array contains null or invalid items");
		}
	}

	NewCheckedListBoxItemArray = gcnew array<CheckedListBoxItem^>(nCount);

	nNewIndex = 0;

	for each (System::Object^ obj in CheckedListBoxItemArray)
	{
		NewCheckedListBoxItemArray[nNewIndex] = (CheckedListBoxItem^)obj;

		++nNewIndex;
	}

	CopyTo(NewCheckedListBoxItemArray, nIndex);
}

int Pep::Forms::CheckedListBoxItemCollection::Add(
  System::Object^ Obj)
{
	if (Obj == nullptr)
	{
		return -1;
	}

	if (m_CheckedListBoxList != nullptr)
	{
		if (Obj->GetType() == CheckedListBoxItem::typeid)
		{
			m_CheckedListBoxList->Add((CheckedListBoxItem^)Obj);

			return m_CheckedListBoxList->Array->Length - 1;
		}
		else if (Obj->GetType() == System::String::typeid)
		{
			CheckedListBoxItem^ Item = gcnew CheckedListBoxItem();

			Item->Name = (System::String^)Obj;
			Item->CheckState = ECheckState::Unchecked;

			m_CheckedListBoxList->Add(Item);

			return m_CheckedListBoxList->Array->Length - 1;
		}
	}

	return -1;
}

bool Pep::Forms::CheckedListBoxItemCollection::Contains(
  System::Object^ Obj)
{
	if (Obj == nullptr || Obj->GetType() != CheckedListBoxItem::typeid)
	{
		return false;
	}

	if (m_CheckedListBoxList != nullptr)
	{
		for (int nIndex = 0; nIndex < m_CheckedListBoxList->Array->Length; ++nIndex)
		{
			if (Obj == m_CheckedListBoxList->Array[nIndex])
			{
				return true;
			}
		}
	}

	return false;
}

int Pep::Forms::CheckedListBoxItemCollection::IndexOf(
  System::Object^ Obj)
{
	if (Obj == nullptr || Obj->GetType() != CheckedListBoxItem::typeid)
	{
		return -1;
	}

	if (m_CheckedListBoxList != nullptr)
	{
		for (int nIndex = 0; nIndex < m_CheckedListBoxList->Array->Length; ++nIndex)
		{
			if (Obj == m_CheckedListBoxList->Array[nIndex])
			{
				return nIndex;
			}
		}
	}

	return -1;
}

void Pep::Forms::CheckedListBoxItemCollection::InsertNonGeneric(
  int nIndex,
  System::Object^ Obj)
{
	array<CheckedListBoxItem^>^ CheckedListBoxItemArray;

	if (Obj == nullptr || Obj->GetType() != CheckedListBoxItem::typeid)
	{
		throw gcnew System::Exception("Item is null or invalid");
	}

	CheckedListBoxItemArray = gcnew	array<CheckedListBoxItem^>(1);

	CheckedListBoxItemArray[0] = (CheckedListBoxItem^)Obj;

	CopyTo(CheckedListBoxItemArray, nIndex);
}

void Pep::Forms::CheckedListBoxItemCollection::Remove(
  System::Object^ Obj)
{
	RemoveAtNonGeneric(IndexOf(Obj));
}

void Pep::Forms::CheckedListBoxItemCollection::RemoveAtNonGeneric(
  int nIndex)
{
	if (nIndex < 0 || nIndex > m_CheckedListBoxList->Array->Length - 1)
	{
		throw gcnew System::ArgumentOutOfRangeException();
	}

	if (m_CheckedListBoxList)
	{
		m_CheckedListBoxList->Remove(nIndex);
	}
}

#pragma endregion

#pragma region System.Collections.Generic.ICollection

void Pep::Forms::CheckedListBoxItemCollection::Add(
  CheckedListBoxItem^ CheckedListBoxItem)
{
	if (m_CheckedListBoxList != nullptr)
	{
		m_CheckedListBoxList->Add(CheckedListBoxItem);
	}
}

void Pep::Forms::CheckedListBoxItemCollection::Clear()
{
	if (m_CheckedListBoxList != nullptr)
	{
		m_CheckedListBoxList->Clear();
	}
}

bool Pep::Forms::CheckedListBoxItemCollection::Contains(
  CheckedListBoxItem^ CheckedListBoxItem)
{
	if (CheckedListBoxItem == nullptr)
	{
		return false;
    }

	if (m_CheckedListBoxList != nullptr)
	{
		for each (Pep::Forms::CheckedListBoxItem^ TmpCheckedListBoxItem in m_CheckedListBoxList->Array)
		{
			if (TmpCheckedListBoxItem == CheckedListBoxItem)
			{
				return true;
			}
		}
	}

	return false;
}

void Pep::Forms::CheckedListBoxItemCollection::CopyTo(
  array<CheckedListBoxItem^>^ CheckedListBoxItemArray,
  int nIndex)
{
	if (CheckedListBoxItemArray == nullptr)
	{
		throw gcnew System::Exception("Array is null");
	}

	if (m_CheckedListBoxList != nullptr)
	{
		m_CheckedListBoxList->CopyTo(CheckedListBoxItemArray, nIndex);
	}
}

bool Pep::Forms::CheckedListBoxItemCollection::Remove(
  CheckedListBoxItem^ CheckedListBoxItem)
{
	if (CheckedListBoxItem == nullptr)
	{
		throw gcnew System::Exception("Item is null");
	}

	if (m_CheckedListBoxList != nullptr)
	{
		array<Pep::Forms::CheckedListBoxItem^>^ CheckedListBoxItemArray = m_CheckedListBoxList->Array;

		for (int nIndex = 0; nIndex < CheckedListBoxItemArray->Length; ++nIndex)
		{
			if (CheckedListBoxItemArray[nIndex] == CheckedListBoxItem)
			{
				return m_CheckedListBoxList->Remove(nIndex);
			}
		}
	}

	return false;
}

#pragma endregion

#pragma region System.Collections.Generic.IEnumerable
			
System::Collections::IEnumerator^ Pep::Forms::CheckedListBoxItemCollection::GetEnumerator()
{
	return GetEnumeratorBase();
}

System::Collections::Generic::IEnumerator<Pep::Forms::CheckedListBoxItem^>^ Pep::Forms::CheckedListBoxItemCollection::GetEnumeratorBase()
{
	System::Collections::Generic::List<CheckedListBoxItem^>^ CheckedListBoxItemList = gcnew System::Collections::Generic::List<CheckedListBoxItem^>();

	if (m_CheckedListBoxList != nullptr)
	{
		for each (Pep::Forms::CheckedListBoxItem^ CheckedListBoxItem in m_CheckedListBoxList->Array)
		{
			CheckedListBoxItemList->Add(CheckedListBoxItem);
		}
	}

	return gcnew CheckedListBoxItemListEnumerator(CheckedListBoxItemList);
}

#pragma endregion

#pragma region System::Collections::Generic::IList

int Pep::Forms::CheckedListBoxItemCollection::IndexOf(
  CheckedListBoxItem^ CheckedListBoxItem)
{
	if (CheckedListBoxItem == nullptr)
	{
		return -1;
	}

	if (m_CheckedListBoxList != nullptr)
	{
		array<Pep::Forms::CheckedListBoxItem^>^ CheckedListBoxItemArray = m_CheckedListBoxList->Array;

		for (int nIndex = 0; nIndex < CheckedListBoxItemArray->Length; ++nIndex)
		{
			if (CheckedListBoxItemArray[nIndex] == CheckedListBoxItem)
			{
				return nIndex;
			}
		}
	}

	return -1;
}

void Pep::Forms::CheckedListBoxItemCollection::Insert(
  int nIndex,
  CheckedListBoxItem^ CheckedListBoxItem)
{
	if (CheckedListBoxItem == nullptr)
	{
		throw gcnew System::Exception("Item is null");
	}

	if (m_CheckedListBoxList != nullptr)
	{
		array<Pep::Forms::CheckedListBoxItem^>^ CheckedListBoxItemArray = gcnew array<Pep::Forms::CheckedListBoxItem^>(1);

		CheckedListBoxItemArray[0] = CheckedListBoxItem;

		m_CheckedListBoxList->CopyTo(CheckedListBoxItemArray, nIndex);
	}
}

void Pep::Forms::CheckedListBoxItemCollection::RemoveAt(
  int nIndex)
{
	if (m_CheckedListBoxList != nullptr)
	{
		m_CheckedListBoxList->Remove(nIndex);
	}
}

#pragma endregion

void Pep::Forms::CheckedListBoxItemCollection::AddRange(
  array<System::Object^>^ ObjectArray)
{
	if (ObjectArray == nullptr)
	{
		throw gcnew System::Exception("Array is null");
	}

	if (m_CheckedListBoxList != nullptr)
	{
		for each (System::Object^ Obj in ObjectArray)
		{
			if (Obj != nullptr && Obj->GetType() == CheckedListBoxItem::typeid)
			{
				m_CheckedListBoxList->Add((CheckedListBoxItem^)Obj);
			}
			else if (Obj != nullptr && Obj->GetType() == System::String::typeid)
			{
				CheckedListBoxItem^ Item = gcnew CheckedListBoxItem();

				Item->Name = (System::String^)Obj;
				Item->CheckState = ECheckState::Unchecked;

				m_CheckedListBoxList->Add(Item);
			}
		}
	}
}

Pep::Forms::CheckedListBoxItem^ Pep::Forms::CheckedListBoxItemCollection::GetItem(
  int nIndex)
{
	if (m_CheckedListBoxList != nullptr)
	{
		array<Pep::Forms::CheckedListBoxItem^>^ CheckedListBoxItemArray = m_CheckedListBoxList->Array;

		if (nIndex < 0 || nIndex > CheckedListBoxItemArray->Length)
		{
			throw gcnew System::ArgumentOutOfRangeException();
		}

		return CheckedListBoxItemArray[nIndex];
	}

	throw gcnew System::ArgumentOutOfRangeException();
}

void Pep::Forms::CheckedListBoxItemCollection::SetItem(
  int nIndex,
  CheckedListBoxItem^ CheckedListBoxItem)
{
	if (m_CheckedListBoxList != nullptr)
	{
		array<Pep::Forms::CheckedListBoxItem^>^ CheckedListBoxItemArray = m_CheckedListBoxList->Array;

		if (nIndex < 0 || nIndex > CheckedListBoxItemArray->Length)
		{
			throw gcnew System::ArgumentOutOfRangeException();
		}

		CheckedListBoxItemArray = gcnew array<Pep::Forms::CheckedListBoxItem^>(1);

		CheckedListBoxItemArray[0] = CheckedListBoxItem;

		m_CheckedListBoxList->CopyTo(CheckedListBoxItemArray, nIndex);
	}

	throw gcnew System::ArgumentOutOfRangeException();
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2020-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
