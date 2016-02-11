#pragma once
#include "baseutrainmodel.h"

class WinHttpClient;

class CUtrainHttpClient :
	public HttpClient
{
public:
	CUtrainHttpClient(const std::wstring& strURL);
	~CUtrainHttpClient();

	bool SetAdditionalRequestHeaders(const std::wstring &additionalRequestHeaders);
	bool SendHttpRequest(const std::wstring &httpVerb = L"GET", bool disableAutoRedirect = false, bool securityConnection = false);
	std::wstring GetResponseStatusCode(void);
	int GetLastError(void);
	std::wstring GetResponseContent(void);
	std::wstring GetResponseCookies(void);

private:
	WinHttpClient* m_pWinHttpClient;
};

class CUtrainControl :
	public CBaseUtrainModel
{
public:
	CUtrainControl(void);
	~CUtrainControl(void);

	 std::string convUTF16toUTF8(const std::wstring strUTF16);
	 HttpClient* createHttpClient(const std::wstring& strURL);
};

