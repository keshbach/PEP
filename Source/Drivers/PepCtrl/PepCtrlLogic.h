/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepCtrlLogic_H)
#define PepCtrlLogic_H

BOOLEAN PepCtrlSetProgrammerMode(IN TPepCtrlPortData* pPortData,
                                 IN UINT32 nProgrammerMode);
BOOLEAN PepCtrlSetVccMode(IN TPepCtrlPortData* pPortData,
                          IN UINT32 nVccMode);
BOOLEAN PepCtrlSetPinPulseMode(IN TPepCtrlPortData* pPortData,
                               IN UINT32 nPinMode);
BOOLEAN PepCtrlSetVppMode(IN TPepCtrlPortData* pPortData,
                          IN UINT32 nVppMode);
BOOLEAN PepCtrlSetAddress(IN TPepCtrlPortData* pPortData,
                          IN UINT32 nAddress);
BOOLEAN PepCtrlGetData(IN TPepCtrlPortData* pPortData,
                       OUT UINT8* pnData);
BOOLEAN PepCtrlSetData(IN TPepCtrlPortData* pPortData,
                       IN UINT8 nData);
BOOLEAN PepCtrlTriggerProgram(IN TPepCtrlPortData* pPortData,
                              OUT PBOOLEAN pbSuccess);
BOOLEAN PepCtrlSetOutputEnable(IN TPepCtrlPortData* pPortData,
                               IN UINT32 nOutputEnable);
BOOLEAN PepCtrlReset(IN TPepCtrlPortData* pPortData);
BOOLEAN PepCtrlInitModes(IN TPepCtrlPortData* pPortData);

#endif /* !defined(PepCtrlLogic_H) */

/***************************************************************************/
/*  Copyright (C) 2006-2019 Kevin Eshbach                                  */
/***************************************************************************/
