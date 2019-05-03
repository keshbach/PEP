/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepCtrlDeviceControl_H)
#define PepCtrlDeviceControl_H

NTSTATUS PepCtrlDeviceControl_SetProgrammerMode(_In_ PIRP pIrp, _In_ TPepCtrlPortData* pPortData, _In_ const PVOID pvInBuf, _In_ ULONG ulInBufLen, _Out_ PVOID pvOutBuf, _In_ ULONG ulOutBufLen);
NTSTATUS PepCtrlDeviceControl_SetVccMode(_In_ PIRP pIrp, _In_ TPepCtrlPortData* pPortData, _In_ const PVOID pvInBuf, _In_ ULONG ulInBufLen, _Out_ PVOID pvOutBuf, _In_ ULONG ulOutBufLen);
NTSTATUS PepCtrlDeviceControl_SetPinPulseMode(_In_ PIRP pIrp, _In_ TPepCtrlPortData* pPortData, _In_ const PVOID pvInBuf, _In_ ULONG ulInBufLen, _Out_ PVOID pvOutBuf, _In_ ULONG ulOutBufLen);
NTSTATUS PepCtrlDeviceControl_SetVppMode(_In_ PIRP pIrp, _In_ TPepCtrlPortData* pPortData, _In_ const PVOID pvInBuf, _In_ ULONG ulInBufLen, _Out_ PVOID pvOutBuf, _In_ ULONG ulOutBufLen);
NTSTATUS PepCtrlDeviceControl_SetAddress(_In_ PIRP pIrp, _In_ TPepCtrlPortData* pPortData, _In_ const PVOID pvInBuf, _In_ ULONG ulInBufLen, _Out_ PVOID pvOutBuf, _In_ ULONG ulOutBufLen);
NTSTATUS PepCtrlDeviceControl_GetData(_In_ PIRP pIrp, _In_ TPepCtrlPortData* pPortData, _In_ const PVOID pvInBuf, _In_ ULONG ulInBufLen, _Out_ PVOID pvOutBuf, _In_ ULONG ulOutBufLen);
NTSTATUS PepCtrlDeviceControl_SetData(_In_ PIRP pIrp, _In_ TPepCtrlPortData* pPortData, _In_ const PVOID pvInBuf, _In_ ULONG ulInBufLen, _Out_ PVOID pvOutBuf, _In_ ULONG ulOutBufLen);
NTSTATUS PepCtrlDeviceControl_TriggerProgram(_In_ PIRP pIrp, _In_ TPepCtrlPortData* pPortData, _In_ const PVOID pvInBuf, _In_ ULONG ulInBufLen, _Out_ PVOID pvOutBuf, _In_ ULONG ulOutBufLen);
NTSTATUS PepCtrlDeviceControl_SetOutputEnable(_In_ PIRP pIrp, _In_ TPepCtrlPortData* pPortData, _In_ const PVOID pvInBuf, _In_ ULONG ulInBufLen, _Out_ PVOID pvOutBuf, _In_ ULONG ulOutBufLen);
NTSTATUS PepCtrlDeviceControl_GetDeviceStatus(_In_ PIRP pIrp, _In_ TPepCtrlPortData* pPortData, _In_ const PVOID pvInBuf, _In_ ULONG ulInBufLen, _Out_ PVOID pvOutBuf, _In_ ULONG ulOutBufLen);
NTSTATUS PepCtrlDeviceControl_DeviceNotification(_In_ PIRP pIrp, _In_ TPepCtrlPortData* pPortData, _In_ const PVOID pvInBuf, _In_ ULONG ulInBufLen, _Out_ PVOID pvOutBuf, _In_ ULONG ulOutBufLen);
NTSTATUS PepCtrlDeviceControl_GetSettings(_In_ PIRP pIrp, _In_ TPepCtrlPortData* pPortData, _In_ const PVOID pvInBuf, _In_ ULONG ulInBufLen, _Out_ PVOID pvOutBuf, _In_ ULONG ulOutBufLen);
NTSTATUS PepCtrlDeviceControl_SetSettings(_In_ PIRP pIrp, _In_ TPepCtrlPortData* pPortData, _In_ const PVOID pvInBuf, _In_ ULONG ulInBufLen, _Out_ PVOID pvOutBuf, _In_ ULONG ulOutBufLen);

#endif /* !defined(PepCtrlDeviceControl_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
