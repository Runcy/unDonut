/**
 *	@file	MainFrame.cpp
 *	@brief	���C���t���[���̎���
 *	@note
 *		+++ mainfrm.h �� �w�b�_��cpp�ɕ����B�܂��A�N���X���ɕ����ăt�@�C������MainFrame�ɕύX.
 */
#include "stdafx.h"

#include "MainFrame.h"

#include "XmlFile.h"

#include "DialogKiller.h"
#include "dialog/DebugWindow.h"
//#include "PropertySheet.h"
#include "DonutOption.h"
#include "MenuEncode.h"
#include "StyleSheetOption.h"
#include "ExStyle.h"
#include "MenuDropTargetWindow.h"
#include "ParseInternetShortcutFile.h"

#include "option/AddressBarPropertyPage.h"	//+++ AddressBar.h��蕪��
#include "option/SearchPropertyPage.h"		//+++ SearchBar.h��蕪��
#include "option/LinkBarPropertyPage.h" 	//+++
#include "option/UrlSecurityOption.h"		//+++
#include "option/RightClickMenuDialog.h"

#include "DialogHook.h"
#include "dialog/OpenURLDialog.h"
#include "api.h"
#include "PluginEventImpl.h"
#include "Thumbnail.h"
#include "FaviconManager.h"
#include "GdiplusUtil.h"


#ifdef _DEBUG
	const bool _Donut_MainFrame_traceOn = false;
	#define dmfTRACE	if (_Donut_MainFrame_traceOn)  ATLTRACE
#else
	#define dmfTRACE
#endif


#if defined USE_ATLDBGMEM
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif




static const CLSID CLSID_IEnumPrivacyRecords = { 0x3050f844, 0x98b5, 0x11cf, { 0xbb, 0x82, 0x00, 0xaa, 0x00, 0xbd, 0xce, 0x0b } };

const UINT		CMainFrame::STDBAR_ID[] 	= { ATL_IDW_COMMAND_BAR, ATL_IDW_TOOLBAR				, IDC_ADDRESSBAR, IDC_MDITAB , IDC_LINKBAR				   , IDC_SEARCHBAR					};
const UINT		CMainFrame::STDBAR_STYLE[]	= { RBBS_USECHEVRON    , RBBS_USECHEVRON | RBBS_BREAK	, RBBS_BREAK	, RBBS_BREAK , RBBS_USECHEVRON | RBBS_BREAK, RBBS_BREAK 					};
//const UINT	CMainFrame::STDBAR_STYLE[]	= { RBBS_USECHEVRON    , RBBS_USECHEVRON | RBBS_BREAK	, RBBS_BREAK	, RBBS_BREAK , RBBS_USECHEVRON | RBBS_BREAK, RBBS_USECHEVRON | RBBS_BREAK	};
const LPTSTR	CMainFrame::STDBAR_TEXT[]	= { _T("")/*NULL*/	   , _T("")/*NULL*/ 				,_T("�A�h���X") , NULL		 , _T("�����N") 			   , _T("����") 					};	// memo. NULL ���ƈ�ԍ��̃{�^�����V�F�u�����Ɋ܂߂邱�Ƃ��ł��Ȃ�


#ifdef _DEBUG
void	CMainFrame::OnDebugCommand(UINT uNotifyCode, int nID, CWindow wndCtl)
{

	RestoreAllTab();
}
#endif



// ===========================================================================
// ������

CMainFrame::CMainFrame()
	: m_FavoriteMenu(this, ID_INSERTPOINT_FAVORITEMENU)
	, m_FavGroupMenu(this, ID_INSERTPOINT_GROUPMENU)
	, m_LinkBar(this, ID_INSERTPOINT_FAVORITEMENU)
	, m_nBackUpTimerID(0)
	, m_wndMDIClient(this, 1)
	, CDDEMessageHandler<CMainFrame>( _T("Donut") )
	, m_hWndFocus(NULL)
	, m_ExplorerBar(m_wndSplit)
	, m_ScriptMenu(ID_INSERTPOINT_SCRIPTMENU, _T("(empty)"), ID_INSERTPOINT_SCRIPTMENU, ID_INSERTPOINT_SCRIPTMENU_END)
	, m_DropScriptMenu(ID_INSERTPOINT_SCRIPTMENU, _T("(empty)"), ID_INSERTPOINT_SCRIPTMENU, ID_INSERTPOINT_SCRIPTMENU_END)
	, m_bShow(FALSE)
	, m_hGroupThread(NULL)
	, m_OnUserOpenFile_nCmdShow(0)
	, m_nMenuBarStyle(-1)		//+++
	, m_nBgColor(-1)			//+++
	, m_bTray(0)				//+++
	, m_bFullScreen(0)			//+++
	, m_bOldMaximized(0)		//+++
	//, m_bMinimized(0)			//+++
//	, m_DownloadManager(this)
	, m_TranslateMenu(this)
	, m_bCancelRButtonUp(false)
	, m_styleSheetMenu(m_hWnd)
	, m_bWM_TIMER(false)
{
	g_pMainWnd = this;			//+++ CreateEx()���Ƀv���O�C���������Ƃ��ŎQ�Ƃ����̂ŁA�Ăь��łȂ� CMainFreame�Őݒ肷��悤�ɕύX.
}



//+++ �e�������������ƂɊ֐�����
/** ���C���t���[���쐬. �e�평����.
 */
LRESULT CMainFrame::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	bHandled = FALSE;

	init_message_loop();							// ���b�Z�[�W���[�v�̏���

	// �I�v�V�����ł��C�ɓ��胁�j���[�̕\���ύX���A���C�ɓ��胁�j���[�X�V
	CFavoritesMenuOption::SetFuncRefreshFav(std::bind(&CMainFrame::_RefreshFavMenu, this));

#ifdef _DEBUG
	m_wndDebug.Create(m_hWnd);						// �f�o�b�O�E�B���h�E�̍쐬
#endif
	init_menus_infomation();						// ���j���[���̏�����

	HWND hWndCmdBar 	= init_commandBarWindow();	// ���j���[�o�[�̏�����
	HWND hWndToolBar	= init_toolBar();			// �c�[���o�[�̏�����
	HWND hWndAddressBar	= init_addressBar();		// �A�h���X�o�[�̏�����
	HWND hWndSearchBar	= init_searchBar();			// �����o�[�̏�����
	HWND hWndMDITab		= init_tabCtrl();			// �^�u�o�[�̏�����
	HWND hWndLinkBar	= init_linkBar();			// �����N�o�[�̏�����

	m_FindBar.Create(m_hWnd);
	m_FindBar.SetUpdateLayoutFunc(boost::bind(&CMainFrame::UpdateLayout, this, _1));

	init_rebar();									// ���o�[�̏�����
	init_statusBar();								// �X�e�[�^�X�o�[�̏�����
	init_pluginManager();							// �v���O�C���}�l�[�W���̏�����

	// �e��o�[�̔z�u
	init_band_position( hWndCmdBar,hWndToolBar,hWndAddressBar,hWndMDITab,hWndLinkBar,hWndSearchBar );

	init_loadStatusBarState();						// �X�e�[�^�X�o�[�̏�Ԑݒ�
	init_mdiClientWindow();							//
	init_splitterWindow();							// ������(�G�N�X�v���[���o�[�ƒʏ�y�[�W)�̏�����
	init_explorerBar();								// �G�N�X�v���[���o�[�̏�����
	init_mdiClient_misc(hWndCmdBar, hWndToolBar);	// mdi-client�֌W�̎G���Ȑݒ�

	CmdUIAddToolBar(hWndToolBar);					// set up UI
	//CmdUIAddToolBar(m_SearchBar.m_wndToolBar);	// set up UI
	CmdUIAddToolBar(m_SearchBar.GetHWndToolBar());	// set up UI

	CDLControlOption::SetUserAgent();				// ���[�U�[�G�[�W�F���g�̐ݒ�

	//SetAutoBackUp();//OnBackUpOptionChanged(0,0,0);// OnCreate��̏����ŕʓr�Ăяo���悤�ɂ���.
	RegisterDragDrop();	//DragAcceptFiles();		// �h���b�O���h���b�v����

	//\\?CCriticalIdleTimer::InstallCriticalIdleTimer(m_hWnd, ID_VIEW_IDLE);

	init_sysMenu();									// �V�X�e�����j���[�ɒǉ�

	SetTimer(ENT_READ_ACCEL, 200);					// �A�N�Z���L�[�ǂݍ���. �x�������邽�߂�WM_TIMER�ŏ���.

	InitSkin();										//�X�L����������

	CDonutSimpleEventManager::RaiseEvent(EVENT_INITIALIZE_COMPLETE);	// �C�x���g�֌W�̏���
	CDialogHook::InstallHook(m_hWnd);				// �_�C�A���O�֌W�̏���

	m_DownloadManager.SetParent(m_hWnd);

	init_loadPlugins();								// �v���O�C����ǂݍ���

	UpdateLayout();									// ��ʍX�V

	GdiplusInit();

	return 0;	//return lRet;
}

//-------------------------------------
/// initialize menus' infomation
void CMainFrame::init_menus_infomation()
{
	/* ���C�ɓ��胁�j���[�ݒ� */
	CMenuHandle 	   menu 		  = m_hMenu;
	CMenuHandle 	   menuFav		  = menu.GetSubMenu(_nPosFavoriteMenu);
	m_FavoriteMenu.InstallExplorerMenu(menuFav);
	m_FavoriteMenu.SetTargetWindow(m_hWnd);
	m_FavoriteMenu.RefreshMenu();

	/* ���C�ɓ���O���[�v�ݒ� */
	CMenuHandle 	   menuGroup	  = menuFav.GetSubMenu(_nPosFavGroupMenu);
	m_FavGroupMenu.InstallExplorerMenu(menuGroup);
	m_FavGroupMenu.SetTargetWindow(m_hWnd);
	m_FavGroupMenu.RefreshMenu();

	/* �X�^�C���V�[�g���j���[�ݒ� */
	CMenuHandle 	   menuCss		  = menu.GetSubMenu(_nPosCssMenu);
	CMenuHandle 	   menuCssSub	  = menuCss.GetSubMenu(_nPosSubCssMenu);
	m_styleSheetMenu.SetRootDirectoryPath( Misc::GetExeDirectory() + _T("CSS") );
	m_styleSheetMenu.SetTargetWindow(m_hWnd);
	m_styleSheetMenu.InstallExplorerMenu(menuCssSub);
	m_styleSheetMenu.RefreshMenu();

	/* �X�N���v�g���j���[�ݒ� */
	m_ScriptMenuMst.LoadMenu(IDR_SCRIPT);
	m_ScriptMenu.SetRootDirectoryPath( Misc::GetExeDirectory() + _T("Script") );
	m_ScriptMenu.SetTargetWindow(m_hWnd);
	m_ScriptMenu.InstallExplorerMenu(m_ScriptMenuMst);
	m_ScriptMenu.RefreshMenu();

	/* DropDown���j���[�̕��̃��[�U�[�X�N���v�g���j���[��ݒ� */
	OnMenuGetScript();

	/* �G���R�[�h���j���[�ݒ� */
	CMenuHandle 	   menuEncode	  = menu.GetSubMenu(_nPosEncodeMenu);
	m_MenuEncode.Init(menuEncode, m_hWnd, _nPosEncodeMenuSub);

	// MRU list
	CMainOption::SetMRUMenuHandle(menu, _nPosMRU);

	//MenuDropTaget
	m_wndMenuDropTarget.Create(m_hWnd, rcDefault, _T("MenuDropTargetWindow"), WS_POPUP, 0);
	m_wndMenuDropTarget.SetTargetMenu(menu);
}

//-------------------------------------
/// create command bar window
HWND	CMainFrame::init_commandBarWindow()
{
	SetMenu(NULL);		// remove menu
	HWND	hWndCmdBar	  = m_CmdBar.Create(m_hWnd,rcDefault,NULL,MTL_SIMPLE_CMDBAR2_PANE_STYLE,0,ATL_IDW_COMMAND_BAR);
	ATLASSERT( ::IsWindow(hWndCmdBar) );

	CIniFileI		prFont( g_szIniFileName, _T("Main") );
	MTL::CLogFont	lf;
	lf.InitDefault();
	if ( lf.GetProfile(prFont) ) {
		m_CmdBar.SetMenuLogFont(lf);

		CFontHandle 	font;	//\\ SetFont�������ł���悤��
		MTLVERIFY( font.CreateFontIndirect(&lf) );
		if (font.m_hFont) 
			SetFont(font);
	}

	m_CmdBar.AttachMenu(m_hMenu);
	m_CmdBar.EnableButton(_nPosWindowMenu, false);
	return hWndCmdBar;
}

//-------------------------------------
/// create toolbar
HWND	CMainFrame::init_toolBar()
{
  #if 1 //+++ �֐������̓s���擾���Ȃ���
	CMenuHandle 	   menu 		  = m_hMenu;
	CMenuHandle 	   menuFav		  = menu.GetSubMenu(_nPosFavoriteMenu);
	CMenuHandle 	   menuGroup	  = menuFav.GetSubMenu(_nPosFavGroupMenu);
	CMenuHandle 	   menuCss		  = menu.GetSubMenu(_nPosCssMenu);
	CMenuHandle 	   menuCssSub	  = menuCss.GetSubMenu(_nPosSubCssMenu);
  #endif

	HWND	hWndToolBar   = m_ToolBar.Create(m_hWnd);
	ATLASSERT( ::IsWindow(hWndToolBar) );

	m_ToolBar.SetDropDownMenu(menuFav, menuGroup, menuCssSub);

	if (m_CmdBar.m_fontMenu.m_hFont) {	// �R�}���h�o�[�̃t�H���g�ݒ�Ɠ�����
		LOGFONT lf;
		m_CmdBar.m_fontMenu.GetLogFont(&lf);
		CFontHandle font;
		m_ToolBar.SetFont(font.CreateFontIndirect(&lf));
	}

	return hWndToolBar;
}

//-------------------------------------
/// create addressbar
HWND	CMainFrame::init_addressBar()
{
	HWND	hWndAddressBar = m_AddressBar.Create(m_hWnd, IDC_ADDRESSBAR, ID_VIEW_GO,
												 16, 16, RGB(255, 0, 255) );
	ATLASSERT( ::IsWindow(hWndAddressBar) );

	//m_AddressBar.m_comboFlat.SetDrawStyle(CSkinOption::s_nComboStyle);
	return hWndAddressBar;
}

//-------------------------------------
/// create searchbar
HWND	CMainFrame::init_searchBar()
{
	HWND	hWndSearchBar  = m_SearchBar.Create(m_hWnd);
	ATLASSERT( ::IsWindow(hWndSearchBar) );

	return	hWndSearchBar;
}

//-------------------------------------
/// create tabctrl
HWND	CMainFrame::init_tabCtrl()
{
	HWND	hWndMDITab = m_MDITab.Create(m_hWnd);
	ATLASSERT( ::IsWindow(hWndMDITab) );

	CFaviconManager::Init(hWndMDITab);

	return hWndMDITab;
}

//------------------------------------
/// create link bar
HWND	CMainFrame::init_linkBar()
{
	CIniFileI prLnk( g_szIniFileName, _T("LinkBar") );
	DWORD		dwStyle   = prLnk.GetValue( _T("ExtendedStyle") );
	prLnk.Close();

	m_LinkBar.SetOptionalStyle(dwStyle);
	HWND	hWndLinkBar   = m_LinkBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_TOOLBAR_PANE_STYLE, 0, IDC_LINKBAR);
	ATLASSERT( ::IsWindow(hWndLinkBar) );

	return 	hWndLinkBar;
}

//------------------------------------
/// create rebar
void	CMainFrame::init_rebar()
{
	DWORD	dwRebarStyle = ATL_SIMPLE_REBAR_STYLE | CCS_NOPARENTALIGN | CCS_NODIVIDER | RBS_DBLCLKTOGGLE;
	{
		CIniFileI	pr( g_szIniFileName, _T("ReBar") );
		DWORD		dwNoBoader 		= pr.GetValue( _T("NoBoader") );
		if (dwNoBoader)
			dwRebarStyle &= ~RBS_BANDBORDERS;	// �{�[�_�[������
	}

	CreateSimpleReBar(dwRebarStyle);
	m_ReBar.SubclassWindow(m_hWndToolBar);

	//CreateSimpleReBar(MTL_SIMPLE_REBAR_STYLE | RBS_DBLCLKTOGGLE);
}

//------------------------------------
/// create statusbar
void	CMainFrame::init_statusBar()
{
	//CreateSimpleStatusBar();
	m_wndStatusBar.Create(m_hWnd, ATL_IDS_IDLEMESSAGE,
							 WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP | SBT_TOOLTIPS);
	ATLASSERT( ::IsWindow(m_wndStatusBar) );
	m_hWndStatusBar = m_wndStatusBar.m_hWnd;
	//		int nPanes[] = { ID_DEFAULT_PANE, ID_PROGRESS_PANE, ID_COMBOBOX_PANE};
	static int   nPanes[] = { ID_DEFAULT_PANE, ID_PROGRESS_PANE, ID_PRIVACY_PANE, ID_SECURE_PANE, ID_COMBOBOX_PANE };
	m_wndStatusBar.SetPanes( nPanes, _countof(nPanes), false );
	m_wndStatusBar.SetCommand( ID_PRIVACY_PANE, ID_PRIVACYREPORT );
	m_wndStatusBar.SetCommand( ID_SECURE_PANE, ID_SECURITYREPORT);
	m_wndStatusBar.SetIcon( ID_PRIVACY_PANE, 1 );				//minit
	m_wndStatusBar.SetIcon( ID_SECURE_PANE , 0 );				//minit
	m_wndStatusBar.SetOwnerDraw( m_wndStatusBar.IsValidBmp() );


	enum {	//+++ ID�̓f�t�H���g�̖��O�ɂȂ��Ă��邪�A���������ꍇ�ɂ�₱�����̂ŁA���̈�A���̈�Ƃ��Ĉ���.
		ID_PAIN_1	= ID_PROGRESS_PANE,
		ID_PAIN_2	= ID_COMBOBOX_PANE,
	};
	DWORD		dwSzPain1 = 125;
	DWORD		dwSzPain2 = 125;
	{
		DWORD		dwVal	  = 0;
		CIniFileI	pr( g_szIniFileName, _T("StatusBar") );

		if (pr.QueryValue( dwVal, _T("SizePain") ) == ERROR_SUCCESS) {
			dwSzPain1 = LOWORD(dwVal);
			dwSzPain2 = HIWORD(dwVal);
		}

		if (pr.QueryValue( dwVal, _T("SwapPain") ) == ERROR_SUCCESS)
			g_bSwapProxy = dwVal != 0;
	}

	//+++ �ʒu�����̏C��.
	if (g_bSwapProxy == FALSE) {
		g_dwProgressPainWidth = dwSzPain1;					//+++ �蔲���Ńv���O���X�y�C���̉������O���[�o���ϐ��ɍT����.
		m_wndStatusBar.SetPaneWidth(ID_PAIN_1, 0);			//dwSzPain1); //�N�����̓y�C���T�C�Y��0
		if (m_wndStatusBar.GetProxyComboBox().IsUseIE())
			dwSzPain2 = 0;									// IE��Proxy���g���ꍇ��Proxy�y�C���T�C�Y��0��
		m_wndStatusBar.SetPaneWidth(ID_PAIN_2, dwSzPain2);
	} else {	// �������Ă���Ƃ�.
		g_dwProgressPainWidth = dwSzPain2;					//+++ �蔲���Ńv���O���X�y�C���̉������O���[�o���ϐ��ɍT����.
		m_wndStatusBar.SetPaneWidth(ID_PAIN_2, dwSzPain2);	//+++ �������Ă�Ƃ��́A�ŏ�����T�C�Y�m��.
		if (m_wndStatusBar.GetProxyComboBox().IsUseIE())
			dwSzPain1 = 0;									// IE��Proxy���g���ꍇ��Proxy�y�C���T�C�Y��0��
		m_wndStatusBar.SetPaneWidth(ID_PAIN_1, dwSzPain1);
	}

	m_wndStatusBar.SetPaneWidth(ID_SECURE_PANE	, 25);
	m_wndStatusBar.SetPaneWidth(ID_PRIVACY_PANE , 25);
}

//---------------------------------------------
/// Plugin Toolbar Load
void CMainFrame::init_pluginManager()
{
	CPluginManager::Init();	//\\����Ȃ����ǂƂ肠�����ǂ�ǂ�
	CPluginManager::ReadPluginData(PLT_TOOLBAR, m_hWnd);
	CPluginManager::LoadAllPlugin(PLT_TOOLBAR, m_hWnd, true);	//�c�[���o�[�v���O�C����S�����[�h

	{
		int nCount = CPluginManager::GetCount(PLT_TOOLBAR);
		for (int i = 0; i < nCount; i++) {
			HWND hWnd = CPluginManager::GetHWND(PLT_TOOLBAR, i);

			if ( ::IsWindow(hWnd) )
				::SetProp( hWnd, _T("Donut_Plugin_ID"), HANDLE( IDC_PLUGIN_TOOLBAR + i) );
		}
	}
	//LoadPluginToolbars();
}

//-------------------------------------
/// load band position
void CMainFrame::init_band_position(
	HWND 	hWndCmdBar,
	HWND	hWndToolBar,
	HWND	hWndAddressBar,
	HWND	hWndMDITab,
	HWND	hWndLinkBar,
	HWND	hWndSearchBar  )
{
	CSimpleArray<HWND> aryHWnd;
	aryHWnd.Add( hWndCmdBar 	);		// ���j���[�o�[
	aryHWnd.Add( hWndToolBar	);		// �c�[���o�[
	aryHWnd.Add( hWndAddressBar );		// �A�h���X�o�[
	aryHWnd.Add( hWndMDITab 	);		// �^�u�o�[
	aryHWnd.Add( hWndLinkBar	);		// �����N�o�[
	aryHWnd.Add( hWndSearchBar	);		// �����o�[

	int nToolbarPluginCount = CPluginManager::GetCount(PLT_TOOLBAR);
	CReBarBandInfo*   pRbis	= new CReBarBandInfo[STDBAR_COUNT + nToolbarPluginCount];

	for (int nIndex = 0; nIndex < aryHWnd.GetSize(); nIndex++) {
		pRbis[nIndex].nIndex	= nIndex;
		pRbis[nIndex].hWnd		= aryHWnd	  [ nIndex ];
		pRbis[nIndex].nID		= STDBAR_ID   [ nIndex ];
		pRbis[nIndex].fStyle	= STDBAR_STYLE[ nIndex ];
		pRbis[nIndex].lpText	= STDBAR_TEXT [ nIndex ];
		pRbis[nIndex].cx		= 0;
	}

	for (int nIndex = 0; nIndex < nToolbarPluginCount; nIndex++) {
		pRbis[STDBAR_COUNT + nIndex].nIndex   = STDBAR_COUNT + nIndex;
		pRbis[STDBAR_COUNT + nIndex].hWnd	  = CPluginManager::GetHWND(PLT_TOOLBAR, nIndex);
		pRbis[STDBAR_COUNT + nIndex].nID	  = IDC_PLUGIN_TOOLBAR + nIndex;
		pRbis[STDBAR_COUNT + nIndex].fStyle   = RBBS_BREAK;
		pRbis[STDBAR_COUNT + nIndex].lpText   = NULL;
		pRbis[STDBAR_COUNT + nIndex].cx 	  = 0;
	}

	{
		CIniFileI pr( g_szIniFileName, _T("ReBar") );
		MtlGetProfileReBarBandsState( pRbis, pRbis + STDBAR_COUNT + nToolbarPluginCount, pr, *this);
	}

	delete[] pRbis;

	m_CmdBar.RefreshBandIdealSize(m_hWndToolBar);
	m_AddressBar.InitReBarBandInfo(m_hWndToolBar);	// if you dislike a header, remove this.
	ShowLinkText(CAddressBarOption::s_bTextVisible);
}

//-------------------------------------
/// load status bar state
void CMainFrame::init_loadStatusBarState()
{
	CIniFileI pr( g_szIniFileName, _T("Main") );
	BOOL	bStatusBarVisible = TRUE;
	MtlGetProfileStatusBarState(pr, m_hWndStatusBar, bStatusBarVisible);
}

//-------------------------------------
/// create mdi client window
void CMainFrame::init_mdiClientWindow()
{
	m_menuWindow	= ::GetSubMenu(m_hMenu, _nPosWindowMenu);
	CreateMDIClient(m_menuWindow.m_hMenu);
	m_wndMDIClient.SubclassWindow(m_hWndMDIClient);

	// NOTE: If WS_CLIPCHILDREN not set, MDI Client will try erase background over MDI child window,
	//		 but as OnEraseBkgnd does nothing, it goes well.
	if (CMainOption::s_bTabMode)
		m_wndMDIClient.ModifyStyle(WS_CLIPCHILDREN, 0); 		// to avoid flicker, but scary
}

//-------------------------------------
/// splitter window
void CMainFrame::init_splitterWindow()
{
	m_hWndClient	= m_wndSplit.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	ATLASSERT( ::IsWindow(m_hWndClient) );
	m_wndSplit.SetSplitterExtendedStyle(SPLIT_GRADIENTBAR/*0*/);
}

//------------------------------------
/// pane container
void CMainFrame::init_explorerBar()
{
	m_ExplorerBar.Create(m_hWndClient);
	m_ExplorerBar.Init(m_hWndMDIClient);
}

//------------------------------------
/// MDIClient misc
void CMainFrame::init_mdiClient_misc(HWND hWndCmdBar, HWND hWndToolBar)
{
	m_mcCmdBar.InstallAsMDICmdBar(hWndCmdBar, m_hWndMDIClient, CMainOption::s_bTabMode);
	m_mcToolBar.InstallAsStandard(hWndToolBar, m_hWnd, true, ID_VIEW_FULLSCREEN);

	m_mcCmdBar.ShowButton(!CMenuOption::s_bDontShowButton);	// ���j���[�̕���{�^����\�����Ȃ�
	m_MDITab.SetMDIClient(m_hWndMDIClient);
}

//-----------------------------------
/// ���b�Z�[�W�t�B���^�ƃA�C�h���n���h����o�^
void CMainFrame::init_message_loop()
{
	// message loop
	CMessageLoop *pLoop 	 = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);
}


//--------------------------------------
/// �V�X�e�����j���[�Ɂu���j���[��\��]��ǉ�
void CMainFrame::init_sysMenu()
{
	HMENU		hSysMenu = ::GetSystemMenu(m_hWnd, FALSE);
	//		::AppendMenu(hSysMenu, MF_ENABLED|MF_STRING, ID_VIEW_COMMANDBAR, _T("���j���[��\��"));

	TCHAR		cText[]	 = _T("���j���[��\��");
	MENUITEMINFO  menuInfo = { sizeof(MENUITEMINFO) };
	menuInfo.fMask		= MIIM_ID | MIIM_TYPE;
	menuInfo.fType		= MFT_STRING;
	menuInfo.wID		= ID_VIEW_COMMANDBAR;
	menuInfo.dwTypeData = cText;
	menuInfo.cch		= sizeof (cText);
	::InsertMenuItem(hSysMenu, 0, MF_BYPOSITION, &menuInfo);
}

//--------------------------------------
/// �v���O�C����ǂݍ���
void CMainFrame::init_loadPlugins()
{
	// �G�N�X�v���[���[�o�[�v���O�C���ǂݍ���
	CPluginManager::LoadAllPlugin(PLT_EXPLORERBAR, m_ExplorerBar.m_PluginBar);

	//�I�y���[�V�����v���O�C���̃��[�h
	CPluginManager::ReadPluginData(PLT_OPERATION);

  #if 1	//+++ ������� DockingBar�v���O�C�����AExplorerBar�Ƃ��Ĉ����Ă݂�...
	CPluginManager::LoadAllPlugin(PLT_DOCKINGBAR, m_ExplorerBar.m_PluginBar);
  #else
	CPluginManager::LoadAllPlugin(PLT_DOCKINGBAR, m_hWnd);
  #endif
}



// ===========================================================================
// �I������
#if 1
CMainFrame::~CMainFrame()
{
	GdiplusTerm();
}

//---------------------------------------
/// �V�X�e�����I�����鎞
void CMainFrame::OnEndSession(BOOL wParam, UINT lParam) 						//�@ShutDown minit
{
	if (wParam == TRUE)
		OnDestroy();
}

//---------------------------------------
/// �E�B���h�E�I����
void CMainFrame::OnDestroy()
{
	SetMsgHandled(FALSE);

	_PrivateTerm();		// �ݒ�̕ۑ�

	//\\MtlSendCommand(m_hWnd, ID_VIEW_STOP_ALL);									// added by DOGSTORE

	//CSearchBoxHook::UninstallSearchHook();
	CDialogHook::UninstallHook();
#ifdef _DEBUG
	//�f�o�b�O�E�B���h�E�폜
	m_wndDebug.Destroy();
#endif
	//�S�v���O�C�����
	CPluginManager::Term();

	m_wndMDIClient.UnsubclassWindow();
	m_ReBar.UnsubclassWindow();

	//�o�b�N�A�b�v�X���b�h�̊J��
	if (m_hGroupThread) {
		DWORD dwResult = ::WaitForSingleObject(m_hGroupThread, DONUT_BACKUP_THREADWAIT);

		if (dwResult == WAIT_TIMEOUT) {
			ErrorLogPrintf(_T("MainFrame::OnDestroy�ɂāA�����X�V�X���b�h�̏I�����o���Ȃ��͗l\n"));
			::TerminateThread(m_hGroupThread, 1);
		}

		CloseHandle(m_hGroupThread);
		m_hGroupThread = NULL;			//+++ �Ō�Ƃ͂����A�ꉞ NULL���Ƃ�.
	}	

	//\\?CCriticalIdleTimer::UninstallCriticalIdleTimer();

	// what can I trust?
	ATLASSERT( ::IsMenu(m_hMenu) );
	::DestroyMenu(m_hMenu);

	_RemoveShortcutTmpDirectory();

	OnMenuRefreshScript(FALSE);

	RevokeDragDrop();

	// ���b�Z�[�W���[�v���烁�b�Z�[�W�t�B���^�ƃA�C�h���n���h�����폜
    CMessageLoop* pLoop = _Module.GetMessageLoop();
    pLoop->RemoveMessageFilter(this);
    pLoop->RemoveIdleHandler(this);

	ATLTRACE(_T("���C���t���[���̏I����...\n"));
#if 0
	CString strPath = Misc::GetExeDirectory() + _T("lock");
	HANDLE hHandle = ::CreateFile(strPath, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hHandle != INVALID_HANDLE_VALUE) {
		::CloseHandle(hHandle);
	} else {
		ATLASSERT(FALSE);
	}
#endif
}

//------------------------------------
/// ShortcutTmp�t�H���_��".tmp"�t�@�C�����폜���ăt�H���_���폜����
void CMainFrame::_RemoveShortcutTmpDirectory()
{
	CString 	strTempPath = Misc::GetExeDirectory() + _T("ShortcutTmp\\");
	if (::PathIsDirectory(strTempPath)) {
		MtlForEachFile( strTempPath, [](const CString& strFile) {
			if ( MtlIsExt( strFile, _T(".tmp") ) )
				::DeleteFile(strFile);
		});
		::RemoveDirectory(strTempPath);
	}
}


//----------------------------------
/// ���C���t���[���̂w�{�^�����������Ƃ�
void CMainFrame::OnClose()
{
	int nDLCount = m_DownloadManager.GetDownloadingCount();
	if (nDLCount > 0) {
		CString msg;
		msg.Format(_T("�_�E�����[�h���̃A�C�e���� %d����܂��B\n�I�����܂����H"), nDLCount);
		if (MessageBox(msg, _T("�m�F"), MB_ICONQUESTION | MB_OKCANCEL | MB_DEFBUTTON2) == IDCANCEL)
			return;
	}
	SetMsgHandled(FALSE);

	if ( !CDonutConfirmOption::OnDonutExit(m_hWnd) ) {
		SetMsgHandled(TRUE);
		if (IsWindowVisible() == FALSE) {
			SetHideTrayIcon();
			Sleep(100);
		}
		return;
	}

	// �^�C�}�[���Ƃ߂�.
	if (m_nBackUpTimerID != 0) {
		KillTimer(m_nBackUpTimerID);
		m_nBackUpTimerID	= 0;			//+++ �ꉞ 0�N���A���Ƃ�.
	}

  #if 1 //+++ _WriteProfile()����o�b�N�A�b�v�̏�����������Ɉڂ�(��ɍs��) ... //����ς���
	//_SaveGroupOption( CStartUpOption::GetDefaultDFGFilePath() );
	SaveAllTab();
  #endif

	CChildFrame::SetMainframeCloseFlag();	//+++ mainfrm��close���邱�Ƃ�ChildFrame�ɋ�����(�i�r���b�N��close�s����߂邽��)


	if ( IsFullScreen() ) {
		CMainOption::s_dwMainExtendedStyle |= MAIN_EX_FULLSCREEN;	// save style
		_ShowBandsFullScreen(FALSE);								// restore bands position
	} else {
		CMainOption::s_dwMainExtendedStyle &= ~MAIN_EX_FULLSCREEN;
	}

	m_mcCmdBar.Uninstall();
	m_mcToolBar.Uninstall();

	_WriteProfile();

	CMainOption::s_bAppClosing = true;

	// delete cash
	DelTempFiles();
	// delete history
	DelHistory();
}


//------------------------------------
/// �L���b�V��(html�Ȃ�)���폜����
void CMainFrame::DelTempFiles()
{
	bool						bDelCash	 = (CMainOption::s_dwMainExtendedStyle2 & MAIN_EX2_DEL_CASH  ) != 0;	//+++ ? TRUE : FALSE;
	bool						bDelCookie	 = (CMainOption::s_dwMainExtendedStyle2 & MAIN_EX2_DEL_COOKIE) != 0;	//+++ ? TRUE : FALSE;

	if (bDelCash == FALSE && bDelCookie == FALSE)
		return;

	bool						bDone		 = FALSE;
	LPINTERNET_CACHE_ENTRY_INFO lpCacheEntry = NULL;

	DWORD						dwTrySize;
	DWORD						dwEntrySize  = 4096;				// start buffer size
	HANDLE						hCacheDir	 = NULL;
	DWORD						dwError 	 = ERROR_INSUFFICIENT_BUFFER;

	do {
		switch (dwError) {
			// need a bigger buffer
		case ERROR_INSUFFICIENT_BUFFER:
			delete[] lpCacheEntry;
			lpCacheEntry			   = (LPINTERNET_CACHE_ENTRY_INFO) new char[dwEntrySize];
			lpCacheEntry->dwStructSize = dwEntrySize;
			dwTrySize				   = dwEntrySize;
			BOOL bSuccess;
			if (hCacheDir == NULL)
				bSuccess = ( hCacheDir = FindFirstUrlCacheEntry(NULL, lpCacheEntry, &dwTrySize) ) != NULL;
			else
				bSuccess = FindNextUrlCacheEntry(hCacheDir, lpCacheEntry, &dwTrySize) != 0;

			if (bSuccess)
				dwError = ERROR_SUCCESS;
			else {
				dwError 	= GetLastError();
				dwEntrySize = dwTrySize;							// use new size returned
			}
			break;

			// we are done
		case ERROR_NO_MORE_ITEMS:
			bDone					   = TRUE;
			//x bResult 			   = TRUE;						//+++ �������ǖ��g�p�̂悤.
			break;

			// we have got an entry
		case ERROR_SUCCESS:
			if (bDelCookie && lpCacheEntry->CacheEntryType & COOKIE_CACHE_ENTRY)
				DeleteUrlCacheEntry(lpCacheEntry->lpszSourceUrlName);

			// Fixed by zzZ(thx
			if ( bDelCash && !( lpCacheEntry->CacheEntryType & (COOKIE_CACHE_ENTRY | URLHISTORY_CACHE_ENTRY) ) )
				DeleteUrlCacheEntry(lpCacheEntry->lpszSourceUrlName);

			// get ready for next entry
			dwTrySize				   = dwEntrySize;

			if ( FindNextUrlCacheEntry(hCacheDir, lpCacheEntry, &dwTrySize) )
				dwError = ERROR_SUCCESS;
			else {
				dwError 	= GetLastError();
				dwEntrySize = dwTrySize;							// use new size returned
			}
			break;

			// unknown error
		default:
			bDone	= TRUE;
			break;
		}

		if (bDone) {
			delete[] lpCacheEntry;

			if (hCacheDir)
				FindCloseUrlCache(hCacheDir);
		}
	} while (!bDone);
}

//-------------------------------
/// �������폜����
void CMainFrame::DelHistory()
{
	bool	bDelHistory  = (CMainOption::s_dwMainExtendedStyle2 & MAIN_EX2_DEL_HISTORY) != 0;	//+++ ? TRUE : FALSE;
	if (bDelHistory == false)
		return;

	CComPtr<IUrlHistoryStg2>	spUrlHistoryStg2;
	HRESULT hr = spUrlHistoryStg2.CoCreateInstance(CLSID_CUrlHistory);
	if ( SUCCEEDED(hr) )
		hr = spUrlHistoryStg2->ClearHistory();
}
#endif	// term

// ===========================================================================
// MainFrame_PreTrnMsg����


#ifndef WM_XBUTTONDOWN
 #define WM_XBUTTONDOWN 				0x020B
 #define WM_XBUTTONUP					0x020C
 #define GET_KEYSTATE_WPARAM(wParam)	( LOWORD(wParam) )
 #define GET_XBUTTON_WPARAM(wParam) 	( HIWORD(wParam) )
 #define MK_XBUTTON1					0x0020
 #define MK_XBUTTON2					0x0040
 #define XBUTTON1						0x0001
 #define XBUTTON2						0x0002
#endif



//�e�E�B���h�E��(��ɃL�[)���b�Z�[�W��]������
BOOL CMainFrame::PreTranslateMessage(MSG *pMsg)
{
	m_bWM_TIMER = pMsg->message == WM_TIMER;
	if (m_bWM_TIMER) {
		if (pMsg->hwnd == NULL && pMsg->lParam == 0)
			return FALSE;
		return FALSE;
	}

	//�R�}���h�o�[(���j���[)
	if ( m_CmdBar.PreTranslateMessage(pMsg) )
		return TRUE;

	//�A�h���X�o�[
	BOOL ptFlag = m_AddressBar.PreTranslateMessage(pMsg);
	if (ptFlag == _MTL_TRANSLATE_HANDLE)
		return TRUE;
	else if (ptFlag == _MTL_TRANSLATE_WANT)
		return FALSE;

	//�����o�[
	ptFlag = m_SearchBar.PreTranslateMessage(pMsg);
	if (ptFlag == _MTL_TRANSLATE_HANDLE)
		return TRUE;
	else if (ptFlag == _MTL_TRANSLATE_WANT)
		return FALSE;

	//�G�N�X�v���[���o�[
	//if (m_ExplorerBar.PreTranslateMessage(pMsg)) return TRUE;
	ptFlag = m_ExplorerBar.PreTranslateMessage(pMsg);
	if (ptFlag == _MTL_TRANSLATE_HANDLE)
		return TRUE;
	else if (ptFlag == _MTL_TRANSLATE_WANT)
		return FALSE;

  #if 1
	//+++ �A�h���X�o�[or�T�[�`�o�[�Ƀt�H�[�J�X���������Ă��鎞�A���̏����ŗ]�v�Ȃ���(�L�[����)�����Ȃ��悤�ɃK�[�h���Ă݂�.
	int	bFocus = false;
	if (m_AddressBar.IsWindow())
		bFocus	|= (::GetFocus() == m_AddressBar.GetEditCtrl().m_hWnd);
	if (::IsWindow(m_SearchBar.GetEditCtrl()/*m_SearchBar.m_hWnd*/))
		bFocus	|= (::GetFocus() == m_SearchBar.GetEditCtrl().m_hWnd);
	HWND hWndFind = m_FindBar.GetHWND();
	if (::IsWindow(hWndFind))
		bFocus	|= ::IsChild(hWndFind, pMsg->hwnd);
#ifdef _DEBUG
	if (::IsWindow(m_wndDebug.m_hWnd)) {
		bFocus |= (::GetFocus() == m_wndDebug.GetEditCtrl().m_hWnd);
	}
#endif
  #endif

	// ���N���X
	if (/*bFocus == 0 &&*/ baseClass::PreTranslateMessage(pMsg) )
		return TRUE;

	// �A�N�e�B�u�E�`���C���h�E�E�B���h�E
	HWND hWnd = 0;
	if (m_hWndMDIClient && ::IsWindow(m_hWndMDIClient))
		hWnd = MDIGetActive();

  #if 0	//+++ ���{�^���N���b�N�̃e�X�g
	if ( pMsg->message == WM_LBUTTONDOWN) {
		printf("%d,%d\n",pMsg->wParam, pMsg->lParam);
	}
  #endif

	// ���{�^���N���b�N�̃`�F�b�N
	if ( pMsg->message == WM_MBUTTONDOWN && hWnd != NULL && ::IsChild(hWnd, pMsg->hwnd) && OnMButtonHook(pMsg) )
		return TRUE;

	// �E�h���b�O�L�����Z���p
	if ( pMsg->message == WM_RBUTTONUP && m_bCancelRButtonUp ) {
		m_bCancelRButtonUp = false;
		return TRUE;
	}

	// �}�E�X�W�F�X�`���[��
	if ( pMsg->message == WM_RBUTTONDOWN && OnRButtonHook(pMsg) )
		return TRUE;

	// �T�C�h�{�^��
	if (pMsg->message == WM_XBUTTONUP) {
		if ( OnXButtonUp( GET_KEYSTATE_WPARAM(pMsg->wParam), GET_XBUTTON_WPARAM(pMsg->wParam),
						 CPoint( GET_X_LPARAM(pMsg->lParam), GET_Y_LPARAM(pMsg->lParam) ) ) )
			return TRUE;
	}

	// �G�N�X�v���[���o�[�������ŕ\��
	if ( (CMainOption::s_dwExplorerBarStyle & MAIN_EXPLORER_AUTOSHOW) == MAIN_EXPLORER_AUTOSHOW )
		ExplorerBarAutoShow(pMsg);

	// BHO �v���O�C����
	if (bFocus == 0 && TranslateMessageToBHO(pMsg) )											//+++ bFocus�`�F�b�N
		return TRUE;

	if (bFocus == 0 && hWnd != NULL && ::SendMessage(hWnd, WM_FORWARDMSG, 0, (LPARAM) pMsg) )	//+++ bFocus�`�F�b�N
		return TRUE;

	return FALSE; // IsDialogMessage(pMsg);
}

//---------------------------------------------
/// �R���g���[���̔z�u�X�V
void CMainFrame::UpdateLayout(BOOL bResizeBars /*= TRUE*/)
{
	CRect	  rc;
	GetClientRect(&rc);

	if (bResizeBars) {
		CReBarCtrl rebar(m_hWndToolBar);
		if (rebar.m_hWnd == NULL) 
			goto END;

		CRect	   rcSrc;
		rebar.GetClientRect(&rcSrc);

		CRect	   rcNew(0, 0, rc.right, rcSrc.Height());
		rebar.MoveWindow(rcNew);
		rebar.RedrawWindow();
	}
END:
	UpdateBarsPosition(rc, bResizeBars);
	if (m_bFullScreen == false)
		rc.top++;

	HWND hWndFind = m_FindBar.GetHWND();
	if (::IsWindowVisible(hWndFind)) {
		CRect rcFind;
		::GetClientRect(hWndFind, &rcFind);
		::SetWindowPos( hWndFind, NULL, rc.left, rc.top, rc.right, rcFind.bottom, SWP_NOZORDER | SWP_NOACTIVATE );
		rc.top += rcFind.bottom;
	}
	if (m_hWndClient) {
		::SetWindowPos( m_hWndClient, NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER | SWP_NOACTIVATE );
	}
}



BOOL CMainFrame::OnXButtonUp(WORD wKeys, WORD wButton, CPoint point)
{
	CString 	strKey;

	switch (wButton) {
	case XBUTTON1: strKey = _T("Side1"); break;
	case XBUTTON2: strKey = _T("Side2"); break;
	}

	DWORD		dwCommand = 0;
	{
		CIniFileI pr( _GetFilePath( _T("MouseEdit.ini") ), _T("MouseCtrl") );
		pr.QueryValue(dwCommand, strKey);
	}

	if (dwCommand == 0)
		return FALSE;

	::SendMessage(m_hWnd, WM_COMMAND, dwCommand, 0);
	return TRUE;
}

//----------------------------------------
/// �J�[�\�����E�B���h�E�̍��[�ɂ���ƃG�N�X�v���[���[�o�[��\������
void CMainFrame::ExplorerBarAutoShow(MSG *pMsg)
{
	if (pMsg->message != WM_MOUSEMOVE)
		return;

	if (pMsg->wParam != 0)
		return;

	if (  m_ExplorerBar.IsFavBarVisible()		== false
	   && m_ExplorerBar.IsClipboardBarVisible() == false
	   && m_ExplorerBar.IsPanelBarVisible() 	== false
	   && m_ExplorerBar.IsPluginBarVisible()	== false)
		return;

	CPoint point;
	::GetCursorPos(&point);

	CRect  rcWndCmd;
	::GetWindowRect(m_hWndToolBar, &rcWndCmd);

	if ( rcWndCmd.PtInRect(point) )
		return;

	ScreenToClient(&point);

	CRect  rcWnd;
	GetClientRect(&rcWnd);

	if (m_ExplorerBar.IsExplorerBarHidden() == false) {
		CRect rcWndExp;
		m_ExplorerBar.GetClientRect(&rcWndExp);
		rcWnd.left += rcWndExp.Width() + 5;

		if ( rcWnd.PtInRect(point) )
			m_wndSplit.SetSinglePaneMode(SPLIT_PANE_RIGHT);
	} else {
		if (point.x < 10 && point.x >= 0) {
			m_ExplorerBar.ShowBar(m_ExplorerBar.m_dwExStyle, true);
		}
	}
}

//-----------------------------------------
/// �����N����~�h���N���b�N�ŐV�K�^�u���J��
BOOL CMainFrame::OnMButtonHook(MSG* pMsg)
{
	DWORD	dwLinkOpenBtnM;
	{
		CIniFileI	 pr( _GetFilePath( _T("MouseEdit.ini") ), _T("MouseCtrl") );
		dwLinkOpenBtnM = pr.GetValue(_T("LinkOpenBtnM"), 0);
	}

	// �{�^�����ݒ肳��Ă��Ȃ�������A�I���.
	if (dwLinkOpenBtnM == 0)
		return FALSE;
	//+++ �����F�f�t�H���g�ł� dwLinkOpenBtmM = IDM_FOLLOWLINKN
	// ATLASSERT(dwLinkOpenBtnM == IDM_FOLLOWLINKN);

  #if 0
	//+++ ����:���̕����̏����́ADonutP��DonutRAPT,unDonut�ł��قǃ\�[�X�ɈႢ������킯�łȂ�����.
	::SendMessage(pMsg->hwnd, WM_LBUTTONDOWN, 0, pMsg->lParam); 	//+++ ���܂��Ȃ�.
	int 	nTabCnt = m_MDITab.GetItemCount();						//+++ �����N���N���b�N�������ǂ����̃`�F�b�N�p.
	::SendMessage(pMsg->hwnd, WM_COMMAND, dwLinkOpenBtnM, 0);		//+++ �����N��V�������ɂЂ炭
	if (m_MDITab.GetItemCount() != nTabCnt) 						//+++ �^�u�������Ă�����A�����N���N���b�N�����̂��낤
		return TRUE;												//+++ true��Ԃ��Ē��{�^���N���b�N�̏�������������Ƃɂ���.
	::SendMessage(pMsg->hwnd, WM_LBUTTONUP, 0, pMsg->lParam);		//+++ �����N�ȊO���N���b�N�����ꍇ���܂��Ȃ��̍��N���b�N�������I�����Ă���.
	return FALSE;													//+++ false��Ԃ����ƂŁAIE�R���|�[�l���g�ɃE�B�[���N���b�N�̏�����C����.

	//+++ ����: �����炭��L�̉E�ɏ������R�����g�̂悤�ȓ����z�肵�����̂��Ǝv����.
	//		"���܂��Ȃ�"�A�́AWM_COMMAND+IDM_FOLLOWLINKN�݂̂𑗂����ꍇ�A
	//		�Ȃ����J��������1�N���b�N�����悤�ŁA����̒��날�킹�Ƃ���
	//		���N���b�N�����������Ƃɂ��Ă���悤��(...�悭�킩��Ȃ�����ǁA���Ԃ�).
	//		�����A�����N�ȊO�̉ӏ����N���b�N�����Ƃ��́A�͈͑I���̊J�n�Ƃ��ċ@�\���Ă��܂����߁A
	//		���̏ꍇ�́A���U���{�^���N���b�N���I�������鏈�������Ă���B
	//
	//		���́A�N���b�N�������ǂ����̔���ŁA�^�u�̌���IDM_FOLLOWLINKN�ɂ����
	//		������(�ϓ�����)���ǂ��������Ă�̂���...
	//		�~�j�b�g���ȑO�̊J���҂̊��ł͗L���̂悤�����Aundonut+��+mod�̊J�����ł́A
	//		�����N�̗L���ɂ�����炸���̎��_�ł̓^�u�����ς��Ȃ��͗l... ��
	//		FALSE��Ԃ������ɗ���A���N���b�N�L�����Z���̏��������J�N���b�N�Ɠ��l�ɍ�p����
	//		���܂��A���ʓI�ɓ����y�[�W���Q�J���Ă��܂��A�Ƃ����o�O�ɉ����Ă��܂��Ă���.
	//
	//		�󋵓I�ɂ�::SendMessage(pMsg->hwnd, WM_COMMAND, dwLinkOpenBtnM, 0);�̎��s��
	//		�V�����^�u���ł��Ă��畜�A���Ă����m���A���N�G�X�g�������s���Ă����߂��Ă��鏈��
	//		�ɕς���Ă��܂����A�Ƃ������ƂɎv����B
	//
	//		���������CChildFrame���ł� �����o�[��ActivateFrame()�̏����̒��Ń^�u�ւ̒ǉ���
	//		��������̂����AWM_COMMAND+dwLinkOpenBtnM��CChildFrame�������ꍇ�A
	//		CChildFrame::OnNewWindow2() ���Ă΂�A���̃��[�g�ł�ActivateFrame()�͒��ɂ͌Ă΂ꂸ�A
	//		���Ƃ�ActivateFrame()���Ă΂��悤�ɂ��Ă���B���̏�Ԃ�OnNewWindow2�͏I���̂ŁA
	//		SendMessage���I����Ă��^�u�̐��͕ς��Ȃ��A�Ƃ������̏�Ԃ́A
	//		�\�[�X�I�ɂ͓��R�Ɏv����̂���... (���̂ւ�̑�؂�donutP�ɂ����̂ڂ��Ă�����)
	//
	//		�ŁAActivateFrame�����ƂŒN���ĂԂ��Ƃ����� CChildFrame::OnBeforeNavigate2() �ŁA
	//		����� MtlBrawser.h��IWebBrowserEvents2Impl::OnBeforeNavigate2����Ă΂�Ă���
	//		�܂�A���܂������Ă���J�����ł́ADonut�\�[�X���Ŗ����I�ɂ��̊֐����Ă�ł��Ȃ�����ǁA
	//		SendMessage���I������܂łɂ�������s�����A�Ƃ������ƂȂ̂��낤.
	//		(�� �Ђ���Ƃ���ƁA�������� OnNewWindow2�ȊO��ChildFrame����Ă����\�������邩��������)
	//
	//		OS,IE,PSDK/ATL,WTL, unDonut �̂ǂ����̏����������Ȃ����̂��낤���A�����悭�킩��Ȃ�
	//		(2007-11-23)
	//		�ǂ���� AtlHost(Ex)�ł�CreateControlLicEx ����,Navigate2()���܂�else�����c���Ă���̂�
	//		�܂��������͗l�B����else���폜�����璼��. (2008-01-01)
  #else //+++ �����N�����������ǂ����̔���ɃX�e�[�^�X�o�[�Ƀ����N�悪�ݒ肳��Ă��邩�ǂ����A��p����悤�ɂ���.
	HWND			hWnd	= MDIGetActive();
	if ( ::IsWindow(hWnd) == 0 )
		return FALSE;
	CChildFrame*	pChild	= GetChildFrame(hWnd);
	bool			bLink	= 0;
	if (pChild) {																//+++ �J�[�\���������N�������Ă�����statusBar�p�̃��b�Z�[�W�����邱�Ƃ𗘗p.
		const CString& str = pChild->strStatusBar();
		if (   str.IsEmpty() == FALSE
			&& (str.Compare(_T("�y�[�W���\������܂���")) != 0 && str.Compare(_T("����")) != 0))			//+++ �����N�̂Ȃ��y�[�W�ł�"�y�[�W���\������܂���"�Ƃ������b�Z�[�W���ݒ肳��Ă���̂ŏ��O.
			bLink = true;

		::SendMessage(pMsg->hwnd, WM_LBUTTONDOWN, 0, pMsg->lParam); 				//+++ ���܂��Ȃ�.
		int 	nTabCnt = m_MDITab.GetItemCount();									//+++ �����N���N���b�N�������ǂ����̃`�F�b�N�p.
		pChild->m_bAllowNewWindow = true;
		::SendMessage(pMsg->hwnd, WM_COMMAND, dwLinkOpenBtnM, 0);					//+++ �����N��V�������ɂЂ炭
		pChild->m_bAllowNewWindow = false;
		int 	nTabCnt2 = m_MDITab.GetItemCount();
		if (nTabCnt != nTabCnt2 || bLink)											//+++ �����N���b�Z�[�W�����邩�A�^�u�������Ă�����A�����N���N���b�N�����Ƃ���.
			return TRUE;															//+++ true��Ԃ��Ē��{�^���N���b�N�̏�������������Ƃɂ���.

		::SendMessage(pMsg->hwnd, WM_LBUTTONUP, 0, pMsg->lParam);					//+++ �����N�ȊO���N���b�N�����ꍇ���܂��Ȃ��̍��N���b�N�������I�����Ă���.
		return FALSE;																//+++ false��Ԃ����ƂŁAIE�R���|�[�l���g�ɃE�B�[���N���b�N�̏�����C����.

	}
	return FALSE;
  #endif
}

//----------------------------------------------
/// Mouse Gesture
BOOL CMainFrame::OnRButtonHook(MSG *pMsg)
{
	HWND		hChildWnd = MDIGetActive();

	if (hChildWnd) {
		BOOL	bUsedMouseGesture = ::SendMessage(hChildWnd, WM_USER_USED_MOUSE_GESTURE, 0, 0) != 0;
		if (bUsedMouseGesture == FALSE)
			return FALSE;
	}

	::SetCapture(m_hWnd);

	CPoint		ptDown;
	ptDown.x = GET_X_LPARAM(pMsg->lParam);
	ptDown.y = GET_Y_LPARAM(pMsg->lParam);
	::ClientToScreen(pMsg->hwnd, &ptDown);

	CPoint		ptBefor   = ptDown;
	CPoint		ptUp	  = ptDown;
	int 		nAng1	  = -1;
	CString 	strMove;
	CString 	strMark1;
	CString 	strMark2;
	DWORD		dwTime	  = -1;
	HWND		hWndHist  = m_hWnd;
	HWND		hWnd	  = NULL;
	BOOL		bNoting   = TRUE;
	MSG 		msg		  = { 0 };
	BOOL		bInner	  = CMouseOption::s_bUseRightDragSearch && _IsSelectedTextInner();
	HMODULE		hModule	  = ::LoadLibrary(_T("ole32.dll"));
	ATLASSERT(hModule);
	CString		strSearchEngine;
	int 		nDiff	  = 0;

	do {
		if (::GetCapture() != hWndHist)
			break;

		if (::GetMessage(&msg, NULL, 0, 0) == FALSE)
			continue;

		
		DWORD	dwCommand	= 0;
		BOOL	bNeedFresh	= FALSE,
				bBreak		= FALSE;

		switch (msg.message) {
		case WM_MOUSEWHEEL:
			dwCommand = GetMouseWheelCmd( (short) HIWORD(msg.wParam) );
			break;

		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
			dwCommand = GetMouseButtonUpCmd(msg.message);
			break;

		case WM_XBUTTONUP:
			dwCommand = GetMouseButtonUpCmd( msg.message, GET_XBUTTON_WPARAM(msg.wParam) );
			break;

		case WM_MOUSEMOVE:
			if (bInner) {
				ptUp.x	  = GET_X_LPARAM(msg.lParam);
				ptUp.y	  = GET_Y_LPARAM(msg.lParam);
				::ClientToScreen(msg.hwnd, &ptUp);

				if (nDiff < 10) {
					// ���������߂�
					nDiff	  = (int) ( sqrt( pow(float (ptBefor.x - ptUp.x), 2.0f) + pow(float (ptBefor.y - ptUp.y), 2.0f) ) );
					if (nDiff < 10)
						break;
				}
				// �J�[�\����ύX
				::SetCursor(::LoadCursor(hModule, MAKEINTRESOURCE(3)));

				CString strMark;
				if (CMouseOption::s_bUseRect) {
					// �p�x�����߂�
					int nAng  = (int) _GetAngle(ptBefor, ptUp);
					if		(nAng <  45 || nAng >  315) {
						strSearchEngine = CMouseOption::s_strREngine;	
						strMark = _T("[��] ");
					} else if (nAng >= 45 && nAng <= 135) {
						strSearchEngine = CMouseOption::s_strTEngine;	
						strMark = _T("[��] ");
					} else if (nAng > 135 && nAng <  225) {
						strSearchEngine = CMouseOption::s_strLEngine;	
						strMark = _T("[��] ");
					} else if (nAng >= 225 && nAng <= 315) {
						strSearchEngine = CMouseOption::s_strBEngine;
						strMark = _T("[��] ");
					}
				} else {
					strSearchEngine = CMouseOption::s_strCEngine;
				}
				if (strSearchEngine.IsEmpty() == FALSE) {
					CString strMsg;
					strMsg.Format(_T("���� %s: %s"), strMark, strSearchEngine);
					::SetWindowText(m_hWndStatusBar, strMsg);
				} else {
					::SetWindowText(m_hWndStatusBar, _T(""));
				}
			} else {
				ptUp.x	  = GET_X_LPARAM(msg.lParam);
				ptUp.y	  = GET_Y_LPARAM(msg.lParam);
				::ClientToScreen(msg.hwnd, &ptUp);
				// ���������߂�
				nDiff	  = (int) ( sqrt( pow(float (ptBefor.x - ptUp.x), 2.0f) + pow(float (ptBefor.y - ptUp.y), 2.0f) ) );
				if (nDiff < 10)
					break;
				// �p�x�����߂�
				nAng1	  = (int) _GetAngle(ptBefor, ptUp);
				ptBefor   = ptUp;

				if		(nAng1 <  45 || nAng1 >  315)
					strMark1 = _T("��");
				else if (nAng1 >= 45 && nAng1 <= 135)
					strMark1 = _T("��");
				else if (nAng1 > 135 && nAng1 <  225)
					strMark1 = _T("��");
				else if (nAng1 >= 225 && nAng1 <= 315)
					strMark1 = _T("��");

				// ���������ɓ������āA����300ms�ȏ�o�����Ȃ�L��
				if (strMark1 == strMark2) {
					DWORD dwTimeNow = ::GetTickCount();
					if ( (dwTimeNow - dwTime) > 300 ) {
						strMark2 = _T("");
						dwTime	 = dwTimeNow;
					}
				}

				if (strMark1 != strMark2) {
					strMove += strMark1;
					strMark2 = strMark1;

					DWORD	dwCommand = 0;
					CIniFileI	pr( _GetFilePath( _T("MouseEdit.ini") ), _T("MouseCtrl") );
					pr.QueryValue(dwCommand, strMove);
					pr.Close();
					CString strCmdName;

					if (dwCommand != 0) {
						// ���v����R�}���h������Ε\��
						CString strTemp;
						CToolTipManager::LoadToolTipText(dwCommand, strTemp);
						strCmdName.Format(_T("[ %s ]"), strTemp);
					}

					// �X�e�[�^�X�o�[�ɕ\��
					CString 	strMsg;
					strMsg.Format(_T("�W�F�X�`���[ : %s %s"), strMove, strCmdName);
					::SetWindowText(m_hWndStatusBar, strMsg);
				}

				dwTime = ::GetTickCount();
			}
			break;

		case WM_KEYDOWN:
			hWnd   = MDIGetActive();
			if (hWnd != NULL)
				SendMessage(hWnd, WM_FORWARDMSG, 0, (LPARAM) &msg);

			break;

		case WM_LBUTTONDOWN:
			if (bInner && nDiff >= 10) {	// �L�����Z������
				msg.message = WM_RBUTTONUP;
				bNoting	= FALSE;
				strSearchEngine.Empty();
				::SetWindowText(m_hWndStatusBar, _T(""));
				bInner = -1;
			}
			break;

		default:
			DispatchMessage(&msg);
			break;
		}

		switch (dwCommand) {
		case ID_FILE_CLOSE:
			hWnd	   = MDIGetActive();
			if (hWnd == NULL)
				break;

			::PostMessage(hWnd, WM_COMMAND, ID_FILE_CLOSE, 0);
			//::PostMessage(hWnd, WM_CLOSE, 0, 0);
			bNoting    = FALSE;
			bNeedFresh = TRUE;
			//goto NEXT;
			break;

		case ID_GET_OUT:				// �ޔ�
		case ID_VIEW_FULLSCREEN:		// �S�̕\��
		case ID_VIEW_UP:				// ���
		case ID_VIEW_BACK:				// �O�ɖ߂�
		case ID_VIEW_FORWARD:			// ���ɐi��
		case ID_VIEW_STOP_ALL:			// ���ׂĒ��~
		case ID_VIEW_REFRESH_ALL:		// ���ׂčX�V
		case ID_WINDOW_CLOSE_ALL:		// ���ׂĕ���
		case ID_WINDOW_CLOSE_EXCEPT:	// ����ȊO����
			::PostMessage(m_hWnd, WM_COMMAND, dwCommand, 0);
			bNoting    = FALSE;
			bBreak	   = TRUE;
			break;

		case 0:
			break;

		default:
			::PostMessage(m_hWnd, WM_COMMAND, dwCommand, 0);
			bNoting    = FALSE;
			bNeedFresh = TRUE;
			break;
		}

		if (bNeedFresh) {
			hWnd = MDIGetActive();
			if (hWnd)
				::RedrawWindow(hWnd, NULL, NULL, RDW_UPDATENOW);
		}

		if (!(GetAsyncKeyState(VK_RBUTTON) & 0x80000000) && msg.message != WM_RBUTTONUP) {
			MSG msgR;
			if (::PeekMessage(&msgR, NULL, 0, 0, PM_NOREMOVE) == 0) {
				break;
			}
		}
	} while (msg.message != WM_RBUTTONUP);

	::ReleaseCapture();
	::FreeLibrary(hModule);
	::SetCursor(::LoadCursor(NULL, IDC_ARROW));	// �J�[�\�������ɖ߂�
	if (bInner && strSearchEngine.IsEmpty() == FALSE) {
		::SetWindowText(m_hWndStatusBar, _T(""));
		CString str = GetActiveSelectedText();
		m_SearchBar.SearchWebWithEngine(str, strSearchEngine);
		bNoting = FALSE;
	}
	if (bInner == -1) {
		m_bCancelRButtonUp = true;
		return TRUE;
	}

	if (bNoting) {
		ptUp.x = GET_X_LPARAM(msg.lParam);
		ptUp.y = GET_Y_LPARAM(msg.lParam);
		::ClientToScreen(msg.hwnd, &ptUp);

		::SetWindowText( m_hWndStatusBar, _T("") );

		DWORD dwCommand = 0;
		CIniFileI	pr( _GetFilePath( _T("MouseEdit.ini") ), _T("MouseCtrl") );
		pr.QueryValue(dwCommand, strMove);
		pr.Close();

		if (dwCommand != 0) {
			::SendMessage(m_hWnd, WM_COMMAND, dwCommand, 0);
			bNoting = FALSE;
		} else if (dwCommand == -1)
			return TRUE;
	}

	if ( bNoting && strMove.IsEmpty() ) {
		::ScreenToClient(pMsg->hwnd, &ptUp);
		pMsg->lParam = MAKELONG(ptUp.x, ptUp.y);

		::PostMessage(pMsg->hwnd, WM_RBUTTONUP, pMsg->wParam, pMsg->lParam);
	}

	return !bNoting;
}


DWORD CMainFrame::GetMouseWheelCmd(short nWheel)
{
	CString 	strKey;
	if (nWheel > 0)
		strKey = _T("WHEEL_UP");
	else
		strKey = _T("WHEEL_DOWN");

	CIniFileI	pr( _GetFilePath( _T("MouseEdit.ini") ), _T("MouseCtrl") );
	DWORD		dwCommand = pr.GetValue(strKey, 0);
	return dwCommand;
}


DWORD CMainFrame::GetMouseButtonUpCmd(UINT uMsg, UINT nXButton/* = 0*/)
{
	CString 	strKey;
	switch (uMsg) {
	case WM_LBUTTONUP:	strKey = _T("LButtonUp");					break;
	case WM_MBUTTONUP:	strKey = _T("MButtonUp");					break;
	case WM_XBUTTONUP:	strKey.Format(_T("XButtonUp%d"), nXButton); break;
	}

	CIniFileI	pr( _GetFilePath( _T("MouseEdit.ini") ), _T("MouseCtrl") );
	DWORD		dwCommand = pr.GetValue(strKey, 0);
	return dwCommand;
}

//------------------------------------
/// �v���O�C���Ƀ��b�Z�[�W���t�B���^����@���^����
BOOL CMainFrame::TranslateMessageToBHO(MSG *pMsg)
{
  #if 1	//+++ �������ŁA��������if������߂�...��߂�Ȃ�?
	if (  (WM_KEYFIRST		 <= pMsg->message) && (pMsg->message <= WM_KEYLAST	)
	   || (WM_IME_SETCONTEXT <= pMsg->message) && (pMsg->message <= WM_IME_KEYUP)
	  #if 0	//+++ ������...
	   || (WM_MOUSEFIRST     <= pMsg->message) && (pMsg->message <= WM_MOUSELAST)
	   || (WM_NCMOUSEMOVE    <= pMsg->message) && (pMsg->message <= 0xAD/*WM_NCXBUTTONDBLCLK*/)
	  #endif
	)
  #endif 
	{
		int nCount = CPluginManager::GetCount(PLT_TOOLBAR);

		for (int i = 0; i < nCount; i++) {
			if ( CPluginManager::Call_PreTranslateMessage(PLT_TOOLBAR, i, pMsg) )
				return TRUE;
		}
	}

	return FALSE;
}


// �I���e�L�X�g��ɃJ�[�\�������邩�ǂ���
bool	CMainFrame::_IsSelectedTextInner()
{
	if (GetActiveSelectedText().IsEmpty())
		return false;

	try {
		HRESULT hr = S_OK;
		CChildFrame* pChild = GetActiveChildFrame();
		if (pChild == NULL) 
			AtlThrow(hr);

		CPoint	pt;
		::GetCursorPos(&pt);
		pChild->ScreenToClient(&pt);

		CComPtr<IDispatch>	spDisp;
		hr = pChild->m_spBrowser->get_Document(&spDisp);
		if (spDisp == NULL)
			AtlThrow(hr);

		CComQIPtr<IHTMLDocument2>	spDocument = spDisp;
		if (spDocument == NULL)
			AtlThrow(hr);


		auto funcGetHTMLWindowOnCursorPos = [](CPoint& pt, IHTMLDocument3* pDoc) -> CComPtr<IHTMLWindow2> {
			auto funcGetIFrameAbsolutePosition = [](CComQIPtr<IHTMLElement>	spIFrame) -> CRect {
				CRect rc;
				spIFrame->get_offsetHeight(&rc.bottom);
				spIFrame->get_offsetWidth(&rc.right);
				CComPtr<IHTMLElement>	spCurElement = spIFrame;
				do {
					CPoint temp;
					spCurElement->get_offsetTop(&temp.y);
					spCurElement->get_offsetLeft(&temp.x);
					rc += temp;
					CComPtr<IHTMLElement>	spTemp;
					spCurElement->get_offsetParent(&spTemp);
					spCurElement.Release();
					spCurElement = spTemp;
				} while (spCurElement.p);
				
				return rc;
			};
			auto funcGetScrollPosition = [](CComQIPtr<IHTMLDocument2> spDoc2) -> CPoint {
				CPoint ptScroll;
				CComPtr<IHTMLElement>	spBody;
				spDoc2->get_body(&spBody);
				CComQIPtr<IHTMLElement2>	spBody2 = spBody;
				spBody2->get_scrollTop(&ptScroll.y);
				spBody2->get_scrollLeft(&ptScroll.x);
				if (ptScroll == CPoint(0, 0)) {
					CComQIPtr<IHTMLDocument3>	spDoc3 = spDoc2;
					CComPtr<IHTMLElement>	spDocumentElement;
					spDoc3->get_documentElement(&spDocumentElement);
					CComQIPtr<IHTMLElement2>	spDocumentElement2 = spDocumentElement;
					spDocumentElement2->get_scrollTop(&ptScroll.y);
					spDocumentElement2->get_scrollLeft(&ptScroll.x);
				}
				return ptScroll;
			};

			HRESULT hr = S_OK;
			CComQIPtr<IHTMLDocument2>	spDoc2 = pDoc;

			CComPtr<IHTMLFramesCollection2>	spFrames;
			spDoc2->get_frames(&spFrames);
			CComPtr<IHTMLElementCollection>	spIFrameCol;
			pDoc->getElementsByTagName(CComBSTR(L"iframe"), &spIFrameCol);
			CComPtr<IHTMLElementCollection>	spFrameCol;
			pDoc->getElementsByTagName(CComBSTR(L"frame"), &spFrameCol);
			
			long frameslength = 0, iframelength = 0, framelength = 0;
			spFrames->get_length(&frameslength);
			spIFrameCol->get_length(&iframelength);
			spFrameCol->get_length(&framelength);
			ATLASSERT(frameslength == iframelength || frameslength == framelength);

			if (frameslength == iframelength && spIFrameCol.p && spFrames.p) {
				for (long i = 0; i < iframelength; ++i) {
					CComVariant vIndex(i);
					CComPtr<IDispatch>	spDisp2;
					spIFrameCol->item(vIndex, vIndex, &spDisp2);
					CRect rcAbsolute = funcGetIFrameAbsolutePosition(spDisp2.p);
					CPoint ptScroll = funcGetScrollPosition(spDoc2);
					CRect rc = rcAbsolute - ptScroll;
					if (rc.PtInRect(pt)) {
						pt.x	-= rc.left;
						pt.y	-= rc.top;
						CComVariant vResult;
						spFrames->item(&vIndex, &vResult);
						CComQIPtr<IHTMLWindow2> spWindow = vResult.pdispVal;
						return spWindow;
					}
				}
			}

			if (frameslength == framelength && spFrameCol.p && spFrames.p) {
				for (long i = 0; i < framelength; ++i) {
					CComVariant vIndex(i);
					CComPtr<IDispatch>	spDisp2;
					spFrameCol->item(vIndex, vIndex, &spDisp2);
					CComQIPtr<IHTMLElement>	spFrame = spDisp2;
					if (spFrame.p) {
						CRect rc = funcGetIFrameAbsolutePosition(spFrame);
						//spFrame->get_offsetLeft(&rc.left);
						//spFrame->get_offsetTop(&rc.top);
						//long temp;
						//spFrame->get_offsetWidth(&temp);
						//rc.right += rc.left + temp;
						//spFrame->get_offsetHeight(&temp);
						//rc.bottom+= rc.top + temp;
						if (rc.PtInRect(pt)) {
							pt.x	-= rc.left;
							pt.y	-= rc.top;
							CComVariant vResult;
							spFrames->item(&vIndex, &vResult);
							CComQIPtr<IHTMLWindow2> spWindow = vResult.pdispVal;
							return spWindow;
						}
					}
				}
			}
			return nullptr;
		};

		CComQIPtr<IHTMLDocument3>	spDocument3 = spDocument;
		CComPtr<IHTMLWindow2> spWindow = funcGetHTMLWindowOnCursorPos(pt, spDocument3);
		if (spWindow) {
			spDocument.Release();
			CComQIPtr<IHTMLDocument2>	spFrameDocument;
			hr = spWindow->get_document(&spFrameDocument);
			if ( FAILED(hr) ) {
				CComQIPtr<IServiceProvider>  spServiceProvider = spWindow;
				ATLASSERT(spServiceProvider);
				CComPtr<IWebBrowser2>	spBrowser;
				hr = spServiceProvider->QueryService(IID_IWebBrowserApp, IID_IWebBrowser2, (void**)&spBrowser);
				if (!spBrowser)
					AtlThrow(hr);
				CComPtr<IDispatch>	spDisp;
				hr = spBrowser->get_Document(&spDisp);
				if (!spDisp)
					AtlThrow(hr);
				spDocument = spDisp;
			} else {
				spDocument = spFrameDocument;
			}
		}

		CComPtr<IDispatch>	spTargetDisp;
		auto funcGetRangeDisp = [&spTargetDisp](CPoint pt, IHTMLDocument2 *pDocument) {
			CComPtr<IHTMLSelectionObject>		spSelection;
			HRESULT 	hr	= pDocument->get_selection(&spSelection);
			if ( SUCCEEDED(hr) ) {
				CComPtr<IDispatch>				spDisp;
				hr	   = spSelection->createRange(&spDisp);
				if ( SUCCEEDED(hr) ) {
					CComQIPtr<IHTMLTxtRange>	spTxtRange = spDisp;
					if (spTxtRange != NULL) {
						CComBSTR				bstrText;
						hr	   = spTxtRange->get_text(&bstrText);
						if (SUCCEEDED(hr) && !!bstrText) {
							spTargetDisp = spDisp;
						}
					}
				}
			}
		};

		funcGetRangeDisp(pt, spDocument);
		if (spTargetDisp == NULL)
			return false;

		CComQIPtr<IHTMLTextRangeMetrics2>	spMetrics = spTargetDisp;
		ATLASSERT(spMetrics);
		CComPtr<IHTMLRect>	spRect;
		hr = spMetrics->getBoundingClientRect(&spRect);
		if (FAILED(hr))
			AtlThrow(hr);

		CRect rc;
		spRect->get_top(&rc.top);
		spRect->get_left(&rc.left);
		spRect->get_right(&rc.right);
		spRect->get_bottom(&rc.bottom);
		if (rc.PtInRect(pt)) {
			return true;
		}
#if 0
		CComPtr<IHTMLRectCollection>	spRcCollection;
		hr = spMetrics->getClientRects(&spRcCollection);
		if (FAILED(hr))
			AtlThrow(hr);

		long l;
		spRcCollection->get_length(&l);
		for (long i = 0; i < l; ++i) {
			VARIANT vIndex;
			vIndex.vt	= VT_I4;
			vIndex.lVal	= i;
			VARIANT vResult;
			if (spRcCollection->item(&vIndex, &vResult) == S_OK) {
				CComQIPtr<IHTMLRect>	spRect = vResult.pdispVal;
				ATLASSERT(spRect);
				CRect rc;
				spRect->get_top(&rc.top);
				spRect->get_left(&rc.left);
				spRect->get_right(&rc.right);
				spRect->get_bottom(&rc.bottom);
				if (rc.PtInRect(pt)) {
					return true;
				}
			}
		}
#endif

	}
	catch (const CAtlException& e) {
		e;
	}
	return false;
}



// ===========================================================================
// �X�L���֌W

void CMainFrame::InitSkin()
{
	/* �t�H���g */
	m_MDITab.SetFont(CSkinOption::s_lfTabBar.CreateFontIndirect());
	m_AddressBar.SetFont(CSkinOption::s_lfAddressBar.CreateFontIndirect());
	m_SearchBar.SetFont(CSkinOption::s_lfSearchBar.CreateFontIndirect());
	m_LinkBar.SetFont(CSkinOption::s_lfLinkBar.CreateFontIndirect());
	m_wndStatusBar.SetProxyComboBoxFont(CSkinOption::s_lfProxyComboBox.CreateFontIndirect());

	/* �X�L�� */
	setMainFrameBg(CSkinOption::s_nMainFrameBgColor);			//+++ ���C���t���[����Bg�ݒ�.
	initCurrentIcon();											//+++ �A�C�R��
	m_CmdBar.setMenuBarStyle(m_hWndToolBar, false); 			//+++ ���j���[ (FEVATWH�̒Z���ɂ��邩�ۂ�)
	m_CmdBar.InvalidateRect(NULL, TRUE);						//���j���[�o�[
	m_ReBar.RefreshSkinState(); 								//ReBar
	m_MDITab.ReloadSkin();										//�^�u
	CToolBarOption::GetProfile();
	m_ToolBar.ReloadSkin(); 									//�c�[���o�[
	m_AddressBar.ReloadSkin(CSkinOption::s_nComboStyle);		//�A�h���X�o�[
	m_SearchBar.ReloadSkin(CSkinOption::s_nComboStyle); 		//�����o�[
	m_LinkBar.InvalidateRect(NULL, TRUE);						//�����N�o�[
	m_ExplorerBar.ReloadSkin(); 								//�G�N�X�v���[���o�[
	m_ExplorerBar.m_PanelBar.ReloadSkin();						//�p�l���o�[
	m_ExplorerBar.m_PluginBar.ReloadSkin(); 					//�v���O�C���o�[
	
	m_wndStatusBar.ReloadSkin( CSkinOption::s_nStatusStyle		//�X�e�[�^�X�o�[
							 , CSkinOption::s_nStatusTextColor
							 , CSkinOption::s_nStatusBackColor);

	setMainFrameCaptionSw(CSkinOption::s_nMainFrameCaption);	//+++ ���C���t���[���̃L���v�V�����̗L��.


}

// [Donut�̃I�v�V����]-[�X�L��]��[�K�p]�������ꂽ�Ƃ�
HRESULT CMainFrame::OnSkinChange()
{
	//���b�Z�[�W���u���[�h�L���X�g����ׂ���
	CSkinOption::GetProfile();

	InitSkin();

	//���t���b�V��
	RedrawWindow(NULL, NULL, RDW_INTERNALPAINT | RDW_UPDATENOW | RDW_ALLCHILDREN);;
	return 0;
}


#if 1 //+++	���C���t���[����Caption on/off
void CMainFrame::setMainFrameCaptionSw(int sw)
{
	if (sw == 0) {	//+++ �L���v�V�������O���ꍇ
		ModifyStyle(WS_CAPTION, 0);
	} else {
		ModifyStyle(0, WS_CAPTION); 			//+++ �L���v�V����������ꍇ.
	}
	m_mcCmdBar.SetExMode(sw ? 0/*�L*/ : m_hWnd/*��*/);

	SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_FRAMECHANGED);
}
#endif


///+++ ���݂̃X�L���̃A�C�R����(��)�ݒ�.
///    �� HICON �̊J�����Ă��Ȃ��Ă悢? �ʖڂŃ��[�N���Ă�?...win�̃��\�[�X�Ǘ����Ă悤�킩���....
void CMainFrame::initCurrentIcon()
{
	//+++ xp �r�W���A���X�^�C�����ꎞ�I��off
  #if 1	//+++ uxtheme.dll �̊֐��̌Ăяo������ύX.
	UxTheme_Wrap::SetWindowTheme(m_hWnd, L" ", L" ");
  #else
	CTheme		theme;
	theme.SetWindowTheme(m_hWnd, L" ", L" ");
  #endif

	/*static*/ HICON hIcon	= 0;
	//if (hIcon)
	//	::CloseHandle(hIcon);
	//hIcon 		= 0;
	CString strDir	= _GetSkinDir();
	m_strIcon		= strDir + _T("MainFrameBig.ico");
	if (Misc::IsExistFile(m_strIcon) == 0)
		m_strIcon	= strDir + _T("icon.ico");
	if (Misc::IsExistFile(m_strIcon))
		hIcon = (HICON)::LoadImage(ModuleHelper::GetResourceInstance(), m_strIcon, IMAGE_ICON, 32, 32, LR_SHARED|LR_LOADFROMFILE );
	if (hIcon == 0) {
		m_strIcon.Empty();
		hIcon = (HICON)::LoadImage(ModuleHelper::GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
	}
	if (hIcon)
		::SetClassLongPtr(m_hWnd, GCLP_HICON  , (LONG_PTR)hIcon );
	//::CloseHandle(hIcon);

	/*static*/ HICON hIconSm	= 0;
	//if (hIconSm)
	//	::CloseHandle(hIconSm);
	//hIconSm			= 0;
	m_strIconSm 		= strDir + _T("MainFrameSmall.ico");
	if (Misc::IsExistFile(m_strIconSm) == 0)
		m_strIconSm 	= strDir + _T("icon.ico");
	if (Misc::IsExistFile(m_strIconSm))
		hIconSm 	= (HICON)::LoadImage(ModuleHelper::GetResourceInstance(), m_strIconSm, IMAGE_ICON, 16, 16, LR_SHARED|LR_LOADFROMFILE);
	if (hIconSm == 0) {
		m_strIconSm.Empty();
		hIconSm   = (HICON)::LoadImage(ModuleHelper::GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
	}
	if (hIconSm)
		::SetClassLongPtr(m_hWnd, GCLP_HICONSM, (LONG_PTR)hIconSm );
	//::CloseHandle(hIconSm);

	//+++ XP�̃r�W���A���X�^�C���ɖ߂�...����͂���Ȃ�����
  #if 1	//+++ uxtheme.dll �̊֐��̌Ăяo������ύX.
	UxTheme_Wrap::SetWindowTheme(m_hWnd, 0, 0);
  #else
	theme.SetWindowTheme(m_hWnd, 0, 0);
  #endif
}


//+++ ���C���t���[����Bg�ݒ�.
void CMainFrame::setMainFrameBg(int bgColor)
{
	m_nBgColor = bgColor;

	CString 	strPath = _GetSkinDir() + _T("bg.bmp");
	m_bmpBg.Attach( AtlLoadBitmapImage(strPath.GetBuffer(0), LR_LOADFROMFILE) );
}


///+++ bg�`��
bool CMainFrame::drawMainFrameBg(HDC hDC)
{
  #if 1 //+++ bg�^�C���̂�
	if (m_bmpBg.m_hBitmap) {		// bg�摜��~���l�߂ĕ\��
		DrawBmpBg_Tile(hDC);
		return 1;
	} else if (m_nBgColor >= 0) {	// �F�w�肪����΁A���̐F�łׂ��h��
		HBRUSH	hBrushBg = CreateSolidBrush(COLORREF(m_nBgColor));
		RECT	rect;
		GetClientRect(&rect);
		::FillRect( hDC, &rect, hBrushBg );
		DeleteObject(hBrushBg);
		return 1;
	}
	return 0;
  #else //+++ �w�i�F��bg�������ɕ\�������K�v������Ƃ�
	if (m_nBgColor < 0 || m_bmpBg.m_hBitmap == 0)
		return 0;

	if (m_nBgColor >= 0) {
		HBRUSH	hBrushBg = CreateSolidBrush(COLORREF(m_nBgColor));
		RECT	rect;
		GetClientRect(&rect);
		::FillRect( hDC, &rect, hBrushBg );
		DeleteObject(hBrushBg);
	}
	if (m_bmpBg.m_hBitmap) {
		//if (m_nBgStyle == SKIN_BG_STYLE_TILE)
		DrawBmpBg_Tile(hDC);
		//else if (m_nBGStyle == SKN_BG_STYLE_STRETCH)
		//DrawBmpBg_Stretch(hDC);
	}
	return 1;
  #endif
}


//+++ bg�`��(�~���l�߂�^�C�v)
void CMainFrame::DrawBmpBg_Tile(HDC hDC)
{
	CRect	rc;
	GetClientRect(&rc);

	CDC 	dcSrc;
	dcSrc.CreateCompatibleDC(hDC);

	HBITMAP hOldbmpSrc	= dcSrc.SelectBitmap(m_bmpBg.m_hBitmap);
	SIZE	size;
	m_bmpBg.GetSize(size);
	DWORD	srcW = size.cx;
	DWORD	srcH = size.cy;
	DWORD	dstW = rc.Width();
	DWORD	dstH = rc.Height();
	for (unsigned y = 0; y < dstH; y += srcH) {
		for (unsigned x = 0; x < dstW; x += srcW) {
			::BitBlt(hDC, x, y, srcW, srcH, dcSrc, 0, 0, SRCCOPY);
		}
	}

	dcSrc.SelectBitmap(hOldbmpSrc);
}


#if 0 //+++ ��ʂ̍X�V���悤�킩��Ȃ��̂ŁA�g�k�͎~��
//+++ ���摜���g�債�ĕ\��.
void CMainFrame::DrawBmpBg_Stretch(HDC hDC)
{
	CRect	rc;
	GetClientRect(&rc);
  #if 0
	InvalidateRect(rc, TRUE);
	UpdateWindow();
  #endif

	CDC 	dcSrc;
	dcSrc.CreateCompatibleDC(hDC);

	HBITMAP hOldbmpSrc	= dcSrc.SelectBitmap(m_bmpBg.m_hBitmap);
	SIZE	size;
	m_bmpBg.GetSize(size);
	DWORD	srcW = size.cx;
	DWORD	srcH = size.cy;
	double	srcR = double(srcW) / double(srcH);
	DWORD	dstW = rc.Width();
	DWORD	dstH = rc.Height();
	double	dstR = double(dstW) / double(dstH);
	if (dstR > srcR) {			//+++ �`��͈͂̂ق����A���摜��������
		srcH = DWORD( srcH * srcR / dstR );
	} else if (dstR < srcR) {	//+++ �`��͈͂̂ق����A���摜�����c��
		srcW = DWORD( srcW * dstR / srcR );
	}
	::StretchBlt(hDC, 0, 0, dstW, dstH, dcSrc , 0, 0, srcW, srcH, SRCCOPY);
	dcSrc.SelectBitmap(hOldbmpSrc);
	//ValidateRect(NULL);
}
#endif


//public:
#if 0 //+++ ���Ƃ�
HICON	CMainFrame::LoadIcon4AboutDialog()
{
	int 	w = ::GetSystemMetrics(SM_CXICON);
	int 	h = ::GetSystemMetrics(SM_CYICON);
	HICON	hIcon = 0;
  #if 1
	if (IsExistFile(m_strIcon))
		hIcon = (HICON)::LoadImage(ModuleHelper::GetResourceInstance(), m_strIcon, IMAGE_ICON, w, h, LR_SHARED|LR_LOADFROMFILE);
	if (hIcon == 0)
  #endif
		hIcon = (HICON)::LoadImage(ModuleHelper::GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, w, h, LR_DEFAULTCOLOR );
	return hIcon;
}
#endif


void	CMainFrame::OpenBingTranslator(const CString& strText)
{
	m_MDITab.SetLinkState(LINKSTATE_A_ON);
	DWORD dwDLFlgs = CDLControlOption::s_dwDLControlFlags & ~(DLCTL_NO_SCRIPTS | DLCTL_NO_RUNACTIVEXCTLS);
	CChildFrame *pChild = CChildFrame::NewWindow(	m_hWndMDIClient,
													m_MDITab,
													m_AddressBar,
													false,
													dwDLFlgs, CDLControlOption::s_dwExtendedStyleFlags);
	if (pChild) {
		pChild->SetWaitBeforeNavigate2Flag();
		pChild->ActivateFrame(-1);	//\\ �^�u�ɒǉ�
		CString strtemp;
		strtemp.Format(_T("SourceText=%s&from=en&to=ja"), strText);
		std::vector<char>	vecStr = Misc::tcs_to_sjis(strtemp);
		pChild->Navigate2(_T("http://www.microsofttranslator.com/Default.aspx"), 0, NULL, 
			_T("Content-Type: application/x-www-form-urlencoded"), (LPVOID)vecStr.data(), vecStr.size() - 1);
	}
}

// ===========================================================================
// MainFrame_OpenFile����

bool CMainFrame::OnDDEOpenFile(const CString &strFileName)
{
	//dmfTRACE(_T("CMainFrame::OnDDEOpenFile(%s)\n"), strFileName);
  #if 1 //+++ �g���C��Ԃ���̕��A�ł̃o�O�΍�.
	DWORD flags = DonutGetStdOpenFlag();
	OnUserOpenFile( strFileName, flags );
	if (CStartUpOption::s_dwActivate) {
		IfTrayRestoreWindow();							//+++ �g���C��Ԃ������畜��.
		if (IsZoomed() == FALSE)
			ShowWindow_Restore(true);
		MtlSetForegroundWindow(m_hWnd);
	}
  #else
	OnUserOpenFile( strFileName, DonutGetStdOpenFlag() );
	if (CStartUpOption::s_dwActivate) {
		//+++ OnUserOpenFile()�̎��_�ōő剻/�t���X�N���[���������ꍇ�̑΍��nCmdShow��n���čl������悤�ɕύX.
		MtlSetForegroundWindow(m_hWnd, m_OnUserOpenFile_nCmdShow);	//+++ ���͂���nCmdShow�͈Ӗ����Ⴄ���̂������Ă��Ă�悤�ȋC������...�����Ƃ܂킵.
	}
  #endif
	// UDT DGSTR ( added by dai
	return true;
}




//public:
// Message handlers
// alternates OpenFile

#if 1	//+++ url�ʊg���v���p�e�B�̏�����ǉ�
HWND CMainFrame::OnUserOpenFile(const CString& strUrl, DWORD dwOpenFlag)
{
  #if 0	//+++ CChildFrame::OnBeforeNavigate2���ŏ�������悤�ɕύX�����̂ŁA�����ok ...�Ƃ�����...
	return UserOpenFile(strUrl, dwOpenFlag);
  #else	//+++ CChildFrame::OnBeforeNavigate2���ŏ�������悤�ɂ����̂ŁA2�d�Ɍ������������Ă��܂����A
		//+++ �I�v�V�������f�̃^�C�~���O�̓s���Ȃ�ׂ������t���O��ݒ肵���ق����悳�����Ȃ̂ŁA���x�]���ɂ���Ȃ�A������

	CString		str(strUrl);
	MtlRemoveStringHeaderAndFooter(str);

	// �����o�[���g���Č�������
	if (str.Left(13) == _T("SearchEngine:")) {
		std::wstring	strUrl2 = str;
		std::wregex	rx(L"SearchEngine:\"(.+?)\" Keyword:\"(.*?)\"");
		std::wsmatch	rt;
		if (std::regex_search(strUrl2, rt, rx)) {
			CString strKeyword = rt.str(2).c_str();
			if (strKeyword.IsEmpty())
				return NULL;
			CString strEngine  = rt.str(1).c_str();
			m_SearchBar.SearchWebWithEngine(strKeyword, strEngine);
			return NULL;
		}
		return NULL;
	}

   #if 0	//+++ �d�l�Ǝv�������A.url���̊g���v���p�e�B�̌�������̂ŁA�����ł͂ł��Ȃ�
	if ( MtlIsExt( str, _T(".url") ) ) {
		if (MTL::ParseInternetShortcutFile(str) == 0)
			return NULL;
	}
   #endif

	DWORD dwExProp	 = 0xFFFFFFFF;
	DWORD dwExProp2	 = 8;
	if (   (_check_flag(D_OPENFILE_NOCREATE, dwOpenFlag) == 0 || CUrlSecurityOption::activePageToo())
		&& CUrlSecurityOption::FindUrl(str, &dwExProp, &dwExProp2, 0)
	) {
		return OpenUrlWithExProp(str, dwOpenFlag, dwExProp, dwExProp2);
	} else {
		return UserOpenFile(str, dwOpenFlag);
	}
  #endif
}
#endif



//+++ url�ʊg���v���p�e�B�Ή��ŁA�{����OnUserOpenFile��UserOpenFile�ɕϖ�. �����𖖂ɒǉ�.
HWND CMainFrame::UserOpenFile(const CString& strFile0, DWORD openFlag, int dlCtrlFlag, int extededStyleFlags)
{
	CString		strFile (strFile0);
	MtlRemoveStringHeaderAndFooter(strFile);

	if ( MtlIsExt( strFile, _T(".url") ) ) {	//+++ ����:url�̂Ƃ��̏���. �g���v���p�e�B�̎擾�Ƃ�����̂ŁA��p��open������...
		return OpenInternetShortcut(strFile, openFlag);
	}

	if (  !MtlIsProtocol( strFile, _T("http") )
	   && !MtlIsProtocol( strFile, _T("https") ) )
	{
		if ( MtlPreOpenFile(strFile) )
			return NULL;	// handled
	  #if 1	//+++	�^�u������Ƃ�
		if (m_MDITab.GetItemCount() > 0) {
			if (strFile.Find(':') < 0 && strFile.Left(1) != _T("/") && strFile.Left(1) != _T("\\")) {	// ���łɋ�̓I�ȃp�X�łȂ��Ƃ��A
				HWND			hWndChild 	= m_MDITab.GetTabHwnd(m_MDITab.GetCurSel());
				CChildFrame*	pChild 		= GetChildFrame(hWndChild);
				if (pChild) {		//+++ �A�N�e�B�u�y�[�W���݂�
					CString strBase  = pChild->GetLocationURL();
					CString str7     = strBase.Left(7);
					if (str7 == "file://") {	//+++ file://��������΁A���[�J���ł̃f�B���N�g���ړ����낤�Ƃ݂Ȃ���
						strFile = Misc::MakeFullPath( strBase, strFile );	//+++ �t���p�X��
					}
				}
			}
		}
	  #endif
	}

	if (strFile.GetLength() > INTERNET_MAX_PATH_LENGTH)
		return NULL;

	// UH (JavaScript dont create window)
	CString 	 strJava = strFile.Left(11);
	strJava.MakeLower();
	if ( strJava == _T("javascript:") ) {
		if ( strFile == _T("javascript:location.reload()") )
			return NULL;
		openFlag |= D_OPENFILE_NOCREATE;
	}

	// dfg files
	if ( MtlIsExt( strFile, _T(".dfg") ) && ::PathFileExists(strFile)) {
		if ( !(CMainOption::s_dwMainExtendedStyle & MAIN_EX_NOCLOSEDFG) ) {
			_LoadGroupOption(strFile, true);
		} else {
			_LoadGroupOption(strFile, false);
		}
		return NULL;
	}

	if ( MtlIsExt( strFile, _T(".xml") ) && ::PathFileExists(strFile) ) {
		if ( !(CMainOption::s_dwMainExtendedStyle & MAIN_EX_NOCLOSEDFG) ) {
			RestoreAllTab(strFile, true);
		} else {
			RestoreAllTab(strFile, false);
		}
		return NULL;
	}

	//+++	�T�C�^�}����(about:blank�̕s��C���Ō���ăR�����g�A�E�g...
	//+++	��������unDonut+���A�����Ƌ@�\���Ă��Ȃ��悤����?�A�킴�킴��������K�v���Ȃ����ȁA��)
	//+++   ������ƋC���ς�����̂ŏ����t�̎b�蕜��
	// minit(about:* pages)
	{
		CString strAbout = strFile.Left(6);
		if ( strAbout == _T("about:") && strFile != _T("about:blank") ) {
			HWND hWndAbout =  _OpenAboutFile(strFile);
			if (hWndAbout)
				return hWndAbout;
		}
	}

	HWND	hWndActive = MDIGetActive();

	int 	nCmdShow   = _check_flag(D_OPENFILE_ACTIVATE, openFlag) ? -1 : SW_SHOWNOACTIVATE;
	if (hWndActive == NULL) {					// no window yet
		nCmdShow = -1;							// always default
	}

	if ( hWndActive != NULL && _check_flag(D_OPENFILE_NOCREATE, openFlag) ) {
		CWebBrowser2 browser = DonutGetIWebBrowser2(hWndActive);

		if ( !browser.IsBrowserNull() ) {
			browser.Navigate2(strFile);

			if ( !_check_flag(D_OPENFILE_NOSETFOCUS, openFlag) ) {
				// reset focus
				::SetFocus(NULL);
				MtlSendCommand(hWndActive, ID_VIEW_SETFOCUS);
			}

			return NULL;
			//return hWndActive;
		}
	}

 	//+++
	//x if (CMainOption::s_dwMainExtendedStyle & (/*MAIN_EX_NOACTIVATE_NEWWIN|*/MAIN_EX_NEWWINDOW))
	{
		//+++ if (dlCtrlFlag < 0)
		//+++	dlCtrlFlag = CDLControlOption::s_dwDLControlFlags;
		CChildFrame* pChild 	  = CChildFrame::NewWindow(m_hWndMDIClient, m_MDITab, m_AddressBar, false/*true*/, dlCtrlFlag, extededStyleFlags);
		if (pChild == NULL)
			return NULL;

		if ( !strFile.IsEmpty() ) {
			pChild->SetWaitBeforeNavigate2Flag();			//+++ �������ABeforeNavigate2()�����s�����܂ł̊ԃA�h���X�o�[���X�V���Ȃ��悤�ɂ���t���O��on... ��������s�v���낤��...
			m_OnUserOpenFile_nCmdShow = pChild->ActivateFrame(nCmdShow);	//\\ �^�u�ɒǉ�

			const SearchPostData& PostData = m_SearchBar.GetSearchPostData();
			if (PostData.pPostData) {	// POST����
				pChild->Navigate2(strFile, 0, NULL, 
					_T("Content-Type: application/x-www-form-urlencoded"), PostData.pPostData, PostData.nPostBytes);
			} else {
				pChild->Navigate2(strFile);
			}
			//pChild->Navigate2(_T("http://www.microsofttranslator.com/Default.aspx"), 0, NULL, _T("Content-Type: application/x-www-form-urlencoded"), (LPVOID)"SourceText=marketplace", ::strlen("SourceText=marketplace"));

			//CString strUrl;
			//strUrl.Format(_T("http://api.microsofttranslator.com/v2/Http.svc/Translate?appId=%s&text=%s&from=en&to=ja"), _T("6CB08565F99A903FB046716AA865A256A122E24C"), _T("test"));
			//pChild->Navigate2(strUrl);


		} else {
			m_OnUserOpenFile_nCmdShow = pChild->ActivateFrame(nCmdShow);	//\\ �^�u�ɒǉ�
		}

		if ( !_check_flag(D_OPENFILE_NOSETFOCUS, openFlag) ) {
			if (MDIGetActive() == pChild->m_hWnd) { // a new window activated, so automatically set focus
				// reset focus
				::SetFocus(NULL);
				MtlSendCommand(pChild->m_hWnd, ID_VIEW_SETFOCUS);
			} else {
				// It's reasonable not to touch a current focus.
			}
		}

		return pChild->m_hWnd;
	}
}



LRESULT CMainFrame::OnOpenWithExProp(_EXPROP_ARGS *pArgs)
{
	if (pArgs == NULL) 
		return 0;

	//\\ ���Ԃ񌟍������Ƃ��ɔ��ł���֐�
	CChildFrame *pActiveChild = GetActiveChildFrame();
	if( pActiveChild)
		pActiveChild->SaveSearchWordflg(false); //\\ �����o�[�Ō��������Ƃ��A�N�e�B�u�ȃ^�u�̌����������ۑ����Ȃ��悤�ɂ���
	
	bool bOldSaveFlag = CSearchBarOption::s_bSaveSearchWord;	// �����o�[�̕����񂪏����錏�Ɉꉞ�̑Ώ�
	CSearchBarOption::s_bSaveSearchWord = false;				// �{���͂���Ȃ��Ƃ�����_��
	HWND hWndNew = OpenUrlWithExProp(pArgs->strUrl, pArgs->dwOpenFlag, pArgs->strIniFile, pArgs->strSection);
	if (hWndNew == NULL) 
		hWndNew = MDIGetActive();
	CSearchBarOption::s_bSaveSearchWord = bOldSaveFlag;

	CChildFrame *pChild  = GetChildFrame(hWndNew);
	if (pChild) {
		CString str = pArgs->strSearchWord;

		// �u�S�p�𔼊p�ɒu���v���`�F�b�N����Ă���ϊ����Ďq�E�B���h�E�ɕ������n���B
		//if (CSearchBarOption::s_bFiltering) 
		//	m_SearchBar.FilterString(str);

	  	//+++ �q���Ɍ����ݒ�𔽉f (�֐���)
	   #if 1 //defined USE_UNDONUT_G_SRC
		if (CSearchBarOption::s_bAutoHilight) {
			pChild->SetSearchWordAutoHilight( str, true );
		}
		//pChild->SetNowSearchWord(str);
	   #else
		pChild->SetSearchWordAutoHilight( str, (dwStatus & STS_AUTOHILIGHT) != 0 );
	   #endif
	}

	return (LRESULT) hWndNew;
}


HWND CMainFrame::OpenInternetShortcut(CString strUrlFile, DWORD dwOpenFlag)
{
	CString strUrl = strUrlFile;

	if ( !MTL::ParseInternetShortcutFile(strUrl) )
		return NULL;

	return OpenUrlWithExProp(strUrl, dwOpenFlag, strUrlFile);
}


HWND CMainFrame::OpenUrlWithExProp(CString strUrl, DWORD dwOpenFlag, DWORD dwExProp, DWORD dwExProp2)
{
	if ( _check_flag(D_OPENFILE_NOCREATE, dwOpenFlag) ) {
		return OpenExPropertyActive(strUrl, dwExProp, dwExProp2, dwOpenFlag);		//�����̃^�u�ŊJ��
	} else {
		return OpenExPropertyNew(strUrl, dwExProp, dwExProp2, dwOpenFlag); 		//�V�K�ɊJ��
	}
}


HWND CMainFrame::OpenUrlWithExProp(CString strUrl, DWORD dwOpenFlag, CString strIniFile, CString strSection /*= DONUT_SECTION*/)
{
	DWORD dwExProp = 0xAAAAAA;		//+++ �����l�ύX
	DWORD dwExProp2= 0x8;			//+++ �g���v���p�e�B�𑝐�.

	if ( CExProperty::CheckExPropertyFlag(dwExProp, dwExProp2, strIniFile, strSection) ) {
		if ( _check_flag(D_OPENFILE_NOCREATE, dwOpenFlag) ) {
			return OpenExPropertyActive(strUrl, dwExProp, dwExProp2, dwOpenFlag);	//�����̃^�u�ŊJ��
		} else {
			return OpenExPropertyNew(strUrl, dwExProp, dwExProp2, dwOpenFlag); 	//�V�K�ɊJ��
		}
	} else {
	  #if 1	//+++	URL�ʊg���v���p�e�B�̃`�F�b�N������.
			//		...���������ACChildFrame::OnBeforeNavigate2���ŏ�������悤�ɕύX�����̂ŁA�����͂Ȃ�
			//		...�ɂ��������������A�ǂ����Adl�I�v�V�������f�̃^�C�~���O�Ƀ��O������悤�Ȃ�ŁA
			//		�Ȃ�ׂ��������f�����悤�ɁA���x�]���Ŕ��f.
		if (   (_check_flag(D_OPENFILE_NOCREATE, dwOpenFlag) == 0 || CUrlSecurityOption::activePageToo())
			&& CUrlSecurityOption::FindUrl(strUrl, &dwExProp, &dwExProp2, 0)
		) {
			return OpenUrlWithExProp(strUrl, dwOpenFlag, dwExProp, dwExProp2);
		}
	  #endif
		return OpenExPropertyNot(strUrl, dwOpenFlag);					//�W���I�v�V�����ŊJ��
	}
}


//�����^�u�Ƀi�r�Q�[�g�����̂��Ɋg���ݒ��K�p����.  //+++ dwExProp2����.
HWND CMainFrame::OpenExPropertyActive(CString &strUrl, DWORD dwExProp, DWORD dwExProp2, DWORD dwOpenFlag)
{
	dwOpenFlag |= D_OPENFILE_NOCREATE;

	//�A�N�e�B�u�ȃ^�u���i�r�Q�[�g���b�N����Ă��邩���m�F
	HWND		 hWndActive = MDIGetActive();
	if ( hWndActive && ::IsWindow(hWndActive) ) {
		CChildFrame *pChild = GetChildFrame(hWndActive);

		if (pChild) {
			DWORD dwExFlag = pChild->view().m_ViewOption.m_dwExStyle;

			if (dwExFlag & DVS_EX_OPENNEWWIN)
				return OpenExPropertyNew(strUrl, dwExProp, dwExProp2, dwOpenFlag);
		}
	}

	//+++ (dwExProp2���݂�UserOpenFile�̌�ɂ������̂�O�Ɉړ�.)
	CExProperty  ExProp(CDLControlOption::s_dwDLControlFlags, CDLControlOption::s_dwExtendedStyleFlags, 0, dwExProp, dwExProp2);

	//�擾����URL���A�N�e�B�u�ȃ^�u�ŊJ������i�W�������ɔC����j
	BOOL		 	bOpened		= FALSE;
	HWND 			hWndNew		= UserOpenFile(strUrl, dwOpenFlag, ExProp.GetDLControlFlags(), ExProp.GetExtendedStyleFlags() );

	if (hWndNew && !hWndActive) {
		hWndActive = hWndNew;											//�E�B���h�E�����������̂ŐV�K�ɊJ����
		bOpened    = TRUE;
	}

	//�g���v���p�e�B��K�p����
	if (hWndActive == NULL)
		return NULL;
	CChildFrame*	pChild 	= GetChildFrame(hWndActive);
	if (!pChild)
		return NULL;

	pChild->view().PutDLControlFlags( ExProp.GetDLControlFlags() );
	pChild->SetViewExStyle(ExProp.GetExtendedStyleFlags(), TRUE);
	pChild->view().m_ViewOption.SetAutoRefreshStyle( ExProp.GetAutoRefreshFlag() );

	if (bOpened)
		return hWndActive;

	return NULL;
}


//�V�K�^�u���J�����̂��g���ݒ��K�p����
HWND CMainFrame::OpenExPropertyNew(CString &strUrl, DWORD dwExProp, DWORD dwExProp2, DWORD dwOpenFlag)
{
  #if 1	//+++
	dwOpenFlag &= ~D_OPENFILE_NOCREATE;

	CExProperty  ExProp(CDLControlOption::s_dwDLControlFlags, CDLControlOption::s_dwExtendedStyleFlags, 0, dwExProp, dwExProp2);
	int			 dlCtrlFlag        = ExProp.GetDLControlFlags();
	int			 extendedStyleFlag = ExProp.GetExtendedStyleFlags();
	//URL�ŐV�K�^�u���J��
	HWND 			hWndNew		= UserOpenFile(strUrl, dwOpenFlag, dlCtrlFlag, extendedStyleFlag);
	if ( hWndNew == 0 || !::IsWindow(hWndNew) )
		return NULL;

	//�g���v���p�e�B��K�p����
	CChildFrame *pChild  = GetChildFrame(hWndNew);
	if (!pChild)
		return NULL;

	pChild->view().PutDLControlFlags( dlCtrlFlag );
	pChild->SetViewExStyle(ExProp.GetExtendedStyleFlags(), TRUE);
	pChild->view().m_ViewOption.SetAutoRefreshStyle( ExProp.GetAutoRefreshFlag() );

	return hWndNew;
  #else
	dwOpenFlag &= ~D_OPENFILE_NOCREATE;

	//URL�ŐV�K�^�u���J��
	HWND 			hWndNew		= UserOpenFile(strUrl, dwOpenFlag);
	if ( hWndNew == 0 || !::IsWindow(hWndNew) )
		return NULL;

	//�g���v���p�e�B��K�p����
	CChildFrame *pChild  = GetChildFrame(hWndNew);
	if (!pChild)
		return NULL;
	CExProperty  ExProp(CDLControlOption::s_dwDLControlFlags, CDLControlOption::s_dwExtendedStyleFlags, 0, dwExProp);

	pChild->view().PutDLControlFlags( ExProp.GetDLControlFlags() );
	pChild->SetViewExStyle(ExProp.GetExtendedStyleFlags(), TRUE);
	pChild->view().m_ViewOption.SetAutoRefreshStyle( ExProp.GetAutoRefreshFlag() );

	return hWndNew;
  #endif
}


//�^�u���J�����̂��A�W���̐ݒ��K�p����
HWND CMainFrame::OpenExPropertyNot(CString &strUrl, DWORD dwOpenFlag)
{
	//�A�N�e�B�u�ȃ^�u���i�r�Q�[�g���b�N����Ă��邩���m�F
	HWND hWndActive = MDIGetActive();

	if ( ::IsWindow(hWndActive) ) {
		CChildFrame *pChild = GetChildFrame(hWndActive);

		if (pChild) {
			DWORD dwExFlag = pChild->view().m_ViewOption.m_dwExStyle;

			if (dwExFlag & DVS_EX_OPENNEWWIN)
				dwOpenFlag &= ~D_OPENFILE_NOCREATE; 			//�V�K�^�u�ŊJ���悤��
		}
	}

	//�擾����URL���J������
	HWND 			hWndNew		= UserOpenFile(strUrl, dwOpenFlag);
	if (hWndNew) {
		//�V�K�ɊJ�����^�u�Ȃ̂ŉ������Ȃ��Ă悢
		return hWndNew;
	} else {
		//�I�v�V������W���̂��̂ŏ㏑��
		CChildFrame *pChild = GetChildFrame( MDIGetActive() );

		if (!pChild)
			return NULL;

		pChild->view().PutDLControlFlags(CDLControlOption::s_dwDLControlFlags);
		pChild->SetViewExStyle(CDLControlOption::s_dwExtendedStyleFlags, TRUE);
		pChild->view().m_ViewOption.SetAutoRefreshStyle(0);
	}

	return NULL;
}



LRESULT CMainFrame::OnFileRecent(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	// get file name from the MRU list
	if (ID_FILE_MRU_FIRST <= wID && wID <= ID_FILE_MRU_LAST)						//���͈�ID����V�͈�ID�֕ϊ�
		wID = wID - ID_FILE_MRU_FIRST + ID_RECENTDOCUMENT_FIRST;

	CRecentDocumentListFixed::RecentDocEntry Entry;

	if ( CMainOption::s_pMru->GetFromList(wID, &Entry) ) {
		DWORD dwOpenFlag = DonutGetStdOpenCreateFlag(); /*DonutGetStdOpenFlag()*/	// Force New Window // UDT DGSTR
		HWND  hWndNew	 = OnUserOpenFile(Entry.szDocName, dwOpenFlag);

		if ( ::IsWindow(hWndNew) ) {
			if (CMainOption::s_bTravelLogClose) {
				CChildFrame *pChild = GetChildFrame(hWndNew);

				if (pChild) {
					_Load_OptionalData2(pChild, Entry.arrFore, Entry.arrBack);
				}
			}
		}

		CMainOption::s_pMru->RemoveFromList(wID);									// UDT DGSTR
	} else {
		::MessageBeep(MB_ICONERROR);
	}

	return 0;
}



//+++ �����F�����N�������ŁA�ʂ̃v���O������������t���ŋN�����ꂽ�ꍇ�ɁA�����ɂ���B
//+++       ���̊O������̒ǉ��\����DDE�o�R�ōs����悤�ŁA������ɂ��邱�Ƃ͂Ȃ�����...?
void CMainFrame::OnNewInstance(ATOM nAtom)			// WM_NEWINSTANCE
{
	enum { NAME_LEN = 0x4000 };
	TCHAR szBuff[NAME_LEN+2];
	szBuff[0]	= 0;
	bool	bActive = !(CMainOption::s_dwMainExtendedStyle & MAIN_EX_NOACTIVATE);

	if (::GlobalGetAtomName(nAtom, szBuff, NAME_LEN) != 0) {
		//\\ 1�s���n�������ɕς����̂�
		CString strPath = szBuff;
		if (strPath.CompareNoCase(_T("-tray")) == 0 || strPath.CompareNoCase(_T("/tray")) == 0)
			bActive = false;
#if 0
		//dmfTRACE(_T("CMainFrame::OnNewInstance: %s\n"), szBuff);
		CString					strPath;
		std::vector<CString>	strs;
		Misc::SeptTextToWords(strs, szBuff);
		for (unsigned i = 0; i < strs.size(); ++i)�@
		{
			CString&	str = strs[i];
			int 		c   = str[0];

			if (c == _T('-') || c == _T('/')) {
				if (str.CompareNoCase(_T("-tray")) == 0 || str.CompareNoCase(_T("/tray"))) {
					bActive = false;
				}
			} else {
				if (strPath.IsEmpty())
					strPath = str;
				else
					strPath += _T(' ') + str;
			}
		}
#endif
		OnUserOpenFile( strPath, DonutGetStdOpenFlag() );
		::GlobalDeleteAtom(nAtom);
	}

	if ( bActive ) {
		IfTrayRestoreWindow();									//+++ �g���C��Ԃ������畜��.
		if (IsZoomed() == FALSE)
			ShowWindow(SW_RESTORE);
		MtlSetForegroundWindow(m_hWnd); 						//�E�C���h�E���A�N�e�B�u�ɂ���
		//if (m_bOldMaximized == 0 && m_bFullScreen == 0) 		//+++
		//	ShowWindow_Restore(1);	//ShowWindow(SW_RESTORE);	//+++ �T�C�Y��߂�.
	}
}



////////////////////////////////////////////////////////////////////////////////
//�t�@�C�����j���[
//�@�R�}���h�n���h��
////////////////////////////////////////////////////////////////////////////////
////�V�K�쐬

//�|�[���y�[�W
void CMainFrame::OnFileNewHome(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	HWND hWndChild = OnUserOpenFile( CString(), DonutGetStdOpenActivateFlag() );

	if (hWndChild) {
		CWebBrowser2 browser = DonutGetIWebBrowser2(hWndChild);

		if (browser.m_spBrowser != NULL)
			browser.GoHome();
	}
}


//���݂̃y�[�W
void CMainFrame::OnFileNewCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	HWND hWndActive = MDIGetActive();

	if (hWndActive) {
		CWebBrowser2 browser = DonutGetIWebBrowser2(hWndActive);

		if (browser.m_spBrowser != NULL) {
			CString strURL = browser.GetLocationURL();

			if ( !strURL.IsEmpty() )
				OnUserOpenFile( strURL, DonutGetStdOpenActivateFlag() );
		}
	}
}


LRESULT CMainFrame::OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	if (CFileNewOption::s_dwFlags == FILENEW_BLANK) {
		OnFileNewBlank(0, 0, 0);
	} else if (CFileNewOption::s_dwFlags == FILENEW_COPY) {
		if (MDIGetActive() != NULL)
			SendMessage(WM_COMMAND, MAKEWPARAM(ID_FILE_NEW_COPY, 0), 0);
		else
			OnFileNewBlank(0, 0, 0);
	} else if (CFileNewOption::s_dwFlags == FILENEW_HOME) {
		OnFileNewHome(0, 0, 0);
	} else if (CFileNewOption::s_dwFlags == FILENEW_USER) {		//+++ ���[�U�[�w��̃y�[�W���J��
		//CIniFileI 	pr( g_szIniFileName, _T("Main") );
		//CString		str = pr.GetStringUW( _T("File_New_UsrPage") );
		//pr.Close();
		CString&	str	= CFileNewOption::s_strUsr;
		if ( !str.IsEmpty() )
			OnUserOpenFile(str, 0);
		else
			OnFileNewBlank(0, 0, 0);
	} else {
		ATLASSERT(FALSE);
		OnFileNewBlank(0, 0, 0);
	}

	return 0;
}


//�󔒃y�[�W
void CMainFrame::OnFileNewBlank(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	OnUserOpenFile( _T("about:blank"), DonutGetStdOpenActivateFlag() );
	PostMessage(WM_COMMAND, ID_SETFOCUS_ADDRESSBAR, 0);
}


//�N���b�v�{�[�h
void CMainFrame::OnFileNewClipBoard(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	CString strText = MtlGetClipboardText();

	if ( strText.IsEmpty() )
		return;

	OnUserOpenFile( strText, DonutGetStdOpenActivateFlag() );
}


//Default.dfg
// UDT DGSTR
#if 0
LRESULT CMainFrame::OnFileOpenDef(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	CString 	strPath = Misc::GetExeDirectory() + _T("Default.dfg");
	ATLTRACE2( atlTraceGeneral, 4, _T("CMainFrame::OnFileOpenDef\n") );
	OnUserOpenFile( strPath, DonutGetStdOpenCreateFlag() );
	return 0;
}
#endif
// ENDE

LRESULT	CMainFrame::OnFileOpenTabList	(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	CString strPath = Misc::GetExeDirectory() + _T("TabList.xml");
	OnUserOpenFile( strPath, DonutGetStdOpenCreateFlag() );
	return 0;
}


void CMainFrame::OnFileNewClipBoard2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	CString strText = MtlGetClipboardText();

	if ( strText.IsEmpty() )
		return;

	OnUserOpenFile( strText, DonutGetStdOpenFlag() );	// allow the option
}



LRESULT CMainFrame::OnFileOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	//dmfTRACE( _T("CMainFrame::OnFileOpen\n") );
	COpenURLDlg dlg;

	if ( dlg.DoModal() == IDOK && !dlg.m_strEdit.IsEmpty() ) {
		OnUserOpenFile( dlg.m_strEdit, DonutGetStdOpenFlag() );
	}

	return 0;
}



/// undonut �̔z�z�T�C�g���J��
LRESULT CMainFrame::OnJumpToWebSite(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	//CString strSite = _T("http://www5.ocn.ne.jp/~minute/tab/"); //unDonut�Â���
	//CString strSite = _T("http://undonut.sakura.ne.jp/"); // unDonut�V������
	//CString strSite = _T("http://tekito.genin.jp/undonut+.html"); //unDonut+
	//CString strSite = _T("http://ichounonakano.sakura.ne.jp/64/undonut/"); //unDonut+mod.	��
	//CString strSite = _T("http://undonut.undo.jp/"); //unDonut+mod.
	CString strSite = _T("http://cid-8830de058eedff85.skydrive.live.com/browse.aspx/%e5%85%ac%e9%96%8b/unDonut"); //Risotto.

	OnUserOpenFile( strSite, DonutGetStdOpenFlag() );
	return 0;
}


///+++ .exe�̂���t�H���_���J��
LRESULT CMainFrame::OnOpenExeDir(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	CString		progDir = Misc::GetExeDirectory();
	ShellExecute(NULL, _T("open"), progDir, progDir, NULL, SW_SHOW);
	return 0;
}



#ifndef USE_DIET	//+++ saitama��p�������̂��O���t�@�C���Ǎ��ɂ��Ĕėp��.
HWND CMainFrame::_OpenAboutFile(CString strFile)
{
  #if 0	//+++ unDonut.ini [ETC] ��SAITAMA=1���`���ĂȂ��ꍇ�͋A��
	{
		CIniFileI pr( g_szIniFileName, _T("ETC") );
		DWORD	  dwFlag = pr.GetValue(_T("SAITAMA"));
		if (dwFlag == 0)
			return 0;
	}
  #endif

	if (strFile.Left(6) != _T("about:"))
		return NULL;
	strFile     = strFile.Mid(6);		// about:
	bool  bWarn = (strFile == "warning");
	if (bWarn == 0) {
		strFile     = Misc::GetFullPath_ForExe( "help\\about\\" + strFile );	//
		if (Misc::IsExistFile(strFile) == 0)	//�t�@�C�����Ȃ������炩����
			return NULL;
	}
	// HWND			hWndChild = OnUserOpenFile( _T("about:blank"), DonutGetStdOpenFlag() );
	HWND			hWndChild = OnUserOpenFile( _T("about:blank"), DonutGetStdOpenFlag() & ~D_OPENFILE_NOCREATE);	// about:�́A���ݑ��ɍ��Ƃ܂��������Ȃ�ŁA�K���ʑ��ɍ��悤�ɂ���.
	if ( !::IsWindow(hWndChild) )
		return NULL;

	CChildFrame*	pChild	  = GetChildFrame(hWndChild);
	if (!pChild)
		return hWndChild;

	CComPtr<IHTMLDocument2> pDoc;
	HRESULT 				hr;
	int 					ct		  = 0;

	while (1) {
		hr = pChild->m_spBrowser->get_Document( (IDispatch **) &pDoc );
		if (SUCCEEDED(hr) && pDoc) {
			break;
		}
		Sleep(100);

		ct++;
		if (ct > 50)
			return hWndChild;
	}

	//+++ CString		strText;		//+++ UNICODE�C���ŁA�ϊ�������stream�Ƃ��ď�������悤�ɕύX
	const char*			pText = NULL;
	unsigned			nSize = 0;
	std::vector<char>	vecText;
	if (bWarn) {	//+++ �x���̂Ƃ��́A�肻�[������ǂ�
		HRSRC	hRes = ::FindResource(_Module.GetModuleInstance(), MAKEINTRESOURCE(IDR_TEXT_WARNING/*IDR_TEXT_SAITAMA*/), _T("TEXT") );
		HGLOBAL hMem = ::LoadResource(_Module.GetModuleInstance(), hRes);
		pText 		 = (LPSTR) ::LockResource(hMem);
		nSize		 = strlen(pText);
	} else {		//+++ �ʏ�� help/about/���̃t�@�C����ǂݍ���
		Misc::FileLoad( strFile, vecText );
		pText  = &vecText[0];
		nSize  = vecText.size();
	}

	//+++ DWORD	nSize  = strText.GetLength() + 1;
	HGLOBAL 	hHTMLText = ::GlobalAlloc(GPTR, (nSize + 16/*�C����*/) );
	if (hHTMLText) {
		::ZeroMemory(hHTMLText, nSize + 16 );
		::CopyMemory(hHTMLText, pText, nSize);
		{
			CComPtr<IStream> pStream;
			hr = ::CreateStreamOnHGlobal(hHTMLText, TRUE, &pStream);
			if ( SUCCEEDED(hr) ) {
				// Call the helper function to load the browser from the stream.
				LoadWebBrowserFromStream( pChild->m_spBrowser , pStream  );
			}
		}
	  #if 0	//+++ �ǂ��� CreateStreamOnHGlobal�̑�������TRUE�Ȃ炻�����ŊJ�����Ă����悤�Ȃ̂ŁA����͕s�v�̂悤(����win64�ł̓n���O)
		::GlobalFree( hHTMLText );
	  #endif
	}

	/*SAFEARRAY *psfArray;
	   VARIANT *pparam;
	   psfArray = SafeArrayCreateVector(VT_VARIANT, 0, 1);
	   if(psfArray){
		hr = SafeArrayAccessData(psfArray,(LPVOID*) & pparam);
		pparam->vt = VT_BSTR;
		pparam->bstrVal = bstrSaitama;
		hr = SafeArrayUnaccessData(psfArray);
		hr = pDoc->write(psfArray);
		SafeArrayDestroy(psfArray);
	   }*/

	//m_AddressBar.GetEditCtrl().SetWindowText(strFile);
	//int index = m_MDITab.GetTabIndex(hWndChild);
	//m_MDITab.SetItemText(index,strFile);
	return hWndChild;
}
#endif



// =============================================================================================




struct _Function_Enum_ChildInfomation {
	CMainFrame*					m_pMainFrame;
	HWND						m_hWndActive;
	std::list<SDfgSaveInfo>&	m_rSaveInfoList;
	int 						m_nIndex;
	bool						m_bSaveFB;
	int 						m_nActiveIndex;

	// �R���X�g���N�^
	_Function_Enum_ChildInfomation(
			CMainFrame*					pMainFrame,
			HWND						hWndActive,
			std::list<SDfgSaveInfo>&	rSaveInfoList,
			int 						nIndex		= 0,
			bool						bSaveFB 	= FALSE)
		:	m_pMainFrame(pMainFrame)
		,	m_hWndActive(hWndActive)
		,	m_rSaveInfoList(rSaveInfoList)
		,	m_nIndex(nIndex)
		,	m_bSaveFB(bSaveFB)
		,	m_nActiveIndex(-1)
	{
	}

	void operator ()(HWND hWnd)
	{
		if (hWnd == m_hWndActive)
			m_nActiveIndex = m_nIndex;

		//3�ȏ�̈����𑗂�̂��ʓ|�Ȃ̂Œ���CChildFrame�𑀍�
		CChildFrame *pChild = m_pMainFrame->GetChildFrame(hWnd);
		if (!pChild) {
			ErrorLogPrintf(_T("dfg�̃Z�[�u�ł̏��擾�Ɏ��s\n"));
			return;
		}

		try {	//+++ �O�̂��ߗ�O�`�F�b�N.
			m_rSaveInfoList.push_back( SDfgSaveInfo() );
			pChild->view().m_ViewOption.GetDfgSaveInfo( m_rSaveInfoList.back(), m_bSaveFB );
		} catch(...) {
			ErrorLogPrintf(_T("dfg�̃Z�[�u���ɗ�O����\n"));
		}
		++m_nIndex;
	}
};


void	CMainFrame::SaveAllTab()
{
	/* ���ׂẴ^�u�̏����擾 */
	std::list<SDfgSaveInfo>	SaveInfoList;
	_Function_Enum_ChildInfomation	f(this, MDIGetActive(), SaveInfoList, 0, CMainOption::s_bTravelLogGroup);
	m_MDITab.ForEachWindow( boost::ref(f) );

	CString	TabList = Misc::GetExeDirectory() + _T("TabList.xml");

	if (::PathFileExists(TabList)) {
		CString strBakFile = Misc::GetFileNameNoExt(TabList) + _T(".bak.xml");
		::MoveFileEx(TabList, strBakFile, MOVEFILE_REPLACE_EXISTING);
	}

	try {
		CXmlFileWrite	xmlWrite(TabList);

		// <TabList>
		xmlWrite.WriteStartElement(L"TabList");
		CString strActiveIndex;
		strActiveIndex.Format(_T("%d"), f.m_nActiveIndex);
		xmlWrite.WriteAttributeString(L"ActiveIndex", strActiveIndex);

		std::list<SDfgSaveInfo>::iterator it = SaveInfoList.begin();
		for (; it != SaveInfoList.end(); ++it) {
			xmlWrite.WriteStartElement(L"Tab");
			xmlWrite.WriteAttributeString(L"title"		, it->m_title);
			xmlWrite.WriteAttributeString(L"url"		, it->m_location_URL);
			CString strDLCtrlFlags;
			strDLCtrlFlags.Format(_T("%d"), it->m_DL_Control_Flags);
			xmlWrite.WriteAttributeString(L"DLCtrlFlags", strDLCtrlFlags);
			CString	strExStyle;
			strExStyle.Format(_T("%d"), it->m_dwExStyle);
			xmlWrite.WriteAttributeString(L"ExStyle"	, strExStyle);
			CString strAutoRefreshStyle;
			strAutoRefreshStyle.Format(_T("%d"), it->m_dwAutoRefreshStyle);
			xmlWrite.WriteAttributeString(L"AutoRefreshStyle", strAutoRefreshStyle);

			xmlWrite.WriteStartElement(L"TravelLog");
			xmlWrite.WriteStartElement(L"Back");
			std::vector<std::pair<CString, CString> >::iterator itback = it->m_listBack.begin();
			for (; itback != it->m_listBack.end(); ++itback) {
				xmlWrite.WriteStartElement(L"item");
				xmlWrite.WriteAttributeString(L"title", itback->first);
				xmlWrite.WriteAttributeString(L"url"  , itback->second);
				xmlWrite.WriteFullEndElement();	// </item>
			}
			xmlWrite.WriteFullEndElement();	// </Back>

			xmlWrite.WriteStartElement(L"Fore");
			std::vector<std::pair<CString, CString> >::iterator itfore = it->m_listFore.begin();
			for (; itfore != it->m_listFore.end();++itfore) {
				xmlWrite.WriteStartElement(L"item");
				xmlWrite.WriteAttributeString(L"title", itfore->first);
				xmlWrite.WriteAttributeString(L"url"  , itfore->second);
				xmlWrite.WriteFullEndElement();	// </item>
			}
			xmlWrite.WriteFullEndElement();	// </Fore>
			xmlWrite.WriteFullEndElement();	// </TravelLog>

			xmlWrite.WriteFullEndElement();	// </Tab>
		}
	}
	catch (LPCTSTR strError) {
		MessageBox(strError);
	}
	catch (...) {
		MessageBox(_T("SaveAllTabList�Ɏ��s"));
	}
}



void	CMainFrame::RestoreAllTab(LPCTSTR strFilePath, bool bClose)
{
	/* default.dfg������΂�������g�� */
	CString	strFile = CStartUpOption::GetDefaultDFGFilePath();
	if (::PathFileExists(strFile)) {
		if ( MtlIsExt( strFile, _T(".dfg") ) ) {
			if ( !(CMainOption::s_dwMainExtendedStyle & MAIN_EX_NOCLOSEDFG) ) {
				_LoadGroupOption(strFile, true);
			} else {
				_LoadGroupOption(strFile, false);
			}
			CString strBakFile = Misc::GetFileNameNoExt(strFile) + _T(".bak.dfg");
			::MoveFileEx(strFile, strBakFile, MOVEFILE_REPLACE_EXISTING);
			return;
		}
	}

	int	nActiveIndex = 0;
	std::vector<SDfgSaveInfo>	vecSaveInfo;
	SDfgSaveInfo				Info;

	CString	TabList;
	if (strFilePath == NULL) {
		TabList = Misc::GetExeDirectory() + _T("TabList.xml");
	} else {
		TabList = strFilePath;
	}

	try {
		CXmlFileRead	xmlRead(TabList);
		CString 		Element;
		XmlNodeType 	nodeType;
		while (xmlRead.Read(&nodeType) == S_OK) {
			switch (nodeType) {
			case XmlNodeType_Element:
				{
					Element = xmlRead.GetLocalName();	// �v�f���擾
					if (Element == _T("TabList")) {
						if (xmlRead.MoveToFirstAttribute()) {
							if (xmlRead.GetLocalName() == _T("ActiveIndex")) {
								nActiveIndex = _ttoi(xmlRead.GetValue());
							}
						}
					}else if (Element == _T("Tab")) {
						if (xmlRead.MoveToFirstAttribute()) {
							do {
								CString strName = xmlRead.GetLocalName();
								if (strName == _T("title")) {
									Info.m_title = xmlRead.GetValue();
								} else if (strName == _T("url")) {
									Info.m_location_URL = xmlRead.GetValue();
								} else if (strName == _T("DLCtrlFlags")) {
									Info.m_DL_Control_Flags = _ttoi(xmlRead.GetValue());
								} else if (strName == _T("ExStyle")) {
									Info.m_dwExStyle = _ttoi(xmlRead.GetValue());
								} else if (strName == _T("AutoRefreshStyle")) {
									Info.m_dwAutoRefreshStyle = _ttoi(xmlRead.GetValue());
								}
							} while (xmlRead.MoveToNextAttribute());
						}
					} else if (Element == _T("Back")) {
						CString	strItem;
						while (xmlRead.GetInternalElement(_T("Back"), strItem)) {
							if (strItem == _T("item")) {
								CString title;
								CString url;
								if (xmlRead.MoveToFirstAttribute()) {
									title = xmlRead.GetValue();
									if (xmlRead.MoveToNextAttribute()) {
										url = xmlRead.GetValue();
										Info.m_listBack.push_back(std::make_pair(title, url));
									}
								}
							}
						}

					} else if (Element == _T("Fore")) {
						CString	strItem;
						while (xmlRead.GetInternalElement(_T("Fore"), strItem)) {
							if (strItem == _T("item")) {
								CString title;
								CString url;
								if (xmlRead.MoveToFirstAttribute()) {
									title = xmlRead.GetValue();
									if (xmlRead.MoveToNextAttribute()) {
										url = xmlRead.GetValue();
										Info.m_listFore.push_back(std::make_pair(title, url));
									}
								}
							}
						}
					}
					break;
				}
			case XmlNodeType_EndElement:
				{
					if (xmlRead.GetLocalName() == _T("Tab")) {
						// </Tab>�ɂ����̂�
						vecSaveInfo.push_back(Info);

						Info.m_listBack.clear();
						Info.m_listFore.clear();
					}
					break;
				}
			}
		}

		/////////////////////////////////////////////////////////////
		// �^�u�𕜌�����

		CLockRedrawMDIClient	 lock(m_hWndMDIClient);
		CDonutTabBar::CLockRedraw lock2(m_MDITab);
		CWaitCursor 			 cur;

		if (bClose) {
			MtlCloseAllMDIChildren(m_hWndMDIClient);
		}

		DWORD			dwCount 	= 0;
		bool			bActiveChildExistAlready = (MDIGetActive() != NULL);

		m_MDITab.InsertHere(true);
		int nInsertIndex = m_MDITab.GetItemCount();
		m_MDITab.SetInsertIndex(nInsertIndex);

		CChildFrame *	pChildActive = NULL;

		/* �^�u�ɒǉ����Ă��� */
		for (UINT i = 0; i < vecSaveInfo.size(); ++i) {
			CChildFrame *pChild = CChildFrame::NewWindow(m_hWndMDIClient, m_MDITab, m_AddressBar);
			if (pChild == NULL)
				continue;

			++nInsertIndex;
			m_MDITab.SetInsertIndex(nInsertIndex);

			// if tab mode, no need to load window placement.
			//pChild->view().m_ViewOption.GetProfile(strFileName, dw, !CMainOption::s_bTabMode);

			// DLCtrl��ݒ�
			pChild->view().PutDLControlFlags(vecSaveInfo[i].m_DL_Control_Flags);
			// Navigate����
			pChild->Navigate2(vecSaveInfo[i].m_location_URL);
			//
			pChild->view().m_ViewOption.m_dwExStyle	= vecSaveInfo[i].m_dwExStyle;

			pChild->view().m_ViewOption.m_dwAutoRefreshStyle = vecSaveInfo[i].m_dwAutoRefreshStyle;
			pChild->view().m_ViewOption.Init();	// �^�C�}�[���J�n

			// activate now!
			pChild->ActivateFrame(MDIGetActive() != NULL ? SW_SHOWNOACTIVATE : -1);

			/* �߂�E�i�ނ̍��ڂ�ݒ肷�� */
			if (CMainOption::s_bTravelLogGroup) {
				pChild->SetArrayHist(vecSaveInfo[i].m_listFore, vecSaveInfo[i].m_listBack);
			}

			if (i == nActiveIndex) {
				pChildActive = pChild;
			}
		}

		m_MDITab.InsertHere(false);
		m_MDITab.SetInsertIndex(-1);

		if (pChildActive == NULL)
			return;

		if (bActiveChildExistAlready == false) {						// there was no active window
			MDIActivate(pChildActive->m_hWnd);
			if (CMainOption::s_bTabMode) {	// in tab mode
				MDIMaximize(pChildActive->m_hWnd);
			}
		} else {												// already an active window exists
			if ( !(CMainOption::s_dwMainExtendedStyle & MAIN_EX_NOACTIVATE) )
				MDIActivate(pChildActive->m_hWnd);
		}

	}
	catch (LPCTSTR strError) {
		MessageBox(strError);
	}
	catch (...) {
		MessageBox(_T("RestoreAllTab�Ɏ��s"));
	}
}

//private:
//+++ �O���[�v�I�v�V�����̃Z�[�u��������������.
//+++ - �t�@�C���̔r����������₷�����邽�߁A�����W�ƁAini�t�@�C���ւ̏������݂̃^�C�~���O�𕪗�.
//+++ - �r�����䎩�̂�CIniFile�ɔC����.
//+++ - ���́A�q���̏����W�̎擾���܂ߕʃX���b�h���ŏ������Ă������A�ǂ������̕ӂ��܂���
//+++	���낢��s����̌��ɂȂ��Ă����͗l.�i"�i�ށE�߂�"��TravelLog�������ɁAie�R���|�[�l���g(dll)��
//+++	���Ńn���O���Ă��������j

// �^�u�P�̏����擾
struct CMainFrame::_Function_EnumChildSaveOption {
	CMainFrame *				m_pMainFrame;
	HWND						m_hWndActive;
	std::list<SDfgSaveInfo>&	m_rDfgSaveInfoList;
	int 						m_nIndex;
	bool						m_bSaveFB;
	int 						m_nActiveIndex;

	_Function_EnumChildSaveOption(
			CMainFrame *				pMainFrame,
			HWND						hWndActive,
			std::list<SDfgSaveInfo>&	rDrfSaveInfoList,
			int 						nIndex		= 0,
			bool						bSaveFB 	= FALSE)
		:	m_pMainFrame(pMainFrame)
		,	m_hWndActive(hWndActive)
		,	m_rDfgSaveInfoList(rDrfSaveInfoList)
		,	m_nIndex(nIndex)
		,	m_bSaveFB(bSaveFB)
		,	m_nActiveIndex(-1)
	{
	}

	void operator ()(HWND hWnd)
	{
		if (hWnd == m_hWndActive)
			m_nActiveIndex = m_nIndex;

		CString 	 strSection;
		strSection.Format(_T("Window%d"), m_nIndex);

		//3�ȏ�̈����𑗂�̂��ʓ|�Ȃ̂Œ���CChildFrame�𑀍�
		CChildFrame *pChild = m_pMainFrame->GetChildFrame(hWnd);
		if (!pChild) {
			ErrorLogPrintf(_T("dfg�̃Z�[�u�ł̏��擾��%s(�q�E�B���h�E%#x)�̎擾�Ɏ��s\n"), LPCTSTR(strSection), hWnd);
			return;
		}

		try {	//+++ �O�̂��ߗ�O�`�F�b�N.
			m_rDfgSaveInfoList.push_back( SDfgSaveInfo() );
			m_rDfgSaveInfoList.back().m_section = strSection;
			pChild->view().m_ViewOption.GetDfgSaveInfo( m_rDfgSaveInfoList.back(), m_bSaveFB );
		} catch(...) {
			ErrorLogPrintf(_T("dfg�̃Z�[�u��%s(�q�E�B���h�E%#x)�̃Z�[�u���ɗ�O����\n"), LPCTSTR(strSection), hWnd);
		}
		++m_nIndex;
	}
};


CMainFrame::CSaveGroupOptionToFile::CSaveGroupOptionToFile()
	:	m_bOldMaximized(0)
	,	m_bSaveTravelLog(0)
	,	m_nIndex(0)
	,	m_nActiveIndex(0)
	,	m_phThread(0)
	,	m_strFileName()
	,	m_dfgSaveInfoList()
{
}

CMainFrame::CSaveGroupOptionToFile::~CSaveGroupOptionToFile() {
	if (m_phThread && *m_phThread) {	//+++ �X���b�h�������Ă���ƁAm_dfgSaveInfoList�̃f�X�g���N�g���܂�������...
		DWORD dwResult = ::WaitForSingleObject(*m_phThread, DONUT_BACKUP_THREADWAIT);
		//x CloseHandle(*m_phThread);
	}
}


void CMainFrame::CSaveGroupOptionToFile::Run(CMainFrame* pMainWnd, const CString &strFileName, bool bDelay) // modified by minit
{
	if (pMainWnd == 0)
		return;
	if (pMainWnd->m_hGroupThread) {
		DWORD dwReturnCode = 0;
		::GetExitCodeThread(pMainWnd->m_hGroupThread, &dwReturnCode);
		if (dwReturnCode == STILL_ACTIVE) {
			ErrorLogPrintf(_T("defalut.dfg�̑O��̃Z�[�u�ɗ\�z�ȏ�Ɏ��Ԃ��������Ă���悤�Ȃ̂ō���͌�����.\n"));
			return;
		}
		//+++ ��U�I��.
		::CloseHandle(pMainWnd->m_hGroupThread);
		pMainWnd->m_hGroupThread = NULL;
	}

	m_phThread			= &pMainWnd->m_hGroupThread;
	m_strFileName		= strFileName;
	m_bOldMaximized		= FALSE;
	m_bSaveTravelLog	= CMainOption::s_bTravelLogGroup != 0/*? TRUE : FALSE*/;
	m_dfgSaveInfoList.clear();
	HWND	hWndActive	= pMainWnd->MDIGetActive(&m_bOldMaximized);

	//+++ �܂��͏��擾�����s��.
	_Function_EnumChildSaveOption	f(pMainWnd, hWndActive, m_dfgSaveInfoList, 0, m_bSaveTravelLog);

	try {	//+++ �Ӗ��Ȃ��Ȃ������ǔO�̂��ߗ�O�`�F�b�N.
		// f=
		pMainWnd->m_MDITab.ForEachWindow( boost::ref(f) );		// MtlForEachMDIChild(m_hWndMDIClient, f);
	} catch (...) {
		ErrorLogPrintf(_T("�O���[�v�I�v�V�����Z�[�u���ɗ�O����\n"));
	}

	m_nIndex		= f.m_nIndex;
	m_nActiveIndex	= f.m_nActiveIndex;

	if (bDelay == 0) {	//+++ ini�t�@�C���ւ̏������݂𓯊��ł���ꍇ.
		WriteIniFile();
	} else {			//+++ ini�t�@�C���ւ̏������݂�񓯊��ɂ���ꍇ.
		UINT	dwThreadID = 0;
		pMainWnd->m_hGroupThread = (HANDLE) _beginthreadex(NULL, 0, CSaveGroupOptionToFile::WriteIniFile_Thread, (LPVOID) this, 0, &dwThreadID);
	}
}

//private:
UINT __stdcall CMainFrame::CSaveGroupOptionToFile::WriteIniFile_Thread(void* pParam)
{
	CSaveGroupOptionToFile* pThis = (CSaveGroupOptionToFile*)pParam;
	pThis->WriteIniFile();
	_endthreadex(0);
	return 0;
}


void CMainFrame::CSaveGroupOptionToFile::WriteIniFile()
{
  #if 1	//+++ �j�P���̃A�C�f�A�ŁA�o�b�N�A�b�v�t�@�C������ .dfg.bak �łȂ� .bak.dfg �ɂ��Ă݂�.
	CString		strFileName	= m_strFileName;
	CString 	strBakName	= Misc::GetFileNameNoExt(strFileName) + ".bak.dfg";
	if (::GetFileAttributes(strBakName) != 0xFFFFFFFF)
		::DeleteFile(strBakName);					// �Â��o�b�N�A�b�v�t�@�C�����폜.
	if (::GetFileAttributes(strFileName) != 0xFFFFFFFF)
		::MoveFile(strFileName, strBakName);		// �����̃t�@�C�����o�b�N�A�b�v�t�@�C���ɂ���.
  #endif

	//+++ dfg�t�@�C���ɃZ�[�u
	CIniFileO	pr( m_strFileName, _T("Window") );
	//pr.RemoveFileToBak();			// strFileName ���폜(.bak��)

	//+++ �S�̂Ɋւ���ݒ��ۑ�
	pr.ChangeSectionName( _T("Header") );
	pr.SetValue( m_nIndex				, _T("count")	  );
	pr.SetValue( m_nActiveIndex 		, _T("active")	  );	// active index
	pr.SetValue( (m_bOldMaximized != 0)	, _T("maximized") );

	//+++ �����Ƃ̐ݒ��ۑ�
	for (std::list<SDfgSaveInfo>::iterator it = m_dfgSaveInfoList.begin();
			it != m_dfgSaveInfoList.end();
			++it)
	{
		it->WriteProfile(pr, m_bSaveTravelLog);
	}
}



// ===========================================================================

///+++
void CMainFrame::_SaveGroupOption(const CString &strFileName, bool bDelay /*=false*/)
{
	m_saveGroupOptionToFile.Run(this, strFileName, bDelay);

  #if 0	//+++ v1.48c �ō폜. ���p�x�̎����Z�[�u���ݒ肳��Ă���ƁA���ɂ���Ă̓p�t�H�[�}���X�������傫���݂����Ȃ̂�off.
	//x RtlSetMinProcWorkingSetSize();				//+++ ( �������̗\��̈���ꎞ�I�ɍŏ����B�E�B���h�E���ŏ��������ꍇ�Ɠ��� ) ... �Ђ���Ƃ���Ƃ��܂��낵���Ȃ���������...
  #endif
}



void CMainFrame::_LoadGroupOption(const CString &strFileName, bool bClose)
{
	//dmfTRACE( _T("CMainFrame::_LoadGroupOption\n") );

	CLockRedrawMDIClient	 lock(m_hWndMDIClient);
	CDonutTabBar::CLockRedraw lock2(m_MDITab);
	CWaitCursor 			 cur;

	if (bClose)
		MtlCloseAllMDIChildren(m_hWndMDIClient);

	DWORD			dwCount 	= 0;
  #if INISECTION_USE_MUTEX != 1
	CIniFileI		pr( strFileName, _T("Header") );
	if (pr.QueryValue( dwCount, _T("count") ) != ERROR_SUCCESS)
		return;
	DWORD			dwActive	= pr.GetValue( _T("active")   , 0 );
	DWORD			dwMaximized = pr.GetValue( _T("maximized"), 0 );
	pr.Close();
  #else
	CIniFileI		pr( strFileName, _T("Header") );
	if (pr.QueryValue( dwCount, _T("count") ) != ERROR_SUCCESS)
		return;
	DWORD			dwActive	= pr.GetValue( _T("active")   , 0 );
	DWORD			dwMaximized = pr.GetValue( _T("maximized"), 0 );
  #endif

	bool			bActiveChildExistAlready = (MDIGetActive() != NULL);

	m_MDITab.InsertHere(true);
	int nInsertIndex = m_MDITab.GetItemCount();
	m_MDITab.SetInsertIndex(nInsertIndex);

	CChildFrame *	pChildActive = NULL;
	for (DWORD dw = 0; dw < dwCount; ++dw) {
		CChildFrame *pChild = CChildFrame::NewWindow(m_hWndMDIClient, m_MDITab, m_AddressBar);
		// if no active child, as there is no client edge in MDI client window,
		// so GetClientRect is different a little and a resizing will occur when switching.
		// That is, only the first child window is activated.
		if (pChild == NULL)
			continue;

		++nInsertIndex;
		m_MDITab.SetInsertIndex(nInsertIndex);

		// activate now!
		pChild->ActivateFrame(MDIGetActive() != NULL ? SW_SHOWNOACTIVATE : -1);

		// if tab mode, no need to load window placement.
		pChild->view().m_ViewOption.GetProfile(strFileName, dw, !CMainOption::s_bTabMode);

		// �߂�E�i�ނ̍��ڂ�ݒ肷��
		if (CMainOption::s_bTravelLogGroup) {
			CString 	strSection;
			strSection.Format(_T("Window%d"), dw);
			_Load_OptionalData(pChild, strFileName, strSection);
		}

		if (dw == dwActive)
			pChildActive = pChild;
	}

	m_MDITab.InsertHere(false);
	m_MDITab.SetInsertIndex(-1);

	if (pChildActive == NULL)
		return;

	if (!bActiveChildExistAlready) {						// there was no active window
		MDIActivate(pChildActive->m_hWnd);
		if (CMainOption::s_bTabMode || dwMaximized == 1) {	// in tab mode
			MDIMaximize(pChildActive->m_hWnd);
		}
	} else {												// already an active window exists
		if ( !(CMainOption::s_dwMainExtendedStyle & MAIN_EX_NOACTIVATE) )
			MDIActivate(pChildActive->m_hWnd);
	}
}



// ===========================================================================

void CMainFrame::OnBackUpOptionChanged(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	if (m_nBackUpTimerID != 0) {
		KillTimer(m_nBackUpTimerID);
		m_nBackUpTimerID = 0;
	}

	if (CMainOption::s_dwMainExtendedStyle & MAIN_EX_BACKUP)
		m_nBackUpTimerID = SetTimer(1, 1000 * 60 * CMainOption::s_dwBackUpTime);
}


//private:
void CMainFrame::OnFavoriteGroupSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	if (MDIGetActive() == NULL)
		return;

	TCHAR		szOldPath[MAX_PATH];	// save current directory
	szOldPath[0]	= 0;	//+++
	::GetCurrentDirectory(MAX_PATH, szOldPath);

	CString dir = DonutGetFavoriteGroupFolder();
	::SetCurrentDirectory( LPCTSTR(dir) );

	const TCHAR szFilter[] = _T("Donut Favorite Group�t�@�C��(*.dfg)\0*.dfg\0\0");
	CFileDialog fileDlg(FALSE, _T("dfg"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);
	fileDlg.m_ofn.lpstrInitialDir = dir;
	fileDlg.m_ofn.lpstrTitle	  = _T("���C�ɓ���O���[�v�̕ۑ�");

	if (fileDlg.DoModal() == IDOK) {
		_SaveGroupOption(fileDlg.m_szFileName);
		::SendMessage(m_hWnd, WM_REFRESH_EXPBAR, 1, 0);
	}

	// restore current directory
	::SetCurrentDirectory(szOldPath);
}


void CMainFrame::OnFavoriteGroupAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	::SendMessage(MDIGetActive(), WM_COMMAND, (WPARAM) ID_FAVORITE_GROUP_ADD, 0);
}


void CMainFrame::OnFavoriteGroupOrganize(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	::ShellExecute(m_hWnd, NULL, DonutGetFavoriteGroupFolder(), NULL, NULL, SW_SHOWNORMAL);
}



#if 1 //+++ WS_CAPTION�������t���X�N���[���A�Ƃ����O����̂Ă��̂ŁAMtl�̊O���֐��������̂�������ֈړ�
template <class _Profile>
static void /*Mtl*/WriteProfileStatusBarState( _Profile &__profile, HWND hWndStatusBar, const CString &strPrefix = _T("statusbar.") )
{
	ATLASSERT( ::IsWindow(hWndStatusBar) );

	// �e���A�ޔ����͕ۑ����Ȃ�
	HWND hWndParent = ::GetParent(hWndStatusBar);
	if (hWndParent) {
		if (::IsWindowVisible(hWndParent) == FALSE)
			return;
		//if (IsFullScreen())
		if (g_pMainWnd->IsFullScreen())		//+++
			return;
	}
	LONG lRet		= __profile.SetValue( ::IsWindowVisible(hWndStatusBar) != 0, strPrefix + _T("Visible") );
	ATLASSERT(lRet == ERROR_SUCCESS);
}
#endif


void CMainFrame::_WriteProfile()
{
	// save frame and status bar
	CIniFileO	pr( g_szIniFileName, _T("Main") );
	MtlWriteProfileMainFrameState(pr, m_hWnd);
	/*Mtl*/WriteProfileStatusBarState(pr, m_hWndStatusBar);

	// save rebar
	pr.ChangeSectionName(_T("ReBar"));
	MtlWriteProfileReBarBandsState(pr, m_hWndToolBar);
	pr.Close();

  #if 0 //+++ OnClose�̂�����ςȂɍs���悤�ɕύX... //����ς肱����
	// save group options
	_SaveGroupOption( CStartUpOption::GetDefaultDFGFilePath() );
  #endif
}



// ====================================================================================

void CMainFrame::OnTimer(UINT_PTR nIDEvent, TIMERPROC dmy /*= 0*/)
{
	switch (nIDEvent) {
	case ENT_READ_ACCEL:		//+++ ����:�N�����݂̂́A�A�N�Z���L�[�ǂݍ���(�x���̂��߃^�C�}�[���g��)
		if (m_hAccel) {
			CAccelerManager 	accelManager(m_hAccel);
			m_hAccel = accelManager.LoadAccelaratorState(m_hAccel);
			::KillTimer(m_hWnd, nIDEvent);
		}
		break;

	default:
		//+++ �����Z�[�u���ݒ肳��Ă����ꍇ�Adefault.dfg���I�[�g�o�b�N�A�b�v.
		if (m_nBackUpTimerID == nIDEvent && m_nBackUpTimerID) {
			//_SaveGroupOption( CStartUpOption::GetDefaultDFGFilePath(), true );
			SaveAllTab();
		} else {
			SetMsgHandled(FALSE);
			return;
		}
		break;
	}
}



BOOL CMainFrame::OnIdle()
{
	if (m_bWM_TIMER/*IsIconic()*/)
		return TRUE;
	//return FALSE;
	// Note. under 0.01 sec (in dbg-mode on 330mhz cpu)
	CmdUIUpdateToolBars();
	CmdUIUpdateStatusBar	(m_hWndStatusBar, ID_DEFAULT_PANE	);
	CmdUIUpdateStatusBar	(m_hWndStatusBar, ID_SECURE_PANE	);
	CmdUIUpdateStatusBar	(m_hWndStatusBar, ID_PRIVACY_PANE	);
	CmdUIUpdateChildWindow	(m_hWndStatusBar, IDC_PROGRESS		);
	m_mcCmdBar.UpdateMDIMenuControl();
	//\\?_FocusChecker();
#if 0
	if ( _check_flag(MAIN_EX_KILLDIALOG, CMainOption::s_dwMainExtendedStyle) )
		CDialogKiller2::KillDialog();
#endif
	return FALSE;
}


void CMainFrame::_OnIdleCritical()
{
	if ( MtlIsApplicationActive(m_hWnd) ) {
		CmdUIUpdateToolBars();
		CmdUIUpdateChildWindow(m_hWndStatusBar, IDC_PROGRESS);
	}

	_FocusChecker();
#if 0
	if ( _check_flag(MAIN_EX_KILLDIALOG, CMainOption::s_dwMainExtendedStyle) )
		CDialogKiller2::KillDialog();
#endif
}


void CMainFrame::_FocusChecker()
{
	// Note. On idle time, give focus back to browser window...
	dmfTRACE( _T("_FocusChecker\n") );

	HWND hWndActive 				 = MDIGetActive();
	if (hWndActive == NULL)
		return;

	HWND hWndForeground 			 = ::GetForegroundWindow();
	bool bMysteriousWindowForeground = MtlIsKindOf( hWndForeground, _T("Internet Explorer_Hidden") );

	if ( bMysteriousWindowForeground && MtlIsWindowCurrentProcess(hWndForeground) ) {
		IfTrayRestoreWindow();							//+++ �g���C��Ԃ������畜��.
		if (m_bOldMaximized == 0 && m_bFullScreen == 0) //+++
			ShowWindow_Restore(1);		//ShowWindow(SW_RESTORE);		//+++ �T�C�Y��߂�
		MtlSetForegroundWindow(m_hWnd);
	}

	if ( !MtlIsApplicationActive(m_hWnd) ) {
		dmfTRACE(_T(" application is not active\n"), hWndForeground);
		return;
	}

	HWND hWndFocus					 = ::GetFocus();
	if ( hWndFocus != NULL && !MtlIsFamily(m_hWnd, hWndFocus) ) {
		dmfTRACE(_T(" not family(focus=%x)\n"), hWndFocus);
		return;
	}

	if ( ::IsChild(m_hWndToolBar, hWndFocus) ) {
		dmfTRACE( _T(" on rebar\n") );
		return;
	}

	if ( ::IsChild(m_ExplorerBar, hWndFocus) ) {
		dmfTRACE( _T(" on explorer bar\n") );
		return;
	}

	if ( hWndFocus == NULL || m_hWnd == hWndFocus || hWndActive == hWndFocus
	   || MtlIsKindOf( hWndFocus, _T("Shell DocObject View") ) )
	{
		dmfTRACE( _T(" Go!!!!!\n") );
		MtlSendCommand(hWndActive, ID_VIEW_SETFOCUS);
	}
}


// ���o�[�ƃr���[�̊Ԃ̉���������
void		CMainFrame::OnPaint(CDCHandle /*dc*/)
{
	SetMsgHandled(FALSE);

	// Constants
	enum _ReBarBorderConstants {
		s_kcxBorder = 2
	};

	CClientDC	dc(m_hWnd);

	CRect rc;
	::GetWindowRect(m_hWndToolBar, &rc);
	ScreenToClient(&rc);

	rc.InflateRect(s_kcxBorder, s_kcxBorder);
	dc.DrawEdge(rc, EDGE_ETCHED, BF_RECT);
}


// ===========================================================================

// UDT DGSTR
LRESULT CMainFrame::UpdateTitleBar(LPCTSTR lpszStatusBar, DWORD /*dwReserved*/)
{
	if ((GetStyle() & WS_CAPTION) != 0 && m_bTray == 0) {	//+++ �`�F�b�N�ǉ�:�^�C�g���o�[���\������Ă���Ƃ�
		if ( !::IsWindowVisible(m_hWndStatusBar) )			//+++ ����:�X�e�[�^�X�o�[���\������Ă��Ȃ�������
			UpdateTitleBarUpsideDown(lpszStatusBar);		//+++ ����:�^�C�g���o�[�ɁA�X�e�[�^�X�o�[��������o��...
	}
	return 0;
}
// ENDE


// UH -  minit
//------------------------
/// ���C�ɓ���̃��j���[�n���h����Ԃ�
LRESULT CMainFrame::OnMenuGetFav()
{
	return (LRESULT)m_FavoriteMenu.GetMenu().m_hMenu;
}

//-------------------------
/// ���C�ɓ���O���[�v�̃��j���[�n���h����Ԃ�
LRESULT CMainFrame::OnMenuGetFavGroup()
{
	return (LRESULT)m_FavGroupMenu.GetMenu().m_hMenu;
}

//-------------------------
/// ���[�U�[�X�N���v�g�̃��j���[�n���h����Ԃ�
LRESULT CMainFrame::OnMenuGetScript()
{
	if ( !m_DropScriptMenu.GetMenu().IsMenu() ) {
		CMenuHandle menu;
		menu.LoadMenu(IDR_DROPDOWN_SCRIPT);
		m_DropScriptMenu.SetRootDirectoryPath( Misc::GetExeDirectory() + _T("Script") );
		m_DropScriptMenu.SetTargetWindow(m_hWnd);
		m_DropScriptMenu.InstallExplorerMenu(menu);
		m_DropScriptMenu.RefreshMenu();
	}

	return (LRESULT) m_DropScriptMenu.GetMenu().m_hMenu;
}


LRESULT CMainFrame::OnMenuGoBack(HMENU hMenu)
{
	HWND hMDIActive = MDIGetActive();

	return ::SendMessage(hMDIActive, WM_MENU_GOBACK, (WPARAM) (HMENU) hMenu, (LPARAM) 0);
}


LRESULT CMainFrame::OnMenuGoForward(HMENU hMenu)
{
	HWND hMDIActive = MDIGetActive();

	return ::SendMessage(hMDIActive, WM_MENU_GOFORWARD, (WPARAM) (HMENU) hMenu, (LPARAM) 0);
}

LRESULT	CMainFrame::OnMenuGetBingTranslate()
{
	return (LRESULT)m_TranslateMenu.GetMenu().m_hMenu;
}


LRESULT CMainFrame::OnMenuRefreshScript(BOOL bInit)
{
	if ( m_DropScriptMenu.GetMenu().IsMenu() ) {
		if (bInit == FALSE) {
			m_DropScriptMenu.GetMenu().DestroyMenu();
			m_bShow = FALSE;
		} else {
			m_bShow = TRUE;
		}
	}

	return S_OK;
}


LRESULT CMainFrame::OnRestrictMessage(BOOL bOn)
{
	m_bShow = bOn;
	return S_OK;
}

// ���C�ɓ��胁�j���[���烊���N�̃p�X��Ԃ�
LRESULT	CMainFrame::OnGetFavoriteFilePath(int nID)
{
	return (LRESULT)(LPCTSTR)m_FavoriteMenu.GetFilePath(nID);
}


LRESULT CMainFrame::OnMenuDrag(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	return m_wndMenuDropTarget.OnMenuDrag(uMsg, wParam, lParam, bHandled);
}



LRESULT CMainFrame::OnMenuGetObject(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	return m_wndMenuDropTarget.OnMenuGetObject(uMsg, wParam, lParam, bHandled);
}



//////////////////////////////////////////////////////////////////
// to avoid the flicker on resizing

BOOL CMainFrame::OnMDIClientEraseBkgnd(CDCHandle dc)
{
  #if 0 //+++ �`��͈͂̍X�V�������Ɛ���ł��Ă��Ȃ��̂őʖ�
	BOOL	bMaximized = 0;
	HWND	hWndActive = MDIGetActive(&bMaximized);
	if (hWndActive != NULL && (CMainOption::s_bTabMode || bMaximized))
		return 1;
  #else
	HWND	hWndActive = MDIGetActive();
	if (hWndActive != NULL && CMainOption::s_bTabMode)
		return 1;
  #endif

  #if 1 //*+++ BG�`��w��.
	if (drawMainFrameBg(dc))
		return 1;
  #endif

	// no need to erase it
	SetMsgHandled(FALSE);
	return 0;
}


void CMainFrame::OnMDIClientSize(UINT nType, CSize size)
{
	PostMessage(WM_COMMAND, MAKEWPARAM(ID_RESIZED, 0), 0);

	BOOL	bMaximized = FALSE;
	HWND	hWndActive = MDIGetActive(&bMaximized);

//	if (m_bFullScreen) {
//		::MoveWindow(hWndActive, 0, 0, size.cx, size.cy, TRUE);
//		return;
//	}

	if (CMainOption::s_bTabMode || hWndActive == NULL || bMaximized == FALSE) { // pass to default
		SetMsgHandled(FALSE);
		return ;
	}

	// NOTE. If you do the following, you can avoid the flicker on resizing.
	//		 I can't understand why it is effective...
	//
	//		 But still you can get a glimpse of other mdi child window's frames.
	//		 I guess it's MDI's bug, but I can't get the way MFC fixed.
	CWindow wndActive(hWndActive);
	CWindow wndView    = wndActive.GetDlgItem(ID_DONUTVIEW);
	wndActive.ModifyStyle(0, WS_CLIPCHILDREN);									// add WS_CLIPCHILDREN
	wndView.ModifyStyle(0, WS_CLIPCHILDREN);
	LRESULT lRet	   = m_wndMDIClient.DefWindowProc();
	m_wndMDIClient.UpdateWindow();
	wndActive.ModifyStyle(WS_CLIPCHILDREN, 0);
	wndView.ModifyStyle(WS_CLIPCHILDREN, 0);

	SetMsgHandled(FALSE);
	return ;
}


//////////////////////////////////////////////////////////////////
// custom draw of addressbar
LRESULT CMainFrame::OnNotify(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	bHandled = FALSE;

	LPNMHDR lpnmh = (LPNMHDR) lParam;
	LRESULT lRet  = 0;

	//if (lpnmh->code == NM_CUSTOMDRAW && lpnmh->hwndFrom == m_hWndToolBar) { 	// from rebar
	//	lRet = m_AddressBar.OnCustomDraw(0, lpnmh, bHandled);
	//}

	if (lpnmh->code == TBN_DROPDOWN) {
		int nCount = CPluginManager::GetCount(PLT_TOOLBAR);

		for (int nIndex = 0; nIndex < nCount; nIndex++) {
			HWND	hWndBar = CPluginManager::GetHWND(PLT_TOOLBAR, nIndex);

			if (lpnmh->hwndFrom != hWndBar)
				continue;

			CPluginManager::Call_ShowToolBarMenu(PLT_TOOLBAR, nIndex, ( (LPNMTOOLBAR) lpnmh )->iItem);

			bHandled = TRUE;
			break;
		}
	}

	return lRet;
}


//////////////////////////////////////////////////////////////////
// the custom message from MDI child
void CMainFrame::OnMDIChild(HWND hWnd, UINT nCode)
{
	SetMsgHandled(FALSE);

	if (nCode == MDICHILD_USER_ALLCLOSED) {
		m_AddressBar.SetWindowText( _T("") );
		::SetWindowText( m_hWndStatusBar, _T("���f�B") );
		m_CmdBar.EnableButton(_nPosWindowMenu, false);
	} else if (nCode == MDICHILD_USER_FIRSTCREATED) {
		m_CmdBar.EnableButton(_nPosWindowMenu, true);
	} else if (nCode == MDICHILD_USER_ACTIVATED) {
		_OnMDIActivate(hWnd);
		OnIdle();						// make sure when cpu is busy
	} else if (nCode == MDICHILD_USER_TITLECHANGED) {
		//OnTitleChanged();
	}
}


BOOL CMainFrame::OnBrowserCanSetFocus()
{										// asked by browser
	HWND hWndFocus = ::GetFocus();

	if (hWndFocus == NULL)
		return TRUE;

	if ( ::IsChild(m_hWndToolBar, hWndFocus) )
		return FALSE;

	if ( m_ExplorerBar.IsChild(hWndFocus) )
		return FALSE;

	return TRUE;
}


void CMainFrame::OnParentNotify(UINT fwEvent, UINT idChild, LPARAM lParam)
{
	if (fwEvent != WM_RBUTTONDOWN)
		return;

	CPoint		pt( GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) );
	ClientToScreen(&pt);

	CRect		rc;
	::GetClientRect(m_hWndToolBar, &rc);
	CPoint		ptRebar  = pt;
	::ScreenToClient(m_hWndToolBar, &ptRebar);

	if ( !rc.PtInRect(ptRebar) )		// not on rebar
		return;
#if 0
	HWND		hWnd	 = ::WindowFromPoint(pt);

	if (hWnd == m_MDITab.m_hWnd) {		// on tab bar
		CPoint ptTab = pt;
		m_MDITab.ScreenToClient(&ptTab);

		if (m_MDITab.HitTest(ptTab) != -1)
			return;
	}
#endif
	CMenuHandle menuView = ::GetSubMenu(m_CmdBar.GetMenu(), _nPosViewMenu);
	CMenuHandle menu	 = menuView.GetSubMenu(0);
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd, NULL);
}


void CMainFrame::OnActivate(UINT nState, BOOL bMinimized, HWND hWndOther)
{
	if (nState == WA_INACTIVE) {
		m_hWndFocus = ::GetFocus();
	} else {
		if (m_hWndFocus)
			::SetFocus(m_hWndFocus);
	}
}


//minit
BOOL CMainFrame::_IsRebarBandLocked()
{
	CReBarCtrl	  rebar(m_hWndToolBar);
	REBARBANDINFO rbbi = { 0 };

	rbbi.cbSize = sizeof (REBARBANDINFO);
	rbbi.fMask	= RBBIM_STYLE;

	if ( !rebar.GetBandInfo(0, &rbbi) )
		return FALSE;

	return (rbbi.fStyle & RBBS_NOGRIPPER) != 0;
}

// �c�[���o�[�Ȃǂ����b�N(�Œ�)����
LRESULT CMainFrame::OnViewToolBarLock(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	m_ReBar.LockBands(!_IsRebarBandLocked());
	//CReBarSupport RebarSup(m_hWndToolBar);

	//RebarSup.SetBandsLock( !_IsRebarBandLocked() );

	return 0;
}


void CMainFrame::OnExplorerBarAutoShow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	if (CMainOption::s_dwExplorerBarStyle & MAIN_EXPLORER_AUTOSHOW)
		CMainOption::s_dwExplorerBarStyle &= ~MAIN_EXPLORER_AUTOSHOW;
	else
		CMainOption::s_dwExplorerBarStyle |= MAIN_EXPLORER_AUTOSHOW;
}


void CMainFrame::_RefreshFavMenu()
{
	m_FavoriteMenu.RefreshMenu();
	m_FavGroupMenu.RefreshMenu();
	m_styleSheetMenu.RefreshMenu();

	m_ExplorerBar.RefreshFavBar();
}


//LRESULT CMainFrame::OnViewIdle(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
//{
//	_OnIdleCritical();
//	return 0;
//}


////////////////////////////////////////////////////////////////////////////////
//�t�@�C�����j���[
//�@�R�}���h�n���h��
////////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnViewHome(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	bool bNew = ::GetAsyncKeyState(VK_CONTROL) < 0 || ::GetAsyncKeyState(VK_SHIFT) < 0;

	if (bNew || MDIGetActive() == NULL) {
		OnFileNewHome(0, 0, 0);
	} else {
		SetMsgHandled(FALSE);
	}
}


void CMainFrame::OnResized(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	if (m_wndStatusBar.m_hWnd == NULL)
		return;
#if 0	// CDonutStatusBarCtrl�Ɉړ�
	//+++ �����F�X�e�[�^�X�o�[�̃v���N�V�E�R���{�{�b�N�X�̃��T�C�Y
	{
		CRect	rcComboPart;
		if (g_bSwapProxy == false)
			m_wndStatusBar.GetRect(4, rcComboPart);
		else
			m_wndStatusBar.GetRect(1, rcComboPart);

		rcComboPart.top   -= 1;
		rcComboPart.bottom = rcComboPart.top + rcComboPart.Height() * 10;
		m_cmbBox.MoveWindow(rcComboPart, TRUE);
	}
#endif
	{
		//�v���O�C���C�x���g - ���T�C�Y
		CPluginManager::BroadCast_PluginEvent(DEVT_CHANGESIZE, 0, 0);
	}
}


// U.H
void CMainFrame::ShowLinkText(BOOL bShow)
{
	REBARBANDINFO rbBand = { sizeof (REBARBANDINFO) };

	int 		  nCount = int ( ::SendMessage(m_hWndToolBar, RB_GETBANDCOUNT, 0, 0) );

	for (int ii = 0; ii < nCount; ii++) {
		rbBand.fMask = RBBIM_ID;
		::SendMessage(m_hWndToolBar, RB_GETBANDINFO, (WPARAM) ii, (LPARAM) (LPREBARBANDINFO) &rbBand);

		if (rbBand.wID != IDC_LINKBAR && rbBand.wID != IDC_SEARCHBAR && rbBand.wID != IDC_ADDRESSBAR)
			continue;

		rbBand.fMask = RBBIM_TEXT;

		if (bShow) {
			switch (rbBand.wID) {
			case IDC_ADDRESSBAR:
				rbBand.lpText = _T("�A�h���X");
				break;

			case IDC_LINKBAR:
				rbBand.lpText = _T("�����N");
				break;

			default:
				rbBand.lpText = _T("����");
			}
		}

		::SendMessage(m_hWndToolBar, RB_SETBANDINFO, (WPARAM) ii, (LPARAM) (LPREBARBANDINFO) &rbBand);
	}
}
// END



// U.H
void CMainFrame::OnFileNewSelectText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	SendMessage(WM_COMMAND, MAKEWPARAM(ID_EDIT_COPY, 0), 0);
	SendMessage(WM_COMMAND, MAKEWPARAM(ID_FILE_NEW_CLIPBOARD, 0), 0);
}


// U.H
void CMainFrame::OnShowTextChg(BOOL bShow)
{
	ShowLinkText(bShow);
	m_AddressBar.ShowAddresText(m_hWndToolBar, bShow);
}


// U.H
LRESULT CMainFrame::OnSysCommand(UINT nID, CPoint point)
{
	switch (nID) {
	case ID_VIEW_COMMANDBAR:
		SendMessage(m_hWnd, WM_COMMAND, ID_VIEW_COMMANDBAR, 0);
		SetMsgHandled(TRUE);
		break;

  #if 1	//+++ �ŏ����{�^�������������ɁA�^�X�N�g���C�ɓ���悤�ɂ��Ă݂�.
	case SC_MINIMIZE:
		if ((CMainOption::s_dwMainExtendedStyle2 & MAIN_EX2_MINBTN2TRAY)	//+++ �ŏ����{�^���Ń^�X�N�g���C�ɓ����ݒ�̂Ƃ��A
			&& (point.x || point.y)											//+++ x,y��0,0�Ȃ�^�X�N�o�[�ŃN���b�N�����ꍇ���낤�ŁA�g���C�ɂ��ꂸ�A�ŏ����������Ă݂�.
		) {
			OnGetOut(0,0,0);
			SetMsgHandled(TRUE);
			break;
		}
		{
			CChildFrame* pChild = GetActiveChildFrame();
			if (pChild)	//�ŏ�������Ƃ��Ƀr���[�Ƀt�H�[�J�X�𓖂Ă�(���������CPU�g�p�����Ȃ���������悤�Ȃ̂�)
				pChild->SetFocus();
		}
		SetMsgHandled(FALSE);
		break;

	case SC_CLOSE:
		if ((CMainOption::s_dwMainExtendedStyle2 & MAIN_EX2_CLOSEBTN2TRAY)	//+++ �ŏ����{�^���Ń^�X�N�g���C�ɓ����ݒ�̂Ƃ��A
			&& (point.x || point.y)											//+++ x,y��0,0�Ȃ�^�X�N�o�[�ŃN���b�N�����ꍇ���낤�ŁA�g���C�ɂ��ꂸ�A�ŏ����������Ă݂�.
		) {
			OnGetOut(0,0,0);
			SetMsgHandled(TRUE);
			break;
		}
		SetMsgHandled(FALSE);
		break;
  #endif

	default:
		SetMsgHandled(FALSE);
		break;
	}

	return 0;
}


LRESULT CMainFrame::OnPopupClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	CIgnoredURLsOption::s_bValid = !CIgnoredURLsOption::s_bValid;
	return 0;
}


LRESULT CMainFrame::OnTitleClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	CCloseTitlesOption::s_bValid = !CCloseTitlesOption::s_bValid;
	return 0;
}


LRESULT CMainFrame::OnDoubleClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	CIgnoredURLsOption::s_bValid = !CIgnoredURLsOption::s_bValid;
	CCloseTitlesOption::s_bValid = !CCloseTitlesOption::s_bValid;
	return 0;
}


LRESULT CMainFrame::OnPrivacyReport(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	HWND	hWndActive = MDIGetActive();
	if (hWndActive == NULL)
		return 0;

	CWebBrowser2	browser    = DonutGetIWebBrowser2(hWndActive);
	CString 		strURL	   = browser.GetLocationURL();
	CComBSTR		bstrURL(strURL);

	CComPtr<IServiceProvider>		spSP;
	browser.m_spBrowser->QueryInterface(IID_IServiceProvider, (void **) &spSP);

	if (spSP == NULL)
		return 0;

	CComPtr<IEnumPrivacyRecords>	spEnumRecords;
	spSP->QueryService(CLSID_IEnumPrivacyRecords, &spEnumRecords);

	// UH
	// spSP->QueryInterface(IID_IEnumPrivacyRecords, (void**) &spEnumRecords);
	// spSP->QueryInterface(CLSID_IEnumPrivacyRecords, (void**) &spEnumRecords);
	if (spEnumRecords == NULL)
		return 0;

	HINSTANCE		hInstDLL;
	DWORD			(WINAPI * __DoPrivacyDlg)(HWND, LPOLESTR, IEnumPrivacyRecords *, BOOL) = NULL;

	if (Misc::getIEMejourVersion() >= 8 && _CheckOsVersion_VistaLater() == 0){//\\ XP+IE8�̏ꍇ
		hInstDLL = ::LoadLibrary( _T("ieframe.dll") );
	} else {//vista+IE8�̏ꍇ
		hInstDLL = ::LoadLibrary( _T("shdocvw.dll") );
	}

	if ( hInstDLL == NULL )
		return 0;

	__DoPrivacyDlg	= ( DWORD (WINAPI *)(HWND, LPOLESTR, IEnumPrivacyRecords *, BOOL) )
						GetProcAddress( hInstDLL, "DoPrivacyDlg" );
	if (__DoPrivacyDlg == NULL)
		return 0;

	BOOL	bPrivacyImpacted = FALSE;
	spEnumRecords->GetPrivacyImpacted(&bPrivacyImpacted);
	__DoPrivacyDlg(m_hWnd, bstrURL, spEnumRecords, !bPrivacyImpacted);

	//\\ ������Y��Ă�H
	::FreeLibrary( hInstDLL );

	return 1;
}


// UH
LRESULT CMainFrame::OnCookiesIE6(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/)
{
	DWORD	  dwLv = 0;

	switch (wID) {
	case ID_URLACTION_COOKIES_BLOCK:	dwLv = PRIVACY_TEMPLATE_NO_COOKIES; 	break;
	case ID_URLACTION_COOKIES_HI:		dwLv = PRIVACY_TEMPLATE_HIGH;			break;
	case ID_URLACTION_COOKIES_MIDHI:	dwLv = PRIVACY_TEMPLATE_MEDIUM_HIGH;	break;
	case ID_URLACTION_COOKIES_MID:		dwLv = PRIVACY_TEMPLATE_MEDIUM; 		break;
	case ID_URLACTION_COOKIES_LOW:		dwLv = PRIVACY_TEMPLATE_MEDIUM_LOW; 	break;
	case ID_URLACTION_COOKIES_ALL:		dwLv = PRIVACY_TEMPLATE_LOW;			break;
	}

	HINSTANCE	hInstDLL;
	DWORD		(WINAPI * __PrivacySetZonePreferenceW)(DWORD, DWORD, DWORD, LPCWSTR) = NULL;

	if ( ( hInstDLL = LoadLibrary(_T("wininet.dll")) ) == NULL )
		return 0;

	__PrivacySetZonePreferenceW = ( DWORD (WINAPI *)(DWORD, DWORD, DWORD, LPCWSTR) )
										GetProcAddress( hInstDLL, "PrivacySetZonePreferenceW" );

	if (__PrivacySetZonePreferenceW == NULL)
		return 0;

	__PrivacySetZonePreferenceW(URLZONE_INTERNET, PRIVACY_TYPE_FIRST_PARTY, dwLv, NULL);
	__PrivacySetZonePreferenceW(URLZONE_INTERNET, PRIVACY_TYPE_THIRD_PARTY, dwLv, NULL);
	return 0;
}


LRESULT CMainFrame::OnWindowCloseAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	if ( !CDonutConfirmOption::OnCloseAll(m_hWnd) )
		return 0;

	CWaitCursor 		 cur;
	CLockRedrawMDIClient lock(m_hWndMDIClient);
	CDonutTabBar::CLockRedraw 		 lock2(m_MDITab);
	MtlCloseAllMDIChildren(m_hWndMDIClient);

	RtlSetMinProcWorkingSetSize();		//+++ ( �������̗\��̈���ꎞ�I�ɍŏ����B�E�B���h�E���ŏ��������ꍇ�Ɠ��� )
	return 0;
}



LRESULT CMainFrame::OnWindowCloseCmp(int nTarIndex, BOOL bLeft)
{
	HWND			   hWndActive = MDIGetActive();

	if (hWndActive == NULL)
		return 0;

	CSimpleArray<HWND> arrWnd;
	int 			   ii;

	for (ii = 0; ii < m_MDITab.GetItemCount(); ii++) {
		HWND hWnd = m_MDITab.GetTabHwnd(ii);

		if (bLeft && ii < nTarIndex)
			arrWnd.Add(hWnd);
		else if (bLeft == FALSE && ii > nTarIndex)
			arrWnd.Add(hWnd);
	}

	for (ii = 0; ii < arrWnd.GetSize(); ii++) {
		SendMessage(arrWnd[ii], WM_CLOSE, 0, 0);
	}

	return 1;
}


#ifndef NO_STYLESHEET
LRESULT CMainFrame::OnChangeCSS(LPCTSTR lpszStyleSheet)
{
	if (CStyleSheetOption::s_bSetUserSheet)
		CStyleSheetOption::SetUserSheetName(lpszStyleSheet);

	//minit

	HWND hWndActive = MDIGetActive();

	if (hWndActive == NULL)
		return 0;

	return SendMessage(hWndActive, WM_USER_CHANGE_CSS, (WPARAM) lpszStyleSheet, 0);
}
#endif


// U.H
// ���j���[�̉E�[�ɃV���[�g�J�b�g�L�[��\������悤�ɐݒ肷��
LRESULT CMainFrame::OnInitMenuPopup(HMENU hMenuPopup, UINT uPos, BOOL bSystemMenu)
{
	// �V�X�e�����j���[�́A�������Ȃ�
	if (bSystemMenu)
		return 0;
	
	if (   hMenuPopup == m_FavoriteMenu.GetMenu().m_hMenu 
		|| hMenuPopup == m_FavGroupMenu.GetMenu().m_hMenu 
		|| hMenuPopup == m_styleSheetMenu.GetMenu().m_hMenu
		|| m_FavoriteMenu.IsSubMenu(hMenuPopup) )
		return 0;

	CMenuHandle 	menu = hMenuPopup;
	CAccelerManager accel(m_hAccel);

	// ��Ԗڂ̃��j���[�������Ȃ����͂���������ĕҏW���Ȃ�
	CString 		strCmd;

	if (menu.GetMenuString(0, strCmd, MF_BYPOSITION) == 0) {
		menu.RemoveMenu(0, MF_BYPOSITION);
		return 0;
	}

	if (CMenuOption::s_bNoCustomMenu) return 0;

	for (int ii = 0; ii < menu.GetMenuItemCount(); ii++) {
		strCmd = _T("");
		UINT	nID    = menu.GetMenuItemID(ii);
		menu.GetMenuString(nID, strCmd, MF_BYCOMMAND);

		if ( strCmd.IsEmpty() ) {
			// NT4�p����
			if (strCmd.LoadString(nID) == FALSE)
				continue;
		}

		CString strShorCut;

		if ( !accel.FindAccelerator(nID, strShorCut) )	//�t�ɂȂ��Ă��̂ŏC�� minit
			continue;

		if (strCmd.Find(_T("\t")) != -1) {
			strCmd = strCmd.Left( strCmd.Find(_T("\t")) );
		}

		if (strShorCut.IsEmpty() == FALSE)
			strCmd = strCmd + _T("\t") + strShorCut;

		UINT	uState = menu.GetMenuState(nID, MF_BYCOMMAND);
		menu.ModifyMenu(nID, MF_BYCOMMAND, nID, strCmd);

		if (uState & MF_CHECKED)
			menu.CheckMenuItem(nID, MF_CHECKED);

		if (uState & MF_GRAYED)
			menu.EnableMenuItem(nID, MF_GRAYED);
	}

	return 0;
}


void CMainFrame::OnRegisterAsBrowser(WORD wNotifyCode, WORD /*wID*/, HWND /*hWndCtl*/)
{
	if (wNotifyCode == NM_ON) {
		MtlForEachMDIChild( m_hWndMDIClient, CSendCommand(ID_REGISTER_AS_BROWSER, NM_ON) );
	} else if (wNotifyCode == NM_OFF) {
		MtlForEachMDIChild( m_hWndMDIClient, CSendCommand(ID_REGISTER_AS_BROWSER, NM_OFF) );
	} else if ( !_check_flag(MAIN_EX_REGISTER_AS_BROWSER, CMainOption::s_dwMainExtendedStyle) ) {
		CMainOption::s_dwMainExtendedStyle |= MAIN_EX_REGISTER_AS_BROWSER;
		MtlForEachMDIChild( m_hWndMDIClient, CSendCommand(ID_REGISTER_AS_BROWSER, NM_ON) );
	} else {
		CMainOption::s_dwMainExtendedStyle &= ~MAIN_EX_REGISTER_AS_BROWSER;
		MtlForEachMDIChild( m_hWndMDIClient, CSendCommand(ID_REGISTER_AS_BROWSER, NM_OFF) );
	}
}

// �摜��ۑ�����
void	CMainFrame::OnSaveImage(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	auto funcGetUrl = [](CComQIPtr<IDispatch> spDisp) -> LPCTSTR {
		CComBSTR strUrl;
		CComQIPtr<IHTMLImgElement>	spImage =  spDisp;
		if (spImage != NULL) {
			spImage->get_href(&strUrl);
		} else {
			CComQIPtr<IHTMLInputElement>	spInput = spDisp;
			if (spInput != NULL) {
				spInput->get_src(&strUrl);
			} else {
				CComQIPtr<IHTMLAreaElement>	spArea = spDisp;
				if (spArea != NULL) {
					spArea->get_href(&strUrl);
				}
			}
		}
		return strUrl;
	};

	CCustomContextMenu* pMenu = CCustomContextMenu::GetInstance();
	if (pMenu) {
#if 0	// �f�o�b�O�p
		CComQIPtr<IHTMLElement>	spElement = pMenu->GetDispatch();
		if (spElement)  {
			CComBSTR str;
			spElement->get_tagName(&str);
			DEBUGPUT(_T("%s\n"), str);
			spElement->get_outerHTML(&str);
			DEBUGPUT(_T("%s\n"), str);
		}
#endif
		CString strUrl = funcGetUrl(pMenu->GetDispatch());

		if (strUrl.IsEmpty() == FALSE)
			m_DownloadManager.DownloadStart(strUrl, NULL, NULL, DLO_SAVEIMAGE);
	} else {
		CChildFrame*	pChild = GetActiveChildFrame();
		if (pChild) {
			CComPtr<IDispatch>	spDisp;
			pChild->m_spBrowser->get_Document(&spDisp);
			CComQIPtr<IHTMLDocument2>	spDocument = spDisp;
			ATLASSERT(spDocument);
			CPoint	pt;
			::GetCursorPos(&pt);
			pChild->ScreenToClient(&pt);
			CComPtr<IHTMLElement>	spHitElement;
			spDocument->elementFromPoint(pt.x, pt.y, &spHitElement);
			// Hit�悪�摜�̏ꍇ
			CComQIPtr<IHTMLImgElement>	spImg = spHitElement;
			if (spImg == NULL) {
				// �t���[���̉\��
				CComQIPtr<IHTMLFrameElement3>	spFrame = spHitElement;
				if (spFrame) {
					LONG x, y;
					spHitElement->get_offsetTop(&y);
					spHitElement->get_offsetLeft(&x);
					pt.Offset(-x, -y);
					CComPtr<IDispatch>	spFrameDisp;
					spFrame->get_contentDocument(&spFrameDisp);
					CComQIPtr<IHTMLDocument2>	spFrameDocument = spFrameDisp;
					if (spFrameDocument == NULL) 
						return;	// �ʃh���C��
					ATLASSERT(spFrameDocument);
					CComPtr<IHTMLElement>	spHitFrameElement;
					spFrameDocument->elementFromPoint(pt.x, pt.y, &spHitFrameElement);
					spImg = spHitFrameElement;
				}
			}
			if (spImg) {
				CString strUrl = funcGetUrl(spImg.p);
				if (strUrl.IsEmpty() == FALSE)
					m_DownloadManager.DownloadStart(strUrl, NULL, NULL, DLO_SAVEIMAGE);
			}

		}
	}
}



void CMainFrame::OnFileWorkOffline(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	MtlSetGlobalOffline( !MtlIsGlobalOffline() );
	UpdateTitleBarUpsideDown();
}


LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	PostMessage(WM_CLOSE);
	return 0;
}


void CMainFrame::OnEditCut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	CEdit editAddress = m_AddressBar.GetEditCtrl();
	CEdit editSearch  = m_SearchBar.GetEditCtrl();

	if (::GetFocus() == editAddress) {
		editAddress.Cut();
	} else if (::GetFocus() == editSearch) {
		editSearch.Cut();
	} else {
		SetMsgHandled(FALSE);							// MtlWeb.h��CWebBrowserCommandHandler������
		return;
	}
}


void CMainFrame::OnEditCopy(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	CEdit editAddress = m_AddressBar.GetEditCtrl();
	CEdit editSearch  = m_SearchBar.GetEditCtrl();

	if (::GetFocus() == editAddress) {
		editAddress.Copy();
	} else if (::GetFocus() == editSearch) {
		editSearch.Copy();
	} else {
		SetMsgHandled(FALSE);							// MtlWeb.h��CWebBrowserCommandHandler������
		return;
	}
}


void CMainFrame::OnEditPaste(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	CEdit editAddress = m_AddressBar.GetEditCtrl();
	CEdit editSearch  = m_SearchBar.GetEditCtrl();

	if (::GetFocus() == editAddress) {
		editAddress.Paste();
	} else if (::GetFocus() == editSearch) {
		editSearch.Paste();
	} else {
		SetMsgHandled(FALSE);
		return;
	}
}


void CMainFrame::OnEditSelectAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	CEdit editAddress = m_AddressBar.GetEditCtrl();
	CEdit editSearch  = m_SearchBar.GetEditCtrl();

	if (::GetFocus() == editAddress) {
		editAddress.SetSelAll();
	} else if (::GetFocus() == editSearch) {
		editSearch.SetSelAll();
	} else {
		SetMsgHandled(FALSE);							// MtlWeb.h��CWebBrowserCommandHandler������
		return;
	}
}


// U.H
LRESULT CMainFrame::OnViewCommandBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	MtlToggleBandVisible(m_hWndToolBar, ATL_IDW_COMMAND_BAR);
	UpdateLayout();
	return 0;
}


LRESULT CMainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	// It sucks, band index is dynamic.
	// ::SendMessage(m_hWndToolBar, RB_SHOWBAND, 0, bNew);	// toolbar is band #0
	MtlToggleBandVisible(m_hWndToolBar, ATL_IDW_TOOLBAR);
	UpdateLayout();
	return 0;
}


LRESULT CMainFrame::OnViewAddressBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	MtlToggleBandVisible(m_hWndToolBar, IDC_ADDRESSBAR);
	UpdateLayout();
	return 0;
}


LRESULT CMainFrame::OnViewLinkBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	MtlToggleBandVisible(m_hWndToolBar, IDC_LINKBAR);
	UpdateLayout();
	return 0;
}


LRESULT CMainFrame::OnViewTabBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	MtlToggleBandVisible(m_hWndToolBar, IDC_MDITAB);
	UpdateLayout();
	return 0;
}



LRESULT CMainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	BOOL bNew = !::IsWindowVisible(m_hWndStatusBar);

	UpdateTitleBar(_T(""), 0);		//+++		status�I�t����I���ɂ����Ƃ��ɁA�^�C�g���o�[�ɏo���Ă����X�e�[�^�X���������������.

	::ShowWindow(m_hWndStatusBar, bNew ? SW_SHOWNOACTIVATE : SW_HIDE);

	UpdateLayout();
	return 0;
}

//---------------------------------------
/// [���[�U�[��`] : �c�[���o�[�̃J�X�^�}�C�Y�_�C�A���O��\������
LRESULT CMainFrame::OnViewToolBarCust(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	m_ToolBar.Customize();
	return 0;
}


void CMainFrame::OnFavoriteAdd(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	::SendMessage(MDIGetActive(), WM_COMMAND, (WPARAM) ID_FAVORITE_ADD, 0);
}


void CMainFrame::OnFavoriteOrganize(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	bool bOldShell = _check_flag(MAIN_EX_ORGFAVORITEOLDSHELL, CMainOption::s_dwMainExtendedStyle);

	//		CString strPath = DonutGetFavoritesFolder();
	//		MtlOrganizeFavorite(m_hWnd, bOldShell, strPath);
	MtlOrganizeFavorite( m_hWnd, bOldShell, DonutGetFavoritesFolder() );
}


//public:
///+++	�I�[�g�o�b�N�A�b�v�̊J�n�ݒ�.
//void CMainFrame::SetAutoBackUp() {
//	OnBackUpOptionChanged(0,0,0);
//}


//private:
void CMainFrame::OnViewOptionDonut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	ShowNewStyleDonutOption();
	return;
}


void CMainFrame::ShowNewStyleDonutOption()
{
	BOOL							bSkinChange = FALSE;

	CMenu							menu		= CExMenuManager::LoadFullMenu();

	CMainPropertyPage				pageMain(m_hWnd);
	CMainPropertyPage2				pageMain2(m_hWnd);
	CDLControlPropertyPage			pageDLC(m_hWnd);
	CMDITabPropertyPage 			pageTab(&m_MDITab, menu.m_hMenu);
	CDonutAddressBarPropertyPage	pageAddress(m_AddressBar, m_SearchBar);
	CDonutFavoritesMenuPropertyPage pageFav;
	CFileNewPropertyPage			pageFileNew;
	CStartUpPropertyPage			pageStartUp;
	CProxyPropertyPage				pageProxy;
	CKeyBoardPropertyPage			pageKeyBoard(m_hAccel, menu.m_hMenu);
	CToolBarPropertyPage			pageToolBar(menu.m_hMenu, &bSkinChange, std::bind(&CDonutToolBar::ReloadSkin, &m_ToolBar));
	CMousePropertyPage				pageMouse(menu.m_hMenu, m_SearchBar.GetSearchEngineMenuHandle());
	CMouseGesturePropertyPage		pageGesture(menu.m_hMenu);
	CSearchPropertyPage 			pageSearch;
	CMenuPropertyPage				pageMenu(menu.m_hMenu, m_CmdBar);
	CRightClickPropertyPage			pageRightMenu(menu);
	CExplorerPropertyPage			pageExplorer;
	CDestroyPropertyPage			pageDestroy;
	CSkinPropertyPage				pageSkin(m_hWnd, &bSkinChange);
	CLinkBarPropertyPage			pageLinks(m_LinkBar);

	CString 						strURL, strTitle;
	HWND							hWndActive	= MDIGetActive();

	if (hWndActive) {
		CWebBrowser2 browser = DonutGetIWebBrowser2(hWndActive);

		if (browser.m_spBrowser != NULL) {
			strURL	 = browser.GetLocationURL();
			strTitle = MtlGetWindowText(hWndActive); // pChild->get_LocationName();
		}
	}

	CIgnoredURLsPropertyPage		pageURLs(strURL);
	CCloseTitlesPropertyPage		pageTitles( strTitle );
	CUrlSecurityPropertyPage		pageUrlSecu(strURL);		//+++
	CDonutExecutablePropertyPage	pageExe;
	CDonutConfirmPropertyPage		pageCnf;
	CPluginPropertyPage 			pagePlugin;

	CTreeViewPropertySheet			sheet( _T("Donut�̃I�v�V����") );

	sheet.AddPage( pageMain				 );
	sheet.AddPage( pageMain2	 , TRUE  );
	sheet.AddPage( pageToolBar			 );
	sheet.AddPage( pageTab		 , TRUE  );
	sheet.AddPage( pageAddress	 , FALSE );
	sheet.AddPage( pageSearch	 , FALSE );
	sheet.AddPage( pageLinks	 , FALSE );
	sheet.AddPage( pageExplorer			 );
	sheet.AddPage( pageMenu				 );
	sheet.AddPage( pageRightMenu , TRUE  );
	sheet.AddPage( pageFav		 , FALSE );
	sheet.AddPage( pageKeyBoard			 );
	sheet.AddPage( pageMouse			 );
	sheet.AddPage( pageGesture	 , TRUE  );
	sheet.AddPage( pageFileNew			 );
	sheet.AddPage( pageStartUp			 );
	sheet.AddPage( pageDestroy			 );
	sheet.AddPage( pageDLC				 );
	sheet.AddPage( pageURLs 	 , TRUE  );
	sheet.AddPage( pageTitles	 , FALSE );
	sheet.AddPage( pageUrlSecu 	 , FALSE );				//+++
	//sheet.AddPage( pageDeterrent	 );

	sheet.AddPage( pageExe			 );
	sheet.AddPage( pageCnf			 );
	sheet.AddPage( pageProxy		 );
	sheet.AddPage( pageSkin 		 );
	sheet.AddPage( pagePlugin		 );

	/* [Donut�̃I�v�V����]��\�� */
	sheet.DoModal();

	//m_cmbBox.ResetProxyList();

	// �L�[�̌ďo
	CAccelerManager accelManager(m_hAccel);
	m_hAccel = accelManager.LoadAccelaratorState(m_hAccel);

  #if 0 //+++	"�^�u���[�h�ŋN������"�̃`�F�b�N�𔽉f... ����ς肵�Ȃ��ł���.
	m_mcCmdBar.setOnlyCloseButton(CMainOption::s_bTabMode);
  #endif
	RtlSetMinProcWorkingSetSize();		//+++ (�������̗\��̈���ꎞ�I�ɍŏ����B�E�B���h�E���ŏ��������ꍇ�Ɠ���)
}


void CMainFrame::OnViewOption(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	if (MDIGetActive() == NULL)
		MtlShowInternetOptions();
}


void CMainFrame::OnViewRefreshAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	RtlSetMinProcWorkingSetSize();		//+++ (�������̗\��̈���ꎞ�I�ɍŏ����B�E�B���h�E���ŏ��������ꍇ�Ɠ���)
	MtlForEachMDIChild( m_hWndMDIClient, CPostCommand(ID_VIEW_REFRESH) );
}


void CMainFrame::OnViewStopAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	MtlForEachMDIChild( m_hWndMDIClient, CPostCommand(ID_VIEW_STOP) );
	RtlSetMinProcWorkingSetSize();		//+++ (�������̗\��̈���ꎞ�I�ɍŏ����B�E�B���h�E���ŏ��������ꍇ�Ɠ���)
}


// Implementation

//private:


//public:
BOOL CMainFrame::AddSimpleReBarBandCtrl(
	HWND	hWndReBar,
	HWND	hWndBand,
	int 	nID,
	LPTSTR	lpstrTitle,
	UINT	fStyle,
	int 	cxWidth,
	BOOL	bFullWidthAlways,
	HBITMAP hBitmap /* = NULL*/)
{
	dmfTRACE( _T("CMainFrame::AddSimpleReBarBandCtrl\n") );
	ATLASSERT( ::IsWindow(hWndReBar) ); 	// must be already created
	ATLASSERT( ::IsWindow(hWndBand) );		// must be already created
	MTLASSERT_KINDOF(REBARCLASSNAME, hWndReBar);

	// Set band info structure
	REBARBANDINFO rbBand;
	rbBand.cbSize	 = sizeof (REBARBANDINFO);
  #if (_WIN32_IE >= 0x0400)
	rbBand.fMask	 = RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_STYLE | RBBIM_ID | RBBIM_SIZE | RBBIM_IDEALSIZE;
  #else
	rbBand.fMask	 = RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_STYLE | RBBIM_ID | RBBIM_SIZE;
  #endif	//!(_WIN32_IE >= 0x0400)

	rbBand.fMask	|= RBBIM_BACKGROUND | RBBIM_TEXT;
	rbBand.fStyle	 = fStyle;
	rbBand.lpText	 = lpstrTitle;
	rbBand.hwndChild = hWndBand;
	rbBand.wID		 = nID;

	rbBand.hbmBack	 = hBitmap;
	rbBand.fStyle	|= RBBS_FIXEDBMP;

	// Calculate the size of the band
	BOOL		  bRet;
	RECT		  rcTmp;
	int 		  nBtnCount = (int) ::SendMessage(hWndBand, TB_BUTTONCOUNT, 0, 0L);

	if (nBtnCount > 0) {
		// �c�[���o�[�̏ꍇ
		bRet				= ::SendMessage(hWndBand, TB_GETITEMRECT, nBtnCount - 1, (LPARAM) &rcTmp) != 0;
		ATLASSERT(bRet);
		rbBand.cx			= (cxWidth != 0) ? cxWidth : rcTmp.right;
		rbBand.cyMinChild	= rcTmp.bottom - rcTmp.top;

		if (bFullWidthAlways) {
			rbBand.cxMinChild = rbBand.cx;
		} else if (lpstrTitle == 0) {
			CRect rcTmp;					// check!!
			bRet			  = ::SendMessage(hWndBand, TB_GETITEMRECT, 0, (LPARAM) &rcTmp) != 0;
			ATLASSERT(bRet);
			rbBand.cxMinChild = rcTmp.right;
		} else {
			rbBand.cxMinChild = 0;
		}
	} else {								// no buttons, either not a toolbar or really has no buttons
		bRet				= ::GetWindowRect(hWndBand, &rcTmp) != 0;
		ATLASSERT(bRet);
		rbBand.cx			= (cxWidth != 0) ? cxWidth : (rcTmp.right - rcTmp.left);
		rbBand.cxMinChild	= (bFullWidthAlways) ? rbBand.cx : 0;
		rbBand.cyMinChild	= rcTmp.bottom - rcTmp.top;
	}

  #if (_WIN32_IE >= 0x0400)
	// NOTE: cxIdeal used for CHEVRON, if MDI cxIdeal changed dynamically.
	rbBand.cxIdeal = rcTmp.right;			//rbBand.cx is not good.
  #endif		//(_WIN32_IE >= 0x0400)

	// Add the band
	LRESULT 	  lRes = ::SendMessage(hWndReBar, RB_INSERTBAND, (WPARAM) -1, (LPARAM) &rbBand);

	if (lRes == 0) {
		ATLTRACE2( atlTraceUI, 0, _T("Failed to add a band to the rebar.\n") );
		return FALSE;
	}

  #if (_WIN32_IE >= 0x0501)
	if (nID == IDC_LINKBAR)
		::SendMessage(hWndBand, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_HIDECLIPPEDBUTTONS);
  #endif		//(_WIN32_IE >= 0x0501)

	return TRUE;
}


// UPDATE_COMMAND_UI_MAP()






// UDT DGSTR
bool CMainFrame::_IsExistHTMLHelp(void)
{
	return ( 0xFFFFFFFF != GetFileAttributes( MtlGetHTMLHelpPath() ) );
}
// ENDE



bool CMainFrame::ChkCookies(UINT nID)
{
	HINSTANCE	hInstDLL = LoadLibrary(_T("wininet.dll"));
	if (hInstDLL == NULL)
		return false;

	DWORD (WINAPI * __PrivacyGetZonePreferenceW)(DWORD, DWORD, LPDWORD, LPWSTR, LPDWORD) = NULL;
	__PrivacyGetZonePreferenceW = ( DWORD (WINAPI *)(DWORD,DWORD,LPDWORD,LPWSTR, LPDWORD) )
									GetProcAddress(hInstDLL,"PrivacyGetZonePreferenceW");
	if (__PrivacyGetZonePreferenceW == NULL)
		return false;

	DWORD	dwCookie1 = 0;
	DWORD	dwCookie2 = 0;
	DWORD	BufLen	  = 0;
	DWORD	ret;
	ret = __PrivacyGetZonePreferenceW(URLZONE_INTERNET, PRIVACY_TYPE_FIRST_PARTY, &dwCookie1, NULL, &BufLen);
	ret = __PrivacyGetZonePreferenceW(URLZONE_INTERNET, PRIVACY_TYPE_THIRD_PARTY, &dwCookie2, NULL, &BufLen);

	bool		bSts		= false;
	switch (nID) {
	case ID_URLACTION_COOKIES_BLOCK:
		if (dwCookie1 == dwCookie2 && dwCookie1 == PRIVACY_TEMPLATE_NO_COOKIES)
			bSts = true;
		break;

	case ID_URLACTION_COOKIES_HI:
		if (dwCookie1 == dwCookie2 && dwCookie1 == PRIVACY_TEMPLATE_HIGH)
			bSts = true;
		break;

	case ID_URLACTION_COOKIES_MIDHI:
		if (dwCookie1 == dwCookie2 && dwCookie1 == PRIVACY_TEMPLATE_MEDIUM_HIGH)
			bSts = true;
		break;

	case ID_URLACTION_COOKIES_MID:
		if (dwCookie1 == dwCookie2 && dwCookie1 == PRIVACY_TEMPLATE_MEDIUM)
			bSts = true;
		break;

	case ID_URLACTION_COOKIES_LOW:
		if (dwCookie1 == dwCookie2 && dwCookie1 == PRIVACY_TEMPLATE_MEDIUM_LOW)
			bSts = true;
		break;

	case ID_URLACTION_COOKIES_ALL:
		if (dwCookie1 == dwCookie2 && dwCookie1 == PRIVACY_TEMPLATE_LOW)
			bSts = true;
		break;

	case ID_URLACTION_COOKIES_CSTM:
		if (dwCookie1 == PRIVACY_TEMPLATE_ADVANCED)
			bSts = true;
		if (dwCookie2 == PRIVACY_TEMPLATE_ADVANCED)
			bSts = true;
		break;
	}

	return bSts;
}


void CMainFrame::OnUpdateWindowArrange(CCmdUI *pCmdUI)
{
	// toolbar has not this command button, called only on menu popuping
	MDIRefreshMenu();
	MtlCompactMDIWindowMenuDocumentList( m_menuWindow, _nWindowMenuItemCount,
										CFavoritesMenuOption::GetMaxMenuItemTextLength() );

	pCmdUI->Enable(MDIGetActive() != NULL && !CMainOption::s_bTabMode);
}


void CMainFrame::OnUpdateProgressUI(CCmdUI *pCmdUI)
{
	CProgressBarCtrl progressbar = pCmdUI->m_wndOther;

	progressbar.ShowWindow(SW_HIDE);
}


void CMainFrame::OnUpdateStautsIcon(CCmdUI *pCmdUI)
{
	pCmdUI->m_wndOther.SendMessage(WM_STATUS_SETICON, MAKEWPARAM(pCmdUI->m_nID, -1), 0);
}


bool CMainFrame::_IsClipboardAvailable()
{
	return ::IsClipboardFormatAvailable(MTL_CF_TEXT) == TRUE;		//+++ UNICODE�Ή�(MTL_CF_TEXT)
}


//minit
LRESULT CMainFrame::OnSpecialKeys(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	int nCode = 0;
	switch (wID) {
	case ID_SPECIAL_HOME:		nCode = VK_HOME;	break;
	case ID_SPECIAL_END:		nCode = VK_END; 	break;
	case ID_SPECIAL_PAGEUP: 	nCode = VK_PRIOR;	break;
	case ID_SPECIAL_PAGEDOWN:	nCode = VK_NEXT;	break;
	case ID_SPECIAL_UP: 		nCode = VK_UP;		break;
	case ID_SPECIAL_DOWN:		nCode = VK_DOWN;	break;
	default:	ATLASSERT(FALSE);
	}
	CChildFrame* pChild = GetActiveChildFrame();
	if (pChild == nullptr) 
		return 0;

	if (wID == ID_SPECIAL_HOME || ID_SPECIAL_END) {
		auto funcGetHTMLWindowOnCursorPos = [](CPoint& pt, IHTMLDocument3* pDoc) -> CComPtr<IHTMLWindow2> {
			auto funcGetIFrameAbsolutePosition = [](CComQIPtr<IHTMLElement>	spIFrame) -> CRect {
				CRect rc;
				spIFrame->get_offsetHeight(&rc.bottom);
				spIFrame->get_offsetWidth(&rc.right);
				CComPtr<IHTMLElement>	spCurElement = spIFrame;
				do {
					CPoint temp;
					spCurElement->get_offsetTop(&temp.y);
					spCurElement->get_offsetLeft(&temp.x);
					rc += temp;
					CComPtr<IHTMLElement>	spTemp;
					spCurElement->get_offsetParent(&spTemp);
					spCurElement.Release();
					spCurElement = spTemp;
				} while (spCurElement.p);
				
				return rc;
			};
			auto funcGetScrollPosition = [](CComQIPtr<IHTMLDocument2> spDoc2) -> CPoint {
				CPoint ptScroll;
				CComPtr<IHTMLElement>	spBody;
				spDoc2->get_body(&spBody);
				CComQIPtr<IHTMLElement2>	spBody2 = spBody;
				spBody2->get_scrollTop(&ptScroll.y);
				spBody2->get_scrollLeft(&ptScroll.x);
				if (ptScroll == CPoint(0, 0)) {
					CComQIPtr<IHTMLDocument3>	spDoc3 = spDoc2;
					CComPtr<IHTMLElement>	spDocumentElement;
					spDoc3->get_documentElement(&spDocumentElement);
					CComQIPtr<IHTMLElement2>	spDocumentElement2 = spDocumentElement;
					spDocumentElement2->get_scrollTop(&ptScroll.y);
					spDocumentElement2->get_scrollLeft(&ptScroll.x);
				}
				return ptScroll;
			};

			HRESULT hr = S_OK;
			CComQIPtr<IHTMLDocument2>	spDoc2 = pDoc;

			CComPtr<IHTMLFramesCollection2>	spFrames;
			spDoc2->get_frames(&spFrames);
			CComPtr<IHTMLElementCollection>	spIFrameCol;
			pDoc->getElementsByTagName(CComBSTR(L"iframe"), &spIFrameCol);
			CComPtr<IHTMLElementCollection>	spFrameCol;
			pDoc->getElementsByTagName(CComBSTR(L"frame"), &spFrameCol);
			
			long frameslength = 0, iframelength = 0, framelength = 0;
			spFrames->get_length(&frameslength);
			spIFrameCol->get_length(&iframelength);
			spFrameCol->get_length(&framelength);
			ATLASSERT(frameslength == iframelength || frameslength == framelength);

			if (frameslength == iframelength && spIFrameCol.p && spFrames.p) {
				for (long i = 0; i < iframelength; ++i) {
					CComVariant vIndex(i);
					CComPtr<IDispatch>	spDisp2;
					spIFrameCol->item(vIndex, vIndex, &spDisp2);
					CRect rcAbsolute = funcGetIFrameAbsolutePosition(spDisp2.p);
					CPoint ptScroll = funcGetScrollPosition(spDoc2);
					CRect rc = rcAbsolute - ptScroll;
					if (rc.PtInRect(pt)) {
						CComVariant vResult;
						spFrames->item(&vIndex, &vResult);
						CComQIPtr<IHTMLWindow2> spWindow = vResult.pdispVal;
						return spWindow;
					}
				}
			}

			if (frameslength == framelength && spFrameCol.p && spFrames.p) {
				for (long i = 0; i < framelength; ++i) {
					CComVariant vIndex(i);
					CComPtr<IDispatch>	spDisp2;
					spFrameCol->item(vIndex, vIndex, &spDisp2);
					CComQIPtr<IHTMLElement>	spFrame = spDisp2;
					if (spFrame.p) {
						CRect rc;
						spFrame->get_offsetLeft(&rc.left);
						spFrame->get_offsetTop(&rc.top);
						long temp;
						spFrame->get_offsetWidth(&temp);
						rc.right += rc.left + temp;
						spFrame->get_offsetHeight(&temp);
						rc.bottom+= rc.top + temp;
						if (rc.PtInRect(pt)) {
							CComVariant vResult;
							spFrames->item(&vIndex, &vResult);
							CComQIPtr<IHTMLWindow2> spWindow = vResult.pdispVal;
							return spWindow;
						}
					}
				}
			}
			return nullptr;
		};

		CComPtr<IDispatch>	spDisp;
		HRESULT hr = pChild->m_spBrowser->get_Document(&spDisp);
		CComQIPtr<IHTMLDocument3>	spDocument = spDisp;
		if (spDocument == nullptr)
			return 0;
		CPoint pt;
		::GetCursorPos(&pt);
		pChild->ScreenToClient(&pt);

		CComPtr<IHTMLWindow2> spWindow = funcGetHTMLWindowOnCursorPos(pt, spDocument);
		if (spWindow) {
			CComQIPtr<IHTMLDocument2>	spFrameDocument;
			hr = spWindow->get_document(&spFrameDocument);
			if ( FAILED(hr) ) {
				CComQIPtr<IServiceProvider>  spServiceProvider = spWindow;
				ATLASSERT(spServiceProvider);
				CComPtr<IWebBrowser2>	spBrowser;
				hr = spServiceProvider->QueryService(IID_IWebBrowserApp, IID_IWebBrowser2, (void**)&spBrowser);
				if (!spBrowser)
					return 0;
				CComPtr<IDispatch>	spDisp;
				hr = spBrowser->get_Document(&spDisp);
				if (!spDisp)
					return 0;
				spFrameDocument = spDisp;
				if (!spFrameDocument)
					return 0;
				spWindow.Release();
				spFrameDocument->get_parentWindow(&spWindow);
				if (!spWindow)
					return 0;
			}

			if (wID == ID_SPECIAL_HOME)
				spWindow->scrollBy(0, -300000);
			else if (wID == ID_SPECIAL_END)
				spWindow->scrollBy(0, 300000);
		} else {
			CComQIPtr<IHTMLDocument2>	spDocument2 = spDocument;
			spDocument2->get_parentWindow(&spWindow);
			ATLASSERT(spWindow);
			if (wID == ID_SPECIAL_HOME)
				spWindow->scrollBy(0, -300000);
			else if (wID == ID_SPECIAL_END)
				spWindow->scrollBy(0, 300000);
		}
	} else {
		pChild->SetFocus();

		INPUT	inputs[2] = { 0 };
		inputs[0].type		= INPUT_KEYBOARD;
		inputs[0].ki.wVk	= nCode;
		inputs[0].ki.wScan	= ::MapVirtualKey(nCode, 0);
		inputs[0].ki.dwFlags= 0;
		inputs[1].type		= INPUT_KEYBOARD;
		inputs[1].ki.wVk	= nCode;
		inputs[1].ki.wScan	= ::MapVirtualKey(nCode, 0);
		inputs[1].ki.dwFlags= KEYEVENTF_KEYUP;
		SendInput(2, inputs, sizeof(INPUT));

	//PostMessage(WM_KEYDOWN, nCode, 0);
	}
	return 0;
}


LRESULT CMainFrame::OnSecurityReport(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	HWND	hWndActive = MDIGetActive();
	if (hWndActive == NULL)
		return 0;

	//With Internet Explorer 6 for Microsoft Windows XP Service Pack 2 (SP2) and later
	//�������Ă�
	CWebBrowser2	   browser	  = DonutGetIWebBrowser2(hWndActive);
	IOleCommandTarget *pct;

	if ( SUCCEEDED( browser.m_spBrowser->QueryInterface(IID_IOleCommandTarget, (void **) &pct) ) ) {
		pct->Exec(&CGID_ShellDocView, SHDVID_SSLSTATUS, 0, NULL, NULL);
		pct->Release();
	}

	return S_OK;
}


//+++ �E�B���h�E�E�T���l�[���\��(�y�[�W�I��)
LRESULT CMainFrame::OnWindowThumbnail(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
  #if 1	//+++ �y�[�W���Ȃ������炩����
	HWND	hWndActive = MDIGetActive();
	if (!hWndActive)
		return 0;
  #endif
  #if 1 //+++	���X�R�����g�������̂��A���߂������p��#if��.
	CThumbnailDlg	dlg;
	dlg.DoModal(m_hWnd, (LPARAM)m_hWndMDIClient);
  #endif
	return S_OK;
}


LRESULT CMainFrame::_OnMDIActivate(HWND hWndActive)
{
	//�v���O�C���Ƀ^�u���ύX���ꂽ���Ƃ�`����
	int 		  nTabIndex;
	IWebBrowser2 *pWB2;

	if (hWndActive == NULL) {
		nTabIndex = -1;
		pWB2	  = NULL;
	} else {
		nTabIndex = m_MDITab.GetTabIndex(hWndActive);
		pWB2	  = DonutGetIWebBrowser2(hWndActive);
	}

	int 		  nCount = CPluginManager::GetCount(PLT_TOOLBAR);

	for (int i = 0; i < nCount; i++) {
		if ( CPluginManager::Call_Event_TabChanged(PLT_TOOLBAR, i, nTabIndex, pWB2) )
			return TRUE;
	}

	return 0;
}


LRESULT CMainFrame::OnShowActiveMenu(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	HWND	hWndActive = MDIGetActive();
	if (!hWndActive)
		return 0;

	int  nIndex 	= m_MDITab.GetTabIndex(hWndActive);
	m_MDITab.ShowTabMenu(nIndex);

	return 0;
}


#ifndef NO_STYLESHEET
//public:
LRESULT CMainFrame::OnUseUserStyleSheet(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	bool	bNowFlag = !CStyleSheetOption::GetUseUserSheet();

	CStyleSheetOption::SetUseUserSheet(bNowFlag);
	return 0;
}


//private:
LRESULT CMainFrame::OnSetUserStyleSheet(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	bool	bNowFlag = !CStyleSheetOption::s_bSetUserSheet;

	CStyleSheetOption::s_bSetUserSheet = bNowFlag;
	return 0;
}
#endif


LRESULT CMainFrame::OnShowExMenu(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	CMenu menu = CExMenuManager::LoadExMenu(CExMenuManager::EXMENU_ID_FIRST);
	POINT pos;

	GetCursorPos(&pos);
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pos.x, pos.y, m_hWnd, NULL);
	return 0;
}


CChildFrame *CMainFrame::GetChildFrame(HWND hWndChild)
{
	if ( !::IsWindow(hWndChild) )
		return NULL;

	//�����@ChildFrame �|�C���^
	//���s�@NULL((BOOL)FALSE)
	return (CChildFrame *) ::SendMessage(hWndChild, WM_GET_CHILDFRAME, 0, 0);
}


CChildFrame *CMainFrame::GetActiveChildFrame()
{
	HWND	hWnd = MDIGetActive();
	if (hWnd)
		return GetChildFrame(hWnd);
	return NULL;
}


//private:

HRESULT CMainFrame::LoadWebBrowserFromStream(IWebBrowser *pWebBrowser, IStream *pStream)
{
	HRESULT 			hr;
	IDispatch * 		pHtmlDoc		   = NULL;
	IPersistStreamInit *pPersistStreamInit = NULL;

	// Retrieve the document object.
	hr = pWebBrowser->get_Document( &pHtmlDoc );
	if ( SUCCEEDED(hr) ) {
		// Query for IPersistStreamInit.
		hr = pHtmlDoc->QueryInterface( IID_IPersistStreamInit,	(void **) &pPersistStreamInit );
		if ( SUCCEEDED(hr) ) {
			// Initialize the document.
			hr = pPersistStreamInit->InitNew();
			if ( SUCCEEDED(hr) ) {
				// Load the contents of the stream.
				hr = pPersistStreamInit->Load( pStream );
			}
			pPersistStreamInit->Release();
		}
	}

	return S_OK;
}


LRESULT CMainFrame::OnShowToolBarMenu()
{
	POINT		pt = {0};

	::GetCursorPos(&pt);

	CMenuHandle submenu = ::GetSubMenu(::GetSubMenu(m_hMenu, 2), 0);
	if ( submenu.IsMenu() )
		submenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);

	return 0;
}



///+++ ����:.url �ɑ΂���g���v���p�e�B
LRESULT CMainFrame::OnSetExProperty(LPCTSTR lpstrUrlFile)
{
	CString 		  strUrlFile = lpstrUrlFile;

	if ( !MtlIsExt( strUrlFile, _T(".url") ) )
		return 0;

	CExPropertyDialog dlg(strUrlFile);
	dlg.DoModal();

	return 0;
}


// ==========================================================================
// �߂�E�i��

BOOL CMainFrame::_Load_OptionalData(CChildFrame *pChild, const CString &strFileName, CString &strSection)
{
	pChild->SetDfgFileNameSection(strFileName, strSection);
	return TRUE;
}


BOOL CMainFrame::_Load_OptionalData2(CChildFrame *pChild,
						 std::vector<std::pair<CString, CString> > &ArrayFore,
						 std::vector<std::pair<CString, CString> > &ArrayBack)
{
	pChild->SetArrayHist( ArrayFore, ArrayBack );
	return TRUE;
}



LRESULT CMainFrame::OnMenuRecentDocument(HMENU hMenu)
{
	CRecentDocumentListFixed *pList;
	pList = CMainOption::s_pMru;
	CMenuHandle 			  menu	 = hMenu;

	if (menu.m_hMenu == NULL)
		return 0;

	if (pList == NULL)
		return 0;

	if (menu.GetMenuItemCount() == 0)
		menu.AppendMenu(MF_ENABLED | MF_STRING, ID_RECENTDOCUMENT_FIRST, (LPCTSTR) NULL);
	else if (menu.GetMenuItemID(0) != ID_RECENTDOCUMENT_FIRST)
		return 0;

	HMENU					  hMenuT = pList->GetMenuHandle();
	pList->SetMenuHandle(menu);
	pList->UpdateMenu();
	pList->SetMenuHandle(hMenuT);

	return 1;
}


LRESULT CMainFrame::OnMenuRecentLast(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	return ::SendMessage(m_hWnd, WM_COMMAND, ID_RECENTDOCUMENT_FIRST, 0);
}


int CMainFrame::_GetRecentCount()
{
	return CMainOption::s_pMru->GetRecentCount();
}



// ==========================================================================
// �^�u�֌W


LRESULT CMainFrame::OnMouseWheel(UINT fwKeys, short zDelta, CPoint point)
{
	// I don't have a wheel mouse...
	if (  CTabBarOption::s_bWheel
	   && MtlIsBandVisible(m_hWndToolBar, IDC_MDITAB) )
	{
		CRect rcTab;
		m_MDITab.GetWindowRect(&rcTab);

		if ( rcTab.PtInRect(point) ) {
			if (zDelta > 0) {
				m_MDITab.LeftTab();
			} else {
				m_MDITab.RightTab();
			}

			SetMsgHandled(TRUE);
			return 0;
		}
	}

	// �E�N���b�N����Ă����� - �X�N���[���̂���r���[�ゾ�ƃr���[���X�N���[�������o�O
	if (fwKeys == VK_RBUTTON) {
		if (zDelta > 0) {
			m_MDITab.LeftTab();
		} else {
			m_MDITab.RightTab();
		}

		SetMsgHandled(TRUE);
		return 0;
	}

	SetMsgHandled(FALSE);
	return 1;
}



LRESULT CMainFrame::OnViewTabBarMulti(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	CTabBarOption::s_bMultiLine = !CTabBarOption::s_bMultiLine;
	CTabBarOption::WriteProfile();

	m_MDITab.ReloadSkin();
	return 0;
}



void CMainFrame::OnTabLeft(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl)
{
	m_MDITab.LeftTab();
}


void CMainFrame::OnTabRight(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl)
{
	m_MDITab.RightTab();
}


#if 1 //+++
//+++ �擪�^�u����1..8�I��. 9�͍Ō�̃^�u��I��
LRESULT CMainFrame::OnTabIdx(WORD /*wNotifyCode*/, WORD wID, HWND hWndCtl, BOOL & /*bHandled*/)
{
	ATLASSERT(wID >= ID_TAB_IDX_1 && wID <= ID_TAB_IDX_LAST);
	int nCount = m_MDITab.GetItemCount();

	if (wID == ID_TAB_IDX_LAST) {
		wID =  nCount - 1;
	} else {
		wID -= ID_TAB_IDX_1;
		if (wID < 0 || wID >=8 || wID >= nCount)
			return 0;
	}
	m_MDITab.SetCurSel(wID);
	return 0;
}
#endif



LRESULT CMainFrame::OnTabListDefault(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	CPoint pos;
	::GetCursorPos(&pos);

	int    nRet = m_MDITab.ShowTabListMenuDefault(pos.x, pos.y);

	if (nRet == -1)
		return 0;

	HWND   hWnd = m_MDITab.GetTabHwnd(nRet);

	if ( ::IsWindow(hWnd) )
		::SendMessage(m_wndMDIClient, WM_MDIACTIVATE, (WPARAM) hWnd, 0);

	return 0;
}


LRESULT CMainFrame::OnTabListVisible(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	CPoint pos;
	::GetCursorPos(&pos);

	int    nRet = m_MDITab.ShowTabListMenuVisible(pos.x, pos.y);

	if (nRet == -1)
		return 0;

	HWND   hWnd = m_MDITab.GetTabHwnd(nRet);

	if ( ::IsWindow(hWnd) )
		::SendMessage(m_wndMDIClient, WM_MDIACTIVATE, (WPARAM) hWnd, 0);

	return 0;
}


LRESULT CMainFrame::OnTabListAlphabet(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	CPoint pos;
	::GetCursorPos(&pos);

	int    nRet = m_MDITab.ShowTabListMenuAlphabet(pos.x, pos.y);

	if (nRet == -1)
		return 0;

	HWND   hWnd = m_MDITab.GetTabHwnd(nRet);

	if ( ::IsWindow(hWnd) )
		::SendMessage(m_wndMDIClient, WM_MDIACTIVATE, (WPARAM) hWnd, 0);

	return 0;
}



// ==========================================================================
// ���T�C�Y�֌W


#if 0	//+++	���s
LRESULT CMainFrame::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
  #if 1	//+++ ������
	m_bMinimized	= (wParam == SIZE_MINIMIZED);
	if (m_bFullScreen == 0 && m_bMinimized == 0)		//+++ �t���X�N���[������k�����ȊO�̏ꍇ��
		m_bOldMaximized = (wParam == SIZE_MAXIMIZED);	//+++ �ő剻���Ă��邩�ǂ������`�F�b�N
  #endif
	return CMDIFrameWindowImpl< CMainFrame >::OnSize(uMsg, wParam, lParam, bHandled);
}
#endif

#if 1	//+++
#if 1	//+++ ���s�̂��܂���
void CMainFrame::ShowWindow_Restore(bool flag)
{
	int nShow = flag ? SW_RESTORE : SW_SHOW;
	ShowWindow( nShow );
}
#else	//���s
void CMainFrame::ShowWindow_Restore(bool)
{
	int nShow = SW_SHOWNORMAL;	//SW_RESTORE;
	if (m_bMinimized)
		nShow = SW_SHOWMINIMIZED;
	if (m_bOldMaximized)		//+++ m_bFullScreen,m_bOldMaximized���������ݒ肳��Ă���Ƃ��ď���.
		nShow = SW_SHOWMAXIMIZED;
	ShowWindow( nShow );
}
#endif
#endif


LRESULT CMainFrame::OnMainFrameMinimize(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	ShowWindow(SW_MINIMIZE);
	//m_bOldMaximized = 0;
	// //m_bMinimized    = 1;
	RtlSetMinProcWorkingSetSize();		//+++ ( �������̗\��̈���ꎞ�I�ɍŏ����B�E�B���h�E���ŏ��������ꍇ�Ɠ��� )
	return 0;
}


LRESULT CMainFrame::OnMainFrameMaximize(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	ShowWindow(SW_MAXIMIZE);
	m_bOldMaximized = 1;
	// //m_bMinimized    = 0;
	RtlSetMinProcWorkingSetSize();		//+++ ( �������̗\��̈���ꎞ�I�ɍŏ����B�E�B���h�E���ŏ��������ꍇ�Ɠ��� )
	return 0;
}


LRESULT CMainFrame::OnMainFrameNormMaxSize(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	if (m_bOldMaximized) {
		//ShowWindow(SW_RESTORE);
		ShowWindow_Restore(1);	//ShowWindow(SW_RESTORE);		//�T�C�Y��߂�
		m_bOldMaximized = 0;
	} else {
		ShowWindow(SW_MAXIMIZE);
		m_bOldMaximized = 1;
	}
	RtlSetMinProcWorkingSetSize();		//+++ ( �������̗\��̈���ꎞ�I�ɍŏ����B�E�B���h�E���ŏ��������ꍇ�Ɠ��� )
	return 0;
}


// ==================================================
// �t���X�N���[���֌W

//+++ �N������̑��T�C�Y�ݒ�.
void CMainFrame::startupMainFrameStayle(int nCmdShow, bool bTray)
{
	if ((nCmdShow == SW_SHOWMINIMIZED || nCmdShow == SW_SHOWMINNOACTIVE)
		&& (CMainOption::s_dwMainExtendedStyle2 & MAIN_EX2_MINBTN2TRAY))		//+++ .lnk�Ƃ��ōŏ����ŁA�N�����ꂽ�Ƃ��A�g���C�ɓ���ݒ�Ȃ�g���C��.
	{
		bTray    = true;
		nCmdShow = SW_SHOWNORMAL;		// ���܂������Ȃ�...���A�c��...
	}
	CIniFileI  pr( g_szIniFileName, _T("Main") );
	int wndSML	= MtlGetProfileMainFrameState(pr, *this, nCmdShow);				//+++ s,m,l,full�𔻕ʂ��Ă���(�b���)
	pr.Close();
	m_bOldMaximized 	= 0;
	if (wndSML == 2)
		m_bOldMaximized = 1;

	if (bTray) {
		OnGetOut(0,0,0);
	} else if (CMainOption::s_dwMainExtendedStyle & MAIN_EX_FULLSCREEN) {
		_FullScreen(TRUE);
	}
}


#if 0
bool CMainFrame::IsFullScreen()
{
  #if 1 //+++
	return m_bFullScreen;
  #else
	return (GetStyle() & WS_CAPTION) == 0;
  #endif
}
#endif



void CMainFrame::OnViewFullScreen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	if (IsFullScreen() == 0)			//+++
	{
		_FullScreen(TRUE);
	} else {
		_FullScreen(FALSE);
	}
}



void CMainFrame::_FullScreen(BOOL bOn)
{
	//static bool m_bOldMaximized = 0;
	_ShowBandsFullScreen(bOn);

	SetRedraw(FALSE);

	if (bOn) {							// remove caption
		// save prev visible
		CWindowPlacement	wndpl;
		GetWindowPlacement(&wndpl);

		{
			CIniFileO	pr( g_szIniFileName, _T("Main") );
			wndpl.WriteProfile(pr, _T("frame."));
			//x pr.Close();
		}
		
		m_bFullScreen = true;

		//m_bMinimized		= 0;
		m_bOldMaximized 	= (wndpl.showCmd == SW_SHOWMAXIMIZED);
		ModifyStyle(WS_CAPTION, 0);

		SetMenu(NULL);
		ShowWindow(SW_HIDE);

		m_mcToolBar.m_bVisible = true;

		ShowWindow(SW_MAXIMIZE);
		if (m_bOldMaximized == 0) {
			SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_FRAMECHANGED);
		}
		
	} else {
		m_bFullScreen = false;

		m_mcToolBar.m_bVisible = false;
		// restore prev visible
	  #if 1 //+++	�L���v�V�����̒����͂Â�������̏ꍇ
		if (CSkinOption::s_nMainFrameCaption)
			ModifyStyle(0, WS_CAPTION);
	  #else
		ModifyStyle(0, WS_CAPTION);
	  #endif
		if (m_bOldMaximized == 0) {
			ShowWindow_Restore(1);		//ShowWindow(SW_RESTORE);		//+++ �T�C�Y��߂�
		} else {
			SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_FRAMECHANGED);
		}
	}
	SetRedraw(TRUE);
	RedrawWindow(NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
	if (UxTheme_Wrap::IsCompositionActive() == FALSE) {
		::RedrawWindow(GetDesktopWindow(), NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
		HWND hWndTray = ::FindWindow(_T("Shell_TrayWnd"), NULL);
		if (hWndTray)
			::RedrawWindow(hWndTray, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
	}
	UpdateLayout(TRUE);
}



void CMainFrame::_ShowBandsFullScreen(BOOL bOn, bool bInit /*= false*/)
{
	static BOOL 	s_bOldVisibleStatusBar;
	int 			nIndex = 0;
	if (bOn) { // remove caption
		// save prev visible
		int 		nToolbarPluginCount = CPluginManager::GetCount(PLT_TOOLBAR);
		m_mapToolbarOldVisible.RemoveAll();

		for (nIndex = 0; nIndex < _countof(STDBAR_ID); nIndex++) {
			m_mapToolbarOldVisible.Add( STDBAR_ID[nIndex], MtlIsBandVisible( m_hWndToolBar, STDBAR_ID[nIndex] ) );
		}
		for (nIndex = 0; nIndex < nToolbarPluginCount; nIndex++) {
			m_mapToolbarOldVisible.Add( IDC_PLUGIN_TOOLBAR + nIndex,
										MtlIsBandVisible( m_hWndToolBar, IDC_PLUGIN_TOOLBAR + nIndex ) );
		}
		s_bOldVisibleStatusBar = ::IsWindowVisible(m_hWndStatusBar) != 0;
		SetRedraw(FALSE);
		{
			CIniFileI pr( g_szIniFileName, _T("FullScreen") );
			MtlShowBand(m_hWndToolBar, ATL_IDW_COMMAND_BAR	, pr.GetValue(_T("ShowMenu")	, FALSE) != 0);
			MtlShowBand(m_hWndToolBar, ATL_IDW_TOOLBAR		, pr.GetValue(_T("ShowToolBar") , FALSE) != 0);
			MtlShowBand(m_hWndToolBar, IDC_ADDRESSBAR		, pr.GetValue(_T("ShowAdress")	, FALSE) != 0);
			MtlShowBand(m_hWndToolBar, IDC_MDITAB			, pr.GetValue(_T("ShowTab") 	, FALSE) != 0);
			MtlShowBand(m_hWndToolBar, IDC_LINKBAR			, pr.GetValue(_T("ShowLink")	, FALSE) != 0);
			MtlShowBand(m_hWndToolBar, IDC_SEARCHBAR		, pr.GetValue(_T("ShowSearch")	, FALSE) != 0);
			if (pr.GetValue(_T("ShowStatus"), FALSE ) == FALSE)
				::ShowWindow(m_hWndStatusBar, SW_HIDE);
			else
				::ShowWindow(m_hWndStatusBar, SW_SHOWNOACTIVATE);
		}
		for (nIndex = 0; nIndex < nToolbarPluginCount; nIndex++)
			MtlShowBand(m_hWndToolBar, IDC_PLUGIN_TOOLBAR + nIndex, FALSE);
	} else {
		SetRedraw(FALSE);
		for (nIndex = 0; nIndex < m_mapToolbarOldVisible.GetSize(); nIndex++)
			MtlShowBand( m_hWndToolBar, m_mapToolbarOldVisible.GetKeyAt(nIndex), m_mapToolbarOldVisible.GetValueAt(nIndex) );

		::ShowWindow(m_hWndStatusBar, s_bOldVisibleStatusBar ? SW_SHOWNOACTIVATE : SW_HIDE);
	}
	SetRedraw(TRUE);
}



// ==================================================
// �g���C�A�C�R���֌W


// UDT DGSTR
void CMainFrame::OnGetOut(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	static int nShow = SW_SHOWNORMAL;

	if ( IsWindowVisible() ) {
		//+++ ����:����Ԃ̂Ƃ��̓g���C��
		m_bTray = true; 		//+++
		WINDOWPLACEMENT wp;
		wp.length = sizeof (WINDOWPLACEMENT);
		::GetWindowPlacement(m_hWnd, &wp);
		nShow		  = wp.showCmd;
	  #if 1 //+++ �t���X�N���[���ȊO�Ȃ獡���ő傩�ǂ������T����
		if (m_bFullScreen == 0)
			m_bOldMaximized = (nShow == SW_MAXIMIZE);
	  #endif
		SetHideTrayIcon();	//+++ �g���C�A�C�R����
		Sleep(100); 		// UDT TODO
	} else {				// just db F9 press , come here :p
		m_bTray      = false;	//+++
		//m_bMinimized = 0;
		ShowWindow_Restore(0);	//x ShowWindow( SW_SHOW /*nShow*/ );

		//+++ TrayMessage(m_hWnd, NIM_DELETE, TM_TRAY, 0, NULL);
		DeleteTrayIcon();	//+++ �g���C�A�C�R���폜
	}
}
// ENDE



// UDT DGSTR ( hide window & display icon )
LRESULT CMainFrame::OnMyNotifyIcon(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/)
{
	if ( IsWindowVisible() ) {
		return -1;
	} else {
		switch (lParam) {
		case WM_LBUTTONUP:
			ShowWindow_Restore(0);	//ShowWindow(SW_SHOW);
			DeleteTrayIcon();	//+++
			return 0;
			break;

		case WM_RBUTTONUP:
		  #if 1	//+++
			{
				::SetForegroundWindow(m_hWnd);
				CMenu/*Handle*/ 	menu0;
				menu0.LoadMenu(IDR_TRAYICON_MENU);
				if (menu0.m_hMenu == NULL)
					return 0;
				CMenuHandle menu = menu0.GetSubMenu(0);
				if (menu.m_hMenu == NULL)
					return 0;

				// �|�b�v�A�b�v���j���[���J��.
				POINT 	pt;
				::GetCursorPos(&pt);
				HRESULT hr = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON| TPM_RETURNCMD, pt.x, pt.y, m_hWnd, NULL);
				if (hr == 57666/*���A*/) {
					ShowWindow_Restore(0);	//ShowWindow(SW_SHOW);
					DeleteTrayIcon();	//+++
					return 0;
				}
				if (hr == 57665/*�I��*/) {
					DeleteTrayIcon();	//+++
					PostMessage(WM_CLOSE, 0, 0);
					break;
				}
			}
		  #else
			DeleteTrayIcon();	//+++
			PostMessage(WM_CLOSE, 0, 0);
			break;
		  #endif
		}

		return -1;
	}
}
// ENDE



#if 1 //+++ �g���C�A�C�R���̐ݒ�.
void CMainFrame::SetHideTrayIcon()
{
  #if 1 //+++
	RtlSetMinProcWorkingSetSize();		//+++ (�������̗\��̈���ꎞ�I�ɍŏ����B�E�B���h�E���ŏ��������ꍇ�Ɠ���)
	HICON hIcon = 0;
	if (Misc::IsExistFile(m_strIconSm))
		hIcon	= (HICON)::LoadImage(ModuleHelper::GetResourceInstance(), m_strIconSm, IMAGE_ICON, 16, 16, LR_SHARED|LR_LOADFROMFILE);
	if (hIcon == 0)
		hIcon = LoadIcon( _Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME) );
	TrayMessage( m_hWnd, NIM_ADD, TM_TRAY, hIcon, DONUT_NAME/*_T("unDonut")*/ );		//+++
  #else //��
	//x TrayMessage( m_hWnd, NIM_ADD, TM_TRAY, IDR_MAINFRAME, DONUT_NAME/*_T("unDonut")*/ );
  #endif
	ShowWindow(SW_HIDE);
}
#endif


//+++ �g���C���̏I��/�g���C�A�C�R���̍폜.
void CMainFrame::DeleteTrayIcon()
{
	TrayMessage(m_hWnd, NIM_DELETE, TM_TRAY, 0, NULL);
	//x m_bTrayFlag = false;

	RtlSetMinProcWorkingSetSize();		//+++ ( �������̗\��̈���ꎞ�I�ɍŏ����B�E�B���h�E���ŏ��������ꍇ�Ɠ��� )
}




// ===========================================================================
// �G�N�X�v���[���o�[

// UDT DGSTR
LRESULT CMainFrame::UpdateExpBar(LPCTSTR lpszExpBar, DWORD dwReserved)
{
	//if(lpszExpBar == NULL) return 0;
	m_ExplorerBar.SetTitle(lpszExpBar);
	return 0;
}
// ENDE


//minit
LRESULT CMainFrame::OnRefreshExpBar(int nType)
{
	m_ExplorerBar.RefreshExpBar(nType);
	return 0;
}


// UDT DGSTR //Update minit
LRESULT CMainFrame::OnFavoriteExpBar(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	m_ExplorerBar.ShowBar(true);
	//m_ExplorerBar.Show();
	MtlSendCommand(m_ExplorerBar.m_FavBar.m_hWnd, wID);
	m_wndSplit.SetSinglePaneMode();
	return 0;
}


void CMainFrame::OnFileNewClipBoardEx(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	m_ExplorerBar.m_ClipBar.OpenClipboardUrl();
}



LRESULT CMainFrame::OnSelectUserFolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	MtlSendCommand(m_ExplorerBar.m_FavBar.m_hWnd, ID_SELECT_USERFOLDER);
	return 0;
}



// ===========================================================================
// �A�h���X�o�[


// U.H
void CMainFrame::OnGoAddressBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
	m_AddressBar.SetFocus();
}


LRESULT CMainFrame::OnViewGoButton(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	// �g�O��
	m_AddressBar.ShowGoButton(!CAddressBarOption::s_bGoBtnVisible);

	return 0;
}


LRESULT CMainFrame::OnViewAddBarDropDown(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	if ( m_AddressBar.GetDroppedStateEx() )
		m_AddressBar.ShowDropDown(FALSE);
	else
		m_AddressBar.ShowDropDown(TRUE);

	return 0;
}


void CMainFrame::OnSetFocusAddressBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl)
{
	::SetFocus( m_AddressBar.GetEditCtrl() );
}





// ===========================================================================
// ����

CString CMainFrame::GetActiveSelectedText()
{
	CChildFrame *pChild = GetActiveChildFrame();
	if (pChild)
		return pChild->GetSelectedTextLine();
	return CString();
}


LRESULT CMainFrame::OnCommandDirect(int nCommand, LPCTSTR lpstr)
{
	CString str(lpstr);

	if (str.Find( _T("tp://") ) != -1
	  #if 1	//+++
		|| str.Find( _T("https://") ) != -1
		|| str.Find( _T("file://") ) != -1
//|| str.Left(11) == _T("javascript:")		//*+++ ����
	  #endif
	){
		Misc::StrToNormalUrl(str);		//+++
		DonutOpenFile( m_hWnd, str, DonutGetStdOpenFlag() );
	} else {
		if (nCommand == ID_SEARCH_DIRECT) {				// ������
			CEdit edit = m_SearchBar.GetEditCtrl();
			edit.SendMessage(WM_CHAR, 'P');
			edit.SetWindowText(lpstr);
			m_SearchBar.SearchWeb();
		} else if (nCommand == ID_OPENLINK_DIRECT) {	// �����N���J��
			SendMessage(WM_COMMAND, ID_EDIT_OPEN_SELECTED_REF, 0);
		}
	}

	return 0;
}



LRESULT CMainFrame::OnSearchBarCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	HWND	hWndActive = MDIGetActive();

	if (hWndActive == NULL)
		return 0;

	CEdit	edit	   = m_SearchBar.GetEditCtrl();
	CString str;	//+++   = MtlGetWindowText(edit);
	str = _GetSelectText(edit);

	if ( str.IsEmpty() )
		return 0;

	switch (wID) {
	case ID_SEARCHBAR_SEL_UP:
		m_SearchBar.SearchPage(FALSE);
		//SendMessage(hWndActive, WM_USER_FIND_KEYWORD, (WPARAM)str.GetBuffer(0), (LPARAM)FALSE);
		break;

	case ID_SEARCHBAR_SEL_DOWN:
		m_SearchBar.SearchPage(TRUE);
		//SendMessage(hWndActive, WM_USER_FIND_KEYWORD, (WPARAM)str.GetBuffer(0), (LPARAM)TRUE);
		break;

	case ID_SEARCHBAR_HILIGHT:
		m_SearchBar.SearchHilight();
		//SendMessage(hWndActive, WM_USER_HILIGHT, (WPARAM)str.GetBuffer(0), (LPARAM)0);
		break;
	}

	return 0;
}


LRESULT CMainFrame::OnHilight(LPCTSTR lpszKeyWord)
{
	HWND hWndActive = MDIGetActive();

	if (hWndActive == NULL)
		return 0;

	return SendMessage(hWndActive, WM_USER_HILIGHT, (WPARAM) lpszKeyWord, 0);
}

/// �����̂Ƃ���Flags�w��͈Ӗ��Ȃ�
LRESULT CMainFrame::OnFindKeyWord(LPCTSTR lpszKeyWord, BOOL bBack, long Flags /*= 0*/)
{
	HWND hWndActive = MDIGetActive();

	if (hWndActive == NULL)
		return 0;

	return SendMessage(hWndActive, WM_USER_FIND_KEYWORD, (WPARAM) lpszKeyWord, (LPARAM) bBack);
}


// U.H
LRESULT CMainFrame::OnViewSearchBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	MtlToggleBandVisible(m_hWndToolBar, IDC_SEARCHBAR);
	UpdateLayout();
	return 0;
}


LRESULT CMainFrame::OnSearchHistory(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	MtlSendCommand(m_ExplorerBar.m_FavBar.m_hWnd, ID_SEARCH_HISTORY);
	return 0;
}


void CMainFrame::OnSetFocusSearchBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl)
{
	::SetFocus( m_SearchBar.GetEditCtrl() );
}


void CMainFrame::OnSetFocusSearchBarEngine(WORD /*wNotifyCode*/, WORD /*wID*/, HWND hWndCtl)
{ //minit
	m_SearchBar.SetFocusToEngine();
}


LRESULT CMainFrame::OnSpecialRefreshSearchEngine(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
	m_SearchBar.RefreshEngine();
	return 0;
}

/// �͈͑I�����ꂽ�e�L�X�g�Ō�������H
HRESULT CMainFrame::OnSearchWebSelText(LPCTSTR lpstrText, LPCTSTR lpstrEngine)
{
	//BOOL	bFirst	 = (DonutGetStdOpenCreateFlag() & D_OPENFILE_ACTIVATE) != 0;	//+++ ? TRUE : FALSE;

	m_SearchBar.SearchWebWithEngine(lpstrText, lpstrEngine);

	return S_OK;
}


//+++
/// �O���[�v�E���j���[�̌����G���W���w��ł̌���
void CMainFrame::OnSearchWeb_engineId(UINT code, int id, HWND hWnd)
{
	ATLASSERT(ID_INSERTPOINT_SEARCHENGINE <= id && id <= ID_INSERTPOINT_SEARCHENGINE_END);

	m_SearchBar.SearchWebWithIndex(GetActiveSelectedText(), id - ID_INSERTPOINT_SEARCHENGINE);
}


//-----------------------------------------
/// �Ǝ��y�[�W�������o�[��\������
void	CMainFrame::SetFocusToSearchBarWithSelectedText()
{
	if (CMainOption::s_bUseCustomFindBar) 
		m_FindBar.ShowFindBar(GetActiveSelectedText());
}



// ==================================================================================

/////////////////////////////////////////////
// DonutP API
/////////////////////////////////////////////


IDispatch *CMainFrame::ApiGetDocumentObject(int nTabIndex)
{
	HWND		 hTabWnd = m_MDITab.GetTabHwnd(nTabIndex);
	if (hTabWnd == NULL)
		return NULL;

	CWebBrowser2 browser = DonutGetIWebBrowser2(hTabWnd);

	IDispatch *  spDocument = 0;
	browser.m_spBrowser->get_Document(&spDocument);
	return (IDispatch *) spDocument;
}


IDispatch *CMainFrame::ApiGetWindowObject(int nTabIndex)
{
	HWND					  hTabWnd = m_MDITab.GetTabHwnd(nTabIndex);
	if (hTabWnd == NULL)
		return NULL;

	CWebBrowser2			  browser = DonutGetIWebBrowser2(hTabWnd);

	CComPtr<IDispatch>		  spDisp  = 0;
	HRESULT 				  hr	  = browser.m_spBrowser->get_Document(&spDisp);
	CComQIPtr<IHTMLDocument2> spDoc   = spDisp;

	IHTMLWindow2 *			  spWnd   = 0;
	spDoc->get_parentWindow(&spWnd);
	return (IDispatch *) spWnd;
}


IDispatch *CMainFrame::ApiGetWebBrowserObject(int nTabIndex)
{
	HWND		  hTabWnd	= m_MDITab.GetTabHwnd(nTabIndex);

	if (hTabWnd == NULL)
		return NULL;

	CWebBrowser2  browser	= DonutGetIWebBrowser2(hTabWnd);

	IWebBrowser2 *spBrowser = 0;
	browser.m_spBrowser->QueryInterface(IID_IWebBrowser2, (void **) &spBrowser);
	return (IDispatch *) spBrowser;
}


long CMainFrame::ApiGetTabIndex()
{
	return m_MDITab.GetCurSel();
}


void CMainFrame::ApiSetTabIndex(int nTabIndex)
{
	m_MDITab.SetCurSel(nTabIndex);
}


void CMainFrame::ApiClose(int nTabIndex)
{
	HWND hTabWnd = m_MDITab.GetTabHwnd(nTabIndex);

	::SendMessage(hTabWnd, WM_COMMAND, ID_FILE_CLOSE, 0);
}


long CMainFrame::ApiGetTabCount()
{
	return m_MDITab.GetItemCount();
}


void CMainFrame::ApiMoveToTab(int nBeforIndex, int nAfterIndex)
{
	CSimpleArray<int> aryBefor;
	aryBefor.Add(nBeforIndex);
	m_MDITab.MoveItems(nAfterIndex + 1, aryBefor);
}


int CMainFrame::ApiNewWindow(BSTR bstrURL, BOOL bActive)
{
	CString 	 strURL(bstrURL);

	int 		 nCmdShow = bActive ? -1 : SW_SHOWNOACTIVATE;

	if (m_MDITab.GetItemCount() == 0)
		nCmdShow = -1;

	CChildFrame *pChild   = CChildFrame::NewWindow(m_hWndMDIClient, m_MDITab, m_AddressBar);
	if (pChild == NULL)
		return -1;

	pChild->ActivateFrame(nCmdShow);

	if ( strURL.IsEmpty() )
		strURL = _T("about:blank");

	pChild->SetWaitBeforeNavigate2Flag();			//+++ �������ABeforeNavigate2()�����s�����܂ł̊ԃA�h���X�o�[���X�V���Ȃ��悤�ɂ���t���O��on
	pChild->Navigate2(strURL);
	return m_MDITab.GetTabIndex(pChild->m_hWnd);
}


void CMainFrame::ApiShowPanelBar()
{
	if ( m_ExplorerBar.IsPanelBarVisible() )
		return;

	SendMessage(WM_COMMAND, ID_VIEW_PANELBAR, 0);
}


long CMainFrame::ApiGetTabState(int nIndex)
{
	DWORD state = 0;
	m_MDITab.GetItemState(nIndex, state);
	if (state == 0)
		return -1;

	long nRet	 = 0;

	if (state & TISS_SELECTED) {
		nRet = 1;
	} else if (state & TISS_MSELECTED) {
		nRet = 2;
	}

	return nRet;
}


IDispatch *CMainFrame::ApiGetPanelWebBrowserObject()
{
	if ( !m_ExplorerBar.m_PanelBar.IsWindow() )
		m_ExplorerBar.m_PanelBar.CreatePanelBar(m_hWnd, FALSE);

	return m_ExplorerBar.m_PanelBar.GetPanelWebBrowserObject();
}


IDispatch *CMainFrame::ApiGetPanelWindowObject()
{
	if ( !m_ExplorerBar.m_PanelBar.IsWindow() )
		m_ExplorerBar.m_PanelBar.CreatePanelBar(m_hWnd, FALSE);

	return m_ExplorerBar.m_PanelBar.GetPanelWindowObject();
}


IDispatch *CMainFrame::ApiGetPanelDocumentObject()
{
	if ( !m_ExplorerBar.m_PanelBar.IsWindow() )
		m_ExplorerBar.m_PanelBar.CreatePanelBar(m_hWnd, FALSE);

	return m_ExplorerBar.m_PanelBar.GetPanelDocumentObject();
}


//IAPI2 by minit
void CMainFrame::ApiExecuteCommand(int nCommand)
{
	::SendMessage(m_hWnd, WM_COMMAND, (WPARAM) (nCommand & 0xFFFF), 0);
}


void CMainFrame::ApiGetSearchText( /*[out, retval]*/ BSTR *bstrText)
{
	CString strBuf;
	CEdit	edit = m_SearchBar.GetEditCtrl();
	int 	len  = edit.GetWindowTextLength() + 1;

	edit.GetWindowText(strBuf.GetBuffer(len), len);
	strBuf.ReleaseBuffer();
	*bstrText = CComBSTR(strBuf).Copy();
}


void CMainFrame::ApiSetSearchText(BSTR bstrText)
{
	CString strText = bstrText;
	CEdit	edit	= m_SearchBar.GetEditCtrl();

	edit.SendMessage(WM_CHAR, 'P');
	edit.SetWindowText(strText);
}


void CMainFrame::ApiGetAddressText( /*[out, retval]*/ BSTR *bstrText)
{
	CString strBuf;
	CEdit	edit = m_AddressBar.GetEditCtrl();
	int 	len  = edit.GetWindowTextLength() + 1;

	edit.GetWindowText(strBuf.GetBuffer(len), len);
	strBuf.ReleaseBuffer();
	*bstrText = CComBSTR(strBuf).Copy();
}


void CMainFrame::ApiSetAddressText(BSTR bstrText)
{
	CString strText = bstrText;
	CEdit	edit	= m_AddressBar.GetEditCtrl();

	edit.SendMessage(WM_CHAR, 'P');
	edit.SetWindowText(strText);
}


LRESULT CMainFrame::ApiGetExtendedTabState(int nIndex)
{
	HWND hWndChild = m_MDITab.GetTabHwnd(nIndex);

	if ( !::IsWindow(hWndChild) )
		return 0x80000000;

	return ::SendMessage(hWndChild, WM_GET_EXTENDED_TABSTYLE, 0, 0);
}


void CMainFrame::ApiSetExtendedTabState(int nIndex, long nState)
{
	HWND hWndChild = m_MDITab.GetTabHwnd(nIndex);

	if ( !::IsWindow(hWndChild) )
		return;

	::SendMessage(hWndChild, WM_SET_EXTENDED_TABSTYLE, (WPARAM) nState, 0);
}


LRESULT CMainFrame::ApiGetKeyState(int nKey)
{
	//return (::GetAsyncKeyState(nKey) & 0x80000000) != 0;		//+++ ���ꂾ�ƁA�����X�N���v�g�ŕs��ł邱�Ƃ��邩���Ȃ�ŁA�Ԍ^�̂ق�������ɍ��킹��.
	return (::GetAsyncKeyState(nKey) & 0x80000000);
}


long CMainFrame::ApiGetProfileInt(BSTR bstrFile, BSTR bstrSection, BSTR bstrKey, int nDefault)
{
	CString 	strFile    = bstrFile;
	CString 	strSection = bstrSection;
	CString 	strKey	   = bstrKey;
	CIniFileI	pr(strFile, strSection);
	return (long)pr.GetValue( strKey, nDefault );
}


void CMainFrame::ApiWriteProfileInt(BSTR bstrFile, BSTR bstrSection, BSTR bstrKey, int nValue)
{
	CString 	strFile    = bstrFile;
	CString 	strSection = bstrSection;
	CString 	strKey	   = bstrKey;
	CIniFileO	pr(strFile, strSection);
	pr.SetValue(nValue, strKey);
	//x pr.Close(); 	//+++
}


void CMainFrame::ApiGetProfileString(BSTR bstrFile, BSTR bstrSection, BSTR bstrKey, BSTR bstrDefault, /*[out, retval]*/ BSTR *bstrText)
{
	CString 	strFile 	= bstrFile;
	CString 	strSection	= bstrSection;
	CString 	strKey		= bstrKey;
	CString 	strDefault	= bstrDefault;
	CIniFileI	pr(strFile, strSection);
	CString 	strBuf		= pr.GetStringUW(strKey, strDefault, 0xFFFFFFFF); //+++ size=0xFFFFFFFF�̏ꍇ�̓o�b�t�@�T�C�Y�g������Ŏ擾.
	*bstrText = CComBSTR(strBuf).Copy();
}


void CMainFrame::ApiWriteProfileString(BSTR bstrFile, BSTR bstrSection, BSTR bstrKey, BSTR bstrText)
{
	CString 		strFile    = bstrFile;
	CString 		strSection = bstrSection;
	CString 		strKey	   = bstrKey;
	CString 		strText    = bstrText;
	CIniFileO	pr(strFile, strSection);
	pr.SetStringUW( strText, strKey );
}


void CMainFrame::ApiGetScriptFolder( /*[out, retval]*/ BSTR *bstrFolder)
{
	ATLASSERT(bstrFolder != 0);
	CString 	strBuf = Misc::GetExeDirectory() + _T("Script\\");
	if (::GetFileAttributes(strBuf) == 0xFFFFFFFF)
		strBuf.Empty();

	*bstrFolder = CComBSTR(strBuf).Copy();
}


void CMainFrame::ApiGetCSSFolder( /*[out, retval]*/ BSTR *bstrFolder)
{
	ATLASSERT(bstrFolder != 0);
	CString 	strBuf = Misc::GetExeDirectory() + _T("CSS\\");
	if (::GetFileAttributes(strBuf) == 0xFFFFFFFF)
		strBuf.Empty();

	*bstrFolder = CComBSTR(strBuf).Copy();
}


void CMainFrame::ApiGetBaseFolder( /*[out, retval]*/ BSTR *bstrFolder)
{
	ATLASSERT(bstrFolder != 0);
	CString 	strBuf = Misc::GetExeDirectory();
	if (::GetFileAttributes(strBuf) == 0xFFFFFFFF)
		strBuf.Empty();

	*bstrFolder = CComBSTR(strBuf).Copy();
}


void CMainFrame::ApiGetExePath( /*[out, retval]*/ BSTR *bstrPath)
{
	ATLASSERT(bstrPath != 0);
	TCHAR Buf[MAX_PATH];
	Buf[0]		= 0;	//+++
	::GetModuleFileName(_Module.GetModuleInstance(), Buf, MAX_PATH);
	*bstrPath 	= CComBSTR(Buf).Copy();
}


void CMainFrame::ApiSetStyleSheet(int nIndex, BSTR bstrStyleSheet, BOOL bOff)
{
	HWND		 hWndChild = m_MDITab.GetTabHwnd(nIndex);

	if ( !::IsWindow(hWndChild) )
		return;

	CChildFrame *pChild    = GetChildFrame(hWndChild);
	if (pChild == NULL)
		return;

	CString 	 strStylePath(bstrStyleSheet);
	CString 	 strStyleTitle;

	if ( bOff || strStylePath.IsEmpty() ) {
		// Off or Default
	} else {
		// Set New StyleSheet
		strStyleTitle = strStylePath.Mid(strStylePath.ReverseFind('\\') + 1);
	}

	pChild->StyleSheet(strStyleTitle, bOff, strStylePath);
}


//IAPI3
void CMainFrame::ApiSaveGroup(BSTR bstrGroupFile)
{
	CString strFile = bstrGroupFile;

	_SaveGroupOption(strFile);
}


void CMainFrame::ApiLoadGroup(BSTR bstrGroupFile, BOOL bClose)
{
	CString strFile = bstrGroupFile;

	_LoadGroupOption(strFile, bClose != 0/*? true : false*/);
}


//private:
///+++ �v���O�C��������n�����ini�t�@�C�����...
struct CMainFrame::_ExtendProfileInfo {
	/*const*/LPCTSTR	lpstrIniPath;
	/*const*/LPCTSTR	lpstrSection;
	BOOL			  	bGroup;
};


//public:
int CMainFrame::ApiNewWindow3(BSTR bstrURL, BOOL bActive, long ExStyle, void *pExInfo)
{
	CString 	 strURL   = bstrURL;

	int 		 nCmdShow = bActive ? -1 : SW_SHOWNOACTIVATE;

	if (m_MDITab.GetItemCount() == 0)
		nCmdShow = -1;

	CChildFrame *pChild   = CChildFrame::NewWindow(m_hWndMDIClient, m_MDITab, m_AddressBar);

	if (pChild == NULL)
		return -1;

	pChild->ActivateFrame(nCmdShow);

	if ( strURL.IsEmpty() )
		strURL = _T("about:blank");

	if (pExInfo && *(INT_PTR *) pExInfo) {
		_ExtendProfileInfo *_pExInfo    = (_ExtendProfileInfo *) pExInfo;
	  #ifdef UNICODE
		const char*			s			= (const char*)_pExInfo->lpstrIniPath;
		const char*			t			= (const char*)_pExInfo->lpstrSection;
		CString 			strPath;
		CString 			strSection;
		if (*s && s[1] == 0) {	//+++ �p�X���̍ŏ��͂قڔ��p���낤�ŁA���A1�o�C�g��path�͂Ȃ����낤�Ƃ��āAutf16�Ɣ��f.
			strPath     = (TCHAR*)s;
			strSection  = (TCHAR*)t;
		} else {				//+++ �v���O�C������MB�łŃR���p�C������Ă���ꍇ���l��.
			strPath     = (char*)s;
			strSection  = (char*)t;
		  #if 0	//+++ �t�@�C�����Ȃ��� URL �Ȃ񂾂���A�܂���
			if (Misc::IsExistFile(strPath) == 0) {	// �t�@�C�������݂��Ȃ��ꍇ�́A����ς�utf16�Ƃ��Ĉ����Ă݂�.
				strPath     = (TCHAR*)s;
				strSection  = (TCHAR*)t;
			}
		  #endif
		}
	  #else
		CString 			strPath     = _pExInfo->lpstrIniPath;
		CString 			strSection  = _pExInfo->lpstrSection;
	  #endif
		try {
			_Load_OptionalData(pChild, strPath, strSection);
		} catch (...) {
			ErrorLogPrintf(_T("ApiNewWindow3"));
			MessageBox( _T("��O�G���[���������܂����B(In NewWindow3 Function)\n")
						_T("���S�̂��߃v���O�������ċN�������Ă��������B")			);
		}

		if (_pExInfo->bGroup) {
			pChild->view().m_ViewOption._GetProfile(strPath, strSection, !CMainOption::s_bTabMode);
			//+++ _GetProfile���� Navigate2 ���Ă���̂ŁA����ł܂�Navigate2����K�v�Ȃ�����A�����ŋA���Ă���
			return m_MDITab.GetTabIndex(pChild->m_hWnd);
		} else {
			pChild->OnSetExtendedTabStyle(ExStyle | CChildFrame::FLAG_SE_NOREFRESH);
		}
	} else {
		pChild->OnSetExtendedTabStyle(ExStyle | CChildFrame::FLAG_SE_NOREFRESH);
	}

	pChild->SetWaitBeforeNavigate2Flag();			//+++ �������ABeforeNavigate2()�����s�����܂ł̊ԃA�h���X�o�[���X�V���Ȃ��悤�ɂ���t���O��on
	pChild->Navigate2(strURL);

	return m_MDITab.GetTabIndex(pChild->m_hWnd);
}


long CMainFrame::ApiAddGroupItem(BSTR bstrGroupFile, int nIndex)
{
	HWND		 hWndChild = m_MDITab.GetTabHwnd(nIndex);

	if ( !::IsWindow(hWndChild) )
		return -1;

	CChildFrame *pChild    = GetChildFrame(hWndChild);

	if (pChild == NULL)
		return -1;

	return pChild->_AddGroupOption(bstrGroupFile);
}


long CMainFrame::ApiDeleteGroupItem(BSTR bstrGroupFile, int nIndex)
{
	CString 		strGroupFile = bstrGroupFile;
	{
		CString 	strSection;
		strSection.Format(_T("Window%d"), nIndex);
		CIniFileO	pr(strGroupFile, strSection);
		bool		bRet = pr.DeleteSection();
		pr.Close();
		if (!bRet) {
			MessageBox(_T("Error1: �Z�N�V�����̍폜�Ɏ��s���܂����B"));
			return 0;
		}
	}

	std::list<CString>	buf;
	bool bRet = FileReadString(strGroupFile, buf);
	if (!bRet) {
		MessageBox(_T("Error2: �t�@�C���̓ǂݍ��݂Ɏ��s���܂����B"));
		return 0;
	}

	std::list<CString>::iterator str;

	for (str = buf.begin(); str != buf.end(); ++str) {
		CString strCheck = str->Left(7);
		strCheck.MakeUpper();

		if ( strCheck == _T("[WINDOW") ) {
			int nSecIndex = _ttoi( str->Mid(7, str->GetLength() - 7 - 1) );
			if (nSecIndex > nIndex) {
				str->Format(_T("[Window%d]"), nSecIndex - 1);
			}
		}
	}

	bRet = FileWriteString(strGroupFile, buf);
	if (!bRet) {
		MessageBox(_T("Error3: �t�@�C���̏������݂Ɏ��s���܂����B"));
		return 0;
	}

	DWORD		dwCount = 0, dwActive = 0, dwMaximize = 0;
	CIniFileIO	pr( strGroupFile, _T("Header") );
	pr.QueryValue( dwCount, _T("Count") );
	if (dwCount) {
		dwCount--;
		pr.SetValue( dwCount, _T("Count") );
	}

	pr.QueryValue( dwActive, _T("active") );
	if (dwCount == 0)
		pr.DeleteValue( _T("active") );
	else if ( (int) dwActive > nIndex )
		pr.SetValue( dwActive - 1, _T("active") );

	pr.QueryValue( dwMaximize, _T("maximized") );

	if (dwCount == 0)
		pr.DeleteValue( _T("maximized") );
	else if ( (int) dwMaximize > nIndex )
		pr.SetValue( dwMaximize - 1, _T("maximized") );

	return 1;
}


//IAPI4
HWND CMainFrame::ApiGetHWND(long nType)
{
	if (nType == 0)
		return m_hWnd;
	return NULL;
}

