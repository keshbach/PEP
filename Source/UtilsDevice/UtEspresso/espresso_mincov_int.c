/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#include "espresso_hdrs.h"

#include "espresso_defs.h"
#include "espresso_sparse.h"
#include "espresso_mincov_int.h"

static TEspressoInt32 select_column(
  TPSM_Matrix A,
  TEspressoInt32 *weight,
  TPSolution indep)
{
    register TPSM_Col pcol;
    register TPSM_Row prow, indep_cols;
    register TPSM_Element p, p1;
    double w, best;
    TEspressoInt32 best_col;

    indep_cols = sm_row_alloc();
    if (indep != NULL)
    {
	    /* Find which columns are in the independent sets */
	    for(p = indep->pRow->pFirst_Col; p != 0; p = p->pNext_Col)
        {
	        prow = MSM_Get_Row(A, p->nCol_Num);
	        for(p1 = prow->pFirst_Col; p1 != 0; p1 = p1->pNext_Col)
            {
    		    sm_row_insert(indep_cols, p1->nCol_Num);
	        }
	    }
    }
    else
    {
	    /* select out of all columns */
	    MSM_Foreach_Col(A, pcol)
        {
	        sm_row_insert(indep_cols, pcol->nCol_Num);
	    }
    }

    /* Find the best column */
    best_col = -1;
    best = -1;

    /* Consider only columns which are in some independent row */
    MSM_Foreach_Row_Element(indep_cols, p1)
    {
	    pcol = MSM_Get_Col(A, p1->nCol_Num);

	    /* Compute the total 'value' of all things covered by the column */
	    w = 0.0;
	    for(p = pcol->pFirst_Row; p != 0; p = p->pNext_Row)
        {
	        prow = MSM_Get_Row(A, p->nRow_Num);
	        w += 1.0 / ((double) prow->nLength - 1.0);
	    }

	    /* divide this by the relative cost of choosing this column */
	    w = w / (double) MWeight(weight, pcol->nCol_Num);

	    /* maximize this ratio */
	    if (w  > best)
        {
	        best_col = pcol->nCol_Num;
	        best = w;
	    }
    }

    sm_row_free(indep_cols);
    return best_col;
}

static void select_essential(
  TPSM_Matrix A,
  TPSolution select,
  TEspressoInt32 *weight,
  TEspressoInt32 bound)			/* must beat this solution */
{
    register TPSM_Element p;
    register TPSM_Row prow, essen;
    TEspressoInt32 delcols, delrows, essen_count;

    do
    {
	/*  Check for dominated columns  */
	delcols = sm_col_dominance(A, weight);

	/*  Find the rows with only 1 element (the essentials) */
	essen = sm_row_alloc();
	MSM_Foreach_Row(A, prow)
    {
	    if (prow->nLength == 1)
        {
		    sm_row_insert(essen, prow->pFirst_Col->nCol_Num);
	    }
	}

	/* Select all of the elements */
	MSM_Foreach_Row_Element(essen, p)
    {
	    solution_accept(select, A, weight, p->nCol_Num);
	    /* Make sure solution still looks good */
	    if (select->nCost >= bound)
        {
		    sm_row_free(essen);
		    return;
	    }
	}
	essen_count = essen->nLength;
	sm_row_free(essen);

	/*  Check for dominated rows  */
	delrows = sm_row_dominance(A);

    } while (delcols > 0 || delrows > 0 || essen_count > 0);
}

TPSolution solution_alloc()
{
    TPSolution sol;

    sol = (TPSolution)UtAllocMem(sizeof(TSolution));
    sol->nCost = 0;
    sol->pRow = sm_row_alloc();

    return sol;
}

void solution_free(
  TPSolution sol)
{
    sm_row_free(sol->pRow);

    if (sol)
    {
        UtFreeMem(sol);
        sol = NULL;
    }
}

TPSolution solution_dup(
  TPSolution sol)
{
    TPSolution new_sol;

    new_sol = (TPSolution)UtAllocMem(sizeof(TSolution));
    new_sol->nCost = sol->nCost;
    new_sol->pRow = sm_row_dup(sol->pRow);

    return new_sol;
}

void solution_add(
  TPSolution sol,
  TEspressoInt32 *weight,
  TEspressoInt32 col)
{
    sm_row_insert(sol->pRow, col);
    sol->nCost += MWeight(weight, col);
}

void solution_accept(
  TPSolution sol,
  TPSM_Matrix A,
  TEspressoInt32 *weight,
  TEspressoInt32 col)
{
    register TPSM_Element p, pnext;
    TPSM_Col pcol;

    solution_add(sol, weight, col);

    /* delete rows covered by this column */
    pcol = MSM_Get_Col(A, col);
    for(p = pcol->pFirst_Row; p != 0; p = pnext)
    {
	    pnext = p->pNext_Row;		/* grab it before it disappears */
	    sm_delrow(A, p->nRow_Num);
    }
}

void solution_reject(
  TPSolution sol,
  TPSM_Matrix A,
  TEspressoInt32 *weight,
  TEspressoInt32 col)
{
    weight;
    sol;

    sm_delcol(A, col);
}

TPSolution solution_choose_best(
  TPSolution best1,
  TPSolution best2)
{
    if (best1 != NULL)
    {
	    if (best2 != NULL)
        {
	        if (best1->nCost <= best2->nCost)
            {
	    	    solution_free(best2);
    		    return best1;
	        }
            else
            {
	    	    solution_free(best1);
		        return best2;
	        }
	    }
        else
        {
	        return best1;
	    }
    }
    else
    {
	    if (best2 != NULL)
        {
	        return best2;
	    }
        else
        {
	        return NULL;
	    }
    }
}

/*
 *  Find the best cover for 'A' (given that 'select' already selected);
 *
 *    - abort search if a solution cannot be found which beats 'bound'
 *
 *    - if any solution meets 'lower_bound', then it is the optimum solution
 *      and can be returned without further work.
 */

TPSolution sm_mincov(
  TPSM_Matrix A,
  TPSolution select,
  TEspressoInt32 *weight,
  TEspressoInt32 lb,
  TEspressoInt32 bound,
  TEspressoInt32 depth,
  TPStats stats)
{
    TPSM_Matrix A1, A2, L, R;
    TPSM_Element p;
    TPSolution select1, select2, best, best1, best2, indep;
    TEspressoInt32 pick, lb_new;

    lb;
    indep;

    /* Start out with some debugging information */
    stats->nNodes++;
    if (depth > stats->nMax_Depth)
    {
        stats->nMax_Depth = depth;
    }

    /* Apply row dominance, column dominance, and select essentials */
    select_essential(A, select, weight, bound);
    if (select->nCost >= bound)
    {
    	return NULL;
    }

    /* See if gimpel's reduction technique applies ... */
#ifdef USE_GIMPEL
    if ( weight == NULL)	/* hack until we fix it */
    {
	    if (gimpel_reduce(A, select, weight, lb, bound, depth, stats, &best))
        {
    	    return best;
	    }
    }
#endif

#ifdef USE_INDEP_SET
    /* Determine bound from here to final solution using independent-set */
    indep = sm_maximal_independent_set(A, weight);

    /* make sure the lower bound is monotonically increasing */
    lb_new = MAX(select->cost + indep->cost, lb);
    pick = select_column(A, weight, indep);
    solution_free(indep);
#else
    lb_new = select->nCost + (A->nNumRows > 0);
    pick = select_column(A, weight, NULL);
#endif

    if (depth == 0)
    {
    	stats->nLower_Bound = lb_new + stats->nGimpel;
    }

    /* Check for bounding based on no better solution possible */
    if (lb_new >= bound)
    {
		best = NULL;
        /* Check for new best solution */
    }
    else if (A->nNumRows == 0)
    {
	    best = solution_dup(select);
        /* Check for a partition of the problem */
    }
    else if (sm_block_partition(A, &L, &R)) 
    {
	    /* Make L the smaller problem */
	    if (L->nNumCols > R->nNumCols)
        {
	        A1 = L;
	        L = R;
	        R = A1;
    	}

        stats->nComp_Count++;

	    /* Solve problem for L */
	    select1 = solution_alloc();
	    stats->nComponent++;
	    best1 = sm_mincov(L, select1, weight, 0, 
				        bound-select->nCost, depth+1, stats);
	    stats->nComponent--;
	    solution_free(select1);
	    sm_free(L);

	    /* Add best solution to the selected set */
	    if (best1 == NULL)
        {
	        best = NULL;
	    }
        else
        {
	        for(p = best1->pRow->pFirst_Col; p != 0; p = p->pNext_Col)
            {
    		    solution_add(select, weight, p->nCol_Num);
	        }
	        solution_free(best1);

	        /* recur for the remaining block */
	        best = sm_mincov(R, select, weight, lb_new, bound, depth+1, stats);
	    }
    	sm_free(R);

        /* We've tried as hard as possible, but now we must split and recur */
    }
    else
    {
        /* Assume we choose this column to be in the covering set */
	    A1 = sm_dup(A);
	    select1 = solution_dup(select);
	    solution_accept(select1, A1, weight, pick);
        best1 = sm_mincov(A1, select1, weight, lb_new, bound, depth+1, stats);
	    solution_free(select1);
	    sm_free(A1);

	    /* Update the upper bound if we found a better solution */
	    if (best1 != NULL && bound > best1->nCost)
        {
	        bound = best1->nCost;
	    }

	    /* See if this is a heuristic covering (no branching) */
	    if (stats->nNo_Branching)
        {
	        return best1;
	    }

	    /* Check for reaching lower bound -- if so, don't actually branch */
	    if (best1 != NULL && best1->nCost == lb_new)
        {
	        return best1;
	    }

        /* Now assume we cannot have that column */
	    A2 = sm_dup(A);
	    select2 = solution_dup(select);
	    solution_reject(select2, A2, weight, pick);
        best2 = sm_mincov(A2, select2, weight, lb_new, bound, depth+1, stats);
	    solution_free(select2);
	    sm_free(A2);

	    best = solution_choose_best(best1, best2);
    }

    return best;
}

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
