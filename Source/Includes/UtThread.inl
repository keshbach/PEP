/***************************************************************************/
/*  Copyright (C) 2018-2018 Kevin Eshbach                                  */
/***************************************************************************/

#if defined(_MANAGED)
#pragma managed(push, off)
#endif

#pragma region Constants

#define MS_VC_EXCEPTION 0x406D1388

#pragma endregion

#pragma region Structures

#pragma pack(push,8)

typedef struct tagTHREADNAMEINFO
{
    DWORD dwType; // Must be 0x1000.
    LPCSTR pszName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAMEINFO;

#pragma pack(pop)

#pragma endregion

#pragma region Functions

static void UtSetThreadName(
  _In_ DWORD dwThreadID,
  _In_z_ LPCSTR pszThreadName)
{
    THREADNAMEINFO ThreadNameInfo;

    ThreadNameInfo.dwType = 0x1000;
    ThreadNameInfo.pszName = pszThreadName;
    ThreadNameInfo.dwThreadID = dwThreadID;
    ThreadNameInfo.dwFlags = 0;

    __try
    {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(ThreadNameInfo) / sizeof(ULONG_PTR), (ULONG_PTR*)&ThreadNameInfo);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
    }
}

#pragma endregion

#if defined(_MANAGED)
#pragma managed(pop)
#endif

/***************************************************************************/
/*  Copyright (C) 2018-2018 Kevin Eshbach                                  */
/***************************************************************************/
