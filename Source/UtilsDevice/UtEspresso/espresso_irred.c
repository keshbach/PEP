/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#include "espresso_hdrs.h"

#include "espresso_defs.h"
#include "espresso_sparse.h"
#include "espresso_irred.h"
#include "espresso_set.h"
#include "espresso_mincov.h"
#include "espresso_cofactor.h"
#include "espresso_setc.h"
#include "espresso_cvrm.h"

/*
 *  taut_special_cases -- check special cases for tautology
 */

TEspressoBool taut_special_cases(
  TPCubeContext pCubeContext,
  TPCube *T)			/* will be disposed if answer is determined */
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TPCubeData pCubeData = &pCubeContext->CubeData;
    register TPCube *T1, *Tsave, p, ceil=pCubeSettings->ppTemp[0], temp=pCubeSettings->ppTemp[1];
    TPCube *A, *B;
    TEspressoInt32 var;

    /* Check for a row of all 1's which implies tautology */
    for(T1 = T+2; (p = *T1++) != NULL; )
    {
	    if (full_row(pCubeContext, p, T[0]))
        {
	        MFree_Cubelist(T);

	        return CEspressoTrue;
	    }
    }

    /* Check for a column of all 0's which implies no tautology */
start:
    MInlineSet_Copy(ceil, T[0]);
    for(T1 = T+2; (p = *T1++) != NULL; )
    {
    	MInlineSet_Or(ceil, ceil, p);
    }
    if (! setp_equal(ceil, pCubeSettings->pFullSet))
    {
	    MFree_Cubelist(T);
	    return CEspressoFalse;
    }

    /* Collect column counts, determine unate variables, etc. */
    massive_count(pCubeContext, T);

    /* If function is unate (and no row of all 1's), then no tautology */
    if (pCubeData->nVars_Unate == pCubeData->nVars_Active)
    {
	    MFree_Cubelist(T);

	    return CEspressoFalse;

        /* If active in a single variable (and no column of 0's) then tautology */
    }
    else if (pCubeData->nVars_Active == 1)
    {
	    MFree_Cubelist(T);

	    return CEspressoTrue;

        /* Check for unate variables, and reduce cover if there are any */
    }
    else if (pCubeData->nVars_Unate != 0)
    {
	/* Form a cube "ceil" with full variables in the unate variables */
	set_copy(ceil, pCubeSettings->pEmptySet);
	for(var = 0; var < pCubeSettings->nNum_Vars; var++)
    {
	    if (pCubeData->pIs_Unate[var])
        {
    		MInlineSet_Or(ceil, ceil, pCubeSettings->ppVar_Mask[var]);
	    }
	}

	/* Save only those cubes that are "full" in all unate variables */
	for(Tsave = T1 = T+2; (p = *T1++) != 0; )
    {
	    if (setp_implies(ceil, set_or(temp, p, T[0])))
        {
		    *Tsave++ = p;
	    }
	}
	*Tsave++ = NULL;
	T[1] = (TPCube) Tsave;

	goto start;

    /* Check for component reduction */
    }
    else if (pCubeData->pnVar_Zeros[pCubeData->nBest] < MCubeListSize(T) / 2)
    {
	    if (cubelist_partition(pCubeContext, T, &A, &B) == 0)
        {
	        return CEspressoMaybe;
	    }
        else
        {
	        MFree_Cubelist(T);
	        if (tautology(pCubeContext, A))
            {
		        MFree_Cubelist(B);

		        return CEspressoTrue;
	        }
            else
            {
    		    return tautology(pCubeContext, B);
	        }
	    }
    }

    /* We tried as hard as we could, but must recurse from here on */
    return CEspressoMaybe;
}

static TEspressoBool ftaut_special_cases(
  TPCubeContext pCubeContext,
  TPCube *T,                 /* will be disposed if answer is determined */
  TPSM_Matrix table)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    TPCubeData pCubeData = &pCubeContext->CubeData;
    register TPCube *T1, *Tsave, p, temp = pCubeSettings->ppTemp[0], ceil = pCubeSettings->ppTemp[1];
    TEspressoInt32 var, rownum;

    /* Check for a row of all 1's in the essential cubes */
    for(T1 = T+2; (p = *T1++) != 0; )
    {
	    if (! MTestP(p, CEspressoRedund))
        {
	        if (full_row(pCubeContext, p, T[0]))
            {
		        /* subspace is covered by essentials -- no new rows for table */
		        MFree_Cubelist(T);

		        return CEspressoTrue;
	        }
	    }
    }

    /* Collect column counts, determine unate variables, etc. */
start:
    massive_count(pCubeContext, T);

    /* If function is unate, find the rows of all 1's */
    if (pCubeData->nVars_Unate == pCubeData->nVars_Active)
    {
	    /* find which nonessentials cover this subspace */
	    rownum = table->pLast_Row ? table->pLast_Row->nRow_Num+1 : 0;
	    sm_insert(table, rownum, pCubeContext->nRp_current);
	    for(T1 = T+2; (p = *T1++) != 0; )
        {
	        if (MTestP(p, CEspressoRedund))
            {
		        /* See if a redundant cube covers this leaf */
		        if (full_row(pCubeContext, p, T[0]))
                {
		            sm_insert(table, rownum, (TEspressoInt32) MSize(p));
		        }
	        }
	    }

	    MFree_Cubelist(T);

	    return CEspressoTrue;

    /* Perform unate reduction if there are any unate variables */
    }
    else if (pCubeData->nVars_Unate != 0)
    {
	    /* Form a cube "ceil" with full variables in the unate variables */
	    set_copy(ceil, pCubeSettings->pEmptySet);
	    for(var = 0; var < pCubeSettings->nNum_Vars; var++)
        {
	        if (pCubeData->pIs_Unate[var])
            {
    		    MInlineSet_Or(ceil, ceil, pCubeSettings->ppVar_Mask[var]);
	        }
	    }

	    /* Save only those cubes that are "full" in all unate variables */
	    for(Tsave = T1 = T+2; (p = *T1++) != 0; )
        {
	        if (setp_implies(ceil, set_or(temp, p, T[0])))
            {
    		    *Tsave++ = p;
	        }
	    }
	    *Tsave++ = 0;
	    T[1] = (TPCube) Tsave;

	    goto start;
    }

    /* Not much we can do about it */
    return CEspressoMaybe;
}

/* tautology -- answer the tautology question for T */
TEspressoBool tautology(
  TPCubeContext pCubeContext,
  TPCube *T)         /* T will be disposed of */
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TPCube cl, cr;
    register TEspressoInt32 best, result;

    if ((result = taut_special_cases(pCubeContext, T)) == CEspressoMaybe)
    {
	    cl = MNew_Cube(pCubeSettings);
	    cr = MNew_Cube(pCubeSettings);
	    best = binate_split_select(pCubeContext, T, cl, cr);
	    result = tautology(pCubeContext, scofactor(pCubeContext, T, cl, best)) &&
		         tautology(pCubeContext, scofactor(pCubeContext, T, cr, best));

	    MFree_Cubelist(T);
	    MFree_Cube(cl);
	    MFree_Cube(cr);
    }

    return result;
}

/* ftautology -- find ways to make a tautology */
static void ftautology(
  TPCubeContext pCubeContext,
  TPCube *T,         	/* T will be disposed of */
  TPSM_Matrix table)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TPCube cl, cr;
    register TEspressoInt32 best;
    
    if (ftaut_special_cases(pCubeContext, T, table) == CEspressoMaybe)
    {
	    cl = MNew_Cube(pCubeSettings);
	    cr = MNew_Cube(pCubeSettings);
	    best = binate_split_select(pCubeContext, T, cl, cr);

	    ftautology(pCubeContext, scofactor(pCubeContext, T, cl, best), table);
	    ftautology(pCubeContext, scofactor(pCubeContext, T, cr, best), table);

	    MFree_Cubelist(T);
	    MFree_Cube(cl);
	    MFree_Cube(cr);
    }
}

/* cube_is_covered -- determine if a cubelist "covers" a single cube */
TEspressoBool cube_is_covered(
  TPCubeContext pCubeContext,
  TPCube *T,
  TPCube c)
{
    return tautology(pCubeContext, cofactor(pCubeContext, T,c));
}

/* fcube_is_covered -- determine exactly how a cubelist "covers" a cube */
static void fcube_is_covered(
  TPCubeContext pCubeContext,
  TPCube *T,
  TPCube c,
  TPSM_Matrix table)
{
    ftautology(pCubeContext, cofactor(pCubeContext, T, c), table);
}

/*
 *   irredundant -- Return a minimal subset of F
 */

TPCover
irredundant(
  TPCubeContext pCubeContext,
  TPCover F,
  TPCover D)
{
    mark_irredundant(pCubeContext, F, D);

    return sf_inactive(F);
}


/*
 *   mark_irredundant -- find redundant cubes, and mark them "INACTIVE"
 */

void mark_irredundant(
  TPCubeContext pCubeContext,
  TPCover F,
  TPCover D)
{
    TPCover E, Rt, Rp;
    TPSet p, p1, last;
    TPSM_Matrix table;
    TPSM_Row cover;
    TPSM_Element pe;

    /* extract a minimum cover */
    irred_split_cover(pCubeContext, F, D, &E, &Rt, &Rp);
    table = irred_derive_table(pCubeContext, D, E, Rp);
    cover = sm_minimum_cover(table, NULL, /* heuristic */ 1);

    /* mark the cubes for the result */
    MForeach_Set(F, last, p)
    {
	    MReset(p, CEspressoActive);
	    MReset(p, CEspressoRelEssen);
    }
    MForeach_Set(E, last, p)
    {
	    p1 = MGetSet(F, MSize(p));
	    assert(setp_equal(p1, p));
	    MSet(p1, CEspressoActive);
	    MSet(p1, CEspressoRelEssen);		/* for essen(), mark as rel. ess. */
    }
    MSM_Foreach_Row_Element(cover, pe)
    {
	    p1 = MGetSet(F, pe->nCol_Num);
	    MSet(p1, CEspressoActive);
    }

    MFree_Cover(pCubeContext, E);
    MFree_Cover(pCubeContext, Rt);
    MFree_Cover(pCubeContext, Rp);
    sm_free(table);
    sm_row_free(cover);
}

/*
 *  irred_split_cover -- find E, Rt, and Rp from the cover F, D
 *
 *	E  -- relatively essential cubes
 *	Rt  -- totally redundant cubes
 *	Rp  -- partially redundant cubes
 */

void irred_split_cover(
  TPCubeContext pCubeContext,
  TPCover F,
  TPCover D,
  TPCover *E,
  TPCover *Rt,
  TPCover *Rp)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TPCube p, last;
    register TEspressoInt32 index;
    TPCover R;
    TPCube *FD, *ED;

    /* number the cubes of F -- these numbers track into E, Rp, Rt, etc. */
    index = 0;
    MForeach_Set(F, last, p)
    {
    	MPutSize(p, index);
    	index++;
    }

    *E = MNew_Cover(pCubeContext, 10, pCubeSettings);
    *Rt = MNew_Cover(pCubeContext, 10, pCubeSettings);
    *Rp = MNew_Cover(pCubeContext, 10, pCubeSettings);
    R = MNew_Cover(pCubeContext, 10, pCubeSettings);

    /* Split F into E and R */
    FD = cube2list(pCubeContext, F, D);
    MForeach_Set(F, last, p)
    {
	    if (cube_is_covered(pCubeContext, FD, p))
        {
	        R = sf_addset(R, p);
	    }
        else
        {
	        *E = sf_addset(*E, p);
	    }
    }
    MFree_Cubelist(FD);

    /* Split R into Rt and Rp */
    ED = cube2list(pCubeContext, *E, D);
    MForeach_Set(R, last, p)
    {
	    if (cube_is_covered(pCubeContext, ED, p))
        {
	        *Rt = sf_addset(*Rt, p);
	    }
        else
        {
	        *Rp = sf_addset(*Rp, p);
	    }
    }
    MFree_Cubelist(ED);

    MFree_Cover(pCubeContext, R);
}

/*
 *  irred_derive_table -- given the covers D, E and the set of
 *  partially redundant primes Rp, build a covering table showing
 *  possible selections of primes to cover Rp.
 */

TPSM_Matrix irred_derive_table(
  TPCubeContext pCubeContext,
  TPCover D,
  TPCover E,
  TPCover Rp)
{
    register TPCube last, p, *list;
    TPSM_Matrix table;
    TEspressoInt32 size_last_dominance, i;

    /* Mark each cube in DE as not part of the redundant set */
    MForeach_Set(D, last, p)
    {
    	MReset(p, CEspressoRedund);
    }
    MForeach_Set(E, last, p)
    {
	    MReset(p, CEspressoRedund);
    }

    /* Mark each cube in Rp as partially redundant */
    MForeach_Set(Rp, last, p)
    {
    	MSet(p, CEspressoRedund);             /* belongs to redundant set */
    }

    /* For each cube in Rp, find ways to cover its minterms */
    list = cube3list(pCubeContext, D, E, Rp);
    table = sm_alloc();
    size_last_dominance = 0;
    i = 0;
    MForeach_Set(Rp, last, p)
    {
	    pCubeContext->nRp_current = MSize(p);
	    fcube_is_covered(pCubeContext, list, p, table);
	    MReset(p, CEspressoRedund);	/* can now consider this cube redundant */
	    /* try to keep memory limits down by reducing table as we go along */
	    if (table->nNumRows - size_last_dominance > 1000)
        {
	        sm_row_dominance(table);
	        size_last_dominance = table->nNumRows;
	    }
	    i++;
    }
    MFree_Cubelist(list);

    return table;
}

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
