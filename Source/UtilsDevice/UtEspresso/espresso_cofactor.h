/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#if !defined(espresso_cofactor_h)
#define espresso_cofactor_h

TPCube* cofactor(TPCubeContext pCubeContext, TPCube *T, register const TPCube c);

TPCube* scofactor(TPCubeContext pCubeContext, TPCube *T, const TPCube c, TEspressoInt32 var);

void massive_count(TPCubeContext pCubeContext, TPCube *T);

TPCover cubeunlist(TPCubeContext pCubeContext, TPCube *A1);

TPCube* cube2list(TPCubeContext pCubeContext, TPCover A, TPCover B);

TPCube* cube3list(TPCubeContext pCubeContext, TPCover A, TPCover B, TPCover C);

TEspressoInt32 binate_split_select(TPCubeContext pCubeContext,
                                  TPCube *T,
                                  register TPCube cleft,
                                  register TPCube cright);

TPCube* cube1list(TPCubeContext pCubeContext, TPCover A);
 
#endif /* end of espresso_cofactor_h */

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
