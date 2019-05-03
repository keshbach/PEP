/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepCtrlPlugPlay_H)
#define PepCtrlPlugPlay_H

NTSTATUS PepCtrlPlugPlayDeviceInterfaceChange(_In_ PVOID pvNotificationStructure, _In_ PVOID pvContext);

VOID PepCtrlPlugPlayClosePortThreads(_In_ TPepCtrlPortData* pPortData);

#endif /* !defined(PepCtrlPlugPlay_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
