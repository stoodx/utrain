
// RailTickesDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include <vector>
#include "afxdtctl.h"


struct Station;
class CUtrainControl;

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

	enum
	{
		timerRefreshSession = 0
	};

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
	void ParserDPRC(std::wstring& strResponse, std::wstring& strJSONResult);
	bool PartParser(std::wstring& strResponse, const wchar_t* str, std::wstring& strTarget);
	bool PartParserWagon(std::wstring& strResponse, const wchar_t* str, std::wstring& strPrice, std::wstring& strSeats);
	std::string UTF16toUTF8(const std::wstring strUTF16); 
	std::string UrlEncode(const std::string str);
	std::string Char2hex(char c);
	std::wstring RequestBookong();
	std::wstring RequestDPRC();

protected:
	CComboBox m_comboFrom;
	CComboBox m_comboTo;
private:
	std::vector<Station*> m_vecpStationsFrom;
	std::vector<Station*> m_vecpStationsTo;
	CUtrainControl* m_pCUtrainControl;


public:
	afx_msg void OnClose();
	afx_msg void OnCbnSelchangeComboAFrom();
	afx_msg void OnCbnSelchangeComboATo();
	afx_msg void OnBnClickedOk();
protected:
	CMonthCalCtrl m_calendar;
	CButton m_btnSearch;
	int m_nBooking;
public:
	afx_msg void OnBnClickedRadioBooking();
	afx_msg void OnBnClickedRadioDprc();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
