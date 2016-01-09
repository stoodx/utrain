
// RailTickesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RailTickes.h"
#include "RailTickesDlg.h"
#include "afxdialogex.h"

#include "WinHttpClient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define CR L"\n" 

#define PUSH_C_FUNCTION(fcn, nargs) \
	duk_push_c_function(ctx, fcn, nargs);\
	duk_put_prop_string(ctx, -2, #fcn);

// CRailTickesDlg dialog

const wchar_t* g_strAlphabet[] = 
{ L"А", L"Б", L"В", L"Г", L"Д", L"Е",  L"Ж", L"З", L"И", L"К", L"Л", L"М", L"Н", L"О", L"П", L"Р", 
	L"С", L"Т", L"У", L"Ф", L"Х", L"Ц", L"Ч", L"Ш", L"Щ ", L"Э", L"Ю", L"Я", NULL 
};

struct Station
{
	std::wstring m_strID;
	std::wstring m_strName;
};

CRailTickesDlg* CRailTickesDlg::m_pCRailTickesDlg = NULL;


CRailTickesDlg::CRailTickesDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CRailTickesDlg::IDD, pParent)
	, m_nBooking(0)
	, m_strToken(L"")
	, m_strResponseCookies(L"")
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pCRailTickesDlg = this;
}

void CRailTickesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_A_FROM, m_comboA_From);
	DDX_Control(pDX, IDC_COMBO_A_TO, m_comboA_To);
	DDX_Control(pDX, IDC_COMBO_FROM, m_comboFrom);
	DDX_Control(pDX, IDC_COMBO_TO, m_comboTo);
	DDX_Control(pDX, IDC_MONTHCALENDAR1, m_calendar);
	DDX_Control(pDX, IDOK, m_btnSearch);
	DDX_Radio(pDX, IDC_RADIO_BOOKING, m_nBooking);
}

BEGIN_MESSAGE_MAP(CRailTickesDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_COMBO_A_FROM, &CRailTickesDlg::OnCbnSelchangeComboAFrom)
	ON_CBN_SELCHANGE(IDC_COMBO_A_TO, &CRailTickesDlg::OnCbnSelchangeComboATo)
	ON_BN_CLICKED(IDOK, &CRailTickesDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_RADIO_BOOKING, &CRailTickesDlg::OnBnClickedRadioBooking)
	ON_BN_CLICKED(IDC_RADIO_DPRC, &CRailTickesDlg::OnBnClickedRadioDprc)
END_MESSAGE_MAP()


// CRailTickesDlg message handlers

BOOL CRailTickesDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	int i = 0;
	while(true)
	{
		if (g_strAlphabet[i] == NULL)
			break;
		m_comboA_From.AddString(g_strAlphabet[i]);
		m_comboA_To.AddString(g_strAlphabet[i++]);
	}

	m_comboA_From.SetCurSel(0);
	m_comboA_To.SetCurSel(1);
	UpdateData(FALSE);

	if (!m_nBooking)
	{
		wstring strResponse;
		if (!SendRequestForToken(L"http://booking.uz.gov.ua/ru/", strResponse))
			AfxMessageBox(strResponse.c_str());
		else
			m_strResponseCookies = strResponse;
	}

	if (!FillStations(m_comboA_From, m_comboFrom, m_vecpStationsFrom) ||
		!FillStations(m_comboA_To, m_comboTo, m_vecpStationsTo))
		return TRUE;

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRailTickesDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CRailTickesDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

bool CRailTickesDlg::FillStations(CComboBox& comboA, CComboBox& comboStation, std::vector<Station*>& vecpStations)
{
	UpdateData(TRUE);
	if (!m_nBooking)
		return FillStationsBooking(comboA, comboStation, vecpStations);
	else
		return FillStationsDPRC(comboA, comboStation, vecpStations);
}

std::wstring CRailTickesDlg::PrintUTF16Converter(std::wstring& str)
{
	wstring strResponse = L"";
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
	wstring strConvert;
	wchar_t chNumber[] = {L'0',L'1',L'2',L'3',L'4',L'5',L'6',L'7',L'8',L'9', L'a', L'b', L'c', L'd', L'e', 'f'};
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

bool CRailTickesDlg::FillStationsBooking(CComboBox& comboA, CComboBox& comboStation, std::vector<Station*>& vecpStations)
{
	CString strError;

	comboStation.ResetContent();
	CleanStations(&vecpStations);

	wstring strURL(L"http://booking.uz.gov.ua/ru/purchase/station/");
	CString strA;
	comboA.GetLBText(comboA.GetCurSel(), strA);
	strURL.append(strA);
	WinHttpClient request(strURL);

	// Set request headers.
	wstring strHeaders = L"Content-Length: ";
	strHeaders += L"0";
	strHeaders += L"\r\nContent-Type: binary/octet-stream\r\n";
	request.SetAdditionalRequestHeaders(strHeaders);

	// Send http post request.
	if ( !request.SendHttpRequest(L"Get"))
	{
		strError.Format(L"Error sending: %i", request.GetLastError());
		AfxMessageBox(strError);
		return false;
	}

	wstring str_httpResponseCode = request.GetResponseStatusCode();
	wstring str_httpResponseContent = request.GetResponseContent();

	if (str_httpResponseCode.compare(L"200"))
	{
		strError.Format(L"Error response: %s", str_httpResponseCode.c_str());
		AfxMessageBox(strError);
		return false;
	}
	if (str_httpResponseContent.empty())
	{
		AfxMessageBox(L"No response");
		return false;
	}

	int nIndex = str_httpResponseContent.find(L"{\"value\":[{");
	if (nIndex == wstring::npos)
	{
		AfxMessageBox(L"Bad format");
		return false;
	}
	nIndex +=  _tcslen(L"{\"value\":[{");
	str_httpResponseContent = str_httpResponseContent.substr(nIndex, str_httpResponseContent.size() - nIndex);
	while(true)
	{
		nIndex = str_httpResponseContent.find(L"\"title\":\"");
		if (nIndex == wstring::npos)
			break;
		nIndex +=  _tcslen(L"\"title\":\"");
		str_httpResponseContent = str_httpResponseContent.substr(nIndex, str_httpResponseContent.size() - nIndex);
		if (str_httpResponseContent.empty())
			break;
		//station
		wstring strName(L"");
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
		if (nIndex == wstring::npos)
			break;
		nIndex +=  _tcslen(L"\"station_id\":");
		str_httpResponseContent = str_httpResponseContent.substr(nIndex, str_httpResponseContent.size() - nIndex);
		if (str_httpResponseContent.empty())
			break;
		nLen = str_httpResponseContent.size();
		wstring strID(L"");
		for (i = 0; i < nLen; i++)
		{
			wchar_t c = str_httpResponseContent[i];
			if (c == L'}')
				break;
			strID += c;
		}
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

bool CRailTickesDlg::FillStationsDPRC(CComboBox& comboA, CComboBox& comboStation, std::vector<Station*>& vecpStations)
{
	CString strError;

	comboStation.ResetContent();
	CleanStations(&vecpStations);

	wstring strURL(L"http://dprc.gov.ua/awg/xml?class_name=IStations&method_name=search_station&var_0=2&var_1=2&var_2=0&var_3=16&var_4=");
	CString strA;
	comboA.GetLBText(comboA.GetCurSel(), strA);
	strURL.append(strA);
	WinHttpClient request(strURL);

	// Set request headers.
	wstring strHeaders = L"Content-Length: ";
	strHeaders += L"0";
	strHeaders += L"\r\nContent-Type: binary/octet-stream\r\n";
	request.SetAdditionalRequestHeaders(strHeaders);

	// Send http post request.
	if ( !request.SendHttpRequest(L"Get"))
	{
		strError.Format(L"Error sending: %i", request.GetLastError());
		AfxMessageBox(strError);
		return false;
	}

	wstring str_httpResponseCode = request.GetResponseStatusCode();
	wstring str_httpResponseContent = request.GetResponseContent();

	if (str_httpResponseCode.compare(L"200"))
	{
		strError.Format(L"Error response: %s", str_httpResponseCode.c_str());
		AfxMessageBox(strError);
		return false;
	}
	if (str_httpResponseContent.empty())
	{
		AfxMessageBox(L"No response");
		return false;
	}

	int nIndex = str_httpResponseContent.find(L"<MSG><var_0>");
	if (nIndex == wstring::npos)
	{
		AfxMessageBox(L"Bad format");
		return false;
	}
	nIndex +=  _tcslen(L"<MSG><var_0>");
	str_httpResponseContent = str_httpResponseContent.substr(nIndex, str_httpResponseContent.size() - nIndex);

	while(true)
	{
		nIndex = str_httpResponseContent.find(L"<childs><i v=\"");
		if (nIndex == wstring::npos)
			break;
		nIndex +=  _tcslen(L"<childs><i v=\"");
		str_httpResponseContent = str_httpResponseContent.substr(nIndex, str_httpResponseContent.size() - nIndex);
		if (str_httpResponseContent.empty())
			break;
		//id
		int i;
		int nLen = str_httpResponseContent.size();
		wstring strID(L"");
		for (i = 0; i < nLen; i++)
		{
			wchar_t c = str_httpResponseContent[i];
			if (c == L'\"')
				break;
			strID += c;
		}
		//name
		nIndex = str_httpResponseContent.find(L"<i v=\"");
		if (nIndex == wstring::npos)
			break;
		nIndex +=  _tcslen(L"<i v=\"");
		str_httpResponseContent = str_httpResponseContent.substr(nIndex, str_httpResponseContent.size() - nIndex);
		if (str_httpResponseContent.empty())
			break;
		wstring strName(L"");
		nLen = str_httpResponseContent.size();
		for (i = 0; i < nLen; i++)
		{
			wchar_t c = str_httpResponseContent[i];
			if (c == L'\"')
				break;
			strName += c;
		}
		Station* pStation = NULL; 
		pStation =  new Station;
		ASSERT(pStation);
		pStation->m_strID = strID;
		pStation->m_strName = strName;
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


void CRailTickesDlg::CleanStations(std::vector<Station*>* pvecpStations)
{
	if (!pvecpStations)
		return;
	int nSize = pvecpStations->size();
	for (int i = 0; i < nSize; i++)
	{
		Station* pStation = pvecpStations->at(i);
		delete pStation; 
		pStation = NULL;
	}
	pvecpStations->clear();
}

void CRailTickesDlg::OnClose()
{
	CleanStations(&m_vecpStationsFrom);
	CleanStations(&m_vecpStationsTo);
	m_pCRailTickesDlg = NULL;
	CDialogEx::OnClose();
}


void CRailTickesDlg::OnCbnSelchangeComboAFrom()
{
	FillStations(m_comboA_From, m_comboFrom, m_vecpStationsFrom);
}


void CRailTickesDlg::OnCbnSelchangeComboATo()
{
	FillStations(m_comboA_To, m_comboTo, m_vecpStationsTo);
}

std::string CRailTickesDlg::UTF16toUTF8(const std::wstring strUTF16)
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


std::string  CRailTickesDlg::UrlEncode(const std::string str)
{
	string escaped("");
	if (str.empty())
		return escaped;

	int len = str.size();
	for (int i = 0; i < len; i++)
	{
		char c = str.at(i);
		if ( (48 <= c && c <= 57) ||//0-9
             (65 <= c && c <= 90) ||//abc...xyz
             (97 <= c && c <= 122) || //ABC...XYZ
             (c == '~' || c == '!' || c=='*' || c == '(' || c == ')' || c == '\'')
        )
        {
            escaped += c ;
        }
        else
        {
            escaped  +=  '%';
            escaped   +=  Char2hex(c); //converts char 255 to string "ff"
        }
	}

	return escaped;
}


std::string  CRailTickesDlg::Char2hex(char c)
{
    char dig1 = (c&0xF0)>>4;
    char dig2 = (c&0x0F);
    if ( 0<= dig1 && dig1<= 9) 
		dig1+=48;    //0,48 in ascii
    if (10<= dig1 && dig1<=15) 
		dig1+= 65-10; //97-10; //a,97 in ascii
    if ( 0<= dig2 && dig2<= 9) 
		dig2+=48;
    if (10<= dig2 && dig2<=15) 
		dig2+= 65-10;//97-10;

    string r("");
    r += dig1;
    r += dig2;
    return r;
}

void CRailTickesDlg::OnBnClickedOk()
{
	UpdateData(TRUE);

	if (m_vecpStationsFrom.empty() ||
		m_vecpStationsTo.empty())
		return;
	
	m_btnSearch.EnableWindow(FALSE);
	m_btnSearch.SetWindowText(L"Ожидайте...");

	wstring strJSON;
	if (m_nBooking)
		strJSON = RequestDPRC();
	else
		strJSON = RequestBookong();

	AfxMessageBox(strJSON.c_str(), MB_OK | MB_ICONINFORMATION); 
	m_btnSearch.EnableWindow(TRUE);
	m_btnSearch.SetWindowText(L"Поиск");
}

bool CRailTickesDlg::SendRequestForToken(const std::wstring& strURL, std::wstring& strResponse)
{
	strResponse = L"";
	WinHttpClient request(strURL);
	// Set request headers.
	wstring strHeaders = L"Content-Length: ";
	strHeaders += L"0";
	strHeaders += L"\r\nContent-Type: binary/octet-stream\r\n";
	request.SetAdditionalRequestHeaders(strHeaders);
	CString strError;

	// Send http post request.
	if (!request.SendHttpRequest(L"Get"))
	{
		strError.Format(L"Error sending: %i", request.GetLastError());
		strResponse =  strError.GetBuffer();
		return false;
	}

	wstring str_httpResponseCode = request.GetResponseStatusCode();
	wstring str_httpResponseContent = request.GetResponseContent();
	wstring str_httpResponseCookies = request.GetResponseCookies();

	if (str_httpResponseCode.compare(L"200"))
	{
		strError.Format(L"Error response: %s", str_httpResponseCode.c_str());
		strResponse =  strError.GetBuffer();
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


std::wstring CRailTickesDlg::CreateUTMCokies()
{
	wstring strCookies(L" ");

	return strCookies;
}

std::wstring CRailTickesDlg::RequestBookong()
{

	if (m_strToken.empty())
	{
		wstring strResponse;
		if (!SendRequestForToken(L"http://booking.uz.gov.ua/ru/", strResponse))
		{
			return CString(strResponse.c_str()).GetBuffer();
		}
		else
			m_strResponseCookies = strResponse;
		return L"Try to send a request once again";
	}

	int nCurrentPosForm = m_comboFrom.GetCurSel();
	int nCurrentPosTo = m_comboTo.GetCurSel();
	const wchar_t* strIDFrom = m_vecpStationsFrom[nCurrentPosForm]->m_strID.c_str();
	const wchar_t* strStationFrom = m_vecpStationsFrom[nCurrentPosForm]->m_strName.c_str();
	const wchar_t* strIDTo = m_vecpStationsTo[nCurrentPosTo]->m_strID.c_str();
	const wchar_t* strStationTo = m_vecpStationsTo[nCurrentPosTo]->m_strName.c_str();

	SYSTEMTIME dateTime;
	m_calendar.GetCurSel(&dateTime);
	wchar_t strURL[MAX_PATH] = {0};
	char strPost[MAX_PATH * 2] = {0};

	char strDay[3] = {0};
	char strMonth[3] = {0};
	if (dateTime.wDay < 10)
		sprintf_s(strDay, 3,  "0%d", dateTime.wDay);
	else
		sprintf_s(strDay, 3,  "%d", dateTime.wDay);
	if (dateTime.wMonth < 10)
		sprintf_s(strMonth, 3,  "0%d", dateTime.wMonth);
	else
		sprintf_s(strMonth, 3,  "%d", dateTime.wMonth);

	_tcscpy_s(strURL, MAX_PATH, L"http://booking.uz.gov.ua/ru/purchase/search/");
	sprintf_s(strPost, 2 * MAX_PATH, 
		"station_id_from=%s&station_id_till=%s&station_from=%s&station_till=%s&date_dep=%s.%s.%d&time_dep=00:00&time_dep_till=&another_ec=0&search=",
		UTF16toUTF8(strIDFrom).c_str(), UTF16toUTF8(strIDTo).c_str(), 
		UrlEncode(UTF16toUTF8(strStationFrom)).c_str(), UrlEncode(UTF16toUTF8(strStationTo)).c_str(), 
		strDay, strMonth, dateTime.wYear);
		
	WinHttpClient request(strURL);
	// Set request headers.
	wstring strHeaders = L"Content-Length: ";
	int nLen =  strlen(strPost);
	request.SetAdditionalDataToSend((BYTE *)strPost, nLen);
	wchar_t szSize[50] = L"";
	swprintf_s(szSize, L"%d", nLen);
	strHeaders += szSize;
	strHeaders += L"\r\nContent-Type: binary/octet-stream\r\n";
	strHeaders += L"GV-Token: ";
	strHeaders += m_strToken;
	strHeaders += L"\r\nGV-Unique-Host: 1";
	strHeaders += L"\r\nGV-Ajax: 1";
	strHeaders += L"\r\nGV-Screen: 1920x1080";
	strHeaders += L"\r\nGV-Referer: http://booking.uz.gov.ua/ru/";
	strHeaders += L"\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*;q=0.8";
	strHeaders += L"\r\nAccept-Language: ru-RU,ru;q=0.8,en-US;q=0.6,en;q=0.4,bg;q=0.2";
	strHeaders += L"\r\nAccept-Encoding: gzip, deflate";
	strHeaders += L"\r\nAccept: */*";
	strHeaders += L"\r\nCookie: ";
	strHeaders += m_strResponseCookies;
	strHeaders += CreateUTMCokies();
	strHeaders += L"\r\n";
	request.SetAdditionalRequestHeaders(strHeaders);
	CString strError;

	// Send http post request.
	if ( !request.SendHttpRequest(L"Post"))
	{
		strError.Format(L"Error sending: %i", request.GetLastError());
		return strError.GetBuffer();
	}

	wstring str_httpResponseCode = request.GetResponseStatusCode();
	wstring str_httpResponseContent = request.GetResponseContent();

	if (str_httpResponseCode.compare(L"200"))
	{
		strError.Format(L"Error response: %s", str_httpResponseCode.c_str());
		return strError.GetBuffer();
	}
	if (str_httpResponseContent.empty())
	{
		return L"No response";
	}
	wstring strJSON;
	ParserBooking(str_httpResponseContent, strJSON);
	return strJSON;
}

std::wstring CRailTickesDlg::RequestDPRC()
{
	int nCurrentPosForm = m_comboFrom.GetCurSel();
	int nCurrentPosTo = m_comboTo.GetCurSel();
	const wchar_t* strIDFrom = m_vecpStationsFrom[nCurrentPosForm]->m_strID.c_str();
	const wchar_t* strStationFrom = m_vecpStationsFrom[nCurrentPosForm]->m_strName.c_str();
	const wchar_t* strIDTo = m_vecpStationsTo[nCurrentPosTo]->m_strID.c_str();
	const wchar_t* strStationTo = m_vecpStationsTo[nCurrentPosTo]->m_strName.c_str();

	SYSTEMTIME dateTime;
	m_calendar.GetCurSel(&dateTime);
	wchar_t strURL[MAX_PATH] = {0};
	if (m_nBooking)
		_stprintf_s(strURL, MAX_PATH, L"http://dprc.gov.ua/show.php?transport_type=2&src=%s&dst=%s&dt=%d-%d-%d&ret_dt=2001-01-01&ps=ec_privat&set_language=1",
			strIDFrom, strIDTo, dateTime.wYear,  dateTime.wMonth, dateTime.wDay);

	WinHttpClient request(strURL);
	// Set request headers.
	wstring strHeaders = L"Content-Length: ";
	strHeaders += L"0";
	strHeaders += L"\r\nContent-Type: binary/octet-stream\r\n";
	request.SetAdditionalRequestHeaders(strHeaders);
	CString strError;

	// Send http post request.
	if ( !request.SendHttpRequest(L"Get"))
	{
		strError.Format(L"Error sending: %i", request.GetLastError());
		return strError.GetBuffer();
	}

	wstring str_httpResponseCode = request.GetResponseStatusCode();
	wstring str_httpResponseContent = request.GetResponseContent();

	if (str_httpResponseCode.compare(L"200"))
	{
		strError.Format(L"Error response: %s", str_httpResponseCode.c_str());
		return strError.GetBuffer();
	}
	if (str_httpResponseContent.empty())
	{
		return L"No response";
	}
	wstring strJSON;
	ParserDPRC(str_httpResponseContent, strJSON);
	return strJSON;
}

bool CRailTickesDlg::PartParser(std::wstring& strResponse, const wchar_t* str, std::wstring& strTarget)
{
	if (strResponse.empty() || !str)
		return false;
	int i, nIndex;

	strTarget = L"";
	nIndex = strResponse.find(str);
	if (nIndex == wstring::npos)
		return false;
	nIndex +=  _tcslen(str);
	strResponse = strResponse.substr(nIndex, strResponse.size() - nIndex);
	if (strResponse.empty())
		return false;
	int nLen = strResponse.size();
	for (i = 0; i < nLen; i++)
	{
		wchar_t ch = strResponse[i];
		if (ch == L'<')
			break;
		strTarget += ch;
	}
	return true; 
}

bool CRailTickesDlg::PartParserWagon(std::wstring& strResponse, const wchar_t* str, std::wstring& strPrice, std::wstring& strSeats)
{
	if (strResponse.empty() || !str)
		return false;
	int i, nIndex1, nIndex2;

	strPrice = L"";
	strSeats = L"";
	bool bResult = false;
	wstring strSub;

	nIndex1 = strResponse.find(str);
	if (nIndex1 == wstring::npos)
		return bResult;
	nIndex1 +=  _tcslen(str);
	strResponse = strResponse.substr(nIndex1, strResponse.size() - nIndex1);
	if (strResponse.empty())
		return bResult;
	nIndex2 = strResponse.find(L"</td>");
	if (nIndex2 == wstring::npos)
	{
		return bResult;
	}
	if (nIndex2 == 0)
	{//no info
		goto end;
	}

	strSub = strResponse.substr(0, nIndex2 + 1);
	if (strSub.empty())
		goto end;

	//price and seats
	nIndex1 = strSub.find(L"<p class='price'>");
	if (nIndex1 != wstring::npos)
	{
		nIndex1 +=  _tcslen(L"<p class='price'>");
		strSub = strSub.substr(nIndex1, strSub.size() - nIndex1);
		if (strSub.empty())
			goto end;
		int nLen = strSub.size();
		for (i = 0; i < nLen; i++)
		{
			wchar_t ch = strSub[i];
			if (ch == L'<')
				break;
			strPrice += ch;
		}
		if (strPrice.empty())
			goto end;
		nIndex1 = strSub.find(L"<p class='seats_avail'>");
		if (nIndex1 == wstring::npos)
			goto end;
		nIndex1 +=  _tcslen(L"<p class='seats_avail'>");
		strSub = strSub.substr(nIndex1, strSub.size() - nIndex1);
		if (strSub.empty())
			goto end;
		nLen = strSub.size();
		for (i = 0; i < nLen; i++)
		{
			wchar_t ch = strSub[i];
			if (ch == L'<')
				break;
			strSeats += ch;
		}
		if (!strSeats.empty())
			bResult = true;
	}
	
end:
	nIndex2 +=  _tcslen(L"</td>");
	strResponse = strResponse.substr(nIndex2, strResponse.size() - nIndex2);
	return bResult;
}

void CRailTickesDlg::ParserBooking(std::wstring& strResponse, std::wstring& strJSONResult)
{
	strResponse = PrintUTF16Converter(strResponse);
	
	//int nIndex = strResponse.find(L"\"error\":true");
	//if (nIndex != wstring::npos)
	{//error
		strJSONResult = strResponse;
		return;
	}
	
}

void CRailTickesDlg::ParserDPRC(std::wstring& strResponse, std::wstring& strJSONResult)
{
	strJSONResult = L"";
	if (strResponse.empty())
	{
		strJSONResult = L"{\"error\":\"No data for operation\"}";
		return;
	}

	wstring strFrom(L"");
	wstring strTo(L"");
	wstring strDate(L"");
	int nIndex = 0;
	int nLen = 0;
	int i;
	wchar_t ch;
	vector<wstring> vecstrTrains;

	//target
	nIndex = strResponse.find(L"<div id=\"tables\" class='tables'>");
	if (nIndex == wstring::npos)
	{
		strJSONResult = L"{\"error\":\"No information\"}";
		return;
	}
	nIndex +=  _tcslen(L"<div id=\"tables\" class='tables'>");
	strResponse = strResponse.substr(nIndex, strResponse.size() - nIndex);
	if (strResponse.empty())
	{
		strJSONResult = L"{\"error\":\"No table\"}";
		return;
	}
	
	//date
	nIndex = strResponse.find(L"<span style='font-weight: bold;'>");
	if (nIndex == wstring::npos)
	{
		strJSONResult = L"{\"error\":\"No date1\"}";
		return;
	}
	nIndex +=  _tcslen(L"<span style='font-weight: bold;'>");
	strResponse = strResponse.substr(nIndex, strResponse.size() - nIndex);
	if (strResponse.empty())
	{
		strJSONResult = L"{\"error\":\"No date2\"}";
		return;
	}
	nLen = strResponse.size();
	for (i = 0; i < nLen; i++)
	{
		ch = strResponse[i];
		if (ch == L'<')
			break;
		strDate += ch;
	}
	if (strDate.empty())
	{
		strJSONResult = L"{\"error\":\"No date3\"}";
		return;
	}

	//from
	nIndex = strResponse.find(L"<span style='font-weight: bold;'>");
	if (nIndex == wstring::npos)
	{
		strJSONResult = L"{\"error\":\"No data for departure1\"}";
		return;
	}
	nIndex +=  _tcslen(L"<span style='font-weight: bold;'>");
	strResponse = strResponse.substr(nIndex, strResponse.size() - nIndex);
	if (strResponse.empty())
	{
		strJSONResult = L"{\"error\":\"No data for departure2\"}";
		return;
	}
	nLen = strResponse.size();
	for (i = 0; i < nLen; i++)
	{
		ch = strResponse[i];
		if (ch == L'<')
			break;
		strFrom += ch;
	}
	if (strFrom.empty())
	{
		strJSONResult = L"{\"error\":\"No data for departure3\"}";
		return;
	}

	//to
	nIndex = strResponse.find(L"<span style='font-weight: bold;'>");
	if (nIndex == wstring::npos)
	{
		strJSONResult = L"{\"error\":\"No data for destination1\"}";
		return;
	}
	nIndex +=  _tcslen(L"<span style='font-weight: bold;'>");
	strResponse = strResponse.substr(nIndex, strResponse.size() - nIndex);
	if (strResponse.empty())
	{
		strJSONResult = L"{\"error\":\"No data for destination2\"}";
		return;
	}
	nLen = strResponse.size();
	for (i = 0; i < nLen; i++)
	{
		ch = strResponse[i];
		if (ch == L'<')
			break;
		strTo += ch;
	}
	if (strTo.empty())
	{
		strJSONResult = L"{\"error\":\"No data for destination3\"}";
		return;
	}
	
	//Trains;
	while(true)
	{
		wstring strTrainNumber;
		wstring strTrainDeparture;
		wstring strTrainDestination;
		wstring strTrainDep;
		wstring strTrainDuration;
		wstring strTrainArrive;
		//wstring strTrainLux;
		wstring strTrainLuxPrice;
		wstring strTrainLuxSeat;
		wstring strTrainCompartmentFirmPrice;
		wstring strTrainCompartmentFirmSeat;
		wstring strTrainCompartmentPrice;
		wstring strTrainCompartmentSeat;
		wstring strTrainThirdClassFirmPrice;
		wstring strTrainThirdClassFirmSeat;
		wstring strTrainThirdClassPrice;
		wstring strTrainThirdClassSeat;
		wstring strTrainSeatsPrice;
		wstring strTrainSeatsSeat;

		nIndex = strResponse.find(L"<tr class=\"train_row\" id=\"row_");
		if (nIndex == wstring::npos)
			break;
		nIndex +=  _tcslen(L"<tr class=\"train_row\" id=\"row_");
		strResponse = strResponse.substr(nIndex, strResponse.size() - nIndex);
		if (strResponse.empty())
			break;

		//number
		if (!PartParser(strResponse, 
			L"<td class=\"info_row train first\" style='font-size: 14pt; vertical-align: top; margin-top: 0px; padding-top: 1px; padding-right: 0px;'>",
			strTrainNumber))
			break;
		if (strTrainNumber.empty())
			break;

		//departue
		if (!PartParser(strResponse, L"<td class=\"info_row name\">", strTrainDeparture))
			break;
		if (strTrainDeparture.empty())
			break;

		//destination
		if (!PartParser(strResponse, L"<td class=\"info_row name\">", strTrainDestination))
			break;
		if (strTrainDestination.empty())
			break;

		//dep.
		if (!PartParser(strResponse, L"<td class=\"info_row depart\">", strTrainDep))
			break;
		if (strTrainDep.empty())
			break;

		//duration
		if (!PartParser(strResponse, L"<td class=\"info_row onway\">&nbsp;", strTrainDuration))
			break;
		if (strTrainDuration.empty())
			break;

		//arrive
		if (!PartParser(strResponse, L"<td class=\"info_row arrive\">",  strTrainArrive))
			break;
		if (strTrainArrive.empty())
			break;

		//lux
		PartParserWagon(strResponse, L" c_1050\">", strTrainLuxPrice, strTrainLuxSeat);

		//compartment firm
		PartParserWagon(strResponse, L" c_1040\">", strTrainCompartmentFirmPrice, strTrainCompartmentFirmSeat);

		//compartment
		PartParserWagon(strResponse, L" c_1030\">", strTrainCompartmentPrice, strTrainCompartmentSeat);

		//third class firm
		PartParserWagon(strResponse, L" c_1025\">", strTrainThirdClassFirmPrice, strTrainThirdClassFirmSeat);

		//third class
		PartParserWagon(strResponse, L" c_1020\">", strTrainThirdClassPrice, strTrainThirdClassSeat);

		//left seats
		PartParserWagon(strResponse, L" c_1001 last\">", strTrainSeatsPrice, strTrainSeatsSeat);

		//sum
		wchar_t* strSumTrains = NULL;
		int nSumLenTrains = 4 * MAX_PATH + 
			strTrainNumber.size() + 
			strTrainDeparture.size() + 
			strTrainDestination.size() +
			strTrainDep.size() + 
			strTrainDuration.size() + 
			strTrainArrive.size() +
			(strTrainLuxPrice.empty() ? 0 : strTrainLuxPrice.size()) +
			(strTrainLuxSeat.empty() ? 0 : strTrainLuxSeat.size()) +
			(strTrainCompartmentFirmPrice.empty() ? 0 : strTrainCompartmentFirmPrice.size()) +
			(strTrainCompartmentFirmSeat.empty() ? 0 : strTrainCompartmentFirmSeat.size()) +
			(strTrainCompartmentPrice.empty() ? 0 : strTrainCompartmentPrice.size()) +
			(strTrainCompartmentSeat.empty() ? 0 : strTrainCompartmentSeat.size()) +
			(strTrainThirdClassFirmPrice.empty() ? 0 : strTrainThirdClassFirmPrice.size()) +
			(strTrainThirdClassFirmSeat.empty() ? 0 :  strTrainThirdClassFirmSeat.size()) +
			(strTrainThirdClassPrice.empty() ? 0 : strTrainThirdClassPrice.size()) +
			(strTrainThirdClassSeat.empty() ? 0 :  strTrainThirdClassSeat.size()) +
			(strTrainSeatsPrice.empty() ? 0 : strTrainSeatsPrice.size()) +
			(strTrainSeatsSeat.empty() ? 0 :  strTrainSeatsSeat.size());

		strSumTrains  = new wchar_t[nSumLenTrains];
		ASSERT(strSumTrains);
		memset((void*)strSumTrains, 0, nSumLenTrains * sizeof(wchar_t));
		_stprintf_s(strSumTrains, nSumLenTrains, 
			L"\"train\": {%s\"number\": \"%s\", \"departure\": \"%s\", \"destination\": \"%s\", \"dep.\": \"%s\", \"duration\": \"%s\", \"arrive\": \"%s\", %s\"lux\": {\"price\": \"%s\", \"seats\": \"%s\"}, \"compartment_firm\": {\"price\": \"%s\", \"seats\": \"%s\"}, \"compartment\": {\"price\": \"%s\", \"seats\": \"%s\"}, %s\"third_class_firm\": {\"price\": \"%s\", \"seats\": \"%s\"},  \"third_class\": {\"price\": \"%s\", \"seats\": \"%s\"}, \"left_seats\": {\"price\": \"%s\", \"seats\": \"%s\"} },%s", 
			CR, strTrainNumber.c_str(), strTrainDeparture.c_str(), strTrainDestination.c_str(), strTrainDep.c_str(), strTrainDuration.c_str(), strTrainArrive.c_str(),
			CR, strTrainLuxPrice.c_str(), strTrainLuxSeat.c_str(),
			strTrainCompartmentFirmPrice.c_str(), strTrainCompartmentFirmSeat.c_str(),
			strTrainCompartmentPrice.c_str(), strTrainCompartmentSeat.c_str(),
			CR, strTrainThirdClassFirmPrice.c_str(), strTrainThirdClassFirmSeat.c_str(),
			strTrainThirdClassPrice.c_str(), strTrainThirdClassSeat.c_str(),
			strTrainSeatsPrice.c_str(),  strTrainSeatsSeat.c_str(), 
							CR);
		vecstrTrains.push_back(strSumTrains);
		delete [] strSumTrains;
	}

	int nSumLen = MAX_PATH + strDate.size() + strFrom.size() + strTo.size();
	wchar_t* strSum = new wchar_t[nSumLen];
	ASSERT(strSum);
	memset((void*)strSum, 0, nSumLen * sizeof(wchar_t));
	_stprintf_s(strSum, nSumLen, L"{\"target\":{%s\"date\": \"%s\",%s\"from\": \"%s\",%s\"to\": \"%s\"%s},%s\"trains\":{%s", 
		CR, strDate.c_str(),CR,strFrom.c_str(),CR,strTo.c_str(),CR,CR,CR, CR);
	strJSONResult = strSum;
	delete [] strSum;
	if (!vecstrTrains.empty())
	{
		nLen = vecstrTrains.size();
		for (i = 0; i < nLen; i++)
		{
			strJSONResult.append(vecstrTrains[i]);
		}
	}
	strJSONResult.append(L"},");
	strJSONResult.append(CR);
	strJSONResult.append(L"}");
}


void CRailTickesDlg::OnBnClickedRadioBooking()
{
	UpdateData(true);
	if (!m_nBooking)
	{
		wstring strResponse;
		if (!SendRequestForToken(L"http://booking.uz.gov.ua/ru/", strResponse))
		{
			AfxMessageBox(strResponse.c_str());
			return;
		}
		else
			m_strResponseCookies = strResponse;
	}

}


void CRailTickesDlg::OnBnClickedRadioDprc()
{
	UpdateData(true);
	if (!m_nBooking)
	{
		wstring strResponse;
		if (!SendRequestForToken(L"http://booking.uz.gov.ua/ru/", strResponse))
		{
			AfxMessageBox(strResponse.c_str());
			return;
		}
		else
			m_strResponseCookies = strResponse;
	}
}

duk_ret_t CRailTickesDlg::get_result_token (duk_context *ctx)
{
	if (m_pCRailTickesDlg)
	{
		m_pCRailTickesDlg->m_strToken = L"";
		string  strResult = duk_require_string(ctx, 0);
		int nIndex = strResult.find("\"gv-token\", \"");
		if (nIndex != string::npos)
		{
			nIndex += strlen("\"gv-token\", \"");
			strResult = strResult.substr(nIndex);
			int nLen = strResult.size();
			for (int i =0; i < nLen; i++)
			{
				char c = strResult.at(i);
				if (c == '\"')
					break;
				m_pCRailTickesDlg->m_strToken += CString(c);
			}
		}
	}
	duk_push_null(ctx);
	return 1;
}
