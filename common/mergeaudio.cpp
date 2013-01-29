#include "stdafx.h"
#include "audioheader.h"
#include "mergeaudio.h"
#include <io.h>

//--------------------------------------------------------------------------------------------------------------

static unsigned long _ComputeMask(AudioHeader *pHdr)
{
	unsigned char *p = (unsigned char *)pHdr;
	unsigned long mask=0;
	unsigned long version = pHdr->hdrversion;
	for(int i=0;i<sizeof(AudioHeader);i++)
	{
		unsigned long carry = mask&0x8000 ? 1 : 0;
		mask|=(unsigned long)p[i];
		mask<<=3;
		mask|=carry;
	}
	return mask;
}

//--------------------------------------------------------------------------------------------------------------

static void _EncryptBuffer(unsigned char *buffer, int length, unsigned long mask)
{
	unsigned long masktmp=mask;

	// do pass 1
	for(int i=0;i<length;i++)
	{
		buffer[i]^=(unsigned char)(masktmp&0xFF);
		bool carry = (masktmp&0x80000000)==0x80000000;
		masktmp<<=1;
		if(carry) masktmp|=1;
	}

	// do pass 2
	for(int i=length-1;i>=0;i--)
	{
		buffer[i]^=(unsigned char)(mask&i);
	}
}

//--------------------------------------------------------------------------------------------------------------

static void _DecryptBuffer(unsigned char *buffer, int length, unsigned long mask)
{
	// undo pass 2
	for(int i=length-1;i>=0;i--)
	{
		buffer[i]^=(unsigned char)(mask&i);
	}

	// undo pass 1
	for(int i=0;i<length;i++)
	{
		buffer[i]^=(unsigned char)(mask&0xFF);
		bool carry = (mask&0x80000000)==0x80000000;
		mask<<=1;
		if(carry) mask|=1;
	}
}

//--------------------------------------------------------------------------------------------------------------

static void _EncryptHeader(AudioHeader *pHdr)
{
	unsigned long mask = pHdr->mask;
	unsigned short msgcount = pHdr->msgcount;
	_EncryptBuffer(((unsigned char *)pHdr)+40, sizeof(AudioHeader)-40,pHdr->mask);
	pHdr->mask = mask;
	pHdr->msgcount = msgcount;
}

//--------------------------------------------------------------------------------------------------------------

void DecryptHeader(AudioHeader *pHdr)
{
	if(pHdr->hdrversion<=AUDIO_VERSION_BETAMSG) return;

	unsigned long mask = pHdr->mask;
	unsigned short msgcount = pHdr->msgcount;
	_DecryptBuffer(((unsigned char *)pHdr)+40, sizeof(AudioHeader)-40,pHdr->mask);
	pHdr->mask = mask;
	pHdr->msgcount = msgcount;
}

//--------------------------------------------------------------------------------------------------------------

static void _EncryptMsg(MsgHeader *pHdr, char *msg, unsigned long mask)
{
	_EncryptBuffer((unsigned char*)msg,pHdr->size,mask+pHdr->tick);
}

//--------------------------------------------------------------------------------------------------------------

void DecryptMsg(MsgHeader *pHdr, char *msg, unsigned long mask)
{
	if(pHdr->version<=AUDIO_MSG_BETA1) return;
	_DecryptBuffer((unsigned char*)msg,pHdr->size,mask+pHdr->tick);
}

//--------------------------------------------------------------------------------------------------------------

int _MergeAudio(const char *repfile, const char *oggfile, const char *outputPath, AudioHeader *pHdr, CString& filename,
				MsgHeader *msgs, const CStringArray& msgText)
{
	// do we have a replay file?
	if(repfile[0]==0 || _access(repfile,0)!=0) return -1;

	// do we have an audio file?
	if(oggfile[0]==0 || _access(oggfile,0)!=0) return -1;

	// build final file name
	filename = strrchr(repfile,'\\')+1;
	filename.Replace(".rep","");
	filename = CString("RWA_")+filename;
	filename = filename.Left(32) + CString(".rep");
	CString outPath(outputPath);
	if(outPath.Right(1)!="\\") outPath+="\\";
	filename = outPath + filename;

	// copy replay into new file
	if(!CopyFile(repfile,filename,FALSE)) return -1;

	// open new replay at the end
	FILE *fp=fopen(filename,"ab");
	if(fp==0) return -1;

	// open audio file
	FILE *fsrc = fopen(oggfile,"rb");
	if(fsrc==0) return -2;

	// reset error flags
	clearerr(fp);
	clearerr(fsrc);

	// get file size
	fseek(fsrc,0,SEEK_END);
	long audiosize=ftell(fsrc);
	fseek(fsrc,0,SEEK_SET);
	pHdr->audiosize=audiosize;

	// write header
	pHdr->mask = _ComputeMask(pHdr);
	_EncryptHeader(pHdr);
	fwrite(pHdr,1,sizeof(AudioHeader),fp);

	// write messages
	for(int i=0;i<pHdr->msgcount;i++)
	{
		CString tmp;
		tmp = msgText[i];
		fwrite(&msgs[i],1,sizeof(MsgHeader),fp);
		_EncryptMsg(&msgs[i],(char*)(const char *)tmp,pHdr->mask);
		fwrite((const char *)tmp,1,msgs[i].size,fp);
	}

	// allocate buffer
	unsigned int bufsize=128000;
	char *buffer = new char[bufsize];

	// write audio
	bool error=false;
	for(;;)
	{
		// transfer data
		size_t read = fread(buffer,1,bufsize,fsrc);
		fwrite(buffer,1,read,fp);
		error=ferror(fp)!=0 || ferror(fsrc)!=0;
		if(read<bufsize) break;
	}

	// free bfufer
	delete[]buffer;

	// close files
	fclose(fsrc);
	fclose(fp);

	return error ? -1 : 0;
}
