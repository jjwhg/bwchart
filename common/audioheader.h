
#ifndef __audioheader_h
#define __audioheader_h

#define BWAUDIOMARKER "BWRecorder"

enum {AUDH_FLAG_ISREPLAY=1,AUDH_FLAG_ISGAME=2};
enum {AUDIO_VERSION_BETA1=0,AUDIO_VERSION_BETAMSG=2, AUDIO_VERSION_BADENCRYPT=3,AUDIO_HEADER_VERSION_LAST}; // AUDIO_VERSION_ENCRYPTED
enum {AUDIO_MSG_BETA1=0,AUDIO_MSG_VERSION_LAST};
;
#pragma pack(1)
struct AudioHeader
{
	unsigned long hdrsize;
	unsigned long hdrversion;
	char header[32];
	char date[9];
	char author[64];
	char language[32];
	char comment[512];
	unsigned long audiosize;
	unsigned long durationS;
	unsigned long starttick;
	unsigned long flags;
	unsigned short msgcount;
	unsigned short _reserved;
	char MAC[12];
	unsigned long mask;
	unsigned long reserved[24];
};
#pragma pack()

#pragma pack(1)
struct MsgHeader
{
	unsigned short version;
	unsigned long tick;
	unsigned char size;
	unsigned long reserved[4];
};
#pragma pack()

#endif