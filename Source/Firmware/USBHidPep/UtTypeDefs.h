/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/

#if !defined(UtTypeDefs_H)
#define UtTypeDefs_H

typedef unsigned char   byte;           // 8-bit
typedef unsigned int    word;           // 16-bit

#define MLowByteFromWord(w) (((byte*)&w)[0])
#define MHighByteFromWord(w) (((byte*)&w)[1])

typedef void (*TFunc)(void);

typedef union tagTPointer
{
    ram byte* pbyRam;                   // Ram byte pointer: 2 bytes pointer pointing
                                        // to 1 byte of data
    ram word* pwRam;                    // Ram word pointer: 2 bytes pointer pointing
                                        // to 2 bytes of data

    rom byte* pbyRom;                   // Size depends on compiler setting
    rom word* pwRom;
} TPointer;

#endif // UtTypeDefs_H

/***************************************************************************/
/*  Copyright (C) 2021-2021 Kevin Eshbach                                  */
/***************************************************************************/
