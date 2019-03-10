/***************************************************************************/
/*  Copyright (C) 2007-2013 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtPalDeviceDump_H)
#define UtPalDeviceDump_H

VOID UtPALDeviceDumpTerms(LPCWSTR* ppszTerms);

VOID UtPALDeviceDumpOutputData(ULONG* pulInputData,
                               TPALDeviceAdapterData* pOutputData,
                               ULONG ulTotalData,
                               BOOL bIncludeHeader);

#endif /* end of UtPalDeviceDump_H */

/***************************************************************************/
/*  Copyright (C) 2007-2013 Kevin Eshbach                                  */
/***************************************************************************/
