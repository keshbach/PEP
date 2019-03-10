/***************************************************************************/
/*  Copyright (C) 2007-2013 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtPalDeviceAdapterDefs_H)
#define UtPalDeviceAdapterDefs_H

/*
    Pin Type values
*/

static LPCWSTR l_pszI_PinTypeValues[] = {
    CInputPinType};

static LPCWSTR l_pszI_NU_PinTypeValues[] = {
    CInputPinType,
    CNotUsedPinType};

static LPCWSTR l_pszO_NU_PinTypeValues[] = {
    COutputPinType,
    CNotUsedPinType};

static LPCWSTR l_pszI_O_PinTypeValues[] = {
    CInputPinType,
    COutputPinType};
    
static LPCWSTR l_pszI_O_R_PinTypeValues[] = {
    CInputPinType,
    COutputPinType,
    CRegisteredPinType};

static LPCWSTR l_pszR_PinTypeValues[] = {
    CRegisteredPinType};

/*
    Pin Polarity values
*/

static LPCWSTR l_pszL_PinPolarityValues[] = {
    CActiveLowPinPolarity};

static LPCWSTR l_pszH_PinPolarityValues[] = {
    CActiveHighPinPolarity};

static LPCWSTR l_pszL_H_PinPolarityValues[] = {
    CActiveLowPinPolarity,
    CActiveHighPinPolarity};

#endif /* end of UtPalDeviceAdapterDefs_H */

/***************************************************************************/
/*  Copyright (C) 2007-2013 Kevin Eshbach                                  */
/***************************************************************************/
