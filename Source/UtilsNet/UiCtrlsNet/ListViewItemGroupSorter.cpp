/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ListViewItemGroupSorter.h"

Common::Forms::ListViewItemGroupSorter::ListViewItemGroupSorter() :
  m_nColumn(0)
{
}

Common::Forms::ListViewItemGroupSorter::ListViewItemGroupSorter(
  System::Int32 nColumn) :
  m_nColumn(nColumn)
{
}

System::Int32 Common::Forms::ListViewItemGroupSorter::Compare(
  System::Object^ Object1,
  System::Object^ Object2)
{
    System::Windows::Forms::ListViewItem^ Item1 = dynamic_cast<System::Windows::Forms::ListViewItem^>(Object1);
    System::Windows::Forms::ListViewItem^ Item2 = dynamic_cast<System::Windows::Forms::ListViewItem^>(Object2);
    System::String^ Text1 = Item1->SubItems[m_nColumn]->Text;
    System::String^ Text2 = Item2->SubItems[m_nColumn]->Text;
    System::Int32 nResult;

    if (Text1->Length < Text2->Length)
    {
        nResult = System::String::Compare(Text1, 0, Text2, 0, Text1->Length);

        if (nResult == 0)
        {
            nResult = -1;
        }
    }
    else if (Text1->Length > Text2->Length)
    {
        nResult = System::String::Compare(Text1, 0, Text2, 0, Text2->Length);

        if (nResult == 0)
        {
            nResult = 1;
        }
    }
    else
    {
        nResult = System::String::Compare(Text1, Text2);
    }

    return nResult;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
