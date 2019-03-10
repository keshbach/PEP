/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#include "espresso_hdrs.h"

#include "espresso_defs.h"
#include "espresso_sparse.h"
#include "espresso_mincov.h"
#include "espresso_mincov_int.h"

static TEspressoInt32 verify_cover(
  TPSM_Matrix A,
  TPSM_Row cover)
{
    TPSM_Row prow;

    MSM_Foreach_Row(A, prow)
    {
	    if (! sm_row_intersects(prow, cover))
        {
	        return 0;
	    }
    }
    return 1;
}

TPSM_Row
sm_minimum_cover(
  TPSM_Matrix A,
  TEspressoInt32 *weight,
  TEspressoInt32 heuristic)		/* set to 1 for a heuristic covering */
{
    TStats stats;
    TPSolution best, select;
    TPSM_Row prow, sol;
    TPSM_Col pcol;
    TPSM_Matrix dup_A;
    TEspressoInt32 nelem, bound;
    double sparsity;

    /* Avoid sillyness */
    if (A->nNumRows <= 0)
    {
    	return sm_row_alloc();		/* easy to cover */
    }

    /* Initialize debugging structure */
    stats.nMax_Depth = -1;
    stats.nNodes = 0;
    stats.nComponent = stats.nComp_Count = 0;
    stats.nGimpel = stats.nGimpel_Count = 0;
    stats.nNo_Branching = heuristic != 0;
    stats.nLower_Bound = -1;

    /* Check the matrix sparsity */
    nelem = 0;
    MSM_Foreach_Row(A, prow)
    {
    	nelem += prow->nLength;
    }
    sparsity = (double) nelem / (double) (A->nNumRows * A->nNumCols);

    /* Determine an upper bound on the solution */
    bound = 1;
    MSM_Foreach_Col(A, pcol)
    {
    	bound += MWeight(weight, pcol->nCol_Num);
    }

    /* Perform the covering */
    select = solution_alloc();
    dup_A = sm_dup(A);
    best = sm_mincov(dup_A, select, weight, 0, bound, 0, &stats);
    sm_free(dup_A);
    solution_free(select);

    sol = sm_row_dup(best->pRow);
    if (! verify_cover(A, sol))
    {
        assert(0);
	    //fail("mincov: internal error -- cover verification failed\n");
    }
    solution_free(best);
    return sol;
}

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
