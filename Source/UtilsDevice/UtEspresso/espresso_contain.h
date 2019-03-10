/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#if !defined(espresso_contain_h)
#define espresso_contain_h

TEspressoInt32 d1_rm_equal(register TPSet* A1, TPCompareFunc pCompareFunc);

TEspressoInt32 rm_equal(TPSet* A1, TPCompareFunc PCompareFunc);

TEspressoInt32 rm_contain(TPSet*A1);

TEspressoInt32 rm_rev_contain(TPSet* A1);

TEspressoInt32 rm2_equal(register TPSet* A1, register TPSet* B1, TPSet* E1, TPCompareFunc pCompareFunc);

TEspressoInt32 rm2_contain(TPSet* A1, TPSet* B1);

TPSet_Family sf_contain(TPCubeContext pCubeContext, TPSet_Family A);

TPSet* sf_sort(TPCubeContext pCubeContext, TPSet_Family A, TPCompareFunc pCompareFunc);

TPSet* sf_list(register const TPSet_Family A);

TPSet_Family sf_unlist(TPCubeContext pCubeContext, TPSet* A1, TEspressoInt32 nTotCnt, TEspressoInt32 nSize);

TPSet_Family sf_rev_contain(TPCubeContext pCubeContext, TPSet_Family A);

TPSet_Family sf_union(TPCubeContext pCubeContext, TPSet_Family A, TPSet_Family B);

TPSet_Family sf_merge(TPCubeContext pCubeContext, TPSet *A1, TPSet *B1, TPSet *E1, TEspressoInt32 nTotCnt, TEspressoInt32 nSize);

TPSet_Family sf_dupl(TPCubeContext pCubeContext, TPSet_Family A);

TPSet_Family dist_merge(TPCubeContext pCubeContext, TPSet_Family A, const TPSet mask);

TPSet_Family d1merge(TPCubeContext pCubeContext, TPSet_Family A, TEspressoInt32 nVar);

#endif /* end of espresso_contain_h */

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
