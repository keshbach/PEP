/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#if !defined(espresso_mincov_int_h)
#define espresso_mincov_int_h

#define MWeight(weight, col) (weight == NULL ? 1 : weight[col])

typedef struct tagTStats
{
    TEspressoInt32 nMax_Depth;		/* deepest the recursion has gone */
    TEspressoInt32 nNodes;			/* total nodes visited */
    TEspressoInt32 nComponent;		/* currently solving a component */
    TEspressoInt32 nComp_Count;	    /* number of components detected */
    TEspressoInt32 nGimpel_Count;	/* number of times Gimpel reduction applied */
    TEspressoInt32 nGimpel;		    /* currently inside Gimpel reduction */
    TEspressoInt32 nNo_Branching;
    TEspressoInt32 nLower_Bound;
} TStats, *TPStats;

typedef struct tagTSolution
{
    TPSM_Row pRow;
    TEspressoInt32 nCost;
} TSolution, *TPSolution;

TPSolution solution_alloc();

void solution_free(TPSolution sol);

void solution_add(TPSolution sol, TEspressoInt32 *weight, TEspressoInt32 col);

void solution_accept(TPSolution sol, TPSM_Matrix A, TEspressoInt32 *weight, TEspressoInt32 col);

void solution_reject(TPSolution sol, TPSM_Matrix A, TEspressoInt32 *weight, TEspressoInt32 col);

TPSolution solution_choose_best(TPSolution best1, TPSolution best2);

TPSolution sm_mincov(TPSM_Matrix A, TPSolution select, TEspressoInt32 *weight, TEspressoInt32 lb, TEspressoInt32 bound, TEspressoInt32 depth, TPStats stats);

#endif /* end of espresso_mincov_int_h */

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
