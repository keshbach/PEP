/***************************************************************************/
/*  Copyright (C) 2009-2019 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtConsole_H)
#define UtConsole_H

#include <Includes/UtExternC.h>

#define UTCONSOLEAPI __stdcall

MExternC BOOL UTCONSOLEAPI UtConsolePrintDipSwitches(LPBOOL pbDipSwitches, ULONG ulTotalDipSwitches);

MExternC BOOL UTCONSOLEAPI UtConsolePrintAppVersion(VOID);

#endif /* end of UtConsole_H */

/***************************************************************************/
/*  Copyright (C) 2009-2019 Kevin Eshbach                                  */
/***************************************************************************/
