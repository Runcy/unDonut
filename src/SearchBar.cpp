/**
 *	@file	SearchBar.cpp
 *	@brief	�����o�[
 *	@note
 *		template�Ŏ�������Ă��� SearchBar.h �𕁒ʂ�class�ɂ��� .h, .cpp ����������.
 */


#include "stdafx.h"
#include "DonutPFunc.h"
#include "DonutViewOption.h"
#include "MtlDragDrop.h"
#include "HlinkDataObject.h"
#include "ExStyle.h"
#include <atlctrls.h>
//#include "StringEncoder.h"			//+++ �s�v��
#include <winnls32.h>
#include "ParseInternetShortcutFile.h"
#include "Donut.h"

#include "SearchBar.h"



#if defined USE_ATLDBGMEM
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



#define ENGINENAME_FOR_NO_SEARCH_INI	_T("search.ini�������̂�Google�̂�")


//////////////////////////////////////////////////////////////////////////////
// CDonutSearchBar

//+++	ParseInternetShortcutFile()�p�Ɏ蔲���Ŏ��g�̃|�C���^��p��...
CDonutSearchBar*	CDonutSearchBar::s_pThis_ = 0;

// Constructor
CDonutSearchBar::CDonutSearchBar()
	: m_wndKeyword(this, 1)
	, m_wndEngine  (this, 2)
	, m_wndKeywordCombo(this, 3)
	, m_cxBtnsBtn(0)			//+++ �����������p
	, m_clrMask(0)				//+++
	, m_hWndKeywordList(0)		//+++
	, m_has(0)					//+++
	, m_nEngineWidth(0)			//+++
	, m_bHilightSw(0)			//+++
	//, m_nHilightBtnNo(0)		//+++
	, m_bExistManifest(IsExistManifestFile())	//+++
	, m_dwTinyWordButton(0)		//+++
	, m_hCursor(NULL)
	, m_bDragAccept(false)
	, m_bShowToolBar(FALSE)
	, m_bLoadedToolBar(FALSE)
	, m_bDragFromItself(false)
{
	//+++ 1�����C���X�^���X������Ȃ��A���낤�Ƃ��āAParseInternetShortcutFile()�p�Ɏ蔲���Ȏ������w���|�C���^.
	ATLASSERT(s_pThis_ == NULL);
	s_pThis_ = this;
}


// Search.ini�̐�΃p�X��Ԃ�
CString CDonutSearchBar::GetSearchIniPath()
{
	CIniFileI pr( g_szIniFileName, _T("Search") );
	CString 	strPath = pr.GetStringUW(_T("Path"));
	pr.Close();

	if (strPath.IsEmpty()) {
		strPath = _GetFilePath( _T("Search\\Search.ini") );
		if (::PathFileExists(strPath) == FALSE)
			strPath = _GetFilePath( _T("Search.ini") ); 		//�ȑO�̎d�l��Go
	}

	return strPath;
}


///+++ ������������擾����
CString	CDonutSearchBar::GetSearchStr()
{
	HWND hWnd	= GetEditCtrl();

	//+++ �Ȃ�ׂ��A�A���P�[�g���������Ȃ��悤�ɂ��Ă݂�.
	enum { NAME_LEN = 0x1000 };
	TCHAR	name[ NAME_LEN ] = _T("\0");
	int 	nLen	= ::GetWindowTextLength(hWnd);
	if (nLen >= NAME_LEN)
		nLen	 	= NAME_LEN - 1;
	int 	nRetLen = ::GetWindowText(hWnd, name, nLen + 1);
	name[nLen]		= _T('\0');	//+++1.48c: ������̃o�O�񍐂̔��f. nLen+1�͕s����...
	m_strKeyword	= name;

	return m_strKeyword;
}

///+++ �G���W�����̎擾.
CString CDonutSearchBar::GetSearchEngineStr()	//+++
{
	HWND	hWnd	= m_cmbEngine;

	//+++ �Ȃ�ׂ��A�A���P�[�g���������Ȃ��悤�ɂ��Ă݂�.
	enum { NAME_LEN = 0x1000 };
	TCHAR	name[ NAME_LEN ] = _T("\0");
	int 	nLen	= ::GetWindowTextLength(hWnd);
	if (nLen >= NAME_LEN)
		nLen	 	= NAME_LEN - 1;
	int 	nRetLen = ::GetWindowText(hWnd, name, nLen + 1);
	name[nLen]		= _T('\0');	//+++1.48c: ������̃o�O�񍐂̔��f. nLen+1�͕s����...
	m_strEngine		= name;

	return m_strEngine;
}


// �T�[�`�G���W�����X�g�����j���[�Ƃ��ĕԂ�
CMenuHandle CDonutSearchBar::GetSearchEngineMenuHandle()
{
  #if 1	//+++
	CMenu		menu0;
	_MakeSearchEngineMenu(menu0);
	CMenuHandle menu = menu0.GetSubMenu(0);
	menu0.RemoveMenu(0, MF_BYPOSITION);
	return 		menu;
  #else
	if (m_engineMenu.m_hMenu == 0)
		MakeSearchEngineMenu(m_engineMenu);
	if (m_engineMenu.m_hMenu == 0)
		return 0;
	CMenuHandle menu = m_engineMenu.GetSubMenu(0);
	return menu;
  #endif
}



/// AddressBar���Ŏg���R���{������̐ݒ�
void CDonutSearchBar::InitComboBox_for_AddressBarPropertyPage(CComboBox& rCmbCtrl, CComboBox& rCmbShift)
{
	int nCount = m_cmbEngine.GetCount();
	for (int i = 0; i < nCount; i++) {
		CString strBuf;
		m_cmbEngine.GetLBText(i, strBuf);
		rCmbCtrl.AddString(strBuf);
		rCmbShift.AddString(strBuf);
	}
}



//�Ȃɂ��`�悪���������Ȃ�o�O��fix minit
// ���o�[�̔w�i���R�s�[����
LRESULT CDonutSearchBar::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	// CAddressBarCtrlImpl����R�s�y
	HWND	hWnd	= GetParent();
	CPoint	pt;

	MapWindowPoints(hWnd, &pt, 1);
	::OffsetWindowOrgEx( (HDC) wParam, pt.x, pt.y, NULL );
	LRESULT lResult = ::SendMessage(hWnd, WM_ERASEBKGND, wParam, 0L);
	::SetWindowOrgEx((HDC) wParam, 0, 0, NULL);
	return 1; //lResult;
}



void CDonutSearchBar::SetSearchStr(const CString& strWord)
{
	::SetWindowText(GetEditCtrl(), strWord);
}


// private:
// �c�[���o�[������������
void CDonutSearchBar::_InitToolBar(int cx, int cy, COLORREF clrMask, UINT nFlags /*= ILC_COLOR24*/)
{
	m_clrMask	= clrMask;

	CImageList	imgs;
	MTLVERIFY( imgs.Create(cx, cy, nFlags | ILC_MASK, 1, 1) );
	CBitmap 	bmp = AtlLoadBitmapImage(LPCTSTR(GetSkinSeachBarPath(0)), LR_LOADFROMFILE);
	if (bmp.m_hBitmap == NULL)
		bmp.LoadBitmap(IDB_SEARCHBUTTON/*nImageBmpID*/);			//+++	skin�̏����������ɂ���̂ŁA�f�t�H���g�f�ނ����̏�Őݒ�ɂ��Ƃ�...
	imgs.Add(bmp, clrMask);

	CImageList	imgsHot;
	MTLVERIFY( imgsHot.Create(cx, cy, nFlags | ILC_MASK, 1, 1) );
	CBitmap 	bmpHot = AtlLoadBitmapImage(LPCTSTR(GetSkinSeachBarPath(1)), LR_LOADFROMFILE);
	if (bmpHot.m_hBitmap == NULL)
		bmpHot.LoadBitmap(IDB_SEARCHBUTTON_HOT/*nHotImageBmpID*/);		//+++	skin�̏����������ɂ���̂ŁA�f�t�H���g�f�ނ����̏�Őݒ�ɂ��Ƃ�...
	imgsHot.Add(bmpHot, clrMask);

  #if 1 //+++ Disable���p�ӂ���
	CString str = GetSkinSeachBarPath(2);
	int 	dis = 0;
	if (::PathFileExists(str) == FALSE) {						//+++ �摜�t�@�C�����Ȃ���
		if (Misc::IsExistFile(GetSkinSeachBarPath(0))) {	//+++ �ʏ킪�����
			str = GetSkinSeachBarPath(0);					//+++ �ʏ��ő�p
		} else {											//+++ �ʏ���Ȃ����
			dis = IDB_SEARCHBUTTON_DIS; 					//+++ �f�t�H���g��Disable����g��.
		}
	}
	CImageList	imgsDis;
	MTLVERIFY( imgsDis.Create(cx, cy, nFlags | ILC_MASK, 1, 1) );
	CBitmap 	bmpDis = AtlLoadBitmapImage(LPCTSTR(str), LR_LOADFROMFILE);
	if (bmpDis.m_hBitmap == NULL && dis)
		bmpDis.LoadBitmap(dis); 					//+++	skin�̏����������ɂ���̂ŁA�f�t�H���g�f�ނ����̏�Őݒ�ɂ��Ƃ�...
	imgsDis.Add(bmpDis, clrMask);
  #endif

	CIniFileI	pr( g_szIniFileName, _T("SEARCH") );
	//DWORD 	jikken = pr.GetValue("Jikken");
	m_dwTinyWordButton = pr.GetValue(_T("NumberButton"));		//+++ �P��{�^���łȂ��A����������5�{�^�����g����? (NoWordButton=1�̎��̂ݗL��)
	pr.Close();

	/* �c�[���o�[����� */
	DWORD		flags  =  WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS
						| CCS_NODIVIDER  /*| CCS_NORESIZE */ | CCS_NOPARENTALIGN | CCS_TOP
						| TBSTYLE_TOOLTIPS | TBSTYLE_FLAT ;
	m_wndToolBar.Create(m_hWnd, rcDefault, _T("SearchWordButton"), flags);

	//+++ �ǂ����Â�os�ł́ACreateWindow���ɐݒ肵����܂����炵��?...�̂Ō�Â���SetWindowLong����K�v������?... �֌W�Ȃ������͗l.
	if (s_bNoWordButton == 0 || m_dwTinyWordButton)
	{
		flags	|= (UINT)m_wndToolBar.GetWindowLong(GWL_STYLE);
		flags	|= TBSTYLE_LIST | TBSTYLE_TRANSPARENT;	//+++ �ǉ� (�����P��̕������\������ɂ͕K�v)
		m_wndToolBar.SetWindowLong(GWL_STYLE, flags);
	}

	m_wndToolBar.SetButtonStructSize();
	m_wndToolBar.SetImageList(imgs);
	m_wndToolBar.SetHotImageList(imgsHot);
	m_wndToolBar.SetDisabledImageList(imgsDis); //+++

	if (s_bNoWordButton) {
		addDefaultToolBarIcon(0);		//+++ �P��{�^���������[�h�ł́A�{�^���ݒ�.
	} else {
		toolBarAddButtons();			//+++ �c�[���o�[�Ƀ{�^����ǉ�.(�P��{�^���A���̏ꍇ)
	}

	m_bShowToolBar	 = TRUE;
	m_bLoadedToolBar = TRUE;

  #if 0	//+++ ���̊֐����������ƂŁA��������̂ł����ł����Ⴞ��
	ShowToolBarIcon(true);
  #endif
}

void	CDonutSearchBar::_RefreshBandInfo(int nHeight)
{
	HWND		  hWndReBar = GetParent();
	CReBarCtrl	  rebar(hWndReBar);
	REBARBANDINFO rbBand = { sizeof (REBARBANDINFO) };
	rbBand.fMask  = RBBIM_CHILDSIZE;

	int nIndex	= rebar.IdToIndex( GetDlgCtrlID() );
	rebar.GetBandInfo(nIndex, &rbBand);

	if ( rbBand.cyMinChild != nHeight ) {
		// Calculate the size of the band
		rbBand.cxMinChild = 0;
		rbBand.cyMinChild = nHeight;

		rebar.SetBandInfo(nIndex, &rbBand);
	}
}




LRESULT CDonutSearchBar::OnToolTipText(int idCtrl, LPNMHDR pnmh, BOOL & /*bHandled*/)
{
	LPNMTTDISPINFO pDispInfo  = (LPNMTTDISPINFO) pnmh;

	pDispInfo->szText[0] = 0;
	if (pDispInfo->uFlags & TTF_IDISHWND)
		return S_OK;

  //#if 1		//+++
  //  CString 	   strKeyword = GetSearchStr();
  //#else
  //  CString 	   strKeyword = MtlGetWindowText(m_wndKeyword);
  //#endif
	CString 	   strHeader  = _T('\"');
	CString 	   strHelp	  = _T("\" ��");
	CString 	   strEngine  = GetSearchEngineStr();	// MtlGetWindowText(m_wndEngine);

	TCHAR		   szBuff[80+4];
	szBuff[0]	= 0;
	AtlCompactPathFixed( szBuff, GetSearchStr()/*strKeyword*/, 80 - strHelp.GetLength() - strHeader.GetLength() );

	strHeader	+= szBuff;
	strHeader	+= strHelp;

	switch (idCtrl) {
	case ID_SEARCH_WEB:
		strHeader += _T("\"") + strEngine + _T("\"�Ō�������");
		break;
	case ID_SEARCHBAR_HILIGHT:
		strHeader += _T("�n�C���C�g����");
		break;
	case ID_SEARCH_PAGE:
		strHeader += _T("���̃y�[�W�Ō�������(��:Shift+Enter ��:Ctrl+Enter)");
		break;
	default:	// ID_SEARCH_WORD00..09
		return S_OK;
	}

	::lstrcpyn(pDispInfo->szText, strHeader, 80);

	return S_OK;
}



void CDonutSearchBar::OnCommand(UINT uFlag, int nID, HWND hWndCtrl)
{
	if (hWndCtrl == m_wndToolBar.m_hWnd) {
	  //#if 1	//+++
	  //	CString str  = GetSearchStr();
	  //#else
	  //	CEdit	edit = GetEditCtrl();
	  //	CString str  = MtlGetWindowText(edit);
	  //#endif
		switch (nID) {
		case ID_SEARCH_WEB:
		  #if 1
			SearchWeb();
		  #else
			_OnCommand_SearchWeb( GetSearchStr() /*str*/ );
		  #endif
			break;
		case ID_SEARCHBAR_HILIGHT:	//ID_SEARCH_HILIGHT:
			//GetHilightBtnFlag();
			//m_bHilightSw^=1;
		  #if 1	//+++
			SearchHilight();
		  #else
			//_OnCommand_SearchHilight( str);
		  #endif
			break;
		case ID_SEARCH_PAGE:
			_OnCommand_SearchPage( (::GetKeyState(VK_SHIFT) >= 0));
			break;
		case ID_SEARCHBAR_WORD00:
		case ID_SEARCHBAR_WORD01:
		case ID_SEARCHBAR_WORD02:
		case ID_SEARCHBAR_WORD03:
		case ID_SEARCHBAR_WORD04:
		case ID_SEARCHBAR_WORD05:
		case ID_SEARCHBAR_WORD06:
		case ID_SEARCHBAR_WORD07:
		case ID_SEARCHBAR_WORD08:
		case ID_SEARCHBAR_WORD09:
		case ID_SEARCHBAR_WORD10:
		case ID_SEARCHBAR_WORD11:
		case ID_SEARCHBAR_WORD12:
		case ID_SEARCHBAR_WORD13:
		case ID_SEARCHBAR_WORD14:
		case ID_SEARCHBAR_WORD15:
		case ID_SEARCHBAR_WORD16:
		case ID_SEARCHBAR_WORD17:
		case ID_SEARCHBAR_WORD18:
		case ID_SEARCHBAR_WORD19:
			_OnCommand_SearchPage((::GetKeyState(VK_SHIFT) >= 0), nID-ID_SEARCHBAR_WORD00);
			break;

		default:
			ATLASSERT(0);
		}

		SetMsgHandled(TRUE);
		return;
	}

	SetMsgHandled(FALSE);
}




/** �n�C���C�g�{�^���������I�ɐݒ肷��
 */
bool CDonutSearchBar::ForceSetHilightBtnOn(bool sw)
{
	bool rc = (BOOL(sw) != m_bHilightSw);
	m_bHilightSw = sw;
	int hilightStat = m_bHilightSw ? TBSTATE_PRESSED : TBSTATE_ENABLED;
	m_wndToolBar.SetButtonInfo(ID_SEARCHBAR_HILIGHT, TBIF_STATE, 0, hilightStat, 0, 0, 0, 0, 0);
	return rc;
}



//+++
void CDonutSearchBar::OnSearchWeb_engineId(UINT code, int id, HWND hWnd)
{
	ATLASSERT(ID_INSERTPOINT_SEARCHENGINE <= id && id <= ID_INSERTPOINT_SEARCHENGINE_END);
	unsigned n = id - ID_INSERTPOINT_SEARCHENGINE;
	CString 	strEngine;
	MtlGetLBTextFixed(m_cmbEngine, n, strEngine);
	//\\CString str = GetSearchStr();
	CString str = Donut_GetActiveSelectedText();	//\\+ �܂��ʂŃo�O�邩��
	SearchWeb_str_engine(str, strEngine);
}



void CDonutSearchBar::SearchWeb()
{
  #if 1	//+++
	CString str = GetSearchStr();
	_OnCommand_SearchWeb( str );
  #endif
}


void	CDonutSearchBar::_OnCommand_SearchWeb(CString &str)
{
	int nTarCate = m_cmbEngine.GetCurSel();
	if (nTarCate == -1) return;

	SearchWeb_str_engine(str, GetSearchEngineStr());
}


//+++ �G���W���̑I�����ł���悤��_OnCommand_SearchWeb�̎��̂𕪗�.
void	CDonutSearchBar::SearchWeb_str_engine(CString &str, const CString&	strSearchEng)
{
  #if 1
	//x CString 	strSearchEng = MtlGetWindowText(m_cmbEngine);
	bool bUrlSearch = false;
	if (::GetFocus() != m_wndKeyword.m_hWnd) {
	   #if 0//\\+	//+++ �����Ȃ̂ŁA���ƂŁA�d�g�݂𒼂�
		// �I��͈͂�����΁A�����D�悷��.
		CString strSel = Donut_GetActiveSelectedText();
		if (strSel.IsEmpty() == 0) {
			str = strSel;
		}
	   #endif
	   #if 1 //+++ v1.48c �ŕύX.
		if (strSearchEng.IsEmpty())	//�T�[�`�G���W�������J���Ȃ猟�����Ȃ�.
			return;
		//+++ addressbar�̕�������g���H
		{
			CIniFileI		pr(GetSearchIniPath(), strSearchEng);
			DWORD			exPropOpt = pr.GetValue(_T("ExPropOpt"), 0);
			pr.Close();
			if (exPropOpt & 1) {	//+++ addressbar�̕���������Ă���ꍇ.
				if (str.IsEmpty()	//+++ ���������񂪋�̏ꍇ.
					|| (_tcsncmp(LPCTSTR(str), _T("http:"),5) != 0 && _tcsncmp(LPCTSTR(str), _T("https:"), 6) != 0)	//+++ ���������񂪂��ł�url�Ȃ炻���p����̂ŁA�����ł͏Ȃ�.
				){
					CString strUrl = GetAddressBarText();
					if (strUrl.IsEmpty() == 0) {
						str 	  = strUrl;
						bUrlSearch = true;
					}
				}
			}
		}
	  #endif
	}
  #endif

	GetHilightBtnFlag();				//+++ �n�C���C�g�{�^���̏�ԃ`�F�b�N
	toolBarAddButtons(true/*str*/);			//+++ �T�[�`�̃c�[���o�[���ɒP���ݒ�

	if ( !str.IsEmpty() ) {
	  #if 1 //+++	OnItemSelected�𕪉������̂�OpenSearch���ĂԂ悤�ɕύX.
		BOOL	bFirst		 = TRUE;
		int 	nLoopCtn	 = 0;
		OpenSearch(str, strSearchEng, nLoopCtn, bFirst);
	  #elif 1 //+++	template����߂��̂�this�ł���.
		this->OnItemSelected(str, strSearchEng);
	  #else
		T *pT = static_cast<T *>(this);
		pT->OnItemSelected(str, strSearchEng);
	  #endif
		if (bUrlSearch == false)		//+++ url�����������ꍇ�́A�����ɓ���Ȃ��Œu��.
			_AddToSearchBoxUnique(str);
	}

  #if 1	//+++ �s�v��������...�e�X�g�I��...
	//+++ �\�����c�[���o�[�������o�[�{�^����on�̎��̂�
	ShowToolBarIcon(m_bShowToolBar/*true*/);
  #endif
}



// �n�C���C�g�{�^�����������Ƃ�
void CDonutSearchBar::_OnCommand_SearchHilight(CString &str)
{
	GetHilightBtnFlag();
	m_bHilightSw ^= 1;

	checkToolBarWords();	//+++
//toolBarAddButtons(false);	//+++ ���߂�...

	str = RemoveShortcutWord(str);
	if (s_bFiltering) {
		FilterString(str);
	}

  #if 0	//+++ �����ŁA�͂����ƁA�n�C���C�g�{�^����on���u�Ԃł������off�ɖ߂�̂ŁA���b�Z�[�W�͓����Ă���
	if (str.IsEmpty()) return;
  #endif
	SendMessage(GetTopLevelParent(), WM_USER_HILIGHT, (WPARAM) LPCTSTR(str)/*str.GetBuffer(0)*/, 0);
	GetHilightBtnFlag();				//+++ �n�C���C�g�{�^���̏�ԃ`�F�b�N
}



void CDonutSearchBar::_OnCommand_SearchPage(BOOL bForward, int no /*=-1*/)
{
	GetHilightBtnFlag();				//+++ �n�C���C�g�{�^���̏�ԃ`�F�b�N
	checkToolBarWords();				//+++

	CEdit	edit	= GetEditCtrl();
  #if 1 //+++ �J�[�\���ʒu�̒P�ꂾ����I������悤�ɂ��Ă݂�.
	CString str;
	if (no >= 0) {
		str 	= _GetSelectText(edit);
		//���p���E�V���[�g�J�b�g���[�h�͏��O
		str = RemoveShortcutWord(str);
		if (s_bFiltering)
			FilterString(str);
		std::vector<CString> strs;
		strs.reserve(10);
		Misc::SeptTextToWords(strs, str);
		if (size_t(no) < strs.size())
			str = strs[no];
	} else {
		str 	= _GetSelectText_OnCursor(edit);
		//���p���E�V���[�g�J�b�g���[�h�͏��O
		str = RemoveShortcutWord(str);
	}
  #else
	CString str 	= _GetSelectText(edit);
	//���p���E�V���[�g�J�b�g���[�h�͏��O
	str = RemoveShortcutWord(str);
  #endif

	CString strExcept = _T(" \t\"\r\n�@");
	str.TrimLeft(strExcept);
	str.TrimRight(strExcept);
	if (s_bFiltering)
		FilterString(str);

	SendMessage(GetTopLevelParent(), WM_USER_FIND_KEYWORD, (WPARAM) str.GetBuffer(0), (LPARAM) bForward);
}




//public
const CString CDonutSearchBar::RemoveShortcutWord(const CString &str)
{
	if (s_bUseShortcut) {
		if (str.Find( _T("\\") ) == 0 || str.Find( _T("/") ) == 0) {
			int nPos = str.Find( _T(" ") );

			if (nPos != -1)
				return str.Mid(nPos + 1);
		}
	}

	return str;
}



///+++
int CDonutSearchBar::btnWidth() const
{
  #if 1
	int  btnW = ::GetSystemMetrics(SM_CXHTHUMB);
	if (m_bExistManifest == 0)
		btnW += 2 * 2;
	else
		btnW += 2;
	return btnW;
  #else
	return 21;
  #endif
}



///+++ �L�[���[�h���͗�����̎��ɃG���W������\�����邽�߂̐ݒ�
void	CDonutSearchBar::SetCmbKeywordEmptyStr()
{
	m_cmbKeyword.setEmptyStr(GetSearchEngineStr(), IDC_EDIT/*1001*/, (m_nEngineWidth <= 8 + btnWidth()));
	m_cmbKeyword.redrawEmptyStr();
}



// private:
LRESULT CDonutSearchBar::OnSize(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
  #ifndef USE_DIET
	//+++ �����P������ڂɂ��邽�߁A�����o�[�̃T�C�Y�������@��ύX.
	if (s_bNoWordButton == false) { //+++ �P��{�^������̂Ƃ�
		return ResizeBar(0,0);
	} else {
		//+++ �P��{�^�������̏ꍇ�͋�����.

		//if (!m_wndToolBar.m_hWnd)
		//	return S_FALSE;
		if (!m_wndKeyword.m_hWnd)
			return S_FALSE;

		CRect	rcDlg;
		GetClientRect(&rcDlg);

		CRect	rcToolbar(rcDlg);
		CString str;

		if (rcDlg.right == 0)
			return S_FALSE;

		if ( GetToolIconState() && m_wndToolBar.m_hWnd ) {
			rcToolbar.left = rcDlg.right - m_cxBtnsBtn - 10;
			m_wndToolBar.SetWindowPos(NULL, rcToolbar, SWP_NOZORDER | SWP_NOSENDCHANGING);
		} else {
			//��\��
			rcToolbar.left = rcDlg.right;
		}

		CRect	rcEngine;
		m_cmbEngine.GetWindowRect(&rcEngine);
		int 	nEngineCX = rcEngine.Width();
		int 	nEngineCY = rcEngine.Height();
		if (nEngineCX <= btnWidth())	//+++
			nEngineCX = btnWidth();
		m_nKeywordWidth	 = nEngineCX;
		rcEngine		 = rcToolbar;
		rcEngine.right	 = rcToolbar.left - s_kcxGap;
		rcEngine.left	 = rcEngine.right - nEngineCX;
		rcEngine.top	 = m_nDefEditT; //minit
		rcEngine.bottom  = rcEngine.top + nEngineCY;

		m_cmbEngine.SetWindowPos(NULL, rcEngine, SWP_NOZORDER | SWP_NOSENDCHANGING);

		CRect	rcKeyword(rcEngine);
		rcKeyword.left	= 0;
		rcKeyword.right = rcEngine.left - s_kcxGap;
		m_cmbKeyword.SetWindowPos(NULL, rcKeyword, SWP_NOZORDER | SWP_NOSENDCHANGING);
	  #if 1	//+++ �b��Ώ�... �{���I�ɏC���ł��Ă��Ȃ���on_
		m_cmbKeyword.SetEditSel(0,0);	//+++ �������͈͉���
	  #endif
		SetCmbKeywordEmptyStr();	//+++

		m_wndToolBar.InvalidateRect(NULL, TRUE);

		return S_OK;
	}
  #else	// USE_DIET���� �P��{�^���̂�
	return ResizeBar(0,0);
  #endif
}



///+++ �o�[�̃T�C�Y�ύX. �L�[���[�h�b�G���W���b�c�[���o�[  �̋��ڂł̈ړ���������difA,difB�Ƃ���.
int CDonutSearchBar::ResizeBar(int difA, int difB)
{
	if (!m_wndKeyword.m_hWnd) {
		return S_FALSE;
	}

	CRect	rcDlg;
	GetClientRect(&rcDlg);
	if (rcDlg.right == 0) {
		return S_FALSE;
	}

	//int	h = rcDlg.Height();

	CRect	rcKeyword;
	m_cmbKeyword.GetWindowRect(&rcKeyword);
	m_nKeywordWidth = rcKeyword.Width();
	int 		h	= rcKeyword.Height();
	m_nKeywordWidth += difA;
	if (m_nKeywordWidth <= btnWidth())
		m_nKeywordWidth = btnWidth();
	rcKeyword.left	= 0;
	rcKeyword.top	= m_nDefEditT;
	rcKeyword.right = m_nKeywordWidth;
	rcKeyword.bottom= rcKeyword.top + h;
	m_cmbKeyword.SetWindowPos(NULL, rcKeyword, SWP_NOZORDER | SWP_NOSENDCHANGING);


	CRect	rcEngine;
	m_cmbEngine.GetWindowRect(&rcEngine);
	h			  = rcEngine.Height();
	m_nEngineWidth = rcEngine.Width();
	m_nEngineWidth += difB;
	if (m_nEngineWidth <= btnWidth()) {
		m_nEngineWidth = btnWidth();
	}
	rcEngine.left	 = rcKeyword.right + s_kcxGap;
	rcEngine.top	 = m_nDefEditT;
	rcEngine.right	 = rcEngine.left	+ m_nEngineWidth;
	rcEngine.bottom  = rcEngine.top + h;
	m_cmbEngine.SetWindowPos(NULL, rcEngine, SWP_NOZORDER | SWP_NOSENDCHANGING);

	CRect	rcToolbar(rcDlg);
	rcToolbar.left = rcEngine.right + s_kcxGap;
	if (rcToolbar.left > rcToolbar.right)
		rcToolbar.left = rcToolbar.right;
	if ( GetToolIconState() ) {
	} else {
		//��\��
		rcToolbar.left = rcDlg.right;
	}
	if (m_wndToolBar.m_hWnd)
		m_wndToolBar.SetWindowPos(NULL, rcToolbar, SWP_NOZORDER | SWP_NOSENDCHANGING);

	m_wndToolBar.InvalidateRect(NULL, TRUE);

	return S_OK;
}


#if 1	//test
bool	CDonutSearchBar::checkEngineNameEnable()
{
	if (::IsWindow(m_hWnd) == 0)
		return false;
	if (::IsWindow(m_cmbKeyword.m_hWnd) == 0)
		return false;

	if (::GetFocus() == GetEditCtrl())
		return false;

	CRect	rcEngine;
	m_cmbEngine.GetWindowRect(&rcEngine);
	int engineW		= rcEngine.Width();
	if (engineW > btnWidth())
		return false;

	CRect	rcKeyword;
	m_cmbKeyword.GetWindowRect(&rcKeyword);
	int keywordW	= rcKeyword.Width();
	if (keywordW <= btnWidth())
		return false;

	//CPaintDC dc(m_cmbKeyword.m_hWnd);
	//dc.TextOut(rcKeyword.left, rcKeyword.top, "Test");
	//dc.TextOut(0, 0, "ABCDEF");

	return true;
}
#endif


void	CDonutSearchBar::checkToolBarWords()	//+++
{
	if (toolBarAddButtons(true)) {
	  #if 1	//+++ �s�v��������...�e�X�g�I��...
		ShowToolBarIcon(m_bShowToolBar/*true*/);	//+++ true���ƃ{�^�������Ă�Ƃ��ɕs����
	  #endif
	}
}



//+++ �������[�h�̕�����𕪉����āA�����c�[���o�[�ɒǉ�
bool CDonutSearchBar::toolBarAddButtons(bool chk)
{
	if (s_bNoWordButton && m_dwTinyWordButton == 0)	//�P��{�^����\�����Ȃ��ꍇ�͒��A.
		return 0;

	CEdit	edit	= GetEditCtrl();

	CString str		= _GetSelectText(edit);
	//���p���E�V���[�g�J�b�g���[�h�͏��O
	str = RemoveShortcutWord(str);
	if (s_bFiltering) {
		FilterString(str);
	}

	if (chk && m_toolBarAddWordStr == str) {
		return ForceSetHilightBtnOn(m_bHilightSw != 0);	//+++ �n�C���C�g�{�^����on/off�͕ς���Ă���\��������̂ōX�V.
	}
	m_toolBarAddWordStr = str;

	std::vector<CString> strs;
	strs.reserve(20);
	Misc::SeptTextToWords(strs, str);

	if (m_wndToolBar.m_hWnd == 0) return 0;

	//+++ �����c�[���o�[�̒��g����U���ׂč폜
	unsigned num = m_wndToolBar.GetButtonCount();
	for (int i = num; --i >= 0;) {
		m_wndToolBar.DeleteButton(i);
	}

	addDefaultToolBarIcon((UINT)strs.size());		//+++ �f�t�H���g�̃A�C�R����ǉ�

	if (s_bNoWordButton == false) {			//+++ �����{�^���̎��̓��[�h�{�^����o�^�����Ȃ�.
		toolBarAddWordTbl(strs);
	}

	return true;
}



//+++  �f�t�H���g�̃A�C�R����ǉ�
int  CDonutSearchBar::addDefaultToolBarIcon(unsigned n)
{
	int hilightStat = m_bHilightSw ? TBSTATE_PRESSED : TBSTATE_ENABLED;
	if (n == 0 || s_bNoWordButton || s_bUsePageButton) {	//+++ �����̌��� (�P�Ƃ̃y�[�W�������{�^������)
		if (s_bNoWordButton && m_dwTinyWordButton) {
			return addDefaultToolBarIcon_tinyWordButton(n);
		}
		//static
		static const TBBUTTON	btns[] = {
			{ 0 , ID_SEARCH_WEB,		TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE	, 0, 0 },// Web
			{ 2 , ID_SEARCH_PAGE,		TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE	, 0, 0 },// Page
			{ 1 , ID_SEARCHBAR_HILIGHT, hilightStat    , TBSTYLE_CHECK	| TBSTYLE_AUTOSIZE	, 0, 0 },// Hilight
		};
		//m_nHilightBtnNo = 2;
	  #if 0	//+++ �{���͂������ł���
		MTLVERIFY( m_wndToolBar.AddButtons(3, (TBBUTTON *) btns ) );
	  #else	//+++ �e�X�g
		for (unsigned i = 0; i < 3; ++i)
			MTLVERIFY( m_wndToolBar.AddButton( (TBBUTTON *) &btns[i] ) );
	  #endif
		m_wndToolBar.SetBitmapSize(14,14);
		if (s_bNoWordButton) {
			m_cxBtnsBtn 	 = (20) * 3 + 1;
		}
		return 3;
	} else {				//+++ �P���p�{�^���L��
		//static
		static const TBBUTTON	btns[] = {
			{ 0 , ID_SEARCH_WEB,		TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE	, 0, 0 },// Web
			{ 1 , ID_SEARCHBAR_HILIGHT, hilightStat    , TBSTYLE_CHECK	| TBSTYLE_AUTOSIZE	, 0, 0 },// Hilight
		};
		//m_nHilightBtnNo = 1;
	  #if 0	//+++ �{���͂������ł���
		MTLVERIFY( m_wndToolBar.AddButtons(2, (TBBUTTON *) btns ) );
	  #else	//+++ �e�X�g
		for (unsigned i = 0; i < 2; ++i)
			MTLVERIFY( m_wndToolBar.AddButton( (TBBUTTON *) &btns[i] ) );
	  #endif
		return 2;
	}
}



#ifndef USE_DIET
//+++ �P��{�^���̑����1�`5�̐����{�^����ݒ�.
//		���P��{�^�������������܂ł̎���/��p��������������.
//		  �P��{�^�����g�p���Ȃ��ݒ�ŁA���Anogui��"NumberButton=1"��ݒ肵���Ƃ��̂ݗ��p�\.
int	CDonutSearchBar::addDefaultToolBarIcon_tinyWordButton(unsigned n)
{
	if (n > 5)
		n = 5;
	int hilightStat = m_bHilightSw ? TBSTATE_PRESSED : TBSTATE_ENABLED;
	//static
	static const TBBUTTON	btns[3] = {
		{ 0 		, ID_SEARCH_WEB,		TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE	, 0, 0 },	// Web
		{ 2 		, ID_SEARCH_PAGE,		TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE	, 0, 0 },	// Page
		{ 1 		, ID_SEARCHBAR_HILIGHT, hilightStat	   , TBSTYLE_CHECK	| TBSTYLE_AUTOSIZE	, 0, 0 },	// Hilight
	};
	//m_nHilightBtnNo = 2;
  #if 0	//+++ �{���͂������ł���
	MTLVERIFY( m_wndToolBar.AddButtons(3, (TBBUTTON *) btns ) );
  #else	//+++ �e�X�g
	for (unsigned i = 0; i < 3; ++i)
		MTLVERIFY( m_wndToolBar.AddButton( (TBBUTTON *) &btns[i] ) );
  #endif
	m_cxBtnsBtn	 = (20) * (3+5) + 1;

	enum { STYLE = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE }; // | TBSTYLE_LIST | TBSTYLE_TRANSPARENT };
	for (unsigned i = 0; i < 5; ++i) {
		const TBBUTTON	btn = { I_IMAGENONE, ID_SEARCHBAR_WORD00+i, (i < n) ? TBSTATE_ENABLED : 0/*TBSTATE_CHECKED*/, STYLE, 0, 0 };
		MTLVERIFY( m_wndToolBar.AddButton( (TBBUTTON *)&btn) );

		CVersional<TBBUTTONINFO> bi;
		bi.cbSize	= sizeof(TBBUTTONINFO);
		bi.dwMask	= TBIF_TEXT	/* | TBIF_STYLE*/;
		bi.fsStyle |= TBSTYLE_AUTOSIZE /*| TBBS_NOPREFIX*/ ;
		TCHAR str[4];
		str[0]		= _T('1'+i);
		str[1]		= _T('\0');
		bi.pszText	= str;
		MTLVERIFY( m_wndToolBar.SetButtonInfo(ID_SEARCHBAR_WORD00+i, &bi) );
	}

	// �T�C�Y�����Ȃ�
	m_wndToolBar.SetMaxTextRows(1);
	CRect rcButton;
	m_wndToolBar.GetItemRect(3, rcButton);
	m_wndToolBar.SetButtonSize(rcButton.Size());
	m_wndToolBar.SetButtonSize(rcButton.Size());
	m_wndToolBar.Invalidate();

	m_wndToolBar.AutoSize();
	return 3;
}
#endif



//+++ vector�œn���ꂽ�����̕�����(20�܂�)�������c�[���o�[�ɓo�^.
void CDonutSearchBar::toolBarAddWordTbl(std::vector<CString>& tbl)
{
	TBBUTTONINFO	bi = { sizeof(TBBUTTONINFO) };
	bi.dwMask	= TBIF_TEXT/* | TBIF_STYLE*/;
	bi.fsStyle |= TBSTYLE_AUTOSIZE | TBSTYLE_NOPREFIX;

	unsigned n	= unsigned(tbl.size());
	if (n > 20) n = 20;
	for (unsigned i = 0; i < n; ++i) {
		enum { STYLE = TBSTYLE_BUTTON | TBSTYLE_AUTOSIZE | TBSTYLE_NOPREFIX };
		// | TBSTYLE_LIST | TBSTYLE_TRANSPARENT };
		TBBUTTON	btn = { 2/*I_IMAGENONE*/, ID_SEARCHBAR_WORD00 +i, TBSTATE_ENABLED, STYLE, 0, 0 };
		MTLVERIFY( m_wndToolBar.AddButton( (TBBUTTON *)&btn) );

		bi.pszText = (LPTSTR) LPCTSTR(tbl[i]);
		MTLVERIFY( m_wndToolBar.SetButtonInfo(ID_SEARCHBAR_WORD00+i, &bi) );

		//m_wndToolBar.AutoSize();
		//CRect rcButton;
		//m_wndToolBar.GetItemRect(TOP+i, rcButton);
		//m_wndToolBar.SetButtonSize(rcButton.Size());
	}

	m_wndToolBar.SetButtonSize(m_ButtonSize);
//	m_wndToolBar.AutoSize();
	m_wndToolBar.Invalidate();
	//ShowToolBarIcon(true);
}



CString 	CDonutSearchBar::GetSkinSeachBarPath(int mode/*BOOL bHot*/)
{
	ATLASSERT(mode >= 0 && mode <= 2);
	static const TCHAR* tbl[] = {
		_T("SearchBar.bmp"),
		_T("SearchBarHot.bmp"),
		_T("SearchBarDis.bmp"),
	};
	return _GetSkinDir() + tbl[ mode ];
}


int		CDonutSearchBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	m_cmbKeyword.Create(m_hWnd);
	m_cmbKeyword.SetDlgCtrlID(IDC_CMB_KEYWORD);
	m_cmbEngine.Create(m_hWnd);
	m_cmbEngine.SetDlgCtrlID(IDC_CMB_ENGIN);

//	CRect		rcDlg;
//	GetWindowRect(&rcDlg);
//	SetWindowPos(NULL, 0, 0, 0, m_nDefDlgH , 0);

	// �ݒ��ǂݍ���
	CIniFileI	pr( g_szIniFileName, _T("SEARCH") );
	m_nKeywordWidth 	 = pr.GetValue( _T("KeywordWidth" ), 150 );
	m_nEngineWidth		 = pr.GetValue( _T("EngWidth"	  ), 150 );
	pr.Close();

	//�R���{�{�b�N�X������
//	m_cmbKeyword.FlatComboBox_Install( cmbkeyword/*GetDlgItem(IDC_CMB_KEYWORD)*/ );
//	m_cmbEngine.FlatComboBox_Install  ( GetDlgItem(IDC_CMB_ENGIN  ) );
//	m_cmbEngine.SetDroppedWidth(150);
	if (s_bSearchListWidth) {
		m_cmbEngine.SetDroppedWidth(s_nSearchListWidth);
	}

	_InitCombo();

	// ����ݒ�
	if (m_nKeywordWidth != 0) {
		CRect	rcKeyword;
		m_cmbKeyword.GetWindowRect(&rcKeyword);
		int 	h		 = rcKeyword.Height();
		rcKeyword.left	 = 0;
		rcKeyword.right  = m_nKeywordWidth;
		rcKeyword.top	 = 0;
		rcKeyword.bottom = h;
		m_cmbKeyword.SetWindowPos(NULL, rcKeyword, SWP_NOZORDER | SWP_NOSENDCHANGING);
	}
	if (m_nEngineWidth != 0) {
		CRect	rcEngine;
		m_cmbEngine.GetWindowRect(&rcEngine);
		int 	h		 = rcEngine.Height();
		rcEngine.left	= 0;
		rcEngine.right	= m_nEngineWidth;
		rcEngine.top	= 0;
		rcEngine.bottom = h;
		m_cmbEngine.SetWindowPos(NULL, rcEngine, SWP_NOZORDER | SWP_NOSENDCHANGING);
	}

	m_wndKeywordCombo.SubclassWindow(m_cmbKeyword.m_hWnd); //minit
	m_wndKeyword.SubclassWindow( m_cmbKeyword.GetDlgItem(IDC_EDIT/*1001*/) );
	m_wndEngine.SubclassWindow(m_cmbEngine.m_hWnd);


	//�c�[���o�[������
	{
		DWORD	dwShowToolBar = STB_SHOWTOOLBARICON;
		CIniFileI		pr( g_szIniFileName, _T("SEARCH") );
		pr.QueryValue( dwShowToolBar, _T("Show_ToolBarIcon") );
		pr.Close();
		m_bShowToolBar	= (dwShowToolBar & STB_SHOWTOOLBARICON) == STB_SHOWTOOLBARICON;
	}

  #if 1 //+++ �������S��������(�������[�̖��ʂ������S&���h���D��)
	_InitToolBar(m_nBmpCX, m_nBmpCY, RGB(255, 0, 255));
	ShowToolBarIcon(m_bShowToolBar);
  #else //+++ ���̏����B�\�����Ȃ���Ⴡ�����[�������B�����A�r���Őݒ肵���ꍇfont���f�Ȃ�.
	if (m_bShowToolBar) {
		_InitToolBar( m_nBmpCX, m_nBmpCY, RGB(255, 0, 255) );
	}
  #endif

	//�h���b�O�h���b�v������
	RegisterDragDrop();

	//�X���b�h�𗘗p���ăR���{�{�b�N�X�Ƀf�[�^��o�^(INI����̓ǂݍ��݂Ɏ��Ԃ������邽��)
	// Thread Create
	m_tdInitComboBox	= boost::thread(boost::bind(&CDonutSearchBar::_SearchThread, this));

	// SetCmbKeywordEmptyStr();	//+++ �ǂ����܂��G���W�������o�^����Ă��Ȃ��̂ŁA���̃^�C�~���O�͂Ȃ�

	return 0;
}

void CDonutSearchBar::OnDestroy()
{
	// Thread Remove
	m_tdInitComboBox.join();

  #if 0
	CRect			rcKeyword;
	m_wndKeyword.GetWindowRect(rcKeyword);
	DWORD			keywordW = rcKeyword.Width();
	CRect			rcEngine;
	m_wndEngine.GetWindowRect(rcEngine);
	DWORD			enginW	 = rcEngine.Width();
  #else

	if (s_bNoWordButton) { //+++ �������̂Ƃ��̒��날�킹
		CRect			rcEngine;
		m_wndEngine.GetWindowRect(rcEngine);
		m_nEngineWidth = rcEngine.Width();
	}
  #endif

	{
		CIniFileIO	pr( g_szIniFileName, _T("SEARCH") );
		pr.SetValue( (DWORD) m_nKeywordWidth, _T("KeywordWidth") );
		pr.SetValue( (DWORD) m_nEngineWidth  , _T("EngWidth") );

		DWORD	dwShowToolBar = m_bShowToolBar ? STB_SHOWTOOLBARICON : 0;
		pr.SetValue( dwShowToolBar, _T("Show_ToolBarIcon") );

		if (s_bLastSel) {
			pr.SetValue( m_cmbEngine.GetCurSel(), _T("SelIndex") );
		}
	}

	m_wndKeyword.UnsubclassWindow();
	m_wndEngine.UnsubclassWindow();
	m_wndKeywordCombo.UnsubclassWindow();

	RevokeDragDrop();

	SaveHistory();
}



//�X���b�h�𗘗p���ăR���{�{�b�N�X�Ƀf�[�^��o�^(INI����̓ǂݍ��݂Ɏ��Ԃ������邽��)
void	CDonutSearchBar::_SearchThread()
{
	_InitialEngine( m_cmbEngine.m_hWnd	);
	_InitialKeyword( m_cmbKeyword.m_hWnd );
}


//private:


void CDonutSearchBar::OnEngineKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_RETURN) {
		_OnEnterKeyDown(ENTER_KEYDOWN_RETURN);
		SetMsgHandled(TRUE);
	} else if (nChar == VK_TAB) {
		m_cmbKeyword.SetFocus();
		SetMsgHandled(TRUE);
	} else {
		SetMsgHandled(FALSE);
	}
}



void CDonutSearchBar::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_RETURN) {
		SetMsgHandled(TRUE);
	} else {
		SetMsgHandled(FALSE);
	}
	//checkToolBarWords();	//+++
}

void CDonutSearchBar::OnKeywordKillFocus(CWindow wndFocus)
{
	SetMsgHandled(FALSE);
	/* �c�[���o�[���X�V���� */
	checkToolBarWords();
}


LRESULT CDonutSearchBar::OnCtlColorListBox(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	bHandled		  = FALSE;
	m_hWndKeywordList = (HWND) lParam;
	return 0;
}



void CDonutSearchBar::OnKeywordKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// if not dropped, eat VK_DOWN
	if ( !m_cmbKeyword.GetDroppedState() && (nChar == VK_DOWN || nChar == VK_UP) ) {
		int nIndexEngine  = m_cmbEngine.GetCurSel();
		int nIndexKeyword = m_cmbKeyword.GetCurSel();

		SetMsgHandled(TRUE);

		if (nChar == VK_UP) {
			if (::GetKeyState(VK_CONTROL) < 0) {
				if (0 > nIndexEngine - 1)
					m_cmbEngine.SetCurSel(m_cmbEngine.GetCount() - 1);
				else
					m_cmbEngine.SetCurSel(nIndexEngine - 1);
				//SetCmbKeywordEmptyStr();	//+++
			} else {
				if (0 > nIndexKeyword - 1)
					m_cmbKeyword.SetCurSel(m_cmbKeyword.GetCount() - 1);
				else
					m_cmbKeyword.SetCurSel(nIndexKeyword - 1);
			}

		} else if (nChar == VK_DOWN) {
			if (::GetKeyState(VK_CONTROL) < 0) {
				int nIndex = m_cmbEngine.GetCurSel();

				if (m_cmbEngine.GetCount() > nIndexEngine + 1)
					m_cmbEngine.SetCurSel(nIndexEngine + 1);
				else
					m_cmbEngine.SetCurSel(0);
				//SetCmbKeywordEmptyStr();	//+++
			} else {
				if (m_cmbKeyword.GetCount() > nIndexKeyword + 1)
					m_cmbKeyword.SetCurSel(nIndexKeyword + 1);
				else
					m_cmbKeyword.SetCurSel(0);
			}
		}
	} else {
		if (nChar == VK_RETURN) {
			_OnEnterKeyDown(ENTER_KEYDOWN_RETURN);
			SetMsgHandled(TRUE);
		} else if (nChar == VK_DELETE) {
			if ( m_cmbKeyword.GetDroppedState() ) {
				if ( DeleteKeywordHistory() )
					SetMsgHandled(TRUE);
			} else
				SetMsgHandled(FALSE);
		} else if (nChar == VK_TAB) {
			m_cmbEngine.SetFocus();
			SetMsgHandled(TRUE);
		} else {
			SetMsgHandled(FALSE);
		}
	}

	//SetCmbKeywordEmptyStr();	//+++	PreTranse�Ŗ��x�`�F�b�N���邩�炱���͖�����.
}



LRESULT CDonutSearchBar::OnKeywordCbnDropDown(LPNMHDR pnmh)
{
	if (m_cmbKeyword.GetCount() == 0)
		::MessageBox(NULL, _T(""), _T(""), MB_OK);

	return FALSE;
}



BOOL CDonutSearchBar::DeleteKeywordHistory()
{
	if ( !::IsWindow(m_hWndKeywordList) )
		return FALSE;

	CListBox List	= m_hWndKeywordList;
	int 	 nIndex = List.GetCurSel();

	if (nIndex == LB_ERR)
		return FALSE;

	m_cmbKeyword.DeleteString(nIndex);
	return TRUE;
}



void CDonutSearchBar::OnEngineRButtonUp(UINT nFlags, CPoint point)
{
	CMenu menu;
	menu.CreatePopupMenu();
	menu.AppendMenu( 0, 1, _T("�g���v���p�e�B") );
	menu.AppendMenu(MF_SEPARATOR);
	int			nIndex = 1;
	auto funcMakeSearchFileListMenu = [&nIndex, &menu](CString strFile) {
		nIndex++;
		menu.AppendMenu(0, nIndex, MtlGetFileName(strFile));
	};
	MtlForEachFileSortEx( Misc::GetExeDirectory() + _T("Search\\"), funcMakeSearchFileListMenu, _T("*.ini") );

	CPoint	pos;

	//GetCursorPos(&pos);
	ATLTRACE("mouseposition1 : left=%4d top=%4d", point.x, point.y);
	::ClientToScreen(m_cmbEngine.m_hWnd, &point);
	ATLTRACE("mouseposition2 : left=%4d top=%4d", point.x, point.y);

	int 	nRet	 = menu.TrackPopupMenu( TPM_LEFTALIGN | TPM_TOPALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD,
							point.x,
							point.y,
							m_cmbEngine.m_hWnd);
	if (nRet == 0)
		return;

	if (nRet == 1) {	// �g���v���p�e�B
		if (!m_cmbEngine.IsWindow() || m_cmbEngine.GetCurSel() == CB_ERR)
			return;
		CString 		  strText;
		m_cmbEngine.GetLBText(m_cmbEngine.GetCurSel(), strText);
		CExPropertyDialog dlg(GetSearchIniPath(), strText, 0);
		dlg.SetTitle(strText);
		dlg.DoModal();	// �����G���W���̐ݒ�_�C�A���O��\������
		return;
	}

	CString 		strTitle;
	menu.GetMenuString(nRet, strTitle, MF_BYCOMMAND);
	CString 		strPath = Misc::GetExeDirectory() + _T("Search\\") + strTitle;

	{
		CIniFileO	pr( g_szIniFileName, _T("Search") );
		pr.SetStringUW( strPath, _T("Path") );
	}

	RefreshEngine();
}



void	CDonutSearchBar::OnEngineSetFocus(HWND hWndBefore)
{
	SetMsgHandled(FALSE);
	::WINNLSEnableIME(m_cmbEngine, FALSE);

	SetCmbKeywordEmptyStr();	//+++
}



void	CDonutSearchBar::OnEngineKillFocus(HWND hWndNew)
{
	SetMsgHandled(FALSE);
	::WINNLSEnableIME(m_cmbEngine, TRUE);

	SetCmbKeywordEmptyStr();	//+++
}



void	CDonutSearchBar::OnMouseMove(UINT nFlags, const CPoint& pt)
{
	if (s_bNoWordButton) {
		//+++ �P��{�^�������̏ꍇ�͋�����.
		CRect	rcKeyword;

		m_cmbKeyword.GetWindowRect(&rcKeyword);
		ScreenToClient(&rcKeyword);

		if ( abs(rcKeyword.right - pt.x) > 5 && ::GetCapture() != m_hWnd )
			return;

		if (m_hCursor == NULL)
			m_hCursor = ::LoadCursor(NULL, IDC_SIZEWE);

		::SetCursor(m_hCursor);

		if ( (nFlags & MK_LBUTTON) ) {
			UpdateLayout(pt);
		}
	} else {	//+++ �P��{�^������̏ꍇ
		CRect	rcKeyword;
		m_cmbKeyword.GetWindowRect(&rcKeyword);
		ScreenToClient(&rcKeyword);
		if ((abs(rcKeyword.right - pt.x) <= 6 || (m_has == 1 && ::GetCapture() == m_hWnd))
			&& (rcKeyword.top <= pt.y && pt.y < rcKeyword.bottom) ) 
		{
			m_has = 1;
			if (m_hCursor == NULL)
				m_hCursor = ::LoadCursor(NULL, IDC_SIZEWE);
			::SetCursor(m_hCursor);
			if ( (nFlags & MK_LBUTTON) ) {
				UpdateLayout(pt);
			}
			return;
		}

		CRect	rcEngine;
		m_cmbEngine.GetWindowRect(&rcEngine);
		ScreenToClient(&rcEngine);
		if ( (abs(rcEngine.right - pt.x) <= 6 || (m_has == 2 && ::GetCapture() == m_hWnd))
			&& (rcEngine.top <= pt.y && pt.y < rcEngine.bottom) ) 
		{
			m_has = 2;
			if (m_hCursor == NULL)
				m_hCursor = ::LoadCursor(NULL, IDC_SIZEWE);
			::SetCursor(m_hCursor);
			if ( (nFlags & MK_LBUTTON) ) {
				UpdateLayout2(pt);
			}
			return;
		}
		m_has = 0;
		return;
	}
}


#if 0
///+++ �G���W���I���̉�������... ���son_.
void CDonutSearchBar::OnSelDblClkForEngine(UINT code, int id, HWND hWnd)
{
	int 	nTarCate	 = m_cmbEngine.GetCurSel();
	if (nTarCate == -1)
		return;

	TCHAR	buf[4096];
	m_cmbEngine.GetLBText(nTarCate, buf);
	CString strSearchEng = buf;
	BOOL	bFirst		 = TRUE;
	int 	nLoopCtn	 = 0;

	CString  str;
	int nIndexCmb = m_cmbKeyword.GetCurSel();
	if (nIndexCmb == -1)
		str = MtlGetWindowText(m_cmbKeyword);
	else
		MtlGetLBTextFixed(m_cmbKeyword.m_hWnd, nIndexCmb, str);
	if ( !str.IsEmpty() )
		OpenSearch(str, strSearchEng, nLoopCtn, bFirst);
	return;
}
#endif



///+++ �G���W���I���̉�������... ���son_...�����ǁA������Ƃ����L�p.
void CDonutSearchBar::OnSelChangeForEngine(UINT code, int id, HWND hWnd)
{
	if (::GetKeyState(VK_RBUTTON) < 0) {	// �����G���W���̃v���p�e�B���J��
		CString 			strText;
		m_cmbEngine.GetLBText(m_cmbEngine.GetCurSel(), strText);
		CExPropertyDialog	dlg(GetSearchIniPath(), strText, 0);
		dlg.SetTitle(strText);
		dlg.DoModal();
		SetCmbKeywordEmptyStr();	//+++
		return;
	}

	bool bSts = false;
	if (id == IDC_CMB_ENGIN) {
		bSts = s_bEngChgGo;
	}

	if (::GetKeyState(VK_SHIFT) < 0) bSts = !bSts;

	if (bSts) {
		//+++ �G���W�����؂�ւ�����ꍇ�́A����������ł�url�����̏ꍇ������̂ŁA������
		//x _OnEnterKeyDown(ENTER_KEYDOWN_SELCHANGE);
		SearchWeb();
	}
}



void CDonutSearchBar::OnSelChange(UINT code, int id, HWND hWnd)
{
	bool bSts = false;
	//x if	(id == IDC_CMB_ENGIN)	 bSts = dwStatus & STS_ENG_CHANGE_GO; else		//+++ �G���W���͕ʊ֐���.
	if (id == IDC_CMB_KEYWORD)		 bSts = s_bKeyChgGo;
	if (::GetKeyState(VK_SHIFT) < 0) bSts = !bSts;

	if (bSts) {
		_OnEnterKeyDown(ENTER_KEYDOWN_SELCHANGE);
	}
}



void CDonutSearchBar::OnLButtonDown(UINT nFlags, const CPoint& pt)
{
	if (s_bNoWordButton) {
		//+++ �P��{�^���Ȃ��̏ꍇ�͋�����
		CRect	rcKeyword;
		m_cmbKeyword.GetWindowRect(&rcKeyword);
		ScreenToClient(&rcKeyword);

		if (abs(rcKeyword.right - pt.x) > 5)
			return;

		SetCapture();
		::SetCursor(m_hCursor);
		m_ptDragStart = pt;
		m_ptDragHist  = pt;
	} else {	//+++ �P��{�^������̏ꍇ
		CRect	rcKeyword;
		m_cmbKeyword.GetWindowRect(&rcKeyword);
		ScreenToClient(&rcKeyword);
		if (abs(rcKeyword.right - pt.x) <= 5) {
			m_has = 1;
			SetCapture();
			::SetCursor(m_hCursor);
			m_ptDragStart = pt;
			m_ptDragHist  = pt;
			return;
		}
		CRect	rcEngine;
		m_cmbEngine.GetWindowRect(&rcEngine);
		ScreenToClient(&rcEngine);
		if (abs(rcEngine.right - pt.x) <= 5) {
			m_has = 2;
			SetCapture();
			::SetCursor(m_hCursor);
			m_ptDragStart = pt;
			m_ptDragHist  = pt;
			return;
		}
		m_has = 0;
	}
}



void CDonutSearchBar::OnLButtonUp(UINT nFlags, const CPoint& pt)
{
	if (::GetCapture() != m_hWnd)
		return;

	::ReleaseCapture();


	if (s_bNoWordButton) {	//+++ �P��{�^���Ȃ��̂Ƃ�
		UpdateLayout(pt);
	} else {				//+++ �P��{�^������̏ꍇ
		if (m_has == 1)
			UpdateLayout(pt);
		else if (m_has == 2)
			UpdateLayout2(pt);
	}
}



void CDonutSearchBar::UpdateLayout(const CPoint& pt)
{
	if (s_bNoWordButton) { // �P��{�^�������̏ꍇ
		int		btnW   = btnWidth();		//+++
		int 	nMoveX = m_ptDragStart.x - pt.x;
		CRect	rcKeyword;

		m_cmbKeyword.GetWindowRect(&rcKeyword);
		ScreenToClient(&rcKeyword);
		rcKeyword.right -= nMoveX;
	  #if 1	//+++
		if (rcKeyword.right < rcKeyword.left+btnW) {
			nMoveX			= rcKeyword.right - rcKeyword.left+btnW;
			rcKeyword.right = rcKeyword.left  + btnW;
		}
	  #endif

		CRect rcEngine;
		m_cmbEngine.GetWindowRect(&rcEngine);
		ScreenToClient(&rcEngine);
		rcEngine.left	-= nMoveX;

	  #if 1	//+++
		if (rcEngine.left > rcEngine.right - btnW) {
			rcEngine.left	= rcEngine.right - btnW;
			rcKeyword.right = rcEngine.left-2;
		}
		m_nEngineWidth = rcEngine.right - rcEngine.left;
	  #endif

		if (rcEngine.left >= rcEngine.right)
			return;

		if (rcKeyword.left >= rcKeyword.right)
			return;

		m_cmbKeyword.SetWindowPos(NULL, rcKeyword, SWP_NOZORDER);
		m_cmbEngine.SetWindowPos(NULL, rcEngine, SWP_NOZORDER);

		m_ptDragStart	 = pt;
		UpdateWindow();
	} else {
		//+++ �P��{�^������̏ꍇ
		ResizeBar(pt.x - m_ptDragStart.x, 0);
		m_ptDragStart	 = pt;
		UpdateWindow();
	}
}



void CDonutSearchBar::UpdateLayout2(const CPoint& pt)
{
	ResizeBar(0, pt.x - m_ptDragStart.x);
	m_ptDragStart	 = pt;
	UpdateWindow();
}


//public: //+++ ���Ƃ�private�ɖ߂����Ƃ肠�����e�X�g.
void CDonutSearchBar::_AddToSearchBoxUnique(const CString &strURL)
{
	// search the same string
	int nCount = m_cmbKeyword.GetCount();
	for (int n = 0; n < nCount; ++n) {
		CString 	str;
		m_cmbKeyword.GetLBText(n, str);
		if (strURL == str) {
			m_cmbKeyword.DeleteString(n);
			break;
		}
	}

	m_cmbKeyword.InsertString(0, strURL);
	m_cmbKeyword.SetCurSel(0);
}


//private:
void CDonutSearchBar::_OnEnterKeyDown(int flag)
{
	CString  str;

	int nIndexCmb = m_cmbKeyword.GetCurSel();
	if (nIndexCmb == -1) {
		str = MtlGetWindowText(m_cmbKeyword);
	} else {
		MtlGetLBTextFixed(m_cmbKeyword.m_hWnd, nIndexCmb, str);
	}

	//	m_cmbKeyword.GetLBText(nIndexCmb, str);

	GetHilightBtnFlag();				//+++ �n�C���C�g�{�^���̏�ԃ`�F�b�N
	checkToolBarWords();				//+++

	if ( !str.IsEmpty() ) {
		SHORT sShift = ::GetKeyState(VK_SHIFT);
		SHORT sCtrl  = ::GetKeyState(VK_CONTROL);

		if (sShift >= 0 && sCtrl >= 0) {
			_OnCommand_SearchWeb(str);
		} else {
		  #if 1 //+++ �J�[�\���ʒu�̒P�ꂾ����I������悤�ɂ��Ă݂�.
			str = _GetSelectText_OnCursor( GetEditCtrl() );
		  #else
			str = _GetSelectText( GetEditCtrl() );
		  #endif

			if (sCtrl < 0)
				SendMessage(GetTopLevelParent(), WM_USER_FIND_KEYWORD, (WPARAM) str.GetBuffer(0), TRUE );
			else if (sShift < 0)
				SendMessage(GetTopLevelParent(), WM_USER_FIND_KEYWORD, (WPARAM) str.GetBuffer(0), FALSE);
		}
	} else {
		m_cmbEngine.ShowDropDown(FALSE); //minit
	}
}



LRESULT CDonutSearchBar::OnCtlColor(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	bHandled = FALSE;

	if (g_bNoReposition == true)
		return 0;

	CRect	rcDesktop;
	::GetWindowRect(::GetDesktopWindow(), &rcDesktop);

	CRect	rcWnd;
	::GetWindowRect( (HWND) lParam, &rcWnd );

	if (rcDesktop.right <= rcWnd.right) {
		int    nDiff = rcWnd.right - rcDesktop.right;
		rcWnd.left	-= nDiff;
		rcWnd.right -= nDiff;
		::SetWindowPos( (HWND) lParam, NULL, rcWnd.left, rcWnd.top, 0, 0, SWP_NOSIZE );
	}

	return 0;
}



#if 0
void CDonutSearchBar::OnItemSelected(const CString& str, const CString& strSearchEng)
{
	int 	nTarCate	 = m_cmbEngine.GetCurSel();
	if (nTarCate == -1)
		return;

	//x CString strSearchEng = MtlGetWindowText(m_cmbEngine);

	BOOL	bFirst		 = TRUE;
	int 	nLoopCtn	 = 0;
	OpenSearch(str, strSearchEng, nLoopCtn, bFirst);
}
#endif

//public:
//+++ ".url"���ɋL�q�����g�����ł̌����ɑΉ����邽�߁A�C��.
void CDonutSearchBar::OpenSearch(CString strWord, CString strSearchEng, int &nLoopCnt, BOOL &bFirst)
{
	if (s_bUseShortcut) {	// �V���[�g�J�b�g�R�[�h���g��
		_ShortcutSearch(strWord, strSearchEng);
	}

	if ( strSearchEng.IsEmpty() ) {	// ��ԏ�̃G���W���Ō�������
		m_cmbEngine.GetLBText(0, strSearchEng);
		if (strSearchEng.IsEmpty())
			return;
	}

	CString 	strSearchPath = GetSearchIniPath();

  #if 1	//+++ 	search.ini���Ȃ��ꍇ�́AGoogle�œK���ɃT�[�`����.
	if (::PathFileExists(strSearchPath) == FALSE && strSearchEng == ENGINENAME_FOR_NO_SEARCH_INI) {
		if (s_bFiltering)
			FilterString(strWord);		// �S�p�X�y�[�X�̒u��
		_EncodeString(strWord, ENCODE_UTF8);
		DonutOpenFile(m_hWnd, _T("http://www.google.co.jp/search?num=100&q=") + strWord, 0);
		return;
	}
  #endif

	CIniFileI		pr(strSearchPath, strSearchEng);
	if (pr.GetValue(_T("Group"), 0 ) != 0) {	// �O���[�v����
		pr.Close();
		OpenSearchGroup(strWord, strSearchEng, nLoopCnt, bFirst);
	} else {
		_EXPROP_ARGS		args;
		CString 			strOpenURL;
		std::vector<char>	vecPostData;
		if (pr.GetValue(_T("UsePost"))) {
			// ����URL�̓ǂݍ���
			strOpenURL	 = pr.GetString( _T("FrontURL") );

			// �����t���L�[���[�h�̓ǂݍ���
			CString 	 strFrontKeyWord = pr.GetString( _T("FrontKeyWord") );
			CString 	 strBackKeyWord  = pr.GetString( _T("BackKeyWord") );

			// ������̍쐬
			CString strSearchWord = strFrontKeyWord + strWord + strBackKeyWord;

			DWORD	dwEncode = pr.GetValue(_T("Encode"), 0);	// �G���R�[�h
			switch (dwEncode) {
			case ENCODE_SHIFT_JIS:
				vecPostData	= Misc::tcs_to_sjis(strSearchWord);
				break;
			case ENCODE_EUC:
				vecPostData	= Misc::tcs_to_eucjp(strSearchWord);
				break;
			case ENCODE_UTF8:
				vecPostData	= Misc::tcs_to_utf8(strSearchWord);
				break;
			default:
				vecPostData	= Misc::tcs_to_sjis(strSearchWord);
				break;
			}
			m_PostData.pPostData	= &vecPostData[0];
			m_PostData.nPostBytes	= (int)vecPostData.size() - 1;
		} else {
			if (GetOpenURLstr(strOpenURL, strWord, pr, CString()) == false)
				return;	// ����URL�̍쐬�Ɏ��s
		}
		DWORD	dwOpenFlags = 0;
		if (bFirst) {
			dwOpenFlags |= D_OPENFILE_ACTIVATE;
			bFirst		 = FALSE;
		}
		if (s_bActiveWindow) 
			dwOpenFlags |= D_OPENFILE_NOCREATE;

		//DonutOpenFile(m_hWnd, strOpenURL, dwOpenFlags);
		
		args.strUrl 	   = strOpenURL;
		args.dwOpenFlag    = dwOpenFlags;
		args.strIniFile    = strSearchPath;
		args.strSection    = strSearchEng;
		args.strSearchWord = RemoveShortcutWord( strWord/*GetSearchStr()*/ );

		SetSearchStr(strWord);
		// ���C���t���[���ɓ�����
		::SendMessage(GetTopLevelParent(), WM_OPEN_WITHEXPROP, (WPARAM) &args, 0);

		m_PostData.pPostData	= NULL;
		m_PostData.nPostBytes	= 0;
	}
}


// ����URL���쐬
bool CDonutSearchBar::GetOpenURLstr(CString& strOpenURL, const CString& strWord0, CIniFileI& pr, const CString& frontURL0)
{
	//����URL�̓ǂݍ���
	CString 	 strFrontURL	 = pr.GetString( _T("FrontURL") );
	if ( strFrontURL.IsEmpty() ) {
		if (frontURL0.IsEmpty())
			return false;
		strFrontURL = frontURL0;
	}
	CString 	 strBackURL 	 = pr.GetString( _T("BackURL") );

	//�����t���L�[���[�h�̓ǂݍ���
	CString 	 strFrontKeyWord = pr.GetString( _T("FrontKeyWord") );
	CString 	 strBackKeyWord  = pr.GetString( _T("BackKeyWord") );

	//������̍쐬
	CString strWord = strFrontKeyWord + strWord0 + strBackKeyWord;

	if (s_bFiltering)
		FilterString(strWord);		// �S�p�X�y�[�X�̒u��

	DWORD	dwEncode = pr.GetValue(_T("Encode"), 0);	// �G���R�[�h
	if (dwEncode != 0)
		_EncodeString(strWord, dwEncode);

	strOpenURL = strFrontURL + strWord + strBackURL;

	return true;
}



//private:
void CDonutSearchBar::OpenSearchGroup(const CString& strWord, const CString& strSearchEng, int &nLoopCnt, BOOL &bFirst)
{
	nLoopCnt++;
	if (nLoopCnt > 10)
		return;	// 10�őł��~��

	CString 	strSearchPath = GetSearchIniPath();
	CIniFileI	pr(strSearchPath, strSearchEng);

	int	nListCnt = (int)pr.GetValue( _T("ListCount"), 0 );	// �����O���[�v���̌����G���W����
	for (int ii = 1; ii <= nListCnt; ++ii) {
		CString 	strKey;
		strKey.Format(_T("%02d"), ii);
		CString 	strSearchEng2 = pr.GetStringUW( strKey );
		if ( strSearchEng2.IsEmpty() )
			continue;

		OpenSearch(strWord, strSearchEng2, nLoopCnt, bFirst);
	}
}


// ��������w�肳�ꂽ���@�ŃG���R�[�h����
void CDonutSearchBar::_EncodeString(CString &str, int dwEncode)	//minit
{
  #if 1	//+++ Unicode�Ή��ō�蒼��
	if	 (dwEncode == ENCODE_SHIFT_JIS)
		str = Misc::urlstr_encode( Misc::tcs_to_sjis(str) );
	else if (dwEncode == ENCODE_EUC)
		str = Misc::urlstr_encode( Misc::tcs_to_eucjp(str) );
	else if (dwEncode == ENCODE_UTF8)
		str = Misc::urlstr_encode( Misc::tcs_to_utf8(str) );
	else
		return;
  #else
	CURLEncoder enc;

	if (dwEncode == 0)
		return;
	else if (dwEncode == ENCODE_SHIFT_JIS)
		enc.URLEncode_SJIS(str);
	else if (dwEncode == ENCODE_EUC)
		enc.URLEncode_EUC(str);
	else if (dwEncode == ENCODE_UTF8)
		enc.URLEncode_UTF8(str);
	else
		return;
	//ATLASSERT(FALSE);
  #endif
}


// strWord����V���[�g�J�b�g�R�[�h�������Ă���ɍ����G���W������������
void CDonutSearchBar::_ShortcutSearch(CString &strWord, CString &strSearchEng)
{
	CString 	strSearchPath = GetSearchIniPath();

	if ( strWord.Left(1) == _T("\\") ) {	// '\'�Ŏn�܂��Ă��邩�ǂ���
		int 	nFind		= strWord.Find(_T(" "));
		CString strShort	= strWord.Mid(1, nFind - 1);

		CIniFileI	pr( strSearchPath, _T("Search-List") );
		int nListCount = (int)pr.GetValue( _T("ListCount") );	// �����G���W���̐�

		if ( strShort.IsEmpty() )
			return;

		strWord = strWord.Mid(nFind + 1);	// ����������̕���

		CString 	strBuf;
		CString 	strKey;
		for (int i = 1; i <= nListCount; i++) {
			// �G���W�������擾
			strKey.Format(_T("%02d"), i);
			CString 		strEngine = pr.GetStringUW( strKey );
			// �V���[�g�J�b�g�R�[�h���擾
			CString strShortcutWord = GetShortcutWord(strEngine);

			// ��r
			if (strShort == strShortcutWord) {
				strSearchEng = strEngine;	// ��������
				return;
			}
		}
	}
}


// strSearchEng�̃V���[�g�J�b�g���[�h��Ԃ�
CString CDonutSearchBar::GetShortcutWord(const CString& strSearchEng)
{
	CIniFileI	pr(GetSearchIniPath(), strSearchEng);
	return	pr.GetString( _T("ShortCutCode") );
}


// �����G���W�����R���{�{�b�N�X�ɓo�^����
void CDonutSearchBar::_InitialEngine(LPVOID lpV)
{
	CComboBox	cmb( (HWND) lpV );

	cmb.ResetContent(); 	//minit
	//::WINNLSEnableIME(cmb,FALSE);

	CString 	strSearchPath 	= GetSearchIniPath();
	int			nListCnt		= 0;
	if (::PathFileExists(strSearchPath)) {
		CIniFileI	pr( strSearchPath, _T("Search-List") );
		nListCnt	= pr.GetValue( _T("ListCount"), 0 );
		CString 	strKey;
		for (int ii = 1; ii <= nListCnt; ii++) {
			strKey.Format(_T("%02d"), ii);
			CString 	strTitle = pr.GetStringUW( strKey );
			if ( strTitle.IsEmpty() )
				continue;

			cmb.AddString(strTitle);
		}
	} else {	// Search.ini���Ȃ�����
		cmb.AddString(ENGINENAME_FOR_NO_SEARCH_INI);
	}

	CIniFileI	pr( g_szIniFileName, _T("SEARCH") );
	int	nIndex = pr.GetValue(_T("SelIndex"));
	int nSelIndex = 0;
	if (s_bLastSel && nIndex < nListCnt) {
		nSelIndex = nIndex;			// �Ō�ɑI�������T�[�`�G���W���𕜌�����
	}
	cmb.SetCurSel(nSelIndex);
}


// �����������R���{�{�b�N�X�ɓo�^����
void CDonutSearchBar::_InitialKeyword(LPVOID lpV)
{
	if (s_bHistorySave == false) 
		return;

	CComboBox	cmb( (HWND) lpV );
	
	CIniFileI	pr( _GetFilePath( _T("WordHistory.ini") ), _T("SEARCH_HISTORY") );
	int	nHistoryCnt = (int)pr.GetValue(_T("HistorySaveCnt"));

	for (int ii = 0; ii < nHistoryCnt; ii++) {
		CString 	strKey;
		strKey.Format(_T("KEYWORD%d"), ii);
		CString 	strKeyWord = pr.GetStringUW( strKey );
		if ( strKeyWord.IsEmpty() )
			continue;

		cmb.AddString(strKeyWord);
	}
}



// public:
BYTE CDonutSearchBar::PreTranslateMessage(MSG *pMsg)
{
  #if 1	//+++ �蔲���ȁA�`��X�V�`�F�b�N
	SetCmbKeywordEmptyStr();		//+++ �L�[���[�h���ɃG���W������\�����邽�߂̃t�H�[�J�X�`�F�b�N
  #endif

	UINT msg  = pMsg->message;
	int  vKey =  (int) pMsg->wParam;

	if (msg == WM_SYSKEYDOWN || msg == WM_SYSKEYUP || msg == WM_KEYDOWN) {
		if ( !IsWindowVisible() || !IsChild(pMsg->hwnd) )									// ignore
			return _MTL_TRANSLATE_PASS;

		// left or right pressed, check shift and control key.
		if (	vKey == VK_UP || vKey == VK_DOWN || vKey == VK_LEFT || vKey == VK_RIGHT
			 || vKey == VK_HOME || vKey == VK_END
			 || vKey == (0x41 + 'C' - 'A')
			 || vKey == (0x41 + 'V' - 'A')
			 || vKey == (0x41 + 'X' - 'A')
			 || vKey == VK_INSERT)
		{
			if (::GetKeyState(VK_SHIFT) < 0 || ::GetKeyState(VK_CONTROL) < 0)
				return _MTL_TRANSLATE_WANT; 												// pass to edit control
		}

		// return key have to be passed
		if (vKey == VK_RETURN) {
			return _MTL_TRANSLATE_WANT;
		}

		// other key have to be passed
		if (VK_LBUTTON <= vKey && vKey <= VK_HELP) {
			BOOL bAlt = HIWORD(pMsg->lParam) & KF_ALTDOWN;

			if (!bAlt && ::GetKeyState(VK_SHIFT) >= 0 && ::GetKeyState(VK_CONTROL) >= 0)	// not pressed
				return _MTL_TRANSLATE_WANT; 												// pass to edit control
		}
	}
  #if 1	//+++	�Ƃ肠�����A�ނ���A�E�N���b�N��WEB�����{�^���͈͓̔��ŉ����ꂽ�ꍇ�A�����G���W�����j���[���o��
	else if (msg == WM_RBUTTONUP) {
		CRect		rect;
		if (m_wndToolBar.GetRect(ID_SEARCH_WEB, rect)) {
			m_wndToolBar.ClientToScreen( rect );
			POINT 	pt;
			::GetCursorPos(&pt);
			if (pt.x >= rect.left && pt.x < rect.right && pt.y >= rect.top && pt.y < rect.bottom) {
				//MtlSendCommand(m_hWnd, ID_SEARCHENGINE_MENU);
				BOOL	dmy=0;
				OnSearchEngineMenu(0,0,0,dmy);
				return _MTL_TRANSLATE_HANDLE;
			}
		}
	}
  #endif
  #if 1 //*+++	�����Ώ�:�����o�[�ɃJ�[�\���������Ԃ�CTRL+RETURN�ŕœ��������������Ƃ��A
		//		�ǂ����̏�����CTRL+ENTER�ŃG���[�����o���Ă���悤�Ȃ̂����A�N���Ɛl��
		//		�킩��Ȃ��̂ŁA�����ŋ����ɃL�[��H���Č떂����.
	else if (msg == WM_CHAR && (vKey == VK_RETURN || vKey == 0x0a) && ::GetKeyState(VK_CONTROL) < 0) {
		if ( !IsWindowVisible() || !IsChild(pMsg->hwnd) )									// ignore
			return _MTL_TRANSLATE_PASS;
		return _MTL_TRANSLATE_HANDLE;
	}
  #endif

	return _MTL_TRANSLATE_PASS;
}



#if 0	//+++ WEB�����{�^���ŉE�N���b�N�����猟���G���W�����j���[��\�����Ă݂�
void CDonutSearchBar::OnToolBarRButtonUp(UINT nFlags, const CPoint& pt)
{
	CRect		rect;
	m_wndToolBar.GetRect(0, rect);
	if (pt.x >= rect.left && pt.x < rect.right && pt.y >= rect.top && pt.y < rect.bottom) {
		//MtlSendCommand(m_hWnd, ID_SEARCHENGINE_MENU);
		BOOL	dmy=0;
		OnSearchEngineMenu(0,0,0,dmy);
	}
}
#endif


// �L�[���[�h�̃G�f�B�b�g�R���g���[����Ԃ�
CEdit CDonutSearchBar::GetEditCtrl()
{
	return CEdit( m_cmbKeyword.GetDlgItem(IDC_EDIT/*1001*/) );
}


// �����G���W���R���{�{�b�N�X�Ƀt�H�[�J�X�����Ă�
void CDonutSearchBar::SetFocusToEngine()
{
	::SetFocus(m_cmbEngine.m_hWnd);
	m_cmbEngine.ShowDropDown(TRUE);
}



//private:
// ����������ۑ�����
void CDonutSearchBar::SaveHistory()
{
	int nItemCount = m_cmbKeyword.GetCount();
	if (nItemCount > s_nHistorySaveCnt) {
		nItemCount = s_nHistorySaveCnt;
	}

	CString 	strFileName = _GetFilePath( _T("WordHistory.ini") );
	CIniFileO	pr(strFileName, _T("SEARCH_HISTORY"));
	pr.DeleteSection();
	pr.SetValue( nItemCount, _T("HistorySaveCnt") );

	if (s_bHistorySave) {
		for (int ii = 0; ii < nItemCount; ii++) {
			CString 	strKeyWord;
			m_cmbKeyword.GetLBText(ii, strKeyWord);

			CString 	strKey;
			strKey.Format(_T("KEYWORD%d"), ii);
			pr.SetStringUW(strKeyWord, strKey);
		}
	}
}



//public:
DROPEFFECT CDonutSearchBar::OnDragEnter(IDataObject *pDataObject, DWORD dwKeyState, CPoint /*point*/)
{
	if (m_bDragFromItself)
		return DROPEFFECT_NONE;

	_DrawDragEffect(false);

	m_bDragAccept = _MtlIsHlinkDataObject(pDataObject);
	return _MtlStandardDropEffect(dwKeyState);
}



DROPEFFECT CDonutSearchBar::OnDragOver(IDataObject *pDataObject, DWORD dwKeyState, CPoint /*point*/, DROPEFFECT dropOkEffect)
{
	if (m_bDragFromItself || !m_bDragAccept)
		return DROPEFFECT_NONE;

	return _MtlStandardDropEffect(dwKeyState) | _MtlFollowDropEffect(dropOkEffect) | DROPEFFECT_COPY;
}



void CDonutSearchBar::OnDragLeave()
{
	if (m_bDragFromItself)
		return;

	_DrawDragEffect(true);
}



//private:
// �h���b�O���ꂽ�Ƃ��g��`��
void CDonutSearchBar::_DrawDragEffect(bool bRemove)
{
	CClientDC dc(m_wndKeyword.m_hWnd);

	CRect	  rect;
	m_wndKeyword.GetClientRect(rect);

	if (bRemove)
		MtlDrawDragRectFixed(dc.m_hDC, &rect, CSize(0, 0), &rect, CSize(2, 2), NULL, NULL);
	else
		MtlDrawDragRectFixed(dc.m_hDC, &rect, CSize(2, 2), NULL, CSize(2, 2),	NULL, NULL);
}



//public:
DROPEFFECT CDonutSearchBar::OnDrop(IDataObject *pDataObject, DROPEFFECT dropEffect, DROPEFFECT dropEffectList, CPoint /*point*/)
{
	if (m_bDragFromItself)
		return DROPEFFECT_NONE;

	_DrawDragEffect(true);

	CString 	strText;

	if (   MtlGetHGlobalText( pDataObject, strText)
		|| MtlGetHGlobalText( pDataObject, strText, ::RegisterClipboardFormat(CFSTR_SHELLURL) ) )
	{
		CEdit edit = GetEditCtrl();
		edit.SendMessage(WM_CHAR, 'P'); //m_cmbKeyword.GetCurSel() == -1�ɂ��邽�߂̋���̍� minit
		edit.SetWindowText(strText);

		BOOL  bSts = s_bDropGo;
		if (::GetKeyState(VK_SHIFT) < 0)
			bSts = !bSts;

		if (bSts) {
			_OnCommand_SearchWeb(strText);
		}

		return DROPEFFECT_COPY;
	}

	return DROPEFFECT_NONE;
}


// �t�H���g��ύX����
void CDonutSearchBar::SetFont(HFONT hFont, BOOL bRedraw /*= TRUE*/)
{
//	CDialogImpl<CDonutSearchBar>::SetFont(hFont, bRedraw);
	__super::SetFont(hFont, bRedraw);
	m_cmbEngine.SetFont(hFont, bRedraw);
	m_cmbKeyword.SetFont(hFont, bRedraw);
	m_wndKeyword.SetFont(hFont, bRedraw);
	if (m_wndToolBar.m_hWnd) {
		m_wndToolBar.SetFont(hFont, bRedraw);
	}

	/* �����o�[�̍������擾 */
	CRect	rc;
	m_cmbKeyword.GetWindowRect(&rc);
	int	height = rc.Height() + 1;

	/* �����o�[�̍������X�V */
	_RefreshBandInfo(height);

	/* �c�[���o�[�̍�����KeywordComboBox�̍����ɍ��킹�� */
	CSize	size;
	m_wndToolBar.GetButtonSize(size);
	m_wndToolBar.SetButtonSize(size.cx, height);
	m_ButtonSize.SetSize(size.cx, height);

	//_InitCombo();								//+++
	ResizeBar(0, 0);
}



//private:
void CDonutSearchBar::_SetVerticalItemCount(CComboBox &cmb, int count)
{
	CRect rc;
	int nIndex = cmb.AddString(_T("DUMMY"));
	int   itemheight = cmb.GetItemHeight(nIndex);
	cmb.DeleteString(nIndex);
	cmb.GetClientRect(&rc);
//	int dh	  = (itemheight > m_nDefDlgH) ? itemheight : m_nDefDlgH;
	int dh = itemheight;
	rc.bottom = rc.top + dh + (itemheight/3) + (itemheight * count) + 2;	//+++ ���������̊����ɂȂ�悤�ɁA�K���Ɍv�Z
	cmb.MoveWindow(&rc);
}



void CDonutSearchBar::_InitCombo()						//minit
{
//	m_cmbEngine.SetItemHeight(-1, m_nItemFontH);
//	m_cmbKeyword.SetItemHeight(-1, m_nItemFontH);

	if (s_bHeightCount) {
		ATLASSERT( 0 < s_nHeightCountCnt && s_nHeightCountCnt < MAXHEIGHTCOUNT );

		_SetVerticalItemCount(m_cmbEngine , s_nHeightCountCnt);
		_SetVerticalItemCount(m_cmbKeyword, s_nHeightCountCnt);
	}
  #if 1	//+++ vista�ȊO�ŃR���{�{�b�N�X�̍����������������������I�ɉ�����Ă݂�...
	else {
		_SetVerticalItemCount(m_cmbEngine , MAXHEIGHTCOUNT);
		_SetVerticalItemCount(m_cmbKeyword, DEFAULT_HEIGHTCOUNT/*50*/);
	}
  #endif
}



//public:
void CDonutSearchBar::ShowToolBarIcon(BOOL flag)
{
	if (flag) {
		if (m_bLoadedToolBar) {
			if ( ::IsWindow(m_wndToolBar.m_hWnd) )
				m_wndToolBar.ShowWindow(SW_NORMAL);
		} else {
			_InitToolBar( m_nBmpCX, m_nBmpCY, RGB(255, 0, 255) );
			m_wndToolBar.ShowWindow(SW_NORMAL);
		}
	} else {
		if (m_bLoadedToolBar) {
			m_wndToolBar.ShowWindow(SW_HIDE);
		}
	}

	m_bShowToolBar = flag;

	//�T�C�Y�ύX
	CRect	rect;
	GetWindowRect(rect);
	CWindow(GetParent()).ScreenToClient(rect);
	int 	width	= rect.right  - rect.left - 1;
	int 	height	= rect.bottom - rect.top;
//	SetWindowPos(NULL, rect.left, rect.top, width,	 height, SWP_NOZORDER | SWP_NOREDRAW);
//	SetWindowPos(NULL, rect.left, rect.top, width+1, height, SWP_NOZORDER);
}

// s_nMinimumLength�ȉ��̕�������폜����
void	CDonutSearchBar::DeleteMinimumLengthWord(CString &strWord)
{
	if ( 1 < s_nMinimumLength && strWord.IsEmpty() == FALSE) {
		std::vector<CString>	strSearchWords;
		Misc::SeptTextToWords(strSearchWords, strWord);
		strWord = _T("");
		std::vector<CString>::iterator it = strSearchWords.begin();
		for ( ; it != strSearchWords.end(); ++it ) {
			if (s_nMinimumLength <= it->GetLength() ) {
				strWord += *it;
				strWord += _T(" ");
			}
		}
	}
}

// �n�C���C�g�{�^�����������Ƃ�
void CDonutSearchBar::SearchHilight()
{
  #if 1 //+++
	CString str  = GetSearchStr();
  #else
	CEdit	edit = GetEditCtrl();
	CString str  = MtlGetWindowText(edit);
  #endif
	str = RemoveShortcutWord(str);
	//if (! str.IsEmpty())
		_OnCommand_SearchHilight(str);
}



void CDonutSearchBar::SearchPage(BOOL bForward)
{
	_OnCommand_SearchPage(bForward);
}


// �X�L�����ēǂݍ��݂���
void CDonutSearchBar::ReloadSkin(int nCmbStyle)
{
	SetComboStyle(nCmbStyle);

	if ( !m_wndToolBar.IsWindow() )
		return;

	m_bExistManifest	= IsExistManifestFile();				//+++

	CImageList	imgs	= m_wndToolBar.GetImageList();
	CImageList	imgsHot = m_wndToolBar.GetHotImageList();
	CImageList	imgsDis = m_wndToolBar.GetDisabledImageList();	//+++

	_ReplaceImageList(GetSkinSeachBarPath(0), imgs	 , IDB_SEARCHBUTTON);
	_ReplaceImageList(GetSkinSeachBarPath(1), imgsHot, IDB_SEARCHBUTTON_HOT);

  #if 1 //+++ Disabled�摜�̑Ή�.
	CString str = GetSkinSeachBarPath(2);
	int 	dis = 0;
	if (::PathFileExists(str) == FALSE) {					//+++ �摜�t�@�C�����Ȃ���
		if (::PathFileExists(GetSkinSeachBarPath(0))) {		//+++ �ʏ킪�����
			str = GetSkinSeachBarPath(0);					//+++ �ʏ��ő�p
		} else {											//+++ �ʏ���Ȃ����
			dis = IDB_SEARCHBUTTON_DIS; 					//+++ �f�t�H���g��Disable����g��.
		}
	}
	_ReplaceImageList(str, imgsDis, dis);					//+++
  #endif

	Invalidate(TRUE);
	m_wndToolBar.Invalidate(TRUE);
}



void	CDonutSearchBar::SetComboStyle(int nCmbStyle)
{
	m_cmbEngine.SetDrawStyle (nCmbStyle);
	m_cmbKeyword.SetDrawStyle(nCmbStyle);
}




//=========================================================================



//+++ �T�[�`�G���W�����j���[�����
bool CDonutSearchBar::_MakeSearchEngineMenu(CMenu& menu0)
{
	menu0.LoadMenu(IDR_SEARCHENGINE_MENU);
	if (menu0.m_hMenu == NULL)
		return false;
	CMenuHandle menu = menu0.GetSubMenu(0);
	if (menu.m_hMenu == NULL)
		return false;
	menu.DeleteMenu(0, MF_BYPOSITION );
	ATLASSERT( menu.GetMenuItemCount() == 0 );
	int 	num = m_cmbEngine.GetCount();
	if (num > ID_INSERTPOINT_SEARCHENGINE_END+1 - ID_INSERTPOINT_SEARCHENGINE) {
		ATLASSERT(FALSE);
		num = ID_INSERTPOINT_SEARCHENGINE_END+1 - ID_INSERTPOINT_SEARCHENGINE;
	}
	for (int i = 0; i < num; ++i) {
		CString 		  strName;
		m_cmbEngine.GetLBText(i, strName);
		menu.AppendMenu(MF_ENABLED | MF_STRING , ID_INSERTPOINT_SEARCHENGINE + i, strName);
	}
	return true;
}



//+++
bool CDonutSearchBar::OnSearchEngineMenu(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
 #if 1	//+++
	// �|�b�v�A�b�v���j���[���J��.
	::SetForegroundWindow(m_hWnd);
	POINT 	pt;
	::GetCursorPos(&pt);
	CMenu 	menu = GetSearchEngineMenuHandle();
	// �|�b�v�A�b�v���j���[��\��
	HRESULT hr = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, m_hWnd, NULL);
	if (hr < ID_INSERTPOINT_SEARCHENGINE || hr > ID_INSERTPOINT_SEARCHENGINE_END)
		return false;	// �Ȃɂ��I������Ȃ�����

	// �I�����ꂽ���̂���T�[�`�G���W�������擾.
	hr -= ID_INSERTPOINT_SEARCHENGINE;
	CString 	strEngine;
	if (menu.GetMenuString(hr, strEngine, MF_BYPOSITION) == 0)
		return false;

	// �I�����ꂽ�G���W���ŁA���݂̌����������web����.
	CString		strKeyword = GetSearchStr();
	SearchWeb_str_engine(strKeyword, strEngine);
	return true;
 #else
	// �T�[�`�G���W�����j���[��p��.
	GetSearchEngineMenu();

	// �|�b�v�A�b�v���j���[���J��.
	::SetForegroundWindow(m_hWnd);
	POINT 	pt;
	::GetCursorPos(&pt);
	CMenuHandle 	menu = m_engineMenu.GetSubMenu(0);
	HRESULT hr = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, pt.x, pt.y, m_hWnd, NULL);
	if (hr <= 0 || hr > menu.GetMenuItemCount())
		return 0;

	// �I�����ꂽ���̂���T�[�`�G���W�������擾.
	hr -= 1;
	CString 	strEngine;
	if (menu.GetMenuString(hr, strEngine, MF_BYCOMMAND) == 0)
		return 0;

	// �I�����ꂽ�G���W���ŁA���݂̌����������web����.
	SearchWeb_str_engine( GetSearchStr(), strEngine );
	return 1;
 #endif
}



//=========================================================================
//++++



#if SEARCH_JIKKEN == 1


LRESULT CDonutSearchBar::OnChevronPushed(int /*idCtrl*/, LPNMHDR pnmh, BOOL &bHandled)
{
	LPNMREBARCHEVRON lpnm = (LPNMREBARCHEVRON) pnmh;

	if ( lpnm->wID != GetDlgCtrlID() ) {
		bHandled = FALSE;
		return 1;
	}

	CMenuHandle 	 menu = PrepareChevronMenu();
	DisplayChevronMenu(menu, lpnm);
	CleanupChevronMenu(menu, lpnm);

	return 0;
}


CMenuHandle CDonutSearchBar::PrepareChevronMenu()
{
#if 0
	CMenuHandle menuCmdBar(m_wndToolBar.m_hMenu);

	// build a menu from hidden items
	CMenuHandle menu;
	BOOL		bRet = menu.CreatePopupMenu();

	ATLASSERT(bRet);
	RECT		rcClient = {0};
	bRet = GetClientRect(&rcClient);
	ATLASSERT(bRet);
	int client_right = rcClient.right;
	unsigned	num = m_arrBtn.GetSize();
	for (unsigned i = 0; i < num; ++i) {
		CCmdBarButton cbb	   = m_arrBtn[i];
		bool		  bEnabled = ( (cbb.m_fsState & CBSTATE_ENABLED) != 0 );

		int cbb_btn_right = cbb.m_rcBtn.right;
		if (cbb_btn_right > client_right) {
			TCHAR			szBuff[100];
			CMenuItemInfo	mii;
			mii.fMask	   = MIIM_TYPE | MIIM_SUBMENU;
			mii.dwTypeData = szBuff;
			mii.cch 	   = sizeof (szBuff) / sizeof (TCHAR);
			bRet		   = menuCmdBar.GetMenuItemInfo(i, TRUE, &mii);
			ATLASSERT(bRet);
			// Note: CmdBar currently supports only drop-down items
			ATLASSERT( ::IsMenu(mii.hSubMenu) );
			bRet		   = menu.AppendMenu( MF_STRING|MF_POPUP|(bEnabled ? MF_ENABLED : MF_GRAYED),
											 (UINT_PTR) mii.hSubMenu,
											 mii.dwTypeData );
			ATLASSERT(bRet);
		}
	}
	if (menu.m_hMenu && menu.GetMenuItemCount() == 0) { // no hidden buttons after all
		menu.DestroyMenu();
		return NULL;
	}

	return menu;
#else
	return NULL;
#endif
}


void CDonutSearchBar::DisplayChevronMenu(CMenuHandle menu, LPNMREBARCHEVRON lpnm)
{
	if (menu.m_hMenu == 0)
		return;

	// convert chevron rect to screen coordinates
	CWindow   wndFrom	 = lpnm->hdr.hwndFrom;
	RECT	  rc		 = lpnm->rc;

	wndFrom.ClientToScreen(&rc);
	// set up flags and rect
	UINT		uMenuFlags = TPM_LEFTBUTTON | TPM_VERTICAL | TPM_LEFTALIGN | TPM_TOPALIGN
						   | (!AtlIsOldWindows() ? TPM_VERPOSANIMATION : 0);
	TPMPARAMS	TPMParams;
	TPMParams.cbSize	= sizeof (TPMPARAMS);
	TPMParams.rcExclude = rc;
	::TrackPopupMenuEx(menu.m_hMenu, uMenuFlags, rc.left, rc.bottom, m_wndParent, &TPMParams);
}


void CDonutSearchBar::CleanupChevronMenu(CMenuHandle menu, LPNMREBARCHEVRON lpnm)
{
	if (menu.m_hMenu) {
		for (int i = menu.GetMenuItemCount() - 1; i >= 0; i--)
			menu.RemoveMenu(i, MF_BYPOSITION);
	}
	menu.DestroyMenu();
	CWindow wndFrom = lpnm->hdr.hwndFrom;
	RECT	rc		= lpnm->rc;
	wndFrom.ClientToScreen(&rc);
	MtlEatNextLButtonDownOnChevron(m_wndParent, rc);
}




#elif SEARCH_JIKKEN == 2


//+++
LRESULT CDonutSearchBar::OnChevronPushed(int /*idCtrl*/, LPNMHDR pnmh, BOOL &bHandled)
{
	ATLASSERT( ( (LPNMREBARCHEVRON) pnmh )->wID == GetDlgCtrlID() );

	if ( !PushChevron( pnmh, GetTopLevelParent() ) ) {
		bHandled = FALSE;
		return 1;
	}

	return 0;
}



//+++
HMENU CDonutSearchBar::ChevronHandler_OnGetChevronMenu(int nCmdID, HMENU &hMenuDestroy)
{
	bool		bDestroy = 0;
	bool		bSubMenu = 0;
	CMenuHandle menu = _GetDropDownMenu(nCmdID, bDestroy, bSubMenu);

	if (bDestroy)
		hMenuDestroy = menu.m_hMenu;

	if (bSubMenu)
		return menu.GetSubMenu(0);
	else
		return menu;
}



// Implemantation
HMENU CDonutSearchBar::_GetDropDownMenu(int nCmdID, bool &bDestroy, bool &bSubMenu)
{
	bDestroy = true;
	bSubMenu = false;
#if 0
	CEdit	edit = GetEditCtrl();
	CString str  = MtlGetWindowText(edit);

	switch (nCmdID) {
	case ID_SEARCH_WEB: 		_OnCommand_SearchWeb(str);								break;
	case ID_SEARCHBAR_HILIGHT:	_OnCommand_SearchHilight(str);							break;	//ID_SEARCH_HILIGHT:
	//+++ case ID_SEARCH_PAGE:	_OnCommand_SearchPage( (::GetKeyState(VK_SHIFT) < 0) ? FALSE : TRUE );	break;
	case ID_SEARCH_PAGE:		_OnCommand_SearchPage( (::GetKeyState(VK_SHIFT) >= 0)); break;
	case ID_SEARCHBAR_WORD00:
	case ID_SEARCHBAR_WORD01:
	case ID_SEARCHBAR_WORD02:
	case ID_SEARCHBAR_WORD03:
	case ID_SEARCHBAR_WORD04:
	case ID_SEARCHBAR_WORD05:
	case ID_SEARCHBAR_WORD06:
	case ID_SEARCHBAR_WORD07:
	case ID_SEARCHBAR_WORD08:
	case ID_SEARCHBAR_WORD09:	_OnCommand_SearchPage((::GetKeyState(VK_SHIFT) >= 0), nCmdID-ID_SEARCHBAR_WORD00);	break;
	default:					ATLASSERT(0);
	}
#endif
	return 0;
}


//+++
void	CDonutSearchBar::Chevronhandler_OnCleanupChevronMenu()
{
}
#endif
