/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#include <ntddk.h>

#include <ntstrsafe.h>

#include "PepCtrlLog.h"

#if !defined(NDEBUG)

VOID __cdecl PepCtrlLog(_In_z_ _Printf_format_string_ PCSTR pszFormat, ...)
{
    va_list arguments;

    va_start(arguments, pszFormat);

    vDbgPrintExWithPrefix("PepCtrl: ", DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, pszFormat, arguments);

    va_end(arguments);
}

#endif

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
