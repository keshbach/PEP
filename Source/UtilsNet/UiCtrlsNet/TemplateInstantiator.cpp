/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ContextMenuStrip.h"
#include "StatusStrip.h"
#include "ToolStrip.h"

#pragma warning (disable:4505)

static void lTemplateInstantiator()
{
	Common::Forms::ContextMenuStrip^ ContextMenuStrip = gcnew Common::Forms::ContextMenuStrip();
	Common::Forms::StatusStrip^ StatusStrip = gcnew Common::Forms::StatusStrip();
	Common::Forms::ToolStrip^ ToolStrip = gcnew Common::Forms::ToolStrip();

	ContextMenuStrip->CreateGroup(nullptr, nullptr);
	StatusStrip->CreateGroup(nullptr, nullptr);
	ToolStrip->CreateGroup(nullptr, nullptr);

	ContextMenuStrip->DestroyGroup(nullptr);
	StatusStrip->DestroyGroup(nullptr);
	ToolStrip->DestroyGroup(nullptr);
}

#pragma warning (default:4505)

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
