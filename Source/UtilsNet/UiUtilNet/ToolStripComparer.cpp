/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2017-2025 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ToolStripComparer.h"

Common::Forms::ToolStripComparer::ToolStripComparer()
{
}

int Common::Forms::ToolStripComparer::Compare(
  System::Windows::Forms::ToolStrip^ x,
  System::Windows::Forms::ToolStrip^ y)
{
	if (x->TabIndex < y->TabIndex)
	{
		return -1;
	}

	if (x->TabIndex > y->TabIndex)
	{
		return 1;
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2017-2025 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
