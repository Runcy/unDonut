/**
*	@brief	CMainFrame::Impl �̎���
*/

CMainFrame::Impl::Impl() : 
	CDDEMessageHandler<Impl>(_T("Donut")),
	m_ExplorerBar(m_SplitterWindow),
	m_hWndRestoreFocus(NULL),
	m_bFullScreen(false)
{
	GdiplusInit();
}

CMainFrame::Impl::~Impl()
{
	GdiplusTerm();

	g_pMainWnd = nullptr;
}

void	CMainFrame::Impl::RestoreAllTab(LPCTSTR strFilePath, bool bCloseAllTab)
{
	/* default.dfg������΂�������g�� */
	CString	strFile = CStartUpOption::GetDefaultDFGFilePath();
	if (::PathFileExists(strFile)) {
		if ( MtlIsExt( strFile, _T(".dfg") ) ) {
			if ( !(CMainOption::s_dwMainExtendedStyle & MAIN_EX_NOCLOSEDFG) ) {
				//_LoadGroupOption(strFile, true);
			} else {
				//_LoadGroupOption(strFile, false);
			}
			CString strBakFile = Misc::GetFileNameNoExt(strFile) + _T(".bak.dfg");
			::MoveFileEx(strFile, strBakFile, MOVEFILE_REPLACE_EXISTING);
			PostMessage(WM_INITPROCESSFINISHED);
			return;
		}
	}

	int	nActiveIndex = 0;

	CString	TabList;
	if (strFilePath == NULL) {
		TabList = Misc::GetExeDirectory() + _T("TabList.xml");
	} else {
		TabList = strFilePath;
	}

	try {
		using boost::property_tree::wptree;

		std::wifstream	filestream(TabList);
		if (!filestream) {
			PostMessage(WM_INITPROCESSFINISHED);
			return ;
		}
		filestream.imbue(std::locale(std::locale(), new std::codecvt_utf8_utf16<wchar_t>));

		wptree	pt;
		boost::property_tree::read_xml(filestream, pt);

		auto SetTravelLog	= [](wptree& ptLog, vector<std::pair<CString, CString> >& vecTravelLog) {
			for (auto it = ptLog.begin(); it != ptLog.end(); ++it) {
				wptree& item = it->second;
				vecTravelLog.push_back(std::pair<CString, CString>(
					item.get(L"<xmlattr>.title", L"").c_str(), 
					item.get(L"<xmlattr>.url", L"").c_str()));
			}
		};

		wptree&	ptChild = pt.get_child(L"TabList");
		auto it = ptChild.begin();
		nActiveIndex = it->second.get(L"ActiveIndex", 0);
		++it;
		int i = 0;
		for (; it != ptChild.end(); ++it) {
			wptree& ptItem = it->second;
			NewChildFrameData*	pdata = new NewChildFrameData(m_ChildFrameClient);

			pdata->strURL	= ptItem.get(L"<xmlattr>.url", L"").c_str();
			pdata->dwDLCtrl	= ptItem.get<DWORD>(L"<xmlattr>.DLCtrlFlags", CDLControlOption::s_dwDLControlFlags);
			pdata->dwExStyle= ptItem.get<DWORD>(L"<xmlattr>.ExStyle",	CDLControlOption::s_dwExtendedStyleFlags);
			pdata->dwAutoRefresh	= ptItem.get<DWORD>(L"<xmlattr>.AutoRefreshStyle", 0);
			SetTravelLog(ptItem.get_child(L"TravelLog.Back"), pdata->TravelLogBack);
			SetTravelLog(ptItem.get_child(L"TravelLog.Fore"), pdata->TravelLogFore);
			pdata->bActive	= (i == nActiveIndex);

			m_deqNewChildFrameData.push_back(std::unique_ptr<NewChildFrameData>(std::move(pdata)));
			++i;
		}
	} catch (...) {
		MessageBox(_T("RestoreAllTab�ŃG���[���������܂���!"));
		PostMessage(WM_INITPROCESSFINISHED);
		return ;
	}

	CLockRedrawMDIClient	 lock(m_ChildFrameClient);
	CDonutTabBar::CLockRedraw lock2(m_TabBar);

	//if (bCloseAllTab) 
	//	MtlCloseAllMDIChildren(m_ChildFrameClient);

	//for (int i = 0; i < nCount; ++i) {
	//	ChildFrameDataOnClose* pData = vecpSaveData[i].release();
	//	NewChildFrameData*	pThis = vecpNewChildData[i];
	//	pThis->funcCallAfterCreated	= [pData, pThis, this](CChildFrame* pChild) {
	//		pChild->SetTravelLog(pData->TravelLogFore, pData->TravelLogBack);
	//		delete pData;
	//		if (pThis->pNext) {
	//			m_MDITab.SetInsertIndex(m_MDITab.GetItemCount());
	//			CChildFrame::AsyncCreate(*pThis->pNext);	// ����ChildFrame���쐬
	//		} else {
	//			m_MDITab.InsertHere(false);
	//			this->PostMessage(WM_INITPROCESSFINISHED);
	//		}
	//		delete pThis;

	//	};
	//}
	if (m_deqNewChildFrameData.size() > 0) {
		m_TabBar.InsertHere(true);
		m_TabBar.SetInsertIndex(m_TabBar.GetItemCount());
		CChildFrame::AsyncCreate(*m_deqNewChildFrameData[0]);
	} else {
		PostMessage(WM_INITPROCESSFINISHED);
	}
}

void	CMainFrame::Impl::SaveAllTab()
{
	HWND hWndActive = m_ChildFrameClient.GetActiveChildFrameWindow();
	int	nCount = 0;
	int nActiveIndex = -1;
	vector<unique_ptr<ChildFrameDataOnClose> >	vecpSaveData;
	auto CollectChildFrameData = [&](HWND hWnd) {
		::SendMessage(hWnd, WM_GETCHILDFRAMEDATA, nCount, 0);

		CString strSharedMemName;
		strSharedMemName.Format(_T("%s%d"), NOWCHILDFRAMEDATAONCLOSESHAREDMEMNAME, nCount);
		HANDLE hMap = ::OpenFileMapping(FILE_MAP_READ, FALSE, strSharedMemName);
		ATLVERIFY( hMap );
		LPTSTR sharedMemData = (LPTSTR)::MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
		std::wstring serializedData = sharedMemData;
		::UnmapViewOfFile((LPVOID)sharedMemData);
		::CloseHandle(hMap);

		::SendMessage(hWnd, WM_GETCHILDFRAMEDATA, -1, 0);

		auto pChildFrameData = new ChildFrameDataOnClose;
		std::wstringstream ss;
		ss << serializedData;
		boost::archive::text_wiarchive ar(ss);
		ar >> *pChildFrameData;
		
		vecpSaveData.push_back(unique_ptr<ChildFrameDataOnClose>(std::move(pChildFrameData)));
		if (hWnd == hWndActive)
			nActiveIndex = nCount;
		++nCount;
	};
	m_TabBar.ForEachWindow(CollectChildFrameData);

	try {
		using boost::property_tree::wptree;

		auto AddTravelLog = [](wptree& ptLog, const vector<std::pair<CString, CString> >& vecTravelLog) {
			for (auto it = vecTravelLog.cbegin(); it != vecTravelLog.cend(); ++it) {
				wptree& ptItem = ptLog.add(L"item", L"");
				ptItem.put(L"<xmlattr>.title", (LPCTSTR)it->first);
				ptItem.put(L"<xmlattr>.url"	 , (LPCTSTR)it->second);
			}
		};
		wptree	pt;
		wptree&	ptTabList = pt.add(L"TabList", L"");
		ptTabList.add(L"<xmlattr>.ActiveIndex", nActiveIndex);
		for (auto it = vecpSaveData.cbegin(); it != vecpSaveData.cend(); ++it) {
			ChildFrameDataOnClose& data = *(*it);
			wptree& ptItem = ptTabList.add(L"Tab", L"");
			ptItem.put(L"<xmlattr>.title", (LPCTSTR)data.strTitle);
			ptItem.put(L"<xmlattr>.url"	 , (LPCTSTR)data.strURL);
			ptItem.put(L"<xmlattr>.DLCtrlFlags", data.dwDLCtrl);
			ptItem.put(L"<xmlattr>.ExStyle",	data.dwExStyle);
			ptItem.put(L"<xmlattr>.AutoRefreshStyle", data.dwAutoRefreshStyle);
			AddTravelLog(ptItem.add(L"TravelLog.Back", L""), data.TravelLogBack);
			AddTravelLog(ptItem.add(L"TravelLog.Fore", L""), data.TravelLogFore);
		}
		using namespace boost::property_tree::xml_parser;

		CString strTempTabList = Misc::GetExeDirectory() + _T("TabList.temp.xml");
		std::wofstream filestream(strTempTabList);
		if (!filestream)
			throw "error";
		filestream.imbue(std::locale(std::locale(), new std::codecvt_utf8_utf16<wchar_t>));
		write_xml(filestream, pt, xml_writer_make_settings(L' ', 2, widen<wchar_t>("UTF-8")));	
		CString	TabList = Misc::GetExeDirectory() + _T("TabList.xml");
		if (::PathFileExists(TabList)) {
			CString strBakFile = Misc::GetFileNameNoExt(TabList) + _T(".bak.xml");
			::MoveFileEx(TabList, strBakFile, MOVEFILE_REPLACE_EXISTING);
		}
		filestream.close();
		::MoveFileEx(strTempTabList, TabList, MOVEFILE_REPLACE_EXISTING);

	} catch (...) {
		MessageBox(_T("SaveAllTab�ŃG���[����!"));
	}
}

void	CMainFrame::Impl::UserOpenFile(LPCTSTR url, DWORD openFlags /*= DonutGetStdOpenFlag()*/, 
									   DWORD DLCtrl /*= -1*/, 
									   DWORD ExStyle /*= -1*/, 
									   DWORD AutoRefresh /*= 0*/)
{
	HWND	hWndActive = m_ChildFrameClient.GetActiveChildFrameWindow();

	CString strFileOrURL = url;
	/* openFlag �� D_OPENFILE_NOCREATE �Ȃ�A�N�e�B�u�ȃy�[�W���ړ����� */
	if ( hWndActive != NULL && _check_flag(D_OPENFILE_NOCREATE, openFlags) ) {
		/*CChildFrame* pChild  = GetActiveChildFrame();
		if (strFileOrURL.Left(11).CompareNoCase(_T("javascript:")) == 0) {
			::PostMessage(pChild->GetHwnd(), WM_EXECUTEUSERJAVASCRIPT, (WPARAM)(LPCTSTR)new CString(strFileOrURL), 0);
		} else*/ {
			//if (DLCtrl)
			//	pChild->SetDLCtrl(DLCtrl.get());
			//if (ExStyle)
			//	pChild->SetExStyle(ExStyle.get());
			//if (AutoRefresh)
			//	pChild->SetAutoRefreshStyle(AutoRefresh.get());
			//pChild->Navigate2(strFileOrURL);
		}

		if ( !_check_flag(D_OPENFILE_NOSETFOCUS, openFlags) ) {
			// reset focus
			::SetFocus(NULL);
			MtlSendCommand(hWndActive, ID_VIEW_SETFOCUS);
		}
	} else {

		/* �V�KChildFrame�쐬 */
		NewChildFrameData	data(m_ChildFrameClient);
		data.strURL		= strFileOrURL;
		data.dwDLCtrl	= DLCtrl;
		data.dwExStyle	= ExStyle;
		data.dwAutoRefresh	= AutoRefresh;
		data.bActive	= _check_flag(openFlags, D_OPENFILE_ACTIVATE) 
			|| m_ChildFrameClient.GetActiveChildFrameWindow() == NULL;
		CChildFrame::AsyncCreate(data);
	}
}

// Overrides

BOOL CMainFrame::Impl::AddSimpleReBarBandCtrl(HWND hWndReBar, HWND hWndBand, int nID, LPTSTR lpstrTitle, UINT fStyle, int cxWidth)
{
	ATLASSERT( ::IsWindow(hWndReBar) ); 	// must be already created
	ATLASSERT( ::IsWindow(hWndBand) );		// must be already created
	MTLASSERT_KINDOF(REBARCLASSNAME, hWndReBar);

	// Set band info structure
	REBARBANDINFO rbBand = { sizeof (REBARBANDINFO) };
	rbBand.fMask	 = RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_STYLE | RBBIM_ID | RBBIM_SIZE | RBBIM_IDEALSIZE;

	rbBand.fMask	|= RBBIM_BACKGROUND | RBBIM_TEXT;
	rbBand.fStyle	 = fStyle;
	rbBand.lpText	 = lpstrTitle;
	rbBand.hwndChild = hWndBand;
	rbBand.wID		 = nID;

	// Calculate the size of the band
	BOOL	bRet;
	RECT	rcTmp;
	int 	nBtnCount = (int) ::SendMessage(hWndBand, TB_BUTTONCOUNT, 0, 0L);
	if (nBtnCount > 0) {
		// �c�[���o�[�̏ꍇ
		rbBand.fStyle	|= RBBS_USECHEVRON;
		bRet				= ::SendMessage(hWndBand, TB_GETITEMRECT, nBtnCount - 1, (LPARAM) &rcTmp) != 0;
		ATLASSERT(bRet);
		rbBand.cx			= (cxWidth != 0) ? cxWidth : rcTmp.right;
		rbBand.cyMinChild	= rcTmp.bottom - rcTmp.top;

		if (lpstrTitle == 0) {
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
		rbBand.cxMinChild	= 0;
		rbBand.cyMinChild	= rcTmp.bottom - rcTmp.top;
	}

	// NOTE: cxIdeal used for CHEVRON, if MDI cxIdeal changed dynamically.
	rbBand.cxIdeal = rcTmp.right;			//rbBand.cx is not good.

	// Add the band
	LRESULT 	  lRes = ::SendMessage(hWndReBar, RB_INSERTBAND, (WPARAM) -1, (LPARAM) &rbBand);

	if (lRes == 0) {
		ATLTRACE2( atlTraceUI, 0, _T("Failed to add a band to the rebar.\n") );
		return FALSE;
	}

	//if (nID == IDC_LINKBAR)
	//	::SendMessage(hWndBand, TB_SETEXTENDEDSTYLE, 0, TBSTYLE_EX_HIDECLIPPEDBUTTONS);

	return TRUE;
}

void CMainFrame::Impl::UpdateLayout(BOOL bResizeBars /*= TRUE*/)
{
	CRect	  rc;
	GetClientRect(&rc);

	if (bResizeBars) {
		//CReBarCtrl rebar(m_hWndToolBar);
		//if (rebar.m_hWnd == NULL) 
		//	goto END;

		CRect	rcSrc;
		m_ReBar.GetClientRect(&rcSrc);

		CRect	rcNew(0, 0, rc.right, rcSrc.Height());
		m_ReBar.MoveWindow(rcNew);
		m_ReBar.RedrawWindow();
	}
//END:
	UpdateBarsPosition(rc, bResizeBars);
	//if (m_bFullScreen == false)
	//	rc.top++;

	// �y�[�W�������o�[
	//HWND hWndFind = m_FindBar.GetHWND();
	//if (::IsWindowVisible(hWndFind)) {
	//	CRect rcFind;
	//	::GetClientRect(hWndFind, &rcFind);
	//	::SetWindowPos( hWndFind, NULL, rc.left, rc.top, rc.right, rcFind.bottom, SWP_NOZORDER | SWP_NOACTIVATE );
	//	rc.top += rcFind.bottom;
	//}

	if (m_hWndClient)
		::SetWindowPos( m_hWndClient, NULL, rc.left, rc.top, rc.Width(), rc.Height(), SWP_NOZORDER | SWP_NOACTIVATE );
}

BOOL CMainFrame::Impl::PreTranslateMessage(MSG* pMsg)
{
	return FALSE;
}

BOOL CMainFrame::Impl::OnIdle()
{
	CmdUIUpdateToolBars();
	CmdUIUpdateStatusBar	(m_hWndStatusBar, ID_DEFAULT_PANE	);
	CmdUIUpdateStatusBar	(m_hWndStatusBar, ID_SECURE_PANE	);
	CmdUIUpdateStatusBar	(m_hWndStatusBar, ID_PRIVACY_PANE	);
	CmdUIUpdateChildWindow	(m_hWndStatusBar, IDC_PROGRESS		);

	return FALSE;
}

bool CMainFrame::Impl::OnDDEOpenFile(const CString& strFileName)
{
	DWORD dwOpen = 0;
	if (CMainOption::s_bExternalNewTab) {
		dwOpen |= D_OPENFILE_CREATETAB;
		if (CMainOption::s_bExternalNewTabActive)
			dwOpen |= D_OPENFILE_ACTIVATE;
	} else {	// �����̃^�u���i�r�Q�[�g����
		dwOpen |= D_OPENFILE_NOCREATE;
		if (!CStartUpOption::s_dwActivate)
			dwOpen |= D_OPENFILE_NOSETFOCUS;
	}

	UserOpenFile( strFileName, dwOpen );
	if (CStartUpOption::s_dwActivate) {
		//IfTrayRestoreWindow();							//+++ �g���C��Ԃ������畜��.
		if (IsZoomed() == FALSE)
			ShowWindow(SW_RESTORE);
			//ShowWindow_Restore(true);
		MtlSetForegroundWindow(m_hWnd);
	}
	return true;
}

int		CMainFrame::Impl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	SetMsgHandled(FALSE);

	// message loop
	CMessageLoop *pLoop = _Module.GetMessageLoop();
	pLoop->AddMessageFilter(this);
	pLoop->AddIdleHandler(this);

	_initRebar();
	HWND hWndCmdBar		= _initCommandBar();
	HWND hWndToolBar	= _initToolBar();
	HWND hWndAddressBar = _initAddressBar();
	HWND hWndSearchBar	= m_SearchBar.Create(m_ReBar);
	HWND hWndTabBar		= _initTabBar();
	HWND hWndLinkBar	= _initLinkBar();
	_initBandPosition(hWndCmdBar, hWndToolBar, hWndAddressBar, hWndSearchBar, hWndLinkBar, hWndTabBar);

	_initStatusBar();
	_initSplitterWindow();
	_initChildFrameClient();
	_initExplorerBar();

	_initSkin();

	_initSysMenu();

	m_ChildFrameUIState.SetMainFrameHWND(m_hWnd);
	CmdUIAddToolBar(hWndToolBar);					// set up UI
	CmdUIAddToolBar(m_SearchBar.GetHWndToolBar());	// set up UI

	CreateGlobalConfig(&m_GlobalConfigManageData);
	SetGlobalConfig(m_GlobalConfigManageData.pGlobalConfig);

	CFaviconManager::Init(hWndTabBar);

	m_DownloadManager.SetParent(m_hWnd);

	// �ŋߕ����^�u�̃��j���[�̐ݒ��I�v�V�����̐ݒ���s��
	m_RecentClosedTabList.SetMenuHandle(CMenuHandle(m_CmdBar.m_hMenu).GetSubMenu(0).GetSubMenu(kPosMRU));
	m_RecentClosedTabList.SetMaxEntries(CMainOption::s_nMaxRecentClosedTabCount);
	m_RecentClosedTabList.SetMenuType(CMainOption::s_RecentClosedTabMenuType);
	m_RecentClosedTabList.ReadFromXmlFile();
	m_RecentClosedTabList.UpdateMenu();

	RegisterDragDrop();

	return 0;
}

void	CMainFrame::Impl::OnDestroy()
{
	SetMsgHandled(FALSE);

	_PrivateTerm();		// �ݒ�̕ۑ�

	RevokeDragDrop();

	_SaveBandPosition();

	CIniFileO	pr(g_szIniFileName, _T("Main"));
	MtlWriteProfileMainFrameState(pr, m_hWnd);

	m_ReBar.UnsubclassWindow();

	DestroyGlobalConfig(&m_GlobalConfigManageData);

	if (CStartUpOption::s_dwFlags == CStartUpOption::STARTUP_LATEST) {
		SaveAllTab();	// ���ݕ\�����̃^�u��ۑ�����
	} else {
		vector<HWND> vechWnd;
		m_TabBar.ForEachWindow([&](HWND hWndChildFrame) {	// �ŋߕ����^�u�ɒǉ�
			vechWnd.push_back(hWndChildFrame);			
		});
		for (auto it = vechWnd.cbegin(); it != vechWnd.cend(); ++it)
			::SendMessage(*it, WM_CLOSE, 0, 0);
	}

	if (CMainOption::s_dwMainExtendedStyle2 & MAIN_EX2_DEL_RECENTCLOSE) {
		m_RecentClosedTabList.DeleteRecentClosedTabFile();
	} else {
		m_RecentClosedTabList.WriteToXmlFile();
	}

	// ���b�Z�[�W���[�v���烁�b�Z�[�W�t�B���^�ƃA�C�h���n���h�����폜
    CMessageLoop* pLoop = _Module.GetMessageLoop();
    pLoop->RemoveMessageFilter(this);
    pLoop->RemoveIdleHandler(this);

}



void	CMainFrame::Impl::_initRebar()
{
	DWORD	dwRebarStyle = ATL_SIMPLE_REBAR_STYLE | CCS_NOPARENTALIGN | CCS_NODIVIDER | RBS_DBLCLKTOGGLE;
	{	// TODO: ��Őݒ�N���X�Ɉڂ�
		CIniFileI	pr( g_szIniFileName, _T("ReBar") );
		DWORD		dwNoBoader 		= pr.GetValue( _T("NoBoader") );
		if (dwNoBoader)
			dwRebarStyle &= ~RBS_BANDBORDERS;	// �{�[�_�[������
	}

	CreateSimpleReBar(dwRebarStyle);
	m_ReBar.SubclassWindow(m_hWndToolBar);
}

HWND	CMainFrame::Impl::_initCommandBar()
{
	SetMenu(NULL);		// remove menu
	HWND	hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, MTL_SIMPLE_CMDBAR2_PANE_STYLE, 0, ATL_IDW_COMMAND_BAR);
	ATLASSERT( ::IsWindow(hWndCmdBar) );

	{
		CIniFileI	prFont( g_szIniFileName, _T("Main") );
		MTL::CLogFont	lf;
		lf.InitDefault();
		if ( lf.GetProfile(prFont) ) {
			m_CmdBar.SetMenuLogFont(lf);

			CFontHandle 	font;	//\\ SetFont�������ł���悤��
			MTLVERIFY( font.CreateFontIndirect(&lf) );
			if (font.m_hFont) 
				SetFont(font);
		}
	}
	CMenuHandle menu;
	menu.LoadMenu(IDR_MAINFRAME);
	m_CmdBar.AttachMenu(menu);

	return hWndCmdBar;
}

HWND	CMainFrame::Impl::_initToolBar()
{
	HWND hWndToolBar = m_ToolBar.Create(m_hWnd);
	ATLASSERT( ::IsWindow(hWndToolBar) );

	if (m_CmdBar.m_fontMenu.m_hFont) {	// �R�}���h�o�[�̃t�H���g�ݒ�Ɠ�����
		LOGFONT lf;
		m_CmdBar.m_fontMenu.GetLogFont(&lf);
		CFontHandle font;
		m_ToolBar.SetFont(font.CreateFontIndirect(&lf));
	}

	return hWndToolBar;
}

HWND	CMainFrame::Impl::_initAddressBar()
{
	HWND hWndAddressBar = m_AddressBar.Create(m_ReBar, IDC_ADDRESSBAR, ID_VIEW_GO,
												 16, 16, RGB(255, 0, 255) );
	ATLASSERT( ::IsWindow(hWndAddressBar) );

	return hWndAddressBar;
}

HWND	CMainFrame::Impl::_initTabBar()
{
	HWND	hWndMDITab = m_TabBar.Create(m_ReBar);
	ATLASSERT( ::IsWindow(hWndMDITab) );

	return hWndMDITab;
}

HWND	CMainFrame::Impl::_initLinkBar()
{
	HWND	hWndLinkBar = m_LinkBar.Create(m_ReBar);
	ATLASSERT( ::IsWindow(hWndLinkBar) );

	return 	hWndLinkBar;
}

void	CMainFrame::Impl::_initBandPosition(HWND hWndCmdBar, 
											HWND hWndToolBar, 
											HWND hWndAddressBar, 
											HWND hWndSearchBar, 
											HWND hWndLinkBar, 
											HWND hWndTabBar	)
{
	std::vector<HWND>	vecBand;
	vecBand.push_back(hWndCmdBar);
	vecBand.push_back(hWndToolBar);
	vecBand.push_back(hWndAddressBar);
	vecBand.push_back(hWndSearchBar);
	vecBand.push_back(hWndLinkBar);
	vecBand.push_back(hWndTabBar);

	struct ReBarBandInfo {
		UINT		nIndex; 	// must be filled, cause stable_sort requires CRT
		HWND		hWnd;
		UINT		nID;
		UINT		fStyle;
		LPTSTR		lpText;
		UINT		cx; 		// can be 0
	};
	std::vector<ReBarBandInfo>	vecReBarBandInfo;
	CIniFileI	pr(g_szIniFileName, _T("ReBar"));
	for (auto it = vecBand.cbegin(); it != vecBand.cend(); ++it) {
		ReBarBandInfo	rbi = { 0 };
		rbi.hWnd	= *it;
		rbi.nID		= ::GetDlgCtrlID((*it));
		CString prefix;
		prefix.Format(_T("band#%d"), rbi.nID);
		rbi.nIndex	= pr.GetValuei(prefix + _T(".index"));
		rbi.cx		= pr.GetValuei(prefix + _T(".cx"));
		rbi.fStyle	= pr.GetValuei(prefix + _T(".fStyle"));
		vecReBarBandInfo.push_back(rbi);
	}
	boost::sort(vecReBarBandInfo, [](const ReBarBandInfo& rbi1, const ReBarBandInfo& rbi2) {
		return rbi1.nIndex < rbi2.nIndex;
	});
	boost::for_each(vecReBarBandInfo, [this](const ReBarBandInfo& rbi) {
		AddSimpleReBarBandCtrl(m_ReBar, rbi.hWnd, rbi.nID, nullptr, (rbi.fStyle & RBBS_BREAK) != 0, rbi.cx);
	});
	m_ReBar.LockBands( (vecReBarBandInfo.front().fStyle & RBBS_NOGRIPPER) != 0 );
	m_CmdBar.RefreshBandIdealSize(m_ReBar);
	m_AddressBar.InitReBarBandInfo(m_ReBar);
}

void	CMainFrame::Impl::_initStatusBar()
{
	m_hWndStatusBar	= m_StatusBar.Create(m_hWnd, ATL_IDS_IDLEMESSAGE,
							 WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | SBARS_SIZEGRIP | SBT_TOOLTIPS);
	ATLASSERT( ::IsWindow(m_hWndStatusBar) );

	//		int nPanes[] = { ID_DEFAULT_PANE, ID_PROGRESS_PANE, ID_COMBOBOX_PANE};
	static int   nPanes[] = { ID_DEFAULT_PANE, ID_PROGRESS_PANE, ID_PRIVACY_PANE, ID_SECURE_PANE, ID_COMBOBOX_PANE };
	m_StatusBar.SetPanes( nPanes, _countof(nPanes), false );
	m_StatusBar.SetCommand( ID_PRIVACY_PANE, ID_PRIVACYREPORT );
	m_StatusBar.SetCommand( ID_SECURE_PANE, ID_SECURITYREPORT);
	m_StatusBar.SetIcon( ID_PRIVACY_PANE, 1 );				//minit
	m_StatusBar.SetIcon( ID_SECURE_PANE , 0 );				//minit
	m_StatusBar.SetOwnerDraw( m_StatusBar.IsValidBmp() );

	// TODO: ������ւ���Ȃ�Ƃ�����
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
		m_StatusBar.SetPaneWidth(ID_PAIN_1, 0);			//dwSzPain1); //�N�����̓y�C���T�C�Y��0
		if (m_StatusBar.GetProxyComboBox().IsUseIE())
			dwSzPain2 = 0;									// IE��Proxy���g���ꍇ��Proxy�y�C���T�C�Y��0��
		m_StatusBar.SetPaneWidth(ID_PAIN_2, dwSzPain2);
	} else {	// �������Ă���Ƃ�.
		g_dwProgressPainWidth = dwSzPain2;					//+++ �蔲���Ńv���O���X�y�C���̉������O���[�o���ϐ��ɍT����.
		m_StatusBar.SetPaneWidth(ID_PAIN_2, dwSzPain2);	//+++ �������Ă�Ƃ��́A�ŏ�����T�C�Y�m��.
		if (m_StatusBar.GetProxyComboBox().IsUseIE())
			dwSzPain1 = 0;									// IE��Proxy���g���ꍇ��Proxy�y�C���T�C�Y��0��
		m_StatusBar.SetPaneWidth(ID_PAIN_1, dwSzPain1);
	}

	m_StatusBar.SetPaneWidth(ID_SECURE_PANE	, 25);
	m_StatusBar.SetPaneWidth(ID_PRIVACY_PANE , 25);
	
	// init_loadStatusBarState
	CIniFileI pr( g_szIniFileName, _T("Main") );
	BOOL	bStatusBarVisible = TRUE;
	MtlGetProfileStatusBarState(pr, m_hWndStatusBar, bStatusBarVisible);
}

void	CMainFrame::Impl::_initSplitterWindow()
{
	m_hWndClient = m_SplitterWindow.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
	ATLASSERT( ::IsWindow(m_hWndClient) );
	m_SplitterWindow.SetSplitterExtendedStyle(SPLIT_GRADIENTBAR/*0*/);
}

void	CMainFrame::Impl::_initChildFrameClient()
{
	m_ChildFrameClient.Create(m_hWndClient);
	ATLASSERT( m_ChildFrameClient.IsWindow() );

	m_TabBar.SetChildFrameClient(&m_ChildFrameClient);
}

void	CMainFrame::Impl::_initExplorerBar()
{
	m_ExplorerBar.Create(m_hWndClient);
	ATLASSERT( m_ExplorerBar.IsWindow() );
	m_ExplorerBar.Init(m_ChildFrameClient);
}

void	CMainFrame::Impl::_initSkin()
{
	/* �t�H���g */
	m_TabBar.SetFont		(CSkinOption::s_lfTabBar.CreateFontIndirect());
	m_AddressBar.SetFont	(CSkinOption::s_lfAddressBar.CreateFontIndirect());
	m_SearchBar.SetFont		(CSkinOption::s_lfSearchBar.CreateFontIndirect());
	m_LinkBar.SetFont		(CSkinOption::s_lfLinkBar.CreateFontIndirect());
	m_StatusBar.SetProxyComboBoxFont(CSkinOption::s_lfProxyComboBox.CreateFontIndirect());

	/* �X�L�� */
	//initCurrentIcon();											//+++ �A�C�R��
	m_CmdBar.setMenuBarStyle(m_hWndToolBar, false); 			//+++ ���j���[ (FEVATWH�̒Z���ɂ��邩�ۂ�)
	m_CmdBar.InvalidateRect(NULL, TRUE);						//���j���[�o�[
	m_ReBar.RefreshSkinState(); 								//ReBar
	m_TabBar.ReloadSkin();										//�^�u
	CToolBarOption::GetProfile();
	m_ToolBar.ReloadSkin(); 									//�c�[���o�[
	m_AddressBar.ReloadSkin(CSkinOption::s_nComboStyle);		//�A�h���X�o�[
	m_SearchBar.ReloadSkin(CSkinOption::s_nComboStyle); 		//�����o�[
	//m_LinkBar.InvalidateRect(NULL, TRUE);						//�����N�o�[
	m_ExplorerBar.ReloadSkin(); 								//�G�N�X�v���[���o�[
	m_ExplorerBar.m_PanelBar.ReloadSkin();						//�p�l���o�[
	m_ExplorerBar.m_PluginBar.ReloadSkin(); 					//�v���O�C���o�[
	
	m_StatusBar.ReloadSkin( CSkinOption::s_nStatusStyle		//�X�e�[�^�X�o�[
							 , CSkinOption::s_nStatusTextColor
							 , CSkinOption::s_nStatusBackColor);
}

void	CMainFrame::Impl::_initSysMenu()
{
	CMenuHandle SysMenu = GetSystemMenu(FALSE);
	//		::AppendMenu(hSysMenu, MF_ENABLED|MF_STRING, ID_VIEW_COMMANDBAR, _T("���j���[��\��"));

	TCHAR		cText[]	 = _T("���j���[��\��");
	MENUITEMINFO  menuInfo = { sizeof(MENUITEMINFO) };
	menuInfo.fMask		= MIIM_ID | MIIM_TYPE;
	menuInfo.fType		= MFT_STRING;
	menuInfo.wID		= ID_VIEW_COMMANDBAR;
	menuInfo.dwTypeData = cText;
	menuInfo.cch		= sizeof (cText);
	SysMenu.InsertMenuItem(0, MF_BYPOSITION, &menuInfo);
}


void	CMainFrame::Impl::_SaveBandPosition()
{
	CIniFileO pr(g_szIniFileName, _T("ReBar"));
	UINT nCount = m_ReBar.GetBandCount();
	for (UINT i = 0; i < nCount; ++i) {
		REBARBANDINFO	rbi = { sizeof(REBARBANDINFO) };
		rbi.fMask	= RBBIM_SIZE | RBBIM_STYLE | RBBIM_CHILD;
		m_ReBar.GetBandInfo(i, &rbi);
		int nCtrlID = ::GetDlgCtrlID(rbi.hwndChild);
		CString prefix;
		prefix.Format(_T("band#%d"), nCtrlID);
		pr.SetValue(i			, prefix + _T(".index"));
		pr.SetValue(rbi.cx		, prefix + _T(".cx"));
		pr.SetValue(rbi.fStyle	, prefix + _T(".fStyle"));
	}
}


// Message handler

/// �t�H�[�J�X�𕜌�����
void	CMainFrame::Impl::OnActivate(UINT nState, BOOL bMinimized, CWindow wndOther)
{
	if (nState == WA_INACTIVE) {
		m_hWndRestoreFocus = ::GetFocus();
	} else if (m_hWndRestoreFocus) {
		::SetFocus(m_hWndRestoreFocus);
	}
}

BOOL	CMainFrame::Impl::OnCopyData(CWindow wnd, PCOPYDATASTRUCT pCopyDataStruct)
{
	switch (pCopyDataStruct->dwData) {
	case kNewDonutInstance:
		{
			CString strURL = static_cast<LPCTSTR>(pCopyDataStruct->lpData);
			UserOpenFile(strURL);
		}
		break;
	case kSearchTextWithEngine:
		{
			CString str = (LPCWSTR)pCopyDataStruct->lpData;
			int nIndex = str.Find(_T('\n'));
			CString strEngine = str.Left(nIndex);
			CString strText	= str.Mid(nIndex + 1);
			m_SearchBar.SearchWebWithEngine(strText, strEngine);
		}
		break;

	case kSetSearchText:
		{
			CString str = (LPCWSTR)pCopyDataStruct->lpData;
			bool bHilight = false;
			if (str.Left(1) == _T("1"))
				bHilight = true;
			m_SearchBar.SetSearchStr(str.Mid(1));
			m_SearchBar.ForceSetHilightBtnOn(bHilight);
		}
		break;

	default:
		SetMsgHandled(FALSE);
		return 0;
	}
	return 0;
}

void	CMainFrame::Impl::OnBrowserTitleChange(HWND hWndChildFrame, LPCTSTR strTitle)
{
	m_TabBar.SetTitle(hWndChildFrame, strTitle);

	// �L���v�V������ύX
	if (m_ChildFrameClient.GetActiveChildFrameWindow() == hWndChildFrame) {
		CString strapp;
		strapp.LoadString(IDR_MAINFRAME);
		CString strMainTitle;
		strMainTitle.Format(_T("%s - %s"), strTitle, strapp);
		SetWindowText(strMainTitle);
	}
}

void	CMainFrame::Impl::OnBrowserLocationChange(LPCTSTR strURL, HICON hFavicon)
{
	m_AddressBar.SetWindowText(strURL);	
	m_AddressBar.ReplaceIcon(hFavicon);
}

/// �����o�[����Ă΂�� ���������Ƃ��ɔ��ł���֐�
LRESULT CMainFrame::Impl::OnOpenWithExProp(_EXPROP_ARGS *pArgs)
{
	ATLASSERT(pArgs);

	//CChildFrame *pActiveChild = GetActiveChildFrame();
	//if( pActiveChild)
	//	pActiveChild->SaveSearchWordflg(false); //\\ �����o�[�Ō��������Ƃ��A�N�e�B�u�ȃ^�u�̌����������ۑ����Ȃ��悤�ɂ���
	
	bool bOldSaveFlag = CSearchBarOption::s_bSaveSearchWord;	// �����o�[�̕����񂪏����錏�Ɉꉞ�̑Ώ�
	CSearchBarOption::s_bSaveSearchWord = false;				// �{���͂���Ȃ��Ƃ�����_��

	NewChildFrameData	data(m_ChildFrameClient);
	data.strURL		= pArgs->strUrl;
	DWORD dwExProp = 0xAAAAAA;		//+++ �����l�ύX
	DWORD dwExProp2= 0x8;			//+++ �g���v���p�e�B�𑝐�.
	if (CExProperty::CheckExPropertyFlag(dwExProp, dwExProp2, pArgs->strIniFile, pArgs->strSection)) {
		CExProperty  ExProp(CDLControlOption::s_dwDLControlFlags, CDLControlOption::s_dwExtendedStyleFlags, 0, dwExProp, dwExProp2);
		data.dwDLCtrl	= ExProp.GetDLControlFlags();
		data.dwExStyle	= ExProp.GetExtendedStyleFlags();
		data.dwAutoRefresh = ExProp.GetAutoRefreshFlag();
	}
	if (CSearchBarOption::s_bAutoHilight) {
		data.bAutoHilight = true;
		data.searchWord	= pArgs->strSearchWord;
	}

	//if (pActiveChild && _check_flag(D_OPENFILE_NOCREATE, pArgs->dwOpenFlag)) {
	//	// �����̃^�u���i�r�Q�[�g
	//	if (data.dwDLCtrl != -1)
	//		pActiveChild->SetMarshalDLCtrl(data.dwDLCtrl);
	//	if (data.dwExStyle != -1)
	//		pActiveChild->SetExStyle(data.dwExStyle);
	//	pActiveChild->Navigate2(data.strURL);
	//	if (data.dwAutoRefresh)
	//		pActiveChild->SetAutoRefreshStyle(data.dwAutoRefresh);
	//} else 
	{
		// �V�K�^�u���쐬����
		CString str = pArgs->strSearchWord;
		//data.funcCallAfterCreated = [str, bOldSaveFlag](CChildFrame* pChild) {
		//	CSearchBarOption::s_bSaveSearchWord = bOldSaveFlag;
	 // 		//+++ �q���Ɍ����ݒ�𔽉f (�֐���)
		//	if (CSearchBarOption::s_bAutoHilight)
		//		pChild->SetSearchWordAutoHilight(str, true);
		//};
		data.bActive	= !_check_flag(CMainOption::s_dwMainExtendedStyle, MAIN_EX_NOACTIVATE);
		CChildFrame::AsyncCreate(data);
	}

	return 0;
}


void	CMainFrame::Impl::OnFileOpen(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (nID == ID_VIEW_HOME) {
		if (m_ChildFrameUIState.GetActiveChildFrameWindowHandle() == NULL
			|| ::GetKeyState(VK_CONTROL) < 0) {
			nID = ID_FILE_NEW_HOME;
		} else {
			SetMsgHandled(FALSE);
			return ;
		}
	}

	if (nID == ID_FILE_NEW) {
		switch (CFileNewOption::s_dwFlags) {
		case FILENEW_BLANK:	nID = ID_FILE_NEW_BLANK;	break;
		case FILENEW_COPY:	nID = ID_FILE_NEW_COPY;		break;
		case FILENEW_HOME:	nID = ID_FILE_NEW_HOME;		break;
		case FILENEW_USER:
			{
				UserOpenFile(CFileNewOption::s_strUsr, DonutGetStdOpenActivateFlag());
				return ;
			}
			break;

		default:	nID = ID_FILE_NEW_BLANK;	break;
		}
	}

	switch (nID) {
	case ID_FILE_NEW_BLANK:
		UserOpenFile(_T("about:blank"), DonutGetStdOpenActivateFlag());
		PostMessage(WM_COMMAND, ID_SETFOCUS_ADDRESSBAR, 0);
		break;

	case ID_FILE_NEW_HOME:	// TODO: ��ŕς���
		UserOpenFile(_T("about:home"), DonutGetStdOpenActivateFlag());
		break;

	case ID_FILE_NEW_COPY:
		break;

	case ID_FILE_NEW_CLIPBOARD:
		{
			CString strText = MtlGetClipboardText();
			if ( strText.IsEmpty() )
				return;
			UserOpenFile( strText, DonutGetStdOpenActivateFlag() );
		}
		break;

	case ID_FILE_OPEN_TABLIST:
		{
			CString strPath = Misc::GetExeDirectory() + _T("TabList.xml");
			UserOpenFile( strPath, DonutGetStdOpenCreateFlag() );
		}
		break;

	case ID_FILE_OPEN:
		{
			COpenURLDlg dlg;
			if ( dlg.DoModal() == IDOK && !dlg.m_strEdit.IsEmpty() ) {
				UserOpenFile( dlg.m_strEdit, DonutGetStdOpenFlag() );
			}
		}
		break;
	}
}

void	CMainFrame::Impl::OnFileRecent(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	// get file name from the MRU list
	if (ID_FILE_MRU_FIRST <= nID && nID <= ID_FILE_MRU_LAST)			// ���͈�ID����V�͈�ID�֕ϊ�
		nID = ID_RECENTDOCUMENT_FIRST + (nID - ID_FILE_MRU_FIRST);

	ChildFrameDataOnClose*	pdata = nullptr;
	ATLVERIFY(m_RecentClosedTabList.GetFromList(nID, &pdata));
	NewChildFrameData	data(m_ChildFrameClient);
	data.strURL		= pdata->strURL;
	data.dwDLCtrl	= pdata->dwDLCtrl;
	data.bActive	= _check_flag(DonutGetStdOpenCreateFlag(), D_OPENFILE_ACTIVATE);	// Force New Window
	data.TravelLogBack = pdata->TravelLogBack;
	data.TravelLogFore = pdata->TravelLogFore;
	m_RecentClosedTabList.RemoveFromList(nID);
	CChildFrame::AsyncCreate(data);
}


void	CMainFrame::Impl::OnEditOperation(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	CEdit editAddress = m_AddressBar.GetEditCtrl();
	CEdit editSearch  = m_SearchBar.GetEditCtrl();

	if (::GetFocus() != editAddress && ::GetFocus() != editSearch) {
		SetMsgHandled(FALSE);
		return ;
	}
	bool bFocusAddressBar = (::GetFocus() == editAddress);
	switch (nID) {
	case ID_EDIT_CUT:	bFocusAddressBar ? editAddress.Cut() : editSearch.Cut();	break;
	case ID_EDIT_COPY:	bFocusAddressBar ? editAddress.Copy() : editSearch.Copy();	break;
	case ID_EDIT_PASTE:	bFocusAddressBar ? editAddress.Paste() : editSearch.Paste();	break;
	case ID_EDIT_SELECT_ALL:	bFocusAddressBar ? editAddress.SetSelAll() : editSearch.SetSelAll();	break;
	}
}

void	CMainFrame::Impl::OnSearchBarCommand(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	HWND hWndActive = m_ChildFrameClient.GetActiveChildFrameWindow();
	if (hWndActive == NULL)
		return ;

	CString str = _GetSelectText(m_SearchBar.GetEditCtrl());
	if ( str.IsEmpty() )
		return ;

	switch (nID) {
	case ID_SEARCHBAR_SEL_UP:	m_SearchBar.SearchPage(false);	break;
	case ID_SEARCHBAR_SEL_DOWN:	m_SearchBar.SearchPage(true);	break;
	case ID_SEARCHBAR_HILIGHT:	m_SearchBar.SearchHilight();	break;
	}
}

/// �����o�[����̃y�[�W������
LRESULT CMainFrame::Impl::OnFindKeyWord(LPCTSTR lpszKeyWord, BOOL bBack, long Flags /*= 0*/)
{
	HWND hWndActive = m_ChildFrameClient.GetActiveChildFrameWindow();
	if (hWndActive == NULL)
		return 0;

	FindKeywordData	findKeywordData = { lpszKeyWord, bBack != 0, Flags };
	std::wstringstream	ss;
	boost::archive::text_woarchive	ar(ss);
	ar << findKeywordData;
	std::wstring strSerializedData = ss.str();

	CString strSharedMemName;
	strSharedMemName.Format(_T("%s0x%x"), FINDKEYWORDATASHAREDMEMNAME, lpszKeyWord);
	HANDLE hMap = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, (static_cast<int>(strSerializedData.size()) + 1) * sizeof(WCHAR), strSharedMemName);
	ATLASSERT( hMap );
	LPTSTR strSharedData = (LPTSTR)::MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	::wcscpy_s(strSharedData, strSerializedData.size() + 1, strSerializedData.c_str());
	::UnmapViewOfFile(static_cast<LPVOID>(strSharedData));

	LRESULT lRet = ::SendMessage(hWndActive, WM_CHILDFRAMEFINDKEYWORD, (WPARAM)lpszKeyWord, 0);

	::CloseHandle(hMap);
	return lRet;
}

LRESULT CMainFrame::Impl::OnHilight(CString strKeyword)
{
	HWND hWndActive = m_ChildFrameClient.GetActiveChildFrameWindow();
	if (hWndActive == NULL)
		return 0;

	COPYDATASTRUCT cds = { sizeof(cds) };
	cds.dwData	= kHilightText;
	cds.lpData	= static_cast<LPVOID>(strKeyword.GetBuffer(0));
	cds.cbData	= (strKeyword.GetLength() + 1) * sizeof(TCHAR);
	return SendMessage(hWndActive, WM_COPYDATA, (WPARAM)m_hWnd, (LPARAM)&cds);
}


void	CMainFrame::Impl::OnViewBar(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	auto funcToggleBandVisible = [this](int nCtrlID) {
		MtlToggleBandVisible(m_ReBar, nCtrlID);
		UpdateLayout();
	};
	// Togle
	switch (nID) {
	case ID_VIEW_SEARCHBAR:		funcToggleBandVisible(IDC_SEARCHBAR);	break;
	case ID_VIEW_COMMANDBAR:	funcToggleBandVisible(ATL_IDW_COMMAND_BAR);	break;
	case ID_VIEW_TOOLBAR:		funcToggleBandVisible(ATL_IDW_TOOLBAR);	break;
	case ID_VIEW_ADDRESSBAR:	funcToggleBandVisible(IDC_ADDRESSBAR);	break;
	case ID_VIEW_LINKBAR:		funcToggleBandVisible(IDC_LINKBAR);	break;
	case ID_VIEW_TABBAR:		funcToggleBandVisible(IDC_MDITAB);	break;

	default:
		switch (nID) {
		case ID_VIEW_GOBUTTON:		m_AddressBar.ShowGoButton(!CAddressBarOption::s_bGoBtnVisible);	break;

		case ID_VIEW_TABBAR_MULTI:
			{
				CTabBarOption::s_bMultiLine = !CTabBarOption::s_bMultiLine;
				CTabBarOption::WriteProfile();	// �ݒ�̕ۑ�
				m_TabBar.ReloadSkin();
			}
			break;

		case ID_VIEW_TOOLBAR_CUST:	m_ToolBar.Customize();	break;

		case ID_VIEW_TOOLBAR_LOCK:
			{
				REBARBANDINFO rbbi = { sizeof (REBARBANDINFO) };
				rbbi.fMask	= RBBIM_STYLE;
				if ( !m_ReBar.GetBandInfo(0, &rbbi) )
					return ;	// Band������Ȃ�

				m_ReBar.LockBands( !( (rbbi.fStyle & RBBS_NOGRIPPER) != 0) );
			}
			break;

		case ID_VIEW_STATUS_BAR:
			{
				BOOL bNew = !::IsWindowVisible(m_hWndStatusBar);
				//UpdateTitleBar(_T(""), 0);		//+++		status�I�t����I���ɂ����Ƃ��ɁA�^�C�g���o�[�ɏo���Ă����X�e�[�^�X���������������.
				CIniFileO	pr(g_szIniFileName, _T("Main"));
				pr.SetValue( bNew, _T("statusbar.Visible") );

				::ShowWindow(m_hWndStatusBar, bNew ? SW_SHOWNOACTIVATE : SW_HIDE);
				UpdateLayout();
			}
			break;
		}
		break;
	}	

}

void	CMainFrame::Impl::OnSetFocusToBar(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	switch (nID) {
	case ID_SETFOCUS_ADDRESSBAR:	::SetFocus( m_AddressBar.GetEditCtrl() );	break;
	case ID_SETFOCUS_SEARCHBAR:		::SetFocus( m_SearchBar.GetEditCtrl() );	break;
	case ID_SETFOCUS_SEARCHBAR_ENGINE:	m_SearchBar.SetFocusToEngine();		break;
	case ID_VIEW_ADDBAR_DROPDOWN:	m_AddressBar.ShowDropDown(!m_AddressBar.GetDroppedStateEx());	break;
	}
}


void	CMainFrame::Impl::OnFavoriteAdd(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	ChildFrameUIData* pUIData = m_ChildFrameUIState.GetActiveChildFrameUIData();
	if (pUIData == nullptr)
		return ;

	bool bOldShell = _check_flag(MAIN_EX_ADDFAVORITEOLDSHELL, CMainOption::s_dwMainExtendedStyle);
	MtlAddFavorite(pUIData->strLocationURL, pUIData->strTitle, bOldShell, DonutGetFavoritesFolder(), m_hWnd);

	SendMessage(WM_REFRESH_EXPBAR);
}

void	CMainFrame::Impl::OnFavoriteOrganize(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	bool bOldShell = _check_flag(MAIN_EX_ORGFAVORITEOLDSHELL, CMainOption::s_dwMainExtendedStyle);
	MtlOrganizeFavorite( m_hWnd, bOldShell, DonutGetFavoritesFolder() );
}

/// IE�̃I�v�V������\������
void	CMainFrame::Impl::OnViewOption(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (m_ChildFrameClient.GetActiveChildFrameWindow() == NULL)
		MtlShowInternetOptions();
	else
		SetMsgHandled(FALSE);	// ChildFrame��
}

/// Donut�̃I�v�V������\��
void	CMainFrame::Impl::OnViewOptionDonut(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	BOOL	bSkinChange = FALSE;

	CMenu	menu		= CExMenuManager::LoadFullMenu();

	CMainPropertyPage				pageMain(m_hWnd);
	CMainPropertyPage2				pageMain2(m_hWnd, m_RecentClosedTabList);
	CDLControlPropertyPage			pageDLC(m_hWnd);
	CMDITabPropertyPage 			pageTab(&m_TabBar, menu.m_hMenu);
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

	CString strURL, strTitle;
	//CChildFrame* pChild = GetActiveChildFrame();
	//if (pChild) {
	//	strURL	 = pChild->GetLocationURL();
	//	strTitle = MtlGetWindowText(pChild->GetHwnd());
	//}

	CIgnoredURLsPropertyPage		pageURLs(strURL);
	CCloseTitlesPropertyPage		pageTitles( strTitle );
	CUrlSecurityPropertyPage		pageUrlSecu(strURL);		//+++
	CUserDefinedCSSPropertyPage		pageUserCSS(strURL);
	CUserDefinedJsPropertyPage		pageUserJs(strURL);
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
	sheet.AddPage( pageUserCSS	 , FALSE );
	sheet.AddPage( pageUserJs	 , FALSE );
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

	RtlSetMinProcWorkingSetSize();		//+++ (�������̗\��̈���ꎞ�I�ɍŏ����B�E�B���h�E���ŏ��������ꍇ�Ɠ���)
}


void	CMainFrame::Impl::OnTabClose(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	switch (nID) {
	case ID_WINDOW_CLOSE_EXCEPT:	/// ���̃E�B���h�E�ȊO�����
	case ID_WINDOW_CLOSE_ALL:		/// �S�ẴE�B���h�E�����(���ׂĕ���)
		{
			if ( !CDonutConfirmOption::OnCloseAll(m_hWnd) )
				return ;
			CWaitCursor  cur;
			CLockRedrawMDIClient		lock(m_ChildFrameClient);
			CDonutTabBar::CLockRedraw	lock2(m_TabBar);
			vector<HWND>	vechwnd;
			HWND	hWndActive = m_ChildFrameUIState.GetActiveChildFrameWindowHandle();
			m_TabBar.ForEachWindow([&](HWND hWnd) { 
				if (nID == ID_WINDOW_CLOSE_EXCEPT && hWnd == hWndActive)
					return ;
				vechwnd.push_back(hWnd);
			});
			boost::for_each(vechwnd, [](HWND hWnd) { ::PostMessage(hWnd, WM_CLOSE, 0, 0); });
			//RtlSetMinProcWorkingSetSize();
		}
		break;

	case ID_LEFT_CLOSE:		/// �^�u�̉E��/���������ׂĕ���
	case ID_RIGHT_CLOSE:
		{
			bool bLeft = (nID == ID_LEFT_CLOSE);
			if ( !CDonutConfirmOption::OnCloseLeftRight(m_hWnd, bLeft) )
				return ;

			CWaitCursor cur;
			HWND	hWndActive = m_ChildFrameClient.GetActiveChildFrameWindow();
			int 	nCurSel    = m_TabBar.GetTabIndex(hWndActive);
			if (nCurSel == -1 || hWndActive == NULL)
				return ;

			CSimpleArray<HWND> arrWnd;
			int nCount = m_TabBar.GetItemCount();
			for (int ii = 0; ii < nCount; ++ii) {
				HWND hWnd = m_TabBar.GetTabHwnd(ii);
				if (bLeft && ii < nCurSel)
					arrWnd.Add(hWnd);
				else if (bLeft == false && ii > nCurSel)
					arrWnd.Add(hWnd);
			}
			for (int ii = 0; ii < arrWnd.GetSize(); ++ii) {
				::PostMessage(arrWnd[ii], WM_CLOSE, 0, 0);
			}
		}
		break;
	}
}

void	CMainFrame::Impl::OnTabSwitch(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	if (nID == ID_TAB_LEFT) {
		m_TabBar.LeftTab();
	} else {
		m_TabBar.RightTab();
	}
}

/// ���C�����j���[�� [�\��]-[�c�[���o�[]��\������
LRESULT CMainFrame::Impl::OnShowToolBarMenu()
{
	CPoint pt;
	::GetCursorPos(&pt);	
	
	CMenuHandle submenu = CMenuHandle(m_CmdBar.m_hMenu).GetSubMenu(2).GetSubMenu(0);
	if ( submenu.IsMenu() )
		submenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd);
	return 0;
}



void	CMainFrame::Impl::OnTabCreate(HWND hWndChildFrame, DWORD dwOption)
{
	if (dwOption & TAB_LINK)
		m_TabBar.SetLinkState(LINKSTATE_A_ON);
	if (!_check_flag(MAIN_EX_NOACTIVATE_NEWWIN, CMainOption::s_dwMainExtendedStyle))
		dwOption |= TAB_ACTIVE;
	m_TabBar.OnMDIChildCreate(hWndChildFrame, (dwOption & TAB_ACTIVE) != 0);
	
	//if ( _check_flag(m_view.m_ViewOption.m_dwExStyle, DVS_EX_OPENNEWWIN)) {
	//	m_MDITab.NavigateLockTab(m_hWnd, true);
	//}
	if (m_deqNewChildFrameData.size() > 0) {
		m_deqNewChildFrameData.pop_front();
		if (m_deqNewChildFrameData.size() > 0) {
			m_TabBar.SetInsertIndex(m_TabBar.GetItemCount());
			CChildFrame::AsyncCreate(*m_deqNewChildFrameData[0]);
		} else {
			m_TabBar.InsertHere(false);
			PostMessage(WM_INITPROCESSFINISHED);
		}
	}
}

void	CMainFrame::Impl::OnTabDestory(HWND hWndChildFrame)
{
	m_TabBar.OnMDIChildDestroy(hWndChildFrame);
}

void	CMainFrame::Impl::OnAddRecentClosedTab(HWND hWndChildFrame)
{
	CString strSharedMemName;
	strSharedMemName.Format(_T("%s0x%x"), CHILDFRAMEDATAONCLOSESHAREDMEMNAME, hWndChildFrame);
	HANDLE hMap = ::OpenFileMapping(FILE_MAP_READ, FALSE, strSharedMemName);
	LPTSTR sharedMemData = (LPTSTR)::MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
	std::wstring serializedData = sharedMemData;
	::UnmapViewOfFile((LPVOID)sharedMemData);
	::CloseHandle(hMap);

	ChildFrameDataOnClose*	pClosedTabData = new ChildFrameDataOnClose;
	std::wstringstream ss;
	ss << serializedData;
	boost::archive::text_wiarchive ar(ss);
	ar >> *pClosedTabData;

	m_RecentClosedTabList.AddToList(pClosedTabData);
}

bool	CMainFrame::Impl::_IsRebarBandLocked()
{
	CReBarCtrl	  rebar(m_hWndToolBar);
	REBARBANDINFO rbbi = { sizeof (REBARBANDINFO) };
	rbbi.fMask	= RBBIM_STYLE;
	if ( !rebar.GetBandInfo(0, &rbbi) )
		return false;

	return (rbbi.fStyle & RBBS_NOGRIPPER) != 0;
}


bool	CMainFrame::Impl::_CheckCookies(UINT nID)
{
	HINSTANCE	hInstDLL = LoadLibrary(_T("wininet.dll"));
	if (hInstDLL == NULL)
		return false;
	typedef DWORD (WINAPI* fpPrivacyGetZonePreferenceW)(DWORD, DWORD, LPDWORD, LPWSTR, LPDWORD);
	fpPrivacyGetZonePreferenceW PrivacyGetZonePreferenceW = 
		(DWORD (WINAPI*)(DWORD,DWORD,LPDWORD,LPWSTR, LPDWORD))GetProcAddress(hInstDLL,"PrivacyGetZonePreferenceW");
	if (PrivacyGetZonePreferenceW == NULL) {
		FreeLibrary(hInstDLL);
		return false;
	}

	DWORD	dwCookie1 = 0;
	DWORD	dwCookie2 = 0;
	DWORD	BufLen	  = 0;
	DWORD	ret;
	ret = PrivacyGetZonePreferenceW(URLZONE_INTERNET, PRIVACY_TYPE_FIRST_PARTY, &dwCookie1, NULL, &BufLen);
	ret = PrivacyGetZonePreferenceW(URLZONE_INTERNET, PRIVACY_TYPE_THIRD_PARTY, &dwCookie2, NULL, &BufLen);

	bool bSts = false;
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
	FreeLibrary(hInstDLL);
	return bSts;
}


void	CMainFrame::Impl::_FullScreen(bool bOn)
{
	_ShowBandsFullScreen(bOn);
	static bool m_bOldMaximized = false;	//\\+
	SetRedraw(FALSE);

	if (bOn) {							// remove caption
		// save prev visible
		CWindowPlacement	wndpl;
		GetWindowPlacement(&wndpl);

		CIniFileO	pr( g_szIniFileName, _T("Main") );
		wndpl.WriteProfile(pr, _T("frame."));

		m_bFullScreen = true;

		m_bOldMaximized 	= (wndpl.showCmd == SW_SHOWMAXIMIZED);
		ModifyStyle(WS_CAPTION, 0);

		SetMenu(NULL);
		ShowWindow(SW_HIDE);

//		m_mcToolBar.m_bVisible = true;

		ShowWindow(SW_MAXIMIZE);
		if (m_bOldMaximized == false) {
			SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_FRAMECHANGED);
		}
		
	} else {
		m_bFullScreen = false;

//		m_mcToolBar.m_bVisible = false;
		// restore prev visible
		if (CSkinOption::s_nMainFrameCaption)	//+++	�L���v�V�����̒����͂Â�������̏ꍇ
			ModifyStyle(0, WS_CAPTION);

		if (m_bOldMaximized == false) {
			ShowWindow(SW_RESTORE);		//+++ �T�C�Y��߂�
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

void	CMainFrame::Impl::_ShowBandsFullScreen(bool bOn)
{
	static BOOL s_bOldVisibleStatusBar;
	static CSimpleMap<UINT, BOOL>	s_mapToolbarOldVisible;
	int nIndex = 0;
	if (bOn) { // remove caption
		// save prev visible
		int nToolbarPluginCount = CPluginManager::GetCount(PLT_TOOLBAR);
		s_mapToolbarOldVisible.RemoveAll();

		for (nIndex = 0; nIndex < _countof(STDBAR_ID); nIndex++) {
			s_mapToolbarOldVisible.Add( STDBAR_ID[nIndex], MtlIsBandVisible( m_hWndToolBar, STDBAR_ID[nIndex] ) );
		}
		//for (nIndex = 0; nIndex < nToolbarPluginCount; nIndex++) {
		//	s_mapToolbarOldVisible.Add( IDC_PLUGIN_TOOLBAR + nIndex,
		//								MtlIsBandVisible( m_hWndToolBar, IDC_PLUGIN_TOOLBAR + nIndex ) );
		//}
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
		//for (nIndex = 0; nIndex < nToolbarPluginCount; nIndex++)
		//	MtlShowBand(m_hWndToolBar, IDC_PLUGIN_TOOLBAR + nIndex, FALSE);
	} else {
		SetRedraw(FALSE);
		for (nIndex = 0; nIndex < s_mapToolbarOldVisible.GetSize(); nIndex++)
			MtlShowBand( m_hWndToolBar, s_mapToolbarOldVisible.GetKeyAt(nIndex), s_mapToolbarOldVisible.GetValueAt(nIndex) );

		::ShowWindow(m_hWndStatusBar, s_bOldVisibleStatusBar ? SW_SHOWNOACTIVATE : SW_HIDE);
	}
	SetRedraw(TRUE);
}

