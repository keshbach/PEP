/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#if !defined(espresso_sparse_h)
#define espresso_sparse_h

/*
 *  sparse matrix element
 */
typedef struct tagTSM_Element
{
    TEspressoInt32         nRow_Num;  /* row number of this element */
    TEspressoInt32         nCol_Num;  /* column number of this element */
    struct tagTSM_Element* pNext_Row; /* next row in this column */
    struct tagTSM_Element* pPrev_Row; /* previous row in this column */
    struct tagTSM_Element* pNext_Col; /* next column in this row */
    struct tagTSM_Element* pPrev_Col; /* previous column in this row */
} TSM_Element, *TPSM_Element;


/*
 *  row header
 */
typedef struct tagTSM_Row
{
    TEspressoInt32     nRow_Num;	/* the row number */
    TEspressoInt32     nLength;		/* number of elements in this row */
    TEspressoInt32     nFlag;		/* user-defined word */
    TPSM_Element       pFirst_Col;	/* first element in this row */
    TPSM_Element       pLast_Col;	/* last element in this row */
    struct tagTSM_Row* pNext_Row;	/* next row (in sm_matrix linked list) */
    struct tagTSM_Row* pPrev_Row;	/* previous row (in sm_matrix linked list) */
} TSM_Row, *TPSM_Row;


/*
 *  column header
 */
typedef struct tagTSM_Col
{
    TEspressoInt32     nCol_Num;	/* the column number */
    TEspressoInt32     nLength;		/* number of elements in this column */
    TEspressoInt32     nFlag;		/* user-defined word */
    TPSM_Element       pFirst_Row;	/* first element in this column */
    TPSM_Element       pLast_Row;	/* last element in this column */
    struct tagTSM_Col* pNext_Col;	/* next column (in sm_matrix linked list) */
    struct tagTSM_Col* pPrev_Col;	/* prev column (in sm_matrix linked list) */
} TSM_Col, *TPSM_Col;


/*
 *  A sparse matrix
 */
typedef struct TSM_Matrix
{
    TPSM_Row*      ppRows;		/* pointer to row headers (by row #) */
    TEspressoInt32 nRows_Size;	/* alloc'ed size of above array */
    TPSM_Col*      ppCols;		/* pointer to column headers (by col #) */
    TEspressoInt32 nCols_Size;	/* alloc'ed size of above array */
    TPSM_Row       pFirst_Row;	/* first row (linked list of all rows) */
    TPSM_Row       pLast_Row;	/* last row (linked list of all rows) */
    TEspressoInt32 nNumRows;	/* number of rows */
    TPSM_Col       pFirst_Col;	/* first column (linked list of columns) */
    TPSM_Col       pLast_Col;	/* last column (linked list of columns) */
    TEspressoInt32 nNumCols;	/* number of columns */
} TSM_Matrix, *TPSM_Matrix;


#define MSM_Get_Col(A, colnum)	\
    (((colnum) >= 0 && (colnum) < (A)->nCols_Size) ? \
	(A)->ppCols[colnum] : NULL)

#define MSM_Get_Row(A, rownum)	\
    (((rownum) >= 0 && (rownum) < (A)->nRows_Size) ? \
	(A)->ppRows[rownum] : NULL)

#define MSM_Foreach_Row(A, prow)	\
	for (prow = A->pFirst_Row; prow != 0; prow = prow->pNext_Row)

#define MSM_Foreach_Col(A, pcol)	\
	for (pcol = A->pFirst_Col; pcol != 0; pcol = pcol->pNext_Col)

#define MSM_Foreach_Row_Element(prow, p)	\
	for (p = prow->pFirst_Col; p != 0; p = p->pNext_Col)


TPCover make_sparse(TPCubeContext pCubeContext, TPCover F, TPCover D, TPCover R);

TPCover mv_reduce(TPCubeContext pCubeContext, TPCover F, TPCover D);






TPSM_Matrix sm_alloc();

void sm_free(TPSM_Matrix A);

void sm_resize(register TPSM_Matrix A, TEspressoInt32 row, TEspressoInt32 col);

TPSM_Matrix sm_dup(TPSM_Matrix A);

TPSM_Element sm_insert(register TPSM_Matrix A, register TEspressoInt32 row, register TEspressoInt32 col);

void sm_cleanup();





TPSM_Row sm_row_alloc();

void sm_row_free(register TPSM_Row prow);

TPSM_Row sm_row_dup(register TPSM_Row prow);

TPSM_Element sm_row_insert(register TPSM_Row prow, register TEspressoInt32 col);

void sm_delrow(TPSM_Matrix A, TEspressoInt32 i);

TEspressoInt32 sm_row_dominance(TPSM_Matrix A);

TEspressoInt32 sm_row_contains(TPSM_Row p1, TPSM_Row p2);

TEspressoInt32 sm_row_intersects(TPSM_Row p1, TPSM_Row p2);

void sm_row_remove_element(register TPSM_Row prow, register TPSM_Element p);





TPSM_Col sm_col_alloc();

void sm_col_free(register TPSM_Col pcol);

void sm_delcol(TPSM_Matrix A, TEspressoInt32 i);

TEspressoInt32 sm_col_dominance(TPSM_Matrix A, TEspressoInt32 *weight);

TEspressoInt32 sm_col_contains(TPSM_Col p1, TPSM_Col p2);



/* originally in part.c */

TEspressoInt32 sm_block_partition(TPSM_Matrix A, TPSM_Matrix *L, TPSM_Matrix *R);

#endif /* end of espresso_sparse_h */

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
