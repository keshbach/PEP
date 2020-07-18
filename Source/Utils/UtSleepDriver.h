/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtSleepDriver_H)
#define UtSleepDriver_H

#define UTSLEEPDRIVERAPI __stdcall

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN UTSLEEPDRIVERAPI UtSleep(_In_ PLARGE_INTEGER pIntervalNanoseconds);

#endif /* !defined(UtSleepDriver_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/
