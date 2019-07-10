/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepCtrlPlugPlay_H)
#define PepCtrlPlugPlay_H

#define TPEPCTRLPLUGPLAYAPI __stdcall

typedef BOOLEAN (TPEPCTRLPLUGPLAYAPI *TPepCtrlPlugPlayDeviceArrivedFunc)(_In_ PDEVICE_OBJECT pDeviceObject);
typedef VOID (TPEPCTRLPLUGPLAYAPI *TPepCtrlPlugPlayDeviceRemovedFunc)(_In_ PDEVICE_OBJECT pDeviceObject);

PVOID PepCtrlPlugPlayAlloc(_In_ PDRIVER_OBJECT pDriverObject,
                           _In_ PDEVICE_OBJECT pDeviceObject,
                           _In_ TPepCtrlPlugPlayDeviceArrivedFunc pDeviceArrivedFunc,
                           _In_ TPepCtrlPlugPlayDeviceRemovedFunc pDeviceRemovedFunc);

VOID PepCtrlPlugPlayFree(PVOID pvData);

BOOLEAN PepCtrlPlugPlayRegister(_In_ LPGUID pGuid,
                                _In_ PCWSTR pszDeviceName,
                                _In_ PVOID pvData);

BOOLEAN PepCtrlPlugPlayUnregister(_In_ PVOID pvData);

BOOLEAN PepCtrlPlugPlayIsDevicePresent(_In_ PVOID pvData);

#endif /* !defined(PepCtrlPlugPlay_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
