/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#include <ntddk.h>

#include "PepCtrlLog.h"

#include <ntstrsafe.h>

#include <Includes/UtMacros.h>

#if !defined(NDEBUG)

static BOOLEAN lFlushFile();

#if defined(ALLOC_PRAGMA)
#pragma alloc_text (PAGE, PepCtrlLogOpenFile)
#pragma alloc_text (PAGE, PepCtrlLogCloseFile)

#pragma alloc_text (PAGE, lFlushFile)
#endif

static CHAR l_cBuffer[5121]; // max 512 buffer according to windows documentation on DbgPrint

static HANDLE l_hFile = NULL;

#pragma region "Local Functions"

_IRQL_requires_max_(DISPATCH_LEVEL)
static NTSTATUS lFlushFileIoCompletion(
  _In_ PDEVICE_OBJECT pDeviceObject,
  _In_ PIRP pIrp,
  _In_ PVOID pvContext)
{
    pDeviceObject;
    pIrp;

    KeSetEvent((PKEVENT)pvContext, IO_NO_INCREMENT, FALSE);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

_IRQL_requires_max_(PASSIVE_LEVEL)
static BOOLEAN lFlushFile()
{
    NTSTATUS Status;
    PFILE_OBJECT pFileObject;
    PDEVICE_OBJECT pDeviceObject;
    PIRP pIrp;
    KEVENT Event;
    IO_STATUS_BLOCK IoStatusBlock;

    PAGED_CODE()

    Status = ObReferenceObjectByHandle(l_hFile, FILE_ALL_ACCESS, *IoFileObjectType, KernelMode, &pFileObject, NULL);

    if (Status != STATUS_SUCCESS)
    {
        return FALSE;
    }

    pDeviceObject = IoGetRelatedDeviceObject(pFileObject);

    RtlZeroMemory(&IoStatusBlock, sizeof(IoStatusBlock));

    KeInitializeEvent(&Event, NotificationEvent, FALSE);

    pIrp = IoBuildDeviceIoControlRequest(IRP_MJ_FLUSH_BUFFERS, pDeviceObject,
                                         NULL, 0, NULL, 0,
                                         TRUE, &Event, &IoStatusBlock);

    if (!pIrp)
    {
        ObDereferenceObject(pFileObject);

        return FALSE;
    }

    IoSetCompletionRoutine(pIrp, lFlushFileIoCompletion, &Event, TRUE, TRUE, TRUE);

    Status = IoCallDriver(pDeviceObject, pIrp);

    if (Status == STATUS_PENDING)
    {
        Status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

        if (Status == STATUS_TIMEOUT)
        {
            IoCancelIrp(pIrp);

            Status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

            Status = STATUS_UNSUCCESSFUL;
        }
    }

    IoCompleteRequest(pIrp, IO_NO_INCREMENT);

    Status = KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);

    ObDereferenceObject(pFileObject);

    return TRUE;
}

#pragma endregion

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID PepCtrlLogOpenFile(
  _In_z_ PCWSTR pszLogFile)
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK StatusBlock;

    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PepCtrl: PepCtrlLogOpenFile entering");

    PAGED_CODE()

    if (KeGetCurrentIrql() != PASSIVE_LEVEL)
    {
        //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PepCtrl: PepCtrlLogOpenFile leaving (Invalid Irql)");

        return;
    }

    RtlInitUnicodeString(&FileName, pszLogFile);

    InitializeObjectAttributes(&ObjectAttributes, &FileName,
                               OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
                               NULL, NULL);

    Status = ZwCreateFile(&l_hFile, SYNCHRONIZE | GENERIC_WRITE, &ObjectAttributes,
                          &StatusBlock, NULL, FILE_ATTRIBUTE_NORMAL, 
                          FILE_SHARE_READ, FILE_OVERWRITE_IF,
                          FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE,
                          NULL, 0);

    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PepCtrl: PepCtrlLogOpenFile leaving");
}

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID PepCtrlLogCloseFile()
{
    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PepCtrl: PepCtrlLogCloseFile entering");

    PAGED_CODE()

    if (l_hFile)
    {
        ZwClose(l_hFile);

        l_hFile = NULL;
    }

    //DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "PepCtrl: PepCtrlLogCloseFile closing");
}

VOID __cdecl PepCtrlLog(_In_z_ _Printf_format_string_ PCSTR pszFormat, ...)
{
    va_list arguments;

    va_start(arguments, pszFormat);

    PepCtrlLogV(pszFormat, arguments);

    va_end(arguments);
}

VOID __cdecl PepCtrlLogV(
  _In_z_ PCSTR pszFormat,
  _In_ va_list argList)
{
    size_t nPrefixLen, nDataLen;
    NTSTATUS Status;
    IO_STATUS_BLOCK StatusBlock;

    RtlStringCchCopyA(l_cBuffer, MArrayLen(l_cBuffer), "PepCtrl: ");

    RtlStringCchLengthA(l_cBuffer, NTSTRSAFE_MAX_CCH, &nPrefixLen);

    Status = RtlStringCchVPrintfA(l_cBuffer + nPrefixLen, MArrayLen(l_cBuffer) - nPrefixLen,
                                  pszFormat, argList);

    DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, l_cBuffer);

    if (l_hFile && KeGetCurrentIrql() == PASSIVE_LEVEL)
    {
        Status = RtlStringCbLengthA(l_cBuffer, NTSTRSAFE_MAX_LENGTH, &nDataLen);

        Status = ZwWriteFile(l_hFile, NULL, NULL, NULL, &StatusBlock,
                             l_cBuffer, (ULONG)nDataLen, NULL, NULL);

        lFlushFile();
    }
}

#endif

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
