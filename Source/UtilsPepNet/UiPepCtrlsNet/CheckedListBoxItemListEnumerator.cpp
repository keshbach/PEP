/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2020-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ICheckedListBoxItemChange.h"

#include "CheckedListBoxEnums.h"
#include "CheckStateChangeEventArgs.h"
#include "CheckedListBoxItem.h"

#include "CheckedListBoxItemListEnumerator.h"

Pep::Forms::CheckedListBoxItemListEnumerator::CheckedListBoxItemListEnumerator() :
	m_CheckedListBoxItemList(nullptr),
	m_nCurrentCheckedListBoxItemIndex(-1),
	m_bAtEndOfList(false)
{
}

Pep::Forms::CheckedListBoxItemListEnumerator::~CheckedListBoxItemListEnumerator()
{
	delete m_CheckedListBoxItemList;

	m_CheckedListBoxItemList = nullptr;
}

Pep::Forms::CheckedListBoxItemListEnumerator::CheckedListBoxItemListEnumerator(
	System::Collections::Generic::List<Pep::Forms::CheckedListBoxItem^>^ CheckedListBoxItemList) :
	m_CheckedListBoxItemList(CheckedListBoxItemList),
	m_nCurrentCheckedListBoxItemIndex(-1),
	m_bAtEndOfList(false)
{
}

System::Boolean Pep::Forms::CheckedListBoxItemListEnumerator::MoveNext(void)
{
	if (m_bAtEndOfList)
	{
		return false;
	}

	++m_nCurrentCheckedListBoxItemIndex;

	if (m_nCurrentCheckedListBoxItemIndex < m_CheckedListBoxItemList->Count)
	{
		return true;
	}

	m_bAtEndOfList = true;

	return false;
}

void Pep::Forms::CheckedListBoxItemListEnumerator::Reset(void)
{
	m_nCurrentCheckedListBoxItemIndex = -1;
	m_bAtEndOfList = false;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2020-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
