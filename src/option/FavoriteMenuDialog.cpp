/**
 *	@file	FavoriteMenuDialog.cpp
 *	@brief	donut�̃I�v�V���� : "���C�ɓ��胁�j���["
 */

#include "stdafx.h"
#include "FavoriteMenuDialog.h"
#include "..\PopupMenu.h"

////////////////////////////////////////////////////////////////////////
// CFavoritesMenuOption

// ini����ݒ��ǂݍ���
void CFavoritesMenuOption::GetProfile()
{


}

// ini�֐ݒ��ۑ�����
void CFavoritesMenuOption::WriteProfile()
{
}




////////////////////////////////////////////////////////////////////////////////
// CDonutFavoritesMenuPropertyPage


// Constructor
CDonutFavoritesMenuPropertyPage::CDonutFavoritesMenuPropertyPage()
	: m_bInit(false)
{
}


// Overrides
BOOL CDonutFavoritesMenuPropertyPage::OnSetActive()
{
	SetModified(TRUE);

	if (m_bInit == false) {
		m_bInit = true;

		DoDataExchange(DDX_LOAD);
	}

	return TRUE;
}


BOOL CDonutFavoritesMenuPropertyPage::OnKillActive()
{
	return TRUE;
}


BOOL CDonutFavoritesMenuPropertyPage::OnApply()
{
	if ( DoDataExchange(DDX_SAVE) ) {

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
void CDonutFavoritesMenuPropertyPage::OnLinkImportFromFolder(UINT uNotifyCode, int nID, CWindow wndCtl)
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
void CDonutFavoritesMenuPropertyPage::OnLinkExportToFolder(UINT uNotifyCode, int nID, CWindow wndCtl)
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

