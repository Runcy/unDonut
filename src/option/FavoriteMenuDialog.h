/**
 *	@file	FavoriteMenuDialog.h
 *	@brief	donut�̃I�v�V���� : "���C�ɓ��胁�j���["
 */

#pragma once

#include "../resource.h"

////////////////////////////////////////////////////////////////////////
// CFavoritesMenuOption

struct CFavoritesMenuOption 
{
	static void		GetProfile();
	static void		WriteProfile();
};


/////////////////////////////////////////////////////////////////
// CDonutFavoritesMenuPropertyPage : [Donut�̃I�v�V����] - [���C�ɓ��胁�j���[]

class CDonutFavoritesMenuPropertyPage
	: public CPropertyPageImpl<CDonutFavoritesMenuPropertyPage>
	, public CWinDataExchange<CDonutFavoritesMenuPropertyPage>
	, protected CFavoritesMenuOption
{
public:
	// Constants
	enum { IDD = IDD_PROPPAGE_FAVORITEMENU };

	// Constructor
	CDonutFavoritesMenuPropertyPage();

	// Overrides
	BOOL	OnSetActive();
	BOOL	OnKillActive();
	BOOL	OnApply();


	// DDX map
	BEGIN_DDX_MAP( CDonutFavoritesMenuPropertyPage )
	END_DDX_MAP()

	// Message map and handlers
	BEGIN_MSG_MAP( CDonutFavoritesMenuPropertyPage )
		COMMAND_ID_HANDLER_EX( IDC_BUTTON_LINKIMPORTFORMFOLDER	, OnLinkImportFromFolder	)
		COMMAND_ID_HANDLER_EX( IDC_BUTTON_LINKEXPORTTOFOLDER	, OnLinkExportToFolder		)
		CHAIN_MSG_MAP( CPropertyPageImpl<CDonutFavoritesMenuPropertyPage> )
	END_MSG_MAP()

	void OnLinkImportFromFolder(UINT uNotifyCode, int nID, CWindow wndCtl);
	void OnLinkExportToFolder(UINT uNotifyCode, int nID, CWindow wndCtl);

private:
	// Data members
	bool	m_bInit;
};



