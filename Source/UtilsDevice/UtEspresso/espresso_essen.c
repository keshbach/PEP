/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#include "espresso_hdrs.h"

#include "espresso_defs.h"
#include "espresso_essen.h"
#include "espresso_set.h"
#include "espresso_setc.h"
#include "espresso_cofactor.h"
#include "espresso_sparse.h"
#include "espresso_irred.h"

/*
    essential -- return a cover consisting of the cubes of F which are
    essential prime implicants (with respect to F u D); Further, remove
    these cubes from the ON-set F, and add them to the OFF-set D.

    Sometimes EXPAND can determine that a cube is not an essential prime.
    If so, it will set the "NONESSEN" flag in the cube.

    We count on IRREDUNDANT to have set the flag RELESSEN to indicate
    that a prime was relatively essential (i.e., covers some minterm
    not contained in any other prime in the current cover), or to have
    reset the flag to indicate that a prime was relatively redundant
    (i.e., all minterms covered by other primes in the current cover).
    Of course, after executing irredundant, all of the primes in the
    cover are relatively essential, but we can mark the primes which
    were redundant at the start of irredundant and avoid an extra check
    on these primes for essentiality.
*/

TPCover essential(
  TPCubeContext pCubeContext,
  TPCover *Fp,
  TPCover *Dp)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TPCube last, p;
    TPCover E, F = *Fp, D = *Dp;

    /* set all cubes in F active */
    sf_active(F);

    /* Might as well start out with some cubes in E */
    E = MNew_Cover(pCubeContext, 10, pCubeSettings);

    MForeach_Set(F, last, p)
    {
	    /* don't test a prime which EXPAND says is nonessential */
	    if (! MTestP(p, CEspressoNoneEssen))
        {
	        /* only test a prime which was relatively essential */
	        if (MTestP(p, CEspressoRelEssen))
            {
		        /* Check essentiality */
		        if (essen_cube(pCubeContext, F, D, p))
                {
		            E = sf_addset(E, p);
		            MReset(p, CEspressoActive);
		            F->nActive_Count--;
		        }
	        }
	    }
    }

    *Fp = sf_inactive(F);               /* delete the inactive cubes from F */
    *Dp = sf_join(pCubeContext, D, E);		/* add the essentials to D */

    sf_free(pCubeContext, D);

    return E;
}

/*
    essen_cube -- check if a single cube is essential or not

    The prime c is essential iff

	consensus((F u D) # c, c) u D

    does not contain c.
*/
TEspressoBool essen_cube(
  TPCubeContext pCubeContext,
  const TPCover F,
  const TPCover D,
  const TPCube c)
{
    TPCover H, FD;
    TPCube *H1;
    TEspressoBool essen;

    /* Append F and D together, and take the sharp-consensus with c */
    FD = sf_join(pCubeContext, F, D);
    H = cb_consensus(pCubeContext, FD, c);
    MFree_Cover(pCubeContext, FD);

    /* Add the don't care set, and see if this covers c */
    H1 = cube2list(pCubeContext, H, D);
    essen = ! cube_is_covered(pCubeContext, H1, c);
    MFree_Cubelist(H1);

    MFree_Cover(pCubeContext, H);
    return essen;
}

/*
 *  cb_consensus -- compute consensus(T # c, c)
 */
TPCover cb_consensus(
  TPCubeContext pCubeContext,
  const TPCover T,
  const TPCube c)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TPCube temp, last, p;
    register TPCover R;

    R = MNew_Cover(pCubeContext, T->nCount*2, pCubeSettings);
    temp = MNew_Cube(pCubeSettings);
    MForeach_Set(T, last, p)
    {
	    if (p != c)
        {
	        switch (cdist01(pCubeContext, p, c))
            {
		        case 0:
		            /* distance-0 needs special care */
		            R = cb_consensus_dist0(pCubeContext, R, p, c);
		            break;
		        case 1:
		            /* distance-1 is easy because no sharping required */
		            consensus(pCubeContext, temp, p, c);
		            R = sf_addset(R, temp);
		            break;
	        }
	    }
    }
    MSet_Free(temp);
    return R;
}

/*
 *  form the sharp-consensus for p and c when they intersect
 *  What we are forming is consensus(p # c, c).
 */
TPCover cb_consensus_dist0(
  TPCubeContext pCubeContext,
  TPCover R,
  register TPCube p,
  register TPCube c)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TEspressoInt32 var;
    TEspressoBool got_one;
    register TPCube temp, mask;
    register TPCube p_diff_c=pCubeSettings->ppTemp[0], p_and_c=pCubeSettings->ppTemp[1];

    /* If c contains p, then this gives us no information for essential test */
    if (setp_implies(p, c))
    {
    	return R;
    }

    /* For the multiple-valued variables */
    temp = MNew_Cube(pCubeSettings);
    got_one = CEspressoFalse;
    MInlineSet_Diff(p_diff_c, p, c);
    MInlineSet_And(p_and_c, p, c);

    for(var = pCubeSettings->nNum_Binary_Vars; var < pCubeSettings->nNum_Vars; var++)
    {
	    /* Check if c(var) is contained in p(var) -- if so, no news */
	    mask = pCubeSettings->ppVar_Mask[var];
	    if (! setp_disjoint(p_diff_c, mask)) 
        {
	        MInlineSet_Merge(temp, c, p_and_c, mask);
	        R = sf_addset(R, temp);
	        got_one = CEspressoTrue;
	    }
    }

    /* if no cube so far, add one for the intersection */
    if (! got_one && pCubeSettings->nNum_Binary_Vars > 0)
    {
	    /* Add a single cube for the intersection of p and c */
	    MInlineSet_And(temp, p, c);
	    R = sf_addset(R, temp);
    }

    MSet_Free(temp);
    return R;
}

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
