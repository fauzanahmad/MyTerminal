#include "MyHeader.h"
#include "strsafe.h"

int g_fauzan = 0;
int g_ahmed = 0;
int g_iCmdNo = -1;
int g_iCharPos = 0;
int g_iCaretPos = 0;
int g_iLineNumber = 1;
int g_iCount = 1;
int g_iReDirect = 0;
TCHAR g_szRedirectFileName[MAX_PATH] = { 0 };
HFONT g_hFont;

TCHAR g_MainBuffer[5012] = { 0 };
TCHAR g_CommandBuffer[5012] = { 0 };
TCHAR* g_pcCurrentParam = NULL;

TCHAR* g_szCommand[] = { _T("cat"), _T("grep"), _T("ls"), _T("clear"), _T("exit"), _T("cd"), _T("cp"), _T("mv"), _T("rm"), _T("exec"), _T("mkdir"), _T("mkfl"), _T("find") , _T("grep"), _T("rmdir"),0};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow)
{
	TCHAR szAppName[] = TEXT("My Terminal");
	HWND hwnd;
	MSG msg;
	WNDCLASSEX wd = { 0 };

	wd.style = CS_HREDRAW | CS_VREDRAW;
	wd.lpfnWndProc = WndProc;
	wd.cbClsExtra = 0;
	wd.cbWndExtra = 0;
	wd.hInstance = hInstance;
	wd.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wd.hCursor = LoadCursor(NULL, IDC_ARROW);
	wd.lpszClassName = szAppName;
	wd.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);//(/*COLOR_BACKGROUND+1*/ BLACK_BRUSH);
	wd.lpszMenuName = NULL;
	wd.cbSize = sizeof(WNDCLASSEX);
	wd.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wd))
	{
		DWORD dw = GetLastError();
		MessageBox(NULL, TEXT("Register Class failed"), TEXT("Error"), MB_OK);
		return -1;
	}

	hwnd = CreateWindow(
		szAppName,
		TEXT("MyTerminal"),
		WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL
	);
	if (NULL == hwnd)
	{
		MessageBox(NULL, TEXT("CreateWindowEx failed"), TEXT("Error"), MB_OK);
		return -1;
	}

	ShowWindow(hwnd, SW_MAXIMIZE);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, hwnd, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	
	RECT rc;
	static BOOL bFlag = FALSE;
	DWORD dw;
	TCHAR szTitle[200] = { 0 };
	static int iCaretX = 2;
	static int iLen = 0;
	size_t Length;

	/*
	static HWND hwndHScroll;
	static HWND hwndVScroll;
	*/

	static int iSelectStart = -1;
	static int iSelectEnd = -1;
	static TCHAR szSelectedText[MAX_PATH] = {0};
	COLORREF prevCol;
	COLORREF prevTextCol;

	static int cxClient;
	static int cyClient;
	static int cxClientMax;
	static int cyClientMax;

	static int cxChar;
	static int cyChar;
	static int cxCharUpper;

	int iFirstLine;
	int iLastLine;
	static int xPos;
	static int yPos;
	TEXTMETRIC tm;
	SCROLLINFO si;
	int i,x,y;
	HRESULT hr;

	FILE* fptr = NULL;
	TCHAR szBuffer[512] = { 0 };
	WORD wScrollNotify = 0xFFFF;
	HANDLE hFile;
	int iCommandCount = 0;
	TCHAR szCommand[3000] = {0};
	TCHAR szCommand2[3000] = { 0 };
	TCHAR szSection[10] = { 0 };
	TCHAR *pcStart = NULL;
	TCHAR *pcEnd = NULL;

	switch (iMsg)
	{
	case WM_CREATE:
	{
		/* for scroll bar */
		g_hFont = CreateFont(20, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
			CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, _T("Hobo"));
		/**/

		hdc = GetDC(hwnd);
		SelectObject(hdc, g_hFont);
		GetTextMetrics(hdc, &tm);
		ReleaseDC(hwnd, hdc);

		cxChar = tm.tmAveCharWidth;
		cyChar = tm.tmHeight + tm.tmExternalLeading;
		cxCharUpper = (tm.tmPitchAndFamily & 1 ? 3 : 2) * cxChar/2;

		cxClientMax = 48 * cxChar + 12 * cxCharUpper;

		GetClientRect(hwnd, &rc);
		/**/

		memset(g_MainBuffer, 0x00, 5012);
		memset(g_CommandBuffer, 0x00, 5012);
		/**/
#ifndef _DEBUG
		GetCurrentDirectory(sizeof(szTitle), szTitle);
		SetWindowText(hwnd, szTitle);
		/**/
		/**/
#else
		SetCurrentDirectory(_T("C:\\Users\\fauza\\Desktop\\fauzanworks"));
		SetWindowText(hwnd, _T("C:\\Users\\fauza\\Desktop\\fauzanworks"));
#endif
		/**/
		DumpOutput(WRITEMODE, NULL);

		/**/
		ShowScrollBar(hwnd, SB_HORZ, TRUE);
		ShowScrollBar(hwnd, SB_VERT, TRUE);
		/**/

		hFile = CreateFile(COMMANDFILENAME, GENERIC_READ | GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile)
			CloseHandle(hFile);

	}break;
	/* for scrollbar */
	case WM_SIZE:
	{
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);

		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = LINES;
		si.nPage = cyClient / cyChar;
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = 2 + cxClientMax / cxChar;
		si.nPage = cxClient / cxChar;
		SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
	}break;
	
	case WM_HSCROLL:
	{
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;

		GetScrollInfo(hwnd, SB_HORZ, &si);
		xPos = si.nPos;

		switch (LOWORD(wParam))
		{
		case SB_LINELEFT:
			si.nPos = si.nPos - 1;
			break;

		case SB_LINERIGHT:
			si.nPos = si.nPos + 1;
			break;
		
		case SB_PAGELEFT:
			si.nPos = si.nPos - si.nPage;
			break;

		case SB_PAGERIGHT:
			si.nPos = si.nPos + si.nPage;
			break;

		case SB_THUMBTRACK:
			si.nPos = si.nTrackPos;
			break;

		default:
			break;
		}
		
		si.fMask = SIF_POS;
		SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
		GetScrollInfo(hwnd, SB_HORZ, &si);

		if (si.nPos != xPos)
		{
			ScrollWindow(hwnd, cxChar*(xPos-si.nPos), 0, NULL, NULL);
			InvalidateRect(hwnd, NULL, TRUE);
		}

	}break;

	case WM_VSCROLL:
	{
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(hwnd, SB_VERT, &si);

		yPos = si.nPos+1;

		switch (LOWORD(wParam))
		{
		case SB_TOP:
			si.nPos = si.nMin;
			break;
		case SB_BOTTOM:
			si.nPos = si.nMax;
			break; 
		case SB_LINEUP:
			si.nPos = si.nPos-1;
			break;
		case SB_LINEDOWN:
			si.nPos = si.nPos+1;
			break;
		case SB_PAGEUP:
			si.nPos = si.nPos - si.nPage;
			break;
		case SB_PAGEDOWN:
			si.nPos = si.nPos + si.nPage;
			break;
		case SB_THUMBTRACK:
			si.nPos = si.nTrackPos;
			break;
		default:
			break;
		}

		si.fMask = SIF_POS;
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
		GetScrollInfo(hwnd, SB_VERT, &si);

		if (si.nPos != yPos)
		{
			ScrollWindow(hwnd, 0, cyChar*(yPos-si.nPos), NULL, NULL);
			InvalidateRect(hwnd, NULL, TRUE);
		}
	}break;

	case WM_SETFOCUS:
	{
		hdc = GetDC(hwnd);
		SelectObject(hdc, g_hFont);
		GetCharWidth32(hdc, '$', '$', &iLen);
		ReleaseDC(hwnd, hdc);
		CreateCaret(hwnd, (HBITMAP)NULL, 0, 18);
		if(iCaretX == 2)
			iCaretX += iLen;
		SetCaretPos(iCaretX, 0);
		ShowCaret(hwnd);
	}
	break;

	case WM_KILLFOCUS:
	{
		HideCaret(hwnd);
		DestroyCaret();
	}break;

	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_UP:
			if (GetKeyState(VK_SHIFT) < 0)
			{
				wScrollNotify = SB_LINEUP;
				if (wScrollNotify != -1)
					SendMessage(hwnd, WM_VSCROLL, MAKELONG(wScrollNotify, 0), 0L);
			}
			else
			{
				InvalidateRect(hwnd, NULL, TRUE);
				iCommandCount = GetPrivateProfileInt(_T("Command"), _T("Count"), 0, COMMANDFILENAME);
				g_iCount--;
				if (g_iCount == 0)
					g_iCount = iCommandCount;
				
				_stprintf(szSection, _T("%d"), g_iCount);
				GetPrivateProfileString(_T("Command"), szSection, _T(""), szCommand, sizeof(szCommand), COMMANDFILENAME);
				if (_tcslen(szCommand) > 0)
				{
					g_iCharPos = _tcslen(szCommand);
					g_iCaretPos = g_iCharPos;

					_tcscpy(g_CommandBuffer, szCommand);
					ReplaceSpaceWithDoller();

					hdc = GetDC(hwnd);
					SelectObject(hdc, g_hFont);
					SetBkColor(hdc, RGB(200, 200, 200));
					GetCharWidth32(hdc, _T('$'), _T('$'), &iLen);
					iLen += GetWidthOfString(hdc, g_MainBuffer);
					TextOut(hdc, 0, 0, _T("$"), 1);
					TextOut(hdc, 10, 0, g_MainBuffer, g_iCharPos);
					ReleaseDC(hwnd, hdc);
					iCaretX = iLen;
					SetCaretPos(iCaretX, 0);
				}
			}
			break;
		case VK_DOWN:
			if (GetKeyState(VK_SHIFT) < 0)
			{
				wScrollNotify = SB_LINEDOWN;
				if (wScrollNotify != -1)
					SendMessage(hwnd, WM_VSCROLL, MAKELONG(wScrollNotify, 0), 0L);
			}
			else
			{
				InvalidateRect(hwnd, NULL, TRUE);
				iCommandCount = GetPrivateProfileInt(_T("Command"), _T("Count"), 0, COMMANDFILENAME);
				g_iCount++;
				if (g_iCount > iCommandCount)
					g_iCount = 1;
				_stprintf(szSection, _T("%d"), g_iCount);
				GetPrivateProfileString(_T("Command"), szSection, _T(""), szCommand, sizeof(szCommand), COMMANDFILENAME);
				if (_tcslen(szCommand) > 0)
				{
					g_iCharPos = _tcslen(szCommand);
					g_iCaretPos = g_iCharPos;

					_tcscpy(g_CommandBuffer, szCommand);
					ReplaceSpaceWithDoller();

					hdc = GetDC(hwnd);
					SelectObject(hdc, g_hFont);
					SetBkColor(hdc, RGB(200, 200, 200));
					GetCharWidth32(hdc, _T('$'), _T('$'), &iLen);
					iLen += GetWidthOfString(hdc, g_MainBuffer);
					TextOut(hdc, 0, 0, _T("$"), 1);
					TextOut(hdc, 10, 0, g_MainBuffer, g_iCharPos);
					ReleaseDC(hwnd, hdc);
					iCaretX = iLen;
					SetCaretPos(iCaretX, 0);
				}
			}
			break;
		case VK_TAB:
			{
				HideCaret(hwnd);
				HandleTab(hwnd, &iCaretX);
				ShowCaret(hwnd);
			}break;
		case VK_LEFT:
			{
				if (GetKeyState(VK_SHIFT) & 0x8000)
				{
					if (iSelectEnd == -1 && iSelectStart == -1)
					{
						iSelectEnd = g_iCaretPos;
						iSelectStart = g_iCaretPos-1;
					}
					else
						iSelectStart = g_iCaretPos - 1;
					_tcsncpy(szSelectedText, g_MainBuffer+iSelectStart, iSelectEnd-iSelectStart);
				}
				else
				{
					iSelectEnd = -1;
					iSelectStart = -1;
				}
				HideCaret(hwnd);
				MoveCaretLeftRight(hwnd, &iCaretX, 0);
				ShowCaret(hwnd);

				if ( -1 != iSelectStart &&  -1 != iSelectEnd)
				{
					hdc = GetDC(hwnd);
					prevTextCol = SetTextColor(hdc, RGB(255, 255, 255));
					prevCol = SetTextColor(hdc, RGB(0, 0, 0));
					SelectObject(hdc, g_hFont);
					TextOut(hdc, iCaretX, 0, szSelectedText, _tcslen(szSelectedText));
					ReleaseDC(hwnd, hdc);
				}
				else
				{
					InvalidateRect(hwnd, NULL, TRUE);
				}

			}break;
		case VK_RIGHT:
			{
				if (GetKeyState(VK_SHIFT) & 0x8000)
				{
					if (iSelectEnd == -1 && iSelectStart == -1)
					{
						iSelectEnd = g_iCaretPos+1;
						iSelectStart = g_iCaretPos;
					}
					else
						iSelectEnd = g_iCaretPos+1;
					_tcsncpy(szSelectedText, g_MainBuffer + iSelectStart, iSelectEnd - iSelectStart);
				}
				else
				{
					iSelectEnd = -1;
					iSelectStart = -1;
				}
				HideCaret(hwnd);
				MoveCaretLeftRight(hwnd, &iCaretX, 1);
				ShowCaret(hwnd);

				if (-1 != iSelectStart && -1 != iSelectEnd)
				{
					hdc = GetDC(hwnd);
					prevTextCol = SetTextColor(hdc, RGB(255, 255, 255));
					prevCol = SetTextColor(hdc, RGB(0, 0, 0));
					SelectObject(hdc, g_hFont);
					TextOut(hdc, iCaretX - GetWidthOfString(hdc, szSelectedText), 0, szSelectedText, _tcslen(szSelectedText));
					ReleaseDC(hwnd, hdc);
				}
				else
				{
					InvalidateRect(hwnd, NULL, TRUE);
				}
			}break;
		case  VK_HOME:
			{
				if (GetKeyState(VK_SHIFT) & 0x8000)
				{
					iSelectEnd = g_iCaretPos;
					iSelectStart = 0;
					
					_tcsncpy(szSelectedText, g_MainBuffer + iSelectStart, iSelectEnd - iSelectStart);
				}
				else
				{
					iSelectEnd = -1;
					iSelectStart = -1;
				}

				HideCaret(hwnd);
				MoveCaretLeftRight(hwnd, &iCaretX, 2);
				ShowCaret(hwnd);

				if (-1 != iSelectStart && -1 != iSelectEnd)
				{
					hdc = GetDC(hwnd);
					prevTextCol = SetTextColor(hdc, RGB(255, 255, 255));
					prevCol = SetTextColor(hdc, RGB(0, 0, 0));
					SelectObject(hdc, g_hFont);
					TextOut(hdc, iCaretX, 0, szSelectedText, _tcslen(szSelectedText));
					ReleaseDC(hwnd, hdc);
				}
				else
				{
					InvalidateRect(hwnd, NULL, TRUE);
				}

			}break;
		case  VK_END:
			{
				if (GetKeyState(VK_SHIFT) & 0x8000)
				{
					iSelectStart = g_iCaretPos;
					iSelectEnd = _tcslen(g_MainBuffer+iSelectStart);

					_tcsncpy(szSelectedText, g_MainBuffer + iSelectStart, iSelectEnd);
				}
				else
				{
					iSelectEnd = -1;
					iSelectStart = -1;
				}

				HideCaret(hwnd);
				MoveCaretLeftRight(hwnd, &iCaretX, 3);
				ShowCaret(hwnd);

				if (-1 != iSelectStart && -1 != iSelectEnd)
				{
					hdc = GetDC(hwnd);
					prevTextCol = SetTextColor(hdc, RGB(255, 255, 255));
					prevCol = SetTextColor(hdc, RGB(0, 0, 0));
					SelectObject(hdc, g_hFont);
					TextOut(hdc, iCaretX - GetWidthOfString(hdc, szSelectedText), 0, szSelectedText, _tcslen(szSelectedText));
					ReleaseDC(hwnd, hdc);
				}
				else
				{
					InvalidateRect(hwnd, NULL, TRUE);
				}

			}break;
		case VK_DELETE:
		{
			if (GetKeyState(VK_CONTROL) & 0x8000)
			{
				memset(g_CommandBuffer, 0x00, sizeof(g_CommandBuffer));
				memset(g_MainBuffer, 0x00, sizeof(g_MainBuffer));
				g_iCaretPos = 0;
				g_iCharPos = 0;

				hdc = GetDC(hwnd);
				SelectObject(hdc, g_hFont);
				GetCharWidth32(hdc, _T('$'), _T('$'), &iLen);
				iCaretX = iLen + 2;
				ReleaseDC(hwnd, hdc);

				SetCaretPos(iCaretX, 0);

				InvalidateRect(hwnd, NULL, TRUE);
			}
			else
			{
				if (iSelectStart > -1 && iSelectEnd > -1)
				{
					pcStart = &g_CommandBuffer[iSelectStart];
					pcEnd = &g_CommandBuffer[iSelectEnd];
					memset(szSelectedText, 0x00, sizeof(szSelectedText));
					_tcscpy(szSelectedText, pcEnd);
					pcStart[0] = _T('\0');
					_tcscpy(pcStart, szSelectedText);
					ReplaceSpaceWithDoller();
					InvalidateRect(hwnd, NULL, TRUE);
					iSelectEnd = -1;
					iSelectStart = -1;
				}
				else
				{
					wParam = 127;
					HandleKeyPress(hwnd, wParam, &iCaretX);
					wParam = VK_DELETE;
				}
			}
			
		}break;
		}
		/*
		if (wParam == VK_TAB)
		{
			HideCaret(hwnd);
			HandleTab(hwnd, &iCaretX);
			ShowCaret(hwnd);
		}
		else if(wParam == VK_LEFT)
		{
			HideCaret(hwnd);
			MoveCaretLeftRight(hwnd, &iCaretX, 0);
			ShowCaret(hwnd);
		}
		else if (wParam == VK_RIGHT)
		{
			HideCaret(hwnd);
			MoveCaretLeftRight(hwnd, &iCaretX, 1);
			ShowCaret(hwnd);
		}
		else if (wParam == VK_HOME)
		{
			HideCaret(hwnd);
			MoveCaretLeftRight(hwnd, &iCaretX, 2);
			ShowCaret(hwnd);
		}
		else if (wParam == VK_END)
		{
			HideCaret(hwnd);
			MoveCaretLeftRight(hwnd, &iCaretX, 3);
			ShowCaret(hwnd);
		}
		else if (wParam == VK_DELETE)
		{
			wParam = 127;
			HandleKeyPress(hwnd, wParam, &iCaretX);
			wParam = VK_DELETE;
		}
		*/
	}break;

	case WM_CHAR:
	{
		HideCaret(hwnd);
		HandleKeyPress(hwnd, wParam, &iCaretX);
		ShowCaret(hwnd);
	}break;

	case WM_PAINT:
	{
		hdc = BeginPaint(hwnd, &ps);
		SetBkColor(hdc, RGB(200, 200, 200));

		// All painting occurs here, between BeginPaint and EndPaint.
		FillRect(hdc, &ps.rcPaint, (HBRUSH)(LTGRAY_BRUSH));

        SelectObject(hdc, g_hFont);

		/*Scrollbar code */
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;
		GetScrollInfo(hwnd, SB_VERT, &si);
		yPos = si.nPos;
		GetScrollInfo(hwnd, SB_HORZ, &si);
		xPos = si.nPos;

		iFirstLine = max(1, yPos + (ps.rcPaint.top-10)/cyChar);
		iLastLine = min(LINES, yPos + ps.rcPaint.bottom / cyChar);

		if (iFirstLine > 0)
			g_fauzan = iFirstLine;
		if (iLastLine < LINES)
			g_ahmed = iLastLine;
		fptr = _tfopen(OUTPUTFILENAME, _T("r"));

		if (g_fauzan == 1)
		{
			TextOut(hdc, 0, 0, _T("$"), 1);
			TextOut(hdc, 10, 0, g_MainBuffer, g_iCharPos);
		}

		if (fptr != NULL)
		{
			for (i = 1; !feof(fptr) && i < g_fauzan; i++)
			{
				memset(szBuffer, 0x00, sizeof(szBuffer));
				_fgetts(szBuffer, sizeof(szBuffer), fptr);

				if (i < g_fauzan)
					continue;
			}

			for (i = iFirstLine; !feof(fptr) && i <= iLastLine; i++)
			{
				memset(szBuffer, 0x00, sizeof(szBuffer));
				_fgetts(szBuffer, 512, fptr);

				
				x = cxChar * (1 - xPos);
				y = cyChar * (i - yPos);

				
				//if (i == g_ahmed)
				//	break;

				iLen = _tcslen(szBuffer);
				hr = StringCchLength(szBuffer, 260, &Length);
				
				//_stprintf(szTitle, _T("x = %d y = %d Length = %d FirstLine %d, LastLine = %d g_fauzan = %d g_ahmed = %d"), x, y, Length, iFirstLine, iLastLine, g_fauzan, g_ahmed );
				//TextOut(hdc, 500, 80, szTitle, 200);
				

				TextOut(hdc, x, y, szBuffer, (int)Length);
			}
			fclose(fptr);
		}
		EndPaint(hwnd, &ps);
		ReleaseDC(hwnd, hdc);
	}break;

	case WM_DESTROY:
	{
		DestroyCaret();
		ExitProcess(0);
		DeleteObject(g_hFont);
		//PostQuitMessage(0);
	}break;
	}
	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

void HandleKeyPress(HWND hwnd, WPARAM wParam, int* piCaretX)
{
	HDC hdc;
	HFONT hFont;
	int iLen = 0;
	TCHAR* pcStart = NULL;
	TCHAR* pcEnd = NULL;
	TCHAR szTemp[260] = { 0 };
	// BackSpace charecters
	if (wParam == 8 && g_iCharPos != 0)
	{
		g_iCharPos--;
		g_iCaretPos--;
		hdc = GetDC(hwnd);
		SelectObject(hdc, g_hFont);
		SetBkColor(hdc, RGB(200, 200, 200));
		GetCharWidth32(hdc, g_MainBuffer[g_iCaretPos], g_MainBuffer[g_iCaretPos], &iLen);
		ReleaseDC(hwnd, hdc);
		*piCaretX -= iLen;
		SetCaretPos(*piCaretX, 0);
		if (g_MainBuffer[g_iCaretPos+1] != _T('\0'))
		{
			g_MainBuffer[g_iCaretPos] = _T('\0');
			g_CommandBuffer[g_iCaretPos] = _T('\0');
			pcStart = &g_MainBuffer[g_iCaretPos];
			pcEnd = &g_MainBuffer[g_iCaretPos+1];
			iLen = _tcslen(pcEnd) + _tcslen(g_MainBuffer);
			_tcscat(pcStart, pcEnd);//problem

			pcStart = &g_CommandBuffer[g_iCaretPos];
			pcEnd = &g_CommandBuffer[g_iCaretPos + 1];
			iLen = _tcslen(pcEnd) + _tcslen(g_CommandBuffer);
			_tcscat(pcStart, pcEnd);//problem

			/*
			pcStart = &g_MainBuffer[g_iCaretPos];
			_tcscpy(szTemp, pcStart);
			g_MainBuffer[g_iCaretPos] = wParam;
			pcStart++;
			_tcscpy(pcStart, szTemp);

			memset(szTemp, 0x00, sizeof(szTemp));

			pcStart = &g_CommandBuffer[g_iCaretPos];
			iLen = _tcslen(pcStart) + 1;
			_tcscpy(szTemp, pcStart);
			g_CommandBuffer[g_iCaretPos] = _T('$');
			pcStart++;
			_tcscpy(pcStart, szTemp);
			*/
		}
		else
		{
			g_MainBuffer[g_iCaretPos] = _T('\0');
			g_CommandBuffer[g_iCaretPos] = _T('\0');
		}

		InvalidateRect(hwnd, NULL, TRUE);
	}
	// normal charecters
	//else if (wParam >= 32 && wParam <= 126)

	else if (AllowCharecters(wParam))
	{
		if (g_MainBuffer[g_iCaretPos] != _T('\0'))
		{
			pcStart = &g_MainBuffer[g_iCaretPos];
			_tcscpy(szTemp, pcStart);
			g_MainBuffer[g_iCaretPos] = wParam;
			pcStart++;
			_tcscpy(pcStart , szTemp);

			memset(szTemp, 0x00, sizeof(szTemp));

			pcStart = &g_CommandBuffer[g_iCaretPos];
			iLen = _tcslen(pcStart)+1;
			_tcscpy(szTemp , pcStart);
			g_CommandBuffer[g_iCaretPos] = wParam;
			pcStart++;
			_tcscpy(pcStart, szTemp);
		}
		else
		{
			g_MainBuffer[g_iCaretPos] = wParam;
			g_CommandBuffer[g_iCaretPos] = wParam;
		}
		g_iCharPos++;
		g_iCaretPos++;
		hdc = GetDC(hwnd);
		SelectObject(hdc, g_hFont);
		/**
		hFont = CreateFont(20, 10, 0, 0, FW_EXTRABOLD, TRUE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
			CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, _T("Hobo"));
		SelectObject(hdc, hFont);
		/**/
		SetBkColor(hdc, RGB(200, 200, 200));
		TextOut(hdc, 10, 0, g_MainBuffer, g_iCharPos);
		GetCharWidth32(hdc, wParam, wParam, &iLen);
		//DeleteObject(hFont);
		ReleaseDC(hwnd, hdc);

		*piCaretX += iLen;
		SetCaretPos(*piCaretX, 0);
	}
	else if (wParam == 32)
	{
		if (g_MainBuffer[g_iCaretPos] != _T('\0'))
		{
			pcStart = &g_MainBuffer[g_iCaretPos];
			_tcscpy(szTemp, pcStart);
			g_MainBuffer[g_iCaretPos] = wParam;
			pcStart++;
			_tcscpy(pcStart, szTemp);

			memset(szTemp, 0x00, sizeof(szTemp));

			pcStart = &g_CommandBuffer[g_iCaretPos];
			iLen = _tcslen(pcStart) + 1;
			_tcscpy(szTemp, pcStart);
			g_CommandBuffer[g_iCaretPos] = _T('$');
			pcStart++;
			_tcscpy(pcStart, szTemp);
		}
		else
		{
			g_MainBuffer[g_iCaretPos] = wParam;
			g_CommandBuffer[g_iCaretPos] = _T('$');
		}
		g_iCharPos++;
		g_iCaretPos++;
		hdc = GetDC(hwnd);
		SelectObject(hdc, g_hFont);
		SetBkColor(hdc, RGB(200, 200, 200));
		/**
		hFont = CreateFont(20, 10, 0, 0, FW_EXTRABOLD, TRUE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
			CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, _T("Hobo"));
		SelectObject(hdc, hFont);
		/**/
		TextOut(hdc, 10, 0, g_MainBuffer, g_iCharPos);
		GetCharWidth32(hdc, wParam, wParam, &iLen);
		//DeleteObject(hFont);
		ReleaseDC(hwnd, hdc);

		*piCaretX += iLen;
		SetCaretPos(*piCaretX, 0);
	}
	//Enter Button Press
	else if (wParam == 13)
	{
		HandleEnterKeyPress(hwnd, wParam);

		memset(g_CommandBuffer, 0x00, sizeof(g_CommandBuffer));
		memset(g_MainBuffer, 0x00, sizeof(g_MainBuffer));

		hdc = GetDC(hwnd);
		SelectObject(hdc, g_hFont);
		GetCharWidth32(hdc, _T('$'), _T('$'), &iLen);
		*piCaretX = iLen+2;
		ReleaseDC(hwnd, hdc);
		SetCaretPos(*piCaretX, 0);
		g_iCaretPos = 0;
		g_iCharPos = 0;

		InvalidateRect(hwnd, NULL, TRUE);
	}
	else if (wParam == 127)
	{
		if (g_iCaretPos == g_iCharPos)
			return;
		g_MainBuffer[g_iCaretPos] = _T('\0');
		g_CommandBuffer[g_iCaretPos] = _T('\0');
		pcStart = &g_MainBuffer[g_iCaretPos];
		pcEnd = &g_MainBuffer[g_iCaretPos + 1];
		iLen = _tcslen(pcEnd) + _tcslen(g_MainBuffer);
		_tcscat(pcStart, pcEnd);

		pcStart = &g_CommandBuffer[g_iCaretPos];
		pcEnd = &g_CommandBuffer[g_iCaretPos + 1];
		iLen = _tcslen(pcEnd) + _tcslen(g_CommandBuffer);
		_tcscat(pcStart, pcEnd);

		g_iCharPos--;
		InvalidateRect(hwnd, NULL, TRUE);
	}
}

void HandleEnterKeyPress(HWND hwnd, WPARAM wParam)
{
	TCHAR* pcStart = NULL;
	int iLen = 0;
	TCHAR szCommand[50] = { 0 };
	
	TCHAR szParam1[260] = {0};
	TCHAR szParam2[260] = { 0 };
	TCHAR szParam3[260] = { 0 };
	TCHAR szParam4[260] = { 0 };
	TCHAR szParam5[260] = { 0 };
	TCHAR szParam6[260] = { 0 };
	TCHAR szParam7[260] = { 0 };
	TCHAR szParam8[260] = { 0 };
	TCHAR szParam9[260] = { 0 };

	int iCommand = 0;
	TCHAR szCommandNo[10] = { 0 };
	TCHAR szCommandName[260] = { 0 };

	int iCharecters = 0;
	int iCommandCode = 0;

	DumpOutput(WRITEMODE, NULL);

	iCommand = GetPrivateProfileInt(_T("Command"), _T("Count"), 0, COMMANDFILENAME);
	if (0 != iCommand)
	{
		_stprintf(szCommandNo, _T("%d"), iCommand);
		GetPrivateProfileString(_T("Command"), szCommandNo, _T(""), szCommandName, sizeof(szCommandName), COMMANDFILENAME);
	}

	if (_tcscmp(szCommandName, g_CommandBuffer) != 0)
	{
		iCommand++;
		memset(szCommandNo, 0x00, sizeof(szCommandNo));
		_stprintf(szCommandNo, _T("%d"), iCommand);
		WritePrivateProfileString(_T("Command"), _T("Count"), szCommandNo, COMMANDFILENAME);
		WritePrivateProfileString(_T("Command"), szCommandNo, g_CommandBuffer, COMMANDFILENAME);
	}

	if (_tcschr(g_CommandBuffer, _T('>')))
	{
		DumpOutput(WRITEMODE, NULL);

		g_iReDirect = 1;
		pcStart = _tcschr(g_CommandBuffer, _T('>'));
		if (NULL != pcStart)
		{
			iLen = 1 + _tcslen(pcStart);
			pcStart += 2;
			if (*pcStart != _T('\0'))
			{
				_tcscpy(g_szRedirectFileName, pcStart);
				DumpOutput(WRITEMODE, g_szRedirectFileName);

				pcStart -= 3;
				memset(pcStart, 0x00, iLen);
			}
		}
	}
	else
	{
		g_iReDirect = 0;
		memset(g_szRedirectFileName, 0x00, sizeof(g_szRedirectFileName));
	}

	/*Get command name*/
	GetCommandNameAndParameters ( szCommand, 50, szParam1, szParam2, szParam3, szParam4, szParam5, szParam6, szParam7, szParam8, szParam9, 260);

	/*Get command Parameters*/

	if (_tcslen(szCommand) == 0)
		return;

	/* GetCommand Code */
	iCommandCode = GetCommandCode(szCommand);

	switch (iCommandCode)
	{
		case MY_CMD_CD:
		{
			fun_cd(hwnd, szParam1);
		}break;

		case MY_CMD_CAT:
		{
			fun_cat(hwnd, szParam1);
		}break;

		case MY_CMD_LS:
		{
			fun_ls(hwnd, szParam1, szParam2);
		}break;

		case MY_CMD_CLEAR:
		{
			fun_clear(hwnd);
		}break;

		case MY_CMD_EXIT:
		{
			fun_exit(hwnd);
		}break;

		case MY_CMD_RM:
		{
			fun_rm(hwnd, szParam1);
		}break;

		case MY_CMD_CP:
		{
			fun_cp(hwnd, szParam1, szParam2);
		}break;

		case MY_CMD_MV:
		{
			fun_mv(hwnd, szParam1, szParam2);
		}break;

		case MY_CMD_MKDIR:
		{
			fun_mkdir(hwnd, szParam1);
		}break;

		case MY_CMD_MKFILE:
		{
			fun_mkfile(hwnd, szParam1);
		}break;

		case MY_CMD_FIND:
		{
			fun_find(hwnd, szParam1, szParam2);
		}break;

		case MY_CMD_EXEC:
		{
			fun_exe(hwnd, szParam1, szParam2, szParam3, szParam4, szParam5, szParam6, szParam7, szParam8, szParam9);
		}break;

		case MY_CMD_RMDIR:
		{
			fun_rmdir(hwnd, szParam1);
		}break;

		case MY_CMD_EVT:
		{
		}break;
	}
	ExecuteGrep(hwnd);
	g_iCount = 1;

	g_iReDirect = 0;
	memset(g_szRedirectFileName, 0x00, sizeof(g_szRedirectFileName));
}

int GetCommandCode(TCHAR * szTemp)
{
	if ( _tcscmp(szTemp, _T("cd")) == 0)
		return MY_CMD_CD;
	else if ( _tcscmp(szTemp, _T("cat")) == 0)
		return MY_CMD_CAT;
	else if ( _tcscmp(szTemp, _T("ls")) == 0)
		return MY_CMD_LS;
	else if ( _tcscmp(szTemp, _T("clear")) == 0)
		return MY_CMD_CLEAR;
	else if ( _tcscmp(szTemp, _T("exit")) == 0)
		return MY_CMD_EXIT;
	else if ( _tcscmp(szTemp, _T("grep")) == 0)
		return MY_CMD_GREP;
	else if ( _tcscmp(szTemp, _T("rm")) == 0)
		return MY_CMD_RM;
	else if (_tcscmp(szTemp, _T("mv")) == 0)
		return MY_CMD_MV;
	else if ( _tcscmp(szTemp, _T("cp")) == 0)
		return MY_CMD_CP;
	else if ( _tcscmp(szTemp, _T("mkdir")) == 0)
		return MY_CMD_MKDIR;
	else if ( _tcscmp(szTemp, _T("mkfl")) == 0)
		return MY_CMD_MKFILE;
	else if ( _tcscmp(szTemp, _T("find")) == 0)
		return MY_CMD_FIND;
	else if ( _tcscmp(szTemp, _T("exec")) == 0)
		return MY_CMD_EXEC;
	else if (_tcscmp(szTemp, _T("rmdir")) == 0)
		return MY_CMD_RMDIR;
	else if ( _tcscmp(szTemp, _T("evt")) == 0)
		return MY_CMD_EVT;

	return -1;
}

int AllowCharecters(WPARAM wParam)
{
	if ((wParam >= 65 && wParam <= 90) || (wParam >= 97 && wParam <= 122))
	{
		return 1;
	}
	else if(wParam == _T('~') || wParam == _T('\\') || wParam == _T('-') || wParam == _T('|') || wParam == _T('*') || wParam == _T('.') || wParam == _T('"') || wParam == _T(':') || wParam == _T('>'))
	{
		return 1;
	}
	if ((wParam >= 48 && wParam <= 57))
	{
		return 1;
	}
	else
		return 0;
}

void GetCommandNameAndParameters(TCHAR* szCommand, int iSize1, TCHAR* szParam1, TCHAR* szParam2, TCHAR* szParam3, TCHAR* szParam4, TCHAR* szParam5, TCHAR* szParam6, TCHAR* szParam7, TCHAR* szParam8, TCHAR* szParam9, int iSize2)
{
	TCHAR* pcStart = NULL;
	TCHAR* pcEnd = NULL;
	TCHAR* pcLimit = NULL;
	TCHAR szTemp[260] = {0};
	int i = 1;

	pcStart = g_CommandBuffer;
	
	pcLimit = _tcschr(pcStart, _T('|'));

	pcEnd = _tcschr(pcStart, _T('$'));
	if (NULL != pcEnd)
	{
		_tcsncpy(szCommand, pcStart, pcEnd - pcStart);
		pcStart = pcEnd + 1;
	}
	else
	{
		_tcscpy(szCommand, pcStart);
		return;
	}

	while (i < 10)
	{
		/* 30/01/2022 */
		if (NULL != pcLimit)
		{
			if (pcStart >= pcLimit)
				break;
		}
		/* 30/01/2022 */

		pcEnd = _tcschr(pcStart, _T('$'));
		if (NULL != pcEnd)
		{
			switch (i)
			{
			case 1:
				_tcsncpy( szParam1, pcStart, pcEnd - pcStart);
				pcStart = pcEnd + 1;
				break;
			case 2:
				_tcsncpy( szParam2, pcStart, pcEnd - pcStart);
				pcStart = pcEnd + 1;
				break;
			case 3:
				_tcsncpy( szParam3, pcStart, pcEnd - pcStart);
				pcStart = pcEnd + 1;
				break;
			case 4:
				_tcsncpy( szParam4, pcStart, pcEnd - pcStart);
				pcStart = pcEnd + 1;
				break;
			case 5:
				_tcsncpy( szParam5, pcStart, pcEnd - pcStart);
				pcStart = pcEnd + 1;
				break;
			case 6:
				_tcsncpy( szParam6, pcStart, pcEnd - pcStart);
				pcStart = pcEnd + 1;
				break;
			case 7:
				_tcsncpy( szParam7, pcStart, pcEnd - pcStart);
				pcStart = pcEnd + 1;
				break;
			case 8:
				_tcsncpy( szParam8, pcStart, pcEnd - pcStart);
				pcStart = pcEnd + 1;
				break;
			case 9:
				_tcsncpy( szParam9, pcStart, pcEnd - pcStart);
				pcStart = pcEnd + 1;
				break;
			}
		}
		else
		{
			switch (i)
			{
			case 1:
				_tcscpy(szParam1, pcStart);
				pcStart = pcEnd + 1;
				break;
			case 2:
				_tcscpy(szParam2, pcStart);
				pcStart = pcEnd + 1;
				break;
			case 3:
				_tcscpy(szParam3, pcStart);
				pcStart = pcEnd + 1;
				break;
			case 4:
				_tcscpy(szParam4, pcStart);
				pcStart = pcEnd + 1;
				break;
			case 5:
				_tcscpy(szParam5, pcStart);
				pcStart = pcEnd + 1;
				break;
			case 6:
				_tcscpy(szParam6, pcStart);
				pcStart = pcEnd + 1;
				break;
			case 7:
				_tcscpy(szParam7, pcStart);
				pcStart = pcEnd + 1;
				break;
			case 8:
				_tcscpy(szParam8, pcStart);
				pcStart = pcEnd + 1;
				break;
			case 9:
				_tcscpy(szParam9, pcStart);
				pcStart = pcEnd + 1;
				break;

				/*
			case 1:
				_tcsncpy(szParam1, pcStart, pcEnd - pcStart);
				pcStart = pcEnd + 1;
				break;
			case 2:
				_tcsncpy(szParam2, pcStart, pcEnd - pcStart);
				pcStart = pcEnd + 1;
				break;
			case 3:
				_tcsncpy(szParam3, pcStart, pcEnd - pcStart);
				pcStart = pcEnd + 1;
				break;
			case 4:
				_tcsncpy(szParam4, pcStart, pcEnd - pcStart);
				pcStart = pcEnd + 1;
				break;
			case 5:
				_tcsncpy(szParam5, pcStart, pcEnd - pcStart);
				pcStart = pcEnd + 1;
				break;
			case 6:
				_tcsncpy(szParam6, pcStart, pcEnd - pcStart);
				pcStart = pcEnd + 1;
				break;
			case 7:
				_tcsncpy(szParam7, pcStart, pcEnd - pcStart);
				pcStart = pcEnd + 1;
				break;
			case 8:
				_tcsncpy(szParam8, pcStart, pcEnd - pcStart);
				pcStart = pcEnd + 1;
				break;
			case 9:
				_tcsncpy(szParam9, pcStart, pcEnd - pcStart);
				pcStart = pcEnd + 1;
				break;
				*/
			}
			break;
		}

		i++;
	}
}

void HandleTab(HWND hwnd, int* iPtr)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	TCHAR szBuffer[2024] = { 0 };
	TCHAR szPath[260] = { 0 };

	TCHAR szCommand[50] = { 0 };

	TCHAR szParam1[260] = { 0 };
	TCHAR szParam2[260] = { 0 };
	TCHAR szParam3[260] = { 0 };
	TCHAR szParam4[260] = { 0 };
	TCHAR szParam5[260] = { 0 };
	TCHAR szParam6[260] = { 0 };
	TCHAR szParam7[260] = { 0 };
	TCHAR szParam8[260] = { 0 };
	TCHAR szParam9[260] = { 0 };

	TCHAR szTemp[260] = { 0 };

	TCHAR* pcStart = NULL;
	TCHAR* pcEnd = NULL;

	TCHAR* pcStart1 = NULL;
	TCHAR* pcStart2 = NULL;
	int iCode = -1;
	HDC hdc;

	DWORD dw;

	int iBackChar = 0;
	int i = 0;
	int j = 0;
	int iLen = 0;
	int iTotalLen = 0;

	GetCommandNameAndParameters(szCommand, 50, szParam1, szParam2, szParam3, szParam4, szParam5, szParam6, szParam7, szParam8, szParam9, 260);

	iCode = GetCommandCode(szCommand);

	if (-1 == iCode)
	{
		for (i = 0; i < COMMAND_COUNT; i++)
		{
			pcStart = g_CommandBuffer;

			iLen = _tcslen(pcStart);
			if (NULL != g_szCommand[i] && _tcsncmp(g_szCommand[i], pcStart, iLen) == 0)
			{
				*pcStart = _T('\0');
				_tcscpy(pcStart, g_szCommand[i]);
				_tcscpy(szTemp, pcStart);
				_tcscat(szTemp, _T(" "));
				_tcscat(pcStart, _T("$"));

				g_iCharPos = _tcslen(g_szCommand[i]) + 1;
				g_iCaretPos = g_iCharPos;

				InvalidateRect(hwnd, NULL, TRUE);

				*iPtr = 2;

				hdc = GetDC(hwnd);
				SelectObject(hdc, g_hFont);
				GetCharWidth32(hdc, _T('$'), _T('$'), &iLen);
				iTotalLen = iLen;
				iTotalLen += GetWidthOfString(hdc, szTemp);
				*iPtr += iTotalLen;
				ReleaseDC(hwnd, hdc);
				SetCaretPos(*iPtr, 0);
				break;
			}
		}
	}
	else
	{
		GetCurrentDirectory(sizeof(szBuffer), szBuffer);

		iBackChar = 0;
		
		//pcStart1 = _tcsrchr(g_CommandBuffer, _T('$'));
		pcStart = _tcsrchr(g_CommandBuffer, _T('$'));
		if (pcStart == NULL)
			pcStart = g_CommandBuffer;
		else
			pcStart++;

		if (_tcslen(pcStart) > 0)
		{
			if (pcStart[0] == _T('~'))
			{
				hdc = GetDC(hwnd);
				SelectObject(hdc, g_hFont);
				GetCharWidth32(hdc, _T('~'), _T('~'), &iLen);
				*iPtr -= iLen;

				g_iCharPos--;
				g_iCaretPos--;

				pcEnd = pcStart + 1;
				_tcscpy(szTemp, pcEnd);
				memset(szBuffer, 0x00, sizeof(szBuffer));
				if (_tcslen(szTemp))
				{
					dw = 2023;
					GetUserName(szBuffer, &dw);

					g_iCharPos -= _tcslen(szTemp);
					iTotalLen = GetWidthOfString(hdc, szTemp);
					*iPtr -= iTotalLen;

					memset(pcStart, 0x00, strlen(pcEnd));
					_stprintf(pcStart, _T("C:\\Users\\%s%s"), szBuffer, szTemp);
					g_iCharPos += _tcslen(pcStart);
					g_iCaretPos = g_iCharPos;
				}
				else
				{
					dw = 2023;
					GetUserName(szBuffer, &dw);
					_stprintf(pcStart, _T("C:\\Users\\%s\\"), szBuffer);
					g_iCharPos += _tcslen(pcStart);
					g_iCaretPos = g_iCharPos;
				}
				memset(szBuffer, 0x00, sizeof(szBuffer));
				_tcscpy(szBuffer, pcStart);

				iTotalLen = GetWidthOfString(hdc, szBuffer);
				*iPtr += iTotalLen;
				SetCaretPos(*iPtr, 0);
				ReleaseDC(hwnd, hdc);
				InvalidateRect(hwnd, NULL, TRUE);
			}
			else//Search file in current directry if TAB is pressed
			{
				//pcStart2 = _tcsrchr(g_CommandBuffer, _T('$'));
				pcStart = _tcsrchr(g_CommandBuffer, _T('$'));
				if (NULL == pcStart)
					return;
				pcStart++;
				pcStart1 = _tcsrchr(pcStart, _T('\\'));

				if ( pcStart1 == NULL)
				{
					//if not absolute path is not found then file will search in current directory
					pcStart1 = _tcschr(g_CommandBuffer, _T('$'));
					if (NULL != pcStart1)
					{
						pcStart1++;
						_tcscpy(szTemp, pcStart);
					}
					GetCurrentDirectory(260, szPath);
					_tcscat(szPath, _T("\\*.*"));

					hFile = FindFirstFile(szPath, &ffd);

					if (hFile != INVALID_HANDLE_VALUE)
					{
						do
						{
							if (_tcscmp(ffd.cFileName, _T(".")) == 0 || _tcscmp(ffd.cFileName, _T("..")) == 0)
								continue;
							else
							{
								if (_tcsstr(ffd.cFileName, szTemp) != NULL)
								{
									hdc = GetDC(hwnd);
									SelectObject(hdc, g_hFont);
									iLen = GetWidthOfString(hdc, szTemp);
									ReleaseDC(hwnd, hdc);

									*iPtr -= iLen;

									iLen = _tcslen(szTemp);
									memset(szTemp, 0x00, sizeof(szTemp));
									_tcscpy(szTemp, ffd.cFileName);
									if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
									{
										_tcscat(szTemp, _T("\\"));
									}
									g_iCharPos -= iLen;
									g_iCaretPos = g_iCharPos;

									g_CommandBuffer[g_iCharPos] = _T('\0');

									pcStart = &g_CommandBuffer[g_iCharPos];

									_tcscpy(pcStart, szTemp);
									iLen = _tcslen(szTemp);

									g_iCharPos += iLen;
									g_iCaretPos = g_iCharPos;

									hdc = GetDC(hwnd);
									SelectObject(hdc, g_hFont);
									iLen = GetWidthOfString(hdc, szTemp);
									ReleaseDC(hwnd, hdc);

									*iPtr += iLen;

									SetCaretPos(*iPtr, 0);
									break;
								}
							}
						} while (FindNextFile(hFile, &ffd) != 0);
						FindClose(hFile);

						InvalidateRect(hwnd, NULL, TRUE);
					}
				}
				else if ( NULL != pcStart1 && pcStart1[1] != _T('\0') )
				{
					//if absolute path found then file will search in absolute Path
					pcStart1++;
					_tcscpy(szTemp, pcStart1);

					pcStart1 = _tcsrchr(g_CommandBuffer, _T('$'));

					if (pcStart1 != NULL)
						pcStart1++;
					else
						pcStart1 = g_CommandBuffer;

					pcEnd = _tcsrchr(g_CommandBuffer, _T('\\'));
					if (NULL != pcEnd)
					{
						_tcsncpy(szPath, pcStart, pcEnd - pcStart);
						_tcscat(szPath, _T("\\*.*"));

						hFile = FindFirstFile(szPath, &ffd);

						if (hFile != INVALID_HANDLE_VALUE)
						{
							do
							{
								if (_tcscmp(ffd.cFileName, _T(".")) == 0 || _tcscmp(ffd.cFileName, _T("..")) == 0)
									continue;
								else
								{
									if (_tcsstr(ffd.cFileName, szTemp) != NULL)
									{
										hdc = GetDC(hwnd);
										SelectObject(hdc, g_hFont);
										iLen = GetWidthOfString(hdc, szTemp);
										ReleaseDC(hwnd, hdc);

										*iPtr -= iLen;

										iLen = _tcslen(szTemp);
										memset(szTemp, 0x00, sizeof(szTemp));
										_tcscpy(szTemp, ffd.cFileName);
										if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
										{
											_tcscat(szTemp, _T("\\"));
										}

										g_iCharPos -= iLen;
										g_iCaretPos = g_iCharPos;

										g_CommandBuffer[g_iCharPos] = _T('\0');

										pcStart = &g_CommandBuffer[g_iCharPos];

										_tcscpy(pcStart, szTemp);
										iLen = _tcslen(szTemp);

										g_iCharPos += iLen;
										g_iCaretPos = g_iCharPos;

										hdc = GetDC(hwnd);
										SelectObject(hdc, g_hFont);
										iLen = GetWidthOfString(hdc, szTemp);
										ReleaseDC(hwnd, hdc);

										*iPtr += iLen;

										SetCaretPos(*iPtr, 0);
										break;
									}
								}
							} while (FindNextFile(hFile, &ffd) != 0);
							FindClose(hFile);

							InvalidateRect(hwnd, NULL, TRUE);
						}
					}

				}
			}
		}
	}
	ReplaceSpaceWithDoller();
}

/*void HandleTab(HWND hwnd, int *iPtr) original
{
	WIN32_FIND_DATA ffd;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	TCHAR szBuffer[2024] = { 0 };
	TCHAR szPath[260] = { 0 };

	TCHAR szCommand[50] = { 0 };

	TCHAR szParam1[260] = { 0 };
	TCHAR szParam2[260] = { 0 };
	TCHAR szParam3[260] = { 0 };
	TCHAR szParam4[260] = { 0 };
	TCHAR szParam5[260] = { 0 };
	TCHAR szParam6[260] = { 0 };
	TCHAR szParam7[260] = { 0 };
	TCHAR szParam8[260] = { 0 };
	TCHAR szParam9[260] = { 0 };

	TCHAR szTemp[260] = { 0 };

	TCHAR *pcStart = NULL;
	TCHAR* pcEnd = NULL;

	TCHAR *pcStart1 = NULL;
	TCHAR* pcStart2 = NULL;
	int iCode = -1;
	HDC hdc;

	DWORD dw;
	
	int iBackChar = 0;
	int i = 0;
	int j = 0;
	int iLen = 0;
	int iTotalLen = 0;
		
	GetCommandNameAndParameters(szCommand, 50, szParam1, szParam2, szParam3, szParam4, szParam5, szParam6, szParam7, szParam8, szParam9, 260);

	iCode = GetCommandCode(szCommand);

	if ( -1 == iCode)
	{
		for (i = 0; i<COMMAND_COUNT; i++)
		{
			pcStart1 = g_MainBuffer;
			pcStart2 = g_CommandBuffer;

			iLen = _tcslen(pcStart1);
			if ( _tcsncmp ( g_szCommand[i], pcStart1, iLen) == 0 )
			{
				*pcStart1 = _T('\0');
				*pcStart2 = _T('\0');
				_tcscpy(pcStart1, g_szCommand[i]);
				_tcscpy(pcStart2, g_szCommand[i]);
				_tcscat(pcStart1, _T(" "));
				_tcscat(pcStart2, _T("$"));

				g_iCharPos = _tcslen(g_szCommand[i])+1;
				g_iCaretPos = g_iCharPos;

				InvalidateRect(hwnd, NULL, TRUE);
				
				*iPtr = 2;
				
				hdc = GetDC(hwnd);
				SelectObject(hdc, g_hFont);
				GetCharWidth32(hdc, _T('$'), _T('$'), &iLen);
				iTotalLen = iLen;
				iTotalLen += GetWidthOfString(hdc, pcStart1);
				*iPtr += iTotalLen;
				ReleaseDC(hwnd, hdc);
				SetCaretPos(*iPtr, 0);
				break;
			}
		}
	}
	else
	{
		GetCurrentDirectory(sizeof(szBuffer), szBuffer);

		iBackChar = 0;
		pcStart = _tcschr(g_MainBuffer, _T(' '));
		if (pcStart == NULL)
			pcStart = g_MainBuffer;
		else
			pcStart++;

		pcStart1 = _tcschr(g_CommandBuffer, _T('$'));
		if (pcStart1 == NULL)
			pcStart1 = g_CommandBuffer;
		else
			pcStart1++;

		if ( _tcslen(pcStart) > 0)
		{
			if (pcStart[0] == _T('~'))
			{
				hdc = GetDC(hwnd);
				SelectObject(hdc, g_hFont);
				GetCharWidth32(hdc, _T('~'), _T('~'), &iLen);
				*iPtr -= iLen;

				g_iCharPos--;
				g_iCaretPos--;

				pcEnd = pcStart + 1;
				_tcscpy(szTemp, pcEnd);
				memset(szBuffer, 0x00, sizeof(szBuffer));
				if (_tcslen(szTemp))
				{
					dw = 2023;
					GetUserName(szBuffer, &dw);

					g_iCharPos -= _tcslen(szTemp);
					iTotalLen = GetWidthOfString(hdc, szTemp);
					*iPtr -= iTotalLen;

					memset(pcStart, 0x00, strlen(pcEnd));
					_stprintf(pcStart, _T("C:\\Users\\%s%s"), szBuffer, szTemp);
					_stprintf(pcStart1, _T("C:\\Users\\%s%s"), szBuffer, szTemp);
					//g_iCharPos += _tcslen(szBuffer) + _tcslen(szTemp) + 1;
					g_iCharPos += _tcslen(pcStart);
					g_iCaretPos = g_iCharPos;
				}
				else
				{
					dw = 2023;
					GetUserName(szBuffer, &dw);
					_stprintf(pcStart, _T("C:\\Users\\%s\\"), szBuffer);
					_stprintf(pcStart1, _T("C:\\Users\\%s\\"), szBuffer);
					g_iCharPos += _tcslen(pcStart);
					g_iCaretPos = g_iCharPos;
				}
				memset(szBuffer, 0x00, sizeof(szBuffer));
				_tcscpy(szBuffer, pcStart);
								
				iTotalLen = GetWidthOfString(hdc, szBuffer);
				*iPtr += iTotalLen;
				SetCaretPos(*iPtr, 0);
				ReleaseDC(hwnd, hdc);
				InvalidateRect(hwnd, NULL, TRUE);
			}
			else//Search file in current directry is TAB is pressed
			{
				pcStart = _tcsrchr(g_MainBuffer, _T('\\'));
				pcStart1 = _tcsrchr(g_CommandBuffer, _T('\\'));

				if (pcStart == NULL && pcStart1 == NULL)
				{
					//if not absolute path is not found then file will search in current directory
					pcStart = _tcschr(g_MainBuffer, _T(' '));
					pcStart1 = _tcschr(g_CommandBuffer, _T('$'));
					if (NULL != pcStart && NULL != pcStart1 )
					{
						pcStart++;
						pcStart1++;
						_tcscpy(szTemp, pcStart);
					}
					GetCurrentDirectory(260, szPath);
					_tcscat(szPath, _T("\\*.*"));

					hFile = FindFirstFile(szPath, &ffd);

					if (hFile != INVALID_HANDLE_VALUE)
					{
						do
						{
							if (_tcscmp(ffd.cFileName, _T(".")) == 0 || _tcscmp(ffd.cFileName, _T("..")) == 0)
								continue;
							else
							{
								if (_tcsstr(ffd.cFileName, szTemp) != NULL)
								{
									hdc = GetDC(hwnd);
									SelectObject(hdc, g_hFont);
									iLen = GetWidthOfString(hdc, szTemp);
									ReleaseDC(hwnd, hdc);

									*iPtr -= iLen;

									iLen = _tcslen(szTemp);
									memset(szTemp, 0x00, sizeof(szTemp));
									_tcscpy(szTemp, ffd.cFileName);
									if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
									{
										_tcscat(szTemp, _T("\\"));
									}
									g_iCharPos -= iLen;
									g_iCaretPos = g_iCharPos;

									g_MainBuffer[g_iCharPos] = _T('\0');

									pcStart = &g_MainBuffer[g_iCharPos];

									_tcscpy(pcStart, szTemp);
									_tcscpy(pcStart1, szTemp);
									iLen = _tcslen(szTemp);

									g_iCharPos += iLen;
									g_iCaretPos = g_iCharPos;

									hdc = GetDC(hwnd);
									SelectObject(hdc, g_hFont);
									iLen = GetWidthOfString(hdc, szTemp);
									ReleaseDC(hwnd, hdc);

									*iPtr += iLen;

									SetCaretPos(*iPtr, 0);
									break;
								}
							}
						} while (FindNextFile(hFile, &ffd) != 0);
						FindClose(hFile);

						InvalidateRect(hwnd, NULL, TRUE);
					}
				}
				else if ((NULL != pcStart && pcStart[1] != _T('\0')) && (NULL != pcStart1 && pcStart1[1] != _T('\0')))
				{
					//if absolute path found then file will search in absolute Path
					pcStart++;
					pcStart1++;
					_tcscpy(szTemp, pcStart);

					pcStart = _tcschr(g_MainBuffer, _T(' '));
					pcStart1 = _tcschr(g_CommandBuffer, _T('$'));
					
					if (pcStart != NULL)
						pcStart++;
					else
						pcStart = g_MainBuffer;

					if (pcStart1 != NULL)
						pcStart1++;
					else
						pcStart1 = g_CommandBuffer;

					pcEnd = _tcsrchr(g_MainBuffer, _T('\\'));
					if (NULL != pcEnd)
					{
						_tcsncpy(szPath, pcStart, pcEnd-pcStart);
						_tcscat(szPath, _T("\\*.*"));

						hFile = FindFirstFile(szPath, &ffd);

						if (hFile != INVALID_HANDLE_VALUE)
						{
							do
							{
								if (_tcscmp(ffd.cFileName, _T(".")) == 0 || _tcscmp(ffd.cFileName, _T("..")) == 0)
									continue;
								else
								{
									if (_tcsstr(ffd.cFileName, szTemp) != NULL)
									{
										hdc = GetDC(hwnd);
										SelectObject(hdc, g_hFont);
										iLen = GetWidthOfString(hdc, szTemp);
										ReleaseDC(hwnd, hdc);

										*iPtr -= iLen;

										iLen = _tcslen(szTemp);
										memset(szTemp, 0x00, sizeof(szTemp));
										_tcscpy(szTemp, ffd.cFileName);
										if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
										{
											_tcscat(szTemp, _T("\\"));
										}

										g_iCharPos -= iLen;
										g_iCaretPos = g_iCharPos;

										g_MainBuffer[g_iCharPos] = _T('\0');
										g_CommandBuffer[g_iCharPos] = _T('\0');

										pcStart = &g_MainBuffer[g_iCharPos];
										pcStart1 = &g_CommandBuffer[g_iCharPos];

										_tcscpy(pcStart, szTemp);
										_tcscpy(pcStart1, szTemp);
										iLen = _tcslen(szTemp);

										g_iCharPos += iLen;
										g_iCaretPos = g_iCharPos;

										hdc = GetDC(hwnd);
										SelectObject(hdc, g_hFont);
										iLen = GetWidthOfString(hdc, szTemp);
										ReleaseDC(hwnd, hdc);

										*iPtr += iLen;

										SetCaretPos(*iPtr, 0);
										break;
									}
								}
							} while (FindNextFile(hFile, &ffd) != 0);
							FindClose(hFile);

							InvalidateRect(hwnd, NULL, TRUE);
						}
					}

				}
			}
		}
	}
}
*/

int GetWidthOfString(HDC hdc, TCHAR * pcStart1)
{
	// get charecter width of string
	int j = 0;
	int iLen = 0;
	int iTotalLen = 0;
	while (j < _tcslen(pcStart1))
	{
		GetCharWidth32(hdc, pcStart1[j], pcStart1[j], &iLen);
		iTotalLen += iLen;
		j++;
	}

	return iTotalLen;
}

void MoveCaretLeftRight(HWND hwnd, int *iPtr, int iFlag)
{
	HDC hdc;
	int iLen = 0;

	int iCharPos = g_iCaretPos;
	if (iFlag == 0)//left
	{
		if (iCharPos == 0)
			return;

		iCharPos--;
		hdc = GetDC(hwnd);
		SelectObject(hdc, g_hFont);
		GetCharWidth32(hdc, g_MainBuffer[iCharPos], g_MainBuffer[iCharPos], &iLen);
		ReleaseDC(hwnd, hdc);

		*iPtr -= iLen;
	}
	else if (iFlag == 1)//right
	{
		if (iCharPos == _tcslen(g_MainBuffer))
			return;
		hdc = GetDC(hwnd);
		SelectObject(hdc, g_hFont);
		GetCharWidth32(hdc,g_MainBuffer[iCharPos], g_MainBuffer[iCharPos], &iLen);
		ReleaseDC(hwnd,hdc);
		iCharPos++;

		*iPtr += iLen;
	}
	else if (iFlag == 2)//Home
	{
		if (iCharPos == 0)
			return;
		hdc = GetDC(hwnd);
		SelectObject(hdc, g_hFont);
		GetCharWidth32(hdc, _T('$'), _T('$'), &iLen);
		ReleaseDC(hwnd, hdc);
		iCharPos = 0;

		*iPtr = iLen+2;
	}
	else if (iFlag == 3)//End
	{
		if (iCharPos == _tcslen(g_MainBuffer))
			return;
		hdc = GetDC(hwnd);
		SelectObject(hdc, g_hFont);
		GetCharWidth32(hdc, _T('$'), _T('$'), &iLen);
		iLen += GetWidthOfString(hdc, g_MainBuffer);
		ReleaseDC(hwnd, hdc);
		iCharPos = _tcslen(g_MainBuffer);

		*iPtr = iLen + 2;
	}
	SetCaretPos(*iPtr, 0);
	g_iCaretPos = iCharPos;
}

void DumpOutput(int iFlag, TCHAR* szBuffer)
{
	FILE* fptr = NULL;

	if (iFlag == 0)
	{
		if (g_iReDirect)
			fptr = _tfopen(g_szRedirectFileName, _T("w"));
		else
			fptr = _tfopen(OUTPUTFILENAME, _T("w"));

		if (fptr == NULL)
			return;

		fclose(fptr);

		return;
	}
	else if (iFlag == 1)
	{
		if (g_iReDirect)
			fptr = _tfopen(g_szRedirectFileName, _T("a+"));
		else
			fptr = _tfopen(OUTPUTFILENAME, _T("a+"));
	}

	if (fptr == NULL)
		return;

	_fputts(szBuffer, fptr);
	if(NULL == _tcschr(szBuffer, _T('\n')))
		_fputts(_T("\n"), fptr);

	fclose(fptr);
}

/*
void ShowOutput(HWND hwnd, int iFirstLine, int iLastLine)
{
	size_t Length;
	HRESULT hr;
	int iLineNumber = 1;
	FILE* fptr = NULL;
	HDC hdc;
	RECT rc;
	TCHAR szBuffer[512] = { 0 };
	int i = 0;

	fptr = _tfopen(OUTPUTFILENAME, _T("r"));

	if (fptr == NULL)
		return;

	hdc = GetDC(hwnd);
	SetBkColor(hdc, RGB(200, 200, 200));
	GetWindowRect(hwnd, &rc);
	InvalidateRect(hwnd, &rc, FALSE);

	while (!feof(fptr))
	{
		memset(szBuffer, 0x00, sizeof(szBuffer));
		_fgetts(szBuffer, sizeof(szBuffer), fptr);

		i++;
		if (i < g_fauzan)
			continue;
		if (i == g_ahmed)
			break;

		hr = StringCchLength(szBuffer, 100, &Length);
		
		if ((FAILED(hr)) | (Length == 0))
			break;

		TextOut(hdc, 0, iLineNumber * 20, szBuffer, (int)Length);
		iLineNumber++;
	}
	ReleaseDC(hwnd, hdc);

	fclose(fptr);
	InvalidateRect(hwnd, NULL, TRUE);
}
*/

void ReplaceSpaceWithDoller()
{
	int i = 0;

	memset(g_MainBuffer, 0x000,sizeof(g_MainBuffer));
	for (i=0; i< _tcslen(g_CommandBuffer); i++)
	{
		if (g_CommandBuffer[i] == _T('$'))
			g_MainBuffer[i] = _T(' ');
		else
			g_MainBuffer[i] = g_CommandBuffer[i];
		
	}
}
