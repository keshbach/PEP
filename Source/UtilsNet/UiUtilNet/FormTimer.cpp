/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "FormTimer.h"

Common::Forms::FormTimer::FormTimer() :
  m_FormObject(nullptr)
{
	InitializeComponent();
}

Common::Forms::FormTimer::~FormTimer()
{
    m_FormObject = nullptr;

	if (components)
	{
		delete components;
	}
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
