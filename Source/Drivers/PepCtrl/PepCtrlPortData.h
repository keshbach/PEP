/***************************************************************************/
/*  Copyright (C) 2007-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepCtrlPortData_H)
#define PepCtrlPortData_H

#include <UtilsPep/UtPepLogicDefs.h>

#if defined(_MSC_VER)
#if defined(_X86_)
#pragma pack(push, 4)
#elif defined(_WIN64)
#pragma pack(push, 8)
#else
#error Need to specify cpu architecture to configure structure padding
#endif
#else
#error Need to specify how to enable byte aligned structure padding
#endif

struct tagTPepCtrlObject;

#define TPEPCTRLAPI __stdcall

typedef BOOLEAN (TPEPCTRLAPI *TPepCtrlAllocPortFunc)(_In_ struct tagTPepCtrlObject* pPortData,
                                                    _In_ LPCWSTR pszDeviceName);
typedef BOOLEAN (TPEPCTRLAPI *TPepCtrlFreePortFunc)(_In_ struct tagTPepCtrlObject* pPortData);
typedef BOOLEAN (TPEPCTRLAPI *TPepCtrlReadBitPortFunc)(_In_ struct tagTPepCtrlObject* pPortData,
                                                       _Out_ PBOOLEAN pbValue);
typedef BOOLEAN (TPEPCTRLAPI *TPepCtrlWritePortFunc)(_In_ struct tagTPepCtrlObject* pPortData,
                                                     _In_ PUCHAR pucData,
                                                     _In_ ULONG ulDataLen);
typedef LPGUID (TPEPCTRLAPI *TPepCtrlGetDevInterfaceGuidFunc)(VOID);

typedef struct tagTPepCtrlFuncs
{
    TPepCtrlAllocPortFunc pAllocPortFunc;
    TPepCtrlFreePortFunc pFreePortFunc;
    TPepCtrlReadBitPortFunc pReadBitPortFunc;
    TPepCtrlWritePortFunc pWritePortFunc;
    TPepCtrlGetDevInterfaceGuidFunc pGetDevInterfaceGuidFunc;
} TPepCtrlFuncs;

typedef struct tagTPepCtrlObject
{
    PFILE_OBJECT pPortFileObject;
    PDEVICE_OBJECT pPortDeviceObject;
    PVOID pvObjectData; /* Port object specific data */
} TPepCtrlObject;

typedef struct tagTPepCtrlRegSettings
{
    LPWSTR pszRegistryPath;
    UINT32 nPortType;
    LPWSTR pszPortDeviceName;
} TPepCtrlRegSettings;

typedef struct tagTPepCtrlPortData
{
    FAST_MUTEX FastMutex;
    PVOID pvPnPNotificationEntry;
    HANDLE hPortArrivedThread;
    HANDLE hPortRemovedThread;
    PIRP pIrp; /* IRP used to indicate when port arrives/removed */
    BOOLEAN bPortEjected;
    TPepCtrlRegSettings RegSettings;
    TPepCtrlFuncs Funcs;
    TPepCtrlObject Object;
    TUtPepLogicData LogicData;
} TPepCtrlPortData;

#if defined(_MSC_VER)
#pragma pack(pop)
#else
#error Need to specify how to restore original structure padding
#endif

#endif /* !defined(PepCtrlPortData_H) */

/***************************************************************************/
/*  Copyright (C) 2007-2019 Kevin Eshbach                                  */
/***************************************************************************/
