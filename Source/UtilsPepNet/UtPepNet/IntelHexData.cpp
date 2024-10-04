/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2024 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "Stdafx.h"

#include "IntelHexData.h"

Pep::Programmer::IntelHexData::IntelHexData()
{
}

Pep::Programmer::IntelHexData::IntelHexData(
  _In_ UINT32 nTotalPages,
  _In_ UINT32* pnAddress,
  _In_ const UINT8** ppData,
  _In_ UINT32* pnDataLen) :
  _In_ m_nTotalPages(nTotalPages)
{
	array<System::Byte>^ DataArray;
	pin_ptr<System::Byte> pbyData;

	m_PageAddressArray = gcnew array<System::UInt32>(nTotalPages);
	m_PageDataArray = gcnew array<array<System::Byte>^>(nTotalPages);

	for (UINT32 nIndex = 0; nIndex < nTotalPages; ++nIndex)
	{
		m_PageAddressArray[nIndex] = pnAddress[nIndex];

		DataArray = gcnew array<System::Byte>(pnDataLen[nIndex]);

		m_PageDataArray[nIndex] = DataArray;

		pbyData = &DataArray[0];

		::CopyMemory(pbyData, ppData[nIndex], pnDataLen[nIndex]);
	}
}

Pep::Programmer::IntelHexData::~IntelHexData()
{
	m_PageAddressArray = nullptr;
	m_PageDataArray = nullptr;
}

System::UInt32 Pep::Programmer::IntelHexData::PageAddress(
  System::UInt32 nPage)
{
	if (nPage >= m_nTotalPages)
	{
		throw gcnew System::Exception(L"Page out of range");
	}

	return m_PageAddressArray[nPage];
}

array<System::Byte>^ Pep::Programmer::IntelHexData::PageData(
  System::UInt32 nPage)
{
	if (nPage >= m_nTotalPages)
	{
		throw gcnew System::Exception(L"Page out of range");
	}

	return m_PageDataArray[nPage];
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2019-2024 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
