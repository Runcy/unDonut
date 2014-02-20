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
	int		nMimimulHilightTextLength;
	
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


// ====================================

#include <functional>
#include <unordered_map>

enum ObserverClass {
	kSupressPopupOption,
	kLoginDataManager,
	kCustomContextMenuOption,
	kUrlSecurityOption,
	kAcceleratorOption,
};

/// �q�v���Z�X�������p����N���X
/// ���̃N���X�ɍX�V�̒ʒm���˗�����

class CSharedDataChangeSubject
{
public:
	static void	AddObserver(ObserverClass obclass, std::function<void (HWND)> callback);
	static void	RemoveObserver(ObserverClass obclass);

	static void NotifyFromMainFrame(ObserverClass obclass, HWND hWndMainFrame);

private:
	// Data members
	static std::unordered_map<ObserverClass, std::function<void (HWND)>> s_mapOBClassAndCallBack;
};

/// ���C���t���[���������p����N���X
/// CSharedDataChangeSubject ���Ɨ����Ōq�����Ă��� �e�q�v���Z�X�ɍX�V�̒ʒm���s����

class CSharedDataChangeNotify
{
public:
	static void	NotifyObserver(ObserverClass obclass);
};














