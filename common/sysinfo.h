//
#ifndef __INCLUDE_SYSINFO_H
#define __INCLUDE_SYSINFO_H

extern const char *GetMACAddress(bool bPrettyPrint=false);
extern bool BuildMagicCode(const char *user, const char *shop, const char *MAC, char *code);

// return 0 if ok
extern int ComputeCodeCRC(int crc, const char *user=0, const char *shop=0, const char *code=0);

// max size in bytes of numeric coded in a string
#define STRNUMERIC_MAX 128

class NumericCoding {
private:
	static const char *m_gszNull[2];
	static const char *m_gszAlphabet[2];

public:
	// Coding
	static void CodeNumeric( unsigned long lNumeric, char *pszNumeric);

	// set file name for ipconfig results
	static void SetIpConfigFileName(const char *file);

	// Decoding: returns true if the player ID is valid
	//static bool bDecodeNumeric( unsigned long *plNumeric, const char *pszNumeric);
};



#endif