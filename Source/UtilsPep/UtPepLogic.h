/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtPepLogic_H)
#define UtPepLogic_H

#include <Includes/UtExternC.h>

#include <UtilsPep/UtPepLogicDefs.h>

_IRQL_requires_max_(PASSIVE_LEVEL)
MExternC PVOID TUTPEPLOGICAPI UtPepLogicAllocLogicContext();

_IRQL_requires_max_(PASSIVE_LEVEL)
MExternC VOID TUTPEPLOGICAPI UtPepLogicFreeLogicContext(_In_ PVOID pvLogicContext);

_IRQL_requires_max_(PASSIVE_LEVEL)
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicSetProgrammerMode(_In_ TUtPepLogicData* pLogicData,
                                                            _In_ UINT32 nProgrammerMode);

_IRQL_requires_max_(PASSIVE_LEVEL)
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicSetVccMode(_In_ TUtPepLogicData* pLogicData,
                                                     _In_ UINT32 nVccMode);

_IRQL_requires_max_(PASSIVE_LEVEL)
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicSetPinPulseMode(_In_ TUtPepLogicData* pLogicData,
                                                          _In_ UINT32 nPinMode);

_IRQL_requires_max_(PASSIVE_LEVEL)
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicSetVppMode(_In_ TUtPepLogicData* pLogicData,
                                                     _In_ UINT32 nVppMode);

_IRQL_requires_max_(PASSIVE_LEVEL)
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicSetAddress(_In_ TUtPepLogicData* pLogicData,
                                                     _In_ UINT32 nAddress);

_IRQL_requires_max_(PASSIVE_LEVEL)
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicSetAddressWithDelay(_In_ TUtPepLogicData* pLogicData,
	                                                          _In_ UINT32 nAddress,
	                                                          _In_ UINT32 nDelayNanoSeconds);

_IRQL_requires_max_(PASSIVE_LEVEL)
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicGetData(_In_ TUtPepLogicData* pLogicData,
                                                  _Out_ UINT8* pnData);

_IRQL_requires_max_(PASSIVE_LEVEL)
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicSetData(_In_ TUtPepLogicData* pLogicData,
                                                  _In_ UINT8 nData);

_IRQL_requires_max_(PASSIVE_LEVEL)
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicTriggerProgram(_In_ TUtPepLogicData* pLogicData,
                                                         _Out_ PBOOLEAN pbSuccess);

_IRQL_requires_max_(PASSIVE_LEVEL)
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicSetOutputEnable(_In_ TUtPepLogicData* pLogicData,
                                                          _In_ UINT32 nOutputEnable);

_IRQL_requires_max_(PASSIVE_LEVEL)
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicReset(_In_ TUtPepLogicData* pLogicData);

_IRQL_requires_max_(PASSIVE_LEVEL)
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicSetDelays(_In_ TUtPepLogicData* pLogicData,
                                                    _In_ UINT32 nChipEnableNanoSeconds,
                                                    _In_ UINT32 nOutputEnableNanoSeconds);

#endif /* !defined(UtPepLogic_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2020 Kevin Eshbach                                  */
/***************************************************************************/
