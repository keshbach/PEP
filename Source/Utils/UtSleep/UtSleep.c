/***************************************************************************/
/*  Copyright (C) 2006-2023 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>

#include <Utils/UtSleep.h>

#include <Includes/UtMacros.h>

BOOL UTSLEEPAPI UtSleep(
  _In_ UINT32 nIntervalNanoseconds)
{
	LONGLONG IntervalMilliseconds;

	IntervalMilliseconds = (LONGLONG)MNanoToMilliseconds(nIntervalNanoseconds);

	if (MMilliToNanoseconds(IntervalMilliseconds) < nIntervalNanoseconds)
	{
		++IntervalMilliseconds;
	}

	Sleep((UINT32)IntervalMilliseconds);

	return TRUE;
}

/***************************************************************************/
/*  Copyright (C) 2006-2023 Kevin Eshbach                                  */
/***************************************************************************/
