/**
 *	@file	CustomContextMenu.h
 *	@brief	atlhostEx.h����ړ������J�X�^���R���e�L�X�g���j���[�̏�������
 */
#pragma once

#include "MtlUser.h"
#include "ToolTipManager.h"
#include "DonutPFunc.h"

#if 1	//+++
#include <FlatComboBox.h>
#include "DonutSearchBar.h"		//+++ GetSearchEngineMenu�̂���
#include "Donut.h"			//+++ Donut_GetActiveStatusStr()�̂���
#endif

#include "option\MenuDialog.h"
#include "option\RightClickMenuDialog.h"

/*
	CCustomContextMenu
	atlhostEx.h����ړ������J�X�^���R���e�L�X�g���j���[�̏�������

	�����S���������������B�Ȃɂ���Ă���̂�������Ȃ��������B�������͎̂����ł���(minit)
 */

///////////////////////////////////////////////////////////////////
// CCustomContextMenu

class CCustomContextMenu 
{
	HWND		m_hWndTopLevel;
	HWND		m_hWndTarget;
  #if 1	//++
	int			m_nContextMenuMode;							//+++ CONTEXT_MENU_DEFAULT,CONTEXT_MENU_TEXTSELECT,CONTEXT_MENU_ANCHOR
	POINT		m_pt;										//+++
	CString		m_strUrl;									//+++
	static CCustomContextMenu*	s_pThis;					//+++
  #endif
	IDispatch*	m_spDisp;
	
public:
	// Constructor/Destructor
	CCustomContextMenu();
	~CCustomContextMenu();

	static CCustomContextMenu* 	GetInstance() { return s_pThis; }							//+++
	int 						GetContextMenuMode() const { return m_nContextMenuMode; }	//+++
	const CString& 				GetAnchorUrl() const { return m_strUrl; }					//+++
	const POINT& 				GetPoint() 	const { return m_pt; }							//+++
	HWND						GetTopLevelWindow() { return m_hWndTopLevel; }
	IDispatch*					GetDispatch() const { return m_spDisp; }

	HRESULT		Show(DWORD dwId, DWORD x, DWORD y, IUnknown* pCommandTarget, IDispatch* pDisp);

private:

	HRESULT		ShowContextMenuEx(POINT *pptPosition);

	// MSHTML requests to display its context menu
	STDMETHOD	(ShowContextMenu) (DWORD dwID, POINT * pptPosition, IUnknown * pCommandTarget, IDispatch * pDispatchObjectHit);

	void		_RestoreSpecialMenu(DWORD dwCmd);
	void		_BeforeInitSpecialMenu(DWORD dwCmd);

	void		CstmContextMenu(HMENU hMenu, DWORD dwID, CSimpleMap<DWORD, DWORD> &mapCmd, CSimpleArray<HMENU> &aryDestroyMenu);
	void		CstmContextMenuDropdown(HMENU hMenu, DWORD dwCmd, CSimpleArray<HMENU> &aryDestroyMenu);

	void		_SetMenuEnable(HMENU hMenu, IOleCommandTarget* spOleCommandTarget);

};

__declspec(selectany) CCustomContextMenu*	CCustomContextMenu::s_pThis = 0;

