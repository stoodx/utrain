#include "BaseUtrainModel.h"
#include <codecvt>

#define PUSH_C_FUNCTION(fcn, nargs) \
	duk_push_c_function(ctx, fcn, nargs);\
	duk_put_prop_string(ctx, -2, #fcn);


CBaseUtrainModel* CBaseUtrainModel::m_pCBaseUtrainModel = NULL;

CBaseUtrainModel::CBaseUtrainModel(void)
	: m_strToken(L"")
	, m_strError(L"")
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
		m_strError =  L"fillStations() - No URL";
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
		strError.Format(L"Error sending: %i", pHttpClient->GetLastError());
		delete pHttpClient;
		return false;
	}

	std::wstring str_httpResponseCode = pHttpClient->GetResponseStatusCode();
	std::wstring str_httpResponseContent = pHttpClient->GetResponseContent();
	delete pHttpClient;

	if (str_httpResponseCode.compare(L"200"))
	{
		strError.Format(L"Error response: %s", str_httpResponseCode.c_str());
		return false;
	}
	if (str_httpResponseContent.empty())
	{
		AfxMessageBox(L"No response");
		return false;
	}

	int nIndex = str_httpResponseContent.find(L"{\"value\":[{");
	if (nIndex == std::wstring::npos)
	{
		AfxMessageBox(L"Bad format");
		return false;
	}
	nIndex +=  _tcslen(L"{\"value\":[{");
	str_httpResponseContent = str_httpResponseContent.substr(nIndex, str_httpResponseContent.size() - nIndex);
	while(true)
	{
		nIndex = str_httpResponseContent.find(L"\"title\":\"");
		if (nIndex == std::wstring::npos)
			break;
		nIndex +=  _tcslen(L"\"title\":\"");
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
		nIndex +=  _tcslen(L"\"station_id\":");
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
		ASSERT(pStation);
		pStation->m_strID = strID;
		pStation->m_strName = PrintUTF16Converter(strName);
		vecpStations.push_back(pStation);
	}
	if (vecpStations.empty())
	{
		AfxMessageBox(L"No enries.");
		return false;
	}

	int nSize = vecpStations.size();
	for (int j = 0; j < nSize; j++)
	{
		Station* pStation = vecpStations[j];
		comboStation.AddString(pStation->m_strName.c_str());
	}
	comboStation.SetCurSel(0);
	return true;

}