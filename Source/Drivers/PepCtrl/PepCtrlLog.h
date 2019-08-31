/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepCtrlLog_H)
#define PepCtrlLog_H

#if !defined(NDEBUG)

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID PepCtrlLogOpenFile(_In_z_ PCWSTR pszLogFile);

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID PepCtrlLogCloseFile();

VOID __cdecl PepCtrlLog(_In_z_ _Printf_format_string_ PCSTR pszFormat, ...);

VOID __cdecl PepCtrlLogV(_In_z_ PCSTR pszFormat, _In_ va_list argList);

#else
#define PepCtrlLogOpenFile(_x_)
#define PepCtrlLogCloseFile()

#define PepCtrlLog(_x_)

#define PepCtrlLogV(_x_)

#endif

#endif /* !defined(PepCtrlLog_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
