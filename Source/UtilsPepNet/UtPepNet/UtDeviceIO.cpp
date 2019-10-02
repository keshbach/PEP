/////////////////////////////////////////////////////////////////////////////
//  Copùright (C) 2007-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include <UtilsDevice/UtPepDevices.h>

#include "IDeviceIO.h"

#include "UtDeviceIO.h"

static BOOL l_bInitDeviceIOFuncs = TRUE;
static TDeviceIOFuncs l_DeviceIOFuncs = {0};

static Pep::Programmer::IDeviceIO::EDeviceOperation lTranslateDeviceOperation(
  EDeviceOperation DeviceOperation)
{
    switch (DeviceOperation)
    {
        case edoRead:
			return Pep::Programmer::IDeviceIO::EDeviceOperation::Read;
        case edoWrite:
			return Pep::Programmer::IDeviceIO::EDeviceOperation::Write;
        case edoProgram:
			return Pep::Programmer::IDeviceIO::EDeviceOperation::Program;
        case edoVerify:
			return Pep::Programmer::IDeviceIO::EDeviceOperation::Verify;
    }

	return Pep::Programmer::IDeviceIO::EDeviceOperation::Unknown;
}

static VOID lManagedBeginDeviceIO(
  ULONG ulTotalAddresses,
  EDeviceOperation DeviceOperation)
{
	Pep::Programmer::IDeviceIO^ pDeviceIO = Pep::Programmer::UtDeviceIO::GetCurrentDeviceIO();

    pDeviceIO->BeginDeviceIO(ulTotalAddresses,
                             lTranslateDeviceOperation(DeviceOperation));
}

static VOID lManagedEndDeviceIO(
  BOOL bErrorOccurred,
  EDeviceOperation DeviceOperation)
{
	Pep::Programmer::IDeviceIO^ pDeviceIO = Pep::Programmer::UtDeviceIO::GetCurrentDeviceIO();

    pDeviceIO->EndDeviceIO(bErrorOccurred ? true : false,
                           lTranslateDeviceOperation(DeviceOperation));
}

static VOID lManagedProgressDeviceIO(
  ULONG ulAddress)
{
	Pep::Programmer::IDeviceIO^ pDeviceIO = Pep::Programmer::UtDeviceIO::GetCurrentDeviceIO();

    pDeviceIO->ProgressDeviceIO(ulAddress);
}

static VOID lManagedVerifyByteErrorDeviceIO(
  ULONG ulAddress,
  BYTE byExpected,
  BYTE byFound)
{
	Pep::Programmer::IDeviceIO^ pDeviceIO = Pep::Programmer::UtDeviceIO::GetCurrentDeviceIO();

    pDeviceIO->VerifyByteErrorDeviceIO(ulAddress, byExpected, byFound);
}

static VOID lManagedVerifyWordErrorDeviceIO(
  ULONG ulAddress,
  WORD wExpected,
  WORD wFound)
{
	Pep::Programmer::IDeviceIO^ pDeviceIO = Pep::Programmer::UtDeviceIO::GetCurrentDeviceIO();

    pDeviceIO->VerifyWordErrorDeviceIO(ulAddress, wExpected, wFound);
}

static BOOL lManagedContinueDeviceIO(VOID)
{
	Pep::Programmer::IDeviceIO^ pDeviceIO = Pep::Programmer::UtDeviceIO::GetCurrentDeviceIO();

    return pDeviceIO->ContinueDeviceIO() ? TRUE : FALSE;
}

static VOID lManagedShowMessageDeviceIO(
  LPCWSTR pszMessage)
{
	Pep::Programmer::IDeviceIO^ pDeviceIO = Pep::Programmer::UtDeviceIO::GetCurrentDeviceIO();
    System::String^ sMessage = gcnew System::String(pszMessage);

    pDeviceIO->ShowMessageDeviceIO(sMessage);
}

static VOID lManagedLogMessageDeviceIO(
  LPCWSTR pszMessage)
{
	Pep::Programmer::IDeviceIO^ pDeviceIO = Pep::Programmer::UtDeviceIO::GetCurrentDeviceIO();
    System::String^ sMessage = gcnew System::String(pszMessage);

    pDeviceIO->LogMessageDeviceIO(sMessage);
}

#pragma unmanaged

static VOID UTPEPDEVICESCALLBACK lUnmanagedBeginDeviceIO(
  ULONG ulTotalAddresses,
  EDeviceOperation DeviceOperation)
{
    lManagedBeginDeviceIO(ulTotalAddresses, DeviceOperation);
}

static VOID UTPEPDEVICESCALLBACK lUnmanagedEndDeviceIO(
  BOOL bErrorOccurred,
  EDeviceOperation DeviceOperation)
{
    lManagedEndDeviceIO(bErrorOccurred, DeviceOperation);
}

static VOID UTPEPDEVICESCALLBACK lUnmanagedProgressDeviceIO(
  ULONG ulAddress)
{
    lManagedProgressDeviceIO(ulAddress);
}

static VOID UTPEPDEVICESCALLBACK lUnmanagedVerifyByteErrorDeviceIO(
  ULONG ulAddress,
  BYTE byExpected,
  BYTE byFound)
{
    lManagedVerifyByteErrorDeviceIO(ulAddress, byExpected, byFound);
}

static VOID UTPEPDEVICESCALLBACK lUnmanagedVerifyWordErrorDeviceIO(
  ULONG ulAddress,
  WORD wExpected,
  WORD wFound)
{
    lManagedVerifyWordErrorDeviceIO(ulAddress, wExpected, wFound);
}

static BOOL UTPEPDEVICESCALLBACK lUnmanagedContinueDeviceIO(VOID)
{
    return lManagedContinueDeviceIO();
}

static VOID UTPEPDEVICESCALLBACK lUnmanagedShowMessageDeviceIO(
  LPCWSTR pszMessage)
{
    lManagedShowMessageDeviceIO(pszMessage);
}

static VOID UTPEPDEVICESCALLBACK lUnmanagedLogMessageDeviceIO(
  LPCWSTR pszMessage)
{
    lManagedLogMessageDeviceIO(pszMessage);
}

static void lInitDeviceIOFuncs(
  TDeviceIOFuncs* pDeviceIOFuncs)
{
    pDeviceIOFuncs->pBeginDeviceIOFunc           = lUnmanagedBeginDeviceIO;
    pDeviceIOFuncs->pEndDeviceIOFunc             = lUnmanagedEndDeviceIO;
    pDeviceIOFuncs->pProgressDeviceIOFunc        = lUnmanagedProgressDeviceIO;
    pDeviceIOFuncs->pVerifyByteErrorDeviceIOFunc = lUnmanagedVerifyByteErrorDeviceIO;
    pDeviceIOFuncs->pVerifyWordErrorDeviceIOFunc = lUnmanagedVerifyWordErrorDeviceIO;
    pDeviceIOFuncs->pContinueDeviceIOFunc        = lUnmanagedContinueDeviceIO;
    pDeviceIOFuncs->pShowMessageDeviceIOFunc     = lUnmanagedShowMessageDeviceIO;
    pDeviceIOFuncs->pLogMessageDeviceIOFunc      = lUnmanagedLogMessageDeviceIO;
}

#pragma managed

TDeviceIOFuncs* Pep::Programmer::UtDeviceIO::GetDeviceIOFuncs(void)
{
    if (l_bInitDeviceIOFuncs == TRUE)
    {
        lInitDeviceIOFuncs(&l_DeviceIOFuncs);
    }

    return &l_DeviceIOFuncs;
}

Pep::Programmer::IDeviceIO^ Pep::Programmer::UtDeviceIO::GetCurrentDeviceIO(void)
{
    return s_pDeviceIO;
}

void Pep::Programmer::UtDeviceIO::SetCurrentDeviceIO(
	Pep::Programmer::IDeviceIO^ pDeviceIO)
{
    s_pDeviceIO = pDeviceIO;
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2007-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
