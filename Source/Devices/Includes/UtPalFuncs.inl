/***************************************************************************/
/*  Copyright (C) 2007-2013 Kevin Eshbach                                  */
/***************************************************************************/

static const TPALData* lFindPALData(
  LPCWSTR pszName)
{
    ULONG ulIndex;

    for (ulIndex = 0; l_Devices[ulIndex].pszName != NULL; ++ulIndex)
    {
        if (lstrcmpiW(l_Devices[ulIndex].pszName, pszName) == 0)
        {
            return &l_Devices[ulIndex].DeviceData.PALData;
        }
    }

    assert(0);

    return NULL;
}

/***************************************************************************/
/*  Copyright (C) 2007-2013 Kevin Eshbach                                  */
/***************************************************************************/
