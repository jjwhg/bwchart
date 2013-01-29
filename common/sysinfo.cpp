//
#include "stdafx.h"
#include <windows.h>
#include <wincon.h>
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <time.h>
#include "sysinfo.h"

static char MACP[17+1];
static char MAC[12+1];
static int nMACSearched = 0;
static int gnCodingMethod = 0;
static char ipconfig[32]="bwrecorder.dta";

#include<Iprtrmib.h>
#include<Iphlpapi.h>

//------------------------------------------------------------------------------------

static const char *_GetAdapterMAC(bool bPrettyPrint)
{
	// get adapter info
	IP_ADAPTER_INFO *pInfo;
	IP_ADAPTER_INFO AdapterInfo[8];
	ULONG size=sizeof(AdapterInfo);
	DWORD err = GetAdaptersInfo(&AdapterInfo[0],  &size);
	pInfo = err == 0 ? &AdapterInfo[0] : 0;

	//char msg[255];
	//sprintf(msg,"%lu need=%lu oneis=%lu",err,size,sizeof(AdapterInfo[0]));
	//if(err!=0) ::MessageBox(0,msg,"error",MB_OK);
	//else ::MessageBox(0,msg,"ok",MB_OK);

	// read all adapters info searching for the right one
	int i=0;
	int pref[8];
	int maxpref=-999,maxidx=-1;
	memset(pref,0,sizeof(pref));
	while(pInfo!=0)
	{
		// build MAC
		sprintf(MAC,"%02X%02X%02X%02X%02X%02X",
			pInfo->Address[0],
			pInfo->Address[1],
			pInfo->Address[2],
			pInfo->Address[3],
			pInfo->Address[4],
			pInfo->Address[5]);

		// count zeros in address
		int zeros=0;
		for(int j=0;j<6;j++)
			if(pInfo->Address[j]==0) zeros++;
		if(zeros>=3) pref[i]-=10;

		// compare to "no adapter" value
		if(strcmp(MAC,"444553540000")==0)
			pref[i]-=5;

		// check ip
		const char *pIP = &pInfo->IpAddressList.IpAddress.String[0];
		if(atoi(pIP)!=0 && atoi(pIP)!=255) pref[i]++;

		// update max
		if(pref[i]>maxpref) {maxpref=pref[i]; maxidx=i;}

		//strcpy(msg,pInfo->AdapterName);
		//strcat(msg,"\r\n");
		//strcat(msg,pIP);
		//strcat(msg,"\r\n");
		//strcat(msg,MAC);
		//::MessageBox(0,msg,"info",MB_OK);

		// try next adapter
		pInfo = pInfo->Next;		
		i++;
	}

	if(maxidx>=0)
	{
		// if we have a valid ip address
		pInfo = &AdapterInfo[maxidx];

		// return MAC
		sprintf(MACP,"%02X-%02X-%02X-%02X-%02X-%02X",
			pInfo->Address[0],
			pInfo->Address[1],
			pInfo->Address[2],
			pInfo->Address[3],
			pInfo->Address[4],
			pInfo->Address[5]);
		sprintf(MAC,"%02X%02X%02X%02X%02X%02X",
			pInfo->Address[0],
			pInfo->Address[1],
			pInfo->Address[2],
			pInfo->Address[3],
			pInfo->Address[4],
			pInfo->Address[5]);

		return bPrettyPrint?MACP:MAC;
	}

	return 0;
}

//------------------------------------------------------------------------------------

// set file name for ipconfig results
void NumericCoding::SetIpConfigFileName(const char *file)
{
	strcpy(ipconfig,file);
}

//------------------------------------------------------------------------------------

// launch a different process
static int _StartProcess(const char *pszExe, const char *pszCmdLine)
{
#ifndef ISMAGIC
	LOG.Print(LOG_DEBUG1,"_StartProcess(%s,%s)",pszExe,pszCmdLine);
#endif

	int nvErr=-1;
	char *pszTmpCmdLine;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

	// init structures
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );

	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_HIDE;

	// make a tmp copy of the command line because CreateProcess will modify it
	pszTmpCmdLine = new char[(strlen(pszExe)+ (pszCmdLine!=0 ? strlen(pszCmdLine) : 0) + 4)];
	if(pszTmpCmdLine == 0) return -1;
	strcpy(pszTmpCmdLine,pszExe);
	if(pszCmdLine!=0) strcat(pszTmpCmdLine," ");
	if(pszCmdLine!=0) strcat(pszTmpCmdLine,pszCmdLine);

	// start process
	if(CreateProcess(NULL,pszTmpCmdLine,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi))
	{
	    // Close process and thread handles. 
		CloseHandle( pi.hProcess );
		CloseHandle( pi.hThread );
		nvErr=0;
	}			  
	else
	{
		nvErr=GetLastError();
		#ifndef ISMAGIC
		LOG.Print(LOG_DEBUG1,"CreateProcess error %lu",GetLastError());
		#endif
	}

	// delete temporary command line
	if(pszTmpCmdLine!=0) delete []pszTmpCmdLine;

	return nvErr;
}

//------------------------------------------------------------------------------------

#define VERS_UNKNOWN 0
#define VERS_WINNT	 1
#define VERS_WIN98	 2
#define VERS_WIN95	 3
#define VERS_WIN31	 4

static int SystemVersion()
{
   OSVERSIONINFOEX osvi;
   BOOL bOsVersionInfoEx;

   // Try calling GetVersionEx using the OSVERSIONINFOEX structure,
   // If that fails, try using the OSVERSIONINFO structure.
   ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
   osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

   if((bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi))==0)
   {
      // If OSVERSIONINFOEX doesn't work, try OSVERSIONINFO.
      osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
      if (! GetVersionEx ( (OSVERSIONINFO *) &osvi) ) 
         return VERS_UNKNOWN;
   }

   switch (osvi.dwPlatformId)
   {
      case VER_PLATFORM_WIN32_NT:
		  return VERS_WINNT;
      case VER_PLATFORM_WIN32_WINDOWS:
         if ((osvi.dwMajorVersion > 4) || 
            ((osvi.dwMajorVersion == 4) && (osvi.dwMinorVersion > 0)))
             return VERS_WIN98;
         else 
			 return VERS_WIN95;
      case VER_PLATFORM_WIN32s:
		  return VERS_WIN31;
   }
   return VERS_UNKNOWN; 
}

//------------------------------------------------------------------------------------

// dump file will be located in same directory than executable
static const char *DumpFileName()
{
	static char DumpFile[255+1];
	if ( GetModuleFileName( NULL, DumpFile, 255 ) != 0) 
	{
		char *p=strrchr(DumpFile,'\\');
		if(p!=0) p[1]=0;
		strcat(DumpFile,ipconfig);
	}
	return DumpFile;
}

//------------------------------------------------------------------------------------

const char *GetMACAddressUsingIPCONFIG(bool bPrettyPrint)
{
	#ifndef ISMAGIC
	LOG.Print(LOG_DEBUG1,"GetMACAddressUsingIPCONFIG");
	#endif

	// try to make sure dump file does not already exist
	#ifndef ISMAGIC
	LOG.Print(LOG_DEBUG1,"pre-remove dump file");
	#endif
	const char *dump = DumpFileName();
	remove(dump);
	if(access(dump,00)==0 && access(dump,06)!=0)
	{
		// if it does, maybe someone is trying to force the MAC address
		// so we simulate failure
		#ifndef ISMAGIC
		LOG.Print(LOG_ERROR,"Dump file is write protected");
		#endif
		return bPrettyPrint?MACP:MAC;
	}

	// execute ipconfig in a hidden command window
	char cmdParam[255];
	sprintf(cmdParam,"/C ipconfig /all > \"%s\"",dump);
	const char *cmdexe = (SystemVersion()==VERS_WINNT) ? "cmd.exe":"command.com";
	_StartProcess(cmdexe, cmdParam);

	// open result file
	FILE * fp = 0;
	for(int i=0; i<3; i++)
	{
		// we try multiple times
		fp = fopen(dump,"rb");
		#ifndef ISMAGIC
		LOG.Print(LOG_DEBUG1,"fp=%x",fp);
		#endif
		if(fp!=0) break;
		Sleep(1000);
	}
	if(fp==0) 
	{
		#ifndef ISMAGIC
		LOG.Print(LOG_DEBUG1,"cant open dump file");
		#endif
		goto Exit;
	}

	for(i=0; i<3; i++)
	{
		// read content
		fseek(fp,SEEK_END,0);
		int size = ftell(fp);
		if(size>0) break;
		Sleep(1000);
	}
	fseek(fp,SEEK_SET,0);

	// read content
	char szBuf[4096+1];
	int read; read=fread(szBuf,1,sizeof(szBuf)-1,fp);
	szBuf[read]=0;

	// close file
	fclose(fp);

	// browse physical addresses
	char *pCur; pCur = szBuf;
	while(true)
	{
		// search for physical address
		char *p=strstr(pCur,"Physical Address");
		if(p==0) p=strstr(pCur,"Adresse physique");
		if(p==0) goto Exit;

		// if found
		#ifndef ISMAGIC
		LOG.Print(LOG_DEBUG1,"found Physical Address");
		#endif

		// skip title
		while(*p!=':' && *p!=0) p++;
		if(*p==0) goto Exit;
		p++;
		// skip blanks
		while(*p==' ') p++;
		if(*p==0) goto Exit;
		//extract MAC address
		strncpy(MACP,p,17);
		MACP[17]=0;
		
		#ifndef ISMAGIC
		LOG.Print(LOG_DEBUG1,"Extracted MAC %s",MACP);
		#endif

		// remove dash if needed
		if(!bPrettyPrint)
		{
			// pair 1
			MAC[0]=MACP[0];
			MAC[1]=MACP[1];
			// pair 2
			MAC[2]=MACP[3];
			MAC[3]=MACP[4];
			// pair 3
			MAC[4]=MACP[6];
			MAC[5]=MACP[7];
			// pair 4
			MAC[6]=MACP[9];
			MAC[7]=MACP[10];
			// pair 5
			MAC[8]=MACP[12];
			MAC[9]=MACP[13];
			// pair 6
			MAC[10]=MACP[15];
			MAC[11]=MACP[16];
			MAC[12]=0;
		}

		// count zeros
		for(int i=0, zeros=0; i<6; i++)
		{
			int val;
			sscanf(&MACP[i*3],"%x",&val);
			if(val==0) zeros++;
		}
				   
		// do we have a reasonable amount of zeros?
		if(zeros<=3) break;

		// if not, keep searching
		pCur = p;
	}

Exit:
	#ifndef ISMAGIC
	LOG.Print(LOG_DEBUG1,"remove dump file");
	#endif
	remove(dump);
	return bPrettyPrint?MACP:MAC;
}

//------------------------------------------------------------------------------------

static const char *__GetMACAddress(bool bPrettyPrint)
{
	//#ifndef ISMAGIC
	//LOG.Print(LOG_DEBUG1,"GetMACAddress");
	//#endif

	if(nMACSearched>1) return bPrettyPrint?MACP:MAC;
	nMACSearched++;

	strcpy(MACP,"00-00-00-00-00-00");
	strcpy(MAC,"000000000000");

	// get adapter info
	const char *macapi = _GetAdapterMAC(bPrettyPrint);
	if(macapi!=0) return macapi;

	return GetMACAddressUsingIPCONFIG(bPrettyPrint);
}

//------------------------------------------------------------------------------------

const char *GetMACAddress(bool bPrettyPrint)
{
	//try once
	const char *mac = __GetMACAddress(bPrettyPrint);
	
	// if mac is empty
	if(strcmp(MAC,"000000000000")==0)
	{
		//wait a bit and try again
		Sleep(1000);
		mac = __GetMACAddress(bPrettyPrint);
	}
	else
		nMACSearched++;

	return mac;
}

//------------------------------------------------------------------------------------

// convert an hexadecimal letter into a digit between 0 and 15
#define VALX(ch) ((ch>='0' && ch<='9') ? (int)ch-(int)'0' : 10 + (int)ch - (int)'A')

// build a magic code from:
// the user name: XXXXXXXXXXX
// the shop name: YYYYYYYYYYY
// and the MAC	: xx-xx-xx-xx-xx-xx or xxxxxxxxxxxx
// Magic code is a 24 letter string. We create one unsigned word for the user name,
// and the shop, and 3 unsigned words for the MAC
//
// return true if we builded a valid code, false if the input was wrong or MAC empty
//
static bool BuildCode(const char *user, const char *shop, const char *MAC, char *code)
{
	if(user==0 || user[0]==0) return false;
	if(shop==0 || shop[0]==0) return false;

	// for newer codings we change the last long and use more of the MAC address
	if(stricmp(shop,"labmusic")!=0 && stricmp(user,"netcast")!=0)
		gnCodingMethod = 1;
	else
		gnCodingMethod = 0;

	// get MAC
	char szMAC[12+1];
	if(MAC!=0)
	{
		if(MAC[2]=='-')
		{
			szMAC[0]=MAC[0];
			szMAC[1]=MAC[1];
			szMAC[2]=MAC[3];
			szMAC[3]=MAC[4];
			szMAC[4]=MAC[6];
			szMAC[5]=MAC[7];
			szMAC[6]=MAC[9];
			szMAC[7]=MAC[10];
			szMAC[8]=MAC[12];
			szMAC[9]=MAC[13];
			szMAC[10]=MAC[15];
			szMAC[11]=MAC[16];
		}
		else
			strncpy(szMAC,MAC,12);
		szMAC[12]=0;
	}
	else
	{
		strcpy(szMAC,GetMACAddress(false));
	}
	if(szMAC[0]==0) return false;

	#ifndef ISMAGIC
	static int once=0;
	if(!once) LOG.Print(LOG_DEBUG1,"BuildCode u=[%s] s=[%s] m=[%s]",user,shop,szMAC);
	once=1;
	#endif

	// use last digits of MAC to build an unsigned long
	unsigned long ul1 = VALX(szMAC[0]);
	ul1 <<= 4; ul1 += VALX(szMAC[7]);
	ul1 <<= 4; ul1 += VALX(szMAC[5]);
	ul1 <<= 4; ul1 += VALX(szMAC[3]);
	ul1 <<= 4; ul1 += VALX(szMAC[10]);
	ul1 <<= 4; ul1 += VALX(szMAC[2]);
	ul1 <<= 4; ul1 += VALX(szMAC[6]);
	ul1 <<= 4; ul1 += VALX(szMAC[1]);

	// use last digits of MAC to build an unsigned long
	unsigned long ul2 = VALX(szMAC[11]);
	ul2 <<= 4; ul2 += VALX(szMAC[9]);
	ul2 <<= 4; ul2 += VALX(szMAC[4]);
	ul2 <<= 4; ul2 += VALX(szMAC[8]);
	ul2 <<= 4; ul2 += VALX(user[0]);
	ul2 <<= 4; ul2 += VALX(shop[0]);
	ul2 <<= 4; ul2 += VALX(user[1]);
	ul2 <<= 4; ul2 += VALX(shop[1]);

	// use letters of user and shop  to build an unsigned long
	unsigned long val1=0UL;
	for(int i=0;i<(int)strlen(user); i++, val1+=(unsigned long)toupper(user[i]), val1<<=1);
	unsigned long val2=0UL;
	for(i=0;i<(int)strlen(shop); i++, val2+=(unsigned long)toupper(shop[i]), val2<<=1);
	unsigned long ul3 = (val1<<16) + (val2%0x00FF);

	// for newer codings we change the last long and use more of the MAC address
	if(gnCodingMethod==1)
	{
		unsigned short word = (unsigned short)(VALX(szMAC[2]));
		word <<= 4; word = (unsigned short)(word + (unsigned short)(VALX(szMAC[9])));
		word <<= 4; word = (unsigned short)(word + (unsigned short)(VALX(szMAC[10])));
		word <<= 4; word = (unsigned short)(word + (unsigned short)(VALX(szMAC[11])));

		ul3 &= 0xFFFF0000;
		ul3 += word;
	}

	//#ifndef ISMAGIC
	//LOG.Print(LOG_DEBUG1,"ul1=%lu ul2=%lu ul3=%lu",ul1,ul2,ul3);
	//#endif

	// create some noise
	ul1+= ul3;
	ul2-= ul3;

	//#ifndef ISMAGIC
	//LOG.Print(LOG_DEBUG1,"ul1=%lu ul2=%lu ul3=%lu",ul1,ul2,ul3);
	//#endif

	// convert longs to strings
	NumericCoding::CodeNumeric(ul1, code);
	NumericCoding::CodeNumeric(ul2, code+8);
	NumericCoding::CodeNumeric(ul3, code+16);
	code[24]=0;

	return true;
}

//------------------------------------------------------------------------------------

// return 0 if ok
int ComputeCodeCRC(int crc, const char *user, const char *shop, const char *code)
{
	int res=0;

	CString strUser = AfxGetApp()->GetProfileString("LOGIN","USER","NETCAST");
	if(user==0) user = strUser;
	CString strShop = AfxGetApp()->GetProfileString("LOGIN","SHOP","LABMUSIC");
	if(shop==0) shop = strShop;
	
	CString AllCode;
	CString strCode = AfxGetApp()->GetProfileString("LOGIN","CODE1","");
	AllCode=AllCode+strCode.Left(4);
	strCode = AfxGetApp()->GetProfileString("LOGIN","CODE2","");
	AllCode=AllCode+strCode.Left(4);
	strCode = AfxGetApp()->GetProfileString("LOGIN","CODE3","");
	AllCode=AllCode+strCode.Left(4);
	strCode = AfxGetApp()->GetProfileString("LOGIN","CODE4","");
	AllCode=AllCode+strCode.Left(4);
	strCode = AfxGetApp()->GetProfileString("LOGIN","CODE5","");
	AllCode=AllCode+strCode.Left(4);
	strCode = AfxGetApp()->GetProfileString("LOGIN","CODE6","");
	AllCode=AllCode+strCode.Left(4);
	if(code==0) code = AllCode;

	//#ifndef ISMAGIC
	//LOG.Print(LOG_DEBUG1,"current code [%s]",(const char*)AllCode);
	//#endif

	// build magic code
	char magic[24+1];
	if(!BuildCode(user, shop, 0, magic)) res += 4;

	//#ifndef ISMAGIC
	//LOG.Print(LOG_DEBUG1,"magic code [%s]",magic);
	//#endif

	// compare with the one entered by user
	res += strcmp(code,magic);

	return res==0 ? res+crc : res+crc%8;
}

//------------------------------------------------------------------------------------

bool BuildMagicCode(const char *user, const char *shop, const char *MAC, char *code)
{
	return BuildCode(user, shop, MAC, code);
}

//------------------------------------------------------------------------------------

// !! THOSE 2 ALPHABETS (STRINGS) MUST NOT SHARE ANY LETTER !!
const char *NumericCoding::m_gszNull[2] = {
	"ZA9M",
	"4AO0"
};
const char *NumericCoding::m_gszAlphabet[2] = {
	"RSTUFGHVXY86BCW012KL7NOPQ34DEIJ5",
	"VX3MDEIJ5RS91Y86BCWQ2KL7NZPTUFGH"
};

// Coding: pszNumeric is a 8 char string. For each 5 bits, we pick a char from the 
// 2^5=32 letters alphabet and place it in the string. We add an 8th char with a checksum.
// For the value 0, we use a different alphabet and place random chars and a random crc.
//
void NumericCoding::CodeNumeric( unsigned long dwNumeric, char *pszNumeric)
{
	unsigned long dwOffset = 0;
	unsigned long dwID;
	unsigned long dwValBase32;
	int i,nChecksum=0;

	// is it a null player ID?
	if( dwNumeric == 0)
	{
		//null player ID are coded with a different alphabet
		srand(GetTickCount());
		for(i=0; i<7; i++) pszNumeric[i]=m_gszNull[gnCodingMethod][rand()%(strlen(m_gszNull[gnCodingMethod]))];
		sprintf(&pszNumeric[7],"%d",rand()%2000);
	}
	else
	{
		// normal player ID
		dwID = dwNumeric;
		for(i=0; i<7; i++)
		{
			dwValBase32 = dwID&0x1F;
			dwID >>= 5;
			pszNumeric[i]=m_gszAlphabet[gnCodingMethod][(dwOffset+dwValBase32)%32];
			nChecksum += (int)i*pszNumeric[i];
			dwOffset += i;
		}
		sprintf(&pszNumeric[7],"%d",nChecksum);
	}
}

//------------------------------------------------------------------------------------

// Decoding: returns true if the player ID is valid
//
/*
bool NumericCoding::bDecodeNumeric( unsigned long *pdwNumeric, const char *pszNumeric)
{
	unsigned long dwOffset = 0;
	unsigned long dwID;
	unsigned long dwValBase32;
	int i,nChecksum=0;
	bool bValid=true;

	// is it long enough?
	if(strlen(pszNumeric)<8) bValid=false;

	// does it belong to one of our alphabets?
	bool bAllNull=true;
	bool bAllNonNull=true;
	for(i=0; i<7; i++) 
	{
		if(strchr(m_gszNull,pszNumeric[i])==0) bAllNull=false;
		if(strchr(m_gszAlphabet,pszNumeric[i])==0) bAllNonNull=false;
	}
	if(!bAllNull && !bAllNonNull) bValid=false;
	if(bAllNull && bAllNonNull) bValid=false;

	// if it is not a null value
	dwID = 0;
	if(!bAllNull)
	{
		for(i=0, dwOffset=0; i<7; i++) {dwOffset += i;}
		for(i=0; i<7 && bValid; i++)
		{
			char *p=strchr(m_gszAlphabet,pszNumeric[7-i-1]);
			if(p==0) {bValid=false; break;}
			dwValBase32 = (unsigned long)(p-&m_gszAlphabet[0]);
			dwOffset -= 7-i-1;
			nChecksum += (7-i-1)*(int)pszNumeric[7-i-1];
			dwValBase32 = (dwValBase32+32-dwOffset)%32;
			dwID <<= 5;
			dwID += dwValBase32;
		}
		if(nChecksum!=atoi(&pszNumeric[7])) bValid=false;
	}
	*pdwNumeric = dwID;

	if(!bValid)
	{
		// INVALID PLAYER ID
		ASSERT(false);
		*pdwNumeric=0;
	}

	return bValid;
}
*/

//------------------------------------------------------------------------------------
