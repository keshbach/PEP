/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/

#include <ntddk.h>

#include <wdmguid.h>

#include "PepCtrlThread.h"
#include "PepCtrlLog.h"

#if defined(ALLOC_PRAGMA)
#pragma alloc_text (PAGE, PepCtrlThreadStart)
#pragma alloc_text (PAGE, PepCtrlThreadStop)
#endif

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN PepCtrlThreadStart(
  _In_ PKSTART_ROUTINE pStartRoutine,
  _In_ PVOID pvContext,
  _Out_ PHANDLE phThread)
{
    BOOLEAN bResult = FALSE;
    NTSTATUS Status;

    PepCtrlLog("PepCtrlThreadStart entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    PepCtrlLog("PepCtrlThreadStart - Creating the thread.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    Status = PsCreateSystemThread(phThread, DELETE | SYNCHRONIZE, NULL, NULL, NULL,
                                  pStartRoutine, pvContext);

    PepCtrlLog("PepCtrlThreadStart - Creation of the thread status: 0x%X.  (Thread: 0x%p)\n",
		       Status, PsGetCurrentThread());

    if (NT_SUCCESS(Status))
    {
        bResult = TRUE;
    }

    PepCtrlLog("PepCtrlThreadStart leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return bResult;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN PepCtrlThreadStop(
  _In_ HANDLE hThread)
{
    BOOLEAN bResult = FALSE;
    NTSTATUS Status;
    PKTHREAD Thread;

    PepCtrlLog("PepCtrlThreadStop entering.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    PAGED_CODE()

    PepCtrlLog("PepCtrlThreadStop - Retrieving a thread object from the thread handle.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    Status = ObReferenceObjectByHandle(hThread, 0, *PsThreadType,
                                       KernelMode, &Thread, NULL);

    if (NT_SUCCESS(Status))
    {
        PepCtrlLog("PepCtrlThreadStop - Retrieved the thread object.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        PepCtrlLog("PepCtrlThreadStop - Waiting for the thread to end.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        Status = KeWaitForSingleObject(Thread, Executive, KernelMode, FALSE, NULL);

        if (NT_SUCCESS(Status))
        {
            PepCtrlLog("PepCtrlThreadStop - Thread has ended.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());
        }
        else
        {
            PepCtrlLog("PepCtrlThreadStop - Failed to wait for the thread to end (0x%X).  (Thread: 0x%p)\n",
				       Status, PsGetCurrentThread());
        }

        PepCtrlLog("PepCtrlThreadStop - Closing the thread handle.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        Status = ZwClose(hThread);

        if (NT_SUCCESS(Status))
        {
            PepCtrlLog("PepCtrlThreadStop - Thread handle closed.  (Thread: 0x%p)\n",
				       PsGetCurrentThread());

            bResult = TRUE;
        }
        else
        {
            PepCtrlLog("PepCtrlThreadStop - Failed to close the thread handle (0x%X).  (Thread: 0x%p)\n",
				       Status, PsGetCurrentThread());
        }

        PepCtrlLog("PepCtrlThreadStop - Releasing the thread object.  (Thread: 0x%p)\n",
			       PsGetCurrentThread());

        ObDereferenceObject(Thread);
    }
    else
    {
        PepCtrlLog("PepCtrlThreadStop - Could not retrieve the thread object from the thread handle (0x%X).  (Thread: 0x%p)\n",
			       Status, PsGetCurrentThread());
    }

    PepCtrlLog("PepCtrlThreadStop leaving.  (Thread: 0x%p)\n",
		       PsGetCurrentThread());

    return bResult;
}

/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/
