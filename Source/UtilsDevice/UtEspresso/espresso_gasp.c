/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

/*
    module: gasp.c

    The "last_gasp" heuristic computes the reduction of each cube in
    the cover (without replacement) and then performs an expansion of
    these cubes.  The cubes which expand to cover some other cube are
    added to the original cover and irredundant finds a minimal subset.

    If one of the reduced cubes expands to cover some other reduced
    cube, then the new prime thus generated is a candidate for reducing
    the size of the cover.

    super_gasp is a variation on this strategy which extracts a minimal
    subset from the set of all prime implicants which cover all
    maximally reduced cubes.
*/

#include "espresso_hdrs.h"

#include "espresso_defs.h"
#include "espresso_gasp.h"
#include "espresso_set.h"
#include "espresso_cofactor.h"
#include "espresso_reduce.h"
#include "espresso_contain.h"
#include "espresso_expand.h"
#include "espresso_sparse.h"
#include "espresso_irred.h"
#include "espresso_cvrmisc.h"

/*
 *  reduce_gasp -- compute the maximal reduction of each cube of F
 *
 *  If a cube does not reduce, it remains prime; otherwise, it is marked
 *  as nonprime.   If the cube is redundant (should NEVER happen here) we
 *  just crap out ...
 *
 *  A cover with all of the cubes of F is returned.  Those that did
 *  reduce are marked "NONPRIME"; those that reduced are marked "PRIME".
 *  The cubes are in the same order as in F.
 */
static TPCover reduce_gasp(
  TPCubeContext pCubeContext,
  TPCover F,
  TPCover D)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TPCube p, last, cunder, *FD;
    TPCover G;

    G = MNew_Cover(pCubeContext, F->nCount, pCubeSettings);
    FD = cube2list(pCubeContext, F, D);

    /* Reduce cubes of F without replacement */
    MForeach_Set(F, last, p)
    {
	    cunder = reduce_cube(pCubeContext, FD, p);
	    if (setp_empty(cunder))
        {
            assert(0);
	        //fatal("empty reduction in reduce_gasp, shouldn't happen");
	    }
        else if (setp_equal(cunder, p))
        {
	        MSet(cunder, CEspressoPrime);			/* just to make sure */
	        G = sf_addset(G, p);		/* it did not reduce ... */
	    }
        else
        {
	        MReset(cunder, CEspressoPrime);		/* it reduced ... */
	        G = sf_addset(G, cunder);
	    }
	    MFree_Cube(cunder);
    }

    MFree_Cubelist(FD);
    return G;
}

/*
 *  expand_gasp -- expand each nonprime cube of F into a prime implicant
 *
 *  The gasp strategy differs in that only those cubes which expand to
 *  cover some other cube are saved; also, all cubes are expanded
 *  regardless of whether they become covered or not.
 */

TPCover expand_gasp(
  TPCubeContext pCubeContext,
   TPCover F,
  const TPCover D,
  const TPCover R,
  const TPCover Foriginal)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TEspressoInt32 c1index;
    TPCover G;

    /* Try to expand each nonprime and noncovered cube */
    G = MNew_Cover(pCubeContext, 10, pCubeSettings);
    for(c1index = 0; c1index < F->nCount; c1index++)
    {
	    expand1_gasp(pCubeContext, F, D, R, Foriginal, c1index, &G);
    }
    G = sf_dupl(pCubeContext, G);
    G = expand(pCubeContext, G, R, /*nonsparse*/ CEspressoFalse);	/* Make them prime ! */

    return G;
}



/*
 *  expand1 -- Expand a single cube against the OFF-set, using the gasp strategy
 */
void expand1_gasp(
  TPCubeContext pCubeContext,
  TPCover F,		/* reduced cubes of ON-set */
  TPCover D,		/* DC-set */
  TPCover R,		/* OFF-set */
  TPCover Foriginal,	/* ON-set before reduction (same order as F) */
  TEspressoInt32 c1index,		/* which index of F (or Freduced) to be checked */
  TPCover *G)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TEspressoInt32 c2index;
    register TPCube p, last, c2under;
    TPCube RAISE, FREESET, temp, *FD, c2essential;
    TPCover F1;

    RAISE = MNew_Cube(pCubeSettings);
    FREESET = MNew_Cube(pCubeSettings);
    temp = MNew_Cube(pCubeSettings);

    /* Initialize the OFF-set */
    R->nActive_Count = R->nCount;
    MForeach_Set(R, last, p)
    {
    	MSet(p, CEspressoActive);
    }
    /* Initialize the reduced ON-set, all nonprime cubes become active */
    F->nActive_Count = F->nCount;
    MForeachi_Set(F, c2index, c2under)
    {
	    if (c1index == c2index || MTestP(c2under, CEspressoPrime))
        {
	        F->nActive_Count--;
	        MReset(c2under, CEspressoActive);
	    }
        else
        {
	        MSet(c2under, CEspressoActive);
	    }
    }

    /* Initialize the raising and unassigned sets */
    set_copy(RAISE, MGetSet(F, c1index));
    set_diff(FREESET, pCubeSettings->pFullSet, RAISE);

    /* Determine parts which must be lowered */
    essen_parts(pCubeContext, R, F, RAISE, FREESET);

    /* Determine parts which can always be raised */
    essen_raising(pCubeContext, R, RAISE, FREESET);

    /* See which, if any, of the reduced cubes we can cover */
    MForeachi_Set(F, c2index, c2under)
    {
	if (MTestP(c2under, CEspressoActive))
    {
	    /* See if this cube can be covered by an expansion */
	    if (setp_implies(c2under, RAISE) ||
            feasibly_covered(pCubeContext, R, c2under, RAISE, temp))
        {
		    /* See if c1under can expanded to cover c2 reduced against
		     * (F - c1) u c1under; if so, c2 can definitely be removed !
		     */

		    /* Copy F and replace c1 with c1under */
		    F1 = sf_save(pCubeContext, Foriginal);
		    set_copy(MGetSet(F1, c1index), MGetSet(F, c1index));

		    /* Reduce c2 against ((F - c1) u c1under) */
		    FD = cube2list(pCubeContext, F1, D);
		    c2essential = reduce_cube(pCubeContext, FD, MGetSet(F1, c2index));
		    MFree_Cubelist(FD);
		    sf_free(pCubeContext, F1);

		    /* See if c2essential is covered by an expansion of c1under */
		    if (feasibly_covered(pCubeContext, R, c2essential, RAISE, temp))
            {
		        set_or(temp, RAISE, c2essential);
		        MReset(temp, CEspressoPrime);		/* cube not prime */
		        *G = sf_addset(*G, temp);
		    }
		    MSet_Free(c2essential);
	    }
	}
    }

    MFree_Cube(RAISE);
    MFree_Cube(FREESET);
    MFree_Cube(temp);
}

/* irred_gasp -- Add new primes to F and find an irredundant subset */
TPCover irred_gasp(
  TPCubeContext pCubeContext,
  TPCover F,
  TPCover D,
  TPCover G)                 /* G is disposed of */
{
    if (G->nCount != 0)
    {
	    F = irredundant(pCubeContext, sf_append(pCubeContext, F, G), D);
    }
    else
    {
	    MFree_Cover(pCubeContext, G);
    }

    return F;
}


/* last_gasp */
TPCover last_gasp(
  TPCubeContext pCubeContext,
  TPCover F,
  TPCover D,
  TPCover R,
  TPCost cost)
{
    TPCover G, G1;

    G = reduce_gasp(pCubeContext, F, D);
    cover_cost(pCubeContext, G, cost);

    G1 = expand_gasp(pCubeContext, G, D, R, F);
    cover_cost(pCubeContext, G1, cost);

    MFree_Cover(pCubeContext, G);

    F = irred_gasp(pCubeContext, F, D, G1);
    cover_cost(pCubeContext, F, cost);

    return F;
}

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
