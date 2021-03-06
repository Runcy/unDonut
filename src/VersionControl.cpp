// VersionControl.cpp

#include "stdafx.h"
#include "VersionControl.h"
#include "IniFile.h"
#include "DonutPFunc.h"
#include "DonutOptions.h"


///////////////////////////////////////////////////////////////////
// CVersionControl

void	CVersionControl::Run()
{
	CIniFileIO	pr(g_szIniFileName, _T("Version"));

	// ファイルが存在しなければ各GetProfileに任せる
	if (::PathFileExists(g_szIniFileName)) {
		DWORD dwVersion = pr.GetValue(_T("IniVersion"), 0);
		if (dwVersion != LATESTVERSION)
			Misc::CopyToBackupFile(g_szIniFileName);

		switch (dwVersion) {
		case 0:	_0to1();
		case 1: _1to2();
		case 2: _2to3();
		case 3: _3to4();
		case 4: _4to5();
		case 5: _5to6();
		case 6: _6to7();
		case 7: _7to8();
		case 8: _8to9();
		case 9: _9to10();
		case 10: _10to11();
		case 11: _11to12();
		case 12: _12to13();
		case 13: _13to14();
			break;
		}
	}
	// 最新バージョンを書き込む
	pr.SetValue(LATESTVERSION, _T("IniVersion"));
	pr.Close();
}


void	CVersionControl::_0to1()
{
	{	// for CMenuOption
		enum { MAIN_EX2_NOCSTMMENU 			= 0x00000001L };

		/* iniから設定を読み込む */
		CIniFileIO	donutProfile(g_szIniFileName, _T("Main"));
		DWORD dwMainExStyle2	= donutProfile.GetValue(_T("Extended_Style2"), TRUE);
		DWORD dwNewMainExStyle2 = (dwMainExStyle2 & ~MAIN_EX2_NOCSTMMENU);
		donutProfile.SetValue(dwNewMainExStyle2, _T("Extended_Style2"));	// 設定から取り除く
		donutProfile.ChangeSectionName(_T("ETC"));
		DWORD dwIeMenuNoCstm	= donutProfile.GetValue(_T("IeMenuNoCstm"), TRUE);
		DWORD dwMenuBarStyle	= donutProfile.GetValue(_T("MenuBarStyle"), 0 );

		CString strMenuPath = _GetFilePath(_T("Menu.ini"));
		CIniFileIO	menuProfile(strMenuPath, _T("Option"));
		DWORD dwR_Equal_L		= menuProfile.GetValue(_T("REqualL"));
		DWORD dwDontShowButton	= menuProfile.GetValue(_T("NoButton"));
		menuProfile.RemoveFileToBak();	// Menu.iniはもう使わないので

		/* iniから設定を削除する */
		donutProfile.DeleteValue(_T("IeMenuNoCstm"));
		donutProfile.DeleteValue(_T("MenuBarStyle"));

		/* 新しい設定を書き込む */
		DWORD dwExStyle = 0;
		dwExStyle |= ((dwMainExStyle2 & MAIN_EX2_NOCSTMMENU) != 0) ? MENU_EX_NOCUSTOMMENU : 0;
		dwExStyle |= dwIeMenuNoCstm		? MENU_EX_NOCUSTOMIEMENU	: 0;
		dwExStyle |= dwR_Equal_L		? MENU_EX_R_EQUAL_L			: 0;
//		dwExStyle |= dwDontShowButton	? MENU_EX_DONTSHOWBUTTON	: 0;

		donutProfile.ChangeSectionName(_T("Menu"));
		donutProfile.SetValue(dwExStyle			, _T("ExStyle"));
		donutProfile.SetValue(dwMenuBarStyle	, _T("MenuBarStyle"));
	}
}

void	CVersionControl::_1to2()
{
	enum {
		OLD_EMS_ADDITIONALMENUITEMNOSEP	= 0x00000002L,
		OLD_EMS_DEL_NOASK				= 0x00000010L,
		OLD_EMS_ORDER_FAVORITES 		= 0x00010000L,
		OLD_EMS_USER_DEFINED_FOLDER		= 0x00040000L,

		EMS_IE_ORDER				= 0x00000002L,	
		EMS_USER_DEFINED_FOLDER		= 0x00000100L,
	};
	CIniFileIO	pr(g_szIniFileName, _T("FavoritesMenu"));
	DWORD	dwStyle = pr.GetValue(_T("Style"));
	dwStyle &= ~OLD_EMS_ADDITIONALMENUITEMNOSEP;
	dwStyle &= ~OLD_EMS_DEL_NOASK;
	DWORD	dwNewStyle = dwStyle;
	if (dwStyle & OLD_EMS_ORDER_FAVORITES) {
		dwNewStyle &= ~OLD_EMS_ORDER_FAVORITES;
		dwNewStyle |= EMS_IE_ORDER;
	}
	if (dwStyle & OLD_EMS_USER_DEFINED_FOLDER) {
		dwNewStyle &= ~OLD_EMS_USER_DEFINED_FOLDER;
		dwNewStyle |= EMS_USER_DEFINED_FOLDER;
	}

	pr.SetValue(dwNewStyle, _T("Style"));
	
}

void	CVersionControl::_2to3()
{
	CIniFileIO	pr(g_szIniFileName, _T("SEARCH"));
	pr.DeleteValue(_T("Show_ToolBarIcon"));
}

void	CVersionControl::_3to4()
{
	CIniFileIO pr(g_szIniFileName, _T("Main"));
	pr.QueryValue(CMainOption::s_dwMainExtendedStyle, _T("Extended_Style"));
	CMainOption::s_dwMainExtendedStyle |= MAIN_EX_EXTERNALNEWTAB | MAIN_EX_EXTERNALNEWTABACTIVE;
	pr.SetValue(CMainOption::s_dwMainExtendedStyle, _T("Extended_Style"));
}

void	CVersionControl::_4to5()
{
	CIniFileIO	pr(g_szIniFileName, _T("LinkBar"));
	pr.DeleteValue(_T("ExtendedStyle"));
}

void	CVersionControl::_5to6()
{
	CIniFileIO pr(g_szIniFileName, _T("Main"));

	enum { MAIN_EX_NOMDI = 0x00000040L };
	DWORD dwExStyle = pr.GetValue(_T("Extended_Style"), CMainOption::s_dwMainExtendedStyle);
	dwExStyle &= ~MAIN_EX_NOMDI;
	pr.SetValue(dwExStyle, _T("Extended_Style"));

	pr.ChangeSectionName(_T("UrlSecurity"));
	pr.DeleteValue(_T("ActPagNavi"));
}

void	CVersionControl::_6to7()
{
	CIniFileIO pr(g_szIniFileName, _T("Main"));
	pr.DeleteValue(_T("Max_Window_Count"));

	enum {  
		MAIN_EX_WINDOWLIMIT = 0x00000010L, 
		MAIN_EX_LOADGLOBALOFFLINE = 0x00004000L,
		MAIN_EX_KILLDIALOG		= 0x00008000L
	};
	DWORD dwExStyle = pr.GetValue(_T("Extended_Style"), CMainOption::s_dwMainExtendedStyle);
	dwExStyle &= ~(MAIN_EX_WINDOWLIMIT | MAIN_EX_LOADGLOBALOFFLINE | MAIN_EX_KILLDIALOG);
	pr.SetValue(dwExStyle, _T("Extended_Style"));
}

/// 設定ファイルを全部 Configフォルダ以下に移動させる
void	CVersionControl::_7to8()
{
	auto funcMoveFileToConfigFolder = [](LPCTSTR configfile) {
		::MoveFileEx(_GetFilePath(configfile), GetConfigFilePath(configfile), MOVEFILE_REPLACE_EXISTING);
	};
	funcMoveFileToConfigFolder(_T("CloseTitle.ini"));
	funcMoveFileToConfigFolder(_T("CloseURL.ini"));
	funcMoveFileToConfigFolder(_T("Download.ini"));
	funcMoveFileToConfigFolder(_T("FavoriteBookmark.xml"));
	funcMoveFileToConfigFolder(_T("LinkBookmark.xml"));
	funcMoveFileToConfigFolder(_T("KeyBoard.ini"));
	::MoveFileEx(_GetFilePath(_T("Menu.xml")), GetConfigFilePath(_T("CustomContextMenu.xml")), MOVEFILE_REPLACE_EXISTING);
	funcMoveFileToConfigFolder(_T("MouseEdit.ini"));
	funcMoveFileToConfigFolder(_T("Proxy.ini"));
	funcMoveFileToConfigFolder(_T("RecentClosedTab.xml"));
	funcMoveFileToConfigFolder(_T("TabList.bak.xml"));
	funcMoveFileToConfigFolder(_T("TabList.xml"));
	funcMoveFileToConfigFolder(_T("UrlEntry.ini"));
	funcMoveFileToConfigFolder(_T("WordHistory.ini"));
	funcMoveFileToConfigFolder(_T("UserDefinedCSSConfig.xml"));
	funcMoveFileToConfigFolder(_T("UserDefinedJavascriptConfig.xml"));
}

/// TabList.xml -> TabList.donutTabList
void	CVersionControl::_8to9()
{
	::MoveFileEx(GetConfigFilePath(_T("TabList.xml")), GetConfigFilePath(_T("TabList.donutTabList")), MOVEFILE_REPLACE_EXISTING);
	::MoveFileEx(GetConfigFilePath(_T("TabList.bak.xml")), GetConfigFilePath(_T("TabList.bak.donutTabList")), MOVEFILE_REPLACE_EXISTING);
}


void	CVersionControl::_9to10()
{
	enum {
		MAIN_EXPLORER_HSCROLL		= 0x00000001L,
		MAIN_EXPLORER_NOSPACE		= 0x00000002L,
		MAIN_EXPLORER_FAV_ORGIMG	= 0x00000004L,
		MAIN_EXPLORER_NODRAGDROP	= 0x00000008L,
		MAIN_EXPLORER_AUTOSHOW		= 0x00000010L,
	};
	CIniFileIO	pr( g_szIniFileName, _T("Main") );
	DWORD dwExplorerBarStyle = 0;
	pr.QueryValue( dwExplorerBarStyle 	, _T("ExplorerBar_Style")	);	// UH
	if (dwExplorerBarStyle & MAIN_EXPLORER_AUTOSHOW)
		dwExplorerBarStyle = EXPLORERBAROPTION_AUTOSHOW;
	pr.DeleteValue(_T("ExplorerBar_Style"));

	pr.ChangeSectionName(_T("ExplorerBar"));
	pr.SetValue(dwExplorerBarStyle, _T("ExplorerBar_Style"));

	pr.ChangeSectionName(_T("Explorer_Bar"));
	pr.DeleteSection();
}


void	CVersionControl::_10to11()
{
	enum { 
		MAIN_EX_ADDFAVORITEOLDSHELL 	= 0x00000800L,
		MAIN_EX_ORGFAVORITEOLDSHELL 	= 0x00001000L,
	};
	CIniFileIO	pr( g_szIniFileName, _T("Main") );
	pr.QueryValue(CMainOption::s_dwMainExtendedStyle, _T("Extended_Style"));
	CMainOption::s_dwMainExtendedStyle &= ~(MAIN_EX_ADDFAVORITEOLDSHELL | MAIN_EX_ORGFAVORITEOLDSHELL);
	pr.SetValue(CMainOption::s_dwMainExtendedStyle, _T("Extended_Style"));
}


void	CVersionControl::_11to12()
{
	CIniFileIO	pr( g_szIniFileName, _T("StartUp") );
	pr.DeleteValue(_T("StartUp_With_Param"));

	pr.ChangeSectionName(_T("Main"));
	pr.DeleteValue(_T("TravelLogGroup"));
	pr.DeleteValue(_T("TravelLogClose"));

	pr.ChangeSectionName(_T("Confirmation") );
	pr.DeleteValue( _T("Script") );

	pr.ChangeSectionName( _T("Menu") );
	pr.DeleteValue( _T("MenuBarStyle") );
	enum { 
		MENU_EX_DONTSHOWBUTTON	= 0x00000008L,	// メニューに最小化etc..ボタンを表示しない 
	};
	DWORD dwExStyle = pr.GetValue(_T("ExStyle"), 0);
	dwExStyle &= ~MENU_EX_DONTSHOWBUTTON;
	pr.SetValue(dwExStyle, _T("ExStyle"));

}


void	CVersionControl::_12to13()
{
	CIniFileO	pr(g_szIniFileName, _T("Browser"));
	pr.DeleteValue(_T("GPURenderStyle"));
}

void	CVersionControl::_13to14()
{
	CIniFileIO	pr(GetConfigFilePath(_T("Download.ini")), _T("ImageDLFolderHistory"));
	CString DLFinishSoundPath = pr.GetString(_T("DLFinishSoundFilePath"));
	pr.DeleteValue(_T("DLFinishSoundFilePath"));

	pr.ChangeSectionName(_T("Main"));
	pr.SetString(DLFinishSoundPath, _T("DLFinishSoundFilePath"));
}
