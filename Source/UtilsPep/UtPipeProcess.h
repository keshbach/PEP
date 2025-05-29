/***************************************************************************/
/*  Copyright (C) 2006-2025 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtPipeProcess_H)
#define UtPipeProcess_H

#include <Includes/UtExternC.h>

#if defined(_MSC_VER)
#pragma pack(push, 4)
#else
#error Need to specify how to enable byte aligned structure padding
#endif

typedef PVOID TUtPipeProcessHandle;

#define UTPIPEPROCESSAPI __stdcall

#define UTPIPEPROCESSCALLBACK __stdcall

typedef VOID (UTPIPEPROCESSCALLBACK* TUtPipeProcessStatusFunc)(LPVOID pvTagData);
typedef VOID (UTPIPEPROCESSCALLBACK* TUtPipeProcessDataFunc)(LPVOID pvTagData, LPCSTR pszData, UINT32 nDataLen);

typedef struct tagTPipeProcessFuncs
{
    TUtPipeProcessStatusFunc pBeginStatusFunc;
    TUtPipeProcessStatusFunc pEndStatusFunc;
    TUtPipeProcessDataFunc pOutputDataFunc;
    TUtPipeProcessDataFunc pErrorDataFunc;
} TPipeProcessFuncs;

MExternC BOOL UTPIPEPROCESSAPI UtPipeProcessInitialize(VOID);

MExternC BOOL UTPIPEPROCESSAPI UtPipeProcessUninitialize(VOID);

MExternC TUtPipeProcessHandle UTPIPEPROCESSAPI UtPipeProcessLaunch(LPCWSTR pszApp,
                                                                   LPCWSTR pszArguments,
                                                                   LPVOID pvTagData,
                                                                   const TPipeProcessFuncs* pPipeProcessFuncs);

MExternC BOOL UTPIPEPROCESSAPI UtPipeProcessPostData(TUtPipeProcessHandle hPipeProcess,
                                                     LPCSTR pszData,
                                                     UINT32 nDataLen);

MExternC BOOL UTPIPEPROCESSAPI UtPipeProcessCancel(TUtPipeProcessHandle hPipeProcess);

MExternC BOOL UTPIPEPROCESSAPI UtPipeProcessTerminate(TUtPipeProcessHandle hPipeProcess);

MExternC BOOL UTPIPEPROCESSAPI UtPipeProcessClose(TUtPipeProcessHandle hPipeProcess);

#if defined(_MSC_VER)
#pragma pack(pop)
#else
#error Need to specify how to restore original structure padding
#endif

#endif /* end of UtPipeProcess_H */

/***************************************************************************/
/*  Copyright (C) 2006-2025 Kevin Eshbach                                  */
/***************************************************************************/
