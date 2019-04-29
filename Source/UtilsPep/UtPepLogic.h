/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtPepLogic_H)
#define UtPepLogic_H

#include <Includes/UtExternC.h>

#include <UtilsPep/UtPepLogicDefs.h>

MExternC PVOID TUTPEPLOGICAPI UtPepLogicAllocLogicContext();
MExternC VOID TUTPEPLOGICAPI UtPepLogicFreeLogicContext(_In_ PVOID pvLogicContext);
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicSetProgrammerMode(_In_ TUtPepLogicData* pLogicData,
                                                            _In_ UINT32 nProgrammerMode);
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicSetVccMode(_In_ TUtPepLogicData* pLogicData,
                                                     _In_ UINT32 nVccMode);
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicSetPinPulseMode(_In_ TUtPepLogicData* pLogicData,
                                                          _In_ UINT32 nPinMode);
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicSetVppMode(_In_ TUtPepLogicData* pLogicData,
                                                     _In_ UINT32 nVppMode);
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicSetAddress(_In_ TUtPepLogicData* pLogicData,
                                                     _In_ UINT32 nAddress);
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicGetData(_In_ TUtPepLogicData* pLogicData,
                                                  _Out_ UINT8* pnData);
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicSetData(_In_ TUtPepLogicData* pLogicData,
                                                  _In_ UINT8 nData);
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicTriggerProgram(_In_ TUtPepLogicData* pLogicData,
                                                         _Out_ PBOOLEAN pbSuccess);
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicSetOutputEnable(_In_ TUtPepLogicData* pLogicData,
                                                          _In_ UINT32 nOutputEnable);
MExternC BOOLEAN TUTPEPLOGICAPI UtPepLogicReset(_In_ TUtPepLogicData* pLogicData);

#endif /* !defined(UtPepLogic_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
