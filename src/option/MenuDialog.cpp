/**
 *	@file	MenuOption.cpp
 *	@brief	donut�̃I�v�V���� : ���j���[
 */

#include "stdafx.h"
#include "MenuDialog.h"
#include "MainOption.h"
#include "RightClickMenuDialog.h"
#include "..\IniFile.h"
#include "..\DonutPFunc.h"
#include "..\ToolTipManager.h"
#include "..\ItemIDList.h"
#include "..\PopupMenu.h"

using namespace MTL;

////////////////////////////////////////////////////////////
// CMenuOption

bool	CMenuOption::s_bNoCustomMenu	= false;
bool	CMenuOption::s_bNoCustomIEMenu	= true;
bool	CMenuOption::s_bR_Equal_L		= false;

// ini����ݒ��ǂݍ���
void	CMenuOption::GetProfile()
{
	{
		CIniFileI	pr(g_szIniFileName, _T("Menu"));
		DWORD dwExStyle = pr.GetValue(_T("ExStyle"), 0);
		s_bNoCustomMenu		= (dwExStyle & MENU_EX_NOCUSTOMMENU		) != 0;
		s_bNoCustomIEMenu	= (dwExStyle & MENU_EX_NOCUSTOMIEMENU	) != 0;
		s_bR_Equal_L		= (dwExStyle & MENU_EX_R_EQUAL_L		) != 0;
	}
}

// ini�֐ݒ��ۑ�����
void	CMenuOption::WriteProfile()
{
	{
		DWORD	dwExStyle = 0;
		if (s_bNoCustomMenu)
			dwExStyle |= MENU_EX_NOCUSTOMMENU;
		if (s_bNoCustomIEMenu)
			dwExStyle |= MENU_EX_NOCUSTOMIEMENU;
		if (s_bR_Equal_L)
			dwExStyle |= MENU_EX_R_EQUAL_L;

		CIniFileO	pr(g_szIniFileName, _T("Menu"));
		pr.SetValue(dwExStyle		, _T("ExStyle"));
		
	}
}



////////////////////////////////////////////////////////////////////////
// CFavoritesMenuOption

bool	CFavoritesMenuOption::s_bPackItem	= false;


// ini����ݒ��ǂݍ���
void CFavoritesMenuOption::GetProfile()
{
	CIniFileI	pr( g_szIniFileName, _T("FavoritesMenu") );
	s_bPackItem = pr.GetValue(_T("PackItem"), s_bPackItem) != 0;
}

// ini�֐ݒ��ۑ�����
void CFavoritesMenuOption::WriteProfile()
{
	CIniFileIO	pr( g_szIniFileName, _T("FavoritesMenu") );
	pr.SetValue(s_bPackItem, _T("PackItem"));
}


////////////////////////////////////////////////////////////
// CMenuPropertyPage

// Constructor
CMenuPropertyPage::CMenuPropertyPage()
	: m_bInit(false)
{
}


// Destructor
CMenuPropertyPage::~CMenuPropertyPage()
{
}



// Overrides
BOOL CMenuPropertyPage::OnSetActive()
{
	SetModified(TRUE);

	if (m_bInit == false) {
		m_bInit = true;
	}

	return DoDataExchange(DDX_LOAD);
}



BOOL CMenuPropertyPage::OnKillActive()
{
	return TRUE;
}



BOOL CMenuPropertyPage::OnApply()
{
	if ( DoDataExchange(DDX_SAVE) ) {
		// �f�[�^��ۑ�
		CMenuOption::WriteProfile();

		CFavoritesMenuOption::WriteProfile();

		return TRUE;
	} else {
		return FALSE;
	}
}


namespace {

CString BrowseForFolder(HWND hwnd)
{
	TCHAR	szDisplayName[MAX_PATH+1] = _T("\0");
	BROWSEINFO	bi = {
		hwnd, NULL, szDisplayName, _T("�t�H���_�I��"),
		BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE, 
		NULL, 0,				0
	};

	CItemIDList idl;
	idl.Attach( ::SHBrowseForFolder(&bi) );
	return idl.GetPath();
}

};	// namespace


/// �����N���t�H���_����C���|�[�g����
void CMenuPropertyPage::OnLinkImportFromFolder(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (MessageBox(_T("�C���|�[�g����ƌ��݂̃����N�͂��ׂč폜����܂����A��낵���ł����H"), _T("�m�F"), MB_YESNO) == IDNO)
		return ;
	CWaitCursor	waitCursor;
	CString folder = BrowseForFolder(m_hWnd);
	if (folder.GetLength() > 0) {
		CRootFavoritePopupMenu::LinkImportFromFolder(folder);
	}
}

/// �����N���t�H���_�փG�N�X�|�[�g����
void CMenuPropertyPage::OnLinkExportToFolder(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	bool bOverWrite = false;
	switch (MessageBox(_T("�t�H���_�Ƀ����N�����łɂ��鎞�A�����N���㏑�����܂����H"), _T("�m�F"), MB_YESNOCANCEL)) {
	case IDYES:	bOverWrite = true;	break;
	case IDNO:	bOverWrite = false;	break;
	case IDCANCEL:
		return ;
	}
	CWaitCursor	waitCursor;
	CString folder = BrowseForFolder(m_hWnd);
	if (folder.GetLength() > 0) {
		CRootFavoritePopupMenu::LinkExportToFolder(folder, bOverWrite);
	}
}


