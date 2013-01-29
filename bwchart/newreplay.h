#ifndef __NEWREPLAY_H
#define __NEWREPLAY_H

class NewReplayNotify
{
public:
	NewReplayNotify();
	~NewReplayNotify() {if(m_hf!=0) CloseHandle(m_hf);}

	bool Start();
	bool CollectReplay();
	const char *GetReplay() const {return m_repfile;}
private:
	HANDLE m_hf;
	CString m_repfile;

	void _BrowseReplays(const char *dir, int &idx);
};

#endif
