/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepCtrlLog_H)
#define PepCtrlLog_H

#if !defined(NDEBUG)
VOID __cdecl PepCtrlLog(_In_z_ _Printf_format_string_ PCSTR pszFormat, ...);
#else
#define PepCtrlLog(_x_) 
#endif

#endif /* !defined(PepCtrlLog_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
