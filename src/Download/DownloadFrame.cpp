// MainFrm.cpp

#include "stdafx.h"
#include "DownloadFrame.h"
#include "DownloadOptionDialog.h"
#include "DownloadManager.h"
#include "../IniFile.h"


/////////////////////////////////////////////////////////////
// CDownloadFrame

// Constructor
CDownloadFrame::CDownloadFrame()
	: m_bVisible(false)
{
}


////////////////////////////////////////
// Message map

int		CDownloadFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// �X�v���b�^�E�B���h�E���쐬
	m_wndSplitter.Create(m_hWnd);
	m_wndSplitter.SetSplitterExtendedStyle(SPLIT_RIGHTALIGNED);

	// DownloadingListView�쐬
	m_wndDownloadingListView.Create(m_wndSplitter, rcDefault, nullptr
		, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_BORDER);

	m_wndSplitter.SetSplitterPane(SPLIT_PANE_TOP, m_wndDownloadingListView);

	// DownloadedListView�쐬
	m_wndDownloadedListView.Create(m_wndSplitter);
	m_wndDownloadedListView.ModifyStyle(0, WS_BORDER);

	m_wndSplitter.SetSplitterPane(SPLIT_PANE_BOTTOM, m_wndDownloadedListView);

	m_wndDownloadingListView.SetAddDownloadedItemfunc(std::bind(&CDownloadedListView::AddDownloadedItem, &m_wndDownloadedListView, std::placeholders::_1));

	// �ȑO�̃��j���[�̍폜
	SetMenu(NULL);

	CToolBarCtrl	wndToolBar;
	HWND hWndToolBar = wndToolBar.Create(m_hWnd, rcDefault, 0, ATL_SIMPLE_TOOLBAR_PANE_STYLE | TBSTYLE_LIST);
	wndToolBar.SetButtonStructSize();
	wndToolBar.SetExtendedStyle(TBSTYLE_EX_MIXEDBUTTONS);

	CBitmap bmpToolBar = AtlLoadBitmap(IDB_DLTOOLBAR);
	CImageList	imagelist;
	imagelist.Create(16, 16, ILC_COLOR32 | ILC_MASK, 4, 1);
	imagelist.Add(bmpToolBar, RGB(0, 0, 0));
	wndToolBar.SetImageList(imagelist);

	wndToolBar.AddButton(ID_DLOPENOPTION , TBSTYLE_BUTTON, TBSTATE_ENABLED , 0, _T("�I�v�V����"), 0);
	wndToolBar.AddButton(ID_SET_DLFOLDER , TBSTYLE_BUTTON, TBSTATE_ENABLED , 1, _T("DL��t�H���_��ݒ肷��"), 0);
	wndToolBar.AddButton(ID_OPEN_DLFOLDER, TBSTYLE_BUTTON, TBSTATE_ENABLED , 2, _T("DL��t�H���_���J��"), 0);
	wndToolBar.AddButton(ID_DLAPP_ABOUT	 , TBSTYLE_BUTTON, TBSTATE_ENABLED , 3, _T("About"), 0);

	CreateSimpleReBar(ATL_SIMPLE_REBAR_NOBORDER_STYLE);

	AddSimpleReBarBand(hWndToolBar, NULL, TRUE);

	m_hWndClient = m_wndSplitter;

	// �ʒu�𕜌�����
	CString strIniFile = GetConfigFilePath(_T("Download.ini"));
	CIniFileI	pr(strIniFile, _T("Main"));
	CRect rcWindow;
	rcWindow.top	= pr.GetValue(_T("top"), -1);
	rcWindow.left	= pr.GetValue(_T("left"), -1);
	rcWindow.right	= pr.GetValue(_T("right"), -1);
	rcWindow.bottom	= pr.GetValue(_T("bottom"), -1);
	if (rcWindow != CRect(-1, -1, -1, -1)) {
		MoveWindow(rcWindow);
	} else {
		enum { kDefaultcxy = 500 };
		MoveWindow(0, 0, kDefaultcxy, kDefaultcxy);
		CenterWindow();
	}
	if (pr.GetValue(_T("ShowWindow"), FALSE) && CDownloadManager::UseDownloadManager()) {
		ShowWindow(TRUE);
		m_bVisible = true;
	}
	int nSplitterPos = pr.GetValue(_T("SplitterPos"), -1);
	if (nSplitterPos < -1)
		nSplitterPos = -1;
	m_wndSplitter.SetSplitterPos(nSplitterPos);

	CDLOptions::LoadProfile();
	if (::PathIsDirectory(CDLOptions::strDLFolderPath) == FALSE && CDownloadManager::UseDownloadManager()) {
		MessageBox(_T("�_�E�����[�h��̃t�H���_�����݂��܂���B\n")
			_T("�_�E�����[�h�J�n�O�ɃI�v�V��������ۑ���t�H���_��ݒ肵�Ă��������B"));
	}

	return 0;
}

//---------------------------------------------
/// �E�B���h�E�j�������Ƃ� : �E�B���h�E�̈ʒu�T�C�Y��ۑ�����
void	CDownloadFrame::OnDestroy()
{
	CString strIniFile = GetConfigFilePath(_T("Download.ini"));
	CIniFileO pr(strIniFile, _T("Main"));

	// �ʒu��ۑ�����
	CRect rcWindow;
	if (m_bVisible && IsIconic() == false) {
		GetWindowRect(rcWindow);
		pr.SetValue(TRUE, _T("ShowWindow"));
	} else {
		rcWindow = m_rcWindowPos;
		pr.SetValue(FALSE, _T("ShowWindow"));
	}
	if (rcWindow.IsRectNull() == FALSE && CDownloadManager::UseDownloadManager()) {
		pr.SetValue(rcWindow.top, _T("top"));
		pr.SetValue(rcWindow.left, _T("left"));
		pr.SetValue(rcWindow.right, _T("right"));
		pr.SetValue(rcWindow.bottom, _T("bottom"));
	}
	int nSplitterPos = m_wndSplitter.GetSplitterPos();
	if (nSplitterPos < 0)
		nSplitterPos = -1;
	pr.SetValue(nSplitterPos, _T("SplitterPos"));

}

//-------------------------------------
/// �I�v�V�����ݒ��ʂ��J��
LRESULT CDownloadFrame::OnOpenOption(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CDLOptionDialog dlg;
	if  (dlg.DoModal() == IDOK) {
		// �ݒ�̍X�V��ʒm
		::SendMessage(m_hWndParent, WM_SETDLCONFIGTOGLOBALCONFIG, 0, 0);
		//m_wndDownloadingListView.SetDLFolder(CDLOption::strDLFolderPath);
	}
	return 0;
}

//-------------------------------------
/// DL��t�H���_��ݒ肷��
LRESULT CDownloadFrame::OnSetDLFolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CString strPath;
	CShellFileOpenDialog SHdlg(NULL, FOS_PICKFOLDERS);
	if (SHdlg.IsNull() == false) {
		if (SHdlg.DoModal(m_hWnd) == IDOK) {
			SHdlg.GetFilePath(strPath);
		}
	} else {
		CFolderDialog dlg(NULL, _T("�t�H���_��I�����Ă��������B"));
		if (dlg.DoModal(m_hWnd) == IDOK) {
			strPath = dlg.GetFolderPath();
			
		}
	}
	if (strPath.IsEmpty() == FALSE) {
		MTL::MtlMakeSureTrailingBackSlash(strPath);
		CDLOptions::strDLFolderPath	   = strPath;
		CDLOptions::SavePathToHistory(strPath, CDLOptions::kDLFolderHistory);
		CDLOptions::SaveProfile();
	}
	return 0;
}

//-------------------------------------
/// DL��t�H���_���J��
LRESULT CDownloadFrame::OnOpenDLFolder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	HINSTANCE ret = ::ShellExecute(m_hWnd, NULL, CDLOptions::strDLFolderPath, NULL, NULL, SW_NORMAL);
	return 0;
}

//-------------------------------------
/// �o�[�W��������\��
LRESULT CDownloadFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	//CAboutDlg dlg;
	//dlg.DoModal();
	MessageBox(_T("�@�@�@version 3.4�@�@�@"), _T("DownloadManager"));
	return 0;
}


//-------------------------------------
/// �E�B���h�E���j������Ȃ��悤�Ɂ��E�B���h�E�̃T�C�Y���T����
void	CDownloadFrame::OnClose()
{
	ShowWindow(SW_HIDE);
	//SetParent(m_hWndParent);
	m_bVisible = false;
	GetWindowRect(&m_rcWindowPos);
}






















