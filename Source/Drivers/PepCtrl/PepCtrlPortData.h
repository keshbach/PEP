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

#define CPepCtrlStateRunning            0x0001
#define CPepCtrlStateDeviceControl      0x0002
#define CPepCtrlStateUnloading          0x0003
#define CPepCtrlStateChangePortSettings 0x0004 

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
typedef LPGUID(TPEPCTRLAPI *TPepCtrlGetDevInterfaceGuidFunc)(VOID);

typedef struct tagTPepCtrlFuncs
{
    TPepCtrlAllocPortFunc pAllocPortFunc;
    TPepCtrlFreePortFunc pFreePortFunc;
    TPepCtrlReadBitPortFunc pReadBitPortFunc;
    TPepCtrlWritePortFunc pWritePortFunc;
    TPepCtrlGetDevInterfaceGuidFunc pGetDeviceInterfaceGuidFunc;
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
    PDRIVER_OBJECT pDriverObject;
    PDEVICE_OBJECT pDeviceObject;
    FAST_MUTEX FastMutex;
    INT32 nState;
    PIRP pIrp; /* IRP used to indicate when port arrives/removed */
    TPepCtrlRegSettings RegSettings;
    TPepCtrlFuncs Funcs;
    TPepCtrlObject Object;
    TUtPepLogicData LogicData;
    PVOID pvPlugPlayData; /* Instance data for the Plug and Play */
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
