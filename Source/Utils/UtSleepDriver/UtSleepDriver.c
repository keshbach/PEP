/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/

#include <ntddk.h>

#include <Utils/UtSleepDriver.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, UtSleep)
#endif

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN UTSLEEPDRIVERAPI UtSleep(
  _In_ PLARGE_INTEGER pIntervalNanoseconds)
{
	NTSTATUS status;
	LARGE_INTEGER Interval;

	PAGED_CODE()

	Interval.QuadPart = pIntervalNanoseconds->QuadPart / 100;

	if (Interval.QuadPart * 100 < pIntervalNanoseconds->QuadPart)
	{
		++Interval.QuadPart;
	}

	status = KeDelayExecutionThread(KernelMode, FALSE, &Interval);

	return (STATUS_SUCCESS == status);
}

/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/
