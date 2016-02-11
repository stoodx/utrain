#pragma once
#include <ctime>
#include <string>

extern "C" {
#include "duktape\duktape.h"
}

class HttpClient
{
public:
	HttpClient(const std::wstring& strURL) {}
	virtual ~HttpClient() {}

	virtual bool SetAdditionalRequestHeaders(const std::wstring &additionalRequestHeaders) = 0;
	virtual bool SendHttpRequest(const std::wstring &httpVerb = L"GET", bool disableAutoRedirect = false, bool securityConnection = false) = 0;
	virtual std::wstring GetResponseStatusCode(void) = 0;
	virtual int GetLastError(void) = 0;
	virtual std::wstring GetResponseContent(void) = 0;
	virtual std::wstring GetResponseCookies(void) = 0;
};

class CBaseUtrainModel
{
public:
	CBaseUtrainModel(void);
	~CBaseUtrainModel(void);

	//Send HTTP request for defining:
	//	GV-TOKEN,
	//	_gv_sessid,
	//	_gv_lang,
	//	HTTPSERVERID
	//strURL: start URL, for example, http://booking.uz.gov.ua 
	//strResponse:  _gv_sessid, _gv_lang, HTTPSERVERID or error
	//return: true - OK (run js decoder), fail - error 
	bool sendRequestForToken(const std::wstring& strURL, std::wstring& strResponse);

	//Unicode convertion 
	//we must use localization functions of OS for it.
	//stl library didn't work for Ukraine and Russian
	virtual std::string convUTF16toUTF8(const std::wstring strUTF16) = 0;

	//factory of  HttpClient
	//strURL: url access
	//return: 
	// - NULL, if it's fail 
	// - HttpClient object, if it's true
	virtual HttpClient* createHttpClient(const std::wstring& strURL) = 0;

	//result of  js run
	static duk_ret_t get_result_token(duk_context *ctx);
	
	//class object for get_result_token()
	static CBaseUtrainModel* m_pCBaseUtrainModel;

private:
	//start time 
	std::time_t m_timeFirstVisit;

	//return size od string
	int getSize(const std::wstring str);

	//session token
	std::wstring  m_strToken;

	//convertation to UTF-16
	std::wstring convUTF8toUTF16(const std::string& str);

};

