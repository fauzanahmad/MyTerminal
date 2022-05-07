#include "MyHeader.h"

extern int g_iCmdNo;
extern int g_iCharPos;
extern int g_iCaretPos;
extern int g_iLineNumber;

extern TCHAR g_MainBuffer[5012];
extern TCHAR g_CommandBuffer[5012];
extern TCHAR *g_pcCurrentParam;

extern TCHAR* g_szCommand[14];

void fun_ls(HWND hwnd, TCHAR *szParam, TCHAR *szParam2)
{
	HDC hdc;
	WIN32_FIND_DATA ffd;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	TCHAR szPath[260] = { 0 };
	int iCount = 0;
	TCHAR szTemp[260] = { 0 };
	TCHAR szTemp1[260] = { 0 };
	TCHAR szTemp2[260] = { 0 };
	TCHAR szTemp3[260] = { 0 };
	TCHAR *pcStart = NULL;
	int iLen = 0;
	DWORD dw;

	SYSTEMTIME sys;

	DumpOutput(WRITEMODE, NULL);
	if (_tcscmp(szParam, _T("-h")) == 0)
	{
		DumpOutput(APPENDMODE, _T("Showing Helps for 'ls' command"));
		DumpOutput(APPENDMODE, _T("ls -d    : Show Directories in Current Path."));
		DumpOutput(APPENDMODE, _T("ls -i    : Showing list of all file with details."));
		DumpOutput(APPENDMODE, _T("ls ~     : Show list of all files of home directory"));
		DumpOutput(APPENDMODE, _T("ls ..    : Show list of all files of Parent directory"));
		DumpOutput(APPENDMODE, _T("ls -all  : Show all files in current directory and parent directory."));
	}
	else if (_tcscmp(szParam, _T("-i")) == 0)
	{
		if (_tcslen(szParam2) > 0)
		{
			if (NULL != _tcsstr(szParam2, _T("*.")) || NULL != _tcsstr(szParam2, _T(".*")))
			{
				if (_tcsrchr(szParam2, _T('\\')))
				{
					_tcscpy(szPath, szParam2);
				}
				else
				{
					GetCurrentDirectory(260, szPath);
					_tcscat(szPath, _T("\\"));
					_tcscat(szPath, szParam2);
				}
			}
			else
			{
				_tcscpy(szPath, szParam2);
				iLen = _tcslen(szPath);
				pcStart = &szPath[iLen - 1];
				if (pcStart[0] == _T('\\'))
					_tcscat(szPath, _T("*.*"));
				else
					_tcscat(szPath, _T("\\*.*"));
			}
		}
		else
		{
			GetCurrentDirectory(260, szPath);
			iLen = _tcslen(szPath);
			pcStart = &szPath[iLen-1];
			if (pcStart[0] == _T('\\'))
				_tcscat(szPath, _T("*.*"));
			else
				_tcscat(szPath, _T("\\*.*"));
		}

		hFile = FindFirstFile(szPath, &ffd);
		if (INVALID_HANDLE_VALUE != hFile)
		{
			do
			{
				if (_tcscmp(ffd.cFileName, _T(".")) == 0 || _tcscmp(ffd.cFileName, _T("..")) == 0)
					continue;
				else
				{
					FileTimeToSystemTime(&ffd.ftCreationTime, &sys);
					_stprintf(szTemp1, _T("Creation time : %d/%d/%d %d:%d:%d"), sys.wDay, sys.wMonth, sys.wYear, sys.wHour, sys.wMinute, sys.wSecond);

					FileTimeToSystemTime(&ffd.ftLastAccessTime, &sys);
					_stprintf(szTemp2, _T("Last Access Time : %d/%d/%d %d:%d:%d"), sys.wDay, sys.wMonth, sys.wYear, sys.wHour, sys.wMinute, sys.wSecond);

					FileTimeToSystemTime(&ffd.ftLastWriteTime, &sys);
					_stprintf(szTemp3, _T("Last Write Time : %d/%d/%d %d:%d:%d"), sys.wDay, sys.wMonth, sys.wYear, sys.wHour, sys.wMinute, sys.wSecond);

					_stprintf(szTemp, _T("%s :\n   %s\n   %s\n   %s\n"), ffd.cFileName, szTemp1, szTemp2, szTemp3);

					DumpOutput(APPENDMODE, szTemp);
				}
			} while (FindNextFile(hFile, &ffd) != 0);

			FindClose(hFile);
		}
	}
	else if (_tcscmp(szParam, _T("-d")) == 0)
	{
		if (_tcslen(szParam2) > 0)
		{
			_tcscpy(szPath, szParam2);
			iLen = _tcslen(szPath);
			pcStart = &szPath[iLen - 1];
			if (pcStart[0] == _T('\\'))
				_tcscat(szPath, _T("*.*"));
			else
				_tcscat(szPath, _T("\\*.*"));
		}
		else
		{
			GetCurrentDirectory(260, szPath);
			iLen = _tcslen(szPath);
			pcStart = &szPath[iLen - 1];
			if (pcStart[0] == _T('\\'))
				_tcscat(szPath, _T("*.*"));
			else
				_tcscat(szPath, _T("\\*.*"));
		}

		hFile = FindFirstFile(szPath, &ffd);
		if (INVALID_HANDLE_VALUE != hFile)
		{
			do
			{
				if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					DumpOutput(APPENDMODE, ffd.cFileName);
				}
			} while (FindNextFile(hFile, &ffd) != 0);

			FindClose(hFile);
		}
	}
	else if (_tcscmp(szParam, _T("-all")) == 0)
	{
		if (_tcslen(szParam2) > 0)
		{
			_tcscpy(szPath, szParam2);

			Recursive(hwnd, szPath);
		}
		else
		{
			Recursive(hwnd, NULL);
		}
	}
	else if (_tcscmp(szParam, _T("~")) == 0)
	{
		dw = 260;
		GetUserName(szTemp, &dw);

		_stprintf(szPath, _T("C:\\Users\\%s\\*.*"), szTemp);

		hFile = FindFirstFile(szPath, &ffd);
		if (INVALID_HANDLE_VALUE != hFile)
		{
			do
			{
				DumpOutput(APPENDMODE, ffd.cFileName);
			} while (FindNextFile(hFile, &ffd) != 0);

			FindClose(hFile);
		}

	}
	else if (_tcscmp(szParam, _T("..")) == 0)
	{
		GetCurrentDirectory(260, szPath);
		pcStart = _tcsrchr(szPath, _T('\\'));
		if (NULL != pcStart)
		{
			pcStart[0] = _T('\0');
			_tcscat(szPath, _T("\\*.*"));
		}

		hFile = FindFirstFile(szPath, &ffd);
		if (INVALID_HANDLE_VALUE != hFile)
		{
			do
			{
				DumpOutput(APPENDMODE, ffd.cFileName);
			} while (FindNextFile(hFile, &ffd) != 0);

			FindClose(hFile);
		}
	}
	else
	{
		if (_tcslen(szParam) > 0)
		{
			if (NULL != _tcsstr(szParam, _T("*.")) || NULL != _tcsstr(szParam, _T(".*")))
			{
				if (_tcsrchr(szParam, _T('\\')))
				{
					_tcscpy(szPath, szParam);
				}
				else
				{
					GetCurrentDirectory(260, szPath);
					_tcscat(szPath, _T("\\"));
					_tcscat(szPath, szParam);
				}
			}
			else
			{
				_tcscpy(szPath, szParam);
				iLen = _tcslen(szPath);
				pcStart = &szPath[iLen - 1];
				if (pcStart[0] == _T('\\'))
					_tcscat(szPath, _T("*.*"));
				else
					_tcscat(szPath, _T("\\*.*"));
			}
		}
		else
		{
			GetCurrentDirectory(260, szPath);
			
			iLen = _tcslen(szPath);
			pcStart = &szPath[iLen-1];
			if (pcStart[0] == _T('\\'))
				_tcscat(szPath, _T("*.*"));
			else
				_tcscat(szPath, _T("\\*.*"));
		}
		
		hFile = FindFirstFile(szPath, &ffd);
		if (INVALID_HANDLE_VALUE != hFile)
		{
			do
			{
				if (_tcscmp(ffd.cFileName, _T(".")) == 0 || _tcscmp(ffd.cFileName, _T("..")) == 0)
					continue;
				else
					DumpOutput(APPENDMODE, ffd.cFileName);
			} while (FindNextFile(hFile, &ffd) != 0);

			FindClose(hFile);
		}
	}
	InvalidateRect(hwnd, NULL, TRUE);
}

BOOL CheckIsFileSystem(TCHAR* ptr)
{
	int iFlag = 0;
	int i = 0;
	const int iDrives = 16;
	TCHAR* szBuffer[] = { _T("C:\\"), _T("D:\\"), _T("E:\\"), _T("F:\\"), _T("G:\\"), _T("H:\\"), _T("I:\\"), _T("J:\\"), _T("K:\\"), _T("L:\\"), _T("M:\\"), _T("N:\\"), _T("O:\\"), _T("P:\\"), _T("Q:\\"), _T("R:\\"), NULL };
	for (i = 0; i < iDrives; i++)
	{
		if (_tcsstr(g_MainBuffer, szBuffer[i]))
		{
			iFlag = 1;
			break;
		}
	}

	return iFlag;
}

void fun_cd(HWND hwnd, TCHAR* szParam)
{
	TCHAR szBuffer[260] = { 0 };
	TCHAR* pcStart = g_CommandBuffer + 3;
	TCHAR* pcEnd = NULL;
	TCHAR szUserName[260] = { 0 };
	DWORD dw = 260;

	if (_tcscmp(szParam, _T("-h")) == 0)
	{
		DumpOutput(APPENDMODE, _T("Showing Helps for 'cd' command"));
		DumpOutput(APPENDMODE, _T("cd 'Path'"));
		return;
	}

	if (CheckIsFileSystem(pcStart))
	{
		_tcscpy(szBuffer, pcStart);

		if ((pcEnd = _tcsstr(pcStart, _T(":\\"))) != NULL)
		{
			_tcsncpy(szBuffer, pcStart, 3);
			if (SetCurrentDirectory(szBuffer))
				SetWindowText(hwnd, szBuffer);
			return;
		}
	}
	else
	{
		GetCurrentDirectory(260, szBuffer);
		if (_tcslen(szBuffer) == 3 && CheckIsFileSystem(szBuffer))
		{
			int iLen = _tcslen(szBuffer) - 1;
			szBuffer[iLen] = 0;
		}
	}

	pcEnd = pcStart;
	while (pcEnd)
	{
		if (_tcsstr(pcEnd, _T("..")))
		{
			pcEnd = _tcsrchr(szBuffer, _T('\\'));
			if (pcEnd != NULL)
				*pcEnd = _T('\0');

			pcEnd = _tcsstr(pcStart, _T(".."));
			
			pcStart = pcEnd + 2;
		}
		else if (_tcsstr(pcEnd, _T("~")))
		{
			memset(szBuffer, 0x00, sizeof(szBuffer));
			dw = 260;
			GetUserName(szUserName, &dw);
			_stprintf(szBuffer, _T("C:\\Users\\%s"), szUserName);
			pcEnd++;
		}
		else if ((pcEnd = _tcschr(pcStart, _T('\\'))) != NULL)
		{
			_tcscat(szBuffer, _T("\\"));
			_tcsncat(szBuffer, pcStart, pcEnd - pcStart);
			pcStart = pcEnd + 1;
		}
		pcEnd = _tcschr(pcStart, _T('\\'));
	}

	if (SetCurrentDirectory(szBuffer))
		SetWindowText(hwnd, szBuffer);
}

void fun_find(HWND hwnd, TCHAR* szParam1, TCHAR* szParam2)
{
	TCHAR szBuffer[260] = { 0 };
	TCHAR szPath[260] = { 0 };
	TCHAR szPath2[260] = { 0 };
	TCHAR szFileName[260] = { 0 };
	HANDLE hFile;
	WIN32_FIND_DATA ffd;
	HDC hdc;
	TCHAR szTemp[520] = { 0 };

	if (_tcscmp(szParam1, _T("-h")) == 0)
	{
		DumpOutput(APPENDMODE, _T("Showing Helps for 'find' command"));
		DumpOutput(APPENDMODE, _T("find fileName"));
		DumpOutput(APPENDMODE, _T("find Path fileName"));
		return;
	}

	if ( _tcslen(szParam2) == 0 )
	{
		GetCurrentDirectory(260, szBuffer);
		_tcscpy(szFileName, szParam1);
	}
	else
	{
		_tcscpy(szBuffer, szParam1);
		_tcscpy(szFileName, szParam2);
	}

	_tcscpy(szPath2, szBuffer);
	
	if ( _tcsstr(szParam1, _T("*.")) != NULL || _tcsstr(szParam1, _T(".*")) != NULL)
	{
		_tcscat(szBuffer, _T("\\"));
		_tcscat(szBuffer, szParam1);
	}
	else
	{
		_tcscat(szBuffer, _T("\\*.*"));
	}

	hFile = FindFirstFile(szBuffer, &ffd);

	if (INVALID_HANDLE_VALUE == hFile)
	{
		return;
	}

	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (_tcscmp(ffd.cFileName, _T(".")) == 0 || _tcscmp(ffd.cFileName, _T("..")) == 0)
				continue;

			_stprintf(szPath, _T("%s\\%s"), szPath2, ffd.cFileName);
			fun_find(hwnd, szPath, szFileName);
		}
		else
		{
			if (NULL != _tcsstr(ffd.cFileName, szFileName) )
			{
				_stprintf(szPath, _T("%s\\%s"), szPath2, ffd.cFileName);
				DumpOutput(APPENDMODE, szPath);
			}
		}
	} while (FindNextFile(hFile, &ffd) != 0);
	FindClose(hFile);
	InvalidateRect(hwnd, NULL, TRUE);
	return;
}

void Recursive(HWND hwnd, TCHAR * szDir)
{
	TCHAR szBuffer[260] = { 0 };
	TCHAR szPath[260] = { 0 };
	TCHAR szPath2[260] = { 0 };
	TCHAR szFileNanme[260] = { 0 };
	HANDLE hFile;
	TCHAR* pcStart = NULL;
	WIN32_FIND_DATA ffd;
	HDC hdc;
	TCHAR szTemp[520] = { 0 };

	if (NULL == szDir)
	{
		GetCurrentDirectory(sizeof(szBuffer), szBuffer);
	}
	else
	{
		_tcscpy(szBuffer, szDir);
	}
	_tcscpy(szPath2, szBuffer);
	
	_tcscat(szBuffer, _T("\\*.*"));

	hFile = FindFirstFile(szBuffer, &ffd);

	if (INVALID_HANDLE_VALUE == hFile)
	{
		MessageBeep(-1);
		return;
	}

	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (_tcscmp(ffd.cFileName, _T(".")) == 0 || _tcscmp(ffd.cFileName, _T("..")) == 0)
				continue;

			_stprintf(szPath, _T("%s\\%s"), szPath2, ffd.cFileName);
			DumpOutput(APPENDMODE, szPath);
			Recursive(hwnd, szPath);
		}
		else
		{
			_stprintf(szPath, _T("%s\\%s"), szPath2, ffd.cFileName);
			DumpOutput(APPENDMODE, szPath);
		}
	} while (FindNextFile(hFile, &ffd) != 0);
	FindClose(hFile);
}

void ExecuteGrep(HWND hwnd)
{
	TCHAR* pcStart = NULL;
	TCHAR* pcEnd = NULL;
	TCHAR szTemp[260] = { 0 };
	TCHAR szBuffer[260] = { 0 };

	FILE* fptr_r = NULL;
	FILE* fptr_w = NULL;

	pcStart = _tcsstr(g_CommandBuffer, _T("grep"));
	if (NULL != pcStart)
	{
		pcEnd = pcStart;
		pcStart = _tcschr(pcEnd, _T('"'));
		if (NULL != pcStart)
		{
			pcStart++;
			if (NULL != pcStart)
			{
				pcEnd = _tcschr(pcStart, _T('"'));
				if (NULL != pcEnd)
				{
					_tcsncpy(szTemp, pcStart, pcEnd - pcStart);

					MoveFile(OUTPUTFILENAME, _T("C:\\Windows\\Temp\\temp_grep.txt"));

					fptr_r = _tfopen(_T("C:\\Windows\\Temp\\temp_grep.txt"), _T("r"));
					fptr_w = _tfopen(OUTPUTFILENAME, _T("w"));

					if (NULL == fptr_w)
						return;
					if (NULL == fptr_r)
						return;

					while (!feof(fptr_r))
					{
						memset(szBuffer, 0x00, sizeof(szBuffer));
						_fgetts(szBuffer, sizeof(szBuffer), fptr_r);
						if (_tcsstr(szBuffer, szTemp) != NULL)
						{
							_fputts(szBuffer, fptr_w);
						}
					}
					fclose(fptr_w);
					fclose(fptr_r);

					DeleteFile(_T("C:\\Windows\\Temp\\temp_grep.txt"));
				}
			}
		}
	}
}

void fun_cat(HWND hwnd, TCHAR* szParam)
{
	FILE* fptr = NULL;
	TCHAR szBuffer[2024] = { 0 };

	if (_tcscmp(szParam, _T("-h")) == 0)
	{
		DumpOutput(APPENDMODE, _T("Showing Helps for 'cat' command"));
		DumpOutput(APPENDMODE, _T("cat fileName"));
		return;
	}

	fptr = _tfopen(szParam, _T("r"));
	if (fptr != NULL)
	{
		while (!feof(fptr))
		{
			memset(szBuffer, 0x00, sizeof(szBuffer));
			_fgetts(szBuffer, sizeof(szBuffer), fptr);
			DumpOutput(APPENDMODE, szBuffer);
		}

		fclose(fptr);
	}
	InvalidateRect(hwnd, NULL, TRUE);
}

void fun_clear(HWND hwnd)
{
	RECT rc;
	GetWindowRect(hwnd, &rc);
	InvalidateRect(hwnd, &rc, FALSE);
}

void fun_exit(HWND hwnd)
{
	exit(0);
}

BOOL fun_mkdir(HWND hwnd, TCHAR *szParam)
{
	TCHAR szBuffer[260] = { 0 };
	HANDLE hFile;
	BOOL bRet = FALSE;

	if (_tcscmp(szParam, _T("-h")) == 0)
	{
		DumpOutput(APPENDMODE, _T("Showing Helps for 'mkdir' command"));
		DumpOutput(APPENDMODE, _T("mkdir DirectoryName"));
		return;
	}

	if (szParam != NULL)
	{
		GetCurrentDirectory(sizeof(szBuffer), szBuffer);

		_tcscat(szBuffer, _T("\\"));
		_tcscat(szBuffer, szParam);

		return CreateDirectory(szBuffer, NULL);
	}

	return FALSE;
}

BOOL fun_mkfile(HWND hwnd, TCHAR* szParam)
{
	TCHAR szBuffer[260] = { 0 };
	HANDLE hFile;

	if (_tcscmp(szParam, _T("-h")) == 0)
	{
		DumpOutput(APPENDMODE, _T("Showing Helps for 'mkfl' command"));
		DumpOutput(APPENDMODE, _T("mkfl FileName"));
		return;
	}

	if (szParam != NULL)
	{
		GetCurrentDirectory(sizeof(szBuffer), szBuffer);

		_tcscat(szBuffer, _T("\\"));
		_tcscat(szBuffer, szParam);

		hFile = CreateFile(szBuffer,        // name of the write
					GENERIC_WRITE,          // open for writing
					0,                      // do not share
					NULL,                   // default security
					CREATE_NEW,             // create new file only
					FILE_ATTRIBUTE_NORMAL,  // normal file
					NULL);

		if (hFile == INVALID_HANDLE_VALUE)
			return FALSE;
		else
		{
			CloseHandle(hFile);
			return TRUE;
		}
	}

	return FALSE;
}

void fun_rm(HWND hwnd, TCHAR* szParam)
{
	WIN32_FIND_DATA ffd;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	TCHAR szBuffer[2024] = { 0 };
	TCHAR* pcStart = NULL;
	TCHAR* pcEnd = NULL;
	TCHAR szTemp[51] = { 0 };
	DWORD dw = 0;
	TCHAR szLocation[260] = {0};

	if (_tcscmp(szParam, _T("-h")) == 0)
	{
		DumpOutput(APPENDMODE, _T("Showing Helps for 'rm' command"));
		DumpOutput(APPENDMODE, _T("rm FileName"));
		DumpOutput(APPENDMODE, _T("rm *.txt"));
		DumpOutput(APPENDMODE, _T("rm abc.*"));
		DumpOutput(APPENDMODE, _T("rm *.*"));
		return;
	}

	if (_tcsstr(szParam, _T("*.")) || _tcsstr(szParam, _T(".*")))
	{
		GetCurrentDirectory(sizeof(szLocation), szLocation);
		_tcscat(szLocation, _T("\\"));
		_tcscat(szLocation, szParam);
		hFile = FindFirstFile(szLocation, &ffd);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (_tcscmp(ffd.cFileName, szParam) == 0 && ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					RemoveDirectory(ffd.cFileName);
				else
					DeleteFile(ffd.cFileName);
				dw = GetLastError();
			} while (FindNextFile(hFile, &ffd) != NULL);
			FindClose(hFile);
		}
	}
	else
	{
		dw = GetFileAttributes(szParam);
		if (dw & FILE_ATTRIBUTE_DIRECTORY)
			RemoveDirectory(szParam);
		else
			DeleteFile(szParam);
		dw = GetLastError();
	}
}

void fun_cp(HWND hwnd, TCHAR *szParam1, TCHAR* szParam2)
{
	TCHAR *szFileName = NULL;
	TCHAR szSource[260] = { 0 };
	TCHAR szDestination[260] = { 0 };
	TCHAR szTemp1[260] = { 0 };
	TCHAR szTemp2[260] = { 0 };
	TCHAR szCurrentPath[260] = { 0 };
	WIN32_FIND_DATA ffd;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dw;

	if (_tcscmp(szParam1, _T("-h")) == 0)
	{
		DumpOutput(APPENDMODE, _T("Showing Helps for 'cp' command"));
		DumpOutput(APPENDMODE, _T("cp FileName DestinationPath"));
		DumpOutput(APPENDMODE, _T("cp abc.* DestinationPath"));
		DumpOutput(APPENDMODE, _T("cp *.txt DestinationPath"));
		DumpOutput(APPENDMODE, _T("cp *.* DestinationPath"));
		return;
	}

	_tcscpy(szSource, szParam1);

	_tcscat(szDestination, szParam2);
	_tcscat(szDestination, _T("\\"));
	
	if (_tcsstr(szParam1, _T("*.")) || _tcsstr(szParam1, _T(".*")))
	{
		hFile = FindFirstFile(szSource, &ffd);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			PathRemoveFileSpec(szSource);
			if (_tcslen(szSource) == 0)
				GetCurrentDirectory(sizeof(szSource), szSource);
			
			_tcscat(szSource, _T("\\"));

			do
			{
				memset(szTemp2, 0x00, sizeof(szTemp2));
				_tcscpy(szTemp2, szSource);
				_tcscat(szTemp2, ffd.cFileName);

				memset(szTemp1, 0x00, sizeof(szTemp1));
				_tcscpy(szTemp1, szDestination);
				_tcscat(szTemp1, ffd.cFileName);

				CopyFile(szTemp2, szTemp1, TRUE);
				dw = GetLastError();
			} while (FindNextFile(hFile, &ffd) != NULL);
			FindClose(hFile);
		}
	}
	else
	{
		szFileName = PathFindFileName(szParam1);
		if (_tcslen(szFileName) > 0)
			_tcscat(szDestination, szFileName);

		CopyFile(szSource, szDestination, FALSE);
		dw = GetLastError();
	}
}

void fun_mv(HWND hwnd, TCHAR* szParam1, TCHAR* szParam2)
{
	TCHAR* szFileName = NULL;
	TCHAR szSource[260] = { 0 };
	TCHAR szDestination[260] = { 0 };
	TCHAR szTemp1[260] = { 0 };
	TCHAR szTemp2[260] = { 0 };
	TCHAR szCurrentPath[260] = { 0 };
	WIN32_FIND_DATA ffd;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dw;

	if (_tcscmp(szParam1, _T("-h")) == 0)
	{
		DumpOutput(APPENDMODE, _T("Showing Helps for 'mv' command"));
		DumpOutput(APPENDMODE, _T("mv FileName DestinationPath"));
		DumpOutput(APPENDMODE, _T("mv abc.* DestinationPath"));
		DumpOutput(APPENDMODE, _T("mv *.txt DestinationPath"));
		DumpOutput(APPENDMODE, _T("mv *.* DestinationPath"));
		return;
	}

	_tcscpy(szSource, szParam1);

	_tcscat(szDestination, szParam2);
	_tcscat(szDestination, _T("\\"));

	if (_tcsstr(szParam1, _T("*.")) || _tcsstr(szParam1, _T(".*")))
	{
		hFile = FindFirstFile(szSource, &ffd);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			PathRemoveFileSpec(szSource);
			if (_tcslen(szSource) == 0)
				GetCurrentDirectory(sizeof(szSource), szSource);

			_tcscat(szSource, _T("\\"));

			do
			{
				memset(szTemp2, 0x00, sizeof(szTemp2));
				_tcscpy(szTemp2, szSource);
				_tcscat(szTemp2, ffd.cFileName);

				memset(szTemp1, 0x00, sizeof(szTemp1));
				_tcscpy(szTemp1, szDestination);
				_tcscat(szTemp1, ffd.cFileName);

				MoveFile(szTemp2, szTemp1, TRUE);
				dw = GetLastError();
			} while (FindNextFile(hFile, &ffd) != NULL);
			FindClose(hFile);
		}
	}
	else
	{
		szFileName = PathFindFileName(szParam1);
		if (_tcslen(szFileName) > 0)
			_tcscat(szDestination, szFileName);

		MoveFile(szSource, szDestination, FALSE);
		dw = GetLastError();
	}
}

void fun_exe(HWND hwnd, TCHAR* szParam1, TCHAR* szParam2, TCHAR* szParam3, TCHAR* szParam4, TCHAR* szParam5, TCHAR* szParam6, TCHAR* szParam7, TCHAR* szParam8, TCHAR* szParam9 )
{
	TCHAR szParameter[512] = { 0 };

	if (_tcscmp(szParam1, _T("-h")) == 0)
	{
		DumpOutput(APPENDMODE, _T("Showing Helps for 'exec' command"));
		DumpOutput(APPENDMODE, _T("exe FileName ..."));
		return;
	}

	if (_tcslen(szParam1) > 0)
	{
		_tcscpy(szParameter, szParam1);
		if (_tcslen(szParam2) > 0)
			_tcscat(szParameter, szParam2);
		if (_tcslen(szParam3) > 0)
			_tcscat(szParameter, szParam3);
		if (_tcslen(szParam4) > 0)
			_tcscat(szParameter, szParam4);
		if (_tcslen(szParam5) > 0)
			_tcscat(szParameter, szParam5);
		if (_tcslen(szParam6) > 0)
			_tcscat(szParameter, szParam6);
		if (_tcslen(szParam7) > 0)
			_tcscat(szParameter, szParam7);
		if (_tcslen(szParam8) > 0)
			_tcscat(szParameter, szParam8);
		if (_tcslen(szParam9) > 0)
			_tcscat(szParameter, szParam9);
		
		ShellExecute(NULL, _T("open"), szParameter, NULL, NULL, SW_SHOWNORMAL);
	}	
}

void fun_redirect(HWND hwnd, TCHAR* szParam)
{
	if (_tcslen(szParam) > 0)
		CopyFile(OUTPUTFILENAME, szParam, FALSE);
}