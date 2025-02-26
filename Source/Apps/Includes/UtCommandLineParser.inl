/***************************************************************************/
/*  Copyright (C) 2018-2024 Kevin Eshbach                                  */
/***************************************************************************/

#if defined(_MANAGED)
#pragma managed(push, off)
#endif

static LPWSTR* l_ppszCommandLineArgs = NULL;
static INT l_nTotalCommandLineArgs = 0;
static BOOL l_bUseParallelPortConfiguration = FALSE;
static BOOL l_bNoUserInteractionConfiguration = FALSE;

#pragma region Functions

static BOOL UtCommandLineParserInitialize(
  _In_ INT argc,
  _In_ LPCWSTR argv[])
{
    l_ppszCommandLineArgs = (LPWSTR*)UtAllocMem(argc * sizeof(WCHAR*));

    for (INT nIndex = 0; nIndex < argc; ++nIndex)
    {
        if (lstrcmpi(argv[nIndex], L"/parallelport") == 0)
        {
            l_bUseParallelPortConfiguration = TRUE;
        }
        else if (lstrcmpi(argv[nIndex], L"/nouserinteraction") == 0)
        {
            l_bNoUserInteractionConfiguration = TRUE;
        }
        else
        {
            l_ppszCommandLineArgs[l_nTotalCommandLineArgs] = (LPWSTR)argv[nIndex];

            ++l_nTotalCommandLineArgs;
        }
    }

    return TRUE;
}

static VOID UtCommandLineParserUninitialize(VOID)
{
    UtFreeMem(l_ppszCommandLineArgs);
}

static BOOL UtCommandLineParserGetUseParallelPortConfiguration(VOID)
{
    return l_bUseParallelPortConfiguration;
}

static BOOL UtCommandLineParserGetNoUserInteractionConfiguration(VOID)
{
    return l_bNoUserInteractionConfiguration;
}

static LPCWSTR* UtCommandLineParserGetArguments(VOID)
{
    return l_ppszCommandLineArgs;
}

static INT UtCommandLineParserGetTotalArguments(VOID)
{
    return l_nTotalCommandLineArgs;
}

#pragma endregion

#if defined(_MANAGED)
#pragma managed(pop)
#endif

/***************************************************************************/
/*  Copyright (C) 2018-2024 Kevin Eshbach                                  */
/***************************************************************************/
