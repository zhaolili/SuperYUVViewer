#pragma once


// CPicSizeDlg dialog

class CPicSizeDlg : public CDialog
{
	DECLARE_DYNAMIC(CPicSizeDlg)

public:
	CPicSizeDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPicSizeDlg();

// Dialog Data
	enum { IDD = IDD_DIALOGPicSize };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnEnChangeEditwidth();
	afx_msg void OnDestroy();
	CString m_strWidth;
	CString m_strHeight;
};
