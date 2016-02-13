#include "BaseUtrainModel.h"

#define PUSH_C_FUNCTION(fcn, nargs) \
	duk_push_c_function(ctx, fcn, nargs);\
	duk_put_prop_string(ctx, -2, #fcn);


CBaseUtrainModel* CBaseUtrainModel::m_pCBaseUtrainModel = NULL;

CBaseUtrainModel::CBaseUtrainModel(void)
	: m_strToken(L"")
	, m_strError(L"")
	, m_strResponseCookies(L"")
	, m_nVisitBooking(0)
{
	m_timeFirstVisit = time(nullptr);
	m_pCBaseUtrainModel = (CBaseUtrainModel*)this;
}


CBaseUtrainModel::~CBaseUtrainModel(void)
{
}


bool CBaseUtrainModel::sendRequestForToken(const std::wstring& strURL, std::wstring& strResponse)
{
	m_strError = L"";
	m_strResponseCookies = L"";
	HttpClient*  pHttpClient = createHttpClient(strURL);
	if (!pHttpClient)
	{
		strResponse = L"No HttpClient object";
		m_strError = strResponse;
		return false;
	}
	strResponse = L"";
	// Set request headers.
	std::wstring strHeaders = L"Content-Length: ";
	strHeaders += L"0";
	strHeaders += L"\r\nContent-Type: binary/octet-stream\r\n";
	pHttpClient->SetAdditionalRequestHeaders(strHeaders);

	// Send http post request.
	if (!pHttpClient->SendHttpRequest(L"Get"))
	{
		strResponse = L"Error sending: ";
		strResponse.append(std::to_wstring(pHttpClient->GetLastError()));
		delete pHttpClient;
		m_strError = strResponse;
		return false;
	}

	std::wstring str_httpResponseCode = pHttpClient->GetResponseStatusCode();
	std::wstring str_httpResponseContent = pHttpClient->GetResponseContent();
	std::wstring str_httpResponseCookies = pHttpClient->GetResponseCookies();
	delete pHttpClient;

	if (str_httpResponseCode.compare(L"200"))
	{
		strResponse = L"Error sending: ";
		strResponse.append(str_httpResponseCode);
		m_strError = strResponse;
		return false;
	}
	if (str_httpResponseContent.empty())
	{
		strResponse =  L"No response";
		m_strError = strResponse;
		return false;
	}

	int nIndex;
	nIndex = str_httpResponseContent.find(L"gaq.push(['_trackPageview']);");
	if (nIndex == std::wstring::npos)
	{
		strResponse = L"Token: bad format";
		m_strError = strResponse;
		return false;
	}
	nIndex += getSize(L"gaq.push(['_trackPageview']);");
	str_httpResponseContent = str_httpResponseContent.substr(nIndex);
	nIndex = str_httpResponseContent.find(L"(function ()");
	if (nIndex == std::wstring::npos)
	{
		strResponse = L"Token: bad format";
		m_strError = strResponse;
		return false;
	}
	str_httpResponseContent = str_httpResponseContent.substr(0, nIndex);
	std::string strToken = convUTF16toUTF8(str_httpResponseContent);

	//gv-token
	duk_context* ctx = duk_create_heap_default();
	if(!ctx)
	{
		strResponse = L"HEAP_CREATION_ERROR";
		m_strError = strResponse;
		return false;
	}
	duk_push_global_object(ctx);
	PUSH_C_FUNCTION(get_result_token, 1);
	//run js
	if (duk_peval_file(ctx, "jjdecode.js") == 0)
	{
		duk_pop(ctx);
		duk_get_prop_string(ctx, -1, "jjdecode");
		duk_push_string(ctx, strToken.c_str());
 		if (duk_pcall(ctx, 1) != 0)
		{
			strResponse = convUTF8toUTF16(duk_safe_to_string(ctx, -1));
			duk_pop(ctx);
			duk_destroy_heap(ctx);
			m_strError = strResponse;
			return false;
		}
		duk_pop(ctx);
	}
	else
	{
		strResponse = convUTF8toUTF16(duk_safe_to_string(ctx, -1));
		duk_pop(ctx);
		duk_destroy_heap(ctx);
		m_strError = strResponse;
		return false;
	}

	duk_pop(ctx);
	duk_destroy_heap(ctx);

	//cookies
	//_gv_sessid
	nIndex = str_httpResponseCookies.find(L"_gv_sessid");
	int nLen = str_httpResponseCookies.size();
	std::wstring strTmp;
	int i;
	wchar_t c;
	if (nIndex == std::wstring::npos)
	{
		strResponse = L"No _gv_sessid";
		m_strError = strResponse;
		return false;
	}
	for(i = nIndex; i < nLen; i++)
	{
		c = str_httpResponseCookies.at(i);
		strResponse += c;
		if (c == L';')
			break;
	}
	strResponse += L' ';
	//_gv_lang
	nIndex = str_httpResponseCookies.find(L"_gv_lang");
	if (nIndex == std::wstring::npos)
	{
		strResponse = L"No _gv_lang";
		m_strError = strResponse;
		return false;
	}
	for(i = nIndex; i < nLen; i++)
	{
		c = str_httpResponseCookies.at(i);
		strResponse += c;
		if (c == L';')
			break;
	}
	strResponse += L' ';
	//HTTPSERVERID
	nIndex = str_httpResponseCookies.find(L"HTTPSERVERID");
	if (nIndex == std::wstring::npos)
	{
		strResponse = L"No HTTPSERVERID";
		m_strError = strResponse;
		return false;
	}
	for(i = nIndex; i < nLen; i++)
	{
		c = str_httpResponseCookies.at(i);
		strResponse += c;
		if (c == L';')
			break;
	}
	strResponse += L' ';
	m_strResponseCookies = strResponse;
	return true;
}



int CBaseUtrainModel::getSize(const std::wstring str)
{
	if (str.empty())
		return 0;
	return str.size();
}

duk_ret_t CBaseUtrainModel::get_result_token (duk_context *ctx)
{
	if (m_pCBaseUtrainModel)
	{
		m_pCBaseUtrainModel->m_strToken = L"";
		std::string  strResult = duk_require_string(ctx, 0);
		std::string  strToken = "";
		int nIndex = strResult.find("\"gv-token\", \"");
		if (nIndex != std::string::npos)
		{
			nIndex += strlen("\"gv-token\", \"");
			strResult = strResult.substr(nIndex);
			int nLen = strResult.size();
			for (int i =0; i < nLen; i++)
			{
				char c = strResult.at(i);
				if (c == '\"')
					break;
				strToken += c;
			}
		}
		if (!strToken.empty())
			m_pCBaseUtrainModel->m_strToken = m_pCBaseUtrainModel->convUTF8toUTF16(strToken);
	}
	duk_push_null(ctx);
	return 1;
}


std::wstring CBaseUtrainModel::convUTF8toUTF16(const std::string& str)
{
	std::wstring wsTmp(str.begin(), str.end());

	return wsTmp;
}


bool CBaseUtrainModel::fillStations(const std::wstring strURL,  std::vector<Station*>& vecpStations)
{
	m_strError = L"";
	if (strURL.empty())
	{
		m_strError =  L"No URL";
		return false;
	}

	HttpClient* pHttpClient = createHttpClient(strURL);
	if (!pHttpClient)
	{
		m_strError =  L"Cannot create HttpClient";
		return false;
	}

	// Set request headers.
	std::wstring strHeaders = L"Content-Length: ";
	strHeaders += L"0";
	strHeaders += L"\r\nContent-Type: binary/octet-stream\r\n";
	pHttpClient->SetAdditionalRequestHeaders(strHeaders);

	// Send http post request.
	if ( !pHttpClient->SendHttpRequest(L"Get"))
	{

		m_strError = L"Error sending: "; 
		m_strError += std::to_wstring(pHttpClient->GetLastError());
		delete pHttpClient;
		return false;
	}

	std::wstring str_httpResponseCode = pHttpClient->GetResponseStatusCode();
	std::wstring str_httpResponseContent = pHttpClient->GetResponseContent();
	delete pHttpClient;

	if (str_httpResponseCode.compare(L"200"))
	{
		m_strError = L"Error response: ";
		m_strError += str_httpResponseCode;
		return false;
	}
	if (str_httpResponseContent.empty())
	{
		m_strError = L"No response";
		return false;
	}

	int nIndex = str_httpResponseContent.find(L"{\"value\":[{");
	if (nIndex == std::wstring::npos)
	{
		m_strError = L"Bad format";
		return false;
	}
	nIndex +=  getSize(L"{\"value\":[{");
	str_httpResponseContent = str_httpResponseContent.substr(nIndex, str_httpResponseContent.size() - nIndex);
	while(true)
	{
		nIndex = str_httpResponseContent.find(L"\"title\":\"");
		if (nIndex == std::wstring::npos)
			break;
		nIndex +=  getSize(L"\"title\":\"");
		str_httpResponseContent = str_httpResponseContent.substr(nIndex, str_httpResponseContent.size() - nIndex);
		if (str_httpResponseContent.empty())
			break;
		//station
		std::wstring strName(L"");
		int i, nLen;
		nLen = str_httpResponseContent.size();
		for (i = 0; i < nLen; i++)
		{
			wchar_t c = str_httpResponseContent[i];
			if (c == L'\"')
				break;
			strName += c;
		}

		//id
		nIndex = str_httpResponseContent.find(L"\"station_id\":");
		if (nIndex == std::wstring::npos)
			break;
		nIndex +=  getSize(L"\"station_id\":");
		str_httpResponseContent = str_httpResponseContent.substr(nIndex, str_httpResponseContent.size() - nIndex);
		if (str_httpResponseContent.empty())
			break;
		nLen = str_httpResponseContent.size();
		std::wstring strID(L"");
		for (i = 0; i < nLen; i++)
		{
			wchar_t c = str_httpResponseContent[i];
			if (c == L'}')
				break;
			strID += c;
		}
		int nId = std::stoi(strID);
		if (nId < 2200000 || nId > 2299999)
			continue; //only Ukraine

		Station* pStation = NULL; 
		pStation =  new Station;
		if(!pStation)
		{
			m_strError = L"Cannot creat Station object";
			return false;
		}
		pStation->m_strID = strID;
		pStation->m_strName = convPrintUTF16(strName);
		vecpStations.push_back(pStation);
	}
	if (vecpStations.empty())
	{
		m_strError = L"No enries.";
		return false;
	}

	return true;
}

std::wstring CBaseUtrainModel::convPrintUTF16(std::wstring& str)
{
	std::wstring strResponse = L"";
	if (str.empty())
		return strResponse;

	typedef enum {
		_empty = 0,
		_start,
		_0,
		_1,
		_2, 
		_3
	} _status;
	_status status = _empty;
	wchar_t ch;
	int nLen = str.size();
	std::wstring strConvert;
	wchar_t chNumber[] = {L'0',L'1',L'2',L'3',L'4',L'5',L'6',L'7',L'8',L'9', L'a', L'b', L'c', L'd', L'e', L'f'};
	unsigned char nNumber[] = {0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xa,0xb,0xc,0xd,0xe,0xf};

	for(int i = 0; i < nLen; i++)
	{
		ch = str.at(i);
		switch (status)
		{
		case _empty:
			if (ch == L'\\')
			{
				status = _start;
			}
			else
				strResponse += ch;
			break;
		case _start:
			if (ch != L'u')
			{
				status = _empty;
				strResponse += L'\\';
				strResponse += ch;
			}
			else
				status = _0;
			break;
		case _0:
			strConvert = ch;
			status = _1;
			break;
		case _1:
			strConvert += ch;
			status = _2;
			break;
		case _2:
			strConvert += ch;
			status = _3;
			break;
		case _3:
			{
				strConvert += ch;
				status = _empty;
				unsigned char strBuf[5] = {0};
				wchar_t chs[4];
				unsigned char n[4];
				chs[0]  = strConvert.at(0);
				chs[1]  = strConvert.at(1);
				chs[2]  = strConvert.at(2);
				chs[3]  = strConvert.at(3);
				int j;
				for (j = 0; j < 4; j++)
				{
					int m = 0;
					while ( m < 16)
					{
						if (chs[j] == chNumber[m])
							break;
						m++;
					}
					n[j] = nNumber[m];
				}
				n[2] = n[2] << 4; 
				strBuf[0] |= n[2];
				strBuf[0] |= n[3];
				n[0] = n[0] << 4;
				strBuf[1] |= n[0];
				strBuf[1] |= n[1];
				strResponse += (wchar_t*)strBuf;
			}
			break;
		default:
			break;
		}
	}

	return strResponse;
}


bool  CBaseUtrainModel::sendRequest(const std::wstring& strURL,
									const std::string& strPost,  
									const std::wstring strReferURL,   
									std::wstring& strResponse)
{
	strResponse = L"";
	m_strError = L"";
	if (strURL.empty() || strPost.empty())
	{
		strResponse = L"Empty input parameters";
		m_strError = strResponse;
		return false;
	}

	HttpClient* pHttpClient = createHttpClient(strURL);
	if (pHttpClient == NULL)
	{
		strResponse = L"Cannot create HttpClient";
		m_strError = strResponse;
		return false;
	}
		
	// Set request headers.
	std::wstring strHeaders = L"Content-Length: ";
	int nLen =  strPost.size();
	pHttpClient->SetAdditionalDataToSend((unsigned char*)strPost.c_str(), nLen);
	wchar_t szSize[50] = L"";
	swprintf_s(szSize, L"%d", nLen);
	strHeaders += szSize;
	strHeaders += L"\r\nContent-Type: application/x-www-form-urlencoded; charset=UTF-8\r\n";
	strHeaders += L"GV-Token: ";
	strHeaders += m_strToken;
	strHeaders += L"\r\nGV-Unique-Host: 1";
	strHeaders += L"\r\nGV-Ajax: 1";
	strHeaders += L"\r\nGV-Screen: 1920x1080";
	strHeaders += L"\r\nGV-Referer: ";
	strHeaders += strReferURL; //http://booking.uz.gov.ua/ru/";
	strHeaders += L"\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*;q=0.8";
	strHeaders += L"\r\nAccept-Language: ru-RU,ru;q=0.8,en-US;q=0.6,en;q=0.4,bg;q=0.2";
	strHeaders += L"\r\nAccept-Encoding: gzip, deflate";
	strHeaders += L"\r\nAccept: */*";
	strHeaders += L"\r\nReferer: ";
	strHeaders += strReferURL; //http://booking.uz.gov.ua/ru/";
	strHeaders += L"\r\nCookie: ";
	strHeaders += m_strResponseCookies;
	strHeaders += createUTMCokies();
	strHeaders += L"\r\n";
    strHeaders += L"Connection: keep-alive\r\n\r\n";

	pHttpClient->SetAdditionalRequestHeaders(strHeaders);

	// Send http post request.
	if ( !pHttpClient->SendHttpRequest(L"Post"))
	{
		strResponse =  L"Error sending: ";
		strResponse += std::to_wstring(pHttpClient->GetLastError());
		m_strError = strResponse;
		delete pHttpClient;
		return false;
	}

	std::wstring str_httpResponseCode = pHttpClient->GetResponseStatusCode();
	std::wstring str_httpResponseContent = pHttpClient->GetResponseContent();
	delete pHttpClient;

	if (str_httpResponseCode.compare(L"200"))
	{
		strResponse =  L"Error response: "; 
		strResponse += str_httpResponseCode;
		m_strError = strResponse;
		return false;
	}
	if (str_httpResponseContent.empty())
	{
		strResponse = L"No response";  
		m_strError = strResponse;
		return false;
	}
	strResponse =  convPrintUTF16(str_httpResponseContent);
	return true;
}


bool CBaseUtrainModel::checkToken()
{
	if (m_strToken.empty())
		return false;
	return true;
}


std::wstring CBaseUtrainModel::createUTMCokies()
{
	std::wstring strCookies(L" __utma=");

	//hash domain
	std::hash<std::wstring> hash;
	std::wstring strDomain = L"kvy.com.ua";
	std::wstring strHashDomain = std::to_wstring(hash(strDomain)); //L"31515437";
	std::wstring strTimeCurrentVisit = std::to_wstring(time(nullptr));

	//utma
	strCookies += strHashDomain;
	strCookies += L'.';
	strCookies += std::to_wstring(21589326); //my ID user in Google Analistics
	strCookies += L'.';
	strCookies += std::to_wstring(m_timeFirstVisit); //first visit
	strCookies += L'.';
	strCookies += std::to_wstring(m_timeFirstVisit);//prev. visit
	strCookies += L'.';
	strCookies += strTimeCurrentVisit;//curr. visit
	strCookies += L'.';
	strCookies += std::to_wstring(m_nVisitBooking++);
	strCookies += L"; __utmb=";

	//utmb
	strCookies += strHashDomain;
	strCookies += L".2.10.";
	strCookies += strTimeCurrentVisit;//curr. visit
	strCookies += L"; __utmc=";

	//utmc
	strCookies += strHashDomain;
	strCookies += L"; __utmt=1; __utmz=";

	//utmz
	strCookies += strHashDomain;
	strCookies += L'.';
	strCookies += strTimeCurrentVisit;//curr. visit
	strCookies += L".1.1.utmcsr=(direct)|utmccn=(direct)|utmcmd=(none);";

	return strCookies;
}