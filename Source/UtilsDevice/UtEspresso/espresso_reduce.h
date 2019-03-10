/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#if !defined(espresso_reduce_h)
#define espresso_reduce_h

TPCover reduce(TPCubeContext pCubeContext, TPCover F, const TPCover D);

TPCube reduce_cube(TPCubeContext pCubeContext, TPCube *FD, const TPCube p);

TPCube sccc(TPCubeContext pCubeContext, TPCube *T);

TPCube sccc_merge(register TPCube left,
                  register TPCube right,
                  register TPCube cl,
                  register TPCube cr);

TPCube sccc_cube(TPCubeContext pCubeContext, register TPCube result, register TPCube p);

TEspressoBool sccc_special_cases(TPCubeContext pCubeContext, TPCube *T, TPCube *result);

#endif /* end of espresso_reduce_h */

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
