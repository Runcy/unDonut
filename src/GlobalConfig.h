/**
*	@file	GlobalConfig.h
*	@brief	MainFrame��ChildFrame�ŋ��L����ݒ�
*/

#pragma once


struct GlobalConfig
{
	//CMainOption
	DWORD	dwMainExtendedStyle;
	DWORD	dwMainExtendedStyle2;
	int		AutoImageResizeType;
	bool	bMultiProcessMode;

	// CMouseOption
	bool	bUseRightDragSearch;
	bool	bUseRect;
	WCHAR	strREngine[256];
	WCHAR	strTEngine[256];
	WCHAR	strLEngine[256];
	WCHAR	strBEngine[256];
	WCHAR	strCEngine[256];
	int		nDragDropCommandID;

	// CMenuOption
	bool	bNoCustomIEMenu;

	// CDLControlOption
	DWORD	dwDLControlFlags;
	DWORD	dwExtendedStyleFlags;

	bool	bChangeUserAgent;
	WCHAR	strUserAgent[256];
	WCHAR	strUserAgentCurrent[256];

	// CSearchBarOption
	bool	bScrollCenter;
	bool	bSaveSearchWord;
	bool	bSaveSearchWordOrg;
	HWND	SearchEditHWND;
	
	// SerachBar
	bool	bHilightSwitch;

	// CAddressBarOption
	bool	bReplaceSpace;

	// CUrlSecurityOption
	bool	bUrlSecurityValid;
	
	// CDownloadManager
	bool	bUseDownloadManager;
	// CDLOptions
	bool	bShowDLManagerOnDL;
	WCHAR	strDefaultDLFolder[MAX_PATH];
	WCHAR	strImageDLFolder[MAX_PATH];
	DWORD	dwDLImageExStyle;

	// ProxyComboBox
	char	ProxyAddress[512];
	char	ProxyBypass[512];

	bool	bMainFrameClosing;

};

struct GlobalConfigManageData
{
	HANDLE hMap;
	GlobalConfig* pGlobalConfig;
};

// MainFrame���쐬/�j������
void	CreateGlobalConfig(GlobalConfigManageData* pMangeData);
void	SetGlobalConfig(GlobalConfig* pConfig);
void	DestroyGlobalConfig(GlobalConfigManageData* pManageData);

// ChildFrame���擾/�ԋp����
GlobalConfigManageData GetGlobalConfig(HWND hWndMainFrame);
void	CloseGlobalConfig(GlobalConfigManageData* pMangeData);





















