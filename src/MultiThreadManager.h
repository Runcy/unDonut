/**
*	@file	MultiThreadManager.h
*	@brief	CChildFrame�p�̃X���b�h�쐬�Ǘ�
*/

#pragma once

#include <vector>

// �O���錾
class CChildFrame;
struct NewChildFrameData;


namespace MultiThreadManager {

/// ���C���t���[���p�̃��b�Z�[�W���[�v
int		RunMainFrameMessageLoop(LPTSTR lpstrCmdLine, int nCmdShow, bool bTray);

/// �}���`�v���Z�X���[�h�ł̎q�v���Z�X���C�����[�v
/// �R�}���h���C�����������Ď������q�v���Z�X�Ƃ��ċN������Ă����Ȃ烋�[�v�ɓ���
bool	RunChildProcessMessageLoop(HINSTANCE hInstance);


// �O���[�o���ϐ�
struct ChildProcessProcessThreadId
{
	DWORD	dwProcessId;
	DWORD	dwThreadId;

	ChildProcessProcessThreadId(DWORD processId, DWORD threadId) : dwProcessId(processId), dwThreadId(threadId) { }
};
extern std::vector<ChildProcessProcessThreadId>	g_vecChildProcessProcessThreadId;

#define	WM_NOTIFYOBSERVERFROMMAINFRAME	(WM_APP + 400)

//===================================================================================

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

/// �}���`�X���b�h/�v���Z�X���[�h�ł̎q�X���b�h�쐬
void	CreateChildFrameThread(NewChildFrameData& data, bool bMultiProcessMode);

/// �}���`�v���Z�X���[�h�Ŏq�v���Z�X�̍쐬
void	CreateChildProcess(NewChildFrameData& data);

};	// namespace MultiThreadManager





