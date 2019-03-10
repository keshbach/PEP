/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/

#if !defined(espresso_pair_h)
#define espresso_pair_h

void set_pair(TPCubeContext pCubeContext, TPPLA PLA);

void set_pair1(TPCubeContext pCubeContext, TPPLA PLA, TEspressoBool adjust_labels);

TPCover pairvar(TPCubeContext pCubeContext, TPCover A, TPPair pair);

TPCover delvar(TPCubeContext pCubeContext, TPCover A, TEspressoBool paired[]);

#endif /* end of espresso_pair_h */

/***************************************************************************/
/*  Copyright (c) 1988, 1989, Regents of the University of California.     */
/*  All rights reserved.                                                   */
/***************************************************************************/
