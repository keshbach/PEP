/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepCtrlLog_H)
#define PepCtrlLog_H

#if !defined(NDEBUG)
VOID PepCtrlLogOpenFile(_In_z_ PCWSTR pszLogFile);
VOID PepCtrlLogCloseFile();

VOID __cdecl PepCtrlLog(_In_z_ _Printf_format_string_ PCSTR pszFormat, ...);
#else
#define PepCtrlLogOpenFile(_x_)
#define PepCtrlLogCloseFile()

#define PepCtrlLog(_x_) 
#endif

#endif /* !defined(PepCtrlLog_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
