#include <Windows.h>
#include <tchar.h>
#include <string.h>
#include <time.h>
#include <shlwapi.h>

//#pragma once
#pragma warning(disable:4996)

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void HandleKeyPress(HWND, WPARAM, int*);
void HandleEnterKeyPress(HWND, WPARAM);
int GetCommandCode(TCHAR*);
int AllowCharecters(WPARAM);
void GetCommandNameAndParameters(TCHAR*, int, TCHAR*, TCHAR*, TCHAR*, TCHAR*, TCHAR*, TCHAR*, TCHAR*, TCHAR*, TCHAR*, int);
void HandleTab(HWND, int*);
void MoveCaretLeftRight(HWND, int*, int);
//void ShowOutput(HWND hwnd, int, int);
void DumpOutput(int, TCHAR*);
void ReplaceSpaceWithDoller();
void ExecuteGrep(HWND);
void fun_cp(HWND, TCHAR*, TCHAR*);
void fun_mv(HWND, TCHAR*, TCHAR*);

BOOL CheckIsFileSystem(TCHAR* ptr);

/* command functions Declarations */
void fun_ls(HWND, TCHAR*);
void fun_find(HWND, TCHAR*, TCHAR*);
void Recursive(HWND, TCHAR*);
void fun_cd(HWND, TCHAR*);
void fun_cat(HWND, TCHAR*);
void fun_clear(HWND hwnd);
void fun_exit(HWND);
BOOL fun_mkdir(HWND, TCHAR*);
BOOL fun_mkfile(HWND, TCHAR*);
void fun_rm(HWND, TCHAR*);
void fun_redirect(HWND, TCHAR*);

#define LINES		5000
#define WRITEMODE	0
#define APPENDMODE	1
#define OUTPUTFILENAME _T("C:\\Windows\\Temp\\MyTerminalOutput.txt")
#define COMMANDFILENAME _T("C:\\Windows\\Temp\\PrevCommands.ini")

#define COMMAND_COUNT	16

#define MY_CMD_CAT		0
#define MY_CMD_GREP		1
#define MY_CMD_LS		2
#define MY_CMD_CD		3
#define MY_CMD_CLEAR	4
#define MY_CMD_EXIT		5
#define MY_CMD_RM		6
#define MY_CMD_CP		7
#define MY_CMD_MKDIR	8
#define MY_CMD_MKFILE	9
#define MY_CMD_FIND		10
#define MY_CMD_EXEC		11
#define MY_CMD_GREP		12
#define MY_CMD_EVT		13
#define MY_CMD_MV		14
#define MY_CMD_RMDIR	15

/*

VK_UP and VK_DOWN

*/