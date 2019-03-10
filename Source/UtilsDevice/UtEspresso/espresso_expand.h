/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#if !defined(espresso_expand_h)
#define espresso_expand_h

TPCover expand(TPCubeContext pCubeContext, TPCover F, const TPCover R, TEspressoBool nonsparse);

void expand1(TPCubeContext pCubeContext, TPCover BB, TPCover CC, TPCube RAISE, TPCube FREESET, TPCube OVEREXPANDED_CUBE,
             TPCube SUPER_CUBE, TPCube INIT_LOWER, TEspressoInt32 *num_covered, TPCube c);

TEspressoInt32 most_frequent(TPCubeContext pCubeContext, TPCover CC, TPCube FREESET);

void setup_BB_CC(register TPCover BB, register TPCover CC);

void select_feasible(TPCubeContext pCubeContext, TPCover BB, TPCover CC, TPCube RAISE, TPCube FREESET, TPCube SUPER_CUBE, TEspressoInt32 *num_covered);

void elim_lowering(TPCubeContext pCubeContext, TPCover BB, TPCover CC, TPCube RAISE, TPCube FREESET);

void essen_parts(TPCubeContext pCubeContext, TPCover BB, TPCover CC, TPCube RAISE, TPCube FREESET);

void essen_raising(TPCubeContext pCubeContext, register TPCover BB, TPCube RAISE, TPCube FREESET);

void mincov(TPCubeContext pCubeContext, TPCover BB, TPCube RAISE, TPCube FREESET);

TEspressoBool feasibly_covered(TPCubeContext pCubeContext, TPCover BB, TPCube c, TPCube RAISE, TPCube new_lower);

#endif /* end of espresso_expand_h */

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
