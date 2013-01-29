#ifndef __chkupdate_h
#define __chkupdate_h

class Updater
{
private:
	CString m_version;
	void _CheckForUpdate(const char *tool, const char *tag, bool automatic);

public:
	Updater(const char *version) : m_version(version) {}
	void CheckForUpdateBW(bool automatic);
	void CheckForUpdateW3(bool automatic);
	void CheckForUpdateBWCoach(bool automatic);
};

#endif
