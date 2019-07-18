/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Port.h"
#include "PortEnumerator.h"

#include "ListPorts.h"

#include <Config/UtListPorts.h>

#include <Utils/UtHeap.h>

Common::ListPorts::Parallel::Parallel()
{
}

Common::ListPorts::Parallel::~Parallel()
{
}

System::Collections::Generic::IEnumerator<Common::ListPorts::Port^>^ Common::ListPorts::Parallel::GetEnumerator()
{
    INT nTotalPorts;
    LPWSTR pszLocation, pszPhysicalDeviceObject;
    INT nLocationLen, nPhysicalDeviceObjectLen;
    System::Collections::Generic::List<Common::ListPorts::Port^>^ PortList;

    PortList = gcnew System::Collections::Generic::List<Common::ListPorts::Port^>();

    if (UtInitHeap() == FALSE)
    {
        throw gcnew System::Exception("Could not allocate a heap.");
    }

    if (UtListPortsInitialize() == FALSE)
    {
        UtUninitHeap();

        throw gcnew System::Exception("Could not retrieve the parallel ports.");
    }

    UtListPortsGetLptPortCount(&nTotalPorts);

    for (INT nIndex = 0; nIndex < nTotalPorts; ++nIndex)
    {
        UtListPortsGetLptPortData(nIndex, epdLocation, NULL, &nLocationLen);
        UtListPortsGetLptPortData(nIndex, epdPhysicalDeviceObjectName, NULL, &nPhysicalDeviceObjectLen);

        pszLocation = (LPWSTR)UtAllocMem(nLocationLen * sizeof(WCHAR));
        pszPhysicalDeviceObject = (LPWSTR)UtAllocMem(nPhysicalDeviceObjectLen * sizeof(WCHAR));

        UtListPortsGetLptPortData(nIndex, epdLocation, pszLocation, &nLocationLen);
        UtListPortsGetLptPortData(nIndex, epdPhysicalDeviceObjectName, pszPhysicalDeviceObject, &nPhysicalDeviceObjectLen);

        PortList->Add(gcnew Common::ListPorts::Port(gcnew System::String(pszLocation),
                                                    gcnew System::String(pszPhysicalDeviceObject)));

        UtFreeMem(pszLocation);
        UtFreeMem(pszPhysicalDeviceObject);
    }

    UtListPortsUninitialize();

    UtUninitHeap();

    return gcnew Common::ListPorts::Enumerator(PortList);
}

System::Collections::IEnumerator^ Common::ListPorts::Parallel::GetEnumeratorBase()
{
    return GetEnumerator();
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
