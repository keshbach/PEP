/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "ROMData.h"
#include "UtDeviceIO.h"

#pragma unmanaged
static VOID lReadROMDevice(
  LPBYTE pbyData,
  ULONG ulDataLen,
  TDeviceIOFuncs* pDeviceIOFuncs,
  TUtPepDevicesReadDeviceFunc pReadDeviceFunc,
  TUtPepDevicesInitFunc pInitDeviceFunc,
  TUtPepDevicesUninitFunc pUninitDeviceFunc)
{
    if (pInitDeviceFunc())
    {
        pReadDeviceFunc(pDeviceIOFuncs, pbyData, ulDataLen);

        pUninitDeviceFunc();
    }
}

static VOID lProgramROMDevice(
  LPBYTE pbyData,
  ULONG ulDataLen,
  TDeviceIOFuncs* pDeviceIOFuncs,
  TUtPepDevicesProgramDeviceFunc pProgramDeviceFunc,
  TUtPepDevicesInitFunc pInitDeviceFunc,
  TUtPepDevicesUninitFunc pUninitDeviceFunc)
{
    if (pInitDeviceFunc())
    {
        pProgramDeviceFunc(pDeviceIOFuncs, pbyData, ulDataLen);

        pUninitDeviceFunc();
    }
}

static VOID lVerifyROMDevice(
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
  const TROMData* pROMData,
  TUtPepDevicesInitFunc pInitDeviceFunc,
  TUtPepDevicesUninitFunc pUninitDeviceFunc) :
  m_pReadDeviceFunc(pROMData->pReadDeviceFunc),
  m_pProgramDeviceFunc(pROMData->pProgramDeviceFunc),
  m_pVerifyDeviceFunc(pROMData->pVerifyDeviceFunc),
  m_pInitDeviceFunc(pInitDeviceFunc),
  m_pUninitDeviceFunc(pUninitDeviceFunc)
{
    m_nSize         = pROMData->nSize;
    m_nBitsPerValue = pROMData->nBitsPerValue;
    m_sDeviceVpp    = gcnew System::String(UtPepDevicesGetDeviceVppName(pROMData->DeviceVpp));

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

    lReadROMDevice(pbyData, byData->Length, pDeviceIOFuncs,
                   m_pReadDeviceFunc, m_pInitDeviceFunc,
                   m_pUninitDeviceFunc);
}

void Pep::Programmer::ROMData::ProgramDevice(
  Pep::Programmer::IDeviceIO^ pDeviceIO,
  array<System::Byte>^ byData)
{
    pin_ptr<System::Byte> pbyData = &byData[0];
	TDeviceIOFuncs* pDeviceIOFuncs = Pep::Programmer::UtDeviceIO::GetDeviceIOFuncs();

	Pep::Programmer::UtDeviceIO::SetCurrentDeviceIO(pDeviceIO);

    lProgramROMDevice(pbyData, byData->Length, pDeviceIOFuncs,
                      m_pProgramDeviceFunc, m_pInitDeviceFunc,
                      m_pUninitDeviceFunc);
}

void Pep::Programmer::ROMData::VerifyDevice(
  Pep::Programmer::IDeviceIO^ pDeviceIO,
  array<System::Byte>^ byData)
{
    pin_ptr<System::Byte> pbyData = &byData[0];
	TDeviceIOFuncs* pDeviceIOFuncs = Pep::Programmer::UtDeviceIO::GetDeviceIOFuncs();

	Pep::Programmer::UtDeviceIO::SetCurrentDeviceIO(pDeviceIO);

    lVerifyROMDevice(pbyData, byData->Length, pDeviceIOFuncs,
                     m_pVerifyDeviceFunc, m_pInitDeviceFunc,
                     m_pUninitDeviceFunc);
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2014 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
