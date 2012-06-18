/**
 *	@file	CmbboxPrxy.h
 *	@brief	�R���{�{�b�N�X�F�v���L�V
 */

#pragma once

#include <time.h>
#include "FlatComboBox.h"


// �O���錾
struct GlobalConfig;

/*
	CComboBoxPrxy
 */
class CComboBoxPrxyR : public CFlatComboBox 
{
public:
	CComboBoxPrxyR();

	void	SetGlobalConfig(GlobalConfig* p) { m_pGlobalConfig = p; }

	BEGIN_MSG_MAP(CComboBoxPrxyR)
		MESSAGE_HANDLER(WM_RBUTTONDOWN, OnRButtonDown)
		MESSAGE_HANDLER(WM_COMMAND, OnCommand)

		MSG_WM_TIMER(OnTimer)
		CHAIN_MSG_MAP(CFlatComboBox)
	END_MSG_MAP()

	bool	IsUseIE() const { return m_bUseIE; }
	void	ResetProxyList();

private:
	LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnRButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	void	OnTimer(UINT_PTR wTimerID, TIMERPROC dmy=0);
	void	OnSelectChange();

	void	ChangeProxy(int nIndex);
	void	SetProxy();
	void	ResetTimer();

	// IE�̂��g��
	bool	UseIE();

	// �o�C�p�X�𓾂�
	CString GetBypass();

	// Data members
	GlobalConfig*	m_pGlobalConfig;
	UINT_PTR	m_nIDEvent;
	bool		m_bUseIE;
};


