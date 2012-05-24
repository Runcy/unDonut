/**
*	@file	MultiThreadManager.h
*	@brief	CChildFrame�p�̃X���b�h�쐬�Ǘ�
*/

#pragma once

// �O���錾
class CChildFrame;
struct NewChildFrameData;


namespace MultiThreadManager {

int Run(LPTSTR lpstrCmdLine, int nCmdShow, bool bTray);


#define NEWCHILDFRAMESHAREDMEMORYNAME	_T("DonutChildFrameSharedMemoryData")

struct NewChildFrameProcessData {
	HWND	hWndParent;
	WCHAR	url[INTERNET_MAX_URL_LENGTH];
	DWORD	dwDLCtrl;
	DWORD	dwExStyle;
	DWORD	dwAutoRefresh;
	bool	bActive;
	bool	bLink;

	// AutoHilight
	bool	bAutoHilight;
	WCHAR	searchWord[512];
};

bool	RunChildProcessMessageLoop(HINSTANCE hInstance);


void	ExecuteChildFrameThread(CChildFrame* pChild, NewChildFrameData* pData);


};	// namespace MultiThreadManager





