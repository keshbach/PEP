/***************************************************************************/
/*  Copyright (C) 2006-2024 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepCtrlDeviceControl_H)
#define PepCtrlDeviceControl_H

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_SetProgrammerMode(_In_ PIRP pIrp,
                                                _In_ TPepCtrlPortData* pPortData,
                                                _In_ const PVOID pvInBuf,
                                                _In_ ULONG ulInBufLen,
                                                _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
                                                _In_ ULONG ulOutBufLen);

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_SetVccMode(_In_ PIRP pIrp,
                                         _In_ TPepCtrlPortData* pPortData,
                                         _In_ const PVOID pvInBuf,
                                         _In_ ULONG ulInBufLen,
                                         _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
                                         _In_ ULONG ulOutBufLen);

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_SetPinPulseMode(_In_ PIRP pIrp,
                                              _In_ TPepCtrlPortData* pPortData,
                                              _In_ const PVOID pvInBuf,
                                              _In_ ULONG ulInBufLen,
                                              _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
                                              _In_ ULONG ulOutBufLen);

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_SetVppMode(_In_ PIRP pIrp,
                                         _In_ TPepCtrlPortData* pPortData,
                                         _In_ const PVOID pvInBuf,
                                         _In_ ULONG ulInBufLen,
                                         _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
                                         _In_ ULONG ulOutBufLen);

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_SetAddress(_In_ PIRP pIrp,
                                         _In_ TPepCtrlPortData* pPortData,
                                         _In_ const PVOID pvInBuf,
                                         _In_ ULONG ulInBufLen,
                                         _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
                                         _In_ ULONG ulOutBufLen);

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_SetAddressWithDelay(_In_ PIRP pIrp,
                                                  _In_ TPepCtrlPortData* pPortData,
                                                  _In_ const PVOID pvInBuf,
                                                  _In_ ULONG ulInBufLen,
                                                  _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
                                                  _In_ ULONG ulOutBufLen);

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_GetData(_In_ PIRP pIrp,
                                      _In_ TPepCtrlPortData* pPortData,
                                      _In_ const PVOID pvInBuf,
                                      _In_ ULONG ulInBufLen,
                                      _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
                                      _In_ ULONG ulOutBufLen);

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_SetData(_In_ PIRP pIrp,
                                      _In_ TPepCtrlPortData* pPortData,
                                      _In_ const PVOID pvInBuf,
                                      _In_ ULONG ulInBufLen,
                                      _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
                                      _In_ ULONG ulOutBufLen);

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_TriggerProgram(_In_ PIRP pIrp,
                                             _In_ TPepCtrlPortData* pPortData,
                                             _In_ const PVOID pvInBuf,
                                             _In_ ULONG ulInBufLen,
                                             _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
                                             _In_ ULONG ulOutBufLen);

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_SetOutputEnable(_In_ PIRP pIrp,
                                              _In_ TPepCtrlPortData* pPortData,
                                              _In_ const PVOID pvInBuf,
                                              _In_ ULONG ulInBufLen,
                                              _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
                                              _In_ ULONG ulOutBufLen);

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_GetDeviceStatus(_In_ PIRP pIrp,
                                              _In_ TPepCtrlPortData* pPortData,
                                              _In_ const PVOID pvInBuf,
                                              _In_ ULONG ulInBufLen,
                                              _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
                                              _In_ ULONG ulOutBufLen);

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_DeviceNotification(_In_ PIRP pIrp,
                                                 _In_ TPepCtrlPortData* pPortData,
                                                 _In_ const PVOID pvInBuf,
                                                 _In_ ULONG ulInBufLen,
                                                 _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
                                                 _In_ ULONG ulOutBufLen);

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_GetPortSettings(_In_ PIRP pIrp,
                                              _In_ TPepCtrlPortData* pPortData,
                                              _In_ const PVOID pvInBuf,
                                              _In_ ULONG ulInBufLen,
                                              _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
                                              _In_ ULONG ulOutBufLen);

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_SetPortSettings(_In_ PIRP pIrp,
                                              _In_ TPepCtrlPortData* pPortData,
                                              _In_ const PVOID pvInBuf,
                                              _In_ ULONG ulInBufLen,
                                              _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
                                              _In_ ULONG ulOutBufLen);

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_SetDelaySettings(_In_ PIRP pIrp,
                                               _In_ TPepCtrlPortData* pPortData,
                                               _In_ const PVOID pvInBuf,
                                               _In_ ULONG ulInBufLen,
                                               _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
                                               _In_ ULONG ulOutBufLen);

_IRQL_requires_max_(PASSIVE_LEVEL)
NTSTATUS PepCtrlDeviceControl_DebugWritePortData(_In_ PIRP pIrp,
                                                 _In_ TPepCtrlPortData* pPortData,
                                                 _In_ const PVOID pvInBuf,
                                                 _In_ ULONG ulInBufLen,
                                                 _Out_writes_(ulOutBufLen) PVOID pvOutBuf,
                                                 _In_ ULONG ulOutBufLen);

#endif /* !defined(PepCtrlDeviceControl_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2024 Kevin Eshbach                                  */
/***************************************************************************/
