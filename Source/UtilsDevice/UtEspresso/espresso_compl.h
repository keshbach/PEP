/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#if !defined(espresso_compl_h)
#define espresso_compl_h

void compl_d1merge(TPCubeContext pCubeContext, register TPCube *L1, register TPCube *R1);

void compl_lift(TPCubeContext pCubeContext, TPCube *A1, TPCube *B1, TPCube bcube, TEspressoInt32 var);

void compl_lift_onset(TPCubeContext pCubeContext, TPCube *A1, TPCover T, TPCube bcube, TEspressoInt32 var);

void compl_lift_onset_complex(TPCubeContext pCubeContext, TPCube *A1, TPCover T, TEspressoInt32 var);

TPCover compl_cube(TPCubeContext pCubeContext, register TPCube p);

TEspressoBool compl_special_cases(TPCubeContext pCubeContext, TPCube *T, TPCover *Tbar);

TPCover compl_merge(TPCubeContext pCubeContext,
                    TPCube *T1,
                    TPCover L,
                    TPCover R,
                    register TPCube cl,
                    register TPCube cr,
                    TEspressoInt32 var,
                    TEspressoInt32 lifting);

TPCover complement(TPCubeContext pCubeContext, TPCube *T);

#endif /* end of espresso_compl_h */

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
