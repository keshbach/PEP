/***************************************************************************/
/*  Copyright (C) 2008-2021 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtMacros_H)
#define UtMacros_H

#define MArrayLen(Array) (sizeof(Array) / sizeof(Array[0]))

#define MMillisecondsToRelativeTime(milliseconds) \
    (((LONGLONG)milliseconds * (LONGLONG)-1000000) / (LONGLONG)100)

/*
  Macro to convert time units
*/

#define MNanoToMilliseconds(nano) (nano / 1000000.0)

#define MMilliToNanoseconds(milli) (milli * 1000000.0)

#define MMicroToNanoseconds(micro) (micro * 1000.0)

#endif /* end of UtMacros_H */

/***************************************************************************/
/*  Copyright (C) 2008-2021 Kevin Eshbach                                  */
/***************************************************************************/
