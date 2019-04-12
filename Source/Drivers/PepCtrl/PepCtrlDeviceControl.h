/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepCtrlDeviceControl_H)
#define PepCtrlDeviceControl_H

NTSTATUS PepCtrlDeviceControl_SetProgrammerMode(IN PIRP pIrp, IN TPepCtrlPortData* pPortData, IN const PVOID pvInBuf, IN ULONG ulInBufLen, OUT PVOID pvOutBuf, IN ULONG ulOutBufLen);
NTSTATUS PepCtrlDeviceControl_SetVccMode(IN PIRP pIrp, IN TPepCtrlPortData* pPortData, IN const PVOID pvInBuf, IN ULONG ulInBufLen, OUT PVOID pvOutBuf, IN ULONG ulOutBufLen);
NTSTATUS PepCtrlDeviceControl_SetPinPulseMode(IN PIRP pIrp, IN TPepCtrlPortData* pPortData, IN const PVOID pvInBuf, IN ULONG ulInBufLen, OUT PVOID pvOutBuf, IN ULONG ulOutBufLen);
NTSTATUS PepCtrlDeviceControl_SetVppMode(IN PIRP pIrp, IN TPepCtrlPortData* pPortData, IN const PVOID pvInBuf, IN ULONG ulInBufLen, OUT PVOID pvOutBuf, IN ULONG ulOutBufLen);
NTSTATUS PepCtrlDeviceControl_SetAddress(IN PIRP pIrp, IN TPepCtrlPortData* pPortData, IN const PVOID pvInBuf, IN ULONG ulInBufLen, OUT PVOID pvOutBuf, IN ULONG ulOutBufLen);
NTSTATUS PepCtrlDeviceControl_GetData(IN PIRP pIrp, IN TPepCtrlPortData* pPortData, IN const PVOID pvInBuf, IN ULONG ulInBufLen, OUT PVOID pvOutBuf, IN ULONG ulOutBufLen);
NTSTATUS PepCtrlDeviceControl_SetData(IN PIRP pIrp, IN TPepCtrlPortData* pPortData, IN const PVOID pvInBuf, IN ULONG ulInBufLen, OUT PVOID pvOutBuf, IN ULONG ulOutBufLen);
NTSTATUS PepCtrlDeviceControl_TriggerProgram(IN PIRP pIrp, IN TPepCtrlPortData* pPortData, IN const PVOID pvInBuf, IN ULONG ulInBufLen, OUT PVOID pvOutBuf, IN ULONG ulOutBufLen);
NTSTATUS PepCtrlDeviceControl_SetOutputEnable(IN PIRP pIrp, IN TPepCtrlPortData* pPortData, IN const PVOID pvInBuf, IN ULONG ulInBufLen, OUT PVOID pvOutBuf, IN ULONG ulOutBufLen);
NTSTATUS PepCtrlDeviceControl_GetDeviceStatus(IN PIRP pIrp, IN TPepCtrlPortData* pPortData, IN const PVOID pvInBuf, IN ULONG ulInBufLen, OUT PVOID pvOutBuf, IN ULONG ulOutBufLen);
NTSTATUS PepCtrlDeviceControl_DeviceNotification(IN PIRP pIrp, IN TPepCtrlPortData* pPortData, IN const PVOID pvInBuf, IN ULONG ulInBufLen, OUT PVOID pvOutBuf, IN ULONG ulOutBufLen);
NTSTATUS PepCtrlDeviceControl_GetSettings(IN PIRP pIrp, IN TPepCtrlPortData* pPortData, IN const PVOID pvInBuf, IN ULONG ulInBufLen, OUT PVOID pvOutBuf, IN ULONG ulOutBufLen);
NTSTATUS PepCtrlDeviceControl_SetSettings(IN PIRP pIrp, IN TPepCtrlPortData* pPortData, IN const PVOID pvInBuf, IN ULONG ulInBufLen, OUT PVOID pvOutBuf, IN ULONG ulOutBufLen);

#endif /* !defined(PepCtrlDeviceControl_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
