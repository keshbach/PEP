/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "StatusStrip.h"

Common::Forms::StatusStrip::StatusStrip()
{
	InitializeComponent();
}

Common::Forms::StatusStrip::~StatusStrip()
{
    if (components)
	{
		delete components;
	}
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2009-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
