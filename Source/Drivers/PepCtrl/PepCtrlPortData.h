/***************************************************************************/
/*  Copyright (C) 2007-2013 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepCtrlPortData_H)
#define PepCtrlPortData_H

#if defined(_MSC_VER)
#pragma pack(push, 4)
#else
#error Need to specify how to enable byte aligned structure padding
#endif

struct tagTPepCtrlObject;

#define TPEPCTRLAPI __stdcall

typedef BOOLEAN (TPEPCTRLAPI *TPepCtrlAllocPortFunc)(IN struct tagTPepCtrlObject* pPortData,
                                                     IN LPCWSTR pszDeviceName);
typedef BOOLEAN (TPEPCTRLAPI *TPepCtrlFreePortFunc)(IN struct tagTPepCtrlObject* pPortData);
typedef BOOLEAN (TPEPCTRLAPI *TPepCtrlReadBitPortFunc)(IN struct tagTPepCtrlObject* pPortData,
                                                       OUT PBOOLEAN pbValue);
typedef BOOLEAN (TPEPCTRLAPI *TPepCtrlWritePortFunc)(IN struct tagTPepCtrlObject* pPortData,
                                                     IN PUCHAR pucData,
                                                     IN ULONG ulDataLen);
typedef LPGUID (TPEPCTRLAPI *TPepCtrlGetDevInterfaceGuidFunc)(VOID);

typedef struct tagTPepCtrlFuncs
{
    TPepCtrlAllocPortFunc pAllocPortFunc;
    TPepCtrlFreePortFunc pFreePortFunc;
    TPepCtrlReadBitPortFunc pReadBitPortFunc;
    TPepCtrlWritePortFunc pWritePortFunc;
    TPepCtrlGetDevInterfaceGuidFunc pGetDevInterfaceGuidFunc;
} TPepCtrlFuncs;

typedef struct tagTPepCtrlModes
{
    UINT32 nProgrammerMode;
    UINT32 nVccMode;
    UINT32 nPinPulseMode;
    UINT32 nVppMode;
} TPepCtrlModes;

typedef struct tagTPepCtrlObject
{
    PFILE_OBJECT pPortFileObject;
    PDEVICE_OBJECT pPortDeviceObject;
    PVOID pvObjectData; /* Port object specific data */
} TPepCtrlObject;

typedef struct tagTPepCtrlPortData
{
    FAST_MUTEX FastMutex;
    LPWSTR pszPortDeviceName;
    PVOID pvPnPNotificationEntry;
    HANDLE hPortArrivedThread;
    HANDLE hPortRemovedThread;
    PIRP pIrp; /* IRP used to indicate when port arrives/removed */
    BOOLEAN bPortEjected;
    UINT32 nLastAddress; /* Last address that was set */
    TPepCtrlFuncs Funcs;
    TPepCtrlModes Modes;
    TPepCtrlObject Object;
} TPepCtrlPortData;

#if defined(_MSC_VER)
#pragma pack(pop)
#else
#error Need to specify how to restore original structure padding
#endif

#endif /* !defined(PepCtrlPortData_H) */

/***************************************************************************/
/*  Copyright (C) 2007-2013 Kevin Eshbach                                  */
/***************************************************************************/
