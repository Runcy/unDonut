/**
 *	@file	Donut.cpp
 *	@brief	main source file for Donut.exe
 */

#include "stdafx.h"
#include "Donut.h"
#include "AtlHostEx.h"
#include "initguid.h"
#include "DonutP.h"
#include "DonutP_i.c"

#include "ie_feature_control.h"
#include "MainFrame.h"								//+++ "MainFrm.h"
#include "API.h"

#ifdef USE_ATL3_BASE_HOSTEX /*_ATL_VER < 0x700*/	//+++
#include "for_ATL3/AtlifaceEx_i.c"
#endif

#include "VersionControl.h"


// Ini file name
TCHAR				g_szIniFileName[MAX_PATH];


extern const UINT	g_uDropDownCommandID[] = {
	ID_FILE_NEW,
	ID_VIEW_BACK,
	ID_VIEW_FORWARD,
	ID_FILE_NEW_CLIPBOARD2,
	ID_DOUBLE_CLOSE,
	ID_DLCTL_CHG_MULTI,
	ID_DLCTL_CHG_SECU,
	ID_URLACTION_COOKIES_CHG,
	ID_RECENT_DOCUMENT
};

extern const UINT	g_uDropDownWholeCommandID[] = {
	ID_VIEW_FONT_SIZE,
	ID_FAVORITES_DROPDOWN,
	ID_AUTO_REFRESH,
	ID_TOOLBAR,
	ID_EXPLORERBAR,
	ID_MOVE,
	ID_OPTION,
	ID_COOKIE_IE6,
	ID_FAVORITES_GROUP_DROPDOWN,
	ID_CSS_DROPDOWN
};


extern const int	g_uDropDownCommandCount 	 = sizeof (g_uDropDownCommandID 	) / sizeof (UINT);
extern const int	g_uDropDownWholeCommandCount = sizeof (g_uDropDownWholeCommandID) / sizeof (UINT);

bool				g_bNoReposition 		  = FALSE;

CServerAppModule	_Module;
CMainFrame *		g_pMainWnd				  = NULL;
CAPI *				g_pAPI					  = NULL;



BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_API, CAPI)
END_OBJECT_MAP()


static void CommandLineArg(CMainFrame& wndMain, LPTSTR lpstrCmdLine);

// �R�}���h���C������URL�����o��
static void PerseUrls(LPCTSTR lpszCommandline, std::vector<CString>& vecUrls)
{
	// �����o�[���g���Č�������
	CString str = lpszCommandline;
	if (str.Left(13) == _T("SearchEngine:")) {
		vecUrls.push_back(str);
		return ;
	}

	std::wstring	strCommandline = lpszCommandline;
	auto			itbegin = strCommandline.cbegin();
	auto			itend	= strCommandline.cend();
	std::wregex		rx(L"(?:\")([^\"]+)(?:\")");	// "�`"
	std::wsmatch	result;
	while (std::regex_search(itbegin, itend, result, rx)) {
		vecUrls.push_back(result[1].str().c_str());
		itbegin = result[0].second;
	}
	if (vecUrls.size() == 0) {
		strCommandline = lpszCommandline;
		itbegin = strCommandline.cbegin();
		itend	= strCommandline.cend();
		std::wregex		rx(L"([^ ]+)");
		std::wsmatch	result;
		while (std::regex_search(itbegin, itend, result, rx)) {
			vecUrls.push_back(result[1].str().c_str());
			itbegin = result[0].second;
		}
	}
}

//+++	�N����������ςȂ̃`�F�b�N.
static bool CheckOneInstance(HINSTANCE hInstance, LPTSTR lpstrCmdLine)
{
	//ChangeWindowMessageFilter(WM_NEWINSTANCE, MSGFLT_ADD); //+++ ���[�U�[���b�Z�[�W��ʉ߂����邽�߂�vista�ł̊J�����͕K�v?(�āA�������������Ȃ�����)
	TCHAR	iniFilePath[MAX_PATH+16] = _T("\0");
	::GetModuleFileName(hInstance, iniFilePath, MAX_PATH);
	size_t	l = ::_tcslen(iniFilePath);
	if (l < 5)
		return false;
	
	::_tcscpy(&iniFilePath[l-4], _T(".ini"));
	DWORD dwMainExtendedStyle = ::GetPrivateProfileInt(_T("Main"), _T("Extended_Style"), 0xABCD0123, iniFilePath);
	if (dwMainExtendedStyle != 0xABCD0123)
		CMainOption::s_dwMainExtendedStyle = dwMainExtendedStyle;
	if (CMainOption::s_dwMainExtendedStyle & MAIN_EX_ONEINSTANCE) { // �����N���������Ȃ�
		//x HWND hWnd = ::FindWindow(_T("WTL:Donut"), NULL);
		HWND hWnd = ::FindWindow(DONUT_WND_CLASS_NAME, NULL);
		if (hWnd) {
			std::vector<CString> strs;
			PerseUrls(lpstrCmdLine, strs);

			for (auto it = strs.cbegin(); it != strs.cend(); ++it) {
				ATOM nAtom = ::GlobalAddAtom(*it);
				::PostMessage(hWnd, WM_NEWINSTANCE, (WPARAM) nAtom, 0);
			}
			return true;
		}
	}
	return false;
}


// ini�t�@�C������ݒ��ǂݍ���
static bool _PrivateInit()
{
	MtlIniFileNameInit(g_szIniFileName, _MAX_PATH);

	CString strPath = Misc::GetExeDirectory() + _T("lock");
	do {
		if (::PathFileExists(strPath)) {
			int nReturn = ::MessageBox(NULL, 
				_T("unDonut�����S�ɏI�����Ă��܂���B\n")
				_T("�ݒ�̕ۑ����̉\��������܂��B\n")
				_T("�������ăv���Z�X�����s���܂����H\n")
				_T("(��������I�ԂƐݒ肪�ǂݍ��܂�Ȃ��\��������܂�)")
				, NULL, MB_ABORTRETRYIGNORE | MB_ICONWARNING);
			if (nReturn == IDABORT) {	// �I������
				return false;
			} else if (nReturn == IDRETRY) {
				continue;
			} else {
				::DeleteFile(strPath);	// �Ƃ肠���������Ă���
			}
		}
		break;

	} while(1);


	CVersionControl().Run();

	CMainOption::GetProfile();
	CAddressBarOption::GetProfile();	// �A�h���X�o�[
	CSearchBarOption::GetProfile();		// �����o�[
	CMenuOption::GetProfile();			// ���j���[
	CCustomContextMenuOption::GetProfile();
	CDLControlOption::GetProfile();
	CIgnoredURLsOption::GetProfile();
	CCloseTitlesOption::GetProfile();
	CFileNewOption::GetProfile();
	CStartUpOption::GetProfile();
	CUrlSecurityOption::GetProfile();
	CDonutConfirmOption::GetProfile();
	CStyleSheetOption::GetProfile();

	ie_feature_control_setting();

	CFavoritesMenuOption::GetProfile();
	CMouseOption::GetProfile();
	CSkinOption::GetProfile();
  #if 0	//+++ atltheme_d.h�̎g�p����߂�
	CThemeDLLLoader::LoadThemeDLL();
  #endif
	CExMenuManager::Initialize();

	return true;
}



// ini�t�@�C���ɐݒ��ۑ�����
void _PrivateTerm()
{
	CMainOption::WriteProfile();
	CDLControlOption::WriteProfile();
	CIgnoredURLsOption::WriteProfile();
	CCloseTitlesOption::WriteProfile();
	CUrlSecurityOption::WriteProfile();
	CFileNewOption::WriteProfile();
	CStartUpOption::WriteProfile();
	CDonutConfirmOption::WriteProfile();
	CStyleSheetOption::WriteProfile();

  #if 0	//+++ atltheme_d.h�̎g�p����߂�
	CThemeDLLLoader::UnLoadThemeDLL();
  #endif
	CExMenuManager::Terminate();

	// don't forget
	CHlinkDataObject::Term();

	ATLTRACE(_T("�ݒ�̕ۑ�����!\n"));
	CString strPath = Misc::GetExeDirectory() + _T("lock");
	::DeleteFile(strPath);
}





///+++ unDonut.ini�t�@�C�����Ȃ��ꍇ�ɁA���̊��t�@�C��������Ă��邩���`�F�b�N.
static bool	HaveEnvFiles()
{
	if (::PathFileExists( Misc::GetFullPath_ForExe(g_szIniFileName/*strIniFile*/) )) {
		return true;	// unDonut.ini ����������Ă�����A�Ƃ肠����ok�Ƃ��Ƃ�.
	}

	if (   ::PathFileExists( Misc::GetFullPath_ForExe(_T("MouseEdit.ini")))
		&& ::PathFileExists( Misc::GetFullPath_ForExe(_T("search\\search.ini")) )) {
		return true;
	}

	return false;
}


static int Run(LPTSTR lpstrCmdLine = NULL, int nCmdShow = SW_SHOWDEFAULT, bool bTray=false)
{
	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	CMainFrame	 wndMain;

	if (wndMain.CreateEx() == NULL) {
		ATLTRACE( _T("Main window creation failed!\n") );
		return 0;
	}

	// g_pMainWnd = &wndMain;	//+++ CreateEx()���Ƀv���O�C���������Ƃ��ŎQ�Ƃ����̂ŁACMainFreame�Őݒ肷��悤�ɕύX.

	// load windowplacement
	wndMain.startupMainFrameStayle(nCmdShow, bTray);

	_Module.Lock();

	if (lpstrCmdLine == 0 || lpstrCmdLine[0] == 0) {	// no command line param
		CStartUpOption::StartUp(wndMain);
	} else {
		if (CStartUpOption::s_dwParam) {
			CStartUpOption::StartUp(wndMain);
		}
		CommandLineArg(wndMain, lpstrCmdLine);
	}

  #if 1 //+++
	//\\ �폜��
   #if 0	//+++ CMainFrame�������ɍs���Ă����A�o�b�N�A�b�v�����̊J�n���A���̃^�C�~���O�ɂ��炵�Ă݂�.
	//+++ �X�^�[�g�y�[�W�̕\�����A��s���郁�b�Z�[�W���ɂ��΂��Ă���.
	//	  ...�Ȃ�ׂ��ŏ��̃y�[�W�\���̏���������ł���A�����X�V�J�n(�ɂȂ��Ăق���..�Ȃ��Ă��Ȃ������)
	if (ForceMessageLoop() == 0)
		return 0;						// ���̒i�K�ŏI�����ꂿ����Ă���A��
   #endif
	g_pMainWnd->SetAutoBackUp();		//�����X�V����Ȃ�A�J�n.

	//RtlSetMinProcWorkingSetSize();		//+++ ( �������̗\��̈���ꎞ�I�ɍŏ����B�E�B���h�E���ŏ��������ꍇ�Ɠ��� )

	// ���ۂ̃��C�����[�v.
	int nRet = theLoop.Run();

	_Module.RemoveMessageLoop();
  #else
	int nRet = theLoop.Run();
	_Module.RemoveMessageLoop();
  #endif

	return nRet;	// �� WTL�̃��C�����N���[�Y����1�𐳏�l�Ƃ��ĕԂ��Ă���̂Œ���.
}


static void CommandLineArg(CMainFrame& wndMain, LPTSTR lpstrCmdLine)
{
	CString 	 strCmdLine = lpstrCmdLine;
	if (strCmdLine.CompareNoCase( _T("/dde") ) != 0) {	
		// it's not from dde. (if dde, do nothing.)

		if (  (strCmdLine[0] == '-' || strCmdLine[0] == '/') 
			&& strCmdLine.Mid(1,4).CompareNoCase(_T("tray")) == 0)	//+++ -tray�I�v�V�������X�L�b�v
			return;

		std::vector<CString> vecUrls;
		PerseUrls(strCmdLine, vecUrls);

		std::for_each(vecUrls.cbegin(), vecUrls.cend(), [&wndMain](const CString& strUrl) {
			wndMain.OnUserOpenFile(strUrl, 0);
		});
	}
}




static HRESULT CreateComponentCategory(CATID catid, WCHAR *catDescription)
{
	ICatRegister *pcr = NULL;
	HRESULT 	  hr  = S_OK ;

	hr	= CoCreateInstance(CLSID_StdComponentCategoriesMgr, NULL, CLSCTX_INPROC_SERVER, IID_ICatRegister, (void **) &pcr);
	if ( FAILED(hr) )
		return hr;

	// Make sure the HKCR\Component Categories\{..catid...}
	// key is registered.
	CATEGORYINFO  catinfo;
	catinfo.catid	= catid;
	catinfo.lcid	= 0x0409 ; // english

	// Make sure the provided description is not too long.
	// Only copy the first 127 characters if it is.
	int len = (int) wcslen(catDescription);
	if (len > 127)
		len = 127;

	::wcsncpy(catinfo.szDescription, catDescription, len);

	// Make sure the description is null terminated.
	catinfo.szDescription[len] = '\0';

	hr	= pcr->RegisterCategories(1, &catinfo);
	pcr->Release();

	return hr;
}

static HRESULT RegisterCLSIDInCategory(REFCLSID clsid, CATID catid)
{
	// Register your component categories information.
	ICatRegister *pcr = NULL ;
	HRESULT 	  hr  = S_OK ;

	hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, NULL, CLSCTX_INPROC_SERVER, IID_ICatRegister, (void **) &pcr);
	if ( SUCCEEDED(hr) ) {
		// Register this category as being "implemented" by the class.
		CATID rgcatid[1] ;
		rgcatid[0] = catid;
		hr		   = pcr->RegisterClassImplCategories(clsid, 1, rgcatid);
	}

	if (pcr != NULL)
		pcr->Release();

	return hr;
}

static HRESULT UnRegisterCLSIDInCategory(REFCLSID clsid, CATID catid)
{
	ICatRegister *pcr = NULL ;
	HRESULT 	  hr  = S_OK ;

	hr = CoCreateInstance(CLSID_StdComponentCategoriesMgr, NULL, CLSCTX_INPROC_SERVER, IID_ICatRegister, (void **) &pcr);
	if ( SUCCEEDED(hr) ) {
		// Unregister this category as being "implemented" by the class.
		CATID rgcatid[1] ;
		rgcatid[0] = catid;
		hr		   = pcr->UnRegisterClassImplCategories(clsid, 1, rgcatid);
	}

	if (pcr != NULL)
		pcr->Release();

	return hr;
}

// �R�}���h���C����������COM�T�[�o�[�o�^/�폜���s��
static int RegisterCOMServer(int &nRet, bool &bRun, bool &bAutomation, bool &bTray)
{
	HRESULT hRes;
	TCHAR	szTokens[] = _T("-/");
	LPCTSTR lpszToken  = _Module.FindOneOf(::GetCommandLine(), szTokens);

	while (lpszToken) {
		CString strToken = Misc::GetStrWord(lpszToken, &lpszToken);
		if (strToken.CompareNoCase(_T("UnregServer") ) == 0) {
			nRet = _Module.UnregisterServer();
			nRet = UnRegisterCLSIDInCategory(CLSID_API, CATID_SafeForInitializing);
			if ( FAILED(nRet) )
				return nRet;

			nRet = UnRegisterCLSIDInCategory(CLSID_API, CATID_SafeForScripting);
			if ( FAILED(nRet) )
				return nRet;

			::MessageBox(NULL, _T("COM�T�[�o�[�폜���܂����B"), _T("unDonut"), 0);
			bRun = false;
			break;

		} else if (strToken.CompareNoCase(_T("RegServer") ) == 0) {
			nRet = _Module.RegisterServer(TRUE, &CLSID_API);
			if (nRet == S_OK) {
				// Mark the control as safe for initializing.
				hRes = CreateComponentCategory(CATID_SafeForInitializing, L"Controls safely initializable from persistent data");
				if ( FAILED(hRes) )
					return hRes;

				hRes = RegisterCLSIDInCategory(CLSID_API, CATID_SafeForInitializing);
				if ( FAILED(hRes) )
					return hRes;

				// Mark the control as safe for scripting.
				hRes = CreateComponentCategory(CATID_SafeForScripting, L"Controls that are safely scriptable");
				if ( FAILED(hRes) )
					return hRes;

				hRes = RegisterCLSIDInCategory(CLSID_API, CATID_SafeForScripting);
				if ( FAILED(hRes) )
					return hRes;
				::MessageBox(NULL, _T("COM�T�[�o�[�o�^���܂����B"), _T("unDonut"), 0);
			} else
				::MessageBox(NULL, _T("COM�T�[�o�[�o�^���s���܂����B"), _T("unDonut"), 0);

			bRun = false;
			break;

		} else if (strToken.CompareNoCase(_T("Automation")) == 0
			||    (strToken.CompareNoCase(_T("Embedding" )) == 0) )
		{
			bAutomation = true;
			break;
	  #if 1	//+++
		} else if (strToken.CompareNoCase(_T("tray") ) == 0) {
			bTray = true;
			break;
	  #endif
		}

		lpszToken = _Module.FindOneOf(lpszToken, szTokens);
	}

	return S_OK;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
// _tWinMain : EntryPoint

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
	// DLL�U���΍�
	SetDllDirectory(_T(""));

#if 0
//  #if defined (_DEBUG) && defined(_CRTDBG_MAP_ALLOC)
	//���������[�N���o�p
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF
				| _CRTDBG_CHECK_ALWAYS_DF
	);
	//_CrtSetBreakAlloc(874);
//  #endif

#ifdef _CRTDBG_MAP_ALLOC
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

#endif
#if 0
	DWORD	dwMaxConection = 20;
	BOOL	b = ::InternetSetOption(NULL, INTERNET_OPTION_MAX_CONNS_PER_SERVER, (LPVOID)&dwMaxConection, sizeof(DWORD));
		b = ::InternetSetOption(NULL, INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER, (LPVOID)&dwMaxConection, sizeof(DWORD));
	b = ::InternetSetOption(NULL, 103, (LPVOID)&dwMaxConection, sizeof(DWORD));
#endif
  #ifdef _DEBUG
	// ATLTRACE�œ��{����g�����߂ɕK�v
	_tsetlocale( LC_ALL, _T("japanese") );
  #endif

	Misc::setHeapAllocLowFlagmentationMode();	//+++

	// �����N���̊m�F
	if (CheckOneInstance(hInstance, lpstrCmdLine)) 
		return 0;

	g_pMainWnd	 = NULL;

	//	HRESULT hRes = ::CoInitialize(NULL);
	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	ATLASSERT( SUCCEEDED(hRes) );
	// If you are running on NT 4.0 or higher you can use the following call instead to
	// make the EXE free threaded. This means that calls come in on a random RPC thread
	//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);

	hRes		 = ::OleInitialize(NULL);
	ATLASSERT( SUCCEEDED(hRes) );

	ATLTRACE(_T("tWinMain\n") _T("CommandLine : %s\n"), lpstrCmdLine);

	/* �R�����R���g���[���������� */
	INITCOMMONCONTROLSEX iccx;
	iccx.dwSize = sizeof (iccx);
	iccx.dwICC	= ICC_COOL_CLASSES | ICC_BAR_CLASSES | ICC_USEREX_CLASSES;
	int 	ret = ::InitCommonControlsEx(&iccx);
	ATLASSERT(ret);

	hRes	= _Module.Init(ObjectMap, hInstance, &LIBID_ATLLib);
	//hRes	= _Module.Init(NULL, hInstance);
	ATLASSERT( SUCCEEDED(hRes) );

	int 	nRet		 = 0;
	bool	bRun		 = true;
	bool	bAutomation  = false;
	bool	bTray		 = false;

	try {	//+++ �O�̂��ߗ�O�`�F�b�N.
		nRet = _PrivateInit();
	} catch (...) {
		ErrorLogPrintf(_T("_PrivateInit�ŃG���[\n"));
		nRet = -1;
	}
	if (nRet <= 0)
		goto END_APP;

	// ActiveX�R���g���[�����z�X�g���邽�߂̏���
	AtlAxWinInit();

	// �R�}���h���C�����͂ɂ���Ă�COM�T�[�o�[�o�^�y�щ������s��
	nRet = RegisterCOMServer(nRet, bRun, bAutomation, bTray);
	if (FAILED(nRet)) {
		ErrorLogPrintf(_T("RegisterCOMServer�ŃG���[\n"));
		nRet = -1;
		goto END_APP;
	}

	CDonutSimpleEventManager::RaiseEvent(EVENT_PROCESS_START);

	if (bRun) {
		_Module.StartMonitor();
		hRes = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE | REGCLS_SUSPENDED);
		ATLASSERT( SUCCEEDED(hRes) );
		hRes = ::CoResumeClassObjects();
		ATLASSERT( SUCCEEDED(hRes) );

		if (bAutomation) {
			CMessageLoop theLoop;
			nRet = theLoop.Run();
		} else {
			//+++ �N�����̊��t�@�C���`�F�b�N. unDonut.ini���Ȃ�
			//	���t�@�C��������ĂȂ�������N���y�[�W��about:warning�ɂ���.
			if (lpstrCmdLine == 0 || lpstrCmdLine[0] == 0) {
				if (HaveEnvFiles() == false)
					lpstrCmdLine = _T("about:warning");
			}

			nRet = Run(lpstrCmdLine, nCmdShow, bTray);
		}
	  #if 1 //+++ WTL�̃��C�����N���[�Y������I�����ɁA�I���R�[�h�Ƃ���1��Ԃ�...
			//+++ OS�ɕԂ��l�Ȃ̂�0�̂ق����悢�͂��ŁA
			//+++ donut�̑��̕����ł�0�ɂ��Ă���悤�Ȃ̂�
			//+++ �������Ȃ��̂ŁA�����I�ɕϊ�.
		if (nRet == 1)
			nRet = 0;
	  #endif

		_Module.RevokeClassObjects();
		::Sleep(_Module.m_dwPause);
	}

	_PrivateTerm();

END_APP:
	_Module.Term();

	::OleUninitialize();
	::CoUninitialize();

	CDonutSimpleEventManager::RaiseEvent(EVENT_PROCESS_END);

 	return nRet;
}



/////////////////////////////////////////////////////////////////////////////


// CChildFrame
void CChildFrame::PreDocumentComplete( /*[in]*/ IDispatch *pDisp, /*[in]*/ VARIANT *URL)
{
	if (!g_pAPI)
		return;

	int nTabIndex = m_MDITab.GetTabIndex(m_hWnd);
	g_pAPI->Fire_DocumentComplete( nTabIndex, pDisp, V_BSTR(URL) );
}




/////////////////////////////////////////////////////////////////////////////
// CAPI
HRESULT STDMETHODCALLTYPE CAPI::Advise(IUnknown *pUnk, DWORD *pdwCookie)
{
	HRESULT hr = CProxyIDonutPEvents<CAPI>::Advise(pUnk, pdwCookie);

	if ( SUCCEEDED(hr) ) {
		g_pAPI = this;
		m_aryCookie.Add(pdwCookie);
	}

	//CString str;
	//str.Format("Advise pUnk=%p cookie=%u",pUnk,*pdwCookie);
	//::MessageBox(NULL,str,_T("check"),MB_OK);
	return hr;
}



HRESULT STDMETHODCALLTYPE CAPI::Unadvise(DWORD dwCookie)
{
	HRESULT hr = CProxyIDonutPEvents<CAPI>::Unadvise(dwCookie);

	if ( SUCCEEDED(hr) )
		g_pAPI = NULL;

	//CString str;
	//str.Format("Unadvise cookie=%u",dwCookie);
	//::MessageBox(NULL,_T("unadvise"),_T("check"),MB_OK);
	return hr;
}




/////////////////////////////////////////////////////////////////////////////
///+++ ���݂̃A�N�e�B�u�łőI�𒆂̃e�L�X�g��Ԃ�.
///+++ �� CSearchBar�����ɗp��. �{���� g_pMainWnd-> �̓����֐����Ăׂ΂������������A
///+++	  include �̈ˑ��֌W���ʓ|�Ȃ̂�...
CString Donut_GetActiveSelectedText()
{
	return g_pMainWnd->GetActiveSelectedText();
}


///+++
CString Donut_GetActiveStatusStr()
{
	return g_pMainWnd->GetActiveChildFrame()->strStatusBar();
}


#if 0
///+++ About�_�C�A���O�p�ɃA�C�R�����[�h (���ƂłȂ�Ƃ�����>����)
HICON Donut_LoadIcon4AboutDialog()
{
	return g_pMainWnd->LoadIcon4AboutDialog();
}
#endif


///+++
void  Donut_ExplorerBar_RefreshFavoriteBar()
{
	CDonutExplorerBar::GetInstance()->RefreshExpBar(0);
}

