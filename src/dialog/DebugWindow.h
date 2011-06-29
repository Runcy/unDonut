/**
 *	@file	DebugWindow.h
 *	@brief	�Ƃ肠�����I�ȓK���f�o�b�O�p�_�C�A���O�𐶐�����N���X
 */
#pragma once

#include <atlbase.h>
#include <atlapp.h>
#include <atlmisc.h>
#include <atlctrls.h>
#include <atlcrack.h>
//#include "resource.h"
#ifndef IDD_DEBUGWINDOW
#define IDD_DEBUGWINDOW                 308
#endif

const bool	g_cnt_b_use_debug_window	=	false;

// DEBUGPUT(_T("test%s\n"), _T("test"));
#ifdef _DEBUG
 #ifndef DEBUGPUT
  #define DEBUGPUT	CDebugWindow::OutPutString
 #endif
 #ifndef DEBUGMENU
  #define DEBUGMENU	CDebugWindow::OutPutMenu
 #endif
#else
 #ifndef DEBUGPUT
  #define DEBUGPUT	__noop
 #endif 
 #ifndef DEBUGMENU
  #define DEBUGMENU	__noop
 #endif
#endif


/**
	�Ƃ肠�����I�ȓK���f�o�b�O�p�_�C�A���O�𐶐�����N���X

	�����[�X�r���h���ɂ̓R���p�C�����炳��Ȃ��悤�ɂ��ׂ��ł͂��邪�A���޸�('A`)
	�g������Create()��Destroy()�̌Ăяo���̊Ԃ�OutPutString���Ăяo������
 */

class CDebugWindow 
	: public CDialogImpl<CDebugWindow>
	, public CMessageFilter
{
	typedef CDialogImpl<CDebugWindow>  baseclass;

	CEdit					m_wndEdit;
	static CDebugWindow*	s_pThis;

public:
	enum { IDD = IDD_DEBUGWINDOW };

	void		Create(HWND hWndParent);
	void		Destroy();
	CEdit		GetEditCtrl() { return m_wndEdit; }

	static void OutPutString(LPCTSTR pstrFormat, ...);
	static void OutPutMenu(CMenuHandle menu);


    virtual BOOL PreTranslateMessage(MSG* pMsg) { 
		return CWindow::IsDialogMessage(pMsg); 
	}

	// Message map
	BEGIN_MSG_MAP( CDebugWindow )
		MSG_WM_INITDIALOG	( OnInitDialog	)
		MSG_WM_DESTROY		( OnDestroy		)
		MSG_WM_SIZE			( OnSize		)
		COMMAND_ID_HANDLER_EX( IDCANCEL, OnCancel )
	END_MSG_MAP()

	BOOL		OnInitDialog(CWindow wndFocus, LPARAM lInitParam);
	void		OnDestroy();
	LRESULT 	OnSize(UINT, CSize size);

	void		OnCancel(UINT uNotifyCode, int nID, CWindow wndCtl);
};


