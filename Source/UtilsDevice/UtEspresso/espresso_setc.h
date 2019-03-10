/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#if !defined(espresso_setc_h)
#define espresso_setc_h

TEspressoInt32 cactive(TPCubeContext pCubeContext, register TPCube a);

TEspressoBool ccommon(TPCubeContext pCubeContext, register TPCube a, register TPCube b, register TPCube cof);

int CEspressoQSortCallStyle descend(void* context, const void* elem1, const void* elem2);

int CEspressoQSortCallStyle ascend(void* context, const void* elem1, const void* elem2);

int CEspressoQSortCallStyle d1_order(void* context, const void* elem1, const void* elem2);

TEspressoInt32 desc1(register TPSet a, register TPSet b);

TEspressoBool cdist0(TPCubeContext pCubeContext, register TPCube a, register TPCube b);

TEspressoInt32 cdist01(TPCubeContext pCubeContext, register TPSet a, register TPSet b);

TEspressoInt32 cdist(TPCubeContext pCubeContext, register TPSet a, register TPSet b);

TPSet force_lower(TPCubeContext pCubeContext, TPSet xlower, register TPSet a, register TPSet b);

void consensus(TPCubeContext pCubeContext, TPCube r, register TPCube a, register TPCube b);

TEspressoBool full_row(TPCubeContext pCubeContext, register TPCube p, register TPCube cof);

#endif /* end of espresso_setc_h */

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
