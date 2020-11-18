/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "PepAppHostNetAppDomainManager.h"

Pep::Application::PepAppHostNetAppDomainManager::PepAppHostNetAppDomainManager()
{
}

void Pep::Application::PepAppHostNetAppDomainManager::InitializeNewDomain(
  System::AppDomainSetup^ AppDomainInfo)
{
	AppDomainInfo;
}

System::Object^ Pep::Application::PepAppHostNetAppDomainManager::InitializeLifetimeService()
{
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
