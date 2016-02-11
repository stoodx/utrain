#include "BaseUtrainModel.h"
#include <codecvt>

#define PUSH_C_FUNCTION(fcn, nargs) \
	duk_push_c_function(ctx, fcn, nargs);\
	duk_put_prop_string(ctx, -2, #fcn);


CBaseUtrainModel* CBaseUtrainModel::m_pCBaseUtrainModel = NULL;

CBaseUtrainModel::CBaseUtrainModel(void)
	: m_strToken(L"")
{
	m_timeFirstVisit = time(nullptr);
	m_pCBaseUtrainModel = (CBaseUtrainModel*)this;
}


CBaseUtrainModel::~CBaseUtrainModel(void)
{
}


bool CBaseUtrainModel::sendRequestForToken(const std::wstring& strURL, std::wstring& strResponse)
{
	HttpClient*  pHttpClient = createHttpClient(strURL);
	if (!pHttpClient)
	{
		strResponse = L"No HttpClient object";
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
		return false;
	}

	std::wstring str_httpResponseCode = pHttpClient->GetResponseStatusCode();
	std::wstring str_httpResponseContent = pHttpClient->GetResponseContent();
	std::wstring str_httpResponseCookies = pHttpClient->GetResponseCookies();

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
	if (nIndex == std::wstring::npos)
	{
		strResponse = L"Token: bad format";
		return false;
	}
	nIndex += getSize(L"gaq.push(['_trackPageview']);");
	str_httpResponseContent = str_httpResponseContent.substr(nIndex);
	nIndex = str_httpResponseContent.find(L"(function ()");
	if (nIndex == std::wstring::npos)
	{
		strResponse = L"Token: bad format";
		return false;
	}
	str_httpResponseContent = str_httpResponseContent.substr(0, nIndex);
	std::string strToken = convUTF16toUTF8(str_httpResponseContent);

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
			strResponse = convUTF8toUTF16(duk_safe_to_string(ctx, -1));
			duk_pop(ctx);
			duk_destroy_heap(ctx);
			return false;
		}
		duk_pop(ctx);
	}
	else
	{
		strResponse = convUTF8toUTF16(duk_safe_to_string(ctx, -1));
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
	std::wstring strTmp;
	int i;
	wchar_t c;
	if (nIndex == std::wstring::npos)
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
	if (nIndex == std::wstring::npos)
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
	if (nIndex == std::wstring::npos)
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
	typedef std::codecvt_utf16<wchar_t> convert_typeX;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.from_bytes(str);
}
