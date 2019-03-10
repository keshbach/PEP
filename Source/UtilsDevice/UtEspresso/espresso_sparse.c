/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#include "espresso_hdrs.h"

#include "espresso_defs.h"
#include "espresso_sparse.h"
#include "espresso_sparse_int.h"
#include "espresso_cvrmisc.h"
#include "espresso_expand.h"
#include "espresso_set.h"
#include "espresso_irred.h"

TPCover make_sparse(
  TPCubeContext pCubeContext,
  TPCover F,
  TPCover D,
  TPCover R)
{
    TCost cost, best_cost;

    cover_cost(pCubeContext, F, &best_cost);

    do
    {
	    F = mv_reduce(pCubeContext, F, D);
        cover_cost(pCubeContext, F, &cost);

	    if (cost.nTotal == best_cost.nTotal)
        {
	        break;
        }

	    copy_cost(&cost, &best_cost);

	    F = expand(pCubeContext, F, R, CEspressoTrue);

        cover_cost(pCubeContext, F, &cost);

	    if (cost.nTotal == best_cost.nTotal)
        {
	        break;
        }

	    copy_cost(&cost, &best_cost);
    } while (pCubeContext->nForce_Irredundant);

    return F;
}

/*
    mv_reduce -- perform an "optimal" reduction of the variables which
    we desire to be sparse

    This could be done using "reduce" and then saving just the desired
    part of the reduction.  Instead, this version uses IRRED to find
    which cubes of an output are redundant.  Note that this gets around
    the cube-ordering problem.

    In normal use, it is expected that the cover is irredundant and
    hence no cubes will be reduced to the empty cube (however, this is
    checked for and such cubes will be deleted)
*/

TPCover mv_reduce(
  TPCubeContext pCubeContext,
  TPCover F,
  TPCover D)
{
    TPCubeSettings pCubeSettings = &pCubeContext->CubeSettings;
    register TEspressoInt32 i, var;
    register TPCube p, p1, last;
    TEspressoInt32 index;
    TPCover F1, D1;
    TPCube *F_cube_table;

    /* loop for each multiple-valued variable */
    for(var = 0; var < pCubeSettings->nNum_Vars; var++)
    {
	if (pCubeSettings->pnSparse[var])
    {
	    /* loop for each part of the variable */
	    for(i = pCubeSettings->pnFirst_Part[var]; i <= pCubeSettings->pnLast_Part[var]; i++)
        {
		    /* remember mapping of F1 cubes back to F cubes */
		    F_cube_table = (TPCube*)UtAllocMem(sizeof(TPCube) * F->nCount);

		    /* 'cofactor' against part #i of variable #var */
		    F1 = MNew_Cover(pCubeContext, F->nCount, pCubeSettings);
		    MForeach_Set(F, last, p)
            {
		        if (MIs_In_Set(p, i))
                {
			        F_cube_table[F1->nCount] = p;
			        p1 = MGetSet(F1, F1->nCount++);
			        set_diff(p1, p, pCubeSettings->ppVar_Mask[var]);
			        MSet_Insert(p1, i);
		        }
		    }

		    /* 'cofactor' against part #i of variable #var */
		    /* not really necessary -- just more efficient ? */
		    D1 = MNew_Cover(pCubeContext, D->nCount, pCubeSettings);
		    MForeach_Set(D, last, p)
            {
		        if (MIs_In_Set(p, i))
                {
			        p1 = MGetSet(D1, D1->nCount++);
			        set_diff(p1, p, pCubeSettings->ppVar_Mask[var]);
			        MSet_Insert(p1, i);
		        }
		    }

		    mark_irredundant(pCubeContext, F1, D1);

		    /* now remove part i from cubes which are redundant */
		    index = 0;
		    MForeach_Set(F1, last, p1)
            {
		        if (! MTestP(p1, CEspressoActive))
                {
			        p = F_cube_table[index];

			        /*   don't reduce a variable which is full
			         *   (unless it is the output variable)
			         */
			        if (var == pCubeSettings->nNum_Vars-1 ||
			             ! setp_implies(pCubeSettings->ppVar_Mask[var], p))
                    {
			            MSet_Remove(p, i);
			        }
			        MReset(p, CEspressoPrime);
		        }
		        index++;
		    }

		    MFree_Cover(pCubeContext, F1);
		    MFree_Cover(pCubeContext, D1);

            if (F_cube_table)
            {
                UtFreeMem(F_cube_table);
                F_cube_table = NULL;
            }
	    }
	}
    }

    /* Check if any cubes disappeared */
    sf_active(F);
    for(var = 0; var < pCubeSettings->nNum_Vars; var++)
    {
	    if (pCubeSettings->pnSparse[var])
        {
	        MForeach_Active_Set(F, last, p)
            {
		        if (setp_disjoint(p, pCubeSettings->ppVar_Mask[var]))
                {
		            MReset(p, CEspressoActive);
		            F->nActive_Count--;
		        }
	        }
	    }
    }

    if (F->nCount != F->nActive_Count)
    {
	    F = sf_inactive(F);
    }
    return F;
}

TPSM_Matrix sm_alloc()
{
    register TPSM_Matrix A;

    A = (TPSM_Matrix)UtAllocMem(sizeof(TSM_Matrix) * 1);
    A->ppRows = NULL;
    A->ppCols = NULL;
    A->nNumRows = A->nNumCols = 0;
    A->nRows_Size = A->nCols_Size = 0;
    A->pFirst_Row = A->pLast_Row = NULL;
    A->pFirst_Col = A->pLast_Col = NULL;

    return A;
}

void sm_free(
  TPSM_Matrix A)
{
    register TPSM_Row prow, pnext_row;
    register TPSM_Col pcol, pnext_col;

    for(prow = A->pFirst_Row; prow != 0; prow = pnext_row)
    {
	    pnext_row = prow->pNext_Row;
	    sm_row_free(prow);
    }

    for(pcol = A->pFirst_Col; pcol != 0; pcol = pnext_col)
    {
	    pnext_col = pcol->pNext_Col;
	    pcol->pFirst_Row = pcol->pLast_Row = NULL;
	    sm_col_free(pcol);
    }

    /* Free the arrays to map row/col numbers into pointers */
    if (A->ppRows)
    {
        UtFreeMem(A->ppRows);
        A->ppRows = NULL;
    }

    if (A->ppCols)
    {
        UtFreeMem(A->ppCols);
        A->ppCols = NULL;
    }

    if (A)
    {
        UtFreeMem(A);
        A = NULL;
    }
}

void sm_resize(
  register TPSM_Matrix A,
  TEspressoInt32 row,
  TEspressoInt32 col)
{
    register TEspressoInt32 i, new_size;

    if (row >= A->nRows_Size) 
    {
    	new_size = MEspressoMax(A->nRows_Size*2, row+1);
	    A->ppRows = A->ppRows ? (TPSM_Row*)UtReAllocMem(A->ppRows, sizeof(TPSM_Row) * new_size) :
                                (TPSM_Row*)UtAllocMem(sizeof(TPSM_Row) * new_size);

	    for(i = A->nRows_Size; i < new_size; i++) 
        {
	        A->ppRows[i] = NULL;
	    }
	    A->nRows_Size = new_size;
    }

    if (col >= A->nCols_Size)
    {
	    new_size = MEspressoMax(A->nCols_Size*2, col+1);
	    A->ppCols = A->ppCols ? (TPSM_Col*)UtReAllocMem(A->ppCols, sizeof(TPSM_Col) * new_size) :
	                            (TPSM_Col*)UtAllocMem(sizeof(TPSM_Col) * new_size);

	    for(i = A->nCols_Size; i < new_size; i++)
        {
	        A->ppCols[i] = NULL;
	    }
	    A->nCols_Size = new_size;
    }
}

TPSM_Matrix sm_dup(
  TPSM_Matrix A)
{
    register TPSM_Row prow;
    register TPSM_Element p;
    register TPSM_Matrix B;

    B = sm_alloc();

    if (A->pLast_Row != 0)
    {
	    sm_resize(B, A->pLast_Row->nRow_Num, A->pLast_Col->nCol_Num);

	    for(prow = A->pFirst_Row; prow != 0; prow = prow->pNext_Row)
        {
	        for(p = prow->pFirst_Col; p != 0; p = p->pNext_Col)
            {
		        sm_insert(B, p->nRow_Num, p->nCol_Num);
	        }
	    }
    }

    return B;
}

/*  
 *  insert -- insert a value into the matrix
 */
TPSM_Element sm_insert(
  register TPSM_Matrix A,
  register TEspressoInt32 row,
  register TEspressoInt32 col)
{
    register TPSM_Row prow;
    register TPSM_Col pcol;
    register TPSM_Element element;
    TPSM_Element save_element;

    if (row >= A->nRows_Size || col >= A->nCols_Size) 
    {
	    sm_resize(A, row, col);
    }

    prow = A->ppRows[row];
    if (prow == NULL)
    {
	    prow = A->ppRows[row] = sm_row_alloc();
	    prow->nRow_Num = row;
	    MSorted_Insert(TSM_Row, A->pFirst_Row, A->pLast_Row, A->nNumRows, 
			           pNext_Row, pPrev_Row, nRow_Num, row, prow);
    }

    pcol = A->ppCols[col];
    if (pcol == NULL)
    {
	    pcol = A->ppCols[col] = sm_col_alloc();
	    pcol->nCol_Num = col;
	    MSorted_Insert(TSM_Col, A->pFirst_Col, A->pLast_Col, A->nNumCols, 
                       pNext_Col, pPrev_Col, nCol_Num, col, pcol);
    }

    /* get a new item, save its address */
    element = (TPSM_Element)UtAllocMem(sizeof(TSM_Element));
    save_element = element;

    /* insert it into the row list */
    MSorted_Insert(TSM_Element, prow->pFirst_Col, prow->pLast_Col, 
                   prow->nLength, pNext_Col, pPrev_Col, nCol_Num, col, element);

    /* if it was used, also insert it into the column list */
    if (element == save_element)
    {
	    MSorted_Insert(TSM_Element, pcol->pFirst_Row, pcol->pLast_Row, 
		               pcol->nLength, pNext_Row, pPrev_Row, nRow_Num, row, element);
    }
    else
    {
    	/* otherwise, it was already in matrix -- free element we allocated */
        if (save_element)
        {
            UtFreeMem(save_element);

            save_element = NULL;
        }
    }

    return element;
}

void sm_cleanup()
{
}

/*
 *  allocate a new row vector 
 */
TPSM_Row sm_row_alloc()
{
    register TPSM_Row prow;

    prow = (TPSM_Row)UtAllocMem(sizeof(TSM_Row));

    prow->nRow_Num = 0;
    prow->nLength = 0;
    prow->pFirst_Col = prow->pLast_Col = NULL;
    prow->pNext_Row = prow->pPrev_Row = NULL;
    prow->nFlag = 0;

    return prow;
}

/*
 *  free a row vector -- for FAST_AND_LOOSE, this is real cheap for rows;
 *  however, freeing a column must still walk down the column discarding
 *  the elements one-by-one; that is the only use for the extra '-DCOLS'
 *  compile flag ...
 */
void sm_row_free(
  register TPSM_Row prow)
{
    register TPSM_Element p, pnext;

    for(p = prow->pFirst_Col; p != 0; p = pnext)
    {
	    pnext = p->pNext_Col;

        if (p)
        {
            UtFreeMem(p);

            p = NULL;
        }
    }

    if (prow)
    {
        UtFreeMem(prow);

        prow = NULL;
    }
}

/*
 *  duplicate an existing row
 */
TPSM_Row sm_row_dup(
  register TPSM_Row prow)
{
    register TPSM_Row pnew;
    register TPSM_Element p;

    pnew = sm_row_alloc();
    for(p = prow->pFirst_Col; p != 0; p = p->pNext_Col)
    {
	    sm_row_insert(pnew, p->nCol_Num);
    }
    return pnew;
}

/*
 *  insert an element into a row vector 
 */
TPSM_Element sm_row_insert(
  register TPSM_Row prow,
  register TEspressoInt32 col)
{
    register TPSM_Element test, element;

    /* get a new item, save its address */
    element = (TPSM_Element)UtAllocMem(sizeof(TSM_Element));
    test = element;
    MSorted_Insert(TSM_Element, prow->pFirst_Col, prow->pLast_Col, prow->nLength, 
		           pNext_Col, pPrev_Col, nCol_Num, col, test);

    /* if item was not used, free it */
    if (element != test)
    {
        if (element)
        {
            UtFreeMem(element);

            element = NULL;
        }
    }

    /* either way, return the current new value */
    return test;
}

void sm_delrow(
  TPSM_Matrix A,
  TEspressoInt32 i)
{
    register TPSM_Element p, pnext;
    TPSM_Col pcol;
    TPSM_Row prow;

    prow = MSM_Get_Row(A, i);
    if (prow != NULL)
    {
	    /* walk across the row */
	    for(p = prow->pFirst_Col; p != 0; p = pnext)
        {
	        pnext = p->pNext_Col;

	        /* unlink the item from the column (and delete it) */
	        pcol = MSM_Get_Col(A, p->nCol_Num);
	        sm_col_remove_element(pcol, p);

	        /* discard the column if it is now empty */
	        if (pcol->pFirst_Row == NULL)
            {
		        sm_delcol(A, pcol->nCol_Num);
	        }
	    }

	    /* discard the row -- we already threw away the elements */ 
	    A->ppRows[i] = NULL;
	    MDLL_Unlink(prow, A->pFirst_Row, A->pLast_Row, 
				    pNext_Row, pPrev_Row, A->nNumRows);
	    prow->pFirst_Col = prow->pLast_Col = NULL;
	    sm_row_free(prow);
    }
}

TEspressoInt32 sm_row_dominance(
  TPSM_Matrix A)
{
    register TPSM_Row prow, prow1;
    register TPSM_Col pcol, least_col;
    register TPSM_Element p, pnext;
    TEspressoInt32 rowcnt;

    rowcnt = A->nNumRows;

    /* Check each row against all other rows */
    for(prow = A->pFirst_Row; prow != 0; prow = prow->pNext_Row)
    {
	    /* Among all columns with a 1 in this row, choose smallest */
	    least_col = MSM_Get_Col(A, prow->pFirst_Col->nCol_Num);
	    for(p = prow->pFirst_Col->pNext_Col; p != 0; p = p->pNext_Col)
        {
	        pcol = MSM_Get_Col(A, p->nCol_Num);
	        if (pcol->nLength < least_col->nLength)
            {
		        least_col = pcol;
	        }
	    }

	    /* Only check for containment against rows in this column */
	    for(p = least_col->pFirst_Row; p != 0; p = pnext)
        {
	        pnext = p->pNext_Row;

	        prow1 = MSM_Get_Row(A, p->nRow_Num);
	        if ((prow1->nLength > prow->nLength) ||
	                  (prow1->nLength == prow->nLength && 
			          prow1->nRow_Num > prow->nRow_Num))
            {
		        if (sm_row_contains(prow, prow1))
                {
		            sm_delrow(A, prow1->nRow_Num);
		        }
	        }
	    }
    }

    return rowcnt - A->nNumRows;
}

/*
 *  return 1 if row p2 contains row p1; 0 otherwise
 */
TEspressoInt32 sm_row_contains(
  TPSM_Row p1,
  TPSM_Row p2)
{
    register TPSM_Element q1, q2;

    q1 = p1->pFirst_Col;
    q2 = p2->pFirst_Col;
    while (q1 != 0)
    {
	    if (q2 == 0 || q1->nCol_Num < q2->nCol_Num) 
        {
	        return 0;
	    }
        else if (q1->nCol_Num == q2->nCol_Num)
        {
	        q1 = q1->pNext_Col;
	        q2 = q2->pNext_Col;
	    }
        else 
        {
	        q2 = q2->pNext_Col;
	    }
    }
    return 1;
}

/*
 *  return 1 if row p1 and row p2 share an element in common
 */
TEspressoInt32 sm_row_intersects(
  TPSM_Row p1,
  TPSM_Row p2)
{
    register TPSM_Element q1, q2;

    q1 = p1->pFirst_Col;
    q2 = p2->pFirst_Col;
    if (q1 == 0 || q2 == 0)
    {
        return 0;
    }

    for(;;)
    {
	    if (q1->nCol_Num < q2->nCol_Num)
        {
	        if ((q1 = q1->pNext_Col) == 0)
            {
    		    return 0;
	        }
	    }
        else if (q1->nCol_Num > q2->nCol_Num)
        {
	        if ((q2 = q2->pNext_Col) == 0)
            {
    		    return 0;
	        }
	    }
        else
        {
	        return 1;
	    }
    }
}

/*
 *  remove an element from a row vector (given a pointer to the element) 
 */
void sm_row_remove_element(
  register TPSM_Row prow,
  register TPSM_Element p)
{
    MDLL_Unlink(p, prow->pFirst_Col, prow->pLast_Col, 
			    pNext_Col, pPrev_Col, prow->nLength);

    if (p)
    {
        UtFreeMem(p);

        p = NULL;
    }
}

/*
 *  allocate a new col vector 
 */
TPSM_Col sm_col_alloc()
{
    register TPSM_Col pcol;

    pcol = (TPSM_Col)UtAllocMem(sizeof(TSM_Col));

    pcol->nCol_Num = 0;
    pcol->nLength = 0;
    pcol->pFirst_Row = pcol->pLast_Row = NULL;
    pcol->pNext_Col = pcol->pPrev_Col = NULL;
    pcol->nFlag = 0;
    return pcol;
}

/*
 *  free a col vector -- for FAST_AND_LOOSE, this is real cheap for cols;
 *  however, freeing a rowumn must still walk down the rowumn discarding
 *  the elements one-by-one; that is the only use for the extra '-DCOLS'
 *  compile flag ...
 */
void sm_col_free(
  register TPSM_Col pcol)
{
    register TPSM_Element p, pnext;

    for(p = pcol->pFirst_Row; p != 0; p = pnext)
    {
	    pnext = p->pNext_Row;

        if (p)
        {
            UtFreeMem(p);

            p = NULL;
        }
    }

    if (pcol)
    {
        UtFreeMem(pcol);

        pcol = NULL;
    }
}

void sm_delcol(
  TPSM_Matrix A,
  TEspressoInt32 i)
{
    register TPSM_Element p, pnext;
    TPSM_Row prow;
    TPSM_Col pcol;

    pcol = MSM_Get_Col(A, i);
    if (pcol != NULL)
    {
	    /* walk down the column */
	    for(p = pcol->pFirst_Row; p != 0; p = pnext)
        {
	        pnext = p->pNext_Row;

	        /* unlink the element from the row (and delete it) */
	        prow = MSM_Get_Row(A, p->nRow_Num);
	        sm_row_remove_element(prow, p);

	        /* discard the row if it is now empty */
	        if (prow->pFirst_Col == NULL)
            {
		        sm_delrow(A, prow->nRow_Num);
	        }
	    }

	    /* discard the column -- we already threw away the elements */ 
	    A->ppCols[i] = NULL;
	    MDLL_Unlink(pcol, A->pFirst_Col, A->pLast_Col, 
			        pNext_Col, pPrev_Col, A->nNumCols);
	    pcol->pFirst_Row = pcol->pLast_Row = NULL;
	    sm_col_free(pcol);
    }
}

TEspressoInt32 sm_col_dominance(
  TPSM_Matrix A,
  TEspressoInt32 *weight)
{
    register TPSM_Row prow;
    register TPSM_Col pcol, pcol1;
    register TPSM_Element p;
    TPSM_Row least_row;
    TPSM_Col next_col;
    TEspressoInt32 colcnt;

    colcnt = A->nNumCols;

    /* Check each column against all other columns */
    for(pcol = A->pFirst_Col; pcol != 0; pcol = next_col)
    {
	    next_col = pcol->pNext_Col;

	    /* Check all rows to find the one with fewest elements */
	    least_row = MSM_Get_Row(A, pcol->pFirst_Row->nRow_Num);
	    for(p = pcol->pFirst_Row->pNext_Row; p != 0; p = p->pNext_Row)
        {
	        prow = MSM_Get_Row(A, p->nRow_Num);
	        if (prow->nLength < least_row->nLength)
            {
    		    least_row = prow;
	        }
	    }

	    /* Only check for containment against columns in this row */
	    for(p = least_row->pFirst_Col; p != 0; p = p->pNext_Col)
        {
	        pcol1 = MSM_Get_Col(A, p->nCol_Num);
	        if (weight != 0 && weight[pcol1->nCol_Num] > weight[pcol->nCol_Num])
            {
    		    continue;
            }

	        if ((pcol1->nLength > pcol->nLength) ||
	               (pcol1->nLength == pcol->nLength && pcol1->nCol_Num > pcol->nCol_Num))
            {
		        if (sm_col_contains(pcol, pcol1))
                {
		            sm_delcol(A, pcol->nCol_Num);
		            break;
		        }
	        }
	    }
    }

    return colcnt - A->nNumCols;
}

/*
 *  return 1 if col p2 contains col p1; 0 otherwise
 */
TEspressoInt32 sm_col_contains(
  TPSM_Col p1,
  TPSM_Col p2)
{
    register TPSM_Element q1, q2;

    q1 = p1->pFirst_Row;
    q2 = p2->pFirst_Row;
    while (q1 != 0)
    {
	    if (q2 == 0 || q1->nRow_Num < q2->nRow_Num)
        {
	        return 0;
	    }
        else if (q1->nRow_Num == q2->nRow_Num)
        {
	        q1 = q1->pNext_Row;
	        q2 = q2->pNext_Row;
	    }
        else
        {
	        q2 = q2->pNext_Row;
	    }
    }
    return 1;
}

/* originally in part.c */

static TEspressoInt32 visit_row(TPSM_Matrix A, TPSM_Row prow, TEspressoInt32 *rows_visited, TEspressoInt32 *cols_visited);
static TEspressoInt32 visit_col(TPSM_Matrix A, TPSM_Col pcol, TEspressoInt32 *rows_visited, TEspressoInt32 *cols_visited);

static void copy_row(
  register TPSM_Matrix A,
  register TPSM_Row prow)
{
    register TPSM_Element p;

    for(p = prow->pFirst_Col; p != 0; p = p->pNext_Col) 
    {
	    sm_insert(A, p->nRow_Num, p->nCol_Num);
    }
}

static TEspressoInt32 visit_row(
  TPSM_Matrix A,
  TPSM_Row prow,
  TEspressoInt32 *rows_visited,
  TEspressoInt32 *cols_visited)
{
    TPSM_Element p;
    TPSM_Col pcol;

    if (! prow->nFlag)
    {
	    prow->nFlag = 1;

	    (*rows_visited)++;

	    if (*rows_visited == A->nNumRows)
        {
	        return 1;
	    }
	    for(p = prow->pFirst_Col; p != 0; p = p->pNext_Col)
        {
	        pcol = MSM_Get_Col(A, p->nCol_Num);
	        if (! pcol->nFlag)
            {
		        if (visit_col(A, pcol, rows_visited, cols_visited))
                {
		            return 1;
		        }
	        }
	    }
    }
    return 0;
}


static TEspressoInt32 visit_col(
  TPSM_Matrix A,
  TPSM_Col pcol,
  TEspressoInt32 *rows_visited,
  TEspressoInt32 *cols_visited)
{
    TPSM_Element p;
    TPSM_Row prow;

    if (! pcol->nFlag)
    {
	    pcol->nFlag = 1;
	    (*cols_visited)++;
	    if (*cols_visited == A->nNumCols)
        {
	        return 1;
	    }
	    for(p = pcol->pFirst_Row; p != 0; p = p->pNext_Row)
        {
	        prow = MSM_Get_Row(A, p->nRow_Num);
	        if (! prow->nFlag)
            {
		        if (visit_row(A, prow, rows_visited, cols_visited))
                {
		            return 1;
		        }
	        }
	    }
    }
    return 0;
}

TEspressoInt32 sm_block_partition(
  TPSM_Matrix A,
  TPSM_Matrix *L,
  TPSM_Matrix *R)
{
    TEspressoInt32 cols_visited, rows_visited;
    register TPSM_Row prow;
    register TPSM_Col pcol;

    /* Avoid the trivial case */
    if (A->nNumRows == 0)
    {
    	return 0;
    }

    /* Reset the visited flags for each row and column */
    for(prow = A->pFirst_Row; prow != 0; prow = prow->pNext_Row)
    {
    	prow->nFlag = 0;
    }
    for(pcol = A->pFirst_Col; pcol != 0; pcol = pcol->pNext_Col)
    {
    	pcol->nFlag = 0;
    }

    cols_visited = rows_visited = 0;
    if (visit_row(A, A->pFirst_Row, &rows_visited, &cols_visited))
    {
	    /* we found all of the rows */
	    return 0;
    }
    else
    {
	    *L = sm_alloc();
	    *R = sm_alloc();
	    for(prow = A->pFirst_Row; prow != 0; prow = prow->pNext_Row)
        {
	        if (prow->nFlag)
            {
    		    copy_row(*L, prow);
	        }
            else
            {
    		    copy_row(*R, prow);
	        }
	    }
	    return 1;
    }
}

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
