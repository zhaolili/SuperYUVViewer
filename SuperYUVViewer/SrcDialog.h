#pragma once
#include "CommonDef.h"
#include "DDrawDisplay.h"
#include "afxcmn.h"


// CSrcDialog dialog

class CSrcDialog : public CDialog
{
	DECLARE_DYNAMIC(CSrcDialog)

public:
	CSrcDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSrcDialog();

// Dialog Data
	enum { IDD = IDD_DIALOGSourcevideo };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual	BOOL PreTranslateMessage(MSG* pMsg);
private:
	// including file path
	CString m_strSourceFileName;
	// source file format
	unsigned char m_ucSrcfileformat;
	// image width
	unsigned int m_u32Width;
	// image height
	unsigned int m_u32Height;
	//store video data
	unsigned char *m_ucVideoDataBuffer;
public:
	CSrcDialog(CString strSrcFile, unsigned char ucSrcfileformat, unsigned int u32Width, unsigned int u32Height);
	afx_msg void OnBnClickedOk();
protected:
	virtual void PostNcDestroy();
public:
	// read source file and display video
	bool ReadAndDisplayVideo(void);
private:
	CDDrawDisplay *m_pDDrawDisp;
public:
	afx_msg void OnPaint();
private:
	// frame number from 0
	unsigned long m_ulFrameNumber;
public:
	BOOL DisplayAPicture(int i32FrameStep);
private:
	void DisplayAPictureYUV420();
	// in bytes
	unsigned long GetFrameSize(void);
	// total number of frames
	unsigned long m_ulTotalFrames;
public:
	afx_msg void OnClose();
	void SetFrameNumber(unsigned long ulFrameNumber);
	CString GetSourceFileName(void);
private:
	CToolTipCtrl m_cToolTip;
public:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
private:
	void GetPixelValue(unsigned char ucPixel[3], int iPosX, int iPosY);
	unsigned int m_u32FrameSize;
	CFile *m_pFile;
};
