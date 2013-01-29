#ifndef __HTTPREQUESTER_H
#define __HTTPREQUESTER_H

class INotifyProgress
{
public:
	virtual void NotifyRead(unsigned long current, unsigned long userparam)=0;
	virtual bool NotifyWrite(unsigned long current, unsigned long userparam)=0;
};

class VEHttpRequester 
{
private:

	void ReadCookies(class CInternetSession* session, const char *headers);
	void StoreCookie(class CInternetSession* session, char *cookie);
	void EncodeForm();
	char *CreateDataBlock(const char *file, const char *fieldname, const char *form);
	int _SendData(CHttpFile* pFile);

	// inputs
	CString m_url;
	CString m_urlReferer;
	CString m_userAgent;
	CString m_form;
	CString m_accept;
	int m_timeoutC;  // seconds
	int m_timeoutSR; // seconds
	int m_verb;

	// outputs
	CStringArray m_cookies;
	CString m_htmlCode;
	CString m_headers;
	DWORD m_HttpStatus;
	CString m_WinetError;

	// internal stuff
	char *m_buffer;
	char *m_data;
	long m_dataSize;
	int m_bufferSize;
	CString m_encodedForm;
	CString m_boundary;

	// progress interface
	INotifyProgress *m_progress;
	unsigned long m_userparam;

public:
	VEHttpRequester() : m_progress(0), m_HttpStatus(0), m_bufferSize(128000), m_timeoutC(10), 
		m_timeoutSR(10), m_userAgent("VEHttpRequester"), m_buffer(0), m_verb(-1),
	    m_data(0), m_accept("text/*") {}
	~VEHttpRequester() {delete m_buffer;}
	int RequestURL(FILE *fp=0);
	int UploadFile(const char *file, const char *fieldname, const char *form);

	void SetUrl(const char *str, int verb=-1) {m_url = str; if(verb>=0) m_verb=verb;}
	void SetForm(const char *str) {m_form = str;}
	void SetHtmlCode(const char *str) {m_htmlCode = str;}
	void SetAccept(const char *str) {m_accept = str;}
	void SetReferrer(const char *str) {m_urlReferer = str;}
	void SetUserAgent(const char *str) {m_userAgent = str;}
	void SetTimout(int tc, int tsr) {m_timeoutC=tc; m_timeoutSR=tsr;}

	// progress interface
	void SetProgress(INotifyProgress *progress, unsigned long userp=0) {m_progress=progress; m_userparam=userp;}
	
	const char *GetHtmlCode() const {return m_htmlCode;}
	const char *GetReferrer() const {return m_urlReferer;}
	const char *GetUserAgent() const {return m_userAgent;}
	const char *GetHeaders() const {return m_headers;}
	const CStringArray& GetCookies() const {return m_cookies;}
	int GetTimeOutC() const {return m_timeoutC;}
	int GetTimeOutSR() const {return m_timeoutSR;}
	DWORD GetHttpStatus() const {return m_HttpStatus;}
	const char *GetWInetError() const {return m_WinetError;}
};

#endif