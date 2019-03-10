/////////////////////////////////////////////////////////////////////////////
//  Copyrig(t (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "RAMData.h"
#include "UtDeviceIO.h"

#pragma unmanaged
static VOID lWriteRAMDevice(
  LPBYTE pbyData,
  ULONG ulDataLen,
  TDeviceIOFuncs* pDeviceIOFuncs,
  TUtPepDevicesWriteDeviceFunc pWriteDeviceFunc,
  TUtPepDevicesInitFunc pInitDeviceFunc,
  TUtPepDevicesUninitFunc pUninitDeviceFunc)
{
    if (pInitDeviceFunc())
    {
        pWriteDeviceFunc(pDeviceIOFuncs, pbyData, ulDataLen);

        pUninitDeviceFunc();
    }
}

static VOID lVerifyRAMDevice(
  LPBYTE pbyData,
  ULONG ulDataLen,
  TDeviceIOFuncs* pDeviceIOFuncs,
  TUtPepDevicesVerifyDeviceFunc pVerifyDeviceFunc,
  TUtPepDevicesInitFunc pInitDeviceFunc,
  TUtPepDevicesUninitFunc pUninitDeviceFunc)
{
    if (pInitDeviceFunc())
    {
        pVerifyDeviceFunc(pDeviceIOFuncs, pbyData, ulDataLen);

        pUninitDeviceFunc();
    }
}
#pragma managed

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
  const TRAMData* pRAMData,
  TUtPepDevicesInitFunc pInitDeviceFunc,
  TUtPepDevicesUninitFunc pUninitDeviceFunc) :
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

    lWriteRAMDevice(pbyData, byData->Length, pDeviceIOFuncs,
                    m_pWriteDeviceFunc, m_pInitDeviceFunc,
                    m_pUninitDeviceFunc);
}

void Pep::Programmer::RAMData::VerifyDevice(
  Pep::Programmer::IDeviceIO^ pDeviceIO,
  array<System::Byte>^ byData)
{
    pin_ptr<System::Byte> pbyData = &byData[0];
	TDeviceIOFuncs* pDeviceIOFuncs = Pep::Programmer::UtDeviceIO::GetDeviceIOFuncs();

	Pep::Programmer::UtDeviceIO::SetCurrentDeviceIO(pDeviceIO);

    lVerifyRAMDevice(pbyData, byData->Length, pDeviceIOFuncs,
                     m_pVerifyDeviceFunc, m_pInitDeviceFunc,
                     m_pUninitDeviceFunc);
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
