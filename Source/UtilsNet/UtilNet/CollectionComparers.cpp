/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2024 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "CollectionComparers.h"

Common::CollectionComparers::StringSortedListComparer::StringSortedListComparer(
  System::Boolean bCaseSensitive)
{
    m_bCaseSensitive = bCaseSensitive;
}

int Common::CollectionComparers::StringSortedListComparer::Compare(
  System::String^ x,
  System::String^ y)
{
    return System::String::Compare(x, y, !m_bCaseSensitive);
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2024 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
