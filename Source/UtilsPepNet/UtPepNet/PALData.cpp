/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "Stdafx.h"

#include <UtilsDevice/UtPepDevices.h>

#include "IDeviceIO.h"

#include "PinConfigValues.h"
#include "PinConfig.h"

#include "PALData.h"
#include "UtDeviceIO.h"

#include <Utils/UtHeap.h>

#include <UtilsDevice/UtPAL.h>

#include <Includes/UtMacros.h>

#pragma region "Local Functions"

static TDevicePinConfig* lAllocDevicePinConfigs(
  _In_ TDevicePinConfigValues* pDevicePinConfigValues,
  _In_ UINT nDevicePinConfigValuesCount,
  _In_ array<Pep::Programmer::PinConfig^>^ PinConfigArray)
{
    TDevicePinConfig* pDevicePinConfigs = NULL;
    TDevicePinConfigValues* pTmpDevicePinConfigValues;

    System::Diagnostics::Debug::Assert(nDevicePinConfigValuesCount == (UINT)PinConfigArray->Length,
                                       L"Config Pin Data not the same length as the device pin config values.");

    pDevicePinConfigs = (TDevicePinConfig*)UtAllocMem(sizeof(TDevicePinConfig) * PinConfigArray->Length);

    for (System::Int32 nIndex = 0; nIndex < PinConfigArray->Length; ++nIndex)
    {
        pDevicePinConfigs[nIndex].nPin = PinConfigArray[nIndex]->Pin;
        pDevicePinConfigs[nIndex].pszType = NULL;
        pDevicePinConfigs[nIndex].pszPolarity = NULL;

        pTmpDevicePinConfigValues = &pDevicePinConfigValues[nIndex];

        for (UINT nTypeIndex = 0;
             nTypeIndex < pTmpDevicePinConfigValues->nTypesCount;
             ++nTypeIndex)
        {
            if (0 == System::String::Compare(PinConfigArray[nIndex]->Type,
                                             gcnew System::String(pTmpDevicePinConfigValues->ppszTypes[nTypeIndex]),
                                             true))
            {
                pDevicePinConfigs[nIndex].pszType = pTmpDevicePinConfigValues->ppszTypes[nTypeIndex];
            }
        }

        for (UINT nPolarityIndex = 0;
             nPolarityIndex < pTmpDevicePinConfigValues->nPolaritiesCount;
             ++nPolarityIndex)
        {
            if (0 == System::String::Compare(PinConfigArray[nIndex]->Polarity,
                                             gcnew System::String(pTmpDevicePinConfigValues->ppszPolarities[nPolarityIndex]),
                                             true))
            {
                pDevicePinConfigs[nIndex].pszPolarity = pTmpDevicePinConfigValues->ppszPolarities[nPolarityIndex];
            }
        }
    }

    return pDevicePinConfigs;
}

inline static void lFreeDevicePinConfigs(
  _In_ TDevicePinConfig* pDevicePinConfig)
{
    UtFreeMem(pDevicePinConfig);
}

#pragma endregion

Pep::Programmer::PALData::PALData(
  _In_ const TPALData* pPALData,
  _In_ TUtPepDevicesInitFunc pInitDeviceFunc,
  _In_ TUtPepDevicesUninitFunc pUninitDeviceFunc,
  _In_ LPCWSTR pszDeviceName,
  _In_ UINT nPinCount,
  _In_ UINT32 nChipEnableNanoseconds,
  _In_ UINT32 nOutputEnableNanoseconds) :
  m_pInitDeviceFunc(pInitDeviceFunc),
  m_pUninitDeviceFunc(pUninitDeviceFunc),
  m_pDevicePinConfigValues(pPALData->pDevicePinConfigValues),
  m_nDevicePinConfigValuesCount(pPALData->nDevicePinConfigValuesCount),
  m_pDevicePinFuseRows(pPALData->pDevicePinFuseRows),
  m_nDevicePinFuseRowsCount(pPALData->nDevicePinFuseRowsCount),
  m_pDevicePinFuseColumns(pPALData->pDevicePinFuseColumns),
  m_nDevicePinFuseColumnsCount(pPALData->nDevicePinFuseColumnsCount),
  m_pGetFuseMapSizeFunc(pPALData->pGetFuseMapSizeFunc),
  m_pConfigFuseMapFunc(pPALData->pConfigFuseMapFunc),
  m_pSetDevicePinConfigsFunc(pPALData->pSetDevicePinConfigsFunc),
  m_pAllocFuseMapTextFunc(pPALData->pAllocFuseMapTextFunc),
  m_pFreeFuseMapTextFunc(pPALData->pFreeFuseMapTextFunc),
  m_pAllocSampleFuseMapTextFunc(pPALData->pAllocSampleFuseMapTextFunc),
  m_pFreeSampleFuseMapTextFunc(pPALData->pFreeSampleFuseMapTextFunc),
  m_pReadDeviceFunc(pPALData->pReadDeviceFunc),
  m_pszDeviceName(pszDeviceName),
  m_nPinCount(nPinCount),
  m_nFuseMapSize(0),
  m_nChipEnableNanoseconds(nChipEnableNanoseconds),
  m_nOutputEnableNanoseconds(nOutputEnableNanoseconds)
{
	ULONG ulFuseMapSize = 0;

	UtPALGetFuseMapSize(pPALData, &ulFuseMapSize);

	m_nFuseMapSize = ulFuseMapSize;

	m_PinConfigValuesArray = gcnew array<Pep::Programmer::PinConfigValues^>(pPALData->nDevicePinConfigValuesCount);

	for (UINT nIndex = 0; nIndex < pPALData->nDevicePinConfigValuesCount; ++nIndex)
	{
		m_PinConfigValuesArray[nIndex] = gcnew Pep::Programmer::PinConfigValues(&pPALData->pDevicePinConfigValues[nIndex]);
	}

    if (m_pInitDeviceFunc && m_pUninitDeviceFunc)
    {
        if (m_pSetDevicePinConfigsFunc && m_pReadDeviceFunc)
        {
			m_ReadDeviceDelegate = gcnew ReadDeviceDelegate(this, &Pep::Programmer::PALData::ReadDevice);
        }
        else
        {
            m_ReadDeviceDelegate = nullptr;
        }
    }
    else
    {
        m_ReadDeviceDelegate = nullptr;
    }
}

Pep::Programmer::PALData::~PALData()
{
    delete m_PinConfigValuesArray;
    delete m_ReadDeviceDelegate;

    m_PinConfigValuesArray = nullptr;
    m_ReadDeviceDelegate = nullptr;
}

System::Boolean Pep::Programmer::PALData::WriteJEDFile(
  System::String^ sFile,
  array<System::Byte>^ byData)
{
    pin_ptr<const wchar_t> pszFile = PtrToStringChars(sFile);
    pin_ptr<System::Byte> pbyData = &byData[0];
	TPALData PALData;

	InitPALData(&PALData);

	return UtPALWriteFuseMapToJEDFile(&PALData, pbyData, byData->Length,
                                      m_pszDeviceName, m_nPinCount, pszFile) ? true : false;
}

System::Boolean Pep::Programmer::PALData::WriteJEDText(
  array<System::Byte>^ byData,
  System::String^% sText)
{
    pin_ptr<System::Byte> pbyData = &byData[0];
    LPWSTR pszText;
	ULONG ulTextLen;
	TPALData PALData;

    sText = L"";

	InitPALData(&PALData);

	if (UtInitHeap() == FALSE)
	{
		return false;
	}

	if (FALSE == UtPALFuseMapText(&PALData, pbyData, byData->Length,
                                  m_pszDeviceName, m_nPinCount, NULL, &ulTextLen))
	{
		UtUninitHeap();

		return false;
	}

	pszText = (LPWSTR)UtAllocMem(ulTextLen * sizeof(WCHAR));

	if (pszText == NULL)
	{
		UtUninitHeap();

		return false;
	}

	if (FALSE == UtPALFuseMapText(&PALData, pbyData, byData->Length,
                                  m_pszDeviceName, m_nPinCount, pszText, &ulTextLen))
	{
		UtFreeMem(pszText);

		UtUninitHeap();

		return false;
	}

    sText = gcnew System::String(pszText);

	UtFreeMem(pszText);

	UtUninitHeap();

    return true;
}

void Pep::Programmer::PALData::ReadDevice(
  _In_ Pep::Programmer::IDeviceIO^ pDeviceIO,
  _In_ array<System::Byte>^ byData,
  _In_ array<Pep::Programmer::PinConfig^>^ PinConfigArray)
{
	pin_ptr<System::Byte> pbyData = &byData[0];
	TDeviceIOFuncs* pDeviceIOFuncs = Pep::Programmer::UtDeviceIO::GetDeviceIOFuncs();
	TDevicePinConfig* pDevicePinConfig;

	if (UtInitHeap() == FALSE)
	{
		return;
	}

	pDevicePinConfig = lAllocDevicePinConfigs(m_pDevicePinConfigValues,
                                              m_nDevicePinConfigValuesCount,
                                              PinConfigArray);

	if (pDevicePinConfig)
	{
		Pep::Programmer::UtDeviceIO::SetCurrentDeviceIO(pDeviceIO);

		if (m_pInitDeviceFunc())
		{
			if (m_pSetDevicePinConfigsFunc(pDevicePinConfig, PinConfigArray->Length))
			{
				m_pReadDeviceFunc(pDeviceIOFuncs, m_nChipEnableNanoseconds,
					              m_nOutputEnableNanoseconds, pbyData,
					              byData->Length);
			}

			m_pUninitDeviceFunc();
		}

		lFreeDevicePinConfigs(pDevicePinConfig);
	}

	UtUninitHeap();
}

void Pep::Programmer::PALData::InitPALData(
  _In_ TPALData* pPALData)
{
	pPALData->pDevicePinConfigValues = m_pDevicePinConfigValues;
	pPALData->nDevicePinConfigValuesCount = m_nDevicePinConfigValuesCount;
	pPALData->pDevicePinFuseRows = m_pDevicePinFuseRows;
	pPALData->nDevicePinFuseRowsCount = m_nDevicePinFuseRowsCount;
	pPALData->pDevicePinFuseColumns = m_pDevicePinFuseColumns;
	pPALData->nDevicePinFuseColumnsCount = m_nDevicePinFuseColumnsCount;
	pPALData->pGetFuseMapSizeFunc = m_pGetFuseMapSizeFunc;
	pPALData->pConfigFuseMapFunc = m_pConfigFuseMapFunc;
	pPALData->pSetDevicePinConfigsFunc = m_pSetDevicePinConfigsFunc;
	pPALData->pAllocFuseMapTextFunc = m_pAllocFuseMapTextFunc;
	pPALData->pFreeFuseMapTextFunc = m_pFreeFuseMapTextFunc;
	pPALData->pAllocSampleFuseMapTextFunc = m_pAllocSampleFuseMapTextFunc;
	pPALData->pFreeSampleFuseMapTextFunc = m_pFreeSampleFuseMapTextFunc;
	pPALData->pReadDeviceFunc = m_pReadDeviceFunc;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
