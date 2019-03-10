/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#include "espresso_hdrs.h"

#include "espresso_defs.h"
#include "espresso_sharp.h"
#include "espresso_set.h"
#include "espresso_setc.h"
#include "espresso_contain.h"

/* cv_intersect -- form the intersection of two covers */

#define CMagic 500               /* save 500 cubes before containment */

TPCover cv_intersect(
  TPCubeContext pCubeContext,
  TPCover A,
  TPCover B)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TPCube pi, pj, lasti, lastj, pt;
    TPCover T, Tsave = NULL;

    /* How large should each temporary result cover be ? */
    T = MNew_Cover(pCubeContext, CMagic, pCubeSettings);
    pt = T->pData;

    /* Form pairwise intersection of each cube of A with each cube of B */
    MForeach_Set(A, lasti, pi)
    {
	    MForeach_Set(B, lastj, pj)
        {
	        if (cdist0(pCubeContext, pi, pj))
            {
		        set_and(pt, pi, pj);
		        if (++T->nCount >= T->nCapacity)
                {
		            if (Tsave == NULL)
                    {
    			        Tsave = sf_contain(pCubeContext, T);
                    }
		            else
                    {
	    		        Tsave = sf_union(pCubeContext, Tsave, sf_contain(pCubeContext, T));
                    }

		            T = MNew_Cover(pCubeContext, CMagic, pCubeSettings);
		            pt = T->pData;
		        }
                else
                {
		            pt += T->nWSize;
                }
	        }
	    }
    }

    if (Tsave == NULL)
    {
	    Tsave = sf_contain(pCubeContext, T);
    }
    else
    {
	    Tsave = sf_union(pCubeContext, Tsave, sf_contain(pCubeContext, T));
    }

    return Tsave;
}

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
