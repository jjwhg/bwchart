/*****************************************************************************/
/* Pkware.h                                   Copyright Ladislav Zezula 1999 */
/*                                                                           */
/* Author : Ladislav Zezula                                                  */
/* E-mail : systek@iol.cz                                                     */
/* WWW    : www.hyperlink.cz/ladik                                            */
/*---------------------------------------------------------------------------*/
/*       PKWARE data compression library structures, tables and functions    */
/*****************************************************************************/

#ifndef __PKWARE_H_
#define __PKWARE_H_

#include "StormPort.h"

//-----------------------------------------------------------------------------
// Typedefs

// Buffer input/output functions for Decompression1
typedef DWORD (WINAPI * FILLINPUTBUFF) (BYTE *, DWORD *, void *);
typedef void  (WINAPI * FILLOUTPUTBUFF)(BYTE *, DWORD *, void *);

//-----------------------------------------------------------------------------
// Structures

typedef struct
{
    BYTE * source;                      // 00 : Source compressed data buffer
    DWORD  sourcePos;                   // 04 : Position in source data buffer
    BYTE * target;                      // 08 : Target data buffer
    DWORD  targetPos;                   // 0C : Position in the target buffer
    DWORD  bytes;                       // 10 : Number of bytes to decompress
} TBufferInfo;

// PKWARE Data Compression Library structure (Size 0x3134 bytes)
typedef struct
{
    DWORD offs00;                       // 0000
    DWORD byte1;                        // 0004 - Seems to be always zero
    DWORD outputPos;                    // 0008 - Position in output buffer
    DWORD rBits;                        // 000C - Number of right bits (2. byte of compressed block)
    DWORD bitMask10;                    // 0010 - Some bit mask
    DWORD byteBuffer;                   // 0014 - 16-bit buffer for processing input data
    DWORD bitsExtra;                    // 0018 - Number of extra bits in byte buffer
    DWORD inputPos;                     // 001C - Source data position
    DWORD inputBytes;                   // 0020 - Number of bytes in input buffer
	void * userData;                    // 0024 - Custom data pointer
    FILLINPUTBUFF  FillInputBuffer;     // 0028 - Pointer to function filling input PKWARE buffer
    FILLOUTPUTBUFF FillOutputBuffer;    // 002C - Pointer to function storing decompressed data to custom buffer
    BYTE  outputBuffer[0x2000];			// 0030 - Output circle buffer. Starting position is 0x1000
    BYTE  inputBuffer[0x800];           // 2234 - Buffer for data to be decompressed
    BYTE  position1[0x100];             // 2A34 - Positions in buffers
    BYTE  position2[0x100];             // 2B34 - Positions in buffers
    BYTE  buff2C34[0x100];              // 2C34 - Buffer for 
    BYTE  buff2D34[0x100];              // 2D34 - Buffer for 
    BYTE  buff2E34[0x80];               // 2EB4 - Buffer for 
    BYTE  buff2EB4[0x100];              // 2EB4 - Buffer for 
    BYTE  buff2FB4[0x100];              // 2FB4 - Buffer for 
    BYTE  nSkip1[0x40];                 // 30B4 - Numbers of bytes to skip copied block length
    BYTE  nSkip2[0x10];                 // 30F4 - Numbers of bits for skip copied block length
    BYTE  nBits[0x10];                  // 3104 - Number of valid bits for copied block
    WORD  buff3114[0x10];               // 3114 - Buffer for 
} TPKWAREStruct;

// Input stream for Huffmann decoding
typedef struct
{
    WORD  * buffer;                     // 00 - Input byte buffer
    DWORD   value;                      // 04 - Input bit buffer
    DWORD   nBits;                      // 08 - Number of bits in 'value'
} TInputStream;

// Huffmann tree item ?
struct THTreeItem
{
    THTreeItem * next;                  // 00 - Pointer to next THTreeItem
    THTreeItem * prev;                  // 04 - Pointer to prev THTreeItem (< 0 if none)
    DWORD        dcmpByte;              // 08 - Index of this item in item pointer array, decompressed byte value
    DWORD        byteValue;             // 0C - Some byte value
    THTreeItem * parent;                // 10 - Pointer to parent THTreeItem (NULL if none)
    THTreeItem * child;                 // 14 - Pointer to child  THTreeItem
};

// Structure used for quick decompress. The 'bitCount' contains number of bits
// and byte value contains result decompressed byte value.
// After each walk through Huffman tree are filled all entries which are
// multiplies of number of bits loaded from input stream. These entries
// contain number of bits and result value. At the next 7 bits is tested this
// structure first. If corresponding entry found, decompression routine will
// not walk through Huffman tree and directly stores output byte to output stream.
typedef struct
{
    DWORD offs00;                       // 00 - TRUE if resolved
    DWORD bitCount;                     // 04 - Bit count
    union
    {
        DWORD        dcmpByte;          // 08 - Byte value for decompress (if bitCount <= 7)
        THTreeItem * item;              // 08 - HTree item (if number of bits is greater than 7
    };
} TQDecompress;

// Structure for Huffman tree (Size 3674 bytes);
typedef struct
{
    BOOL         zeroFirst;             // 0000 - TRUE if the first block byte was zero.
    DWORD        offs0004;              // 0004 - Some flag
    THTreeItem   items[0x203];          // 0008 - HTree items
    DWORD        offs3048;              // 3048 - 
    DWORD        offs304C;              // 3048 - 
    DWORD        offs3050;              // 3050 - Always NULL (?)
    //- Sometimes used as HTree item -----------
    THTreeItem * offs3054;              // 3054 - Pointer to Huffman tree item
    THTreeItem * offs3058;              // 3058 - Pointer to Huffman tree item (< 0 if invalid)
    THTreeItem * offs305C;              // 305C - Usually NULL
    THTreeItem * first;                 // 3060 - Pointer to top (first) Huffman tree item
    THTreeItem * last;                  // 3064 - Pointer to bottom (last) Huffman tree item (< 0 if invalid)
    DWORD        nItems;                // 3068 - Number of used HTree items
    //-------------------------------------------
    THTreeItem * item306C[0x102];       // 306C - HTree item pointer array
    TQDecompress qd3474[0x80];          // 3474 - Array for quick decompression
} THuffTree;

//-----------------------------------------------------------------------------
// Tables for decompression (In PkwareTables.cpp)

extern BYTE  Table1502A630[];
extern DWORD Dword1502AF44[];
extern DWORD Dword1502AFC4[];
extern BYTE  Table15034690[];
extern BYTE  Table150346D0[];
extern BYTE  nBits[];
extern WORD  Table15034720[];
extern BYTE  Table15034740[];
extern BYTE  Table15034750[];
extern BYTE  Table15034760[];
extern BYTE  Table15034790[];
extern WORD  Table15034860[];

//-----------------------------------------------------------------------------
// Compression and decompression functions

// Four PKWARE decompression functions
void Decompression1(BYTE * outBuffer, DWORD * pOutLength, BYTE * inpBuffer, DWORD inpLength);
void Decompression2(BYTE * outBuffer, DWORD * pOutLength, BYTE * inpBuffer, DWORD inpLength);
void Decompression3(BYTE * outBuffer, DWORD * pOutLength, BYTE * inpBuffer, DWORD inpLength);
void Explode4(BYTE * outBuffer, DWORD * pOutLength, BYTE * inpBuffer, DWORD inpLength);

// Main decompression functions
BOOL WINAPI Decompress(BYTE * outBuffer, DWORD * pOutLength, BYTE * inpBuffer, DWORD inpLength);

#endif  // __PKWARE_H_
