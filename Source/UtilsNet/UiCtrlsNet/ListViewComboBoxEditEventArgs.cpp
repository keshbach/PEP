/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ListViewComboBoxEditEventArgs.h"

Common::Forms::ListViewComboBoxEditEventArgs::ListViewComboBoxEditEventArgs(
  System::Int32 nRow,
  System::Int32 nColumn,
  System::String^ sText)
{
    m_bCancelEdit = false;
    m_Row = nRow;
    m_Column = nColumn;
    m_sText = sText;
}

Common::Forms::ListViewComboBoxEditEventArgs::ListViewComboBoxEditEventArgs()
{
    m_bCancelEdit = false;
    m_Row = -1;
    m_Column = -1;
    m_sText = nullptr;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
