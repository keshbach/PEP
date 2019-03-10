/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#if !defined(espresso_essen_h)
#define espresso_essen_h

TPCover essential(TPCubeContext pCubeContext, TPCover *Fp, TPCover *Dp);

TEspressoBool essen_cube(TPCubeContext pCubeContext, const TPCover F, const TPCover D, const TPCube c);

TPCover cb_consensus(TPCubeContext pCubeContext, const TPCover T, const TPCube c);

TPCover cb_consensus_dist0(TPCubeContext pCubeContext, TPCover R, register TPCube p, register TPCube c);

#endif /* end of espresso_essen_h */

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
