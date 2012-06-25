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

/// �}���`�v���Z�X���[�h�ł̎q�v���Z�X���C�����[�v
bool	RunChildProcessMessageLoop(HINSTANCE hInstance);

/// �}���`�v���Z�X���[�h�ł̎q�X���b�h�쐬
void	AddChildThread(NewChildFrameData* pData);


/// �}���`�X���b�h���[�h�ł̎q�X���b�h�쐬
void	ExecuteChildFrameThread(CChildFrame* pChild, NewChildFrameData* pData);


};	// namespace MultiThreadManager





