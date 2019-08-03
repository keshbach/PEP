/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#include <ntddk.h>

#include <wdmguid.h>

#include "PepCtrlThread.h"
#include "PepCtrlLog.h"

#if defined(ALLOC_PRAGMA)
#pragma alloc_text (PAGE, PepCtrlThreadStart)
#pragma alloc_text (PAGE, PepCtrlThreadStop)
#endif

BOOLEAN PepCtrlThreadStart(
  _In_ PKSTART_ROUTINE pStartRoutine,
  _In_ PVOID pvContext,
  _Out_ PHANDLE phThread)
{
    BOOLEAN bResult = FALSE;
    NTSTATUS Status;

    PepCtrlLog("PepCtrlThreadStart entering.\n");

    PAGED_CODE()

    PepCtrlLog("PepCtrlThreadStart - Creating the thread.\n");

    Status = PsCreateSystemThread(phThread, DELETE | SYNCHRONIZE, NULL, NULL, NULL,
                                  pStartRoutine, pvContext);

    PepCtrlLog("PepCtrlThreadStart - Creation of the thread status: 0x%X\n", Status);

    if (NT_SUCCESS(Status))
    {
        bResult = TRUE;
    }

    PepCtrlLog("PepCtrlThreadStart leaving.\n");

    return bResult;
}

BOOLEAN PepCtrlThreadStop(
  _In_ HANDLE hThread)
{
    BOOLEAN bResult = FALSE;
    NTSTATUS Status;
    PKTHREAD Thread;

    PepCtrlLog("PepCtrlThreadStop entering.\n");

    PAGED_CODE()

    PepCtrlLog("PepCtrlThreadStop - Retrieving a thread object from the thread handle.\n");

    Status = ObReferenceObjectByHandle(hThread, 0, *PsThreadType,
                                       KernelMode, &Thread, NULL);

    if (NT_SUCCESS(Status))
    {
        PepCtrlLog("PepCtrlThreadStop - Retrieved the thread object.\n");

        PepCtrlLog("PepCtrlThreadStop - Waiting for the thread to end.\n");

        Status = KeWaitForSingleObject(Thread, Executive, KernelMode, FALSE, NULL);

        if (NT_SUCCESS(Status))
        {
            PepCtrlLog("PepCtrlThreadStop - Thread has ended.\n");
        }
        else
        {
            PepCtrlLog("PepCtrlThreadStop - Failed to wait for the thread to end.  (0x%X)\n", Status);
        }

        PepCtrlLog("PepCtrlThreadStop - Closing the thread handle.\n");

        Status = ZwClose(hThread);

        if (NT_SUCCESS(Status))
        {
            PepCtrlLog("PepCtrlThreadStop - Thread handle closed.\n");

            bResult = TRUE;
        }
        else
        {
            PepCtrlLog("PepCtrlThreadStop - Failed to close the thread handle.  (0x%X)\n", Status);
        }

        PepCtrlLog("PepCtrlThreadStop - Releasing the thread object.\n");

        ObDereferenceObject(Thread);
    }
    else
    {
        PepCtrlLog("PepCtrlThreadStop - Could not retrieve the thread object from the thread handle.  (0x%X)\n", Status);
    }

    PepCtrlLog("PepCtrlThreadStop leaving.\n");

    return bResult;
}

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
