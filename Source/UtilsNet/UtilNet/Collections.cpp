/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2024 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "CollectionComparers.h"
#include "Collections.h"

Common::Collections::StringCollection::StringCollection() :
  m_Comparer(System::StringComparer::CurrentCultureIgnoreCase)
{
}

System::Boolean Common::Collections::StringCollection::Contains(
  System::String^ sValue)
{
    for each (System::String^ sCurValue in this)
    {
        if (0 == m_Comparer->Compare(sCurValue, sValue))
        {
            return true;
        }
    }

    return false;
}

Common::Collections::StringCollection^ Common::Collections::StringCollection::MakeCopy()
{
    Common::Collections::StringCollection^ NewColl = gcnew Common::Collections::StringCollection();

    for each (System::String^ sValue in this)
    {
        NewColl->Add(sValue);
    }

    return NewColl;
}

Common::Collections::StringCollection^ Common::Collections::StringCollection::Merge(
  Common::Collections::StringCollection^ ExistingColl)
{
    Common::Collections::StringCollection^ NewColl = gcnew Common::Collections::StringCollection();

    for each (System::String^ sValue in this)
    {
        if (!ExistingColl->Contains(sValue))
        {
            NewColl->Add(sValue);
        }
    }

    return NewColl;
}

Common::Collections::StringCollection^ Common::Collections::StringCollection::FromArray(
  array<System::String^>^ StringArray)
{
    Common::Collections::StringCollection^ Coll = gcnew Common::Collections::StringCollection();

    for (System::Int32 nIndex = 0; nIndex < StringArray->Length; ++nIndex)
    {
        Coll->Add(StringArray[nIndex]);
    }

    return Coll;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2024 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
