/**
 *	@file	Donut.h
 *	@brief	Donut�ŗL��Window���b�Z�[�W��`�{����悤�̃T�u���[�`��.
 */

#pragma once

#include "option/MainOption.h"
#include  "DonutDefine.h"

//-------------------------------------------------------------------------------------


inline IWebBrowser2 *DonutGetIWebBrowser2(HWND hWnd)
{
	return (IWebBrowser2 *) ::SendMessage(hWnd, WM_USER_GET_IWEBBROWSER, 0, 0);
}


/// Donut�Ńt�@�C����URL���J��
void	DonutOpenFile(const CString &strFileOrURL);
void	DonutOpenFile(const CString &strFileOrURL, DWORD dwOpenFlag);

/// ctrl �� shift ��������Ă���� �^�u�̍쐬�t���O���g�O������
inline void 	DonutToggleOpenFlag(DWORD &dwFlag)
{
	if (::GetAsyncKeyState(VK_CONTROL) < 0 || ::GetAsyncKeyState(VK_SHIFT) < 0) {
		if ( _check_flag(D_OPENFILE_NOCREATE, dwFlag) )
			dwFlag &= ~D_OPENFILE_NOCREATE;
		else
			dwFlag |= D_OPENFILE_NOCREATE;
	}
}

/// �I�v�V�����Ŏw�肳�ꂽ�W���̃I�[�v�����@��Ԃ�
inline DWORD	DonutGetStdOpenFlag()
{
	DWORD	dwFlag = 0;
	if ( !_check_flag(MAIN_EX_NEWWINDOW, CMainOption::s_dwMainExtendedStyle) )
		dwFlag |= D_OPENFILE_NOCREATE;
	if ( !_check_flag(MAIN_EX_NOACTIVATE, CMainOption::s_dwMainExtendedStyle) )
		dwFlag |= D_OPENFILE_ACTIVATE;

	DonutToggleOpenFlag(dwFlag);
	return dwFlag;
}

/// �Ԃ����t���O�� �^�u�쐬 & �I�v�V�����Őݒ肳��Ă���� �A�N�e�B�u�ɂ���t���O��Ԃ�
inline DWORD DonutGetStdOpenActivateFlag()
{
	DWORD	dwFlag = 0;

	if ( !_check_flag(MAIN_EX_NOACTIVATE, CMainOption::s_dwMainExtendedStyle) )
		dwFlag |= D_OPENFILE_ACTIVATE;

	return dwFlag;
}


//==============================================================================
/// Donut.cpp�Ɏ������Ă���֐�

void	PerseUrls(LPCTSTR lpszCommandline, std::vector<CString>& vecUrls);

void	CommandLineArg(CMainFrame& wndMain, LPTSTR lpstrCmdLine);

void 	_PrivateTerm();


