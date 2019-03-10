/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TreeNodeAscendingSorter.h"

System::Int32 Common::Forms::TreeNodeAscendingSorter::Compare(
    System::Object^ Object1,
    System::Object^ Object2)
{
    System::Windows::Forms::TreeNode^ Node1 = dynamic_cast<System::Windows::Forms::TreeNode^>(Object1);
    System::Windows::Forms::TreeNode^ Node2 = dynamic_cast<System::Windows::Forms::TreeNode^>(Object2);

    return System::StringComparer::CurrentCulture->Compare(Node1->Text, Node2->Text);
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
