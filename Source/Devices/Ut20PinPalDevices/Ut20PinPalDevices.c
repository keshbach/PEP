/***************************************************************************/
/*  Copyright (C) 2007-2020 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>
#include <strsafe.h>
#include <assert.h>

#include <UtilsDevice/UtPepDevices.h>
#include <UtilsDevice/UtPalDevices.h>
#include <UtilsDevice/UtPALDefs.h>

#include <Devices/Includes/UtDevicePinDefs.h>

#include <Devices/Includes/UtPalDeviceAdapterDefs.h>

#include <Config/UtPepCtrl.h>

#include <Utils/UtHeap.h>

#include <Includes/UtMacros.h>

static BOOL UTPEPDEVICESAPI lGenericInit(VOID);
static BOOL UTPEPDEVICESAPI lGenericUninit(VOID);

static ULONG UTPEPDEVICESAPI lGal18V10GetFuseMapSize(VOID);

static VOID UTPEPDEVICESAPI lGal18V10ConfigFuseMap(LPBYTE pbyData);

static BOOL UTPEPDEVICESAPI lPal10L8SetDevicePinConfigs(const TDevicePinConfig* pDevicePinConfigs, UINT nTotalDevicePinConfigs);
static BOOL UTPEPDEVICESAPI lPal12L6SetDevicePinConfigs(const TDevicePinConfig* pDevicePinConfigs, UINT nTotalDevicePinConfigs);
static BOOL UTPEPDEVICESAPI lPal12H6SetDevicePinConfigs(const TDevicePinConfig* pDevicePinConfigs, UINT nTotalDevicePinConfigs);
static BOOL UTPEPDEVICESAPI lPal16L2SetDevicePinConfigs(const TDevicePinConfig* pDevicePinConfigs, UINT nTotalDevicePinConfigs);
static BOOL UTPEPDEVICESAPI lPal16L8SetDevicePinConfigs(const TDevicePinConfig* pDevicePinConfigs, UINT nTotalDevicePinConfigs);
static BOOL UTPEPDEVICESAPI lPal16R4SetDevicePinConfigs(const TDevicePinConfig* pDevicePinConfigs, UINT nTotalDevicePinConfigs);
static BOOL UTPEPDEVICESAPI lPal16R6SetDevicePinConfigs(const TDevicePinConfig* pDevicePinConfigs, UINT nTotalDevicePinConfigs);
static BOOL UTPEPDEVICESAPI lPal16R8SetDevicePinConfigs(const TDevicePinConfig* pDevicePinConfigs, UINT nTotalDevicePinConfigs);
static BOOL UTPEPDEVICESAPI lGal18V10SetDevicePinConfigs(const TDevicePinConfig* pDevicePinConfigs, UINT nTotalDevicePinConfigs);

static LPCWSTR UTPEPDEVICESAPI lGal18V10AllocFuseMapText(const LPBYTE pbyData);
static VOID UTPEPDEVICESAPI lGal18V10FreeFuseMapText(LPCWSTR pszFuseMapText);

static LPCWSTR UTPEPDEVICESAPI lGal18V10AllocSampleFuseMapText(VOID);
static VOID UTPEPDEVICESAPI lGal18V10FreeSampleFuseMapText(LPCWSTR pszFuseMapText);

static VOID UTPEPDEVICESAPI lPal10L8ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI lPal12L6ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI lPal12H6ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI lPal16L2ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI lPal16L8ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI lPal16R4ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI lPal16R6ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI lPal16R8ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI lGal18V10ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, LPBYTE pbyData, ULONG ulDataLen);

static LPCWSTR l_pszPal10L8PinNames[] = {
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Ground,
    CDevicePin_Input,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Vcc};

static LPCWSTR l_pszPal12L6PinNames[] = {
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Ground,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Input,
    CDevicePin_Vcc};

static LPCWSTR l_pszPal12H6PinNames[] = {
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Ground,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Vcc};

static LPCWSTR l_pszPal16L2PinNames[] = {
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Ground,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Vcc};

static LPCWSTR l_pszPal16L8PinNames[] = {
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Ground,
    CDevicePin_Input,
    CDevicePin_Output,
    CDevicePin_InputOutput,
    CDevicePin_InputOutput,
    CDevicePin_InputOutput,
    CDevicePin_InputOutput,
    CDevicePin_InputOutput,
    CDevicePin_InputOutput,
    CDevicePin_Output,
    CDevicePin_Vcc};

static LPCWSTR l_pszPal16R4PinNames[] = {
    CDevicePin_Clock,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Ground,
    CDevicePin_OutputEnable,
    CDevicePin_InputOutput,
    CDevicePin_InputOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_InputOutput,
    CDevicePin_InputOutput,
    CDevicePin_Vcc};

static LPCWSTR l_pszPal16R6PinNames[] = {
    CDevicePin_Clock,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Ground,
    CDevicePin_OutputEnable,
    CDevicePin_InputOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_InputOutput,
    CDevicePin_Vcc};

static LPCWSTR l_pszPal16R8PinNames[] = {
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Ground,
    CDevicePin_OutputEnable,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_Vcc};

static LPCWSTR l_pszGal18V10PinNames[] = {
    CDevicePin_InputClock,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_InputOutputRegisteredOutput,
    CDevicePin_Ground,
    CDevicePin_InputOutputRegisteredOutput,
    CDevicePin_InputOutputRegisteredOutput,
    CDevicePin_InputOutputRegisteredOutput,
    CDevicePin_InputOutputRegisteredOutput,
    CDevicePin_InputOutputRegisteredOutput,
    CDevicePin_InputOutputRegisteredOutput,
    CDevicePin_InputOutputRegisteredOutput,
    CDevicePin_InputOutputRegisteredOutput,
    CDevicePin_InputOutputRegisteredOutput,
    CDevicePin_Vcc};

static TDevicePinFuseRows l_Pal10L8PinFuseRows[] = {
    {12, CNoOutputEnableFuseRow, 280, 300},
    {13, CNoOutputEnableFuseRow, 240, 260},
    {14, CNoOutputEnableFuseRow, 200, 220},
    {15, CNoOutputEnableFuseRow, 160, 180},
    {16, CNoOutputEnableFuseRow, 120, 140},
    {17, CNoOutputEnableFuseRow, 80, 100},
    {18, CNoOutputEnableFuseRow, 40, 60},
    {19, CNoOutputEnableFuseRow, 0, 20}};

static TDevicePinFuseRows l_Pal12L6PinFuseRows[] = {
    {13, CNoOutputEnableFuseRow, 288, 360},
    {14, CNoOutputEnableFuseRow, 240, 264},
    {15, CNoOutputEnableFuseRow, 192, 216},
    {16, CNoOutputEnableFuseRow, 144, 168},
    {17, CNoOutputEnableFuseRow, 96, 120}, 
    {18, CNoOutputEnableFuseRow, 0, 72}};

static TDevicePinFuseRows l_Pal12H6PinFuseRows[] = {
    {13, CNoOutputEnableFuseRow, 288, 360},
    {14, CNoOutputEnableFuseRow, 240, 264},
    {15, CNoOutputEnableFuseRow, 192, 216},
    {16, CNoOutputEnableFuseRow, 144, 168},
    {17, CNoOutputEnableFuseRow, 96, 120}, 
    {18, CNoOutputEnableFuseRow, 0, 72}};

static TDevicePinFuseRows l_Pal16L2PinFuseRows[] = {
    {15, CNoOutputEnableFuseRow, 256, 480}, 
    {16, CNoOutputEnableFuseRow, 0, 224}};

static TDevicePinFuseRows l_Pal16L8PinFuseRows[] = {
    {12, 1792, 1824, 2016},
    {13, 1536, 1568, 1760},
    {14, 1280, 1312, 1504},
    {15, 1024, 1056, 1248},
    {16, 768, 800, 992},
    {17, 512, 544, 736},
    {18, 256, 288, 480},
    {19, 0, 32, 224}};

static TDevicePinFuseRows l_Pal16R4PinFuseRows[] = {
    {12, 1792, 1824, 2016},
    {13, 1536, 1568, 1760},
    {14, CNoOutputEnableFuseRow, 1280, 1504}, /* Registered Output */
    {15, CNoOutputEnableFuseRow, 1024, 1248}, /* Registered Output */
    {16, CNoOutputEnableFuseRow, 768, 992},   /* Registered Output */
    {17, CNoOutputEnableFuseRow, 512, 736},   /* Registered Output */
    {18, 256, 288, 480},
    {19, 0, 32, 224}};

static TDevicePinFuseRows l_Pal16R6PinFuseRows[] = {
    {12, 1792, 1824, 2016},
    {13, CNoOutputEnableFuseRow, 1536, 1760}, /* Registered Output */
    {14, CNoOutputEnableFuseRow, 1280, 1504}, /* Registered Output */
    {15, CNoOutputEnableFuseRow, 1024, 1248}, /* Registered Output */
    {16, CNoOutputEnableFuseRow, 768, 992},   /* Registered Output */
    {17, CNoOutputEnableFuseRow, 512, 736},   /* Registered Output */
    {18, CNoOutputEnableFuseRow, 256, 480},   /* Registered Output */
    {19, 0, 32, 224}};

static TDevicePinFuseRows l_Pal16R8PinFuseRows[] = {
    {12, CNoOutputEnableFuseRow, 1792, 2016}, /* Registered Output */
    {13, CNoOutputEnableFuseRow, 1536, 1760}, /* Registered Output */
    {14, CNoOutputEnableFuseRow, 1280, 1504}, /* Registered Output */
    {15, CNoOutputEnableFuseRow, 1024, 1248}, /* Registered Output */
    {16, CNoOutputEnableFuseRow, 768, 992},   /* Registered Output */
    {17, CNoOutputEnableFuseRow, 512, 736},   /* Registered Output */
    {18, CNoOutputEnableFuseRow, 256, 480},   /* Registered Output */
    {19, CNoOutputEnableFuseRow, 0, 224}};    /* Registered Output */

static TDevicePinFuseRows l_Gal18V10PinFuseRows[] = {
    {9,  3096, 3132, 3384},
    {11, 2772, 2808, 3060},
    {12, 2448, 2484, 2736},
    {13, 2124, 2160, 2412},
    {14, 1728, 1764, 2088},
    {15, 1332, 1368, 1692},
    {16, 1008, 1044, 1296},
    {17, 684,  720,  972},
    {18, 360,  396,  648},
    {19, 36,   72,   324}};

static TDevicePinFuseColumns l_Pal10L8PinFuseColumns[] = {
    {1,  3,  2},
    {2,  1,  0},
    {3,  5,  4},
    {4,  7,  6},
    {5,  9,  8},
    {6,  11, 10},
    {7,  13, 12},
    {8,  15, 14},
    {9,  17, 16},
    {11, 19, 18}};

static TDevicePinFuseColumns l_Pal12L6PinFuseColumns[] = {
    {1,  3,  2},
    {2,  1,  0},
    {3,  5,  4},
    {4,  9,  8},
    {5,  11, 10},
    {6,  13, 12},
    {7,  15, 14},
    {8,  17, 16},
    {9,  21, 20},
    {11, 23, 22},
    {12, 19, 18},
    {19, 7,  6}};

static TDevicePinFuseColumns l_Pal12H6PinFuseColumns[] = {
    {1,  3,  2},
    {2,  1,  0},
    {3,  5,  4},
    {4,  9,  8},
    {5,  11, 10},
    {6,  13, 12},
    {7,  15, 14},
    {8,  17, 16},
    {9,  21, 20},
    {11, 23, 22},
    {12, 19, 18},
    {19, 7,  6}};

static TDevicePinFuseColumns l_Pal16L2PinFuseColumns[] = {
	{1, 3, 2},
	{2, 1, 0},
	{3, 5, 4},
	{4, 9, 8},
	{5, 13, 12},
	{6, 17, 16},
	{7, 21, 20},
	{8, 25, 24},
	{9, 29, 28},
	{11, 31, 30},
	{12, 27, 26},
	{13, 23, 22},
	{14, 19, 18},
	{17, 15, 14},
	{18, 11, 10},
	{19, 7, 6}};

static TDevicePinFuseColumns l_Pal16L8PinFuseColumns[] = {
    {1, 3,   2},
    {2, 1,   0},
    {3, 5,   4},
    {4, 9,   8},
    {5, 13,  12},
    {6, 17,  16},
    {7, 21,  20},
    {8, 25,  24},
    {9, 29,  28},
    {11, 31, 30},
    {13, 27, 26},
    {14, 23, 22},
    {15, 19, 18},
    {16, 15, 14},
    {17, 11, 10},
    {18, 7,  6}};

static TDevicePinFuseColumns l_Pal16R4PinFuseColumns[] = {
    {2, 1,   0},
    {3, 5,   4},
    {4, 9,   8},
    {5, 13,  12},
    {6, 17,  16},
    {7, 21,  20},
    {8, 25,  24}, 
    {9, 29,  28},
    {12, 31, 30},
    {13, 27, 26},
    {14, 23, 22}, /* Registered Output */
    {15, 19, 18}, /* Registered Output */
    {16, 15, 14}, /* Registered Output */
    {17, 11, 10}, /* Registered Output */
    {18, 7,  6},
    {19, 3,  2}};

static TDevicePinFuseColumns l_Pal16R6PinFuseColumns[] = {
    {2,  1,  0},
    {3,  5,  4},
    {4,  9,  8},
    {5,  13, 12},
    {6,  17, 16},
    {7,  21, 20},
    {8,  25, 24},
    {9,  29, 28},
    {12, 31, 30},
    {13, 27, 26}, /* Registered Output */
    {14, 23, 22}, /* Registered Output */
    {15, 19, 18}, /* Registered Output */
    {16, 15, 14}, /* Registered Output */
    {17, 11, 10}, /* Registered Output */
    {18, 7,  6},  /* Registered Output */
    {19, 3,  2}};

static TDevicePinFuseColumns l_Pal16R8PinFuseColumns[] = {
    {2,  1,  0},
    {3,  5,  4},
    {4,  9,  8},
    {5,  13, 12},
    {6,  17, 16},
    {7,  21, 20},
    {8,  25, 24},
    {9,  29, 28},
    {12, 31, 30}, /* Registered Output */
    {13, 27, 26}, /* Registered Output */
    {14, 23, 22}, /* Registered Output */
    {15, 19, 18}, /* Registered Output */
    {16, 15, 14}, /* Registered Output */
    {17, 11, 10}, /* Registered Output */
    {18, 7,  6},  /* Registered Output */
    {19, 3,  2}}; /* Registered Output */

static TDevicePinFuseColumns l_Gal18V10PinFuseColumns[] = {
    {1,  1,  0},
    {2,  5,  4},
    {3,  9,  8},
    {4,  13, 12},
    {5,  17, 16},
    {6,  21, 20},
    {7,  25, 24},
    {8,  29, 28},
    {9,  35, 34},
    {11, 33, 32},
    {12, 31, 30},
    {13, 27, 26},
    {14, 23, 22},
    {15, 19, 18},
    {16, 15, 14},
    {17, 11, 10},
    {18, 7,  6},
    {19, 3,  2}};

static TDevicePinConfigValues l_Pal10L8PinConfigValues[] = {
    MDevicePinConfigValuesNoPolarity(1, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(2, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(3, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(4, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(5, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(6, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(7, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(8, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(9, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(11, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValues(12, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(13, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(14, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(15, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(16, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(17, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(18, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(19, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues)
};

static TDevicePinConfigValues l_Pal12L6PinConfigValues[] = {
    MDevicePinConfigValuesNoPolarity(1, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(2, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(3, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(4, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(5, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(6, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(7, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(8, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(9, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(11, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(12, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValues(13, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(14, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(15, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(16, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(17, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(18, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValuesNoPolarity(19, l_pszI_NU_PinTypeValues)
};

static TDevicePinConfigValues l_Pal12H6PinConfigValues[] = {
    MDevicePinConfigValuesNoPolarity(1, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(2, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(3, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(4, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(5, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(6, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(7, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(8, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(9, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(11, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(12, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValues(13, l_pszO_NU_PinTypeValues, l_pszH_PinPolarityValues),
    MDevicePinConfigValues(14, l_pszO_NU_PinTypeValues, l_pszH_PinPolarityValues),
    MDevicePinConfigValues(15, l_pszO_NU_PinTypeValues, l_pszH_PinPolarityValues),
    MDevicePinConfigValues(16, l_pszO_NU_PinTypeValues, l_pszH_PinPolarityValues),
    MDevicePinConfigValues(17, l_pszO_NU_PinTypeValues, l_pszH_PinPolarityValues),
    MDevicePinConfigValues(18, l_pszO_NU_PinTypeValues, l_pszH_PinPolarityValues),
    MDevicePinConfigValuesNoPolarity(19, l_pszI_NU_PinTypeValues)
};

static TDevicePinConfigValues l_Pal16L2PinConfigValues[] = {
    MDevicePinConfigValuesNoPolarity(1, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(2, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(3, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(4, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(5, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(6, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(7, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(8, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(9, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(11, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(12, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(13, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(14, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValues(15, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(16, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValuesNoPolarity(17, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(18, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(19, l_pszI_NU_PinTypeValues)
};

static TDevicePinConfigValues l_Pal16L8PinConfigValues[] = {
    MDevicePinConfigValuesNoPolarity(1, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(2, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(3, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(4, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(5, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(6, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(7, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(8, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(9, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(11, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValues(12, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(13, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(14, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(15, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(16, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(17, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(18, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(19, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues)
};

static TDevicePinConfigValues l_Pal16R4PinConfigValues[] = {
    MDevicePinConfigValuesNoPolarity(1, l_pszI_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(2, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(3, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(4, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(5, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(6, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(7, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(8, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(9, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(11, l_pszI_PinTypeValues),
    MDevicePinConfigValues(12, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(13, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(14, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(15, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(16, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(17, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(18, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(19, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues)
};

static TDevicePinConfigValues l_Pal16R6PinConfigValues[] = {
    MDevicePinConfigValuesNoPolarity(1, l_pszI_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(2, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(3, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(4, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(5, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(6, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(7, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(8, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(9, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(11, l_pszI_PinTypeValues),
    MDevicePinConfigValues(12, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(13, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(14, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(15, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(16, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(17, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(18, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(19, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues)
};

static TDevicePinConfigValues l_Pal16R8PinConfigValues[] = {
    MDevicePinConfigValuesNoPolarity(1, l_pszI_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(2, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(3, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(4, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(5, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(6, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(7, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(8, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(9, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(11, l_pszI_PinTypeValues),
    MDevicePinConfigValues(12, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(13, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(14, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(15, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(16, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(17, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(18, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(19, l_pszR_PinTypeValues, l_pszL_PinPolarityValues)
};

static TDevicePinConfigValues l_Gal18V10PinConfigValues[] = {
    MDevicePinConfigValuesNoPolarity(1, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(2, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(3, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(4, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(5, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(6, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(7, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(8, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValues(9, l_pszI_O_R_PinTypeValues, l_pszL_H_PinPolarityValues),
    MDevicePinConfigValues(11, l_pszI_O_R_PinTypeValues, l_pszL_H_PinPolarityValues),
    MDevicePinConfigValues(12, l_pszI_O_R_PinTypeValues, l_pszL_H_PinPolarityValues),
    MDevicePinConfigValues(13, l_pszI_O_R_PinTypeValues, l_pszL_H_PinPolarityValues),
    MDevicePinConfigValues(14, l_pszI_O_R_PinTypeValues, l_pszL_H_PinPolarityValues),
    MDevicePinConfigValues(15, l_pszI_O_R_PinTypeValues, l_pszL_H_PinPolarityValues),
    MDevicePinConfigValues(16, l_pszI_O_R_PinTypeValues, l_pszL_H_PinPolarityValues),
    MDevicePinConfigValues(17, l_pszI_O_R_PinTypeValues, l_pszL_H_PinPolarityValues),
    MDevicePinConfigValues(18, l_pszI_O_R_PinTypeValues, l_pszL_H_PinPolarityValues),
    MDevicePinConfigValues(19, l_pszI_O_R_PinTypeValues, l_pszL_H_PinPolarityValues)
};

/*
    Devices
*/

DEVICES_BEGIN
    DEVICE_DATA_BEGIN(PAL10L8)
        DEVICE_PAL
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(20)
        DEVICE_PINNAMES(l_pszPal10L8PinNames)
        DEVICE_ADAPTER("Part# PAL20/24")
        DEVICE_MESSAGE("Set the switch to 20 Pins.")
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_PAL_DATA_BEGIN
            DEVICE_PAL_PINCONFIGVALUES(l_Pal10L8PinConfigValues)
            DEVICE_PAL_PINFUSEROWS(l_Pal10L8PinFuseRows)
            DEVICE_PAL_PINFUSECOLUMNS(l_Pal10L8PinFuseColumns)
            DEVICE_PAL_GETFUSEMAPSIZE_FUNC_NONE
            DEVICE_PAL_CONFIGFUSEMAP_FUNC_NONE
            DEVICE_PAL_SETDEVICEPINCONFIGS_FUNC(lPal10L8SetDevicePinConfigs)
            DEVICE_PAL_FUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_SAMPLEFUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_READ_FUNC(lPal10L8ReadDevice)
        DEVICE_PAL_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(PAL12L6)
        DEVICE_PAL
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(20)
        DEVICE_PINNAMES(l_pszPal12L6PinNames)
        DEVICE_ADAPTER("Part# PAL20/24")
        DEVICE_MESSAGE("Set the switch to 20 Pins.")
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_PAL_DATA_BEGIN
            DEVICE_PAL_PINCONFIGVALUES(l_Pal12L6PinConfigValues)
            DEVICE_PAL_PINFUSEROWS(l_Pal12L6PinFuseRows)
            DEVICE_PAL_PINFUSECOLUMNS(l_Pal12L6PinFuseColumns)
            DEVICE_PAL_GETFUSEMAPSIZE_FUNC_NONE
            DEVICE_PAL_CONFIGFUSEMAP_FUNC_NONE
            DEVICE_PAL_SETDEVICEPINCONFIGS_FUNC(lPal12L6SetDevicePinConfigs)
            DEVICE_PAL_FUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_SAMPLEFUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_READ_FUNC(lPal12L6ReadDevice)
        DEVICE_PAL_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(PAL12H6)
        DEVICE_PAL
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(20)
        DEVICE_PINNAMES(l_pszPal12H6PinNames)
        DEVICE_ADAPTER("Part# PAL20/24")
        DEVICE_MESSAGE("Set the switch to 20 Pins.")
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_PAL_DATA_BEGIN
            DEVICE_PAL_PINCONFIGVALUES(l_Pal12H6PinConfigValues)
            DEVICE_PAL_PINFUSEROWS(l_Pal12H6PinFuseRows)
            DEVICE_PAL_PINFUSECOLUMNS(l_Pal12H6PinFuseColumns)
            DEVICE_PAL_GETFUSEMAPSIZE_FUNC_NONE
            DEVICE_PAL_CONFIGFUSEMAP_FUNC_NONE
            DEVICE_PAL_SETDEVICEPINCONFIGS_FUNC(lPal12H6SetDevicePinConfigs)
            DEVICE_PAL_FUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_SAMPLEFUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_READ_FUNC(lPal12H6ReadDevice)
        DEVICE_PAL_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(PAL16L2)
        DEVICE_PAL
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(20)
        DEVICE_PINNAMES(l_pszPal16L2PinNames)
        DEVICE_ADAPTER("Part# PAL20/24")
        DEVICE_MESSAGE("Set the switch to 20 Pins.")
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_PAL_DATA_BEGIN
            DEVICE_PAL_PINCONFIGVALUES(l_Pal16L2PinConfigValues)
            DEVICE_PAL_PINFUSEROWS(l_Pal16L2PinFuseRows)
            DEVICE_PAL_PINFUSECOLUMNS(l_Pal16L2PinFuseColumns)
            DEVICE_PAL_GETFUSEMAPSIZE_FUNC_NONE
            DEVICE_PAL_CONFIGFUSEMAP_FUNC_NONE
            DEVICE_PAL_SETDEVICEPINCONFIGS_FUNC(lPal16L2SetDevicePinConfigs)
            DEVICE_PAL_FUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_SAMPLEFUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_READ_FUNC(lPal16L2ReadDevice)
        DEVICE_PAL_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(PAL16L8)
        DEVICE_PAL
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(20)
        DEVICE_PINNAMES(l_pszPal16L8PinNames)
        DEVICE_ADAPTER("Part# PAL20/24")
        DEVICE_MESSAGE("Set the switch to 20 Pins.")
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_PAL_DATA_BEGIN
            DEVICE_PAL_PINCONFIGVALUES(l_Pal16L8PinConfigValues)
            DEVICE_PAL_PINFUSEROWS(l_Pal16L8PinFuseRows)
            DEVICE_PAL_PINFUSECOLUMNS(l_Pal16L8PinFuseColumns)
            DEVICE_PAL_GETFUSEMAPSIZE_FUNC_NONE
            DEVICE_PAL_CONFIGFUSEMAP_FUNC_NONE
            DEVICE_PAL_SETDEVICEPINCONFIGS_FUNC(lPal16L8SetDevicePinConfigs)
            DEVICE_PAL_FUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_SAMPLEFUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_READ_FUNC(lPal16L8ReadDevice)
        DEVICE_PAL_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(PAL16R4)
        DEVICE_PAL
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(20)
        DEVICE_PINNAMES(l_pszPal16R4PinNames)
        DEVICE_ADAPTER("Part# PAL20/24")
        DEVICE_MESSAGE("Set the switch to 20 Pins.")
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_PAL_DATA_BEGIN
            DEVICE_PAL_PINCONFIGVALUES(l_Pal16R4PinConfigValues)
            DEVICE_PAL_PINFUSEROWS(l_Pal16R4PinFuseRows)
            DEVICE_PAL_PINFUSECOLUMNS(l_Pal16R4PinFuseColumns)
            DEVICE_PAL_GETFUSEMAPSIZE_FUNC_NONE
            DEVICE_PAL_CONFIGFUSEMAP_FUNC_NONE
            DEVICE_PAL_SETDEVICEPINCONFIGS_FUNC(lPal16R4SetDevicePinConfigs)
            DEVICE_PAL_FUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_SAMPLEFUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_READ_FUNC(lPal16R4ReadDevice)
        DEVICE_PAL_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(PAL16R6)
        DEVICE_PAL
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(20)
        DEVICE_PINNAMES(l_pszPal16R6PinNames)
        DEVICE_ADAPTER("Part# PAL20/24")
        DEVICE_MESSAGE("Set the switch to 20 Pins.")
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_PAL_DATA_BEGIN
            DEVICE_PAL_PINCONFIGVALUES(l_Pal16R6PinConfigValues)
            DEVICE_PAL_PINFUSEROWS(l_Pal16R6PinFuseRows)
            DEVICE_PAL_PINFUSECOLUMNS(l_Pal16R6PinFuseColumns)
            DEVICE_PAL_GETFUSEMAPSIZE_FUNC_NONE
            DEVICE_PAL_CONFIGFUSEMAP_FUNC_NONE
            DEVICE_PAL_SETDEVICEPINCONFIGS_FUNC(lPal16R6SetDevicePinConfigs)
            DEVICE_PAL_FUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_SAMPLEFUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_READ_FUNC(lPal16R6ReadDevice)
        DEVICE_PAL_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(PAL16R8)
        DEVICE_PAL
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(20)
        DEVICE_PINNAMES(l_pszPal16R8PinNames)
        DEVICE_ADAPTER("Part# PAL20/24")
        DEVICE_MESSAGE("Set the switch to 20 Pins.")
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_PAL_DATA_BEGIN
            DEVICE_PAL_PINCONFIGVALUES(l_Pal16R8PinConfigValues)
            DEVICE_PAL_PINFUSEROWS(l_Pal16R8PinFuseRows)
            DEVICE_PAL_PINFUSECOLUMNS(l_Pal16R8PinFuseColumns)
            DEVICE_PAL_GETFUSEMAPSIZE_FUNC_NONE
            DEVICE_PAL_CONFIGFUSEMAP_FUNC_NONE
            DEVICE_PAL_SETDEVICEPINCONFIGS_FUNC(lPal16R8SetDevicePinConfigs)
            DEVICE_PAL_FUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_SAMPLEFUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_READ_FUNC(lPal16R8ReadDevice)
        DEVICE_PAL_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(GAL18V10)
        DEVICE_PAL
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(20)
        DEVICE_PINNAMES(l_pszGal18V10PinNames)
        DEVICE_ADAPTER("Part# PAL20/24")
        DEVICE_MESSAGE("Set the switch to 20 Pins.")
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
		DEVICE_CHIP_ENABLE_NANO_SECS_NONE
		DEVICE_OUTPUT_ENABLE_NANO_SECS_NONE
		DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_PAL_DATA_BEGIN
            DEVICE_PAL_PINCONFIGVALUES(l_Gal18V10PinConfigValues)
            DEVICE_PAL_PINFUSEROWS(l_Gal18V10PinFuseRows)
            DEVICE_PAL_PINFUSECOLUMNS(l_Gal18V10PinFuseColumns)
            DEVICE_PAL_GETFUSEMAPSIZE_FUNC(lGal18V10GetFuseMapSize)
            DEVICE_PAL_CONFIGFUSEMAP_FUNC(lGal18V10ConfigFuseMap)
            DEVICE_PAL_SETDEVICEPINCONFIGS_FUNC(lGal18V10SetDevicePinConfigs)
            DEVICE_PAL_FUSEMAPTEXT_FUNCS(lGal18V10AllocFuseMapText, lGal18V10FreeFuseMapText)
            DEVICE_PAL_SAMPLEFUSEMAPTEXT_FUNCS(lGal18V10AllocSampleFuseMapText, lGal18V10FreeSampleFuseMapText)
            DEVICE_PAL_READ_FUNC(lGal18V10ReadDevice)
        DEVICE_PAL_DATA_END
    DEVICE_DATA_END
DEVICES_END

#include <Devices/Includes/UtPalFuncs.inl>

static BOOL UTPEPDEVICESAPI lGenericInit(VOID)
{
    return UtPALDeviceInit();
}

static BOOL UTPEPDEVICESAPI lGenericUninit(VOID)
{
    return UtPALDeviceUninit();
}

static ULONG UTPEPDEVICESAPI lGal18V10GetFuseMapSize(VOID)
{
    return 3540;
}

static VOID UTPEPDEVICESAPI lGal18V10ConfigFuseMap(
  LPBYTE pbyData)
{
    typedef struct tagTOutputLogicMacrocell
    {
        UINT nPin;
        ULONG nFuseS0; /* Output polarity */
        ULONG nFuseS1; /* 0 - Registered or 1 - combinatorial */
    } TOutputLogicMacrocell;

    static TOutputLogicMacrocell OutputLogicMacrocells[] = {
        {9,  3474, 3475},
        {11, 3472, 3473},
        {12, 3470, 3471},
        {13, 3468, 3469},
        {14, 3466, 3467},
        {15, 3464, 3465},
        {16, 3462, 3463},
        {17, 3460, 3461},
        {18, 3458, 3459},
        {19, 3456, 3457}};
    ULONG nIndex;
    BYTE byFuseS0;
    UINT nPin24;

    for (nIndex = 0; nIndex < MArrayLen(OutputLogicMacrocells); ++nIndex)
    {
        nPin24 = MPALDevice20PinTo24Pin(OutputLogicMacrocells[nIndex].nPin);

        switch (UtPALDeviceGetDevicePinPolarity(nPin24))
        {
            case epdppActiveLow:
                byFuseS0 = CPALFuseIntact;
                break;
            case epdppActiveHigh:
                byFuseS0 = CPALFuseBlown;
                break;
            default:
                assert(0);

                byFuseS0 = CPALFuseIntact;
                break;
        }

        switch (UtPALDeviceGetDevicePinType(nPin24))
        {
            case epdptOutput:
                pbyData[OutputLogicMacrocells[nIndex].nFuseS0] = byFuseS0;
                pbyData[OutputLogicMacrocells[nIndex].nFuseS1] = CPALFuseBlown;
                break;
            case epdptRegistered:
                pbyData[OutputLogicMacrocells[nIndex].nFuseS0] = byFuseS0;
                pbyData[OutputLogicMacrocells[nIndex].nFuseS1] = CPALFuseIntact;
                break;
            case epdptInput:
                pbyData[OutputLogicMacrocells[nIndex].nFuseS0] = CPALFuseIntact;
                pbyData[OutputLogicMacrocells[nIndex].nFuseS1] = CPALFuseBlown;
                break;
            default:
                assert(0);
                break;
        }
    }
}

static BOOL UTPEPDEVICESAPI lPal10L8SetDevicePinConfigs(
  const TDevicePinConfig* pDevicePinConfigs,
  UINT nTotalDevicePinConfigs)
{
    return UtPALDeviceSetDevicePinConfigs(lFindPALData(L"PAL10L8"),
                                          pDevicePinConfigs,
                                          nTotalDevicePinConfigs, FALSE);
}

static BOOL UTPEPDEVICESAPI lPal12L6SetDevicePinConfigs(
  const TDevicePinConfig* pDevicePinConfigs,
  UINT nTotalDevicePinConfigs)
{
    return UtPALDeviceSetDevicePinConfigs(lFindPALData(L"PAL12L6"),
                                          pDevicePinConfigs,
                                          nTotalDevicePinConfigs, FALSE);
}

static BOOL UTPEPDEVICESAPI lPal12H6SetDevicePinConfigs(
  const TDevicePinConfig* pDevicePinConfigs,
  UINT nTotalDevicePinConfigs)
{
    return UtPALDeviceSetDevicePinConfigs(lFindPALData(L"PAL12H6"),
                                          pDevicePinConfigs,
                                          nTotalDevicePinConfigs, FALSE);
}

static BOOL UTPEPDEVICESAPI lPal16L2SetDevicePinConfigs(
  const TDevicePinConfig* pDevicePinConfigs,
  UINT nTotalDevicePinConfigs)
{
    return UtPALDeviceSetDevicePinConfigs(lFindPALData(L"PAL16L2"),
                                          pDevicePinConfigs,
                                          nTotalDevicePinConfigs, FALSE);
}

static BOOL UTPEPDEVICESAPI lPal16L8SetDevicePinConfigs(
  const TDevicePinConfig* pDevicePinConfigs,
  UINT nTotalDevicePinConfigs)
{
    return UtPALDeviceSetDevicePinConfigs(lFindPALData(L"PAL16L8"),
                                          pDevicePinConfigs,
                                          nTotalDevicePinConfigs, FALSE);
}

static BOOL UTPEPDEVICESAPI lPal16R4SetDevicePinConfigs(
  const TDevicePinConfig* pDevicePinConfigs,
  UINT nTotalDevicePinConfigs)
{
    return UtPALDeviceSetDevicePinConfigs(lFindPALData(L"PAL16R4"),
                                          pDevicePinConfigs,
                                          nTotalDevicePinConfigs, FALSE);
}

static BOOL UTPEPDEVICESAPI lPal16R6SetDevicePinConfigs(
  const TDevicePinConfig* pDevicePinConfigs,
  UINT nTotalDevicePinConfigs)
{
    return UtPALDeviceSetDevicePinConfigs(lFindPALData(L"PAL16R6"),
                                          pDevicePinConfigs,
                                          nTotalDevicePinConfigs, FALSE);
}

static BOOL UTPEPDEVICESAPI lPal16R8SetDevicePinConfigs(
  const TDevicePinConfig* pDevicePinConfigs,
  UINT nTotalDevicePinConfigs)
{
    return UtPALDeviceSetDevicePinConfigs(lFindPALData(L"PAL16R8"),
                                          pDevicePinConfigs,
                                          nTotalDevicePinConfigs, FALSE);
}

static BOOL UTPEPDEVICESAPI lGal18V10SetDevicePinConfigs(
  const TDevicePinConfig* pDevicePinConfigs,
  UINT nTotalDevicePinConfigs)
{
    return UtPALDeviceSetDevicePinConfigs(lFindPALData(L"GAL18V10"),
                                          pDevicePinConfigs,
                                          nTotalDevicePinConfigs, FALSE);
}

static LPCWSTR UTPEPDEVICESAPI lGal18V10AllocFuseMapText(
  const LPBYTE pbyData)
{
    static WCHAR cLineEndData[] = {L'*', 0x0d, 0x0a, 0x00};
    WCHAR cLineNumFormat[20];
    ULONG nFuseMapTextLen, nIndex, nIndex2, nFuseRow;
    LPWSTR pszFuseMapText;
    LPBYTE pbyDataPos;

    if (FALSE == UtInitHeap())
    {
        return NULL;
    }

    nFuseMapTextLen = (9 * 8) * 45; /* 9 rows to 8 OLMC's and 45 characters per row */
    nFuseMapTextLen += ((2 * 11) * 45); /* 11 rows to 2 OLMC's and 45 characters per row */
    nFuseMapTextLen += 45; /* asynchronous reset and 45 characters per row */
    nFuseMapTextLen += 45; /* synchronous preset and 45 characters per row */
    nFuseMapTextLen += 28; /* OLMC mode characters */
    nFuseMapTextLen += 75; /* electronic signature characters */

    pszFuseMapText = (LPWSTR)UtAllocMem(nFuseMapTextLen * sizeof(WCHAR));

    *pszFuseMapText = 0;

    pbyDataPos = pbyData;

    nFuseRow = 0;

    for (nIndex = 0; nIndex < 96; ++nIndex)
    {
        if (SUCCEEDED(StringCchPrintfW(cLineNumFormat, MArrayLen(cLineNumFormat),
                                       L"L%04d ", nFuseRow)))
        {
            StringCchCatW(pszFuseMapText, nFuseMapTextLen, cLineNumFormat);
        }

        for (nIndex2 = 0; nIndex2 < 36; ++nIndex2)
        {
            if (*pbyDataPos == CPALFuseIntact)
            {
                StringCchCatW(pszFuseMapText, nFuseMapTextLen, CPALFuseIntactStr);
            }
            else if (*pbyDataPos == CPALFuseBlown)
            {
                StringCchCatW(pszFuseMapText, nFuseMapTextLen, CPALFuseBlownStr);
            }
            else
            {
                assert(0);
            }

            ++pbyDataPos;
            ++nFuseRow;
        }

        StringCchCatW(pszFuseMapText, nFuseMapTextLen, cLineEndData);
    }

    /* Output Logic Macrocell mode fuses */

    StringCchCatW(pszFuseMapText, nFuseMapTextLen, L"L3456 ");

    for (nIndex = 0; nIndex < 20; ++nIndex)
    {
        if (*pbyDataPos == CPALFuseIntact)
        {
            StringCchCatW(pszFuseMapText, nFuseMapTextLen, CPALFuseIntactStr);
        }
        else if (*pbyDataPos == CPALFuseBlown)
        {
            StringCchCatW(pszFuseMapText, nFuseMapTextLen, CPALFuseBlownStr);
        }
        else
        {
            assert(0);
        }

        ++pbyDataPos;
    }

    StringCchCatW(pszFuseMapText, nFuseMapTextLen, cLineEndData);

    /* Electronic signature character fuses */

    StringCchCatW(pszFuseMapText, nFuseMapTextLen, L"L3476 ");

    for (nIndex = 0; nIndex < 64; ++nIndex)
    {
        if (*pbyDataPos == CPALFuseIntact)
        {
            StringCchCatW(pszFuseMapText, nFuseMapTextLen, CPALFuseIntactStr);
        }
        else if (*pbyDataPos == CPALFuseBlown)
        {
            StringCchCatW(pszFuseMapText, nFuseMapTextLen, CPALFuseBlownStr);
        }
        else
        {
            assert(0);
        }

        ++pbyDataPos;
    }

    StringCchCatW(pszFuseMapText, nFuseMapTextLen, cLineEndData);

    return pszFuseMapText;
}

static VOID UTPEPDEVICESAPI lGal18V10FreeFuseMapText(
  LPCWSTR pszFuseMapText)
{
    if (pszFuseMapText)
    {
        UtFreeMem((LPWSTR)pszFuseMapText);
    }

    UtUninitHeap();
}

static LPCWSTR UTPEPDEVICESAPI lGal18V10AllocSampleFuseMapText(VOID)
{
    WCHAR cLineNumFormat[20];
    ULONG nSampleFuseMapTextLen, nIndex, nFuseRow;
    LPWSTR pszSampleFuseMapText;

    if (FALSE == UtInitHeap())
    {
        return NULL;
    }

    nSampleFuseMapTextLen = (9 * 8) * 45; /* 9 rows to 8 OLMC's and 45 characters per row */
    nSampleFuseMapTextLen += ((2 * 11) * 45); /* 11 rows to 2 OLMC's and 45 characters per row */
    nSampleFuseMapTextLen += 45; /* asynchronous reset and 45 characters per row */
    nSampleFuseMapTextLen += 45; /* synchronous preset and 45 characters per row */
    nSampleFuseMapTextLen += 28; /* OLMC mode characters */
    nSampleFuseMapTextLen += 75; /* electronic signature characters */

    pszSampleFuseMapText = (LPWSTR)UtAllocMem(nSampleFuseMapTextLen * sizeof(WCHAR));

    *pszSampleFuseMapText = 0;

    nFuseRow = 0;

    for (nIndex = 0; nIndex < 96; ++nIndex)
    {
        if (SUCCEEDED(StringCchPrintfW(cLineNumFormat, MArrayLen(cLineNumFormat),
                                       L"L%04d", nFuseRow)))
        {
            StringCchCatW(pszSampleFuseMapText, nSampleFuseMapTextLen,
                          cLineNumFormat);

            StringCchCatW(pszSampleFuseMapText, nSampleFuseMapTextLen,
                          L" xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx*\r\n");
        }

        nFuseRow += 36;
    }

    StringCchCatW(pszSampleFuseMapText, nSampleFuseMapTextLen,
                  L"L3456 xxxxxxxxxxxxxxxxxxxx*\r\n");

    StringCchCatW(pszSampleFuseMapText, nSampleFuseMapTextLen,
                  L"L3476 xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx*\r\n");

    return pszSampleFuseMapText;
}

static VOID UTPEPDEVICESAPI lGal18V10FreeSampleFuseMapText(
  LPCWSTR pszFuseMapText)
{
    if (pszFuseMapText)
    {
        UtFreeMem((LPWSTR)pszFuseMapText);
    }

    UtUninitHeap();
}

static VOID UTPEPDEVICESAPI lPal10L8ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    UtPALDeviceCombinatorialRead(pDeviceIOFuncs, lFindPALData(L"PAL10L8"),
                                 pbyData, ulDataLen, FALSE);
}

static VOID UTPEPDEVICESAPI lPal12L6ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    UtPALDeviceCombinatorialRead(pDeviceIOFuncs, lFindPALData(L"PAL12L6"),
                                 pbyData, ulDataLen, FALSE);
}

static VOID UTPEPDEVICESAPI lPal12H6ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    UtPALDeviceCombinatorialRead(pDeviceIOFuncs, lFindPALData(L"PAL12H6"),
                                 pbyData, ulDataLen, FALSE);
}

static VOID UTPEPDEVICESAPI lPal16L2ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    UtPALDeviceCombinatorialRead(pDeviceIOFuncs, lFindPALData(L"PAL16L2"),
                                 pbyData, ulDataLen, FALSE);
}

static VOID UTPEPDEVICESAPI lPal16L8ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    UtPALDeviceCombinatorialRead(pDeviceIOFuncs, lFindPALData(L"PAL16L8"),
                                 pbyData, ulDataLen, FALSE);
}

static VOID UTPEPDEVICESAPI lPal16R4ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    UtPALDeviceRegisteredRead(pDeviceIOFuncs, lFindPALData(L"PAL16R4"),
                              pbyData, ulDataLen, FALSE);
}

static VOID UTPEPDEVICESAPI lPal16R6ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    UtPALDeviceRegisteredRead(pDeviceIOFuncs, lFindPALData(L"PAL16R6"),
                              pbyData, ulDataLen, FALSE);
}

static VOID UTPEPDEVICESAPI lPal16R8ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    UtPALDeviceRegisteredRead(pDeviceIOFuncs, lFindPALData(L"PAL16R8"),
                              pbyData, ulDataLen, FALSE);
}

static VOID UTPEPDEVICESAPI lGal18V10ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    const TPALData* pPALData = lFindPALData(L"GAL18V10");

    if (0 == UtPALDeviceGetTotalPinTypes(epdptRegistered))
    {
        UtPALDeviceCombinatorialRead(pDeviceIOFuncs, pPALData,
                                     pbyData, ulDataLen, FALSE);
    }
    else
    {
        UtPALDeviceRegisteredRead(pDeviceIOFuncs, pPALData,
                                  pbyData, ulDataLen, FALSE);
    }
}

/***************************************************************************/
/*  Copyright (C) 2007-2020 Kevin Eshbach                                  */
/***************************************************************************/
