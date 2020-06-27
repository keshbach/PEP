/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "Stdafx.h"

#include <UtilsDevice/UtPepDevices.h>

#include "IDeviceIO.h"

#include "ROMData.h"
#include "UtDeviceIO.h"

Pep::Programmer::ROMData::ROMData()
{
}

Pep::Programmer::ROMData::~ROMData()
{
    delete m_sDeviceVpp;
    delete m_ReadDeviceDelegate;
    delete m_ProgramDeviceDelegate;
    delete m_VerifyDeviceDelegate;

    m_sDeviceVpp = nullptr;
    m_ReadDeviceDelegate = nullptr;
    m_ProgramDeviceDelegate = nullptr;
    m_VerifyDeviceDelegate = nullptr;
}

Pep::Programmer::ROMData::ROMData(
  _In_ const TROMData* pROMData,
  _In_ TUtPepDevicesInitFunc pInitDeviceFunc,
  _In_ TUtPepDevicesUninitFunc pUninitDeviceFunc,
  _In_ UINT32 nChipEnableNanoseconds,
  _In_ UINT32 nOutputEnableNanoseconds) :
  m_pReadDeviceFunc(pROMData->pReadDeviceFunc),
  m_pProgramDeviceFunc(pROMData->pProgramDeviceFunc),
  m_pVerifyDeviceFunc(pROMData->pVerifyDeviceFunc),
  m_pInitDeviceFunc(pInitDeviceFunc),
  m_pUninitDeviceFunc(pUninitDeviceFunc)
{
    m_nSize                    = pROMData->nSize;
    m_nBitsPerValue            = pROMData->nBitsPerValue;
    m_sDeviceVpp               = gcnew System::String(UtPepDevicesGetDeviceVppName(pROMData->DeviceVpp));
	m_nChipEnableNanoseconds   = nChipEnableNanoseconds;
	m_nOutputEnableNanoseconds = nOutputEnableNanoseconds;

    if (m_pInitDeviceFunc && m_pUninitDeviceFunc)
    {
        if (m_pReadDeviceFunc)
        {
			m_ReadDeviceDelegate = gcnew ReadDeviceDelegate(this, &Pep::Programmer::ROMData::ReadDevice);
        }
        else
        {
            m_ReadDeviceDelegate = nullptr;
        }

        if (m_pProgramDeviceFunc)
        {
			m_ProgramDeviceDelegate = gcnew ProgramDeviceDelegate(this, &Pep::Programmer::ROMData::ProgramDevice);
        }
        else
        {
            m_ProgramDeviceDelegate = nullptr;
        }

        if (m_pVerifyDeviceFunc)
        {
			m_VerifyDeviceDelegate = gcnew VerifyDeviceDelegate(this, &Pep::Programmer::ROMData::VerifyDevice);
        }
        else
        {
            m_VerifyDeviceDelegate = nullptr;
        }
    }
    else
    {
        m_ReadDeviceDelegate = nullptr;
        m_ProgramDeviceDelegate = nullptr;
        m_VerifyDeviceDelegate = nullptr;
    }
}

void Pep::Programmer::ROMData::ReadDevice(
  Pep::Programmer::IDeviceIO^ pDeviceIO,
  array<System::Byte>^ byData)
{
    pin_ptr<System::Byte> pbyData = &byData[0];
	TDeviceIOFuncs* pDeviceIOFuncs = Pep::Programmer::UtDeviceIO::GetDeviceIOFuncs();

	Pep::Programmer::UtDeviceIO::SetCurrentDeviceIO(pDeviceIO);

	if (m_pInitDeviceFunc())
	{
		m_pReadDeviceFunc(pDeviceIOFuncs, m_nChipEnableNanoseconds,
			              m_nOutputEnableNanoseconds, 
			              pbyData, byData->Length);

		m_pUninitDeviceFunc();
	}
}

void Pep::Programmer::ROMData::ProgramDevice(
  Pep::Programmer::IDeviceIO^ pDeviceIO,
  array<System::Byte>^ byData)
{
    pin_ptr<System::Byte> pbyData = &byData[0];
	TDeviceIOFuncs* pDeviceIOFuncs = Pep::Programmer::UtDeviceIO::GetDeviceIOFuncs();

	Pep::Programmer::UtDeviceIO::SetCurrentDeviceIO(pDeviceIO);

	if (m_pInitDeviceFunc())
	{
		m_pProgramDeviceFunc(pDeviceIOFuncs, m_nChipEnableNanoseconds,
			                 m_nOutputEnableNanoseconds,
			                 pbyData, byData->Length);

		m_pUninitDeviceFunc();
	}
}

void Pep::Programmer::ROMData::VerifyDevice(
  Pep::Programmer::IDeviceIO^ pDeviceIO,
  array<System::Byte>^ byData)
{
    pin_ptr<System::Byte> pbyData = &byData[0];
	TDeviceIOFuncs* pDeviceIOFuncs = Pep::Programmer::UtDeviceIO::GetDeviceIOFuncs();

	Pep::Programmer::UtDeviceIO::SetCurrentDeviceIO(pDeviceIO);

	if (m_pInitDeviceFunc())
	{
		m_pVerifyDeviceFunc(pDeviceIOFuncs, m_nChipEnableNanoseconds,
			                m_nOutputEnableNanoseconds,
			                pbyData, byData->Length);

		m_pUninitDeviceFunc();
	}
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2020 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
