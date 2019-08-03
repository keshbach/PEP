/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#include <ntddk.h>

#include <ntstrsafe.h>

#include "PepCtrlLog.h"

#if !defined(NDEBUG)

#if defined(ALLOC_PRAGMA)
#pragma alloc_text (PAGE, PepCtrlLogOpenFile)
#pragma alloc_text (PAGE, PepCtrlLogCloseFile)
#endif

static HANDLE l_hFile = NULL;

VOID PepCtrlLogOpenFile(
  _In_z_ PCWSTR pszLogFile)
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK StatusBlock;

    PAGED_CODE()

    if (KeGetCurrentIrql() != PASSIVE_LEVEL)
    {
        return;
    }

    RtlInitUnicodeString(&FileName, pszLogFile);

    InitializeObjectAttributes(&ObjectAttributes, &FileName,
                               OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
                               NULL, NULL);

    Status = ZwCreateFile(&l_hFile, GENERIC_WRITE, &ObjectAttributes,
                          &StatusBlock, NULL, FILE_ATTRIBUTE_NORMAL | FILE_SHARE_READ,
                          0, FILE_OVERWRITE_IF, FILE_SYNCHRONOUS_IO_NONALERT,
                          NULL, 0);
}

VOID PepCtrlLogCloseFile()
{
    PAGED_CODE()

    if (l_hFile)
    {
        ZwClose(l_hFile);

        l_hFile = NULL;
    }
}

VOID __cdecl PepCtrlLog(_In_z_ _Printf_format_string_ PCSTR pszFormat, ...)
{
    size_t nDataLen;
    NTSTATUS Status;
    IO_STATUS_BLOCK StatusBlock;

    va_list arguments;

    va_start(arguments, pszFormat);

    vDbgPrintExWithPrefix("PepCtrl: ", DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, pszFormat, arguments);

    va_end(arguments);

    if (l_hFile && KeGetCurrentIrql() == PASSIVE_LEVEL)
    {
        nDataLen = RtlStringCbLengthA(pszFormat, NTSTRSAFE_MAX_LENGTH, &nDataLen);

        Status = ZwWriteFile(l_hFile, NULL, NULL, NULL, &StatusBlock,
                             (PVOID)pszFormat, (ULONG)nDataLen, NULL, NULL);
    }
}

#endif

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
