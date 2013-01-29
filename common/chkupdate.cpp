// chkupdate.cpp : implementation file
//

#include "stdafx.h"
#include "chkupdate.h"
#include "httprequester.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//--------------------------------------------------------------------------------------------------------------

void Updater::CheckForUpdateBW(bool automatic)
{
	_CheckForUpdate("BWChart","download bwchart", automatic);
}

//--------------------------------------------------------------------------------------------------------------

void Updater::CheckForUpdateW3(bool automatic)
{
	_CheckForUpdate("W3Chart","download w3chart", automatic);
}

//--------------------------------------------------------------------------------------------------------------

void Updater::CheckForUpdateBWCoach(bool automatic)
{
	_CheckForUpdate("BWCoach","download bwcoach", automatic);
}

//--------------------------------------------------------------------------------------------------------------

void Updater::_CheckForUpdate(const char *tool, const char *tag, bool automatic)
{
	// dont do an automatic check if the user already did a manual check
	static bool manualCheckDone=false;
	if(manualCheckDone && automatic) return;

	// dont auto-check twice the same day
	if(automatic)
	{
		CTime now = CTime::GetCurrentTime();
		int lastcheck = AfxGetApp()->GetProfileInt("UPDATE","TODAY",-1);
		if(lastcheck==now.GetDay()) return;
	}

	if(!automatic) manualCheckDone=true;

	// download version summary
	CString msg;
	CString result;
	VEHttpRequester req;	
	bool needUpdate=false;
	req.SetUrl("http://bwchart.teamliquid.net/download.js");

	// user agent
	CString userAgent;
	userAgent.Format("%s %s",tool,(const char*)m_version);
	req.SetUserAgent(userAgent);

	// request url
	CWaitCursor wait;
	req.SetTimout(3, 3);
	int err = req.RequestURL();
	if(err==0)
	{
		// look for bwchart version tag
		result = req.GetHtmlCode();
		result.MakeLower();
		char *version = (char*)strstr(result,tag);
		if(version==0)
		{
			// cant find tag
			msg.Format("Unable to check updates. Please try later");
		}
		else
		{
			// extract version
			version+=strlen(tag)+1;
			version[5]=0;

			//compare with our version
			if(_stricmp(version,m_version)<0)
				msg.Format("Your version (%s) is more recent than the one on the server (%s).\r\nYou don't need to update.",(const char *)m_version,version);
			else if(_stricmp(version,m_version)>0)
			{
				needUpdate=true;
				msg.Format("A newer version (%s) is available on the server.\r\nDo you want to visit www.bwchart.com for download?",version);
			}
			else
				msg.Format("Your version (%s) is up to date.\r\nThere is no need to download another version.",(const char *)m_version);

			// save current day to avoid rechecking today
			CTime now = CTime::GetCurrentTime();
			AfxGetApp()->WriteProfileInt("UPDATE","TODAY",now.GetDay());
		}
	}
	else
	{
		msg.Format("Unable to reach %s server. Please try later. (err=%d)",tool,req.GetHttpStatus());
	}

	// display result
	if(!automatic || needUpdate) 
	{
		if(needUpdate) 
		{
			if(::MessageBox(0,msg,"Update Available",MB_YESNO)==IDYES)
			{
				ShellExecute(0,"open","http://www.bwchart.com","","",SW_SHOW);
				AfxGetMainWnd()->DestroyWindow();
			}
		}
		else 
			::MessageBox(0,msg,err==0?"Result":"Error",MB_OK|(err==0?0:MB_ICONEXCLAMATION));
	}
}

//--------------------------------------------------------------------------------------------------------------

