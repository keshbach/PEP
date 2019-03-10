/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "PinConfigValues.h"

Pep::Programmer::PinConfigValues::PinConfigValues()
{
}

Pep::Programmer::PinConfigValues::~PinConfigValues()
{
    delete m_TypesArray;
    delete m_PolaritiesArray;

    m_TypesArray = nullptr;
    m_PolaritiesArray = nullptr;
}

Pep::Programmer::PinConfigValues::PinConfigValues(
  const TDevicePinConfigValues* pDevicePinConfigValues) :
  m_nPin(pDevicePinConfigValues->nPin),
  m_TypesArray(nullptr),
  m_PolaritiesArray(nullptr)
{
    if (pDevicePinConfigValues->nTypesCount > 0)
    {
    	m_TypesArray = gcnew array<System::String^>(pDevicePinConfigValues->nTypesCount);

	    for (UINT nIndex = 0; nIndex < pDevicePinConfigValues->nTypesCount; ++nIndex)
	    {
		    m_TypesArray[nIndex] = gcnew System::String(pDevicePinConfigValues->ppszTypes[nIndex]);
	    }
    }

    if (pDevicePinConfigValues->nPolaritiesCount > 0)
    {
        m_PolaritiesArray = gcnew array<System::String^>(pDevicePinConfigValues->nPolaritiesCount);

	    for (UINT nIndex = 0; nIndex < pDevicePinConfigValues->nPolaritiesCount; ++nIndex)
	    {
		    m_PolaritiesArray[nIndex] = gcnew System::String(pDevicePinConfigValues->ppszPolarities[nIndex]);
	    }
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
