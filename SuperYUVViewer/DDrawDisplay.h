#pragma once

#include <ddraw.h>

class CDDrawDisplay
{
public:
	CDDrawDisplay(void);
	CDDrawDisplay(char yuvforamt);
	~CDDrawDisplay(void);
public:
	BOOL DDrawDispInit(HWND hWnd, unsigned int src_width, unsigned int src_height, int dst_width, int dst_height);
	VOID	DDrawDispReinit(HWND hWnd, unsigned int src_width, unsigned int src_height, int dst_width, int dst_height);
	VOID	DDrawDispDone();
	VOID	DrawUpdateDisp(unsigned char *src[3], int stride[3]);

private:
	unsigned int	m_u32src_width;
	unsigned int	m_u32src_height;
	int		m_i32dst_width;
	int		m_i32dst_height;
	char	m_chFormat;

	HWND	m_hWnd;
	LPDIRECTDRAW7			m_lpDD;
	LPDIRECTDRAWSURFACE7	m_lpDDSPrimary;			//primary screen
	LPDIRECTDRAWSURFACE7	m_lpDDOffScreen;		//off-screen
	DDSURFACEDESC2			m_cDDSDesc;			//surface description
};
