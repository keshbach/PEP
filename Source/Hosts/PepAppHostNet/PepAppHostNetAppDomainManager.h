/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

using namespace System;
//using namespace System::Reflection;
//using namespace System::Runtime::CompilerServices;
using namespace System::Runtime::InteropServices;
using namespace System::Security::Permissions;
using namespace System::Security;

[assembly:System::Security::AllowPartiallyTrustedCallersAttribute()];

namespace Pep
{
	namespace Application
	{
		[ComVisibleAttribute(true),
			GuidAttribute("B3171751-407E-4B8E-9792-2E8EE6128B0D"),
		    ClassInterfaceAttribute(ClassInterfaceType::None)]
		[SecuritySafeCriticalAttribute]
//[SecurityPermissionAttribute(SecurityAction::LinkDemand, Flags = SecurityPermissionFlag::Infrastructure)]
//[SecurityPermissionAttribute(SecurityAction::InheritanceDemand, Flags = SecurityPermissionFlag::Infrastructure)]
		public ref class PepAppHostNetAppDomainManager : System::AppDomainManager
		{
		public:
			PepAppHostNetAppDomainManager();

		public:
			void InitializeNewDomain(System::AppDomainSetup^ AppDomainInfo) override;

			System::Object^ InitializeLifetimeService() override;
		};
	}
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
