/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#include "espresso_hdrs.h"

#include "espresso_defs.h"
#include "espresso_opo.h"
#include "espresso_set.h"

/*
 *  set_phase -- given a "cube" which describes which phases of the output
 *  are to be implemented, compute the appropriate on-set and off-set
 */
TPPLA set_phase(
  TPCubeContext pCubeContext,
  /*INOUT*/ TPPLA PLA)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TPCover F1, R1;
    register TPCube last, p, outmask;
    register TPCube temp=pCubeSettings->ppTemp[0], phase=PLA->pPhase, phase1=pCubeSettings->ppTemp[1];

    outmask = pCubeSettings->ppVar_Mask[pCubeSettings->nNum_Vars - 1];
    set_diff(phase1, outmask, phase);
    set_or(phase1, set_diff(temp, pCubeSettings->pFullSet, outmask), phase1);
    F1 = MNew_Cover(pCubeContext, (PLA->pF)->nCount + (PLA->pR)->nCount, pCubeSettings);
    R1 = MNew_Cover(pCubeContext, (PLA->pF)->nCount + (PLA->pR)->nCount, pCubeSettings);

    MForeach_Set(PLA->pF, last, p)
    {
	    if (! setp_disjoint(set_and(temp, p, phase), outmask))
        {
	        set_copy(MGetSet(F1, F1->nCount++), temp);
        }
	    if (! setp_disjoint(set_and(temp, p, phase1), outmask))
        {
	        set_copy(MGetSet(R1, R1->nCount++), temp);
        }
    }

    MForeach_Set(PLA->pR, last, p)
    {
	    if (! setp_disjoint(set_and(temp, p, phase), outmask))
        {
	        set_copy(MGetSet(R1, R1->nCount++), temp);
        }
	    if (! setp_disjoint(set_and(temp, p, phase1), outmask))
        {
	        set_copy(MGetSet(F1, F1->nCount++), temp);
        }
    }

    MFree_Cover(pCubeContext, PLA->pF);
    MFree_Cover(pCubeContext, PLA->pR);

    PLA->pF = F1;
    PLA->pR = R1;

    return PLA;
}

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
