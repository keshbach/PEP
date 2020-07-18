/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>

#include <Utils/UtSleep.h>

#include <Includes/UtMacros.h>

BOOL UTSLEEPAPI UtSleep(
  _In_ PLARGE_INTEGER pIntervalNanoseconds)
{
	LARGE_INTEGER IntervalMilliseconds;

	IntervalMilliseconds.QuadPart = (LONGLONG)MNanoToMilliseconds(pIntervalNanoseconds->QuadPart);

	if ((LONGLONG)MMilliToNanoseconds(IntervalMilliseconds.QuadPart) < pIntervalNanoseconds->QuadPart)
	{
		IntervalMilliseconds.QuadPart += 1;
	}

	Sleep(IntervalMilliseconds.LowPart);

	return TRUE;
}

/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/
