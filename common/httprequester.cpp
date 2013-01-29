// httprequest"'.cpp : implementation file
//

#include "stdafx.h"
#include <afxinet.h>
#include "httprequester.h"

/*
Status Code Definitions	38

100 Continue	39
101 Switching Protocols	39

200 OK	39
201 Created	39
202 Accepted	40
203 Non-Authoritative Information	40
204 No Content	40
205 Reset Content	40
206 Partial Content	40

300 Multiple Choices	41
301 Moved Permanently	41
302 Found	42
303 See Other	42
304 Not Modified	42
305 Use Proxy	43
306 (Unused)	43
307 Temporary Redirect	43

400 Bad Request	43
401 Unauthorized	43
402 Payment Required	44
403 Forbidden	44
404 Not Found	44
405 Method Not Allowed	44
406 Not Acceptable	44
407 Proxy Authentication Required	44
408 Request Timeout	44
409 Conflict	45
410 Gone	45
411 Length Required	45
412 Precondition Failed	45
413 Request Entity Too Large	45
414 Request-URI Too Long	45
415 Unsupported Media Type	46
416 Requested Range Not Satisfiable	46
417 Expectation Failed	46

500 Internal Server Error	46
501 Not Implemented	46
502 Bad Gateway	46
503 Service Unavailable	46
504 Gateway Timeout	47
505 HTTP Version Not Supported	47
*/

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//------------------------------------------------------------------------------------

void VEHttpRequester::ReadCookies(CInternetSession* /*session*/, const char *headers)
{
	char *cookies = new char[strlen(headers)+1];
	strcpy(cookies,headers);

	char *begin = cookies;
	while(true)
	{
		char *p=strstr(begin,"Set-Cookie");
		if(p==0) break;
		p=strchr(p,':')+1;
		while(*p==' ' && *p!='\r') p++;

		char *pEnd = strstr(p,"\r\n");
		*pEnd=0;

		m_cookies.Add(CString(p));
		//if(m_storeCookies) StoreCookie(session, p); // done by WinInet automatically
		begin = pEnd+2;
	}

	delete[] cookies;
}

//------------------------------------------------------------------------------------

void VEHttpRequester::StoreCookie(CInternetSession* session, char *cookie)
{
	// cookie: name = value
	char *value = strchr(cookie,'=');
	*value=0; value++;
	session->SetCookie(m_url, cookie, value);
}

//------------------------------------------------------------------------------------

static CString MakeHexa(char c)
{
	char str[8+1];
	sprintf(str,"%%%2x",(int)c);
	return CString(str);
}

//------------------------------------------------------------------------------------

void VEHttpRequester::EncodeForm()
{
	// m_form:   name = value <CRLF>
	// m_encodedForm:   name=value&name=value& ... &name=value
	// space -> +
	// a/z A/Z 0/9 stay the same
	// all other chars become %xy where xy is the hexadecimal ascii value
	char *encform = new char[m_form.GetLength()*2];
	strcpy(encform,m_form);

	m_encodedForm = "";
	for(char *p=strtok(encform,"\r\n"); p!=0; p=strtok(0,"\r\n"))
	{
		char *value=strchr(p,'=');
		if(value==0) continue;
		*value=0; value++;
		while(*value==' ') value++;
		if(!m_encodedForm.IsEmpty()) m_encodedForm +="&";
		m_encodedForm += CString(p);
		m_encodedForm += CString("=");
		for(int i=0; i<(int)strlen(value); i++)
		{
			char c= value[i];
			if((c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9'))
				m_encodedForm+=CString(c);
			else if(c==' ')
				m_encodedForm+=CString('+');
			else 
				m_encodedForm+=MakeHexa(c);
		}
	}

	delete[] encform;
}

//------------------------------------------------------------------------------------

// file: path to file that must be uploaded
// fieldname: name of the FILE control in the web page (usually="file")
// form: name=value1<CRLF>name=value2<CRLF>
//
char *VEHttpRequester::CreateDataBlock(const char *file, const char *fieldname, const char *form)
{
	// open file
	FILE * fp=fopen(file,"rb");
	if(fp==0) return 0;

	// get length
	fseek(fp,0,SEEK_END);
	long length=ftell(fp);
	fseek(fp,0,SEEK_SET);

	// allocate boundary
	char boundary[128];
	DWORD now = GetTickCount();
	sprintf(boundary,"---------------------------%x%x",LOWORD(now),HIWORD(now));
	m_boundary = boundary;

	// file header
	CString header;
	header.Format("--%s\r\nContent-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n\r\n", boundary,fieldname,file);

	// allocate data block
	long size = length+header.GetLength()+2;
	char *data = (char*)malloc(size);

	// copy header
	memcpy(data,(const char*)header,header.GetLength());

	// read file
	fread(data+header.GetLength(),length,1,fp);
	long offset = header.GetLength()+length;
	memcpy(data+offset,"\r\n",2);
	offset+=2;

	// close file
	fclose(fp);

	// add other fields
	char *encform = new char[strlen(form)*2];
	strcpy(encform,form);

	// for each field
	for(char *p=strtok(encform,"\r\n"); p!=0; p=strtok(0,"\r\n"))
	{
		// extract name and value
		char *value=strchr(p,'=');
		if(value==0) continue;
		*value=0; value++;
		while(*value==' ') value++;

		// create entry
		CString entry;
		entry.Format("--%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",boundary,p,value);

		// add entry
		size+=entry.GetLength();
		data = (char*)realloc(data,size);
		memcpy(data+offset,(const char *)entry,entry.GetLength());
		offset+=entry.GetLength();
	}

	delete[] encform;

	// add final entry
	CString entry;
	entry.Format("--%s--\r\n",boundary);
	size+=entry.GetLength();
	data = (char*)realloc(data,size);
	memcpy(data+offset,(const char *)entry,entry.GetLength());
	offset+=entry.GetLength();

	m_dataSize = size;
	return data;
}

//------------------------------------------------------------------------------------

// file: path to file that must be uploaded
// fieldname: name of the FILE control in the web page (usually="file")
// form: name=value1<CRLF>name=value2<CRLF>
//
int VEHttpRequester::UploadFile(const char *file, const char *fieldname, const char *form)
{
	// create body
	m_data = CreateDataBlock(file, fieldname, form);
	if(m_data == 0) return -1;

	// send request
	int to = m_timeoutSR;
	if(m_timeoutSR<60) m_timeoutSR = 60;
	int err = RequestURL();
	m_timeoutSR=to;

	// free body
	free(m_data);
	m_data=0;

	return err;
}

//------------------------------------------------------------------------------------

int VEHttpRequester::_SendData(CHttpFile* pFile)
{
	int err=0;

	// send request
	DWORD toBlock = 8000;
	if((DWORD)m_dataSize <= toBlock)
	{
		pFile->SendRequest(0,0,(void*)m_data,m_dataSize);
		if(m_progress) m_progress->NotifyWrite(m_dataSize,m_userparam);
		return 0;
	}

	pFile->SendRequestEx(m_dataSize);
	DWORD toSend = m_dataSize;
	DWORD idx=0;
	DWORD totalsent=0;
	while(toSend>0)
	{
		DWORD snd = min(toSend,toBlock);
		pFile->Write(m_data+idx,snd);
		idx+=snd;
		toSend-=snd;
		if(m_progress) {totalsent+=snd; m_progress->NotifyWrite(totalsent,m_userparam);}
	}

	pFile->EndRequest();

	return err;
}

//------------------------------------------------------------------------------------

// if fp!=0, we write the data read into a file instead of into internal buffer
//
int VEHttpRequester::RequestURL(FILE *fp)
{
	CHttpConnection* pServer = NULL;
	CHttpFile* pFile = NULL;
	int nRetCode =0;

	// create session & set timeouts
	CInternetSession session(m_userAgent);
	session.SetOption(INTERNET_OPTION_RECEIVE_TIMEOUT ,m_timeoutSR*1000);
	session.SetOption(INTERNET_OPTION_SEND_TIMEOUT ,m_timeoutSR*1000);
	session.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT ,m_timeoutC*1000);

	// prepare headers
	char szHeaders[128] = _T("Accept: ");
	strcat(szHeaders,m_accept);
	strcat(szHeaders,"\r\nUser-Agent: ");
	strcat(szHeaders,m_userAgent);
	strcat(szHeaders,"\r\n");
	
	// check to see if this is a reasonable URL
	CString strServerName;
	CString strObject;
	INTERNET_PORT nPort;
	DWORD dwServiceType;
	if (!AfxParseURL(m_url, dwServiceType, strServerName, strObject, nPort) ||
		dwServiceType != INTERNET_SERVICE_HTTP)
	{
		//m_parent->AddMsg("Error: can only use URLs beginning with http://");
		//ThrowSpidermanException(1);
		goto Exit;
	}

	try
	{
		// connect to server
		pServer = session.GetHttpConnection(strServerName, nPort);
		if(pServer==0) goto Exit;

		// open request
		if(m_verb==-1) m_verb = CHttpConnection::HTTP_VERB_GET;
		const char *referer = m_urlReferer.IsEmpty() ? 0 : (const char*)m_urlReferer;
		pFile = pServer->OpenRequest(m_verb,
			strObject, referer, 1, NULL, NULL, 
			INTERNET_FLAG_EXISTING_CONNECT | INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE);
		if(pFile==0) goto Exit;

		// add request headers
		if(!pFile->AddRequestHeaders("Accept-Encoding: identity",HTTP_ADDREQ_FLAG_REPLACE ))
		{
			DWORD err=GetLastError();
		}
		pFile->AddRequestHeaders(szHeaders);

		// if we are posting a form
		if(!m_form.IsEmpty() && m_verb==CHttpConnection::HTTP_VERB_POST)
		{
			// include header for the correct content type
			pFile->AddRequestHeaders("Content-Type: application/x-www-form-urlencoded\r\n");
			
			// encode form data
			EncodeForm();
			
			// add content length
			CString strContentL;
			strContentL.Format("Content-Length: %d\r\n",m_encodedForm.GetLength());
			pFile->AddRequestHeaders(strContentL);

			// send request
			pFile->SendRequest(0,0,(void*)(const char*)m_encodedForm,m_encodedForm.GetLength());
		}
		// if we are uploading a file
		else if(m_data!=0 && m_verb==CHttpConnection::HTTP_VERB_POST)
		{
			// include header for the correct content type
			CString contentType;
			contentType.Format("Content-Type: multipart/form-data; boundary=%s\r\n",(const char*)m_boundary);
			pFile->AddRequestHeaders(contentType);
			
			// add content length
			CString strContentL;
			strContentL.Format("Content-Length: %d\r\n",m_dataSize);
			pFile->AddRequestHeaders(strContentL);
	
			// send request
			_SendData(pFile);
			//pFile->SendRequest(0,0,(void*)m_data,m_dataSize);
		}
		else
		{
			// send request
			pFile->SendRequest();
		}

		// read request headers
		//CString reqhead;
		//pFile->QueryInfo(HTTP_QUERY_FLAG_REQUEST_HEADERS|HTTP_QUERY_COOKIE,reqhead);

		// query status
		pFile->QueryInfoStatusCode(m_HttpStatus);

		// read headers
		pFile->QueryInfo(HTTP_QUERY_RAW_HEADERS_CRLF,m_headers);

		// read cookies
		ReadCookies(&session, m_headers);

		// check content-encoding, must be empty
		CString info;
		//pFile->QueryInfo(HTTP_QUERY_RAW_HEADERS,info);
		pFile->QueryInfo(HTTP_QUERY_CONTENT_ENCODING,info);
		while(!info.IsEmpty())
		{
			pFile->AddRequestHeaders("Accept-Encoding: identity",HTTP_ADDREQ_FLAG_REPLACE );
			pFile->QueryInfo(HTTP_QUERY_CONTENT_ENCODING,info);
			pFile->SendRequest();

			// query status
			pFile->QueryInfoStatusCode(m_HttpStatus);
		}

		// if it ok?
		if (m_HttpStatus != HTTP_STATUS_OK)
		{
			// move on....
			nRetCode = (int)m_HttpStatus;
			goto Exit;
		}

		// allocate buffer
		if(m_buffer==0) m_buffer = new char[m_bufferSize];

		// read html page
		int totalread=0;
		int read;
		if(fp==0) 
		{
			// read chunk by chunk
			while((read = pFile->Read(m_buffer+totalread,m_bufferSize-totalread))>0)
				totalread+=read;
			m_buffer[totalread]=0;

			// return page to caller
			m_htmlCode = m_buffer;
		}
		else
		{
			// read chunck by chunk into local file
			while((read = pFile->Read(m_buffer,m_bufferSize))>0)
			{
				fwrite(m_buffer,1,read,fp);
				totalread+=read;
				if(m_progress) m_progress->NotifyRead(totalread,m_userparam);
			}
			m_htmlCode = "";
		}
	}
	catch (CInternetException* pEx)
	{
		// catch errors from WinINet
		TCHAR szErr[1024];
		pEx->GetErrorMessage(szErr, 1024);
		nRetCode = 2;
		m_WinetError = szErr;
		pEx->Delete();
	}

Exit:
	// close request
	if(pFile) pFile->Close();
	delete pFile;

	// close connection
	if(pServer) pServer->Close();
	delete pServer;

	// close session
	session.Close();

	// return code
	return nRetCode ;
}

//------------------------------------------------------------------------------------
