/***************************************************************************/
/*  Copyright (C) 2006-2021 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(ParallelPortPepCtrl_H)
#define ParallelPortPepCtrl_H

BOOL ParallelPortPepCtrlInitialize(_In_ TUtPepCtrlDeviceChangeFunc pDeviceChangeFunc);
BOOL ParallelPortPepCtrlUninitialize(VOID);

_Success_(return)
BOOL ParallelPortPepCtrlGetDeviceName(_Out_ LPWSTR pszDeviceName, _Out_ LPINT pnDeviceNameLen);

BOOL ParallelPortPepCtrlSetDeviceName(_Const_ _In_ LPCWSTR pszDeviceName);

BOOL ParallelPortPepCtrlSetDelaySettings(_In_ UINT32 nChipEnableNanoSeconds, _In_ UINT32 nOutputEnableNanoSeconds);

_Success_(return)
BOOL ParallelPortPepCtrlIsDevicePresent(_Out_writes_(sizeof(BOOL)) LPBOOL pbPresent);

BOOL ParallelPortPepCtrlReset(VOID);

BOOL ParallelPortPepCtrlSetProgrammerMode(_In_ UINT32 nProgrammerMode);

BOOL ParallelPortPepCtrlSetVccMode(_In_ UINT32 nVccMode);

BOOL ParallelPortPepCtrlSetPinPulseMode(_In_ UINT32 nPinPulseMode);

BOOL ParallelPortPepCtrlSetVppMode(_In_ UINT32 nVppMode);

_Success_(return)
BOOL ParallelPortPepCtrlReadData(_In_ UINT32 nAddress, _Out_writes_(nDataLen) LPBYTE pbyData, _In_ UINT32 nDataLen);

_Success_(return)
BOOL ParallelPortPepCtrlReadUserData(_Const_ _In_reads_(nReadUserDataLen) const TUtPepCtrlReadUserData* pReadUserData, _In_ UINT32 nReadUserDataLen, _Out_writes_(nDataLen) LPBYTE pbyData, _In_ UINT32 nDataLen);

_Success_(return)
BOOL ParallelPortPepCtrlReadUserDataWithDelay(_Const_ _In_reads_(nReadUserDataWithDelayLen) const TUtPepCtrlReadUserDataWithDelay* pReadUserDataWithDelay, _In_ UINT32 nReadUserDataWithDelayLen, _Out_writes_(nDataLen) LPBYTE pbyData, _In_ UINT32 nDataLen);

BOOL ParallelPortPepCtrlProgramData(_In_ UINT nAddress, _Const_ _In_reads_(nDataLen) LPBYTE pbyData, _In_ UINT32 nDataLen);

BOOL ParallelPortPepCtrlProgramUserData(_Const_ _In_reads_(nProgramUserDataLen) const TUtPepCtrlProgramUserData* pProgramUserData, _In_ UINT32 nProgramUserDataLen, _Const_ _In_reads_(nDataLen) LPBYTE pbyData, _In_ UINT32 nDataLen);

#endif /* ParallelPOrtPepCtrl_H */

/***************************************************************************/
/*  Copyright (C) 2006-2021 Kevin Eshbach                                  */
/***************************************************************************/
