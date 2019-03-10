/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ListViewItemGroupSequentialSorter.h"

Common::Forms::ListViewItemGroupSequentialSorter::ListViewItemGroupSequentialSorter() :
  m_nColumn(0)
{
}

Common::Forms::ListViewItemGroupSequentialSorter::ListViewItemGroupSequentialSorter(
  System::Int32 nColumn) :
  m_nColumn(nColumn)
{
}

System::Int32 Common::Forms::ListViewItemGroupSequentialSorter::Compare(
  System::Object^ Object1,
  System::Object^ Object2)
{
    System::Windows::Forms::ListViewItem^ Item1 = dynamic_cast<System::Windows::Forms::ListViewItem^>(Object1);
    System::Windows::Forms::ListViewItem^ Item2 = dynamic_cast<System::Windows::Forms::ListViewItem^>(Object2);
    System::String^ Text1 = Item1->SubItems[m_nColumn]->Text;
    System::String^ Text2 = Item2->SubItems[m_nColumn]->Text;
    System::Int32 nResult, nMatchingChars;

    if (Text1->Length < Text2->Length)
    {
        nMatchingChars = FindMatchingChars(Text1, Text2);

        if (nMatchingChars < Text1->Length)
        {
            if (nMatchingChars > 0)
            {
                nResult = -1;
            }
            else
            {
                if (CompareNumbers(Text1, Text2, &nResult) == false)
                {
                    nResult = System::String::Compare(Text1, 0, Text2, 0, Text1->Length);
                }
            }
        }
        else
        {
            nResult = -1;
        }
    }
    else if (Text1->Length > Text2->Length)
    {
        nMatchingChars = FindMatchingChars(Text1, Text2);

        if (nMatchingChars < Text2->Length)
        {
            if (nMatchingChars > 0)
            {
                nResult = 1;
            }
            else
            {
                if (CompareNumbers(Text1, Text2, &nResult) == false)
                {
                    nResult = System::String::Compare(Text1, 0, Text2, 0, Text2->Length);
                }
            }
        }
        else
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

System::Int32 Common::Forms::ListViewItemGroupSequentialSorter::FindMatchingChars(
  System::String^ String1,
  System::String^ String2)
{
    int nIndex = 0;

    while (nIndex < String1->Length && nIndex < String2->Length)
    {
        if (String1[nIndex] != String2[nIndex])
        {
            return nIndex;
        }

        ++nIndex;
    }

    return nIndex;
}

bool Common::Forms::ListViewItemGroupSequentialSorter::CompareNumbers(
  System::String^ String1,
  System::String^ String2,
  System::Int32* pnResult)
{
    System::UInt32 nNum1, nNum2;

    *pnResult = 0;

    if (System::UInt32::TryParse(String1, nNum1) &&
        System::UInt32::TryParse(String2, nNum2))
    {
        if (nNum1 < nNum2)
        {
            *pnResult = -1;
        }
        else if (nNum1 > nNum2)
        {
            *pnResult = 1;
        }

        return true;
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
