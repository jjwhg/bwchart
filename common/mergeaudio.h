#ifndef __mergeaudio_h
#define __mergeaudio_h

// message to change BWPlayer version of BW patch
#define WM_BWPLAYERVERSION WM_USER+124

int _MergeAudio(const char *repfile, const char *oggfile, const char *outputPath, struct AudioHeader *pHdr, 
				CString& filename, MsgHeader *msgs, const CStringArray& msgText);

void DecryptHeader(struct AudioHeader *pHdr);
void DecryptMsg(struct MsgHeader *pHdr, char *msg, unsigned long mask);

#endif
