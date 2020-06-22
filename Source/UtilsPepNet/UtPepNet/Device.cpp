/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "Stdafx.h"

#include <UtilsDevice/UtPepDevices.h>

#include <Includes/UtMacros.h>

#include "Device.h"

#include "IDeviceIO.h"

#include "PinConfigValues.h"
#include "PinConfig.h"

#include "ROMData.h"
#include "RAMData.h"
#include "PALData.h"

Pep::Programmer::Device::Device(
  _In_ const TDevice* pDevice)
{
    ULONG ulTotalDipSwitches = MArrayLen(pDevice->bDipSwitches);
	LPCWSTR pszPinDiagram;

    m_sName                    = gcnew System::String(pDevice->pszName);
    m_sDeviceType              = gcnew System::String(UtPepDevicesGetDeviceTypeName(pDevice->DeviceType));
	m_sDevicePackage           = gcnew System::String(UtPepDevicesGetDevicePackageName(pDevice->DevicePackage));
    m_nPinCount                = pDevice->nPinCount;
	m_sPinNames                = gcnew array<System::String^>(pDevice->nPinCount);
    m_bDipSwitches             = gcnew array<System::Boolean>(ulTotalDipSwitches);
	m_nChipEnableNanoseconds   = pDevice->nChipEnableNanoseconds;
	m_nOutputEnableNanoseconds = pDevice->nOutputEnableNanoseconds;

    if (pDevice->pszAdapter)
    {
        m_sAdapter = gcnew System::String(pDevice->pszAdapter);
    }
    else
    {
        m_sAdapter = nullptr;
    }

    if (pDevice->pszMessage)
    {
        m_sMessage = gcnew System::String(pDevice->pszMessage);
    }
    else
    {
        m_sMessage = nullptr;
    }

	for (ULONG ulIndex = 0; ulIndex < pDevice->nPinCount; ++ulIndex)
	{
		m_sPinNames[ulIndex] = gcnew System::String(pDevice->ppszPinNames[ulIndex]);
	}

    for (ULONG ulIndex = 0; ulIndex < ulTotalDipSwitches; ++ulIndex)
    {
        m_bDipSwitches[ulIndex] = pDevice->bDipSwitches[ulIndex] ? true : false;
    }

    switch (pDevice->DeviceType)
    {
        case edtEPROM:
        case edtEPROMWithIO:
        case edtPROM:
			m_pDeviceData = gcnew Pep::Programmer::ROMData(&pDevice->DeviceData.ROMData,
                                                           pDevice->DeviceData.pInitFunc,
                                                           pDevice->DeviceData.pUninitFuncs);
            break;
        case edtSRAM:
			m_pDeviceData = gcnew Pep::Programmer::RAMData(&pDevice->DeviceData.RAMData,
                                                           pDevice->DeviceData.pInitFunc,
                                                           pDevice->DeviceData.pUninitFuncs);
            break;
        case edtPAL:
			m_pDeviceData = gcnew Pep::Programmer::PALData(&pDevice->DeviceData.PALData,
                                                           pDevice->DeviceData.pInitFunc,
                                                           pDevice->DeviceData.pUninitFuncs,
                                                           pDevice->pszName,
                                                           pDevice->nPinCount);
            break;
        default:
            System::Diagnostics::Debug::Assert(false, L"Unknown Device Data");

            m_pDeviceData = nullptr;
            break;
    }

    pszPinDiagram = UtPepDevicesAllocPinDiagram(pDevice->DevicePackage,
                                                pDevice->ppszPinNames,
                                                pDevice->nPinCount);

    if (pszPinDiagram)
    {
        m_sPinNamesDiagram = gcnew System::String(pszPinDiagram);

		UtPepDevicesFreePinDiagram(pszPinDiagram);
    }
}

Pep::Programmer::Device::~Device()
{
    delete m_sName;
    delete m_sDeviceType;
    delete m_sDevicePackage;
    delete m_sPinNames;
    delete m_sPinNamesDiagram;
    delete m_sAdapter;
    delete m_sMessage;
    delete m_bDipSwitches;
    delete m_pDeviceData;

    m_sName = nullptr;
    m_sDeviceType = nullptr;
    m_sDevicePackage = nullptr;
    m_sPinNames = nullptr;
    m_sPinNamesDiagram = nullptr;
    m_sAdapter = nullptr;
    m_sMessage = nullptr;
    m_bDipSwitches = nullptr;
	m_pDeviceData = nullptr;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
