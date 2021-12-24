/***************************************************************************/
/*  Copyright (C) 2018-2021 Kevin Eshbach                                  */
/***************************************************************************/

#if defined(_MANAGED)
#pragma managed(push, off)
#endif

static LPWSTR* l_ppszCommandLineArgs = NULL;
static INT l_nTotalCommandLineArgs = 0;
static BOOL l_bUseParallelPortConfiguration = FALSE;

#pragma region Functions

static BOOL UtCommandLineParserInitialize(
  _In_ INT argc,
  _In_ LPCWSTR argv[])
{
    l_ppszCommandLineArgs = (LPWSTR*)UtAllocMem(argc * sizeof(WCHAR*));

    for (INT nIndex = 0; nIndex < argc; ++nIndex)
    {
        if (lstrcmpi(argv[nIndex], L"/parallelport") != 0)
        {
            l_ppszCommandLineArgs[l_nTotalCommandLineArgs] = (LPWSTR)argv[nIndex];

            ++l_nTotalCommandLineArgs;
        }
        else
        {
            l_bUseParallelPortConfiguration = TRUE;
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
/*  Copyright (C) 2018-2021 Kevin Eshbach                                  */
/***************************************************************************/
