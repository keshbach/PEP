/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Port.h"
#include "PortEnumerator.h"

#include "ListPorts.h"

#include <Config/UtListPorts.h>

#include <Utils/UtHeap.h>

#pragma region "Constants"

#define CUtListPortsDllName L"UtListPorts.dll"

#pragma endregion

#pragma region "Typedefs"

typedef BOOL (UTLISTPORTSAPI *TUtListPortsInitializeFunc)(VOID);
typedef BOOL (UTLISTPORTSAPI *TUtListPortsUninitializeFunc)(VOID);
typedef BOOL (UTLISTPORTSAPI *TUtListPortsGetLptPortCountFunc)(_Out_ LPINT pnCount);
typedef BOOL (UTLISTPORTSAPI *TUtListPortsGetLptPortDataFunc)(_In_ INT nIndex, _In_ INT nPortData, _Out_ LPWSTR pszData, _Out_ LPINT pnDataLen);

#pragma endregion

Common::ListPorts::Parallel::Parallel()
{
}

System::Collections::Generic::IEnumerator<Common::ListPorts::Port^>^ Common::ListPorts::Parallel::GetEnumerator()
{
    HMODULE hListPorts = ::LoadLibraryW(CUtListPortsDllName);
    TUtListPortsInitializeFunc pListPortsInitialize;
    TUtListPortsUninitializeFunc pListPortsUninitialize;
    TUtListPortsGetLptPortCountFunc pListPortsGetLptPortCount;
    TUtListPortsGetLptPortDataFunc pListPortsGetLptPortData;
    INT nTotalPorts;
    LPWSTR pszLocation, pszPhysicalDeviceObject;
    INT nLocationLen, nPhysicalDeviceObjectLen;
    System::Collections::Generic::List<Common::ListPorts::Port^>^ PortList;

    if (hListPorts == NULL)
    {
        throw gcnew System::Exception("Could not load the UtListPorts.dll file.");
    }

    pListPortsInitialize = (TUtListPortsInitializeFunc)::GetProcAddress(hListPorts, "UtListPortsInitialize");
    pListPortsUninitialize = (TUtListPortsUninitializeFunc)::GetProcAddress(hListPorts, "UtListPortsUninitialize");
    pListPortsGetLptPortCount = (TUtListPortsGetLptPortCountFunc)::GetProcAddress(hListPorts, "UtListPortsGetLptPortCount");
    pListPortsGetLptPortData = (TUtListPortsGetLptPortDataFunc)::GetProcAddress(hListPorts, "UtListPortsGetLptPortData");

    if (pListPortsInitialize == NULL || pListPortsUninitialize == NULL ||
        pListPortsGetLptPortCount == NULL || pListPortsGetLptPortData == NULL)
    {
        ::FreeLibrary(hListPorts);

        throw gcnew System::Exception("Faled to find all of the list port functions .");
    }

    PortList = gcnew System::Collections::Generic::List<Common::ListPorts::Port^>();

    if (UtInitHeap() == FALSE)
    {
        ::FreeLibrary(hListPorts);

        throw gcnew System::Exception("Could not allocate a heap.");
    }

    if (pListPortsInitialize() == FALSE)
    {
        UtUninitHeap();

        ::FreeLibrary(hListPorts);

        throw gcnew System::Exception("Could not retrieve the parallel ports.");
    }

    pListPortsGetLptPortCount(&nTotalPorts);

    for (INT nIndex = 0; nIndex < nTotalPorts; ++nIndex)
    {
        pListPortsGetLptPortData(nIndex, epdLocation, NULL, &nLocationLen);
        pListPortsGetLptPortData(nIndex, epdPhysicalDeviceObjectName, NULL, &nPhysicalDeviceObjectLen);

        pszLocation = (LPWSTR)UtAllocMem(nLocationLen * sizeof(WCHAR));
        pszPhysicalDeviceObject = (LPWSTR)UtAllocMem(nPhysicalDeviceObjectLen * sizeof(WCHAR));

        pListPortsGetLptPortData(nIndex, epdLocation, pszLocation, &nLocationLen);
        pListPortsGetLptPortData(nIndex, epdPhysicalDeviceObjectName, pszPhysicalDeviceObject, &nPhysicalDeviceObjectLen);

        PortList->Add(gcnew Common::ListPorts::Port(gcnew System::String(pszLocation),
                                                    gcnew System::String(pszPhysicalDeviceObject)));

        UtFreeMem(pszLocation);
        UtFreeMem(pszPhysicalDeviceObject);
    }

    pListPortsUninitialize();

    UtUninitHeap();

    ::FreeLibrary(hListPorts);

    return gcnew Common::ListPorts::Enumerator(PortList);
}

System::Collections::IEnumerator^ Common::ListPorts::Parallel::GetEnumeratorBase()
{
    return GetEnumerator();
}

/////////////////////////////////////////////////////////////////////////////
//  Copyright (C) 2006-2019 Kevin Eshbach
/////////////////////////////////////////////////////////////////////////////
