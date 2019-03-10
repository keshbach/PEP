/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ToolStripProgressBar.h"

Common::Forms::ToolStripProgressBar::ToolStripProgressBar()
{
    InitializeComponent();
}

Common::Forms::ToolStripProgressBar::ToolStripProgressBar(
  System::String^ sName) :
  System::Windows::Forms::ToolStripProgressBar(sName)
{
}

Common::Forms::ToolStripProgressBar::~ToolStripProgressBar()
{
    if (components)
    {
        delete components;
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2014-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
