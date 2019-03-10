/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#if !defined(espresso_cvrm_h)
#define espresso_cvrm_h

TPCover unravel_range(TPCubeContext pCubeContext, const TPCover B, TEspressoInt32 nStart, TEspressoInt32 nEnd);

TPCover unravel(TPCubeContext pCubeContext, const TPCover B, TEspressoInt32 nStart);

TEspressoInt32 cubelist_partition(TPCubeContext pCubeContext, TPCube *T, TPCube **A, TPCube **B);

TPCover cof_output(TPCubeContext pCubeContext, TPCover T, register TEspressoInt32 i);

TPCover random_order(register TPCover F);

TPCover mini_sort(TPCubeContext pCubeContext, TPCover F, TPCompareFunc pCompareFunc);

TPCover sort_reduce(TPCubeContext pCubeContext, const TPCover T);

#endif /* end of espresso_cvrm_h */

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
