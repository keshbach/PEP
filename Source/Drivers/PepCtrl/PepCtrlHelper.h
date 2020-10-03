/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepCtrlHelper_H)
#define PepCtrlHelper_H

#if !defined(NDEBUG)

PCSTR PepCtrlHelperTranslateControlCode(_In_ ULONG ulIOControlCode);

PCSTR PepCtrlHelperTranslateState(_In_ INT32 nState);

#else

#define PepCtrlHelperTranslateControlCode(_x_)

#define PepCtrlHelperTranslateState(_x_)


#endif

#endif /* !defined(PepCtrlHelper_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/
