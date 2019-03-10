/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TreeNodeDescendingSorter.h"

System::Int32 Common::Forms::TreeNodeDescendingSorter::Compare(
    System::Object^ Object1,
    System::Object^ Object2)
{
    System::Windows::Forms::TreeNode^ Node1 = dynamic_cast<System::Windows::Forms::TreeNode^>(Object1);
    System::Windows::Forms::TreeNode^ Node2 = dynamic_cast<System::Windows::Forms::TreeNode^>(Object2);
    System::Int32 nResult;

    nResult = System::StringComparer::CurrentCulture->Compare(Node1->Text, Node2->Text);

    if (nResult < 0)
    {
        nResult = 1;
    }
    else if (nResult > 0)
    {
        nResult = -1;
    }

    return nResult;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
