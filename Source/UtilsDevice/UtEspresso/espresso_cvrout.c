/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#include "espresso_hdrs.h"

#include "espresso_defs.h"
#include "espresso_cvrout.h"
#include "espresso_cvrin.h"

void makeup_labels(
  TPCubeContext pCubeContext,
  TPPLA PLA)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TEspressoInt32 var, i, ind;
    TEspressoInt32 nLabelLen = 15;

    if (PLA->ppszLabel == NULL)
    {
	    PLA_labels(pCubeContext, PLA);
    }

    for(var = 0; var < pCubeSettings->nNum_Vars; var++)
    {
	    for(i = 0; i < pCubeSettings->pnPart_Size[var]; i++)
        {
	        ind = pCubeSettings->pnFirst_Part[var] + i;

	        if (PLA->ppszLabel[ind] == NULL)
            {
		        PLA->ppszLabel[ind] = (LPTSTR)UtAllocMem(sizeof(TCHAR) * nLabelLen);

		        if (var < pCubeSettings->nNum_Binary_Vars)
                {
		            if ((i % 2) == 0)
                    {
			            StringCchPrintf(PLA->ppszLabel[ind], nLabelLen, TEXT("v%d.bar"), var);
                    }
		            else
                    {
			            StringCchPrintf(PLA->ppszLabel[ind], nLabelLen, TEXT("v%d"), var);
                    }
                }
		        else
                {
		            StringCchPrintf(PLA->ppszLabel[ind], nLabelLen, TEXT("v%d.%d"), var, i);
                }
	        }
	    }
    }
}

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
