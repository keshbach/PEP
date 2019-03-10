/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ZipItem.h"

#include "ZipEnumerator.h"

Common::Zip::Enumerator::Enumerator() :
  m_ZipItemList(nullptr),
  m_nCurrentZipItemIndex(-1),
  m_bAtEndOfList(false)
{
}

Common::Zip::Enumerator::~Enumerator()
{
    delete m_ZipItemList;

    m_ZipItemList = nullptr;
}

Common::Zip::Enumerator::Enumerator(
  System::Collections::Generic::List<Common::Zip::Item^>^ ZipItemList) :
  m_ZipItemList(ZipItemList),
  m_nCurrentZipItemIndex(-1),
  m_bAtEndOfList(false)
{
}

System::Boolean Common::Zip::Enumerator::MoveNext(void)
{
    if (m_bAtEndOfList)
    {
        return false;
    }

    ++m_nCurrentZipItemIndex;

    if (m_nCurrentZipItemIndex < m_ZipItemList->Count)
    {
        return true;
    }

    m_bAtEndOfList = true;

    return false;
}

void Common::Zip::Enumerator::Reset(void)
{
    m_nCurrentZipItemIndex = -1;
    m_bAtEndOfList = false;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
