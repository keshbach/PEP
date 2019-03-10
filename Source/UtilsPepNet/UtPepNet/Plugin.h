/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
 
#pragma once

namespace Pep
{
	namespace Programmer
	{
		public ref class Plugin sealed
		{
		public:
			property System::String^ Name
			{
				System::String^ get()
				{
					return m_sName;
				}
			}

			property System::UInt16 ProductMajorVersion
			{
				System::UInt16 get()
				{
					return m_nProductMajorVersion;
				}
			}

			property System::UInt16 ProductMinorVersion
			{
				System::UInt16 get()
				{
					return m_nProductMinorVersion;
				}
			}

			property System::UInt16 ProductBuildVersion
			{
				System::UInt16 get()
				{
					return m_nProductBuildVersion;
				}
			}

			property System::UInt16 ProductPrivateVersion
			{
				System::UInt16 get()
				{
					return m_nProductPrivateVersion;
				}
			}

		internal:
			Plugin(LPCWSTR pszName,
					WORD wProductMajorVersion,
					WORD wProtuctMinorVersion,
					WORD wProductBuildVersion,
					WORD wProductPrivateVersion);
			~Plugin();

		private:
			System::String^ m_sName;
			System::UInt16 m_nProductMajorVersion;
			System::UInt16 m_nProductMinorVersion;
			System::UInt16 m_nProductBuildVersion;
			System::UInt16 m_nProductPrivateVersion;

		private:
			Plugin();
		};
	}
};

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
