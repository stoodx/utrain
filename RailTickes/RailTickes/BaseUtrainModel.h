#pragma once
#include <ctime>
#include <string>

class HttpClient
{
public:
	HttpClient(const std::wstring& strURL);

	virtual bool SetAdditionalRequestHeaders(const wstring &additionalRequestHeaders) = 0;
};

class CBaseUtrainModel
{
public:
	CBaseUtrainModel(HttpClient** ppHttpClient);
	~CBaseUtrainModel(void);

	//Send HTTP reequest for defining:
	//	GV-TOKEN,
	//	_gv_sessid,
	//	_gv_lang,
	//	HTTPSERVERID
	//strURL: start URL, for example, http://booking.uz.gov.ua 
	//strResponse:  _gv_sessid, _gv_lang, HTTPSERVERID or error
	//return: true - OK (run js decoder), fail - error 
	bool sendRequestForToken(const std::wstring& strURL, std::wstring& strResponse);

private:
	//start time 
	std::time_t m_timeFirstVisit;
	HttpClient** m_ppHttpClient;
};

