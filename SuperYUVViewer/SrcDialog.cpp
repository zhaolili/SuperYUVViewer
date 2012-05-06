// SrcDialog.cpp : implementation file
//

#include "stdafx.h"
#include "SuperYUVViewer.h"
#include "SrcDialog.h"

/************************************************************************/
/* Multi thread                                                         */
/************************************************************************/
extern	HANDLE hLoopPlayThread;
extern	HANDLE	 hPressPlayEvent;

HANDLE hReadOverEvent;	//read file finished event
HANDLE hReadFileThread;	//read file thread
typedef struct  
{
	CFile				*pFile;
	unsigned char	*ucSrcBuf;
	unsigned long	framesize;
	unsigned char	ucCounter;
}STRUCTINPUTPAR2;

//STRUCTINPUTPAR2 *pStructIPar;

DWORD WINAPI ReadFileFunc(STRUCTINPUTPAR2* lpPar)
{
	char ucCounter = ((STRUCTINPUTPAR2* )lpPar)->ucCounter;
	((STRUCTINPUTPAR2* )lpPar)->pFile->Read(((STRUCTINPUTPAR2* )lpPar)->ucSrcBuf, ((STRUCTINPUTPAR2* )lpPar)->framesize);
	/*((STRUCTINPUTPAR2* )lpPar)->pFile->Close();
	delete ((STRUCTINPUTPAR2* )lpPar)->pFile;*/
	SetEvent(hReadOverEvent);
	return 1;
}


// CSrcDialog dialog

IMPLEMENT_DYNAMIC(CSrcDialog, CDialog)

CSrcDialog::CSrcDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CSrcDialog::IDD, pParent)
	, m_strSourceFileName(_T(""))
	, m_ucSrcfileformat(0)
	, m_u32Width(0)
	, m_u32Height(0)
	, m_ucVideoDataBuffer(NULL)
	, m_pDDrawDisp(NULL)
	, m_ulFrameNumber(0)
	, m_ulTotalFrames(0)
	, m_u32FrameSize(0)
	, m_pFile(NULL)
{

}

CSrcDialog::~CSrcDialog()
{
	/*release memory*/
	if (m_ucVideoDataBuffer)
	{
		delete[] m_ucVideoDataBuffer;
		m_ucVideoDataBuffer = NULL;
	}
	if (m_pFile)
	{
		m_pFile->Close();
		delete m_pFile;
		m_pFile = NULL;
	}
	/**/
	delete m_pDDrawDisp;
	CloseHandle(hReadOverEvent);
}

void CSrcDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSrcDialog, CDialog)
	ON_BN_CLICKED(IDOK, &CSrcDialog::OnBnClickedOk)
	ON_WM_PAINT()
	ON_WM_CLOSE()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()


// CSrcDialog message handlers

BOOL CSrcDialog::PreTranslateMessage(MSG* pMsg)
{
	CDialog::PreTranslateMessage(pMsg);

	//
	if (m_cToolTip.m_hWnd!=NULL)
		m_cToolTip.RelayEvent(pMsg);

	return FALSE;
}

BOOL CSrcDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here

	/************************************************************************/
	/* initialize variables                                                  */
	/************************************************************************/
	m_ulFrameNumber	= 0;

	/************************************************************************/
	/* set the title bar                                                    */
	/************************************************************************/
	CString strTitle;
	strTitle.Format(_T("%d"), (int)m_ulFrameNumber);
	SetWindowText(strTitle);

	/************************************************************************/
	/* set the display dialog to the picture size                           */
	/************************************************************************/
	//Get dialog screen position
	CRect rectWindow;
	GetWindowRect(&rectWindow);
	//get client position
	CRect rectClient;
	GetClientRect(&rectClient);
	//convert client coordinates to screen coordinates
	ClientToScreen(&rectClient);
	//reset size
	int	cx, cy, dx, dy_top, dy_bot;
	dx = rectClient.left - rectWindow.left;
	dx = (dx>0) ? dx :(-dx);
	dy_top	= rectClient.top - rectWindow.top;
	dy_top	= (dy_top>0) ? dy_top : (-dy_top);
	dy_bot	= rectWindow.bottom - rectClient.bottom;
	dy_bot	= (dy_bot>0) ? dy_bot : (-dy_bot);
	cx = m_u32Width + (dx<<1);
	cy = m_u32Height + dy_bot + dy_top;
	::SetWindowPos(this->m_hWnd, HWND_BOTTOM, 0, 0, cx, cy, SWP_NOMOVE);
	
	/************************************************************************/
	/* initialize directdraw object                                         */
	/************************************************************************/
	m_pDDrawDisp	= new CDDrawDisplay(m_ucSrcfileformat);
	if(!m_pDDrawDisp->DDrawDispInit(this->m_hWnd, m_u32Width, m_u32Height, m_u32Width, m_u32Height))
		return FALSE;

	/************************************************************************/
	/* create tool tip                                                      */
	/************************************************************************/
	m_cToolTip.Create(this);
	m_cToolTip.AddTool(this, TTS_ALWAYSTIP);
	m_cToolTip.SetTipTextColor(RGB(255,0,0));
	m_cToolTip.SetDelayTime(INFINITE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

CSrcDialog::CSrcDialog(CString strSrcFile, unsigned char ucSrcfileformat, unsigned int u32Width, unsigned int u32Height)
{
	//initialization
	m_pDDrawDisp	= NULL;
	m_ucVideoDataBuffer	= NULL;
	m_strSourceFileName	= strSrcFile;
	m_ucSrcfileformat			= ucSrcfileformat;
	m_u32Height				= u32Height;
	m_u32Width					= u32Width;

	long lLumasize;
	switch (m_ucSrcfileformat)
	{
	case YUV420:
		lLumasize = m_u32Width*m_u32Height;
		m_u32FrameSize =  lLumasize+(lLumasize>>1);
		break;
	case YUV422:
	case YUV444:
		m_u32FrameSize = 0;
		break;
	}
}

void CSrcDialog::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	OnOK();
}

void CSrcDialog::PostNcDestroy()
{
	// TODO: Add your specialized code here and/or call the base class

	CDialog::PostNcDestroy();
}

// read source file and display video
bool CSrcDialog::ReadAndDisplayVideo(void)
{
	/************************************************************************/
	/*read a frame                                                          */
	/************************************************************************/
	m_pFile = new CFile(m_strSourceFileName, CFile::modeRead);
	ULONGLONG	ullFileLen = m_pFile->GetLength();
	m_pFile->SeekToBegin();
	unsigned long	ulFrameSize = GetFrameSize();	
	m_ulTotalFrames = ullFileLen/ulFrameSize;

	if (m_ucVideoDataBuffer!=NULL)
	{
		delete[] m_ucVideoDataBuffer;
		m_ucVideoDataBuffer = NULL;
	}
	//memory allocation, size: one frame
	m_ucVideoDataBuffer = (unsigned char *)calloc(ulFrameSize, sizeof(unsigned char));
	if (m_ucVideoDataBuffer == NULL)
		MessageBox(_T("Memory allocation failed."));

	m_pFile->Read(m_ucVideoDataBuffer, ulFrameSize);	
	//show window
	this->ShowWindow(SW_SHOW);
	/************************************************************************/
	/* display																																   */
	/************************************************************************/
	unsigned int uiSize = m_u32Height*m_u32Width;
	unsigned char *srcBuf[3] = {m_ucVideoDataBuffer, m_ucVideoDataBuffer+uiSize, m_ucVideoDataBuffer+uiSize+(uiSize>>2)};
	int	iStride[3] = {m_u32Width, m_u32Width>>1, m_u32Width>>1};
	if (m_pDDrawDisp)
	{
		m_pDDrawDisp->DrawUpdateDisp(srcBuf, iStride);
	}

	return true;
}

void CSrcDialog::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CDialog::OnPaint() for painting messages
	/*currently designed for YUV420*/
	unsigned int uiSizeluma = m_u32Height*m_u32Width;
	unsigned int uiSize		= uiSizeluma+(uiSizeluma>>1);
	unsigned char *srcBufStart = m_ucVideoDataBuffer; 
	unsigned char *srcBuf[3] = {srcBufStart, srcBufStart+uiSizeluma, srcBufStart+uiSizeluma+(uiSizeluma>>2)};
	int	iStride[3] = {m_u32Width, m_u32Width>>1, m_u32Width>>1};
	if (m_pDDrawDisp)
	{
		m_pDDrawDisp->DrawUpdateDisp(srcBuf, iStride);
	}
}

BOOL CSrcDialog::DisplayAPicture(int i32FrameStep)
{
	//update frame number
	int		i32FrameNO = m_ulFrameNumber + i32FrameStep;
	i32FrameNO = (i32FrameNO>0) ? i32FrameNO : 0;
	if (i32FrameNO>=m_ulTotalFrames)
	{
		m_ulFrameNumber = m_ulTotalFrames - 1;
		MessageBox(_T("Reach the end of file!"));
		return FALSE;
	}
	m_ulFrameNumber = i32FrameNO;
	
	//WaitForSingleObject(hReadOverEvent, INFINITE);	//wait until reading is over
	//set title bar frame number
	CString strFrameNo;
	strFrameNo.Format(_T("%d"), m_ulFrameNumber);
	SetWindowText(strFrameNo);

	//display a picture
	switch (m_ucSrcfileformat)
	{
	case YUV420:
		DisplayAPictureYUV420();
		break;
	case YUV422:
	case YUV444:
	default:
		break;
	}
	return TRUE;
}

void CSrcDialog::DisplayAPictureYUV420()
{
	unsigned int uiSizeluma = m_u32Height*m_u32Width;
	unsigned int uiSize		= uiSizeluma+(uiSizeluma>>1);
	unsigned char *srcBufStart = m_ucVideoDataBuffer; 
	unsigned char *srcBuf[3] = {srcBufStart, srcBufStart+uiSizeluma, srcBufStart+uiSizeluma+(uiSizeluma>>2)};
	int	iStride[3] = {m_u32Width, m_u32Width>>1, m_u32Width>>1};
	/************************************************************************/
	/* read pixel data to memory buffer                                                                     */
	/************************************************************************/
	m_pFile->SeekToBegin();
	m_pFile->Seek(uiSize*m_ulFrameNumber, CFile::begin);
	m_pFile->Read(m_ucVideoDataBuffer, uiSize);
	
	/************************************************************************/
	/* display                                                                     */
	/************************************************************************/
	if (m_pDDrawDisp)
	{
		m_pDDrawDisp->DrawUpdateDisp(srcBuf, iStride);
	}
}

// in bytes
unsigned long CSrcDialog::GetFrameSize(void)
{
	long	lLumasize;
	switch (m_ucSrcfileformat)
	{
	case YUV420:
		lLumasize = m_u32Height*m_u32Width;
		return lLumasize+(lLumasize>>1);
	case YUV422:
		return 0;
	case YUV444:
		return 0;
	}
	return 0;
}

void CSrcDialog::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	//CDialog::OnClose();

	ResetEvent(hPressPlayEvent);
	Sleep(50);
	DestroyWindow();
}

void CSrcDialog::SetFrameNumber(unsigned long ulFrameNumber)
{
	m_ulFrameNumber = ulFrameNumber;
}

CString CSrcDialog::GetSourceFileName(void)
{
	return m_strSourceFileName;
}

void CSrcDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	/************************************************************************/
	/* get pixel value                                                      */
	/************************************************************************/
	unsigned char unPixel[3]={0,0,0};		//YUV or RGB
	GetPixelValue(unPixel, point.x, point.y);

	/************************************************************************/
	/* get tool tip                                                         */
	/************************************************************************/
	CString	strPos;
	strPos.Format(_T("(%d,%d)Y[%d]"), point.x, point.y, unPixel[0]);

	m_cToolTip.UpdateTipText(strPos, this);
	CDialog::OnMouseMove(nFlags, point);
}

void CSrcDialog::GetPixelValue(unsigned char ucPixel[3], int iPosX, int iPosY)
{
	//currently only for YUV420

	//get frame number
	/*CString	strWndText;
	int		i32FrmNo;
	GetWindowText(strWndText);
	i32FrmNo = (unsigned int)_tcstoul(strWndText, NULL, 10);*/
	//get pixel value
	long lPos = iPosY*m_u32Width + iPosX;
	ucPixel[0] = m_ucVideoDataBuffer[lPos];	
}


