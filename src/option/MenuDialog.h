/**
 *	@file	MenuOption.h
 *	@brief	donut�̃I�v�V���� : ���j���[
 */

#pragma once

#include "../resource.h"


enum EMenuEx {
	MENU_EX_NOCUSTOMMENU	= 0x00000001L,	// ���j���[�̕�����ҏW���Ȃ�
	MENU_EX_NOCUSTOMIEMENU	= 0x00000002L,	// �E�N���b�N���j���[���J�X�^�����Ȃ�
	MENU_EX_R_EQUAL_L		= 0x00000004L,	// �E�N���b�N���j���[��̉E�N���b�N��...
	//MENU_EX_DONTSHOWBUTTON	= 0x00000008L,	// ���j���[�ɍŏ���etc..�{�^����\�����Ȃ�
};

////////////////////////////////////////////////////////////
// CMenuOption

class CMenuOption
{
public:
	static bool		s_bNoCustomMenu;	// ���j���[�̕�����ҏW���Ȃ�
	static bool		s_bNoCustomIEMenu;	// �E�N���b�N���j���[���J�X�^�����Ȃ�
	static bool		s_bR_Equal_L;		// �E�N���b�N���j���[��̉E�N���b�N��...

public:
	static void		GetProfile();
	static void		WriteProfile();

};


////////////////////////////////////////////////////////////////////////
// CFavoritesMenuOption

class CFavoritesMenuOption 
{
public:
	static bool	s_bPackItem;	// �A�C�e���̊Ԋu���l�߂�

public:
	static void		GetProfile();
	static void		WriteProfile();

};



////////////////////////////////////////////////////////////
// CMenuPropertyPage : [Donut�̃I�v�V����] - [���j���[]

class CMenuPropertyPage : 
	public CPropertyPageImpl<CMenuPropertyPage>,
	public CWinDataExchange<CMenuPropertyPage>,
	protected CMenuOption
{
public:
	// Constants
	enum { IDD = IDD_PROPPAGE_MENU };

	// Constructor/Destructor
	CMenuPropertyPage();
	~CMenuPropertyPage();

	// Overrides
	BOOL	OnSetActive();
	BOOL	OnKillActive();
	BOOL	OnApply();

	// DDX map
	BEGIN_DDX_MAP( CMenuPropertyPage )
		DDX_CHECK( IDC_NO_CSTM_TXT_MENU 	, s_bNoCustomMenu )
		DDX_CHECK( IDC_NO_CSTM_IE_MENU		, s_bNoCustomIEMenu)
		DDX_CHECK( IDC_CHECK_R_EQUAL_L		, s_bR_Equal_L	)

		DDX_CHECK( IDC_CHECK_PACK_ITEM	, CFavoritesMenuOption::s_bPackItem	)
	END_DDX_MAP()

	// Message map and handlers
	BEGIN_MSG_MAP( CMenuPropertyPage )

		COMMAND_ID_HANDLER_EX( IDC_BUTTON_LINKIMPORTFORMFOLDER	, OnLinkImportFromFolder	)
		COMMAND_ID_HANDLER_EX( IDC_BUTTON_LINKEXPORTTOFOLDER	, OnLinkExportToFolder		)

		CHAIN_MSG_MAP( CPropertyPageImpl<CMenuPropertyPage> )
	END_MSG_MAP()

	void OnLinkImportFromFolder(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnLinkExportToFolder(UINT uNotifyCode, int nID, CWindow wndCtl);

private:

	// Data members
	bool	m_bInit;


};

