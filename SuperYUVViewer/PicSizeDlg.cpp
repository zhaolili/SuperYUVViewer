// PicSizeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SuperYUVViewer.h"
#include "PicSizeDlg.h"


// CPicSizeDlg dialog

IMPLEMENT_DYNAMIC(CPicSizeDlg, CDialog)

CPicSizeDlg::CPicSizeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPicSizeDlg::IDD, pParent)
	, m_strWidth(_T(""))
	, m_strHeight(_T(""))
{

}

CPicSizeDlg::~CPicSizeDlg()
{
}

void CPicSizeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPicSizeDlg, CDialog)
	ON_EN_CHANGE(IDC_EDITWidth, &CPicSizeDlg::OnEnChangeEditwidth)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CPicSizeDlg message handlers

void CPicSizeDlg::OnEnChangeEditwidth()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here
}

void CPicSizeDlg::OnDestroy()
{

	GetDlgItemText(IDC_EDITWidth, m_strWidth);
	GetDlgItemText(IDC_EDITHeight, m_strHeight);

	CDialog::OnDestroy();

	// TODO: Add your message handler code here
}
