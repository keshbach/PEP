/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepCtrlPlugPlay_H)
#define PepCtrlPlugPlay_H

NTSTATUS PepCtrlPlugPlayDeviceInterfaceChange(IN PVOID pvNotificationStructure, IN PVOID pvContext);

VOID PepCtrlPlugPlayClosePortThreads(IN TPepCtrlPortData* pPortData);

#endif /* !defined(PepCtrlPlugPlay_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
