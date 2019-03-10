
/*  Copyright (C) 2007-2016 Kevin Eshbach                                  */
/***************************************************************************/

#include <windows.h>
#include <assert.h>

#include <UtilsDevice/UtPepDevices.h>
#include <UtilsDevice/UtPalDevices.h>
#include <UtilsDevice/UtPALDefs.h>

#include <Devices/Includes/UtDevicePinDefs.h>

#include <Devices/Includes/UtPalDeviceAdapterDefs.h>

#include <Config/UtPepCtrl.h>

#include <Includes/UtMacros.h>

static BOOL UTPEPDEVICESAPI lGenericInit(VOID);
static BOOL UTPEPDEVICESAPI lGenericUninit(VOID);

static BOOL UTPEPDEVICESAPI lPal6L16SetDevicePinConfigs(const TDevicePinConfig* pDevicePinConfigs, UINT nTotalDevicePinConfigs);
static BOOL UTPEPDEVICESAPI lPal8L14SetDevicePinConfigs(const TDevicePinConfig* pDevicePinConfigs, UINT nTotalDevicePinConfigs);
static BOOL UTPEPDEVICESAPI lPal20L8SetDevicePinConfigs(const TDevicePinConfig* pDevicePinConfigs, UINT nTotalDevicePinConfigs);
static BOOL UTPEPDEVICESAPI lPal20L10SetDevicePinConfigs(const TDevicePinConfig* pDevicePinConfigs, UINT nTotalDevicePinConfigs);
static BOOL UTPEPDEVICESAPI lPal20R4SetDevicePinConfigs(const TDevicePinConfig* pDevicePinConfigs, UINT nTotalDevicePinConfigs);
static BOOL UTPEPDEVICESAPI lPal20R6SetDevicePinConfigs(const TDevicePinConfig* pDevicePinConfigs, UINT nTotalDevicePinConfigs);
static BOOL UTPEPDEVICESAPI lPal20R8SetDevicePinConfigs(const TDevicePinConfig* pDevicePinConfigs, UINT nTotalDevicePinConfigs);
static BOOL UTPEPDEVICESAPI lPal20X4SetDevicePinConfigs(const TDevicePinConfig* pDevicePinConfigs, UINT nTotalDevicePinConfigs);
static BOOL UTPEPDEVICESAPI lPal20X8SetDevicePinConfigs(const TDevicePinConfig* pDevicePinConfigs, UINT nTotalDevicePinConfigs);
static BOOL UTPEPDEVICESAPI lPal20X10SetDevicePinConfigs(const TDevicePinConfig* pDevicePinConfigs, UINT nTotalDevicePinConfigs);

static VOID UTPEPDEVICESAPI lPal6L16ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI lPal8L14ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI lPal20L8ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI lPal20L10ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI lPal20R4ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI lPal20R6ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI lPal20R8ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI lPal20X4ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI lPal20X8ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, LPBYTE pbyData, ULONG ulDataLen);
static VOID UTPEPDEVICESAPI lPal20X10ReadDevice(const TDeviceIOFuncs* pDeviceIOFuncs, LPBYTE pbyData, ULONG ulDataLen);

static LPCWSTR l_pszPal6L16PinNames[] = {
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Ground,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Vcc};

static LPCWSTR l_pszPal8L14PinNames[] = {
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Input,
    CDevicePin_Output,
    CDevicePin_Ground,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Vcc};

static LPCWSTR l_pszPal20L8PinNames[] = {
    CDevicePin_Input,
    CDevicePin_Input,
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
    CDevicePin_InputOutput,
    CDevicePin_InputOutput,
    CDevicePin_InputOutput,
    CDevicePin_InputOutput,
    CDevicePin_InputOutput,
    CDevicePin_InputOutput,
    CDevicePin_Output,
    CDevicePin_Input,
    CDevicePin_Vcc};

static LPCWSTR l_pszPal20L10PinNames[] = {
    CDevicePin_Input,
    CDevicePin_Input,
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
    CDevicePin_InputOutput,
    CDevicePin_InputOutput,
    CDevicePin_Output,
    CDevicePin_Vcc};

static LPCWSTR l_pszPal20R4PinNames[] = {
    CDevicePin_Clock,
    CDevicePin_Input,
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
    CDevicePin_Input,
    CDevicePin_InputOutput,
    CDevicePin_InputOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_InputOutput,
    CDevicePin_InputOutput,
    CDevicePin_Input,
    CDevicePin_Vcc};

static LPCWSTR l_pszPal20R6PinNames[] = {
    CDevicePin_Clock,
    CDevicePin_Input,
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
    CDevicePin_Input,
    CDevicePin_InputOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_InputOutput,
    CDevicePin_Input,
    CDevicePin_Vcc};

static LPCWSTR l_pszPal20R8PinNames[] = {
    CDevicePin_Clock,
    CDevicePin_Input,
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
    CDevicePin_Input,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_Input,
    CDevicePin_Vcc};

static LPCWSTR l_pszPal20X4PinNames[] = {
    CDevicePin_Clock,
    CDevicePin_Input,
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
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Output,
    CDevicePin_Vcc};

static LPCWSTR l_pszPal20X8PinNames[] = {
    CDevicePin_Clock,
    CDevicePin_Input,
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
    CDevicePin_Output,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_Output,
    CDevicePin_Vcc};

static LPCWSTR l_pszPal20X10PinNames[] = {
    CDevicePin_Clock,
    CDevicePin_Input,
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
    CDevicePin_RegisteredOutput,
    CDevicePin_RegisteredOutput,
    CDevicePin_Vcc};

static TDevicePinFuseRows l_Pal6L16PinFuseRows[] = {
    { 1, CNoOutputEnableFuseRow, 0, 0 },
    { 2, CNoOutputEnableFuseRow, 24, 24 },
    { 3, CNoOutputEnableFuseRow, 36, 36 },
    { 10, CNoOutputEnableFuseRow, 132, 132 },
    { 11, CNoOutputEnableFuseRow, 168, 168 },
    { 13, CNoOutputEnableFuseRow, 180, 180 },
    { 14, CNoOutputEnableFuseRow, 156, 156 },
    { 15, CNoOutputEnableFuseRow, 144, 144 },
    { 16, CNoOutputEnableFuseRow, 120, 120 },
    { 17, CNoOutputEnableFuseRow, 108, 108 },
    { 18, CNoOutputEnableFuseRow, 96, 96 },
    { 19, CNoOutputEnableFuseRow, 84, 84 },
    { 20, CNoOutputEnableFuseRow, 72, 72 },
    { 21, CNoOutputEnableFuseRow, 60, 60 },
    { 22, CNoOutputEnableFuseRow, 48, 48 },
    { 23, CNoOutputEnableFuseRow, 12, 12 } };

static TDevicePinFuseRows l_Pal8L14PinFuseRows[] = {
    { 1, CNoOutputEnableFuseRow, 0, 0 },
    { 2, CNoOutputEnableFuseRow, 32, 32 },
    { 11, CNoOutputEnableFuseRow, 192, 192 },
    { 13, CNoOutputEnableFuseRow, 208, 208 },
    { 14, CNoOutputEnableFuseRow, 176, 176 },
    { 15, CNoOutputEnableFuseRow, 160, 160 },
    { 16, CNoOutputEnableFuseRow, 144, 144 },
    { 17, CNoOutputEnableFuseRow, 128, 128 },
    { 18, CNoOutputEnableFuseRow, 112, 112 },
    { 19, CNoOutputEnableFuseRow, 96, 96 },
    { 20, CNoOutputEnableFuseRow, 80, 80 },
    { 21, CNoOutputEnableFuseRow, 64, 64 },
    { 22, CNoOutputEnableFuseRow, 48, 48 },
    { 23, CNoOutputEnableFuseRow, 16, 16 } };

static TDevicePinFuseRows l_Pal20L8PinFuseRows[] = {
    {15, 2240, 2280, 2520},
    {16, 1920, 1960, 2200},
    {17, 1600, 1640, 1880},
    {18, 1280, 1320, 1560},
    {19, 960, 1000, 1240},
    {20, 640, 680, 920},
    {21, 320, 360, 600},
    {22, 0, 40, 280}};

static TDevicePinFuseRows l_Pal20L10PinFuseRows[] = {
    {14, 1440, 1480, 1560},
    {15, 1280, 1320, 1400},
    {16, 1120, 1160, 1240},
    {17, 960,  1000, 1080},
    {18, 800,  840,  920},
    {19, 640,  680,  760},
    {20, 480,  520,  600},
    {21, 320,  360,  440},
    {22, 160,  200,  280},
    {23, 0,    40,   120}};

static TDevicePinFuseRows l_Pal20R4PinFuseRows[] = {
    {15, 2240, 2280, 2520},
    {16, 1920, 1960, 2200},
    {17, CNoOutputEnableFuseRow, 1600, 1880}, /* Registered Output */
    {18, CNoOutputEnableFuseRow, 1280, 1560}, /* Registered Output */
    {19, CNoOutputEnableFuseRow, 960, 1240},  /* Registered Output */
    {20, CNoOutputEnableFuseRow, 640, 920},   /* Registered Output */
    {21, 320, 360, 600},
    {22, 0, 40, 280}};

static TDevicePinFuseRows l_Pal20R6PinFuseRows[] = {
    {15, 2240, 2280, 2520},
    {16, CNoOutputEnableFuseRow, 1920, 2200}, /* Registered Output */
    {17, CNoOutputEnableFuseRow, 1600, 1880}, /* Registered Output */
    {18, CNoOutputEnableFuseRow, 1280, 1560}, /* Registered Output */
    {19, CNoOutputEnableFuseRow, 960, 1240},  /* Registered Output */
    {20, CNoOutputEnableFuseRow, 640, 920},   /* Registered Output */
    {21, CNoOutputEnableFuseRow, 320, 600},   /* Registered Output */
    {22, 0, 40, 280}};

static TDevicePinFuseRows l_Pal20R8PinFuseRows[] = {
    {15, CNoOutputEnableFuseRow, 2240, 2520}, /* Registered Output */
    {16, CNoOutputEnableFuseRow, 1920, 2200}, /* Registered Output */
    {17, CNoOutputEnableFuseRow, 1600, 1880}, /* Registered Output */
    {18, CNoOutputEnableFuseRow, 1280, 1560}, /* Registered Output */
    {19, CNoOutputEnableFuseRow, 960, 1240},  /* Registered Output */
    {20, CNoOutputEnableFuseRow, 640, 920},   /* Registered Output */
    {21, CNoOutputEnableFuseRow, 320, 600},   /* Registered Output */
    {22, CNoOutputEnableFuseRow, 0, 280}};    /* Registered Output */

static TDevicePinFuseRows l_Pal20X4PinFuseRows[] = {
    {14, 1440, 1480, 1560},
    {15, 1280, 1320, 1400},
    {16, 1120, 1160, 1240},
    {17, CNoOutputEnableFuseRow, 960, 1080},  /* Registered Output */
    {18, CNoOutputEnableFuseRow, 800, 920},   /* Registered Output */
    {19, CNoOutputEnableFuseRow, 640, 760},   /* Registered Output */
    {20, CNoOutputEnableFuseRow, 480, 600},   /* Registered Output */
    {21, 320, 360, 440},
    {22, 160, 200, 280},
    {23, 0, 40, 120}};

static TDevicePinFuseRows l_Pal20X8PinFuseRows[] = {
    {14, 1440, 1480, 1560},
    {15, CNoOutputEnableFuseRow, 1280, 1400}, /* Registered Output */
    {16, CNoOutputEnableFuseRow, 1120, 1240}, /* Registered Output */
    {17, CNoOutputEnableFuseRow, 960, 1080},  /* Registered Output */
    {18, CNoOutputEnableFuseRow, 800, 920},   /* Registered Output */
    {19, CNoOutputEnableFuseRow, 640, 760},   /* Registered Output */
    {20, CNoOutputEnableFuseRow, 480, 600},   /* Registered Output */
    {21, CNoOutputEnableFuseRow, 320, 440},   /* Registered Output */
    {22, CNoOutputEnableFuseRow, 160, 280},   /* Registered Output */
    {23, 0, 40, 120}};

static TDevicePinFuseRows l_Pal20X10PinFuseRows[] = {
    {14, CNoOutputEnableFuseRow, 1440, 1560}, /* Registered Output */
    {15, CNoOutputEnableFuseRow, 1280, 1400}, /* Registered Output */
    {16, CNoOutputEnableFuseRow, 1120, 1240}, /* Registered Output */
    {17, CNoOutputEnableFuseRow, 960, 1080},  /* Registered Output */
    {18, CNoOutputEnableFuseRow, 800, 920},   /* Registered Output */
    {19, CNoOutputEnableFuseRow, 640, 760},   /* Registered Output */
    {20, CNoOutputEnableFuseRow, 480, 600},   /* Registered Output */
    {21, CNoOutputEnableFuseRow, 320, 440},   /* Registered Output */
    {22, CNoOutputEnableFuseRow, 160, 280},   /* Registered Output */
    {23, CNoOutputEnableFuseRow, 0, 120}};    /* Registered Output */

static TDevicePinFuseColumns l_Pal6L16PinFuseColumns[] = {
    {4, 1, 0},
    {5, 3, 2},
    {6, 5, 4},
    {7, 7, 6},
    {8, 9, 8},
    {9, 11, 10}};

static TDevicePinFuseColumns l_Pal8L14PinFuseColumns[] = {
    {3, 1, 0},
    {4, 3, 2},
    {5, 5, 4},
    {6, 7, 6},
    {7, 9, 8},
    {8, 11, 10},
    {9, 13, 12},
    {10, 15, 14}};

static TDevicePinFuseColumns l_Pal20L8PinFuseColumns[] = {
    {1, 3, 2},
    {2, 1, 0},
    {3, 5, 4},
    {4, 9, 8},
    {5, 13, 12},
    {6, 17, 16},
    {7, 21, 20},
    {8, 25, 24},
    {9, 29, 28},
    {10, 33, 32},
    {11, 37, 36},
    {13, 39, 38},
    {14, 35, 34},
    {16, 31, 30},
    {17, 27, 26},
    {18, 23, 22},
    {19, 19, 18},
    {20, 15, 14},
    {21, 11, 10},
    {23, 7, 6}};

static TDevicePinFuseColumns l_Pal20L10PinFuseColumns[] = {
    {1, 3, 2},
    {2, 1, 0},
    {3, 5, 4},
    {4, 9, 8},
    {5, 13, 12},
    {6, 17, 16},
    {7, 21, 20},
    {8, 25, 24},
    {9, 29, 28},
    {10, 33, 32},
    {11, 37, 36},
    {13, 39, 38},
    {15, 35, 34},
    {16, 31, 30},
    {17, 27, 26},
    {18, 23, 22},
    {19, 19, 18},
    {20, 15, 14},
    {21, 11, 10},
    {22, 7, 6}};

static TDevicePinFuseColumns l_Pal20R4PinFuseColumns[] = {
    {2, 1, 0},
    {3, 5, 4},
    {4, 9, 8},
    {5, 13, 12},
    {6, 17, 16},
    {7, 21, 20},
    {8, 25, 24},
    {9, 29, 28},
    {10, 33, 32},
    {11, 37, 36},
    {14, 39, 38},
    {15, 35, 34},
    {16, 31, 30},
    {17, 27, 26},
    {18, 23, 22},
    {19, 19, 18},
    {20, 15, 14},
    {21, 11, 10},
    {22, 7, 6},
    {23, 3, 2}};

static TDevicePinFuseColumns l_Pal20R6PinFuseColumns[] = {
    {2, 1, 0},
    {3, 5, 4},
    {4, 9, 8},
    {5, 13, 12},
    {6, 17, 16},
    {7, 21, 20},
    {8, 25, 24},
    {9, 29, 28},
    {10, 33, 32},
    {11, 37, 36},
    {14, 39, 38},
    {15, 35, 34},
    {16, 31, 30},
    {17, 27, 26},
    {18, 23, 22},
    {19, 19, 18},
    {20, 15, 14},
    {21, 11, 10},
    {22, 7, 6},
    {23, 3, 2}};

static TDevicePinFuseColumns l_Pal20R8PinFuseColumns[] = {
    {2, 1, 0},
    {3, 5, 4},
    {4, 9, 8},
    {5, 13, 12},
    {6, 17, 16},
    {7, 21, 20},
    {8, 25, 24},
    {9, 29, 28},
    {10, 33, 32},
    {11, 37, 36},
    {14, 39, 38},
    {15, 35, 34},
    {16, 31, 30},
    {17, 27, 26},
    {18, 23, 22},
    {19, 19, 18},
    {20, 15, 14},
    {21, 11, 10},
    {22, 7, 6},
    {23, 3, 2}};

static TDevicePinFuseColumns l_Pal20X4PinFuseColumns[] = {
    {2, 1, 0},
    {3, 5, 4},
    {4, 9, 8},
    {5, 13, 12},
    {6, 17, 16},
    {7, 21, 20},
    {8, 25, 24},
    {9, 29, 28},
    {10, 33, 32},
    {11, 37, 36},
    {14, 39, 38},
    {15, 35, 34},
    {16, 31, 30},
    {17, 27, 26},
    {18, 23, 22},
    {19, 19, 18},
    {20, 15, 14},
    {21, 11, 10},
    {22, 7, 6},
    {23, 3, 2}};

static TDevicePinFuseColumns l_Pal20X8PinFuseColumns[] = {
    {2, 1, 0},
    {3, 5, 4},
    {4, 9, 8},
    {5, 13, 12},
    {6, 17, 16},
    {7, 21, 20},
    {8, 25, 24},
    {9, 29, 28},
    {10, 33, 32},
    {11, 37, 36},
    {14, 39, 38},
    {15, 35, 34},
    {16, 31, 30},
    {17, 27, 26},
    {18, 23, 22},
    {19, 19, 18},
    {20, 15, 14},
    {21, 11, 10},
    {22, 7, 6},
    {23, 3, 2}};

static TDevicePinFuseColumns l_Pal20X10PinFuseColumns[] = {
    {2, 1, 0},
    {3, 5, 4},
    {4, 9, 8},
    {5, 13, 12},
    {6, 17, 16},
    {7, 21, 20},
    {8, 25, 24},
    {9, 29, 28},
    {10, 33, 32},
    {11, 37, 36},
    {14, 39, 38},
    {15, 35, 34},
    {16, 31, 30},
    {17, 27, 26},
    {18, 23, 22},
    {19, 19, 18},
    {20, 15, 14},
    {21, 11, 10},
    {22, 7, 6},
    {23, 3, 2}};

static TDevicePinConfigValues l_Pal6L16PinConfigValues[] = {
    MDevicePinConfigValues(1, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(2, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(3, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValuesNoPolarity(4, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(5, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(6, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(7, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(8, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(9, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValues(10, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(11, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(13, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(14, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(15, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(16, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(17, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(18, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(19, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(20, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(21, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(22, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(23, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues)
};

static TDevicePinConfigValues l_Pal8L14PinConfigValues[] = {
    MDevicePinConfigValues(1, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(2, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValuesNoPolarity(3, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(4, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(5, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(6, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(7, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(8, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(9, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(10, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValues(11, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(13, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(14, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(15, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(16, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(17, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(18, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(19, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(20, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(21, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(22, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(23, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues)
};

static TDevicePinConfigValues l_Pal20L8PinConfigValues[] = {
    MDevicePinConfigValuesNoPolarity(1, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(2, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(3, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(4, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(5, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(6, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(7, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(8, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(9, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(10, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(11, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(13, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(14, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValues(15, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(16, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(17, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(18, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(19, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(20, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(21, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(22, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValuesNoPolarity(23, l_pszI_NU_PinTypeValues)
};

static TDevicePinConfigValues l_Pal20L10PinConfigValues[] = {
    MDevicePinConfigValuesNoPolarity(1, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(2, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(3, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(4, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(5, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(6, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(7, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(8, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(9, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(10, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(11, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(13, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValues(14, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(15, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(16, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(17, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(18, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(19, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(20, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(21, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(22, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(23, l_pszO_NU_PinTypeValues, l_pszL_PinPolarityValues)
};

static TDevicePinConfigValues l_Pal20R4PinConfigValues[] = {
    MDevicePinConfigValuesNoPolarity(1, l_pszI_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(2, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(3, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(4, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(5, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(6, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(7, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(8, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(9, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(10, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(11, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(13, l_pszI_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(14, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValues(15, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(16, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(17, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(18, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(19, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(20, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(21, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(22, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValuesNoPolarity(23, l_pszI_NU_PinTypeValues)
};

static TDevicePinConfigValues l_Pal20R6PinConfigValues[] = {
    MDevicePinConfigValuesNoPolarity(1, l_pszI_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(2, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(3, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(4, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(5, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(6, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(7, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(8, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(9, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(10, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(11, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(13, l_pszI_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(14, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValues(15, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(16, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(17, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(18, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(19, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(20, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(21, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(22, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValuesNoPolarity(23, l_pszI_NU_PinTypeValues)
};

static TDevicePinConfigValues l_Pal20R8PinConfigValues[] = {
    MDevicePinConfigValuesNoPolarity(1, l_pszI_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(2, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(3, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(4, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(5, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(6, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(7, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(8, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(9, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(10, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(11, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(13, l_pszI_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(14, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValues(15, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(16, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(17, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(18, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(19, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(20, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(21, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(22, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValuesNoPolarity(23, l_pszI_NU_PinTypeValues)
};

static TDevicePinConfigValues l_Pal20X4PinConfigValues[] = {
    MDevicePinConfigValuesNoPolarity(1, l_pszI_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(2, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(3, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(4, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(5, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(6, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(7, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(8, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(9, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(10, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(11, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(13, l_pszI_PinTypeValues),
    MDevicePinConfigValues(14, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(15, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(16, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(17, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(18, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(19, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(20, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(21, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(22, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(23, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues)
};

static TDevicePinConfigValues l_Pal20X8PinConfigValues[] = {
    MDevicePinConfigValuesNoPolarity(1, l_pszI_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(2, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(3, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(4, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(5, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(6, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(7, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(8, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(9, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(10, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(11, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(13, l_pszI_PinTypeValues),
    MDevicePinConfigValues(14, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(15, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(16, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(17, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(18, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(19, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(20, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(21, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(22, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(23, l_pszI_O_PinTypeValues, l_pszL_PinPolarityValues)
};

static TDevicePinConfigValues l_Pal20X10PinConfigValues[] = {
    MDevicePinConfigValuesNoPolarity(1, l_pszI_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(2, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(3, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(4, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(5, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(6, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(7, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(8, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(9, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(10, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(11, l_pszI_NU_PinTypeValues),
    MDevicePinConfigValuesNoPolarity(13, l_pszI_PinTypeValues),
    MDevicePinConfigValues(14, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(15, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(16, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(17, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(18, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(19, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(20, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(21, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(22, l_pszR_PinTypeValues, l_pszL_PinPolarityValues),
    MDevicePinConfigValues(23, l_pszR_PinTypeValues, l_pszL_PinPolarityValues)
};

/*
    Devices
*/

DEVICES_BEGIN
    DEVICE_DATA_BEGIN(PAL6L16)
        DEVICE_PAL
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_pszPal6L16PinNames)
        DEVICE_ADAPTER("Part# PAL20/24")
        DEVICE_MESSAGE("Set the switch to 24 Pins.")
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
        DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_PAL_DATA_BEGIN
            DEVICE_PAL_PINCONFIGVALUES(l_Pal6L16PinConfigValues)
            DEVICE_PAL_PINFUSEROWS(l_Pal6L16PinFuseRows)
            DEVICE_PAL_PINFUSECOLUMNS(l_Pal6L16PinFuseColumns)
            DEVICE_PAL_GETFUSEMAPSIZE_FUNC_NONE
            DEVICE_PAL_CONFIGFUSEMAP_FUNC_NONE
            DEVICE_PAL_SETDEVICEPINCONFIGS_FUNC(lPal6L16SetDevicePinConfigs)
            DEVICE_PAL_FUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_SAMPLEFUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_READ_FUNC(lPal6L16ReadDevice)
        DEVICE_PAL_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(PAL8L14)
        DEVICE_PAL
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_pszPal8L14PinNames)
        DEVICE_ADAPTER("Part# PAL20/24")
        DEVICE_MESSAGE("Set the switch to 24 Pins.")
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
        DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_PAL_DATA_BEGIN
            DEVICE_PAL_PINCONFIGVALUES(l_Pal8L14PinConfigValues)
            DEVICE_PAL_PINFUSEROWS(l_Pal8L14PinFuseRows)
            DEVICE_PAL_PINFUSECOLUMNS(l_Pal8L14PinFuseColumns)
            DEVICE_PAL_GETFUSEMAPSIZE_FUNC_NONE
            DEVICE_PAL_CONFIGFUSEMAP_FUNC_NONE
            DEVICE_PAL_SETDEVICEPINCONFIGS_FUNC(lPal8L14SetDevicePinConfigs)
            DEVICE_PAL_FUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_SAMPLEFUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_READ_FUNC(lPal8L14ReadDevice)
        DEVICE_PAL_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(PAL20L8)
        DEVICE_PAL
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_pszPal20L8PinNames)
        DEVICE_ADAPTER("Part# PAL20/24")
        DEVICE_MESSAGE("Set the switch to 24 Pins.")
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
        DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_PAL_DATA_BEGIN
            DEVICE_PAL_PINCONFIGVALUES(l_Pal20L8PinConfigValues)
            DEVICE_PAL_PINFUSEROWS(l_Pal20L8PinFuseRows)
            DEVICE_PAL_PINFUSECOLUMNS(l_Pal20L8PinFuseColumns)
            DEVICE_PAL_GETFUSEMAPSIZE_FUNC_NONE
            DEVICE_PAL_CONFIGFUSEMAP_FUNC_NONE
            DEVICE_PAL_SETDEVICEPINCONFIGS_FUNC(lPal20L8SetDevicePinConfigs)
            DEVICE_PAL_FUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_SAMPLEFUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_READ_FUNC(lPal20L8ReadDevice)
        DEVICE_PAL_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(PAL20L10)
        DEVICE_PAL
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_pszPal20L10PinNames)
        DEVICE_ADAPTER("Part# PAL20/24")
        DEVICE_MESSAGE("Set the switch to 24 Pins.")
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
        DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_PAL_DATA_BEGIN
            DEVICE_PAL_PINCONFIGVALUES(l_Pal20L10PinConfigValues)
            DEVICE_PAL_PINFUSEROWS(l_Pal20L10PinFuseRows)
            DEVICE_PAL_PINFUSECOLUMNS(l_Pal20L10PinFuseColumns)
            DEVICE_PAL_GETFUSEMAPSIZE_FUNC_NONE
            DEVICE_PAL_CONFIGFUSEMAP_FUNC_NONE
            DEVICE_PAL_SETDEVICEPINCONFIGS_FUNC(lPal20L10SetDevicePinConfigs)
            DEVICE_PAL_FUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_SAMPLEFUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_READ_FUNC(lPal20L10ReadDevice)
        DEVICE_PAL_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(PAL20R4)
        DEVICE_PAL
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_pszPal20R4PinNames)
        DEVICE_ADAPTER("Part# PAL20/24")
        DEVICE_MESSAGE("Set the switch to 24 Pins.")
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
        DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_PAL_DATA_BEGIN
            DEVICE_PAL_PINCONFIGVALUES(l_Pal20R4PinConfigValues)
            DEVICE_PAL_PINFUSEROWS(l_Pal20R4PinFuseRows)
            DEVICE_PAL_PINFUSECOLUMNS(l_Pal20R4PinFuseColumns)
            DEVICE_PAL_GETFUSEMAPSIZE_FUNC_NONE
            DEVICE_PAL_CONFIGFUSEMAP_FUNC_NONE
            DEVICE_PAL_SETDEVICEPINCONFIGS_FUNC(lPal20R4SetDevicePinConfigs)
            DEVICE_PAL_FUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_SAMPLEFUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_READ_FUNC(lPal20R4ReadDevice)
        DEVICE_PAL_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(PAL20R6)
        DEVICE_PAL
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_pszPal20R6PinNames)
        DEVICE_ADAPTER("Part# PAL20/24")
        DEVICE_MESSAGE("Set the switch to 24 Pins.")
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
        DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_PAL_DATA_BEGIN
            DEVICE_PAL_PINCONFIGVALUES(l_Pal20R6PinConfigValues)
            DEVICE_PAL_PINFUSEROWS(l_Pal20R6PinFuseRows)
            DEVICE_PAL_PINFUSECOLUMNS(l_Pal20R6PinFuseColumns)
            DEVICE_PAL_GETFUSEMAPSIZE_FUNC_NONE
            DEVICE_PAL_CONFIGFUSEMAP_FUNC_NONE
            DEVICE_PAL_SETDEVICEPINCONFIGS_FUNC(lPal20R6SetDevicePinConfigs)
            DEVICE_PAL_FUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_SAMPLEFUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_READ_FUNC(lPal20R6ReadDevice)
        DEVICE_PAL_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(PAL20R8)
        DEVICE_PAL
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_pszPal20R8PinNames)
        DEVICE_ADAPTER("Part# PAL20/24")
        DEVICE_MESSAGE("Set the switch to 24 Pins.")
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
        DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_PAL_DATA_BEGIN
            DEVICE_PAL_PINCONFIGVALUES(l_Pal20R8PinConfigValues)
            DEVICE_PAL_PINFUSEROWS(l_Pal20R8PinFuseRows)
            DEVICE_PAL_PINFUSECOLUMNS(l_Pal20R8PinFuseColumns)
            DEVICE_PAL_GETFUSEMAPSIZE_FUNC_NONE
            DEVICE_PAL_CONFIGFUSEMAP_FUNC_NONE
            DEVICE_PAL_SETDEVICEPINCONFIGS_FUNC(lPal20R8SetDevicePinConfigs)
            DEVICE_PAL_FUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_SAMPLEFUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_READ_FUNC(lPal20R8ReadDevice)
        DEVICE_PAL_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(PAL20X4)
        DEVICE_PAL
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_pszPal20X4PinNames)
        DEVICE_ADAPTER("Part# PAL20/24")
        DEVICE_MESSAGE("Set the switch to 24 Pins.")
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
        DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_PAL_DATA_BEGIN
            DEVICE_PAL_PINCONFIGVALUES(l_Pal20X4PinConfigValues)
            DEVICE_PAL_PINFUSEROWS(l_Pal20X4PinFuseRows)
            DEVICE_PAL_PINFUSECOLUMNS(l_Pal20X4PinFuseColumns)
            DEVICE_PAL_GETFUSEMAPSIZE_FUNC_NONE
            DEVICE_PAL_CONFIGFUSEMAP_FUNC_NONE
            DEVICE_PAL_SETDEVICEPINCONFIGS_FUNC(lPal20X4SetDevicePinConfigs)
            DEVICE_PAL_FUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_SAMPLEFUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_READ_FUNC(lPal20X4ReadDevice)
        DEVICE_PAL_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(PAL20X8)
        DEVICE_PAL
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_pszPal20X8PinNames)
        DEVICE_ADAPTER("Part# PAL20/24")
        DEVICE_MESSAGE("Set the switch to 24 Pins.")
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
        DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_PAL_DATA_BEGIN
            DEVICE_PAL_PINCONFIGVALUES(l_Pal20X8PinConfigValues)
            DEVICE_PAL_PINFUSEROWS(l_Pal20X8PinFuseRows)
            DEVICE_PAL_PINFUSECOLUMNS(l_Pal20X8PinFuseColumns)
            DEVICE_PAL_GETFUSEMAPSIZE_FUNC_NONE
            DEVICE_PAL_CONFIGFUSEMAP_FUNC_NONE
            DEVICE_PAL_SETDEVICEPINCONFIGS_FUNC(lPal20X8SetDevicePinConfigs)
            DEVICE_PAL_FUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_SAMPLEFUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_READ_FUNC(lPal20X8ReadDevice)
        DEVICE_PAL_DATA_END
    DEVICE_DATA_END
    DEVICE_DATA_BEGIN(PAL20X10)
        DEVICE_PAL
        DEVICE_DIP_PACKAGE
        DEVICE_PINCOUNT(24)
        DEVICE_PINNAMES(l_pszPal20X10PinNames)
        DEVICE_ADAPTER("Part# PAL20/24")
        DEVICE_MESSAGE("Set the switch to 24 Pins.")
        DEVICE_DIPSWITCHES(FALSE, FALSE, FALSE, TRUE, FALSE, TRUE, FALSE, FALSE)
        DEVICE_INIT_FUNC(lGenericInit)
        DEVICE_UNINIT_FUNC(lGenericUninit)
        DEVICE_PAL_DATA_BEGIN
            DEVICE_PAL_PINCONFIGVALUES(l_Pal20X10PinConfigValues)
            DEVICE_PAL_PINFUSEROWS(l_Pal20X10PinFuseRows)
            DEVICE_PAL_PINFUSECOLUMNS(l_Pal20X10PinFuseColumns)
            DEVICE_PAL_GETFUSEMAPSIZE_FUNC_NONE
            DEVICE_PAL_CONFIGFUSEMAP_FUNC_NONE
            DEVICE_PAL_SETDEVICEPINCONFIGS_FUNC(lPal20X10SetDevicePinConfigs)
            DEVICE_PAL_FUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_SAMPLEFUSEMAPTEXT_FUNCS_NONE
            DEVICE_PAL_READ_FUNC(lPal20X10ReadDevice)
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

static BOOL UTPEPDEVICESAPI lPal6L16SetDevicePinConfigs(
  const TDevicePinConfig* pDevicePinConfigs,
  UINT nTotalDevicePinConfigs)
{
    return UtPALDeviceSetDevicePinConfigs(lFindPALData(L"PAL6L16"),
                                          pDevicePinConfigs,
                                          nTotalDevicePinConfigs, TRUE);
}

static BOOL UTPEPDEVICESAPI lPal8L14SetDevicePinConfigs(
  const TDevicePinConfig* pDevicePinConfigs,
  UINT nTotalDevicePinConfigs)
{
    return UtPALDeviceSetDevicePinConfigs(lFindPALData(L"PAL8L14"),
                                          pDevicePinConfigs,
                                          nTotalDevicePinConfigs, TRUE);
}

static BOOL UTPEPDEVICESAPI lPal20L8SetDevicePinConfigs(
  const TDevicePinConfig* pDevicePinConfigs,
  UINT nTotalDevicePinConfigs)
{
    return UtPALDeviceSetDevicePinConfigs(lFindPALData(L"PAL20L8"),
                                          pDevicePinConfigs,
                                          nTotalDevicePinConfigs, TRUE);
}

static BOOL UTPEPDEVICESAPI lPal20L10SetDevicePinConfigs(
  const TDevicePinConfig* pDevicePinConfigs,
  UINT nTotalDevicePinConfigs)
{
    return UtPALDeviceSetDevicePinConfigs(lFindPALData(L"PAL20L10"),
                                          pDevicePinConfigs,
                                          nTotalDevicePinConfigs, TRUE);
}

static BOOL UTPEPDEVICESAPI lPal20R4SetDevicePinConfigs(
  const TDevicePinConfig* pDevicePinConfigs,
  UINT nTotalDevicePinConfigs)
{
    return UtPALDeviceSetDevicePinConfigs(lFindPALData(L"PAL20R4"),
                                          pDevicePinConfigs,
                                          nTotalDevicePinConfigs, TRUE);
}

static BOOL UTPEPDEVICESAPI lPal20R6SetDevicePinConfigs(
  const TDevicePinConfig* pDevicePinConfigs,
  UINT nTotalDevicePinConfigs)
{
    return UtPALDeviceSetDevicePinConfigs(lFindPALData(L"PAL20R6"),
                                          pDevicePinConfigs,
                                          nTotalDevicePinConfigs, TRUE);
}

static BOOL UTPEPDEVICESAPI lPal20R8SetDevicePinConfigs(
  const TDevicePinConfig* pDevicePinConfigs,
  UINT nTotalDevicePinConfigs)
{
    return UtPALDeviceSetDevicePinConfigs(lFindPALData(L"PAL20R8"),
                                          pDevicePinConfigs,
                                          nTotalDevicePinConfigs, TRUE);
}

static BOOL UTPEPDEVICESAPI lPal20X4SetDevicePinConfigs(
  const TDevicePinConfig* pDevicePinConfigs,
  UINT nTotalDevicePinConfigs)
{
    return UtPALDeviceSetDevicePinConfigs(lFindPALData(L"PAL20X4"),
                                          pDevicePinConfigs,
                                          nTotalDevicePinConfigs, TRUE);
}

static BOOL UTPEPDEVICESAPI lPal20X8SetDevicePinConfigs(
  const TDevicePinConfig* pDevicePinConfigs,
  UINT nTotalDevicePinConfigs)
{
    return UtPALDeviceSetDevicePinConfigs(lFindPALData(L"PAL20X8"),
                                          pDevicePinConfigs,
                                          nTotalDevicePinConfigs, TRUE);
}

static BOOL UTPEPDEVICESAPI lPal20X10SetDevicePinConfigs(
  const TDevicePinConfig* pDevicePinConfigs,
  UINT nTotalDevicePinConfigs)
{
    return UtPALDeviceSetDevicePinConfigs(lFindPALData(L"PAL20X10"),
                                          pDevicePinConfigs,
                                          nTotalDevicePinConfigs, TRUE);
}

static VOID UTPEPDEVICESAPI lPal6L16ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    UINT nMappedPins[] = {1, 2, 3, 10};

    UtPALDeviceCombinatorialReadAdapter(pDeviceIOFuncs, lFindPALData(L"PAL6L16"),
                                        pbyData, ulDataLen, L"Part #PAL6L16",
                                        nMappedPins, MArrayLen(nMappedPins));
}

static VOID UTPEPDEVICESAPI lPal8L14ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    UINT nMappedPins[] = {1, 2};

    UtPALDeviceCombinatorialReadAdapter(pDeviceIOFuncs, lFindPALData(L"PAL8L14"),
                                        pbyData, ulDataLen, L"Part #PAL8L14",
                                        nMappedPins, MArrayLen(nMappedPins));
}

static VOID UTPEPDEVICESAPI lPal20L8ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    UtPALDeviceCombinatorialRead(pDeviceIOFuncs, lFindPALData(L"PAL20L8"),
                                 pbyData, ulDataLen, TRUE);
}

static VOID UTPEPDEVICESAPI lPal20L10ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    UtPALDeviceCombinatorialRead(pDeviceIOFuncs, lFindPALData(L"PAL20L10"),
                                 pbyData, ulDataLen, TRUE);
}

static VOID UTPEPDEVICESAPI lPal20R4ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    UtPALDeviceRegisteredRead(pDeviceIOFuncs, lFindPALData(L"PAL20R4"),
                              pbyData, ulDataLen, TRUE);
}

static VOID UTPEPDEVICESAPI lPal20R6ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    UtPALDeviceRegisteredRead(pDeviceIOFuncs, lFindPALData(L"PAL20R6"),
                              pbyData, ulDataLen, TRUE);
}

static VOID UTPEPDEVICESAPI lPal20R8ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    UtPALDeviceRegisteredRead(pDeviceIOFuncs, lFindPALData(L"PAL20R8"),
                              pbyData, ulDataLen, TRUE);
}

static VOID UTPEPDEVICESAPI lPal20X4ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    UtPALDeviceRegisteredRead(pDeviceIOFuncs, lFindPALData(L"PAL20X4"),
                              pbyData, ulDataLen, TRUE);
}

static VOID UTPEPDEVICESAPI lPal20X8ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    UtPALDeviceRegisteredRead(pDeviceIOFuncs, lFindPALData(L"PAL20X8"),
                              pbyData, ulDataLen, TRUE);
}

static VOID UTPEPDEVICESAPI lPal20X10ReadDevice(
  const TDeviceIOFuncs* pDeviceIOFuncs,
  LPBYTE pbyData,
  ULONG ulDataLen)
{
    UtPALDeviceRegisteredRead(pDeviceIOFuncs, lFindPALData(L"PAL20X10"),
                              pbyData, ulDataLen, TRUE);
}

/***************************************************************************/
/*  Copyright (C) 2007-2016 Kevin Eshbach                                  */
/***************************************************************************/
