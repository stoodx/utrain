#include "BaseUtrainModel.h"


HttpClient::HttpClient(const std::wstring& strURL)
{
}

CBaseUtrainModel::CBaseUtrainModel(HttpClient** ppHttpClient)
	: m_ppHttpClient(ppHttpClient)
{
	m_timeFirstVisit = time(nullptr);
}


CBaseUtrainModel::~CBaseUtrainModel(void)
{
	if ( (*m_ppHttpClient) == NULL)
		delete *m_ppHttpClient;
}


bool CBaseUtrainModel::sendRequestForToken(const std::wstring& strURL, std::wstring& strResponse)
{
	strResponse = L"";
	if (*m_ppHttpClient)
	{
		delete *m_ppHttpClient;
		*m_ppHttpClient = NULL;
	}
	*m_ppHttpClient  = new HttpClient(strURL);
	// Set request headers.
	std::wstring strHeaders = L"Content-Length: ";
	strHeaders += L"0";
	strHeaders += L"\r\nContent-Type: binary/octet-stream\r\n";
	(*m_ppHttpClient)->SetAdditionalRequestHeaders(strHeaders);

	// Send http post request.
	if (!request.SendHttpRequest(L"Get"))
	{
		strResponse = L"Error sending: ";
		strResponse.append(std::to_wstring(request.GetLastError()));
		return false;
	}

	wstring str_httpResponseCode = request.GetResponseStatusCode();
	wstring str_httpResponseContent = request.GetResponseContent();
	wstring str_httpResponseCookies = request.GetResponseCookies();

	if (str_httpResponseCode.compare(L"200"))
	{
		strResponse = L"Error sending: ";
		strResponse.append(str_httpResponseCode);
		return false;
	}
	if (str_httpResponseContent.empty())
	{
		strResponse =  L"No response";
		return false;
	}

	int nIndex;
	nIndex = str_httpResponseContent.find(L"gaq.push(['_trackPageview']);");
	if (nIndex == wstring::npos)
	{
		strResponse = L"Token: bad format";
		return false;
	}
	nIndex += _tcslen(L"gaq.push(['_trackPageview']);");
	str_httpResponseContent = str_httpResponseContent.substr(nIndex);
	nIndex = str_httpResponseContent.find(L"(function ()");
	if (nIndex == wstring::npos)
	{
		strResponse = L"Token: bad format";
		return false;
	}
	str_httpResponseContent = str_httpResponseContent.substr(0, nIndex);
	string strToken = UTF16toUTF8(str_httpResponseContent);

	//gv-token
	duk_context* ctx = duk_create_heap_default();
	if(!ctx)
	{
		strResponse = L"HEAP_CREATION_ERROR";
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
			strResponse = CString(duk_safe_to_string(ctx, -1));
			duk_pop(ctx);
			duk_destroy_heap(ctx);
			return false;
		}
		duk_pop(ctx);
	}
	else
	{
		strResponse = CString(duk_safe_to_string(ctx, -1));
		duk_pop(ctx);
		duk_destroy_heap(ctx);
		return false;
	}

	duk_pop(ctx);
	duk_destroy_heap(ctx);

	//cookies
	//_gv_sessid
	nIndex = str_httpResponseCookies.find(L"_gv_sessid");
	int nLen = str_httpResponseCookies.size();
	wstring strTmp;
	int i;
	wchar_t c;
	if (nIndex == wstring::npos)
	{
		strResponse = L"No _gv_sessid";
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
	if (nIndex == wstring::npos)
	{
		strResponse = L"No _gv_lang";
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
	if (nIndex == wstring::npos)
	{
		strResponse = L"No HTTPSERVERID";
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
