// IME Test.cpp : Defines the entry point for the application.
 //

#include "stdafx.h"
#include <dbghelp.h>
#include "TInput.h"

/*----------------------------------------------------------------------------*/ 
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
int					GetCodePageFromLang( LANGID langid );

TInput		g_input;

/*----------------------------------------------------------------------------*/ 
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	const TCHAR szWindowName[] = _T("IME Test");

	WNDCLASSEX wcex;

	wcex.cbSize			= sizeof(WNDCLASSEX); 
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= szWindowName;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, IDI_APPLICATION);

	ATOM atom = RegisterClassEx(&wcex);

	g_input.InitInput();

	HWND hWnd = CreateWindow(szWindowName, szWindowName, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 400, 300, NULL, NULL, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

/*----------------------------------------------------------------------------*/ 
void ShowInputText(HDC hdc, HWND hWnd)
{
	HFONT hFont = CreateFont(16, 0, 0, 0, 400, FALSE, FALSE, FALSE, g_input.GetCharSet(), OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, _T("±¼¸²"));

	if(hFont) 
	{
		HFONT hFontOld = (HFONT)SelectObject(hdc, (HGDIOBJ)hFont);

		TCHAR text[256];
		int textLen;
		SIZE editTextSize;

		/* Draw Indicator */ 
		SetTextColor(hdc, 0x00000000);

		if(g_input.GetImeState() == 1) 
		{
			SetBkColor(hdc, 0x007F7FFF);
		}
		else
		{
			SetBkColor(hdc, 0x00CCCCCC);
		}

		const wchar_t* indicator = g_input.GetIndicator();
		TextOutW(hdc, 0, 0, indicator, wcslen(indicator));
		
		/* Draw Edit Text */ 
		SetBkColor(hdc, 0x00FFFFFF);

		textLen = g_input.GetInput(text, sizeof(text)/sizeof(text[0]));

		TextOut(hdc, 20, 0, text, textLen);

		GetTextExtentPoint32(hdc, text, textLen, &editTextSize);
		
		/* Draw Compostion Text */ 
		SetBkColor(hdc, 0x00CCCCCC);

		textLen = g_input.GetComp(text, sizeof(text)/sizeof(text[0]));

		TextOut(hdc, 20 + editTextSize.cx, 0, text, textLen);

		/* Draw Underline */ 
		int start, end;
		SIZE ulStart, ulEnd;

		g_input.GetUnderLine(&start, &end);
		GetTextExtentPoint32(hdc, text, start, &ulStart);
		GetTextExtentPoint32(hdc, text, end, &ulEnd);

		HPEN hPen = CreatePen(PS_DOT, 0, 0x000000FF);
		HPEN hOldPen = (HPEN)SelectObject(hdc, (HGDIOBJ)hPen);

		MoveToEx(hdc, 20 + editTextSize.cx+ulStart.cx, ulEnd.cy-1, NULL);
		LineTo(hdc, 20 + editTextSize.cx+ulEnd.cx, ulEnd.cy-1);

		SelectObject(hdc, (HGDIOBJ)hOldPen);

		int cx = 0;
		int cy = 20;
		/* Draw Reading */ 
		SetBkColor(hdc, 0x007FFF7F);

		textLen = g_input.GetReading(text, sizeof(text)/sizeof(text[0]));
		if(!g_input.IsVerticalReading()) 
		{
			TextOut(hdc, 0, cy, text, textLen);
		}
		else
		{
			TCHAR* begin = text;
			TCHAR* end = text + textLen;
			for(int i=0; i<4 && begin<end; ++i) 
			{
				//char* next = CharNextExA(g_input.GetCodePage(), begin, 0);
				TCHAR* next = begin+1;

				TextOut(hdc, 0, cy, begin, next-begin);

				SIZE readSize;

				GetTextExtentPoint32(hdc, begin, next-begin, &readSize);

				cy += readSize.cy;

				begin = next;
			}
		}

		/* Draw Candidate */ 
		SetTextColor(hdc, 0x00000000);

		int count = min(g_input.GetCandidateCount(), g_input.GetCandidatePageStart()+g_input.GetCandidatePageSize());
		for(int i=g_input.GetCandidatePageStart(); i<count; ++i) 
		{
			if(i == g_input.GetCandidateSelection()) 
			{
				SetBkColor(hdc, 0x00FFCFCF);
			}
			else
			{
				SetBkColor(hdc, 0x00FF7F7F);
			}

			int textLen = g_input.GetCandidate(i, text, sizeof(text)/sizeof(text[0]));

			TextOut(hdc, cx, cy, text, textLen);

			SIZE candSize;

			GetTextExtentPoint32(hdc, text, textLen, &candSize);

			if(!g_input.IsVerticalCandidate()) 
			{
				cx += candSize.cx;
			}
			else
			{
				cy += candSize.cy;
			}
		}

		SelectObject(hdc, (HGDIOBJ)hFontOld);
	}
}

/*----------------------------------------------------------------------------*/ 
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) 
	{
	case WM_CREATE:
		g_input.OnCreate(hWnd, wParam, lParam);
		return 0L;

	/* IME Messages */ 
	case WM_INPUTLANGCHANGE:
		g_input.OnInputLanguageChange(hWnd, wParam, lParam);
		InvalidateRect(hWnd, NULL, TRUE);
		break;

	case WM_IME_SETCONTEXT:
		lParam = 0;
		break;

	case WM_IME_STARTCOMPOSITION:
		return 0L;

	case WM_IME_COMPOSITION:
		if(g_input.OnComposition(hWnd, wParam, lParam)) 
		{
			InvalidateRect(hWnd, NULL, TRUE);
			return 0L;
		}
		break;

	case WM_IME_ENDCOMPOSITION:
		if(g_input.OnEndComposition(hWnd, wParam, lParam)) 
		{
			InvalidateRect(hWnd, NULL, TRUE);
			return 0L;
		}
		break;

	case WM_IME_NOTIFY:
		if(g_input.OnNotify(hWnd, wParam, lParam)) 
		{
			InvalidateRect(hWnd, NULL, TRUE);
			return 0L;
		}
		InvalidateRect(hWnd, NULL, TRUE);
		break;

	case WM_CHAR:
		g_input.OnChar(hWnd, wParam, lParam);
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	/* End of IME Messages */ 

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		ShowInputText(hdc, hWnd);
		EndPaint(hWnd, &ps);
		return 0L;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0L;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*----------------------------------------------------------------------------*/ 
