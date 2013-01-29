/*****************************************************************************/
/* Pkware.cpp                                 Copyright Ladislav Zezula 1999 */
/*                                                                           */
/* Author : Ladislav Zezula                                                  */
/* E-mail : systek@iol.cz                                                    */
/* WWW    : www.hyperlink.cz/ladik                                           */
/*---------------------------------------------------------------------------*/
/* PKWARE Data Compression Library decompression routines.                   */
/* This source file was created by dissassembling Storm.dll                  */
/*****************************************************************************/

#include "stdafx.h"
#include <assert.h>
//#include <windows.h>

#include "Pkware.h"

//-----------------------------------------------------------------------------
// Defines

#define INSERT_ITEM    1                    
#define SWITCH_ITEMS   2                    // Switch the item1 and item2

#define POINTER_COMPLEMENT(ptr)  (THTreeItem *)~(DWORD)(ptr)

//-----------------------------------------------------------------------------
// Local structures

// Table of decompression routines for each type of compression
typedef struct
{
    BYTE mask;                          // This bit must be set to use this decompression routine
    void (*Decompression)(BYTE *, DWORD *, BYTE *, DWORD);
} TFuncTable;

/*****************************************************************************/
/*                                                                           */
/*                     Decompression methods 1 and 2                         */
/*                                                                           */
/*  Not known type of decompression. If you know what's decompression type   */
/*  it is, please e-mail me.                                                 */
/*                                                                           */
/*****************************************************************************/

//----------------------------------------------------------------------------
// DWORD Call15007AA0(BYTE * target, DWORD outLength, BYTE * source, DWORD inpLength, DWORD param)

static DWORD Call15007AA0(BYTE * target, DWORD outLength, BYTE * source, DWORD inpLength, DWORD param)
//           (15007AA0)   ECX            EDX              [ESP+28]       [ESP+2C]         [ESP+30]
{
    BYTE * targetStart = target;            // [ESP+10] Output buffer copy
    BYTE * sourceStart = source;            // [ESP+14] Input buffer copy
    BYTE * sourceEnd   = source + inpLength;// [ESP+28] End of input buffer
    LONG   array1[2];                       // [ESP+18] 
    LONG   array2[2];                       // [ESP+20] 
    DWORD  index;                           // EBP

    array1[0]  = 0x2C;
    array1[1]  = 0x2C;
    source += 2;

    // 15007AD7
    if(param != 0)
    {
        DWORD * array = (DWORD *)array2;    // ECX

        for(DWORD count = 0; count < param; count++) // ESI
        {
            LONG    temp;                   // EBP

            temp = *(SHORT *)source;
            source += sizeof(SHORT);
            *array++ = temp;

            if(outLength < 2)
                return (DWORD)(target - targetStart);

            *(WORD *)target = (WORD)temp;
            target    += sizeof(WORD);
            outLength -= sizeof(WORD);
        }
    }

    index = param - 1;                      // EBP
    while(source < sourceEnd)
    {
        BYTE oneByte = *source++;           // EBX

        if(param == 2)
            index = (index == 0) ? 1 : 0;   // EBP

        // Get one byte from input buffer
        // 15007B25
        if(oneByte & 0x80)
        {
            // 15007B32
            switch(oneByte & 0x7F)
            {
                case 0:     // 15007B8E
                    if(array1[index] != 0)
                        array1[index]--;

                    if(outLength < 2)
                        break;

                    *(WORD *)target = (WORD)array2[index];
                    target    += sizeof(WORD);
                    outLength -= sizeof(WORD);
                    continue;

                case 1:     // 15007B72
                    array1[index] += 8;   // EBX also
                    if(array1[index] > 0x58)
                        array1[index] = 0x58;
                    if(param == 2)
                        index = (index == 0) ? 1 : 0;
                    continue;

                case 2:
                    continue;

                default:
                    array1[index] -= 8;
                    if(array1[index] < 0)
                        array1[index] = 0;

                    if(param != 2)
                        continue;

                    index = (index == 0) ? 1 : 0;
                    continue;
            }
        }
        else
        {
            DWORD temp1 = Dword1502AFC4[array1[index]]; // EDI
            DWORD temp2 = temp1 >> sourceStart[1];      // ESI
            LONG  temp3 = array2[index];                // ECX

            if(oneByte & 0x01)          // EBX = oneByte
                temp2 += (temp1 >> 0);

            if(oneByte & 0x02)
                temp2 += (temp1 >> 1);

            if(oneByte & 0x04)
                temp2 += (temp1 >> 2);

            if(oneByte & 0x08)
                temp2 += (temp1 >> 3);

            if(oneByte & 0x10)
                temp2 += (temp1 >> 4);

            if(oneByte & 0x20)
                temp2 += (temp1 >> 5);

            if(oneByte & 0x40)
            {
                temp3 -= temp2;
                if(temp3 <= (LONG)0xFFFF8000)
                    temp3 = (LONG)0xFFFF8000;
            }
            else
            {
                temp3 += temp2;
                if(temp3 >= 0x7FFF)
                    temp3 = 0x7FFF;
            }
            array2[index] = temp3;
            if(outLength < 2)
                break;

            temp2 = array1[index];
            oneByte &= 0x1F;
            *(WORD *)target = (WORD)temp3;
            target += sizeof(WORD);

            outLength -= 2;
            temp2 += Dword1502AF44[oneByte];
            array1[index] = temp2;

            if(array1[index] < 0)
                array1[index] = 0;
            else if(array1[index] > 0x58)
                array1[index] = 0x58;
        }
    }
    return (DWORD)(target - targetStart);
}

/*****************************************************************************/
/*                                                                           */
/*                     Decompression methods 3                               */
/*                                                                           */
/* This decompression is based at Huffman decompression. The dictionary      */
/* (decompressed bytes) are not stored in compressed block, but in static    */
/* buffer. Huffman tree structure is given by the first byte of the block.   */
/* Huffman tree can modify itself during the decompression process.          */
/*                                                                           */
/*****************************************************************************/

//-----------------------------------------------------------------------------
// TInputStream functions

// Initializes input bit stream. 
static void InitInputStream(TInputStream * is, BYTE * source, DWORD inpLength)
//          (15006730)          ECX                [ESP+4]          [ESP+8]
{
    DWORD * temp = (DWORD *)source;

    is->value  = *temp++;               // Store the first 32 bits
    is->buffer = (WORD *)temp;          // Store input buffer pointer
    is->nBits  = 32;                    // Initialiy 32 bits
}

// Gets one bit from input stream
static DWORD GetBit(TInputStream * is)
//           (15006FE0)    ECX
{
    DWORD bit = (is->value & 1);

    is->value >>= 1;
    is->nBits--;

    if(is->nBits == 0)
    {
        DWORD * temp = (DWORD *)is->buffer;

        is->value  = *temp++;
        is->buffer = (WORD *)temp;
        is->nBits  = 32;
    }
    return bit;
}    

//-----------------------------------------------------------------------------
// Huffman tree item manipulating functions

// Gets previous Huffman tree item (?)
static THTreeItem * GetPrevItem(THTreeItem * item, DWORD value)
//                  (1500E250)  ECX                [ESP+4]
{
    THTreeItem * prev = item->prev;      // EAX

    if((LONG)prev < 0)
        return POINTER_COMPLEMENT(prev);
    if((LONG)value < 0)
        value = (LONG)(item - item->next->prev);

    return (THTreeItem *)((char *)prev + value);
}

// Inserts item into the tree (?)
static void InsertItem(THTreeItem ** itemPtr, THTreeItem * item, DWORD where, THTreeItem * item2)
//          (150083A0) ECX,                    [ESP+10]           [ESP+14]      [ESP+18]
{
    THTreeItem * next = item->next;     // EDI - next to the first item
    THTreeItem * prev = item->prev;     // ESI - prev to the first item
    THTreeItem * next2;                 // Pointer to the next item
    THTreeItem * prev2;                 // Pointer to previous item
    
    // 150083A7
    // The same code like in RemoveItem(item);
    if(next != 0)                       // If the first item already has next one
    {
        if((LONG)prev < 0)
            prev = POINTER_COMPLEMENT(prev);
        else
            // ??? usually item == next->prev, so what is it ?
            prev = (THTreeItem *)((BYTE *)prev + (DWORD)((BYTE *)item - (BYTE *)(next->prev)));

        // 150083C1
        // Remove the item from the tree
        prev->next = next;
        next->prev = prev;

        // Invalidate 'prev' and 'next' pointer
        item->next = 0;
        item->prev = 0;
    }

    // 150083D8
    if(item2 == NULL)                   // EDX - If the second item is not entered,
        item2 = (THTreeItem *)&itemPtr[1]; // take the first tree item

    // 150083D8
    switch(where)
    {
        // 150083ED
        case SWITCH_ITEMS :             // Switch the two items
            // Switch the items         
            item->next  = item2->next;  // ECX = item2->next (Pointer to pointer to first)
            item->prev  = item2->next->prev;
            item2->next->prev = item;
            item2->next = item;         // Set the first item
            return;
        
        // 15008402
        case INSERT_ITEM:               // Insert as the last item
            item->next = item2;         // Set next item (or pointer to pointer to first item)
            item->prev = item2->prev;   // Set prev item (or last item in the tree)

            next2 = itemPtr[0];         // ESI - NULL usually
            prev2 = item2->prev;        // ECX - Prev item to the second (or last tree item)
            
            if((LONG)prev2 < 0)
            {
                prev2 = POINTER_COMPLEMENT(prev);

                prev2->next = item;
                item2->prev = item;     // Next after last item
                return;
            }

            // 15008420
            if((LONG)next2 < 0)
                next2 = (THTreeItem *)(DWORD)((BYTE *)item2 - (BYTE *)(item2->next->prev));

            // 1500842B
            prev2 = (THTreeItem *)((CHAR *)prev2 + (DWORD)next2);// ???
            prev2->next = item;
            item2->prev = item;         // Set the next/last item
            return;

        default:
            return;
    }
}

// Removes item from the tree (?)
static void RemoveItem(THTreeItem * item)
//          (15024080) ECX
{
    THTreeItem * next = item->next;     // ESI
    THTreeItem * prev = item->prev;     // EDX

    if(next == NULL)
        return;

    if((LONG)prev < 0)
        prev = POINTER_COMPLEMENT(prev);
    else
        // ??? usually item == next->prev, so what is it ?
        prev = (THTreeItem *)((BYTE *)prev + (DWORD)((BYTE *)item - (BYTE *)(next->prev)));

    // Remove HTree item from the chain
    prev->next = next;                  // Sets the 'first' pointer
    next->prev = item->prev;

    // Invalidate pointers
    item->next = NULL;
    item->prev = NULL;
}

//-----------------------------------------------------------------------------
// Huffman tree functions

static void ClearHuffmanTree(THuffTree * htr)
//          (150067D0)       ECX
{
    THTreeItem * item  = (THTreeItem *)htr->items;    // ECX

    for(DWORD count = 0x203; count != 0; item++, count--)
    {
        item->next = NULL;
        item->prev = NULL;
    }

    htr->offs3050 = 0;
    htr->offs3054 = (THTreeItem *)&htr->offs3054;
    htr->offs3058 = POINTER_COMPLEMENT(htr->offs3054);
    
    htr->offs305C = 0;
    htr->first    = (THTreeItem *)&htr->first;
    htr->last     = POINTER_COMPLEMENT(htr->first);

    htr->nItems   = 0;
    htr->offs0004 = 1;
}

static void InitHuffmanTree(THuffTree * htr)
//     (15006CA0)     ECX
{
    TQDecompress * qd;
    DWORD          count;

    ClearHuffmanTree(htr);

    for(qd = (TQDecompress *)htr->qd3474, count = 0; count < 0x80; count++, qd++)
        qd->offs00 = 0;
}

// Builds Huffman tree. Called with the first 8 bits loaded from input stream
static void BuildHuffmanTree(THuffTree * htr, DWORD oneByte)
//          (15006A10)       ECX              [ESP+1C]
{
    DWORD         maxByte;              // [ESP+10] - The greatest character found in table
    THTreeItem ** itemPtr;              // [ESP+14] - Pointer to Huffman tree item pointer array
//  DWORD         DwordESP18;           // [ESP+18] -
    BYTE        * byteArray;            // [ESP+1C] - Pointer to BYTE in Table1502A630
    THTreeItem  * last;
    THTreeItem  * child1;
    DWORD	i;                      // egcs in linux doesn't like multiple for loops without an explicit i

    // 15006A19
    // Test pointer for negative value
    if((LONG)(last = htr->last) > 0)    // ESI - Last entry
    {
        THTreeItem * temp;              // EAX

        if(last->next != NULL)          // ESI->next
            RemoveItem(htr->last);
                                        // EDI = &htr->offs3054
        htr->offs3058 = (THTreeItem *)&htr->offs3054; // [EDI+4]
        last->prev    = htr->offs3058;  // EAX

        temp = GetPrevItem((THTreeItem *)&htr->offs3054, (LONG)&htr->offs3050);

        temp->next    = last;
        htr->offs3054 = last;
    }

    // Clear all pointers in HTree item array
    memset(htr->item306C, 0, sizeof(htr->item306C));

    // 15006A63:
    maxByte = 0;                        // Greatest character found init to zero.
    itemPtr = (THTreeItem **)&htr->item306C;           // Pointer to current entry in HTree item pointer array

    // Ensure we have low 8 bits only
    oneByte  &= 0xFF;                   // EAX also
    byteArray = Table1502A630 + ((oneByte << 7) + oneByte) * 2; // EDI also

    // 15006A8C
    // EBP = i;
    for(i = 0; i < 0x100; itemPtr++, i++)
    {
        THTreeItem * item    = htr->offs3058;   // ESI - Item to be created
        THTreeItem * temp2   = htr->offs3058;   // EDI
        BYTE         oneByte = byteArray[i];    // EAX

        // Skip all the bytes which are zero.
        if(byteArray[i] == 0)                   // EAX - 0xC3, 0xD9, ...
            continue;

        // 15006A9C - If not valid pointer, take the first available item in the array
        if((LONG)item <= 0)    
            item = &htr->items[htr->nItems++];

        // 15006AB6 - Insert this item as the top of the tree
        InsertItem(&htr->offs305C, item, SWITCH_ITEMS, NULL);

        item->parent    = NULL;                 // Invalidate child and parent
        item->child     = NULL;
        *itemPtr        = item;                 // Store pointer into pointer array

        item->dcmpByte  = i;                    // Store counter
        item->byteValue = oneByte;              // Store byte value
        if(oneByte >= maxByte)
        {
            maxByte = oneByte;
            continue;
        }

        // 15006AED
        // Find the first item which has byte value greater than current one byte
        if((LONG)(temp2 = htr->last) > 0)       // EDI - Pointer to the last item
        {
            // 15006AF7
            if(temp2 != NULL)
            {
                do  // 15006AFB
                {
                    if(temp2->byteValue >= oneByte)
                        goto _15006B09;
                    temp2 = temp2->prev;
                }
                while((LONG)temp2 > 0);
            }
        }
        temp2 = NULL;

        // 15006B09
        _15006B09:
        if(item->next != NULL)
            RemoveItem(item);

        // 15006B15
        if(temp2 == NULL)
            temp2 = (THTreeItem *)&htr->first;

        // 15006B1F
        item->next = temp2->next;
        item->prev = temp2->next->prev;
        temp2->next->prev = item;
        temp2->next = item;
    }

    // 15006B4A
    for(; i < 0x102; i++)
    {
        THTreeItem ** itemPtr = &htr->item306C[i];  // EDI

        // 15006B59
        THTreeItem * item = htr->offs3058;          // ESI
        if((LONG)item <= 0)
            item = &htr->items[htr->nItems++];

        InsertItem(&htr->offs305C, item, INSERT_ITEM, NULL);

        // 15006B89
        item->dcmpByte   = i;
        item->byteValue  = 1;
        item->parent     = NULL;
        item->child      = NULL;
        *itemPtr++ = item;
    }

    // 15006BAA
    if((LONG)(child1 = htr->last) > 0)              // EDI - last item (first child to item
    {
        THTreeItem * child2;                        // EBP
        THTreeItem * item;                          // ESI

        // 15006BB8
        while((LONG)(child2 = child1->prev) > 0)
        {
            if((LONG)(item = htr->offs3058) <= 0)
                item = &htr->items[htr->nItems++];

            // 15006BE3
            InsertItem(&htr->offs305C, item, SWITCH_ITEMS, NULL);

            // 15006BF3
            item->parent = NULL;
            item->child  = NULL;

            //EDX = child2->byteValue + child1->byteValue;
            //EAX = child1->byteValue;
            //ECX = maxByte;                        // The greatest character (0xFF usually)

            item->byteValue = child1->byteValue + child2->byteValue; // 0x02
            item->child     = child1;                                // Prev item in the 
            child1->parent  = item;
            child2->parent  = item;

            // EAX = item->byteValue;
            if(item->byteValue >= maxByte)
                maxByte = item->byteValue;
            else
            {
                THTreeItem * temp1 = child2->prev;   // EDI

                if((LONG)temp1 > 0)
                {
                    // 15006C2D
                    do
                    {
                        if(temp1->byteValue >= item->byteValue)
                            goto _15006C3B;
                        temp1 = temp1->prev;
                    }
                    while((LONG)temp1 > 0);
                }
                temp1 = NULL;

                _15006C3B:
                if(item->next != 0)
                {
                    THTreeItem * temp4 = GetPrevItem(item, -1);
                                                                    
                    temp4->next      = item->next;                 // The first item changed
                    item->next->prev = item->prev;                 // First->prev changed to negative value
                    item->next = NULL;
                    item->prev = NULL;
                }

                // 15006C62
                if(temp1 == NULL)
                    temp1 = (THTreeItem *)&htr->first;

                item->next = temp1->next;                           // Set item with 0x100 byte value
                item->prev = temp1->next->prev;                     // Set item with 0x17 byte value
                temp1->next->prev = item;                           // Changed prev of item with
                temp1->next = item;
            }
            // 15006C7B
            if((LONG)(child1 = child2->prev) <= 0)
                break;
        }
    }
    // 15006C88
    htr->offs0004 = 1;
}

// Modifies Huffman tree. Adds new item and changes
static void ModifyHuffmanTree(THuffTree * htr, DWORD index)
//          (15006870)   ECX              ESP+14
{
    THTreeItem * item = htr->offs3058;                              // ESI
    THTreeItem * last = ((LONG)htr->last <= 0) ? NULL : htr->last;  // EBX
    THTreeItem * temp;                                              // EAX 

    // Prepare the first item to insert to the tree
    if((LONG)item <= 0)
        item = &htr->items[htr->nItems++];

    // 150068A6
    // If item item has any next item, remove it from the chain
    if(item->next != NULL)
    {
        THTreeItem * temp = GetPrevItem(item, -1);                  // EAX

        temp->next = item->next;
        item->next->prev = item->prev;
        item->next = NULL;
        item->prev = NULL;
    }

    item->next = (THTreeItem *)&htr->first;
    item->prev = htr->last;
    temp = GetPrevItem(item->next, (LONG)htr->offs305C);

    // 150068E9
    temp->next = item;
    htr->last  = item;

    item->parent = NULL;
    item->child  = NULL;

    // 150068F6
    item->dcmpByte  = last->dcmpByte;      // Copy item index
    item->byteValue = last->byteValue;  // Copy item byte value
    item->parent    = last;             // Set parent to last item
    htr->item306C[last->dcmpByte] = item;  // Insert item into item pointer array

    // Prepare the second item to insert into the tree
    if((LONG)(item = htr->offs3058) <= 0)
        item = &htr->items[htr->nItems++];

    // 1500692E
    if(item->next != NULL)
    {
        temp = GetPrevItem(item, -1);   // EAX

        temp->next = item->next;
        item->next->prev = item->prev;
        item->next = NULL;
        item->prev = NULL;
    }
    // 1500694C
    item->next = (THTreeItem *)&htr->first;
    item->prev = htr->last;
    temp = GetPrevItem(item->next, (LONG)htr->offs305C);

    // 15006968
    temp->next = item;
    htr->last  = item;

    // 1500696E
    item->child     = NULL;
    item->dcmpByte  = index;
    item->byteValue = 0;
    item->parent    = last;
    last->child     = item;
    htr->item306C[index] = item;

    do
    {
        THTreeItem * temp1 = item;      // EDI
        THTreeItem * temp2;             // EBX
        DWORD        byteValue;         // EAX

        // 15006993
        byteValue = ++item->byteValue;

        // Pass through all previous which have its value greater than byteValue
        while((LONG)(temp2 = temp1->prev) > 0)  // EBX
        {
            if(temp2->byteValue >= byteValue)
                goto _150069AE;

            temp1 = temp1->prev;
        }
        // 150069AC
        temp2 = NULL;

        _150069AE:
        if(temp1 == item)
            continue;

        // 150069B2
        // Switch temp1 with item
        InsertItem(&htr->offs305C, temp1, SWITCH_ITEMS, item);
        InsertItem(&htr->offs305C, item,  SWITCH_ITEMS, temp2);

        // 150069D0
        // Switch parents of item and temp1
        temp = temp1->parent->child;
        if(item == item->parent->child)
            item->parent->child = temp1;

        if(temp1 == temp)
            temp1->parent->child = item;

        // 150069ED
        // Switch parents of item and temp2
        temp = item->parent;
        item ->parent = temp1->parent;
        temp1->parent = temp;
        htr->offs0004++;
    }
    while((LONG)(item = item->parent) > 0);
}

// Call15007010(htr, htr->item306C[ECX]);
static void Call15007010(THuffTree * htr, THTreeItem * item)
//          (15007010)   ECX              [ESP+10]
{
    assert("This code needs to be tested first !!!");
/*
15007010   push        ecx
15007011   push        ebp
15007012   push        esi
15007013   mov         esi,dword ptr [esp+10h]
15007017   xor         ebp,ebp
15007019   cmp         esi,ebp
1500701B   mov         dword ptr [esp+8],ecx
1500701F   je          1500710F
15007025   push        ebx
15007026   push        edi
15007027   mov         eax,dword ptr [esi+0Ch]
1500702A   mov         edi,esi
1500702C   inc         eax
1500702D   mov         dword ptr [esi+0Ch],eax
15007030   mov         ecx,eax
15007032   mov         eax,dword ptr [edi+4]
15007035   cmp         eax,ebp
15007037   jle         15007048
15007039   mov         ebx,eax
1500703B   cmp         ebx,ebp
1500703D   je          1500704A
1500703F   cmp         dword ptr [ebx+0Ch],ecx
15007042   jae         1500704A
15007044   mov         edi,ebx
15007046   jmp         15007032
15007048   xor         ebx,ebx
1500704A   cmp         edi,esi
1500704C   je          15007102
15007052   cmp         dword ptr [edi],ebp
15007054   je          15007070
15007056   push        0FFh
15007058   mov         ecx,edi
1500705A   call        1500E250
1500705F   mov         ecx,dword ptr [edi]
15007061   mov         dword ptr [eax],ecx
15007063   mov         edx,dword ptr [edi]
15007065   mov         eax,dword ptr [edi+4]
15007068   mov         dword ptr [edx+4],eax
1500706B   mov         dword ptr [edi],ebp
1500706D   mov         dword ptr [edi+4],ebp
15007070   cmp         esi,ebp
15007072   je          15007078
15007074   mov         ecx,esi
15007076   jmp         15007082
15007078   mov         ecx,dword ptr [esp+10h]
1500707C   add         ecx,3060h
15007082   mov         eax,dword ptr [ecx]
15007084   mov         dword ptr [edi],eax
15007086   mov         edx,dword ptr [eax+4]
15007089   mov         dword ptr [edi+4],edx
1500708C   mov         dword ptr [eax+4],edi
1500708F   mov         dword ptr [ecx],edi
15007091   mov         eax,dword ptr [esi]
15007093   cmp         eax,ebp
15007095   je          150070B1
15007097   push        0FFh
15007099   mov         ecx,esi
1500709B   call        1500E250
150070A0   mov         ecx,dword ptr [esi]
150070A2   mov         dword ptr [eax],ecx
150070A4   mov         edx,dword ptr [esi]
150070A6   mov         eax,dword ptr [esi+4]
150070A9   mov         dword ptr [edx+4],eax
150070AC   mov         dword ptr [esi],ebp
150070AE   mov         dword ptr [esi+4],ebp
150070B1   cmp         ebx,ebp
150070B3   je          150070BD
150070B5   mov         edx,dword ptr [esp+10h]
150070B9   mov         ecx,ebx
150070BB   jmp         150070C7
150070BD   mov         edx,dword ptr [esp+10h]
150070C1   lea         ecx,[edx+3060h]
150070C7   mov         eax,dword ptr [ecx]
150070C9   mov         dword ptr [esi],eax
150070CB   mov         ebx,dword ptr [eax+4]
150070CE   mov         dword ptr [esi+4],ebx
150070D1   mov         dword ptr [eax+4],esi
150070D4   mov         dword ptr [ecx],esi
150070D6   mov         eax,dword ptr [esi+10h]
150070D9   mov         ecx,dword ptr [edi+10h]
150070DC   mov         ebx,dword ptr [eax+14h]
150070DF   mov         ecx,dword ptr [ecx+14h]
150070E2   cmp         ebx,esi
150070E4   jne         150070E9
150070E6   mov         dword ptr [eax+14h],edi
150070E9   cmp         ecx,edi
150070EB   jne         150070F3
150070ED   mov         eax,dword ptr [edi+10h]
150070F0   mov         dword ptr [eax+14h],esi
150070F3   mov         ecx,dword ptr [edi+10h]
150070F6   mov         eax,dword ptr [esi+10h]
150070F9   mov         dword ptr [esi+10h],ecx
150070FC   mov         dword ptr [edi+10h],eax
150070FF   inc         dword ptr [edx+4]
15007102   mov         esi,dword ptr [esi+10h]
15007105   cmp         esi,ebp
15007107   jne         15007027
1500710D   pop         edi
1500710E   pop         ebx
1500710F   pop         esi
15007110   pop         ebp
15007111   pop         ecx
15007112   ret         4
*/
}

// Decompression using Huffman tree
static DWORD Call15006CD0(THuffTree * htr, BYTE * target, DWORD outLength, TInputStream * is)
//           (15006CD0)   ECX                 EDX               ESP+            ESP+
{
    THTreeItem * item;                  // [ESP+10] - Current item if walking HTree
    DWORD        bitCount;              // [ESP+14] - Bit counter if walking HTree
    DWORD        oneByte;               // [ESP+18] - 8 bits from bit stream/Pointer to target
    BYTE       * outPtr;                // [ESP+18] - Current pointer to output buffer
    BOOL         hasQDEntry;            // [ESP+1C] - TRUE if entry for quick decompression if filled
    THTreeItem * itemAt7 = NULL;        // [ESP+20] - HTree item found at 7th bit
    THTreeItem * temp;                  // For every use
    DWORD        dcmpByte;              // Decompressed byte value
    
    if(outLength == 0)
        return 0;

    // If too few bits in input bit buffer, we have to load next 16 bits
    if(is->nBits <= 8)                  // (ECX) Number of bits
    {
        is->value |= (*is->buffer++ << is->nBits);
        is->nBits += 16;
    }

    // Get 8 bits from input stream
    oneByte = (is->value & 0xFF);       // EAX - oneByte, ECX - (oneByte & 0xFF)
    is->value >>= 8;
    is->nBits  -= 8;

    // 15006D2D
    BuildHuffmanTree(htr, oneByte);     // Initialize the Huffman tree

    htr->zeroFirst = (oneByte == 0) ? TRUE : FALSE;
    outPtr = target;                 // (ECX) Copy pointer to output data

    // 15006D50
    while(1)
    {
        TQDecompress * qd;              // For quick decompress
        DWORD sevenBits;                // 7 bits from input stream

        if(is->nBits <= 7)
        {
            is->value |= (*is->buffer++ << is->nBits);
            is->nBits += 16;
        }

        // 15006D74
        // Get 7 bits from input stream
        sevenBits = (is->value & 0x7F); // ESI

        // Try to use quick decompression. Check TQDecompress array for corresponding item.
        // If found, ise the result byte instead.
        qd = &htr->qd3474[sevenBits];   // EBP

        // If there is a quick-pass possible
        hasQDEntry = (qd->offs00 == htr->offs0004) ? TRUE : FALSE;

        // Start passing the Huffman tree. Set item to tree root item
        item = htr->first;
//      item = ((LONG)htr->first->next->prev <= 0) ? NULL : htr->first->next->prev;

        // If we can use quick decompress, use it.
        if(hasQDEntry == TRUE)
        {
            // 15006D99
            // Check the bit count is greater than 7, move item to 7 levels deeper
            if((bitCount = qd->bitCount) > 7)       // ECX
            {
                is->value >>= 7;
                is->nBits  -= 7;
                item        = qd->item; // Don't start with root item, but with some deeper-laying
                goto _startHuffWalk;
            }

            // If OK, use their byte value
            is->value >>= bitCount;
            is->nBits  -= bitCount;
            dcmpByte    = qd->dcmpByte; // ECX
        }
        else    // 15006DCE
        {
            // Walk through Huffman Tree
            // 15006DEF
            _startHuffWalk:
            bitCount = 0;               // Clear bit counter
            do
            {
                item = item->child;     // Move down by one level
                if(GetBit(is) != 0)     // If current bit is set, move to previous
                    item = item->prev;  // item in current level

                // 15006E16
                if(++bitCount == 7)     // If we are at 7th bit, store current HTree item.
                    itemAt7 = item;     // Store Huffman tree item
            }
            while(item->child != NULL); // Walk until tree has no deeper level

            // 15006E2F
            // If quick decompress entry is not filled yet, fill it.
            if(hasQDEntry == FALSE)     // EAX also
            {
                if(bitCount > 7)        // If we passed more than 7 bits, store bitCount and item
                {
                    qd->offs00   = htr->offs0004;   // Value indicates that entry is resolved
                    qd->bitCount = bitCount;        // Number of bits passed
                    qd->item     = itemAt7;         // Store item at 7th bit
                }
                else                    // If we passed less than 7 bits, fill
                {                       // entry and bit count multipliers
                    DWORD index;        // Index for quick-decompress entry
                    DWORD addIndex;     // Add value for index

                    index    = sevenBits & (0xFFFFFFFF >> (32 - bitCount));
                    addIndex = (1 << bitCount);

                    // 15006E7B
                    for(qd = &htr->qd3474[index]; index <= 0x7F; index += addIndex, qd += addIndex)
                    {
                        qd->offs00   = htr->offs0004;
                        qd->bitCount = bitCount;
                        qd->dcmpByte = item->dcmpByte;
                    }
                }
            }
            // 15006E9A
            dcmpByte = item->dcmpByte;
        }

        // 15006EA5
        if(dcmpByte == 0x101)        // Huffman tree needs to be modified
        {
            // 15006EB3
            // Check if there is enough bits in the buffer
            if(is->nBits <= 8)
            {
                is->value |= (*is->buffer++ << is->nBits);
                is->nBits += 16;
            }

            // 15006ED7
            // Get 8 bits from the buffer
            oneByte     = (is->value & 0xFF); // ESI also
            is->value >>= 8;
            is->nBits  -= 8;

            // Modify Huffman tree
            ModifyHuffmanTree(htr, oneByte);

            // Get lastly added tree item
            item = htr->item306C[oneByte];          // ESI

            if(htr->zeroFirst == FALSE && item != NULL)
            {
                // 15006F15
                do
                {
                    THTreeItem * temp1 = item;  // EDI
                    THTreeItem * temp2;         // EBP
                    DWORD        byteValue;     // EAX

                    byteValue = ++item->byteValue;

                    // 15006F1E
                    while((LONG)(temp2 = temp1->prev) > 0)
                    {
                        if(temp2->byteValue >= byteValue)
                            goto _15006F30;

                        temp1 = temp1->prev;
                    }
                    temp2 = NULL;

                    // 15006F30
                    _15006F30:
                    if(temp1 == item)
                        continue;

                    // 15006F34
                    InsertItem(&htr->offs305C, temp1, SWITCH_ITEMS, item);
                    InsertItem(&htr->offs305C, item,  SWITCH_ITEMS, temp2);

                    // 15006F5B
                    temp = temp1->parent->child;
                    if(item == item->parent->child)
                        item->parent->child = temp1;

                    if(temp1 == temp)
                        temp1->parent->child = item;

                    // 15006F6D
                    // Switch parents of item and temp2
                    temp = item->parent;
                    item ->parent = temp1->parent;
                    temp1->parent = temp;
                    htr->offs0004++;
                }
                while((LONG)(item = item->parent) > 0);
            }
            dcmpByte = oneByte;
        }

        // 15006F8B
        if(dcmpByte == 0x100)        // End of data ?
            return (DWORD)(outPtr - target);

        // 15006F97
        // Save character in the output buffer
        *outPtr++ = (BYTE)dcmpByte;
        if(--outLength == 0)
            return (DWORD)(outPtr - target);

        if(htr->zeroFirst == TRUE)
            Call15007010(htr, htr->item306C[item->byteValue]);
    }
    return (DWORD)(outPtr - target);
}

/*****************************************************************************/
/*                                                                           */
/*                          Decompression method 4                           */
/*                                                                           */
/*  Not known type of decompression. If you know what's decompression type   */
/*  it is, please e-mail me.                                                 */
/*  Warning : Addresses here correspond to Storm.dll from Diablo 1.00        */
/*                                                                           */
/*****************************************************************************/

//-----------------------------------------------------------------------------
// CopySourceData(BYTE * buffer, DWORD * maxBytes, void * info)
// (1500C860)       ECX               EDX                Stack
//
// Function for load input compressed data from custom buffer into PKWARE
// buffer. Required by PKWARE Data Compression Library.
// Returns number of bytes copied into PKWARE buffer
//    
//   char  * buffer   - Pointer to PKWARE source buffer.
//   DWORD * maxBytes - Max. number of bytes which can be copied into PKWARE buffer.
//   void  * userData - Pointer to user data (passed into decompress function)

static DWORD WINAPI CopySourceData(BYTE * buffer, DWORD * maxBytes, void * userData)
{
    TBufferInfo * info = (TBufferInfo *)userData;
    DWORD remain = info->bytes - info->sourcePos;

    // Cut remaining bytes to size of target buffer
    if(remain >= *maxBytes)
        remain = *maxBytes;
    
    // Copy data into PKWARE buffer
    memcpy(buffer, info->source + info->sourcePos, remain);
    info->sourcePos += remain;

    return remain;
}

//------------------------------------------------------------------------
// CopyTargetData(BYTE * buffer, DWORD * maxBytes, TBufferInfo * info)
// (1500C8A0)       ECX                EDX                Stack
//
// Function for store decompressed data from PKWARE buffer
// to custom target buffer. Required by PKWARE Data Compression Library.
// Returns nothing
//    
//   char  * buffer   - Pointer to PKWARE buffer with decompressed data
//   DWORD * maxBytes - Max. number of bytes which can be copied into PKWARE buffer.
//   void  * userData - Pointer to user data (passed into decompress function)

static void WINAPI CopyTargetData(BYTE * buffer, DWORD * maxBytes, void * userData)
{
    TBufferInfo * info = (TBufferInfo *)userData;

    memcpy(info->target + info->targetPos, buffer, *maxBytes);
    info->targetPos += *maxBytes;
}

// Skips given number of bits in bit buffer. Result is stored in ps->byteBuffer
// If no data in input buffer, returns TRUE
static BOOL SkipBits(TPKWAREStruct * ps, DWORD nBits)
//     (1502C850)    ECX                 EDX
{
    // If number of bits required is less than number of (bits in the buffer) ?
    if(nBits <= ps->bitsExtra)
    {
        ps->bitsExtra   -= nBits;
        ps->byteBuffer >>= nBits;
        return FALSE;
    }

    // 1502C86D:
    // Load input buffer if necessary
    ps->byteBuffer >>= ps->bitsExtra;
    if(ps->inputBytes == ps->inputPos)
    {
        ps->inputPos = sizeof(ps->inputBuffer);
        if((ps->inputBytes = ps->FillInputBuffer(ps->inputBuffer, &ps->inputPos, ps->userData)) == 0)
            return TRUE;
        ps->inputPos = 0;
    }

    // 1502C8A8:
    // Update bit buffer
    ps->byteBuffer  |= (ps->inputBuffer[ps->inputPos++] << 8);
    ps->byteBuffer >>= (nBits - ps->bitsExtra);
    ps->bitsExtra    = (ps->bitsExtra - nBits) + 8;
    return FALSE;
}

// DWORD GetMoveBack(TPKWAREStruct * ps, DWORD something)
static DWORD GetMoveBack(TPKWAREStruct * ps, DWORD something)
//           (1502C7D0)  ECX                 EDX
{
    DWORD pos   = ps->position1[(ps->byteBuffer & 0xFF)]; // (EDI)
    DWORD nSkip = ps->nSkip1[pos];                        // (EDX)    

    // 1502C7E9:
    if(SkipBits(ps, nSkip) == TRUE)
        return 0;

    // 1502C801:
    if(something == 2)
    {
        pos = (pos << 2) | (ps->byteBuffer & 0x03);

        if(SkipBits(ps, 2) == TRUE)
            return 0;
    }
    else
    {
        pos = (pos << ps->rBits) | (ps->byteBuffer & ps->bitMask10);

        if(SkipBits(ps, ps->rBits) == TRUE)
            return 0;
    }
    return pos+1;
}

// Returns : 0x000 - 0x0FF : One byte from compressed file.
//           0x100 - 0x305 : Copy previous block (0x100 = 1 byte)
//           0x306         : Out of buffer (?)
static DWORD GetOneByte(TPKWAREStruct * ps)
//     (1502C620)       ECX
{
    DWORD pos;                          // Position in buffers
    DWORD nSkip;                        // Number of bits to skip

    // Test the current bit in byte buffer. If is not set, simply return the next byte.
    if(ps->byteBuffer & 1)                  // (ESI) - ps
    {
        // Skip current bit in the buffer
        if(SkipBits(ps, 1) == TRUE)
            return 0x306;   

        // (EDI) The next bits are position in buffers
        pos = ps->position2[(ps->byteBuffer & 0xFF)];
        
        // (EDX) Get number of bits to skip
        nSkip = ps->nSkip2[pos];

        // EBX - (ps + pos)

        if(SkipBits(ps, nSkip) == TRUE)
            return 0x306;

        if(ps->nBits[pos] != 0)         // (CL)
        {
            // (EBX) - Byte buffer content masked by number of bits
            DWORD value = ps->byteBuffer & ((1 << ps->nBits[pos]) - 1);

            if(SkipBits(ps, ps->nBits[pos]) == TRUE)
            {
                if((pos + value) != 0x10E)
                    return 0x306;
            }
            pos = value + ps->buff3114[pos];
        }
        // 1502C6BC:
        return pos + 0x100;                 // Return number of bytes to repeat
    }

    // 1502C6C8:
    // Skip tested bit
    if(SkipBits(ps, 1) == TRUE)
        return 306;

    // 1502C6DA:
    // If the first block byte is zero, read 8 bits and return them as one byte.
    if(ps->byte1 == 0)
    {
        DWORD oneByte = (ps->byteBuffer & 0xFF);
        if(SkipBits(ps, 8) == TRUE)
            return 0x306;
        return oneByte;
    }

    // 1502C702:
    DWORD ECX = ps->byteBuffer;
    DWORD EAX = ECX & 0xFF;

    assert("This code needs to be tested first !!!");

    if(EAX != 0)
    {
        pos = ps->buff2C34[EAX];
        if(pos == 0xFF)
        {
            if(ECX & 0x3F)
            {
                if(SkipBits(ps, 4) == TRUE)
                    return 0x306;

                pos = ps->buff2D34[ps->byteBuffer & 0xFF];
            }
            else
            {
                if(SkipBits(ps, 6) == TRUE)
                    return 0x306;
                pos = ps->buff2E34[ps->byteBuffer & 0x7F];
            }
        }
    }
    else
    {
        // 1502C77B:
        if(SkipBits(ps, 8) == TRUE)
            return 0x306;
    }
    
    // 1502C7A4:
    pos   = ps->buff2EB4[ps->byteBuffer & 0xFF];
    nSkip = ps->buff2FB4[pos];

    if(SkipBits(ps, nSkip) == TRUE)
        return 0x306;
    return 0;
}

static DWORD __explode_1(TPKWAREStruct * ps)
{
	DWORD oneByte;                      // One byte from compressed file
	DWORD copyBytes;                    // Number of bytes to copy

	ps->outputPos = 0;                  // Initialize output buffer position

    // If end of data or error, terminate decompress
	while((oneByte = GetOneByte(ps)) < 0x305)
	{
        // If one byte is greater than 0x100, means "Repeat n - 0xFE bytes"
        if(oneByte >= 0x100)
		{
            BYTE * source;          // ECX
            BYTE * target;          // EDX
            DWORD  copyLength = oneByte - 0xFE;
            DWORD  moveBack;

            // 1502C56D:
            // Get length of data to copy
            if((moveBack = GetMoveBack(ps, copyLength)) == 0)
				break;

            // Target and source pointer
            // 1502C584:
            target = &ps->outputBuffer[ps->outputPos];
            source = target - moveBack;
			ps->outputPos += copyLength;

			// 1502C598:
            while(copyLength-- > 0)
                *target++ = *source++;
		}
        else
			ps->outputBuffer[ps->outputPos++] = (BYTE)oneByte;
	
		// If number of extracted bytes has reached 1/2 of output buffer,
        // flush output buffer.
        if(ps->outputPos >= 0x1000)
        {
            // Copy decompressed data into user buffer
            copyBytes = 0x1000;
            ps->FillOutputBuffer(ps->outputBuffer, &copyBytes, ps->userData);

            // If there are some data left, keep them alive
            ps->outputPos -= 0x1000;
            memcpy(ps->outputBuffer, &ps->outputBuffer[0x1000], ps->outputPos);
        }
    }

    // 1502C5EF:
	if((copyBytes = ps->outputPos) != 0)
	    ps->FillOutputBuffer(ps->outputBuffer, &copyBytes, ps->userData);
	return 0x306;
}

static void __explode_5(LONG count, BYTE * buffer1, BYTE * table, BYTE * buffer2)
//          (1502C8E0)   ECX         EDX             [ESP+14]        [ESP+18]
{
	for(LONG i = count-1; i >= 0; i--)             // EBX - count
	{
        DWORD idx1 = table[i];
        DWORD idx2 = 1 << buffer1[i];

		do
		{
            buffer2[idx1] = (BYTE)i;
			idx1         += idx2;
        }
        while(idx1 < 0x100);
	}
}

static void __explode_6(TPKWAREStruct * ps, WORD * table)
{
    DWORD  idx1, idx2;                              // EBP, EAX
    WORD   index   = 0xFE;                          // EDX

    for(DWORD i = 0x00FF; i >= 0; i--)
    {
        idx1 = ps->buff2FB4[i];
        if(idx1 <= 8)
        {
            idx2 = table[i];
            idx1 = (1 << idx1);

            do
            {
                ps->buff2C34[idx2] = (char)i;
                idx2 += idx1;
            }
            while(idx2 < 0x100);
        }
        else
        {
            idx2 = table[i] & 0x00FF;
            if(idx2 != 0)
            {
                // idx2 &= 0xFFFF;
                ps->buff2C34[idx2] = (BYTE)0xFF;

                if(table[i] & 0x3F)
                {
                    ps->buff2FB4[index] -= 4;
                    idx1 = (1 << ps->buff2FB4[index]);
                    idx2 = (table[i] >> 4);

                    do
                    {
                        ps->buff2D34[idx2] = (BYTE)i;
                        idx2 += idx1;
                    }
                    while(idx2 < 0x0100);
                }
                else
                {
                    ps->buff2FB4[index] -= 6;
                    idx1 = (1 << ps->buff2FB4[index]);
                    idx2 = (table[i] >> 6);

                    do
                    {
                        ps->buff2E34[idx2] = (BYTE)i;
                        idx2 += idx1;
                    }
                    while(idx2 < 0x0080);
                }
            }
            else
            {
                ps->buff2FB4[index] -= 8;
                idx1 = (1 << ps->buff2FB4[index]);
                idx2 = (table[i] >> 8);

                do
                {
                    ps->buff2EB4[idx2] = (BYTE)i;
                    idx2 += idx1;
                }
                while(idx2 < 0x0100);
            }
        }
    }
}

static DWORD __explode(FILLINPUTBUFF FillInputBuffer, FILLOUTPUTBUFF FillOutputBuffer, TPKWAREStruct * ps, void * userData)
//           (1502C3C0)            ECX                            EDX                              [ESP+10]            [ESP+14]
{
    ps->FillInputBuffer  = FillInputBuffer;
    ps->FillOutputBuffer = FillOutputBuffer;
    ps->userData         = userData;
   
    // Copy source data from decompress info
    ps->inputPos = sizeof(ps->inputBuffer);
    ps->inputBytes = ps->FillInputBuffer(ps->inputBuffer, &ps->inputPos, ps->userData);
    if(ps->inputBytes <= 4)
        return 3;

    // 1502C401:
    ps->byte1      = ps->inputBuffer[0];    // (EAX)
    ps->rBits      = ps->inputBuffer[1];    // (ECX)
    ps->byteBuffer = ps->inputBuffer[2];    // (EDX) Initialize 16-bit work buffer
    ps->bitsExtra  = 0;                     // Number of byteBuffer >> shifts 
    ps->inputPos   = 3;                     // (EDI) Initialize input buffer position

    // 1502C42B:                            // EDX - ps->rBits
    if(4 <= ps->rBits && ps->rBits <= 6) 
    {
        // 1502C440:
        ps->bitMask10 = 0xFFFF >> (0x10 - ps->rBits); // Shifted by 'sar' instruction

        // 1502C44E:
        if(ps->byte1 > 1)
            return 2;
        
        if(ps->byte1 == 1)
        {
                // 1502C465:
                // 0x100 bytes
                memcpy(ps->buff2FB4, Table15034760, sizeof(ps->buff2FB4));
                __explode_6(ps, Table15034860);
        }

        // 1502C481:
        // EDI = &ps->nSkip2
        // 0x10 bytes
        memcpy(ps->nSkip2, Table15034740, sizeof(ps->nSkip2));
        __explode_5(0x10, ps->nSkip2, Table15034750, ps->position2);

        // EDI = &ps->offs30B4;
        // 0x10 bytes
        memcpy(ps->nBits, nBits, sizeof(ps->nBits));

        // 0x10 words
        memcpy(ps->buff3114, Table15034720, sizeof(ps->buff3114));

        // 0x40 bytes
        // EAX = &ps->position1
        memcpy(ps->nSkip1, Table15034690, sizeof(ps->nSkip1));
        __explode_5(0x40, ps->nSkip1, Table150346D0, ps->position1);

        return (__explode_1(ps) == 0x305) ? 0 : 0x0F;
    }
    // 1502C517:
    return 1;
}

/*****************************************************************************/
/*                                                                           */
/*                      Public decompression functions                       */
/*                                                                           */
/*  All functions have the same interface.                                   */
/*                                                                           */
/*   BYTE  * outBuffer  - Buffer to store decompressed output                */
/*   DWORD * pOutLength - Pointer to DWORD contains requested output data    */
/*                        size. This pointer is filled by real decompressed  */
/*                        data size.                                         */
/*   BYTE  * inpBuffer  - Buffer containing input compressed data.           */
/*   DWORD   inpLength  - Input data length                                  */
/*                                                                           */
/*****************************************************************************/

//-----------------------------------------------------------------------------
// void Decompression1(BYTE * outBuffer, DWORD * pOutLength, BYTE * inpBuffer, DWORD inpLength)

void Decompression1(BYTE * outBuffer, DWORD * pOutLength, BYTE * inpBuffer, DWORD inpLength)
//   (15007A80)     ECX            EDX                [ESP+]     [ESP+]
{
    *pOutLength = Call15007AA0(outBuffer, *pOutLength, inpBuffer, inpLength, 1);
}

//-----------------------------------------------------------------------------
// void Decompression2(BYTE * outBuffer, DWORD * pOutLength, BYTE * inpBuffer, DWORD inpLength)

void Decompression2(BYTE * outBuffer, DWORD * pOutLength, BYTE * inpBuffer, DWORD inpLength)
//   (15007CE0)     ECX               EDX               [ESP+]            [ESP+]
{
    *pOutLength = Call15007AA0(outBuffer, *pOutLength, inpBuffer, inpLength, 2);
}

//-----------------------------------------------------------------------------
// void Decompression3(BYTE * outBuffer, DWORD * pOutLength, BYTE * inpBuffer, DWORD inpLength)
// Huffman decompression

void Decompression3(BYTE * outBuffer, DWORD * pOutLength, BYTE * inpBuffer, DWORD inpLength)
//   (15007E90)     ECX               EDX                 [ESP+3A98]        [ESP+3A9C]
//                  0x00DD0078        *0x00D6FF1C         0x00DC1079        0x519
//                                          (0x1000)
{
    TInputStream is;                // [ESP+0C] - Input stream
    THuffTree    htr;               // [ESP+18] - Decompress info structure
//  THTreeItem * item;              // For every use

    InitInputStream(&is, inpBuffer, inpLength);
    InitHuffmanTree(&htr);

    *pOutLength = Call15006CD0(&htr, outBuffer, *pOutLength, &is);

    // EDI = 0;
/*
    // 15007ED9
    while(1)
    {
        THTreeItem * temp1 = item;  // ESI
        THTreeItem * temp2;         // EAX or whatever

        item = htr.last;
        if((LONG)item <= NULL)
            break;
        if(temp1->next == NULL)
            continue;

        temp2 = GetPrevItem(item, -1);

        // Remove item from the chain
        temp2->next = temp1->next;
        temp1->next->prev = temp1->prev;
        temp1->next = NULL;
        temp1->next = NULL;
    }

    // 15007F06
    if((item = htr.first) != NULL)
    {
        if((LONG)ECX < 0)
            ECX = POINTER_COMPLEMENT(ECX);

15007F06   mov         edx,dword ptr [esp+3078h]
15007F0D   cmp         edx,edi
15007F0F   je          15007F48
15007F11   cmp         ecx,edi
15007F13   jge         15007F19
15007F15   not         ecx
15007F17   jmp         15007F27
        
        else
        {
            ESI = EDX->prev;
            EAX = &htr.first - EDX->prev;
            ECX += (&htr.first - EDX->prev);
        }

15007F19   mov         esi,dword ptr [edx+4]
15007F1C   lea         eax,[esp+3078h]
15007F23   sub         eax,esi
15007F25   add         ecx,eax

        ECX->next = EDX;
        ECX = htr.first;
        EDX = htr.last;
        ECX->prev = htr.last;
        htr.first = NULL;
        htr.last = NULL;

15007F27   mov         dword ptr [ecx],edx
15007F29   mov         ecx,dword ptr [esp+3078h]
15007F30   mov         edx,dword ptr [esp+307Ch]
15007F37   mov         dword ptr [ecx+4],edx
15007F3A   mov         dword ptr [esp+3078h],edi
15007F41   mov         dword ptr [esp+307Ch],edi

    }

    for(ECX = htr.offs3058; (LONG)ECX > 0;)
    {

15007F48   mov         ecx,dword ptr [esp+3070h]
15007F4F   cmp         ecx,edi
15007F51   jle         15007F75

        ESI = ECX;
        if(ECX->next == NULL)
            continue;

15007F53   mov         esi,ecx
15007F55   je          15007F75
15007F57   cmp         dword ptr [ecx],edi
15007F59   je          15007F4F

        EAX = GetPrevItem(ECX, -1);

15007F5B   push        0FFh
15007F5D   call        1500E250

        ECX = ESI->next;
        EAX->next = ESI->next;
        EDX = ESI->next;
        EAX = ESI->prev;
        EDX->prev = EAX;
        ESI->next = NULL;
        ESI->prev = NULL;
    }

15007F62   mov         ecx,dword ptr [esi]
15007F64   mov         dword ptr [eax],ecx
15007F66   mov         edx,dword ptr [esi]
15007F68   mov         eax,dword ptr [esi+4]
15007F6B   mov         dword ptr [edx+4],eax
15007F6E   mov         dword ptr [esi],edi
15007F70   mov         dword ptr [esi+4],edi
15007F73   jmp         15007F48

    EDX = htr.offs3054;

    if(EDX != NULL)
    {
15007F75   mov         edx,dword ptr [esp+306Ch]
15007F7C   cmp         edx,edi
15007F7E   je          15007FB7

        if((LONG)ECX < 0)
            ECX = POINTER_COMPLEMENT(ECX);
15007F80   cmp         ecx,edi
15007F82   jge         15007F88
15007F84   not         ecx
15007F86   jmp         15007F96

        else
        {
            ESI = EDX->prev;
            EAX = &htr.offs3054 - EDX->prev;
            ECX += EAX;

15007F88   mov         esi,dword ptr [edx+4]
15007F8B   lea         eax,[esp+306Ch]
15007F92   sub         eax,esi
15007F94   add         ecx,eax
        
        }

        ECX->next = EDX;
        ECX = htr.offs3054;
        EDX = htr.offs3058;
        ECX->prev = htr.offs3058;
        htr.offs3054 = NULL;
        htr.offs3058 = NULL;

15007F96   mov         dword ptr [ecx],edx
15007F98   mov         ecx,dword ptr [esp+306Ch]
15007F9F   mov         edx,dword ptr [esp+3070h]
15007FA6   mov         dword ptr [ecx+4],edx
15007FA9   mov         dword ptr [esp+306Ch],edi
15007FB0   mov         dword ptr [esp+3070h],edi
    
    }

    ESI = &htr.items[0x202];
    EBX = 0x203;


15007FB7   lea         esi,[esp+3068h]
15007FBE   mov         ebx,203h

    do
    {
        EAX = ESI->offs???
        ESI--;
        if(ESI->offs??? = NULL)
            continue;

        EAX = GetPrevItem(ESI, -1);

15007FC3   mov         eax,dword ptr [esi-18h]
15007FC6   sub         esi,18h
15007FC9   cmp         eax,edi
15007FCB   je          15008007
15007FCD   push        0FFh
15007FCF   mov         ecx,esi
15007FD1   call        1500E250

        ECX = ESI->next;
        EAX->next = ESI->next;
        EDX = ESI->next;
        EAX = ESI->prev;
        EDX->prev = EAX;
        ESI->next = NULL;
        ESI->prev = NULL;
        EAX = 0;

15007FD6   mov         ecx,dword ptr [esi]
15007FD8   mov         dword ptr [eax],ecx
15007FDA   mov         edx,dword ptr [esi]
15007FDC   mov         eax,dword ptr [esi+4]
15007FDF   mov         dword ptr [edx+4],eax
15007FE2   mov         eax,edi

        if(EAX == 0)
            continue;

        EAX = GetPrevItem(ESI, -1)

15007FE4   cmp         eax,edi
15007FE6   mov         dword ptr [esi],edi
15007FE8   mov         dword ptr [esi+4],edi
15007FEB   je          15008007
15007FED   push        0FFh
15007FEF   mov         ecx,esi
15007FF1   call        1500E250

        ECX = ESI->next;
        EAX->next = ESI->next;
        EDX = ESI->next;
        EAX = ESI->prev;
        EDX->prev = EAX;
        ESI->next = NULL;
        ESI->prev = NULL;

15007FF6   mov         ecx,dword ptr [esi]
15007FF8   mov         dword ptr [eax],ecx
15007FFA   mov         edx,dword ptr [esi]
15007FFC   mov         eax,dword ptr [esi+4]
15007FFF   mov         dword ptr [edx+4],eax
15008002   mov         dword ptr [esi],edi
15008004   mov         dword ptr [esi+4],edi

    }
    while(--EBX != 0);

15008007   dec         ebx
15008008   jne         15007FC3


1500800A   pop         edi
1500800B   pop         esi
1500800C   pop         ebx
1500800D   add         esp,3A80h
15008013   ret         8
*/
}

//----------------------------------------------------------------------------
// void Explode4(BYTE * outBuffer, DWORD * pOutLength, BYTE * inpBuffer, DWORD inpLength)

void Explode4(BYTE * outBuffer, DWORD * pOutLength, BYTE * inpBuffer, DWORD inpLength)
//   (15007680)     ECX               EDX               EDX               [ESP+3A98]
{
    TPKWAREStruct ps;
    TBufferInfo info;       // Our own buffer information structure

    // Fill decompress information structure and decompress it.
    info.source    = inpBuffer;
    info.sourcePos = 0;
    info.target    = outBuffer;
    info.targetPos = 0;
    info.bytes     = inpLength;

    __explode(CopySourceData, CopyTargetData, &ps, &info);

    if(pOutLength != NULL)
        *pOutLength = info.targetPos;
}

/*****************************************************************************/
/*                                                                           */
/*                       Main decompression function                         */
/*                                                                           */
/* This function is base decompression function of PKWARE Data Compression   */
/* Library. It reads the first byte of the block and decodes which compres-  */
/* sions are applied to this block. There are four decompression types pos-  */
/* sible here. Each bit of the first byte means one decompression type.      */
/* The four decompression are dome by the 'DecompressionX()' functions.      */
/*                                                                           */
/*****************************************************************************/

// 1502B148
// This table contains decompress functions which can be applied to
// compressed blocks. Each bit set means the corresponding
// decompression method/function must be applied.
static TFuncTable fTable2[] =
{
    {0x40, Decompression1},
    {0x80, Decompression2},
    {0x01, Decompression3},               // Huffman decompression
    {0x08, Explode4}
};

//-----------------------------------------------------------------------------
// PKWARE Data Compression Library main decompression function. 

BOOL WINAPI Decompress(BYTE * outBuffer, DWORD * pOutLength, BYTE * inpBuffer, DWORD inpLength)
//   (150081F0)        [ESP+1C],         [ESP+20],           [ESP+24],         [ESP+28]
{
    TFuncTable * ft;
    BYTE * buffer = NULL;               // Temporary storage for decompressed data
    BYTE * work = NULL;
    DWORD  outLength = *pOutLength;     // [ESP+10] For storage number of output bytes
    DWORD  cflags;                      // The first character from input data means compression types
    DWORD  cflags2;                     // Temporary variable
    DWORD  count;                       // [ESP+28] - Counter for every use

    if(inpLength == outLength)
    {
        if(inpBuffer == outBuffer)
            return TRUE;

        memcpy(outBuffer, inpBuffer, inpLength);
        return TRUE;
    }
    
    count  = 0;                         // Number of compressions used
    cflags = *inpBuffer++;              // Get applied compression types
    inpLength--;                        // Decrement input length
    cflags2 = cflags;                   // EDI - Compression flags copy
    
    // Search compression table type and get all types of compression
    for(ft = &fTable2[3]; ft >= fTable2; ft--)
    {
        // We have to apply this decompression ?
        if(cflags & ft->mask)
            count++;

        // Clear this flag from temporary variable and move to prev
        cflags2 &= ~ft->mask;
    }

    // Check if there is some flags remaining
    // (E.g. compressed by future version of PKWARE DCL)
    if(cflags2 != 0)
        return FALSE;

    // 150082B0
    // If there is more than only one compression, we have to allocate extended buffer
    if(count >= 2)
        buffer = new BYTE [outLength];

    // 150082F1
    for(count = 0, ft = &fTable2[3]; ft >= fTable2; ft--)
    {
        // If not used this kind of compression ?
        if((cflags & ft->mask) == 0)
            continue;

        // If odd case, use target buffer for output, otherwise use allocated workbuffer
        work      = (count++ & 1) ? buffer : outBuffer;
        outLength = *pOutLength;

#ifdef _DEBUG
        // Needed for tests
        memset(work, 0, outLength);
#endif
        // 15008332
        // Decompress buffer using corresponding function
        ft->Decompression(work, &outLength, inpBuffer, inpLength);

        // Move output length to src length for next compression
        inpLength = outLength;
        inpBuffer = work;
    }

    // If output buffer is not the same like target buffer, we have to copy data
    if(work != outBuffer)
        memcpy(outBuffer, inpBuffer, outLength);
    *pOutLength = outLength;

    // Delete work buffer, if necessary
    if(buffer != NULL)
        delete buffer;
    return TRUE;
}

