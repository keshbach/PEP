/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "WaitCursor.h"

Common::Forms::WaitCursor::WaitCursor(
  System::Windows::Forms::Control^ pControl)
{
    m_pControl = pControl;
    m_pPreviousCursor = m_pControl->Cursor;
    m_pControl->Cursor = System::Windows::Forms::Cursors::WaitCursor;
}

Common::Forms::WaitCursor::~WaitCursor()
{
    if (m_pControl != nullptr)
    {
        m_pControl->Cursor = m_pPreviousCursor;

        m_pControl = nullptr;
        m_pPreviousCursor = nullptr;
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
