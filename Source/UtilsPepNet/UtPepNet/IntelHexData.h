/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2024 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Pep
{
	namespace Programmer
	{
		public ref class IntelHexData sealed
		{
		public:
			property System::UInt32 TotalPages
			{
				System::UInt32 get()
				{
					return m_nTotalPages;
				}
			}

		public:
			System::UInt32 PageAddress(System::UInt32 nPage);
			array<System::Byte>^ PageData(System::UInt32 nPage);

		internal:
			IntelHexData(_In_ UINT32 nTotalPages,
                         _In_ UINT32* pnAddress,
				         _In_ const UINT8** ppData,
				         _In_ UINT32* pnDataLen);
			~IntelHexData();

		private:
			System::UInt32 m_nTotalPages;
			array<System::UInt32>^ m_PageAddressArray;
			array<array<System::Byte>^>^ m_PageDataArray;

		private:
			IntelHexData();
		};
	}
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2024 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
