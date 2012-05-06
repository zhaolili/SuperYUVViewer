// SuperYUVViewerDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "SrcDialog.h"
#include "PicSizeDlg.h"
#include "DDrawDisplay.h"
#include "atltypes.h"


// CSuperYUVViewerDlg dialog
class CSuperYUVViewerDlg : public CDialog
{
// Construction
public:
	CSuperYUVViewerDlg(CWnd* pParent = NULL);	// standard constructor


// Dialog Data
	enum { IDD = IDD_SUPERYUVVIEWER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedButtonopen1();

	// source file name, path included
	CEdit m_ctrEditSourceFile;
	// pointer to the dialog displaying source video
	CSrcDialog *m_pDlgSrcVideo;
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedRadio3();
	afx_msg void OnBnClickedRadio4();
private:
	unsigned char m_ucSrcvideoformat;
public:
	afx_msg void OnClose();
	CComboBox m_cCBPicSize;
	afx_msg void OnCbnSelchangeCombopicsize();
private:
	// picture width
	unsigned int m_u32Width;
	// picture height
	unsigned int m_u32Height;
	// get picture width and height according to the picture format
	void GetPictureSize(char cFmt, unsigned int & u32Width, unsigned int & u32Height);
public:
	afx_msg void OnBnClickedCheck5();
private:
	CRect m_rectLarge;
	CRect m_rectSmall;
public:
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButtonplay();
private:
	bool m_bPlay;
public:
	afx_msg void OnBnClickedButtonclear();
	afx_msg void OnBnClickedButtonreset();
	afx_msg void OnEnUpdateEditsourcefile();
	afx_msg void OnEnSetfocusEditsourcefile();
private:
	bool IsVideoWindowOpen(void);
public:
	afx_msg void OnMoving(UINT fwSide, LPRECT pRect);
	afx_msg void OnMove(int x, int y);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
private:
	unsigned int GetSleepTime(void);
public:
	afx_msg void OnDropFiles(HDROP hDropInfo);
};
