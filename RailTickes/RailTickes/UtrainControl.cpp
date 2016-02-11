#include "stdafx.h"
#include "UtrainControl.h"
#include <string>
#include "WinHttpClient.h"


CUtrainHttpClient::CUtrainHttpClient(const std::wstring& strURL) 
	: HttpClient(strURL)
	, m_pWinHttpClient(NULL)
{
	m_pWinHttpClient = new WinHttpClient(strURL);
	ASSERT(m_pWinHttpClient);
}

CUtrainHttpClient::~CUtrainHttpClient()
{
	if (m_pWinHttpClient)
		delete m_pWinHttpClient;
}


bool CUtrainHttpClient::SetAdditionalRequestHeaders(const std::wstring &additionalRequestHeaders)
{
	return m_pWinHttpClient->SetAdditionalRequestHeaders(additionalRequestHeaders);
}

bool CUtrainHttpClient::SendHttpRequest(const std::wstring &httpVerb, bool disableAutoRedirect, bool securityConnection)
{
	return m_pWinHttpClient->SendHttpRequest(httpVerb, disableAutoRedirect, securityConnection);
}

std::wstring CUtrainHttpClient::GetResponseStatusCode(void)
{
	return m_pWinHttpClient->GetResponseStatusCode();
}

int CUtrainHttpClient::GetLastError(void)
{
	return m_pWinHttpClient->GetLastError();
}

std::wstring CUtrainHttpClient::GetResponseContent(void)
{
	return m_pWinHttpClient->GetResponseContent();
}

std::wstring CUtrainHttpClient::GetResponseCookies(void)
{
	return m_pWinHttpClient->GetResponseCookies();
}



//////////////////////////////
CUtrainControl::CUtrainControl(void)
{
}


CUtrainControl::~CUtrainControl(void)
{
}



std::string CUtrainControl::convUTF16toUTF8(const std::wstring strUTF16)
{
   string strUTF8;
   int len = WideCharToMultiByte(CP_UTF8, 0, strUTF16.c_str(), -1, NULL, 0, 0, 0);
   if (len>1)
   { 
      char *ptr = new char[len + 1];
	  memset(ptr, 0, len+ 1);
      if (ptr) 
		  WideCharToMultiByte(CP_UTF8, 0, strUTF16.c_str(), -1, ptr, len, 0, 0);
	  strUTF8 = ptr;
	  delete [] ptr;
   }
   return strUTF8;

}

HttpClient* CUtrainControl::createHttpClient(const std::wstring& strURL)
{
	CUtrainHttpClient* pClient = NULL;
	if (strURL.empty())
		return NULL;
	pClient = new CUtrainHttpClient(strURL);
	if (pClient == NULL)
		return NULL;
	return (HttpClient*)pClient;
}