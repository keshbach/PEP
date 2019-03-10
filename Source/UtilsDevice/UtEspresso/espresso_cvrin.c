/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#include "espresso_hdrs.h"

#include "espresso_defs.h"
#include "espresso_cvrin.h"
#include "espresso_set.h"

TPPLA new_PLA()
{
    TPPLA PLA;

    PLA = (TPPLA)UtAllocMem(sizeof(TPLA) * 1);

    PLA->pF = PLA->pD = PLA->pR = NULL;
    PLA->pPhase = NULL;
    PLA->pPair = NULL;
    PLA->ppszLabel = NULL;
    PLA->nPLA_Type = 0;
    PLA->pSymbolic = NULL;
    PLA->pSymbolic_Output = NULL;

    return PLA;
}

void PLA_labels(
  TPCubeContext pCubeContext,
  TPPLA PLA)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TEspressoInt32 i;

    PLA->ppszLabel = (LPTSTR*)UtAllocMem(sizeof(LPTSTR) * pCubeSettings->nSize);

    for(i = 0; i < pCubeSettings->nSize; i++)
    {
	    PLA->ppszLabel[i] = NULL;
    }
}

void free_PLA(
  TPCubeContext pCubeContext,
  TPPLA PLA)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TSymbolic_List *p2, *p2next;
    TSymbolic *p1, *p1next;
    TEspressoInt32 i;

    if (PLA->pF != NULL)
    {
    	MFree_Cover(pCubeContext, PLA->pF);
    }
             
    if (PLA->pR != NULL)
    {
	    MFree_Cover(pCubeContext, PLA->pR);
    }

    if (PLA->pD != NULL)
    {
	    MFree_Cover(pCubeContext, PLA->pD);
    }

    if (PLA->pPhase != NULL)
    {
	    MFree_Cube(PLA->pPhase);
    }

    if (PLA->pPair != NULL)
    {
        if (PLA->pPair->pnVar1)
        {
            UtFreeMem(PLA->pPair->pnVar1);
        }

        if (PLA->pPair->pnVar2)
        {
            UtFreeMem(PLA->pPair->pnVar2);
        }

        if (PLA->pPair)
        {
            UtFreeMem(PLA->pPair);
        }
    }

    if (PLA->ppszLabel != NULL)
    {
	    for(i = 0; i < pCubeSettings->nSize; i++)
        {
	        if (PLA->ppszLabel[i] != NULL)
            {
                UtFreeMem(PLA->ppszLabel[i]);
            }
        }

        if (PLA->ppszLabel)
        {
            UtFreeMem(PLA->ppszLabel);
        }
    }

    for(p1 = PLA->pSymbolic; p1 != NULL; p1 = p1next)
    {
	    for(p2 = p1->pSymbolic_List; p2 != NULL; p2 = p2next)
        {
	        p2next = p2->pNext;

            if (p2)
            {
                UtFreeMem(p2);
            }
	    }
	    p1next = p1->pNext;

        if (p1)
        {
            UtFreeMem(p1);
        }
    }

    PLA->pSymbolic = NULL;

    for(p1 = PLA->pSymbolic_Output; p1 != NULL; p1 = p1next) 
    {
	    for(p2 = p1->pSymbolic_List; p2 != NULL; p2 = p2next)
        {
	        p2next = p2->pNext;

            if (p2)
            {
                UtFreeMem(p2);
            }
	    }

	    p1next = p1->pNext;

        if (p1)
        {
            UtFreeMem(p1);
        }
    }

    PLA->pSymbolic_Output = NULL;

    if (PLA)
    {
        UtFreeMem(PLA);
    }
}

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
