/***************************************************************************/
/*  Copyright (C) 2012-2012 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtEspresso_H)
#define UtEspresso_H

#include <Includes/UtExternC.h>

#define UTESPRESSOAPI __stdcall

MExternC LPWSTR* UTESPRESSOAPI Espresso_AllocMinimizedTerms(LPCWSTR* ppszTerms);
MExternC VOID UTESPRESSOAPI Espresso_FreeMinimizedTerms(LPWSTR* ppszTerms);

#endif /* end of UtEspresso_H */

/***************************************************************************/
/*  Copyright (C) 2012-2012 Kevin Eshbach                                  */
/***************************************************************************/
