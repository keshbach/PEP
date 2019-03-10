/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "PALData.h"
#include "UtDeviceIO.h"

#include <Utils/UtHeap.h>

#include <UtilsDevice/UtPAL.h>

#include <Includes/UtMacros.h>

#pragma unmanaged
static LPVOID lUnmanagedUtAllocMem(
  ULONG ulBytes)
{
    return UtAllocMem(ulBytes);
}

static TPALData* lUnmanagedAllocPALData(
  const TPALData* pPALData)
{
    TPALData* pNewPALData = (TPALData*)UtAllocMem(sizeof(TPALData));

    if (pNewPALData)
    {
        ::CopyMemory(pNewPALData, pPALData, sizeof(TPALData));
    }

    return pNewPALData;
}

static VOID lUnmanagedFreePALData(
  TPALData* pPALData)
{
    UtFreeMem(pPALData);
}

static BOOL lUnmanagedFreeDevicePinConfigs(
  TDevicePinConfig* pDevicePinConfigs)
{
    if (pDevicePinConfigs)
    {
        UtFreeMem(pDevicePinConfigs);
    }

    return TRUE;
}

static VOID lUnmanagedReadPALDevice(
  LPBYTE pbyData,
  ULONG ulDataLen,
  TDevicePinConfig* pDevicePinConfigs,
  ULONG ulTotalDevicePinConfigs,
  TDeviceIOFuncs* pDeviceIOFuncs,
  TUtPepDevicesSetDevicePinConfigsFunc pSetDevicePinConfigsFunc,
  TUtPepDevicesReadDeviceFunc pReadDeviceFunc,
  TUtPepDevicesInitFunc pInitDeviceFunc,
  TUtPepDevicesUninitFunc pUninitDeviceFunc)
{
    if (pInitDeviceFunc())
    {
        if (pSetDevicePinConfigsFunc(pDevicePinConfigs, ulTotalDevicePinConfigs))
        {
            pReadDeviceFunc(pDeviceIOFuncs, pbyData, ulDataLen);
        }

        pUninitDeviceFunc();
    }
}

static BOOL lUnmanagedWriteJEDFile(
  TPALData* pPALData,
  LPBYTE pbyData,
  ULONG ulDataLen,
  LPCWSTR pszDeviceName,
  UINT nPinCount,
  LPCWSTR pszFile)
{
    return UtPALWriteFuseMapToJEDFile(pPALData, pbyData, ulDataLen,
                                      pszDeviceName, nPinCount, pszFile);
}

static LPWSTR lUnmanagedAllocJEDFile(
  TPALData* pPALData,
  LPBYTE pbyData,
  ULONG ulDataLen,
  LPCWSTR pszDeviceName,
  UINT nPinCount)
{
    LPWSTR pszText;
    ULONG ulTextLen;
    BOOL bResult;

    bResult = UtPALFuseMapText(pPALData, pbyData, ulDataLen,
                               pszDeviceName, nPinCount, NULL, &ulTextLen);

    if (bResult == FALSE)
    {
        return NULL;
    }

    pszText = (LPWSTR)lUnmanagedUtAllocMem(ulTextLen * sizeof(WCHAR));

    if (pszText == NULL)
    {
        return NULL;
    }

    bResult = UtPALFuseMapText(pPALData, pbyData, ulDataLen,
                               pszDeviceName, nPinCount, pszText,
                               &ulTextLen);

    return pszText;
}

static VOID lUnmanagedFreeJEDFile(
  LPWSTR pszText)
{
    UtFreeMem(pszText);
}

static ULONG lUnmanagedGetFuseMapSize(
  const TPALData* pPALData)
{
    ULONG ulFuseMapSize = 0;

    UtPALGetFuseMapSize(pPALData, &ulFuseMapSize);

    return ulFuseMapSize;
}
#pragma managed

static TDevicePinConfig* lAllocDevicePinConfigs(
  TDevicePinConfigValues* pDevicePinConfigValues,
  UINT nDevicePinConfigValuesCount,
  array<Pep::Programmer::PinConfig^>^ PinConfigArray)
{
    TDevicePinConfig* pDevicePinConfigs = NULL;
    TDevicePinConfigValues* pTmpDevicePinConfigValues;

    System::Diagnostics::Debug::Assert(nDevicePinConfigValuesCount == (UINT)PinConfigArray->Length,
                                       L"Config Pin Data not the same length as the device pin config values.");

    pDevicePinConfigs = (TDevicePinConfig*)lUnmanagedUtAllocMem(sizeof(TDevicePinConfig) * PinConfigArray->Length);

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

Pep::Programmer::PALData::PALData(
  const TPALData* pPALData,
  TUtPepDevicesInitFunc pInitDeviceFunc,
  TUtPepDevicesUninitFunc pUninitDeviceFunc,
  LPCWSTR pszDeviceName,
  UINT nPinCount) :
  m_pSetDevicePinConfigsFunc(pPALData->pSetDevicePinConfigsFunc),
  m_pReadDeviceFunc(pPALData->pReadDeviceFunc),
  m_pInitDeviceFunc(pInitDeviceFunc),
  m_pUninitDeviceFunc(pUninitDeviceFunc),
  m_pPALData(NULL),
  m_pszDeviceName(pszDeviceName),
  m_nPinCount(nPinCount),
  m_nFuseMapSize(0)
{
    m_pPALData = lUnmanagedAllocPALData(pPALData);

    m_nFuseMapSize = lUnmanagedGetFuseMapSize(pPALData);

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

    this->!PALData();
}

Pep::Programmer::PALData::!PALData()
{
    Close();
}

void Pep::Programmer::PALData::ReadDevice(
  Pep::Programmer::IDeviceIO^ pDeviceIO,
  array<System::Byte>^ byData,
  array<Pep::Programmer::PinConfig^>^ PinConfigArray)
{
    pin_ptr<System::Byte> pbyData = &byData[0];
    TDeviceIOFuncs* pDeviceIOFuncs = Pep::Programmer::UtDeviceIO::GetDeviceIOFuncs();
    TDevicePinConfig* pDevicePinConfigs;

    pDevicePinConfigs = lAllocDevicePinConfigs(m_pPALData->pDevicePinConfigValues,
                                               m_pPALData->nDevicePinConfigValuesCount,
                                               PinConfigArray);

	Pep::Programmer::UtDeviceIO::SetCurrentDeviceIO(pDeviceIO);

    lUnmanagedReadPALDevice(pbyData, byData->Length, pDevicePinConfigs,
                            PinConfigArray->Length, pDeviceIOFuncs,
                            m_pSetDevicePinConfigsFunc,
                            m_pReadDeviceFunc,
                            m_pInitDeviceFunc,
                            m_pUninitDeviceFunc);

    lUnmanagedFreeDevicePinConfigs(pDevicePinConfigs);
}

System::Boolean Pep::Programmer::PALData::WriteJEDFile(
  System::String^ sFile,
  array<System::Byte>^ byData)
{
    pin_ptr<const wchar_t> pszFile = PtrToStringChars(sFile);
    pin_ptr<System::Byte> pbyData = &byData[0];
    BOOL bResult;

    bResult = lUnmanagedWriteJEDFile(m_pPALData, pbyData, byData->Length,
                                     m_pszDeviceName, m_nPinCount, pszFile);

    return (bResult == TRUE) ? true : false;
}

System::Boolean Pep::Programmer::PALData::WriteJEDText(
  array<System::Byte>^ byData,
  System::String^% sText)
{
    pin_ptr<System::Byte> pbyData = &byData[0];
    LPWSTR pszText;

    sText = L"";

    pszText = lUnmanagedAllocJEDFile(m_pPALData, pbyData, byData->Length,
                                     m_pszDeviceName, m_nPinCount);

    if (pszText == NULL)
    {
        return false;
    }

    sText = gcnew System::String(pszText);

    lUnmanagedFreeJEDFile(pszText);

    return true;
}

void Pep::Programmer::PALData::Close()
{
    if (m_pPALData)
    {
        lUnmanagedFreePALData(m_pPALData);

        m_pPALData = NULL;
    }
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
