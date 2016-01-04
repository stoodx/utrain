
// RailTickesDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include <vector>
#include "afxdtctl.h"


struct Station;

// CRailTickesDlg dialog
class CRailTickesDlg : public CDialogEx
{
// Construction
public:
	CRailTickesDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_RAILTICKES_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	CComboBox m_comboA_From;
	CComboBox m_comboA_To;
private:
	bool FillStations(CComboBox& comboA, CComboBox& comboStation, std::vector<Station*>& vecpStations);
	bool FillStationsDPRC(CComboBox& comboA, CComboBox& comboStation, std::vector<Station*>& vecpStations);
	bool FillStationsBooking(CComboBox& comboA, CComboBox& comboStation, std::vector<Station*>& vecpStations);
	void CleanStations(std::vector<Station*>* pvecpStations);
	void ParserBooking(std::wstring& strResponse, std::wstring& strJSONResult);
	void ParserDPRC(std::wstring& strResponse, std::wstring& strJSONResult);
	bool PartParser(std::wstring& strResponse, const wchar_t* str, std::wstring& strTarget);
	bool PartParserWagon(std::wstring& strResponse, const wchar_t* str, std::wstring& strPrice, std::wstring& strSeats);
	std::wstring PrintUTF16Converter(std::wstring& str);
	std::string UTF16toUTF8(const std::wstring strUTF16);
	CString UrlEncode(CString str);
	CString Char2hex(TCHAR c);
protected:
	CComboBox m_comboFrom;
	CComboBox m_comboTo;
private:
	std::vector<Station*> m_vecpStationsFrom;
	std::vector<Station*> m_vecpStationsTo;
public:
	afx_msg void OnClose();
	afx_msg void OnCbnSelchangeComboAFrom();
	afx_msg void OnCbnSelchangeComboATo();
	afx_msg void OnBnClickedOk();
protected:
	CMonthCalCtrl m_calendar;
	CButton m_btnSearch;
	int m_nBooking;
};
