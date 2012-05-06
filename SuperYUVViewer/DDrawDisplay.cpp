#include "StdAfx.h"
#include "DDrawDisplay.h"

#pragma comment(lib, "ddraw.lib")
#pragma comment(lib, "dxguid.lib")

CDDrawDisplay::CDDrawDisplay(void)
{
}
CDDrawDisplay::CDDrawDisplay(char yuvformat)
{
	m_chFormat = yuvformat;
}

CDDrawDisplay::~CDDrawDisplay(void)
{
	DDrawDispDone();
}

static DDPIXELFORMAT ddpfOverlayFormats[] = 
{
	{sizeof(DDPIXELFORMAT), DDPF_FOURCC|DDPF_YUV, MAKEFOURCC('Y', 'V', '1', '2'), 12, 0, 0, 0, 0}, //YUV420,Y ,V, U
	{sizeof(DDPIXELFORMAT), DDPF_FOURCC|DDPF_YUV, MAKEFOURCC('U', 'Y', 'V', 'Y'), 0, 0, 0, 0, 0}, //UYVY, one of YUV422 format
	//
};

#define  PF_TABLE_SIZE 2

BOOL CDDrawDisplay::DDrawDispInit(HWND hWnd, unsigned int src_width, unsigned int src_height, int dst_width,  int dst_height)
{
	/*get source and destination video size*/	
	m_u32src_width	= src_width;
	m_u32src_height	= src_height;
	m_i32dst_width	= dst_width;
	m_i32dst_height	= dst_height;
	/*get display dialog handle*/
	m_hWnd		= hWnd;

	//creat a direct draw object
	if (DirectDrawCreateEx(NULL, (void **)&m_lpDD, IID_IDirectDraw7, NULL)!= DD_OK)
	{
		//MessageBox(_T("creat	DIRECTDRAW object failed!"));
		return FALSE;
	}

	//set cooperation level
	if (m_lpDD->SetCooperativeLevel(hWnd, DDSCL_NORMAL) != DD_OK)
		return FALSE;

	//create a primary surface
	ZeroMemory(&m_cDDSDesc, sizeof(m_cDDSDesc));
	m_cDDSDesc.dwSize = sizeof(m_cDDSDesc);
	m_cDDSDesc.dwFlags = DDSD_CAPS;
	m_cDDSDesc.ddsCaps.dwCaps	= DDSCAPS_PRIMARYSURFACE;

	if (m_lpDD->CreateSurface(&m_cDDSDesc, &m_lpDDSPrimary, NULL)!=DD_OK)
	{
		return FALSE;
	}
		

	//creat a clipper. Very useful!
	//Clipper is so important the primary screen will always float on the topmost without clipper.
	LPDIRECTDRAWCLIPPER	lpDDClipper;
	m_lpDD->CreateClipper(0, &lpDDClipper, NULL);
	lpDDClipper->SetHWnd(0, hWnd);
	m_lpDDSPrimary->SetClipper(lpDDClipper);


	//create an off-screen surface
	ZeroMemory(&m_cDDSDesc, sizeof(m_cDDSDesc));
	m_cDDSDesc.dwSize = sizeof(m_cDDSDesc);
	//m_cDDSDesc.ddsCaps.dwCaps		= DDSCAPS_VIDEOMEMORY|DDSCAPS_OVERLAY;
	m_cDDSDesc.ddsCaps.dwCaps		= DDSCAPS_OFFSCREENPLAIN/*|DDSCAPS_FLIP|DDSCAPS_COMPLEX*/;
	m_cDDSDesc.dwFlags	= DDSD_CAPS|DDSD_HEIGHT|DDSD_WIDTH|DDSD_PIXELFORMAT;
	m_cDDSDesc.dwWidth	= src_width;
	m_cDDSDesc.dwHeight	= src_height;
	m_cDDSDesc.ddpfPixelFormat = ddpfOverlayFormats[m_chFormat];

	HRESULT hR;
	hR= m_lpDD->CreateSurface(&m_cDDSDesc, &m_lpDDOffScreen, NULL);
	if (hR!=DD_OK)
	{
		switch (hR)
		{
		case DDERR_INCOMPATIBLEPRIMARY:
			AfxMessageBox(_T("DDERR_INCOMPATIBLEPRIMARY"));
			break;
		case DDERR_INVALIDCAPS:
			AfxMessageBox(_T("DDERR_INVALIDCAPS"));
			break;
		case DDERR_INVALIDOBJECT:
			AfxMessageBox(_T("DDERR_INVALIDOBJECT"));
			break;
		case DDERR_INVALIDPARAMS:
			AfxMessageBox(_T("DDERR_INVALIDPARAMS"));
			break;
		case DDERR_INVALIDPIXELFORMAT:
			AfxMessageBox(_T("DDERR_INVALIDPIXELFORMAT"));
			break;
		case DDERR_NOALPHAHW:
			AfxMessageBox(_T("DDERR_NOALPHAHW"));
			break;
		case DDERR_NOCOOPERATIVELEVELSET:
			AfxMessageBox(_T("DDERR_NOCOOPERATIVELEVELSET"));
			break;
		case DDERR_NOFLIPHW:
			AfxMessageBox(_T("DDERR_NOFLIPHW:"));
			break;
		case DDERR_OUTOFMEMORY:
			AfxMessageBox(_T("DDERR_OUTOFMEMORY"));
			break;
		case DDERR_OUTOFVIDEOMEMORY:
			AfxMessageBox(_T("DDERR_OUTOFVIDEOMEMORY"));
			break;
		}
		return FALSE;
	}

	return TRUE;
}

VOID CDDrawDisplay::DDrawDispReinit(HWND hWnd, unsigned int src_width, unsigned int src_height, int dst_width, int dst_height)
{
	DDrawDispDone();
	DDrawDispInit(hWnd, src_width, src_height, dst_width, dst_height);
}

VOID CDDrawDisplay::DDrawDispDone()
{
	if (m_lpDDOffScreen != NULL)
	{
		m_lpDDOffScreen->Release();
		m_lpDDOffScreen = NULL;
	}

	if (m_lpDDSPrimary != NULL)
	{
		m_lpDDSPrimary->Release();
		m_lpDDSPrimary = NULL;
	}

	if (m_lpDD != NULL)
	{
		m_lpDD->Release();
		m_lpDD = NULL;
	}
}

VOID CDDrawDisplay::DrawUpdateDisp(unsigned char *src[3], int stride[3])
{
	HRESULT hRet;

	//lock down the surface so we can not modify its content
	do {
		hRet	= m_lpDDOffScreen->Lock(NULL, &m_cDDSDesc, DDLOCK_WAIT|DDLOCK_WRITEONLY, NULL);
	}while (hRet == DDERR_WASSTILLDRAWING);
	
	if (FAILED(hRet))
		return;

	//fill the off screen, Note for YV12, it's 8 bit Y plane followed by 8 bit 2x2 subsampled V and U planes.
	LPBYTE lpSurf = (LPBYTE)m_cDDSDesc.lpSurface;
	if (lpSurf)
	{
		int i;
		//fill Y data
		for (i=0; i<m_u32src_height; i++)
		{
			memcpy(lpSurf, src[0], m_u32src_width);
			src[0]	+= stride[0];
			lpSurf	+= m_cDDSDesc.lPitch;
		}
		//fill V data
		for (i=0; i<(m_u32src_height>>1); i++)
		{
			memcpy(lpSurf, src[2], m_u32src_width>>1);
			src[2]	+= stride[2];
			lpSurf	+= m_cDDSDesc.lPitch>>1;
		}
		//fill U data
		for (i=0; i<(m_u32src_height>>1); i++)
		{
			memcpy(lpSurf, src[1], m_u32src_width>>1);
			src[1]	+= stride[1];
			lpSurf	+= m_cDDSDesc.lPitch>>1;
		}
	}

	m_lpDDOffScreen->Unlock(NULL);

	/************************************************************************/
	/* Points to a RECT structure that defines the upper left and lower right points of the rectangle on the destination surface which is to be blted to.
	Points to the IDirectDrawSurface interface of the DirectDrawSurface object. This is the source for the blit operation.
    Points to a RECT structure that defines the upper left and lower right points of the rectangle on the source surface which is to be blted from.
	Pointer to a DDBLTFX structure */
	/************************************************************************/
	RECT	rD;

	RECT cRect;
	//get client coordinates of the display dialog
	GetClientRect(m_hWnd, &cRect);
	CPoint cPointTL(cRect.left, cRect.top), cPointBR(cRect.right, cRect.bottom);		//top left and bottom right
	//get screen coordinates of the display client
	ClientToScreen(m_hWnd, &cPointTL);
	ClientToScreen(m_hWnd, &cPointBR);

	/*set destination display picture size*/
	rD.left	= cPointTL.x;
	rD.top	= cPointTL.y;
	rD.right	= rD.left + m_i32dst_width;
	rD.bottom	= rD.top + m_i32dst_height;

	DDBLTFX   ddbltfx;
	memset(&ddbltfx, 0, sizeof(ddbltfx));
	ddbltfx.dwSize = sizeof(ddbltfx);
	ddbltfx.dwROP = SRCCOPY;



	hRet = m_lpDDSPrimary->Blt(&rD, m_lpDDOffScreen, NULL, DDBLT_WAIT, NULL);
	/*if (m_lpDDSPrimary->IsLost()==DDERR_SURFACELOST)
		m_lpDDSPrimary->Restore();*/
	//m_lpDDOffScreen->UpdateOverlayZOrder(DDOVERZ_MOVEBACKWARD, m_lpDDOffScreen);
	//hRet	= m_lpDDOffScreen->UpdateOverlay(NULL, m_lpDDSPrimary, &rD, DDOVER_SHOW|DDOVERZ_INSERTINBACKOF, NULL);
	
}

