/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2025 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ToolStripMenuItem.h"

#include "IUpdateToolStripItems.h"

#include "Includes/UtTemplates.h"

Common::Forms::ToolStripMenuItem::ToolStripMenuItem()
{
}

Common::Forms::ToolStripMenuItem::~ToolStripMenuItem()
{
}

void Common::Forms::ToolStripMenuItem::OnEnabledChanged(
  System::EventArgs^ e)
{
	System::Windows::Forms::ToolStripPanel^ ToolStripPanel;
	Common::Forms::IUpdateToolStripItems^ UpdateToolStripItems;

	System::Windows::Forms::ToolStripMenuItem::OnEnabledChanged(e);

	if (OwnerItem != nullptr)
	{
		// Menu strip item

		if (OwnerItem->Owner->Parent != nullptr)
		{
			ToolStripPanel = (System::Windows::Forms::ToolStripPanel^)OwnerItem->Owner->Parent;

			if (IsInstance<Common::Forms::IUpdateToolStripItems^>(ToolStripPanel->ParentForm))
			{
				UpdateToolStripItems = (Common::Forms::IUpdateToolStripItems^)ToolStripPanel->ParentForm;

				UpdateToolStripItems->UpdateToolStripItems(this);
			}
		}
	}
	else
	{
		// Context menu item
	}
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2025 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
