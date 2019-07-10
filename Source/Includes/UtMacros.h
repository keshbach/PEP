/***************************************************************************/
/*  Copyright (C) 2008-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtMacros_H)
#define UtMacros_H

#define MArrayLen(Array) (sizeof(##Array##) / sizeof(##Array##[0]))

#define MMillisecondsToRelativeTime(milliseconds) \
    (((LONGLONG)milliseconds * (LONGLONG)-1000000) / (LONGLONG)100)

#endif /* end of UtMacros_H */

/***************************************************************************/
/*  Copyright (C) 2008-2019 Kevin Eshbach                                  */
/***************************************************************************/
