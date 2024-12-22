/***************************************************************************/
/*  Copyright (C) 2006-2024 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UsbPepCtrl_H)
#define UsbPepCtrl_H

BOOL UsbPepCtrlInitialize(_In_ TUtPepCtrlDeviceChangeFunc pDeviceChangeFunc);
BOOL UsbPepCtrlUninitialize(VOID);

_Success_(return)
BOOL UsbPepCtrlGetDeviceName(_Out_ LPWSTR pszDeviceName, _Out_ LPINT pnDeviceNameLen);

BOOL UsbPepCtrlSetDeviceName(_Const_ _In_ LPCWSTR pszDeviceName);

BOOL UsbPepCtrlSetDelaySettings(_In_ UINT32 nChipEnableNanoSeconds, _In_ UINT32 nOutputEnableNanoSeconds);

_Success_(return)
BOOL UsbPepCtrlIsDevicePresent(_Out_writes_(sizeof(BOOL)) LPBOOL pbPresent);

BOOL UsbPepCtrlReset(VOID);

BOOL UsbPepCtrlSetProgrammerMode(_In_ UINT32 nProgrammerMode);

BOOL UsbPepCtrlSetVccMode(_In_ UINT32 nVccMode);

BOOL UsbPepCtrlSetPinPulseMode(_In_ UINT32 nPinPulseMode);

BOOL UsbPepCtrlSetVppMode(_In_ UINT32 nVppMode);

_Success_(return)
BOOL UsbPepCtrlReadData(_In_ UINT32 nAddress, _Out_writes_(nDataLen) LPBYTE pbyData, _In_ UINT32 nDataLen);

_Success_(return)
BOOL UsbPepCtrlReadUserData(_Const_ _In_reads_(nReadUserDataLen) const TUtPepCtrlReadUserData* pReadUserData, _In_ UINT32 nReadUserDataLen, _Out_writes_(nDataLen) LPBYTE pbyData, _In_ UINT32 nDataLen);

_Success_(return)
BOOL UsbPepCtrlReadUserDataWithDelay(_Const_ _In_reads_(nReadUserDataWithDelayLen) const TUtPepCtrlReadUserDataWithDelay* pReadUserDataWithDelay, _In_ UINT32 nReadUserDataWithDelayLen, _Out_writes_(nDataLen) LPBYTE pbyData, _In_ UINT32 nDataLen);

BOOL UsbPepCtrlProgramData(_In_ UINT nAddress, _Const_ _In_reads_(nDataLen) LPBYTE pbyData, _In_ UINT32 nDataLen);

BOOL UsbPepCtrlProgramUserData(_Const_ _In_reads_(nProgramUserDataLen) const TUtPepCtrlProgramUserData* pProgramUserData, _In_ UINT32 nProgramUserDataLen, _Const_ _In_reads_(nDataLen) LPBYTE pbyData, _In_ UINT32 nDataLen);

BOOL UsbPepCtrlDebugWritePortData(_In_ UINT8 nWritePortData);

#endif /* UsbPepCtrl_H */

/***************************************************************************/
/*  Copyright (C) 2006-2024 Kevin Eshbach                                  */
/***************************************************************************/
