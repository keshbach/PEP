/***************************************************************************/
/*  Copyright (C) 2006-2023 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtSleepDriver_H)
#define UtSleepDriver_H

#define UTSLEEPDRIVERAPI __stdcall

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN UTSLEEPDRIVERAPI UtSleep(_In_ UINT32 nIntervalNanoseconds);

#endif /* !defined(UtSleepDriver_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2023 Kevin Eshbach                                  */
/***************************************************************************/
