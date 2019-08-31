/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepCtrlPlugPlay_H)
#define PepCtrlPlugPlay_H

#define TPEPCTRLPLUGPLAYAPI __stdcall

typedef BOOLEAN (TPEPCTRLPLUGPLAYAPI *TPepCtrlPlugPlayDeviceArrivedFunc)(_In_ PDEVICE_OBJECT pDeviceObject);
typedef VOID (TPEPCTRLPLUGPLAYAPI *TPepCtrlPlugPlayDeviceRemovedFunc)(_In_ PDEVICE_OBJECT pDeviceObject);

_IRQL_requires_max_(PASSIVE_LEVEL)
PVOID PepCtrlPlugPlayAlloc(_In_ PDRIVER_OBJECT pDriverObject,
                           _In_ PDEVICE_OBJECT pDeviceObject,
                           _In_ TPepCtrlPlugPlayDeviceArrivedFunc pDeviceArrivedFunc,
                           _In_ TPepCtrlPlugPlayDeviceRemovedFunc pDeviceRemovedFunc);

_IRQL_requires_max_(PASSIVE_LEVEL)
VOID PepCtrlPlugPlayFree(PVOID pvData);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN PepCtrlPlugPlayRegister(_In_ LPGUID pGuid,
                                _In_ PCWSTR pszDeviceName,
                                _In_ PVOID pvData);

_IRQL_requires_max_(PASSIVE_LEVEL)
BOOLEAN PepCtrlPlugPlayUnregister(_In_ PVOID pvData);

_IRQL_requires_max_(APC_LEVEL)
BOOLEAN PepCtrlPlugPlayIsDevicePresent(_In_ PVOID pvData);

#endif /* !defined(PepCtrlPlugPlay_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
