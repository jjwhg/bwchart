#ifndef _unpack_h
#define _unpack_h

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#define min(a, b)  (((a) < (b)) ? (a) : (b)) 
/* #define max(a,b) ((a)>(b)?(a):(b)) */
                                                                                
#define SEC_HDR 0x00000001
#define SEC_MAP 0x00000002
#define SEC_CMD 0x00000004
#define SEC_ALL 0x00000007
                                                                                                                
typedef unsigned char byte;
typedef unsigned short int word;
typedef unsigned int dword;

typedef byte hdr_t;				/* nearly complete header information available */
typedef byte cmd_t;				/* no data definition yet */
typedef byte map_t;				/* uncomplete data definition yet */

typedef struct replay_dec_s {
    int             hdr_size;
    hdr_t           *hdr;
    int             cmd_size;
    cmd_t           *cmd;
    int             map_size;
    map_t           *map;
} replay_dec_t;

/* function prototypes */
/* int replay_pack(replay_dec_t *replay, const char *path); */
void replay_unpack(replay_dec_t *replay, const char *path, int sections);

#endif /* _unpack_h */
