/***************************************************************************/
/*  Copyright (C) 2012-2012 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(MatchTerms_H)
#define MatchTerms_H

BOOL AreSimiliarTermsPresent(LPCWSTR* ppszTerms, LPCWSTR* ppszTermsMatch);

LPWSTR* AllocMergedTerms(LPCWSTR* ppszTerms, LPCWSTR* ppszTermsMatch);

VOID FreeMergedTerms(LPWSTR* ppszTerms);

#endif /* end of MatchTerms_H */

/***************************************************************************/
/*  Copyright (C) 2012-2012 Kevin Eshbach                                  */
/***************************************************************************/
