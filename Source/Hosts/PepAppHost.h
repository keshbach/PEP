/***************************************************************************/
/*  Copyright (C) 2019-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(PepAppHost_H)
#define PepAppHost_H

#include <Includes/UtExternC.h>

#define PEPAPPHOSTAPI __stdcall

MExternC BOOL PEPAPPHOSTAPI PepAppHostInitialize(VOID);

MExternC BOOL PEPAPPHOSTAPI PepAppHostUninitialize(VOID);

MExternC BOOL PEPAPPHOSTAPI PepAppHostExecute(_Out_ LPDWORD pdwExitCode);

#endif /* end of PepAppHost_H */

/***************************************************************************/
/*  Copyright (C) 2019-2019 Kevin Eshbach                                  */
/***************************************************************************/
