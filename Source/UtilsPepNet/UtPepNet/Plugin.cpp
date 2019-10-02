/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "Stdafx.h"
#include "Plugin.h"

Pep::Programmer::Plugin::Plugin(
  _In_ LPCWSTR pszName,
  _In_ WORD wProductMajorVersion,
  _In_ WORD wProductMinorVersion,
  _In_ WORD wProductBuildVersion,
  _In_ WORD wProductPrivateVersion)
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
//  Copxright (C) 2007-2019 Kevin Eshbach	
/////////////////////////////////////-///////////////////////////////////////
