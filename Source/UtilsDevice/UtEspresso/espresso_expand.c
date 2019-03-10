/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#include "espresso_hdrs.h"

#include "espresso_defs.h"
#include "espresso_expand.h"
#include "espresso_cvrm.h"
#include "espresso_set.h"
#include "espresso_setc.h"
#include "espresso_sminterf.h"

/*
    module: expand.c
    purpose: Perform the Espresso-II Expansion Step

    The idea is to take each nonprime cube of the on-set and expand it
    into a prime implicant such that we can cover as many other cubes
    of the on-set.  If no cube of the on-set can be covered, then we
    expand each cube into a large prime implicant by transforming the
    problem into a minimum covering problem which is solved by the
    heuristics of minimum_cover.

    These routines revolve around having a representation of the
    OFF-set.  (In contrast to the Espresso-II manuscript, we do NOT
    require an "unwrapped" version of the OFF-set).

    Some conventions on variable names:

	SUPER_CUBE is the supercube of all cubes which can be covered
	by an expansion of the cube being expanded

	OVEREXPANDED_CUBE is the cube which would result from expanding
	all parts which can expand individually of the cube being expanded

	RAISE is the current expansion of the current cube

	FREESET is the set of parts which haven't been raised or lowered yet.

	INIT_LOWER is a set of parts to be removed from the free parts before
	starting the expansion
*/

/*
    expand -- expand each nonprime cube of F into a prime implicant

    If nonsparse is true, only the non-sparse variables will be expanded;
    this is done by forcing all of the sparse variables out of the free set.
*/

TPCover expand(
  TPCubeContext pCubeContext,
   TPCover F,
  const TPCover R,
  TEspressoBool nonsparse)              /* expand non-sparse variables only */
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TPCube last, p;
    TPCube RAISE, FREESET, INIT_LOWER, SUPER_CUBE, OVEREXPANDED_CUBE;
    TEspressoInt32 var, num_covered;
    TEspressoBool change;

    /* Order the cubes according to "chewing-away from the edges" of mini */
    if (pCubeContext->use_random_order)
    {
	    F = random_order(F);
    }
    else
    {
	    F = mini_sort(pCubeContext, F, ascend);
    }

    /* Allocate memory for variables needed by expand1() */
    RAISE = MNew_Cube(pCubeSettings);
    FREESET = MNew_Cube(pCubeSettings);
    INIT_LOWER = MNew_Cube(pCubeSettings);
    SUPER_CUBE = MNew_Cube(pCubeSettings);
    OVEREXPANDED_CUBE = MNew_Cube(pCubeSettings);

    /* Setup the initial lowering set (differs only for nonsparse) */
    if (nonsparse)
    {
	    for(var = 0; var < pCubeSettings->nNum_Vars; var++)
        {
	        if (pCubeSettings->pnSparse[var])
            {
		        set_or(INIT_LOWER, INIT_LOWER, pCubeSettings->ppVar_Mask[var]);
            }
        }
    }

    /* Mark all cubes as not covered, and maybe essential */
    MForeach_Set(F, last, p)
    {
	    MReset(p, CEspressoCovered);
	    MReset(p, CEspressoNoneEssen);
    }

    /* Try to expand each nonprime and noncovered cube */
    MForeach_Set(F, last, p)
    {
	    /* do not expand if PRIME or if covered by previous expansion */
	    if (! MTestP(p, CEspressoPrime) && ! MTestP(p, CEspressoCovered))
        {
	        /* expand the cube p, result is RAISE */
	        expand1(pCubeContext, R, F, RAISE, FREESET, OVEREXPANDED_CUBE, SUPER_CUBE,
		    INIT_LOWER, &num_covered, p);
	        set_copy(p, RAISE);
	        MSet(p, CEspressoPrime);
	        MReset(p, CEspressoCovered);		/* not really necessary */

	        /* See if we generated an inessential prime */
	        if (num_covered == 0 && ! setp_equal(p, OVEREXPANDED_CUBE))
            {
		        MSet(p, CEspressoNoneEssen);
	        }
	    }
    }

    /* Delete any cubes of F which became covered during the expansion */
    F->nActive_Count = 0;
    change = CEspressoFalse;
    MForeach_Set(F, last, p)
    {
	    if (MTestP(p, CEspressoCovered))
        {
	        MReset(p, CEspressoActive);
	        change = CEspressoTrue;
	    }
        else
        {
	        MSet(p, CEspressoActive);
	        F->nActive_Count++;
	    }
    }

    if (change)
    {
	    F = sf_inactive(F);
    }

    MFree_Cube(RAISE);
    MFree_Cube(FREESET);
    MFree_Cube(INIT_LOWER);
    MFree_Cube(SUPER_CUBE);
    MFree_Cube(OVEREXPANDED_CUBE);

    return F;
}

/*
    expand1 -- Expand a single cube against the OFF-set
*/
void expand1(
  TPCubeContext pCubeContext,
  TPCover BB,			/* Blocking matrix (OFF-set) */
  TPCover CC,			/* Covering matrix (ON-set) */
  TPCube RAISE,			/* The current parts which have been raised */
  TPCube FREESET,			/* The current parts which are free */
  TPCube OVEREXPANDED_CUBE,	/* Overexpanded cube of c */
  TPCube SUPER_CUBE,		/* Supercube of all cubes of CC we cover */
  TPCube INIT_LOWER,		/* Parts to initially remove from FREESET */
  TEspressoInt32 *num_covered,		/* Number of cubes of CC which are covered */
  TPCube c)			/* The cube to be expanded */
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TEspressoInt32 bestindex;

    /* initialize BB and CC */
    MSet(c, CEspressoPrime);		/* don't try to cover ourself */
    setup_BB_CC(BB, CC);

    /* initialize count of # cubes covered, and the supercube of them */
    *num_covered = 0;
    set_copy(SUPER_CUBE, c);

    /* Initialize the lowering, raising and unassigned sets */
    set_copy(RAISE, c);
    set_diff(FREESET, pCubeSettings->pFullSet, RAISE);

    /* If some parts are forced into lowering set, remove them */
    if (! setp_empty(INIT_LOWER))
    {
	    set_diff(FREESET, FREESET, INIT_LOWER);
	    elim_lowering(pCubeContext, BB, CC, RAISE, FREESET);
    }

    /* Determine what can be raised, and return the over-expanded cube */
    essen_parts(pCubeContext, BB, CC, RAISE, FREESET);
    set_or(OVEREXPANDED_CUBE, RAISE, FREESET);

    /* While there are still cubes which can be covered, cover them ! */
    if (CC->nActive_Count > 0)
    {
	    select_feasible(pCubeContext, BB, CC, RAISE, FREESET, SUPER_CUBE, num_covered);
    }

    /* While there are still cubes covered by the overexpanded cube ... */
    while (CC->nActive_Count > 0)
    {
	    bestindex = most_frequent(pCubeContext, CC, FREESET);
	    MSet_Insert(RAISE, bestindex);
	    MSet_Remove(FREESET, bestindex);
	    essen_parts(pCubeContext, BB, CC, RAISE, FREESET);
    }

    /* Finally, when all else fails, choose the largest possible prime */
    /* We will loop only if we decide unravelling OFF-set is too expensive */
    while (BB->nActive_Count > 0)
    {
    	mincov(pCubeContext, BB, RAISE, FREESET);
    }

    /* Raise any remaining free coordinates */
    set_or(RAISE, RAISE, FREESET);
}

/*
    most_frequent -- When all else fails, select a reasonable part to raise
    The active cubes of CC are the cubes which are covered by the
    overexpanded cube of the original cube (however, we know that none
    of them can actually be covered by a feasible expansion of the
    original cube).  We resort to the MINI strategy of selecting to
    raise the part which will cover the same part in the most cubes of CC.
*/
TEspressoInt32 most_frequent(
  TPCubeContext pCubeContext,
  TPCover CC,
  TPCube FREESET)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TEspressoInt32 i, best_part, best_count, *count;
    register TPSet p, last;

    /* Count occurences of each variable */

    count = (TEspressoInt32*)UtAllocMem(sizeof(TEspressoInt32) * pCubeSettings->nSize);

    for(i = 0; i < pCubeSettings->nSize; i++)
    {
	    count[i] = 0;
    }

    if (CC != NULL)
	    MForeach_Active_Set(CC, last, p)
	        set_adjcnt(p, count, 1);

    /* Now find which free part occurs most often */
    best_count = best_part = -1;
    for (i = 0; i < pCubeSettings->nSize; i++)
    {
	    if (MIs_In_Set(FREESET,i) && count[i] > best_count)
        {
	        best_part = i;
	        best_count = count[i];
	    }
    }

    if (count)
    {
        UtFreeMem(count);
        count = NULL; 
    }

    return best_part;
}

/*
    setup_BB_CC -- set up the blocking and covering set families;

    Note that the blocking family is merely the set of cubes of R, and
    that CC is the set of cubes of F which might possibly be covered
    (i.e., nonprime cubes, and cubes not already covered)
*/

void setup_BB_CC(
  register TPCover BB,
  register TPCover CC)
{
    register TPCube p, last;

    /* Create the block and cover set families */
    BB->nActive_Count = BB->nCount;
    MForeach_Set(BB, last, p)
	    MSet(p, CEspressoActive);

    if (CC != NULL) 
    {
	    CC->nActive_Count = CC->nCount;
	    MForeach_Set(CC, last, p)
        {
	        if (MTestP(p, CEspressoCovered) || MTestP(p, CEspressoPrime))
            {
	    	    CC->nActive_Count--, MReset(p, CEspressoActive);
            }
	        else
            {
    		    MSet(p, CEspressoActive);
            }
        }
    }
}

/*
    select_feasible -- Determine if there are cubes which can be covered,
    and if so, raise those parts necessary to cover as many as possible.

    We really don't check to maximize the number that can be covered;
    instead, we check, for each fcc, how many other fcc remain fcc
    after expanding to cover the fcc.  (Essentially one-level lookahead).
*/

void select_feasible(
  TPCubeContext pCubeContext,
  TPCover BB,
  TPCover CC,
  TPCube RAISE,
  TPCube FREESET,
  TPCube SUPER_CUBE,
  TEspressoInt32 *num_covered)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TPCube p, last, bestfeas, *feas;
    register TEspressoInt32 i, j;
    TPCube *feas_new_lower;
    TEspressoInt32 bestcount, bestsize, count, size, numfeas, lastfeas;
    TPCover new_lower;

    bestfeas = NULL;

    /*  Start out with all cubes covered by the over-expanded cube as
     *  the "possibly" feasibly-covered cubes (pfcc)
     */
    feas = (TPCube*)UtAllocMem(sizeof(TPCube) * CC->nActive_Count);
    numfeas = 0;
    MForeach_Active_Set(CC, last, p)
	    feas[numfeas++] = p;

    /* Setup extra cubes to record parts forced low after a covering */
    feas_new_lower = (TPCube*)UtAllocMem(sizeof(TPCube) * CC->nActive_Count);
    new_lower = MNew_Cover(pCubeContext, numfeas, pCubeSettings);
    for(i = 0; i < numfeas; i++)
    {
	    feas_new_lower[i] = MGetSet(new_lower, i);
    }

loop:
    /* Find the essentially raised parts -- this might cover some cubes
       for us, without having to find out if they are fcc or not
    */
    essen_raising(pCubeContext, BB, RAISE, FREESET);

    /* Now check all "possibly" feasibly covered cubes to check feasibility */
    lastfeas = numfeas;
    numfeas = 0;
    for(i = 0; i < lastfeas; i++)
    {
	    p = feas[i];

	    /* Check active because essen_parts might have removed it */
	    if (MTestP(p, CEspressoActive))
        {
	        /*  See if the cube is already covered by RAISE --
	         *  this can happen because of essen_raising() or because of
	         *  the previous "loop"
	         */
	        if (setp_implies(p, RAISE))
            {
		        (*num_covered) += 1;
		        set_or(SUPER_CUBE, SUPER_CUBE, p);
		        CC->nActive_Count--;
		        MReset(p, CEspressoActive);
		        MSet(p, CEspressoCovered);
	            /* otherwise, test if it is feasibly covered */
	        }
            else if (feasibly_covered(pCubeContext, BB,p,RAISE,feas_new_lower[numfeas]))
            {
		        feas[numfeas] = p;			/* save the fcc */
		        numfeas++;
	        }
	    }
    }

    /* Exit here if there are no feasibly covered cubes */
    if (numfeas == 0)
    {
        if (feas)
        {
            UtFreeMem(feas);
            feas = NULL;
        }

        if (feas_new_lower)
        {
            UtFreeMem(feas_new_lower);
            feas_new_lower = NULL;
        }

	    MFree_Cover(pCubeContext, new_lower);
	    return;
    }

    /* Now find which is the best feasibly covered cube */
    bestcount = 0;
    bestsize = 9999;
    for(i = 0; i < numfeas; i++)
    {
	size = set_dist(pCubeContext, feas[i], FREESET);	/* # of newly raised parts */
	count = 0;	/* # of other cubes which remain fcc after raising */

#define NEW
#ifdef NEW
	for(j = 0; j < numfeas; j++)
	    if (setp_disjoint(feas_new_lower[i], feas[j]))
		count++;
#else
	for(j = 0; j < numfeas; j++)
	    if (setp_implies(feas[j], feas[i]))
		count++;
#endif
	    if (count > bestcount)
        {
	        bestcount = count;
	        bestfeas = feas[i];
	        bestsize = size;
	    }
        else if (count == bestcount && size < bestsize)
        {
	        bestfeas = feas[i];
	        bestsize = size;
	    }
    }

    /* Add the necessary parts to the raising set */
    set_or(RAISE, RAISE, bestfeas);
    set_diff(FREESET, FREESET, RAISE);
    essen_parts(pCubeContext, BB, CC, RAISE, FREESET);
    goto loop;
/* NOTREACHED */
}

/*
    elim_lowering -- after removing parts from FREESET, we can reduce the
    size of both BB and CC.

    We mark as inactive any cube of BB which does not intersect the
    overexpanded cube (i.e., RAISE + FREESET).  Likewise, we remove
    from CC any cube which is not covered by the overexpanded cube.
*/

void elim_lowering(
  TPCubeContext pCubeContext,
  TPCover BB,
  TPCover CC,
  TPCube RAISE,
  TPCube FREESET)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TPCube p, r = set_or(pCubeSettings->ppTemp[0], RAISE, FREESET);
    TPCube last;

    /*
     *  Remove sets of BB which are orthogonal to future expansions
     */
    MForeach_Active_Set(BB, last, p)
    {
    	if (! cdist0(pCubeContext, p, r))
        {
    	    BB->nActive_Count--, MReset(p, CEspressoActive);
        }
    }


    /*
     *  Remove sets of CC which cannot be covered by future expansions
     */
    if (CC != NULL)
    {
	    MForeach_Active_Set(CC, last, p)
        {
    	    if (! setp_implies(p, r))
    	    	CC->nActive_Count--, MReset(p, CEspressoActive);
    	}
    }
}

/*
    essen_parts -- determine which parts are forced into the lowering
    set to insure that the cube be orthognal to the OFF-set.

    If any cube of the OFF-set is distance 1 from the raising cube,
    then we must lower all parts of the conflicting variable.  (If the
    cube is distance 0, we detect this error here.)

    If there are essentially lowered parts, we can remove from consideration
    any cubes of the OFF-set which are more than distance 1 from the
    overexpanded cube of RAISE.
*/

void essen_parts(
  TPCubeContext pCubeContext,
  TPCover BB,
  TPCover CC,
  TPCube RAISE,
  TPCube FREESET)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TPCube p, r = RAISE;
    TPCube lastp, xlower = pCubeSettings->ppTemp[0];
    TEspressoInt32 dist;

    set_copy(xlower, pCubeSettings->pEmptySet);

    MForeach_Active_Set(BB, lastp, p)
    {
    	if ((dist = cdist01(pCubeContext, p, r)) > 1)
            goto exit_if;

	    if (dist == 0)
        {
            assert(0);
	        //fatal("ON-set and OFF-set are not orthogonal");
	    }
        else
        {
	        force_lower(pCubeContext, xlower, p, r);
	        BB->nActive_Count--;
	        MReset(p, CEspressoActive);
	    }
exit_if: ;
    }

    if (! setp_empty(xlower))
    {
	    set_diff(FREESET, FREESET, xlower);/* remove from free set */
	    elim_lowering(pCubeContext, BB, CC, RAISE, FREESET);
    }
}

/*
    essen_raising -- determine which parts may always be added to
    the raising set without restricting further expansions

    General rule: if some part is not blocked by any cube of BB, then
    this part can always be raised.
*/

void essen_raising(
  TPCubeContext pCubeContext,
  register TPCover BB,
  TPCube RAISE,
  TPCube FREESET)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TPCube last, p, xraise = pCubeSettings->ppTemp[0];

    /* Form union of all cubes of BB, and then take complement wrt FREESET */
    set_copy(xraise, pCubeSettings->pEmptySet);
    MForeach_Active_Set(BB, last, p)
	    MInlineSet_Or(xraise, xraise, p);
    set_diff(xraise, FREESET, xraise);

    set_or(RAISE, RAISE, xraise);         /* add to raising set */
    set_diff(FREESET, FREESET, xraise);       /* remove from free set */
}

/*
    mincov -- transform the problem of expanding a cube to a maximally-
    large prime implicant into the problem of selecting a minimum
    cardinality cover over a family of sets.

    When we get to this point, we must unravel the remaining off-set.
    This may be painful.
*/

void mincov(
  TPCubeContext pCubeContext,
  TPCover BB,
  TPCube RAISE,
  TPCube FREESET)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TEspressoInt32 expansion, nset, var, dist;
    TPSet_Family B;
    register TPCube xraise=pCubeSettings->ppTemp[0], xlower, p, last, plower;

#ifdef RANDOM_MINCOV
    dist = random() % set_ord(FREESET);
    for(var = 0; var < cubesize && dist >= 0; var++)
    {
	    if (is_in_set(FREESET, var))
        {
	        dist--;
	    }
    }

    set_insert(RAISE, var);
    set_remove(FREESET, var);
    essen_parts(BB, /*CC*/ NULL, RAISE, FREESET);
#else

    /* Create B which are those cubes which we must avoid intersecting */
    B = MNew_Cover(pCubeContext, BB->nActive_Count, pCubeSettings);
    MForeach_Active_Set(BB, last, p)
    {
    	plower = set_copy(MGetSet(B, B->nCount++), pCubeSettings->pEmptySet);
	    force_lower(pCubeContext, plower, p, RAISE);
    }

    /* Determine how many sets it will blow up into after the unravel */
    nset = 0;
    MForeach_Set(B, last, p)
    {
	expansion = 1;
	for(var = pCubeSettings->nNum_Binary_Vars; var < pCubeSettings->nNum_Vars; var++)
    {
	    if ((dist=set_dist(pCubeContext, p, pCubeSettings->ppVar_Mask[var])) > 1) 
        {
		    expansion *= dist;
		    if (expansion > 500)
            {
                goto heuristic_mincov;
            }
	    }
	}
	nset += expansion;
	if (nset > 500)
    {
        goto heuristic_mincov;
    }
    }

    B = unravel(pCubeContext, B, pCubeSettings->nNum_Binary_Vars);
    xlower = do_sm_minimum_cover(B);

    /* Add any remaining free parts to the raising set */
    set_or(RAISE, RAISE, set_diff(xraise, FREESET, xlower));
    set_copy(FREESET, pCubeSettings->pEmptySet);	/* free set is empty */
    BB->nActive_Count = 0;			/* BB satisfied */
    sf_free(pCubeContext, B);
    MSet_Free(xlower);
    return;

heuristic_mincov:
    sf_free(pCubeContext, B);
    /* most_frequent will pick first free part */
    MSet_Insert(RAISE, most_frequent(pCubeContext, /*CC*/ NULL, FREESET));
    set_diff(FREESET, FREESET, RAISE);
    essen_parts(pCubeContext, BB, /*CC*/ NULL, RAISE, FREESET);
    return;
#endif
}

/*
    feasibly_covered -- determine if the cube c is feasibly covered
    (i.e., if it is possible to raise all of the necessary variables
    while still insuring orthogonality with R).  Also, if c is feasibly
    covered, then compute the new set of parts which are forced into
    the lowering set.
*/

TEspressoBool feasibly_covered(
  TPCubeContext pCubeContext,
  TPCover BB,
  TPCube c,
  TPCube RAISE,
  TPCube new_lower)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TPCube p, r = set_or(pCubeSettings->ppTemp[0], RAISE, c);
    TEspressoInt32 dist;
    TPCube lastp;

    set_copy(new_lower, pCubeSettings->pEmptySet);
    MForeach_Active_Set(BB, lastp, p)
    {
    	if ((dist = cdist01(pCubeContext, p, r)) > 1)
            goto exit_if;

	    if (dist == 0)
        {
	        return CEspressoFalse;
        }
	    else
        {
	        force_lower(pCubeContext, new_lower, p, r);
        }

        exit_if: ;
    }
    return CEspressoTrue;
}

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
