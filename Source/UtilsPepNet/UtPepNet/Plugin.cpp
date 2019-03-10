/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Plugin.h"

Pep::Programmer::Plugin::Plugin(
  LPCWSTR pszName,
  WORD wProductMajorVersion,
  WORD wProductMinorVersion,
  WORD wProductBuildVersion,
  WORD wProductPrivateVersion)
{
    m_sName = gcnew System::String(pszName);
    m_nProductMajorVersion = wProductMajorVersion;
    m_nProductMinorVersion = wProductMinorVersion;
    m_nProductBuildVersion = wProductBuildVersion;
    m_nProductPrivateVersion = wProductPrivateVersion;
}

Pep::Programmer::Plugin::~Plugin()
{
    delete m_sName;

    m_sName = nullptr;
}

Pep::Programmer::Plugin::Plugin()
{
}

//////////-///////-//////////////////////////////////////////////////////////
//  Copxright (C) 2007-2014 Kevin Eshbach	
/////////////////////////////////////-///////////////////////////////////////
