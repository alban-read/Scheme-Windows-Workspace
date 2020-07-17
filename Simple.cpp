
#include "stdafx.h"
#include "Simple.h"
#pragma comment (lib,"Gdiplus.lib")

void CViewSimple::OnDraw(CDC& dc)
{
	UNREFERENCED_PARAMETER(dc);
}

LRESULT CViewSimple::on_mouse_activate(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	SetFocus();
	return FinalWindowProc(uMsg, wParam, lParam);
}

LRESULT CViewSimple::on_size(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(uMsg);
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	Invalidate();
	return 0L;
}

LRESULT CViewSimple::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_MOUSEACTIVATE:		return on_mouse_activate(uMsg, wParam, lParam);
	case WM_SIZE:				return on_size(uMsg, wParam, lParam);
	default: ;
	}

	return WndProcDefault(uMsg, wParam, lParam);
}

