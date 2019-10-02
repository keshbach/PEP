/////////////////////////////////////////////////////////////////////////////
//  Copyrig(t (C) 2007-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "Stdafx.h"

#include <UtilsDevice/UtPepDevices.h>

#include "IDeviceIO.h"

#include "RAMData.h"
#include "UtDeviceIO.h"

Pep::Programmer::RAMData::RAMData()
{
}

Pep::Programmer::RAMData::~RAMData()
{
    delete m_WriteDeviceDelegate;
    delete m_VerifyDeviceDelegate;

    m_WriteDeviceDelegate = nullptr;
    m_VerifyDeviceDelegate = nullptr;
}

Pep::Programmer::RAMData::RAMData(
  _In_ const TRAMData* pRAMData,
  _In_ TUtPepDevicesInitFunc pInitDeviceFunc,
  _In_ TUtPepDevicesUninitFunc pUninitDeviceFunc) :
  m_pWriteDeviceFunc(pRAMData->pWriteDeviceFunc),
  m_pVerifyDeviceFunc(pRAMData->pVerifyDeviceFunc),
  m_pInitDeviceFunc(pInitDeviceFunc),
  m_pUninitDeviceFunc(pUninitDeviceFunc)
{
    m_nSize         = pRAMData->nSize;
    m_nBitsPerValue = pRAMData->nBitsPerValue;

    if (m_pInitDeviceFunc && m_pUninitDeviceFunc)
    {
        if (m_pWriteDeviceFunc)
        {
			m_WriteDeviceDelegate = gcnew WriteDeviceDelegate(this, &Pep::Programmer::RAMData::WriteDevice);
        }
        else
        {
            m_WriteDeviceDelegate = nullptr;
        }

        if (m_pVerifyDeviceFunc)
        {
			m_VerifyDeviceDelegate = gcnew VerifyDeviceDelegate(this, &Pep::Programmer::RAMData::VerifyDevice);
        }
        else
        {
            m_VerifyDeviceDelegate = nullptr;
        }
    }
    else
    {
        m_WriteDeviceDelegate = nullptr;
        m_VerifyDeviceDelegate = nullptr;
    }
}

void Pep::Programmer::RAMData::WriteDevice(
  Pep::Programmer::IDeviceIO^ pDeviceIO,
  array<System::Byte>^ byData)
{
    pin_ptr<System::Byte> pbyData = &byData[0];
	TDeviceIOFuncs* pDeviceIOFuncs = Pep::Programmer::UtDeviceIO::GetDeviceIOFuncs();

	Pep::Programmer::UtDeviceIO::SetCurrentDeviceIO(pDeviceIO);

	if (m_pInitDeviceFunc())
	{
		m_pWriteDeviceFunc(pDeviceIOFuncs, pbyData, byData->Length);

		m_pUninitDeviceFunc();
	}
}

void Pep::Programmer::RAMData::VerifyDevice(
  Pep::Programmer::IDeviceIO^ pDeviceIO,
  array<System::Byte>^ byData)
{
    pin_ptr<System::Byte> pbyData = &byData[0];
	TDeviceIOFuncs* pDeviceIOFuncs = Pep::Programmer::UtDeviceIO::GetDeviceIOFuncs();

	Pep::Programmer::UtDeviceIO::SetCurrentDeviceIO(pDeviceIO);

	if (m_pInitDeviceFunc())
	{
		m_pVerifyDeviceFunc(pDeviceIOFuncs, pbyData, byData->Length);

		m_pUninitDeviceFunc();
	}
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
