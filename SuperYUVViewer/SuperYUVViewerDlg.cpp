// SuperYUVViewerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SuperYUVViewer.h"
#include "SuperYUVViewerDlg.h"
#include "SrcDialog.h"
#include <string>
using namespace std;


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/************************************************************************/
/* Multi thread                                                         */
/************************************************************************/
typedef struct  
{
	CSuperYUVViewerDlg	*pYUVDialog;
	CSrcDialog						*pSrcDialog;
	UINT	u32SleepMS;					//sleep time
	bool		*bPlay;
}STRUCTINPUTPAR;

STRUCTINPUTPAR *pStructIPar;
HANDLE	hPressPlayEvent;					//event that button play is pressed
HANDLE	hLoopPlayThread;
extern	HANDLE hReadOverEvent;

DWORD WINAPI LoopPlay(STRUCTINPUTPAR * lpPar);		//for play button
DWORD WINAPI LoopPlay(STRUCTINPUTPAR * lpPar)
{
	while (TRUE)
	{
		WaitForSingleObject(hPressPlayEvent, INFINITE);
		if (!((STRUCTINPUTPAR *)lpPar->pSrcDialog->DisplayAPicture(1)))
		{
			*((STRUCTINPUTPAR *)lpPar)->bPlay = TRUE;
			((STRUCTINPUTPAR *)lpPar)->pYUVDialog->SetDlgItemText(IDC_BUTTONPlay, _T("PLAY"));
			return -1;
		}			
		Sleep(((STRUCTINPUTPAR *)lpPar)->u32SleepMS);
	}
	return 0;
}




// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CSuperYUVViewerDlg dialog
CSuperYUVViewerDlg::CSuperYUVViewerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSuperYUVViewerDlg::IDD, pParent)
	, m_pDlgSrcVideo(NULL)
	, m_ucSrcvideoformat(0)
	, m_u32Width(0)
	, m_u32Height(0)
	, m_bPlay(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSuperYUVViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDITSourceFile, m_ctrEditSourceFile);

	DDX_Control(pDX, IDC_COMBO1, m_cCBPicSize);
}

BEGIN_MESSAGE_MAP(CSuperYUVViewerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTONOPEN1, &CSuperYUVViewerDlg::OnBnClickedButtonopen1)
	ON_BN_CLICKED(IDC_RADIO2, &CSuperYUVViewerDlg::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO3, &CSuperYUVViewerDlg::OnBnClickedRadio3)
	ON_BN_CLICKED(IDC_RADIO4, &CSuperYUVViewerDlg::OnBnClickedRadio4)
	ON_WM_CLOSE()
	ON_CBN_SELCHANGE(IDC_COMBOPicSize, &CSuperYUVViewerDlg::OnCbnSelchangeCombopicsize)
	ON_BN_CLICKED(IDC_CHECK5, &CSuperYUVViewerDlg::OnBnClickedCheck5)
	ON_BN_CLICKED(IDC_BUTTON3, &CSuperYUVViewerDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CSuperYUVViewerDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTONPlay, &CSuperYUVViewerDlg::OnBnClickedButtonplay)
	ON_BN_CLICKED(IDC_BUTTONClear, &CSuperYUVViewerDlg::OnBnClickedButtonclear)
	ON_BN_CLICKED(IDC_BUTTONReset, &CSuperYUVViewerDlg::OnBnClickedButtonreset)
	ON_EN_UPDATE(IDC_EDITSourceFile, &CSuperYUVViewerDlg::OnEnUpdateEditsourcefile)
	ON_EN_SETFOCUS(IDC_EDITSourceFile, &CSuperYUVViewerDlg::OnEnSetfocusEditsourcefile)
	ON_WM_MOVING()
	ON_WM_MOVE()
	ON_WM_CREATE()
	ON_WM_DROPFILES()
END_MESSAGE_MAP()


// CSuperYUVViewerDlg message handlers

BOOL CSuperYUVViewerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	/*initialize dialog face*/
	CRect rectSeparator;
	::GetWindowRect(this->m_hWnd, &m_rectLarge);		
	GetDlgItem(IDC_SEPARATOR)->GetWindowRect(&rectSeparator);

	m_rectSmall.left = m_rectLarge.left;
	m_rectSmall.right = m_rectLarge.right;
	m_rectSmall.top	= m_rectLarge.top;
	m_rectSmall.bottom = rectSeparator.bottom;	
	SetWindowPos(NULL, 0, 0, m_rectSmall.Width(), m_rectSmall.Height(), SWP_NOMOVE|SWP_NOZORDER);

	/*initialize radio button*/
	CButton *pRadioBnt = (CButton *)GetDlgItem(IDC_RADIOYUV420);
	pRadioBnt->SetCheck(1);
	m_ucSrcvideoformat = YUV420;

	/*initialize combo box*/
	m_cCBPicSize.AddString(_T("QCIF"));
	m_cCBPicSize.AddString(_T("QVGA"));
	m_cCBPicSize.AddString(_T("CIF"));
	m_cCBPicSize.AddString(_T("VGA"));
	m_cCBPicSize.AddString(_T("4CIF"));
	m_cCBPicSize.AddString(_T("720P"));
	m_cCBPicSize.AddString(_T("OTHER"));
	m_cCBPicSize.SetCurSel(0);
	m_u32Width	= 176;
	m_u32Height	= 144;

	/*set default step size*/
	SetDlgItemInt(IDC_EDITStepSize, 1, FALSE);

	/*respond to drag operation*/
	CWnd::DragAcceptFiles(TRUE);


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSuperYUVViewerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSuperYUVViewerDlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSuperYUVViewerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CSuperYUVViewerDlg::OnBnClickedButtonopen1()
{
	// TODO: Add your control notification handler code here
	/*return if the display window is already opened*/
	if (IsVideoWindowOpen())
	{
		MessageBox(_T("video already opened!"));
		return;
	}


	//
	m_bPlay = FALSE;
	SetDlgItemText(IDC_BUTTONPlay, _T("PLAY"));

	/*get source file*/
	CString strFilePath;
	
	if (0 == GetDlgItemText(IDC_EDITSourceFile, strFilePath))
	{
		/*open source file*/
		CFileDialog cFileOpenDlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("Yuv Files (*.yuv)|*.yuv|264 Files (*.264)|*.264|All Files (*.*)|*.*||"), NULL, 0, 0);
		cFileOpenDlg.m_ofn.lpstrTitle = TEXT("Source file");

		INT retVal = cFileOpenDlg.DoModal();
		CString strExt = cFileOpenDlg.GetFileExt();
		strFilePath = cFileOpenDlg.GetPathName();

		while(retVal!=IDCANCEL && strExt!="264" && strExt!="yuv")
		{
			MessageBox(TEXT("Invalid input file!"));
			retVal = cFileOpenDlg.DoModal();

			strExt = cFileOpenDlg.GetFileExt();
			strFilePath = cFileOpenDlg.GetFolderPath();
		}
		if (retVal == IDCANCEL)
			return;
	}	

	/*get input file name*/
	SetDlgItemText(IDC_EDITSourceFile, strFilePath);
	
	/*create the dialog to display source video*/
	if (m_pDlgSrcVideo)	{delete	m_pDlgSrcVideo;}
	m_pDlgSrcVideo = new CSrcDialog(strFilePath, m_ucSrcvideoformat, m_u32Width, m_u32Height);
	if (m_pDlgSrcVideo->Create(IDD_DIALOGSourcevideo, GetDesktopWindow()))
	{
		//the following read the whole input file, but only display the first picture
		m_pDlgSrcVideo->ReadAndDisplayVideo();
	}
	else
	{
		MessageBox(_T("video display failed."));
	}

	/*create thread and event for button play */
	pStructIPar = (STRUCTINPUTPAR *)calloc(1, sizeof(STRUCTINPUTPAR));
	pStructIPar->pYUVDialog	= this;
	pStructIPar->pSrcDialog = m_pDlgSrcVideo;
	pStructIPar->u32SleepMS = GetSleepTime();	//sleep so that the video won't play so fast
	pStructIPar->bPlay		= &m_bPlay;
	//create event
	hPressPlayEvent	= ::CreateEvent(NULL,TRUE, FALSE, NULL);
	hLoopPlayThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)LoopPlay, pStructIPar, 0, NULL);
	//close handles
	//CloseHandle(hPressPlayEvent);
	CloseHandle(hLoopPlayThread);
}

//on radio button yuv420
void CSuperYUVViewerDlg::OnBnClickedRadio1()
{
	// TODO: Add your control notification handler code here
	m_ucSrcvideoformat = YUV420;
}
//on radio button yuv422
void CSuperYUVViewerDlg::OnBnClickedRadio2()
{
	// TODO: Add your control notification handler code here
	m_ucSrcvideoformat = YUV422;
}

//on radio button yuv444
void CSuperYUVViewerDlg::OnBnClickedRadio3()
{
	// TODO: Add your control notification handler code here
	m_ucSrcvideoformat = YUV444;
}

//on radio button 264
void CSuperYUVViewerDlg::OnBnClickedRadio4()
{
	// TODO: Add your control notification handler code here
	m_ucSrcvideoformat = BS264;
}

void CSuperYUVViewerDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	CloseHandle(hPressPlayEvent);
	if (m_pDlgSrcVideo)
	{
		delete m_pDlgSrcVideo;
		m_pDlgSrcVideo	= NULL;
	}
	DestroyWindow();
}

void CSuperYUVViewerDlg::OnCbnSelchangeCombopicsize()
{
	// TODO: Add your control notification handler code here
	CString strPicFmt;
	int		iSel;
	iSel = m_cCBPicSize.GetCurSel();
	m_cCBPicSize.GetLBText(iSel, strPicFmt);	
	UpdateData(TRUE);

	if (iSel<6)
	{
		GetPictureSize((char)iSel, m_u32Width, m_u32Height);
		return;
	}


	/*get picture size specified by the user*/
	CPicSizeDlg cPSDlg;
	INT ret;
	ret	= cPSDlg.DoModal();

	m_u32Width		= (unsigned int)_tcstoul(cPSDlg.m_strWidth, NULL, 10);
	m_u32Height	= (unsigned int)_tcstoul(cPSDlg.m_strHeight, NULL, 10);

	while (ret != IDCANCEL && (m_u32Height==0 || m_u32Width==0))
	{
		ret	= cPSDlg.DoModal();
		m_u32Width		= (unsigned int)_tcstoul(cPSDlg.m_strWidth, NULL, 10);
		m_u32Height	= (unsigned int)_tcstoul(cPSDlg.m_strHeight, NULL, 10);
	}
	//set defalut picture size
	if (ret == IDCANCEL)
	{
		m_cCBPicSize.SetCurSel(0);
		m_u32Width		= 176;
		m_u32Height	= 144;
	}

}

// get picture width and height according to the picture format
void CSuperYUVViewerDlg::GetPictureSize(char cFmt, unsigned int & u32Width, unsigned int & u32Height)
{
	switch (cFmt)
	{
	case 0:
		u32Width	= 176;
		u32Height	= 144;
		break;
	case 1:
		u32Width	= 320;
		u32Height	= 240;
		break;
	case 2:
		u32Width	= 352;
		u32Height	= 288;
		break;
	case 3:
		u32Width	= 640;
		u32Height	= 480;
		break;
	case 4:
		u32Width	= 704;
		u32Height	= 576;
		break;
	case 5:
		u32Width	= 1280;
		u32Height	= 720;
		break;
	}
	return;
}

void CSuperYUVViewerDlg::OnBnClickedCheck5()
{
	// TODO: Add your control notification handler code here
	
	if (BST_CHECKED == ((CButton *)GetDlgItem(IDC_CHECKPSNR))->GetCheck())
		SetWindowPos(NULL, 0, 0, m_rectLarge.Width(), m_rectLarge.Height(), SWP_NOMOVE|SWP_NOZORDER);
	else
		SetWindowPos(NULL, 0, 0, m_rectSmall.Width(), m_rectSmall.Height(), SWP_NOMOVE|SWP_NOZORDER);

}

//on button next
void CSuperYUVViewerDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	BOOL bTrans = TRUE;
	int i32Step = GetDlgItemInt(IDC_EDITStepSize, &bTrans, FALSE);
	m_pDlgSrcVideo->DisplayAPicture(i32Step);
}

//on button previous
void CSuperYUVViewerDlg::OnBnClickedButton4()
{
	// TODO: Add your control notification handler code here
	BOOL bTrans = TRUE;
	int i32Step = GetDlgItemInt(IDC_EDITStepSize, &bTrans, FALSE);
	m_pDlgSrcVideo->DisplayAPicture(-i32Step);
}

void CSuperYUVViewerDlg::OnBnClickedButtonplay()
{
	// TODO: Add your control notification handler code here	
	if (IsVideoWindowOpen())
	{
		CString strButton;
		GetDlgItemText(IDC_BUTTONPlay, strButton);
		if (strButton == _T("PLAY"))
		{
			if (m_bPlay )
			{
				hLoopPlayThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)LoopPlay, pStructIPar, 0, NULL);
				CloseHandle(hLoopPlayThread);
				m_bPlay	= FALSE;
			}
			WaitForSingleObject(hReadOverEvent, INFINITE);
			ResetEvent(hReadOverEvent);
			//set event
			SetEvent(hPressPlayEvent);
			SetDlgItemText(IDC_BUTTONPlay, _T("PAUSE"));
		}
		else
		{
			ASSERT(strButton == _T("PAUSE"));
			//reset signal
			::ResetEvent(hPressPlayEvent);
			SetDlgItemText(IDC_BUTTONPlay, _T("PLAY"));
		}		
	}
}


void CSuperYUVViewerDlg::OnBnClickedButtonclear()
{
	// TODO: Add your control notification handler code here
	// clear source file name, if display window has been closed.
	if (!IsVideoWindowOpen()/*NULL==m_pDlgSrcVideo || !m_pDlgSrcVideo->IsWindowVisible()*/)
	{
		CString strNUll = _T("");
		SetDlgItemText(IDC_EDITSourceFile, strNUll);
		SetDlgItemText(IDC_BUTTONPlay, _T("PLAY"));
	}
}

void CSuperYUVViewerDlg::OnBnClickedButtonreset()
{
	// TODO: Add your control notification handler code here
	if (IsVideoWindowOpen())
	{
		m_pDlgSrcVideo->SetFrameNumber(0);
		m_pDlgSrcVideo->DisplayAPicture(0);
		SetDlgItemText(IDC_BUTTONPlay, _T("PLAY"));
		ResetEvent(hPressPlayEvent);
	}
}

void CSuperYUVViewerDlg::OnEnUpdateEditsourcefile()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function to send the EM_SETEVENTMASK message to the control
	// with the ENM_UPDATE flag ORed into the lParam mask.

	// TODO:  Add your control notification handler code here

}

void CSuperYUVViewerDlg::OnEnSetfocusEditsourcefile()
{
	// TODO: Add your control notification handler code here
	CEdit *cEditbox = (CEdit *)GetDlgItem(IDC_EDITSourceFile);
	if (m_pDlgSrcVideo && m_pDlgSrcVideo->IsWindowVisible())
	{
		cEditbox->SetReadOnly(TRUE);
		SetDlgItemText(IDC_EDITSourceFile, m_pDlgSrcVideo->GetSourceFileName());
		return;
	}
	cEditbox->SetReadOnly(FALSE);
	
}

bool CSuperYUVViewerDlg::IsVideoWindowOpen(void)
{
	//if (m_pDlgSrcVideo && m_pDlgSrcVideo->IsWindowVisible())
	if (m_pDlgSrcVideo && NULL != m_pDlgSrcVideo->GetSafeHwnd())
		return TRUE;

	return FALSE;
}

void CSuperYUVViewerDlg::OnMoving(UINT fwSide, LPRECT pRect)
{
	CDialog::OnMoving(fwSide, pRect);

	//OnPaint();

	// TODO: Add your message handler code here
}

void CSuperYUVViewerDlg::OnMove(int x, int y)
{
	CDialog::OnMove(x, y);
	//OnPaint();

	// TODO: Add your message handler code here
}

int CSuperYUVViewerDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  Add your specialized creation code here

	return 0;
}

unsigned int CSuperYUVViewerDlg::GetSleepTime(void)
{
	if (m_u32Width<=352)
		return SLEEPTIMESMALL;
	else if (m_u32Width<=704)
		return SLEEPTIMEMEDIUM;
	else if (m_u32Width<=1280)
		return SLEEPTIMELARGE;
	else
		return SLEEPTIMEHD;
}

void CSuperYUVViewerDlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO: Add your message handler code here and/or call default
	WCHAR strPath[MAX_PATH];
	DragQueryFile(hDropInfo, 0, (LPWSTR)strPath, MAX_PATH);
	SetDlgItemText(IDC_EDITSourceFile, _T(""));
	SetDlgItemText(IDC_EDITSourceFile, LPCTSTR(strPath));

	//CDialog::OnDropFiles(hDropInfo);
}
