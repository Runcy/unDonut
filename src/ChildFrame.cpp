/**
 *	@file	ChildFrame.cpp
 *	@brief	�^�u�y�[�W�P�̏���.
 */

#include "stdafx.h"
#include "ChildFrame.h"
#include "MtlBrowser.h"
#include "MtlWin.h"
#include "DonutView.h"
#include "MultiThreadManager.h"
#include "ChildFrameCommandUIUpdater.h"
#include "option\MainOption.h"
#include "option\DLControlOption.h"
#include "option\MouseDialog.h"
#include "option\IgnoreURLsOption.h"
#include "option\CloseTitleOption.h"
#include "option\UrlSecurityOption.h"
#include "option\SearchPropertyPage.h"
#include "option\AddressBarPropertyPage.h"
#include "FaviconManager.h"
#include "ToolTipManager.h"
#include "PluginManager.h"
#include "Download\DownloadManager.h"
#include "MainFrame.h"

DECLARE_REGISTERED_MESSAGE(GetMarshalIWebBrowserPtr)

#define MSG_WM_GETMARSHALIWEBBROWSERPTR()							   \
{															   \
	static UINT WM_GETMARSHALIWEBBROWSERPTR = GET_REGISTERED_MESSAGE(GetMarshalIWebBrowserPtr);	\
	if ( uMsg == WM_GETMARSHALIWEBBROWSERPTR ) { \
		SetMsgHandled(TRUE);								   \
		IStream*	pStream = nullptr;							\
		CoMarshalInterThreadInterfaceInStream(IID_IWebBrowser2, m_spBrowser, &pStream);	\
		lResult = (LRESULT)pStream;									   \
		if ( IsMsgHandled() )								   \
			return TRUE;									   \
	}														   \
}

namespace {

	
static const LPCTSTR	g_lpszLight[] = {
	_T("<span id='unDonutHilight' style='color:black;background:#FFFF00'>"),
	_T("<span id='unDonutHilight' style='color:black;background:#00FFFF'>"),
	_T("<span id='unDonutHilight' style='color:black;background:#FF00FF'>"),
	_T("<span id='unDonutHilight' style='color:black;background:#7FFF00'>"),
	_T("<span id='unDonutHilight' style='color:black;background:#1F8FFF'>"),
};

static const int	g_LIGHTMAX	= _countof(g_lpszLight);


struct _Function_SelectEmpt {
	void operator ()(IHTMLDocument2 *pDocument)
	{
		CComPtr<IHTMLSelectionObject> spSelection;
		HRESULT 	hr = pDocument->get_selection(&spSelection);
		if (spSelection)
			spSelection->empty();
	}
};

struct _Function_Hilight2 {
	LPCTSTR 	m_lpszKeyWord;
	BOOL		m_bHilight;

	_Function_Hilight2(LPCTSTR lpszKeyWord, BOOL bHilight)
		: m_lpszKeyWord(lpszKeyWord), m_bHilight(bHilight)
	{	}

	void operator ()(IHTMLDocument2* pDocument)
	{
		if (m_bHilight) {
			if ( !FindHilight(pDocument) ) {
				MakeHilight(pDocument);
			}
		} else {
			RemoveHilight(pDocument);
		}
	}

	// �n�C���C�g�쐬
	void MakeHilight(IHTMLDocument2* pDocument)
	{
	try {
		// �L�[���[�h�̍ŏ��̈����擾
		CString		strKeyWord = m_lpszKeyWord;

		//+++ �P���؂�𒲐�
		LPCTSTR		strExcept	= _T(" \t\"\r\n�@");
		strKeyWord = _tcstok( strKeyWord.GetBuffer(0), strExcept );
		strKeyWord.TrimLeft(strExcept);
		strKeyWord.TrimRight(strExcept);

		int 	nLightIndex = 0;
		HRESULT hr;

		// �L�[���[�h����ɂȂ�܂ŌJ��Ԃ�
		while ( !strKeyWord.IsEmpty() ) {
			CComPtr<IHTMLElement>		spHTMLElement;
			// <body>���擾
			hr = pDocument->get_body(&spHTMLElement);
			if (spHTMLElement == NULL) 
				break;

			CComQIPtr<IHTMLBodyElement>	spHTMLBody = spHTMLElement;
			if (spHTMLBody == NULL) 
				break;

			// �e�L�X�g�����W���擾
			CComPtr<IHTMLTxtRange>	  spHTMLTxtRange;
			hr = spHTMLBody->createTextRange(&spHTMLTxtRange);
			if (!spHTMLTxtRange)
				AtlThrow(hr);			

			//+++ �ő�L�[���[�h��(�������[�v�΍�)
			static unsigned maxKeyword	= Misc::getIEMejourVersion() <= 6 ? 1000 : 10000;
			//+++ �������[�v��Ԃ������I�������邽�߁A���[�v���J�E���g����
			unsigned num = 0;

			// �L�[���[�h������
			CComBSTR		bstrText= strKeyWord;
			VARIANT_BOOL	vBool	= VARIANT_FALSE;
			while (spHTMLTxtRange->findText(bstrText, 1, 0, &vBool), vBool == VARIANT_TRUE) {
				// ���ݑI�����Ă���HTML�e�L�X�g���擾
				CComBSTR	bstrTextNow;
				hr = spHTMLTxtRange->get_text(&bstrTextNow);
				if (FAILED(hr))
					AtlThrow(hr);

				// <span>��t��
				CComBSTR	bstrTextNew;
				bstrTextNew.Append(g_lpszLight[nLightIndex]);	// <span �`
				bstrTextNew.Append(bstrTextNow);
				bstrTextNew.Append(_T("</span>"));


				CComPtr<IHTMLElement> spParentElement;
				hr = spHTMLTxtRange->parentElement(&spParentElement);
				if (FAILED(hr))
					AtlThrow(hr);

				CComBSTR	bstrParentTag;
				hr = spParentElement->get_tagName(&bstrParentTag);
				if (FAILED(hr))
					AtlThrow(hr);

				if (   bstrParentTag != _T("SCRIPT")
					&& bstrParentTag != _T("NOSCRIPT")
					&& bstrParentTag != _T("TEXTAREA")
					&& bstrParentTag != _T("STYLE"))
				{
					hr = spHTMLTxtRange->pasteHTML(bstrTextNew);	// �n�C���C�g����
					if (FAILED(hr))
						AtlThrow(hr);

					//+++ �ʏ�̃y�[�W�Ńn�C���C�g�u��������Ȃɂ����邱�Ƃ͂Ȃ����낤�ŁA�������[�v�����ł����ǂ߂��Ƃ�
					if (++num > maxKeyword)		
						break;
				}
				spHTMLTxtRange->collapse(VARIANT_FALSE);	// Caret�̈ʒu��I�������e�L�X�g�̈�ԉ���
			}

			++nLightIndex;
			if (nLightIndex >= g_LIGHTMAX)
				nLightIndex = 0;

			// ���̃L�[���[�h��
			strKeyWord = _tcstok(NULL, strExcept);
			strKeyWord.TrimLeft(strExcept);
			strKeyWord.TrimRight(strExcept);
		}

	} catch (const CAtlException& e) {
			e;	// ��O������Ԃ�
	}	// try
	}

	// �n�C���C�g����������
	void RemoveHilight(IHTMLDocument2* pDocument)
	{
		CComBSTR	hilightID(L"unDonutHilight");
		CComBSTR	hilightTag(L"SPAN");

		CComPtr<IHTMLElementCollection> pAll;

		if (SUCCEEDED( pDocument->get_all(&pAll) ) && pAll != NULL) {
			CComVariant 		id(L"unDonutHilight");
			CComPtr<IDispatch>	pDisp;
			CComVariant 		vIndex(0);
			pAll->item(id, vIndex, &pDisp);
			if (pDisp == NULL)
				return;

			CComPtr<IUnknown>	pUnk;

			if (SUCCEEDED( pAll->get__newEnum(&pUnk) ) && pUnk != NULL) {
				CComQIPtr<IEnumVARIANT> pEnumVariant = pUnk;

				if (pEnumVariant != NULL) {
					VARIANT  v;
					ULONG	 ul;
					CComBSTR bstrTagName;
					CComBSTR bstrID;
					CComBSTR bstrTmp;

					while (pEnumVariant->Next(1, &v, &ul) == S_OK) {
						CComQIPtr<IHTMLElement> pElement = v.pdispVal;
						VariantClear(&v);

						if (pElement != NULL) {
							bstrTagName.Empty();
							bstrID.Empty();
							pElement->get_tagName(&bstrTagName);
							pElement->get_id(&bstrID);

							if (bstrTagName == hilightTag && bstrID == hilightID) {
								bstrTmp.Empty();
								pElement->get_innerHTML(&bstrTmp);
								pElement->put_outerHTML(bstrTmp);
							}
						}
					}
				}
			}
		}
	}

	// �n�C���C�g�����łɂ���Ă��邩�m�F����
	BOOL FindHilight(IHTMLDocument2* pDocument)
	{
		CComPtr<IHTMLElementCollection> 	pAll;

		if (SUCCEEDED(pDocument->get_all(&pAll)) && pAll != NULL) {
			CComVariant 		id(L"unDonutHilight");
			CComPtr<IDispatch>	pDisp;
			CComVariant 		vIndex(0);
			pAll->item(id, vIndex, &pDisp);
			if (pDisp != NULL) {
				return TRUE;
			}
		}
		return FALSE;
	}
};

};	// namespace

/////////////////////////////////////////////////////////////
// CChildFrame::Impl

class CChildFrame::Impl :
	public CDoubleBufferWindowImpl<Impl>,
	public CMessageFilter,
	public IWebBrowserEvents2Impl<Impl, ID_DONUTVIEW>,
	public CWebBrowserCommandHandler<Impl>,
	public CWebBrowser2
{
	friend class CChildFrame;

public:
	DECLARE_WND_CLASS_EX(_T("DonutChildFrame"), 0, 0/*COLOR_APPWORKSPACE*/)

	Impl(CChildFrame* pChild);

	void	SetThreadRefCount(int* pCount) { m_pThreadRefCount = pCount; }

	DWORD	GetExStyle() const { return m_view.GetExStyle(); }
	void	SetExStyle(DWORD dwStyle);
	void	SetDLCtrl(DWORD dwDLCtrl) { m_view.PutDLControlFlags(dwDLCtrl); }
	void	SetMarshalDLCtrl(DWORD dwDLCtrl) { m_dwMarshalDLCtrlFlags = dwDLCtrl; }
	void	SetAutoRefreshStyle(DWORD dwAutoRefresh) { m_view.SetAutoRefreshStyle(dwAutoRefresh); }
	void	SaveSearchWordflg(bool bSave) { m_bSaveSearchWordflg = bSave; }
	void 	SetSearchWordAutoHilight(const CString& str, bool bAutoHilight);
	void	SetTravelLog(const vector<std::pair<CString, CString> >& fore, const vector<std::pair<CString, CString> >& back) {
		m_TravelLogFore = fore; m_TravelLogBack = back;
	}

	CString	GetSelectedText();
	CString GetSelectedTextLine();

	// Event handlers
	void	OnBeforeNavigate2(IDispatch*		pDisp,
						   const CString&		strURL,
						   DWORD				nFlags,
						   const CString&		strTargetFrameName,
						   CSimpleArray<BYTE>&	baPostedData,
						   const CString&		strHeaders,
						   bool&				bCancel );
	void	OnDownloadBegin() { }
	void	OnDownloadComplete();
	void	OnTitleChange(const CString& strTitle);
	void	OnProgressChange(long progress, long progressMax);
	void	OnStatusTextChange(const CString& strText);
	void	OnSetSecureLockIcon(long nSecureLockIcon);
	void	OnPrivacyImpactedStateChange(bool bPrivacyImpacted);
	void	OnStateConnecting();
	void	OnStateDownloading();
	void	OnStateCompleted();
	void	OnDocumentComplete(IDispatch *pDisp, const CString& strURL);
	void	OnCommandStateChange(long Command, bool bEnable);
	void	OnNewWindow2(IDispatch **ppDisp, bool& bCancel);
	void	OnNewWindow3(IDispatch **ppDisp, bool& bCancel, DWORD dwFlags, BSTR bstrUrlContext,  BSTR bstrUrl);
	void	OnWindowClosing(bool IsChildWindow, bool& bCancel);

	// PreTranslateMessage�p
	BOOL	OnRButtonHook(MSG* pMsg);
	BOOL	OnMButtonHook(MSG* pMsg);
	BOOL	OnXButtonUp(WORD wKeys, WORD wButton);

	// Overrides
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void OnFinalMessage(HWND /*hWnd*/) { delete m_pParentChild;/*this;*/ }
	void	searchEngines(const CString &strKeyWord);
	void	DoPaint(CDCHandle dc);

	// Message map
	BEGIN_MSG_MAP( Impl )
		MSG_WM_CREATE		( OnCreate )
		MSG_WM_DESTROY		( OnDestroy )
		MSG_WM_CLOSE		( OnClose )
		MSG_WM_SIZE			( OnSize )
		MSG_WM_GETMARSHALIWEBBROWSERPTR()
		USER_MSG_WM_CHILDFRAMEACTIVATE( OnChildFrameActivate )
		USER_MSG_WM_SET_CHILDFRAME( OnGetChildFrame )
		USER_MSG_WM_GETCHILDFRAMEDATA( OnGetChildFrameData )
		USER_MSG_WM_MENU_GOBACK 	( OnMenuGoBack		)
		USER_MSG_WM_MENU_GOFORWARD	( OnMenuGoForward	)
		USER_MSG_WM_GETSELECTEDTEXT	( OnGetSelectedText	)
		USER_MSG_WM_EXECUTEUSERJAVASCRIPT( OnExecuteUserJavascript )
		USER_MSG_WM_SETPAGEBITMAP	( OnSetPageBitmap )
		USER_MSG_WM_DRAWCHILDFRAMEPAGE( OnDrawChildFramePage )
		USER_MSG_WM_INCREMENTTHREADREFCOUNT()

		// �t�@�C��
		COMMAND_ID_HANDLER_EX( ID_EDIT_OPEN_SELECTED_REF, OnEditOpenSelectedRef 	)	// �����N���J��
		COMMAND_ID_HANDLER_EX( ID_EDIT_OPEN_SELECTED_TEXT,OnEditOpenSelectedText	)	// URL�e�L�X�g���J��

		// �ҏW
		COMMAND_ID_HANDLER_EX( ID_EDIT_FIND 			, OnEditFind				)
		COMMAND_ID_HANDLER_EX( ID_EDIT_FIND_MAX 		, OnEditFindMax 			)
		COMMAND_ID_HANDLER_EX( ID_TITLE_COPY			, OnTitleCopy				)
		COMMAND_ID_HANDLER_EX( ID_URL_COPY				, OnUrlCopy 				)
		COMMAND_ID_HANDLER_EX( ID_COPY_TITLEANDURL		, OnTitleAndUrlCopy 		)

		// �\��
		COMMAND_ID_HANDLER_EX( ID_VIEW_SETFOCUS 		, OnViewSetFocus			)
		COMMAND_ID_HANDLER_EX( ID_VIEW_STOP				, OnViewStop				)
		
		// �c�[��
		COMMAND_ID_HANDLER_EX( ID_EDIT_IGNORE			, OnAddClosePopupUrl		)
		COMMAND_ID_HANDLER_EX( ID_EDIT_CLOSE_TITLE		, OnAddClosePopupTitle		)

		// �E�B���h�E
		COMMAND_ID_HANDLER_EX( ID_FILE_CLOSE			, OnFileClose				)

		COMMAND_RANGE_HANDLER_EX( ID_VIEW_BACK1   , ID_VIEW_BACK9	, OnViewBackX	)
		COMMAND_RANGE_HANDLER_EX( ID_VIEW_FORWARD1, ID_VIEW_FORWARD9, OnViewForwardX)
		
		// �����o�[����
		MSG_WM_USER_HILIGHT 		( OnHilight 		)
		MSG_WM_USER_FIND_KEYWORD	( OnFindKeyWord 	)
		// �Ǝ��y�[�W�������o�[����
		USER_MSG_WM_HILIGHTFROMFINDBAR( OnHilightFromFindBar )
		USER_MSG_WM_REMOVEHILIGHT( OnRemoveHilight )

		COMMAND_ID_HANDLER_EX( ID_HTMLZOOM_MENU			, OnHtmlZoomMenu			)
		// Special command
		COMMAND_ID_HANDLER_EX( ID_HTMLZOOM_ADD			, OnHtmlZoom				)
		COMMAND_ID_HANDLER_EX( ID_HTMLZOOM_SUB			, OnHtmlZoom				)
		COMMAND_ID_HANDLER_EX( ID_HTMLZOOM_100TOGLE		, OnHtmlZoom				)
		COMMAND_RANGE_HANDLER_EX( ID_HTMLZOOM_400 , ID_HTMLZOOM_050 , OnHtmlZoom    )
		COMMAND_ID_HANDLER_EX( ID_SPECIAL_HOME 		, OnSpecialKeys		)
		COMMAND_ID_HANDLER_EX( ID_SPECIAL_END		, OnSpecialKeys		)
		COMMAND_ID_HANDLER_EX( ID_SPECIAL_PAGEUP	, OnSpecialKeys		)
		COMMAND_ID_HANDLER_EX( ID_SPECIAL_PAGEDOWN	, OnSpecialKeys		)
		COMMAND_ID_HANDLER_EX( ID_SPECIAL_UP		, OnSpecialKeys		)
		COMMAND_ID_HANDLER_EX( ID_SPECIAL_DOWN 		, OnSpecialKeys		)
		COMMAND_ID_HANDLER_EX( ID_SAVEIMAGE			, OnSaveImage		)

		COMMAND_ID_HANDLER_EX( ID_VIEW_REFRESH		, OnViewRefresh 	)

		CHAIN_COMMANDS_MEMBER( m_view )
		CHAIN_MSG_MAP( CDoubleBufferWindowImpl<Impl> )
		CHAIN_MSG_MAP( CWebBrowserCommandHandler<Impl> )

		if (uMsg == WM_COMMAND)
			GetTopLevelWindow().PostMessage(WM_COMMAND_FROM_CHILDFRAME, wParam, lParam);
	END_MSG_MAP()

	int		OnCreate(LPCREATESTRUCT /*lpCreateStruct*/);
	void	OnDestroy();
	void	OnClose();
	void	OnSize(UINT nType, CSize size);
	void	OnChildFrameActivate(HWND hWndAct, HWND hWndDeact);	// �^�u�̐؂�ւ����ʒm�����
	CChildFrame* OnGetChildFrame() { return m_pParentChild; }
	void	OnGetChildFrameData(ChildFrameDataOnClose* pData) { _CollectDataOnClose(*pData); }

	LRESULT OnMenuGoBack(HMENU hMenu)	 { MenuChgGoBack(hMenu);	return 0; }
	LRESULT OnMenuGoForward(HMENU hMenu) { MenuChgGoForward(hMenu); return 0; }
	void	OnGetSelectedText(LPCTSTR* ppStr);
	void	OnExecuteUserJavascript(CString* pstrScriptText);
	void	OnSetPageBitmap(HBITMAP* pBmp) { m_pPageBitmap = pBmp; }
	void	OnDrawChildFramePage(CDCHandle dc);

	// �t�@�C��
	void 	OnEditOpenSelectedRef(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	void 	OnEditOpenSelectedText(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);

	// �ҏW
	void	OnEditFind(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);
	void	OnEditFindMax(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/) { _OnEditFindMax(0, 0, NULL); }
	void	OnTitleCopy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);
	void	OnUrlCopy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);
	void	OnTitleAndUrlCopy(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);	

	// �\��
	void	OnViewSetFocus(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/) { m_view.SetFocus(); }
	void	OnViewStop(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);
	void	OnViewBackX(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);
	void	OnViewForwardX(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/);

	// �c�[��
	void 	OnAddClosePopupUrl(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);
	void 	OnAddClosePopupTitle(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);

	// �E�B���h�E
	void 	OnFileClose(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);

	// �����o�[����
	LRESULT OnHilight(CString strKeyWord);
	LRESULT OnFindKeyWord(LPCTSTR lpszKeyWord, BOOL bFindDown, long Flags = 0);
	// �Ǝ��y�[�W�������o�[����
	int		OnHilightFromFindBar(LPCTSTR strText, bool bNoHighlight, bool bEraseOld, long Flags);
	void	OnRemoveHilight();

	void	OnHtmlZoomMenu(UINT uNotifyCode, int nID, CWindow wndCtl);
	// Specla command
	void	OnHtmlZoom(UINT uNotifyCode, int nID, CWindow wndCtl);
	void	OnSpecialKeys(UINT uNotifyCode, int nID, CWindow wndCtl);
	void	OnSaveImage(UINT uNotifyCode, int nID, CWindow wndCtl);

	void	OnViewRefresh(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);

private:
	DWORD	_GetInheritedDLCtrlFlags();
	DWORD	_GetInheritedExStyleFlags();
	bool	_CursorOnSelectedText();
	void	_InitTravelLog();
	void	_CollectDataOnClose(ChildFrameDataOnClose& data);
	void	_AutoImageResize(bool bFirst);
	void	_SetFavicon(const CString& strURL);
	void	_HilightOnce(IDispatch *pDisp, LPCTSTR lpszKeyWord);
	BOOL 	_FindKeyWordOne(IHTMLDocument2* pDocument, const CString& strKeyword, BOOL bFindDown, long Flags = 0);

	// Data members
	CChildFrame*	m_pParentChild;
	CDonutView	m_view;
	CChildFrameUIStateChange	m_UIChange;
	int*	m_pThreadRefCount;
	bool	m_bNowActive;
	bool	m_bSaveSearchWordflg;
	CString	m_strSearchWord;
	CComBSTR	m_strBookmark;
	int 		m_nPainBookmark;
	CString		m_strOldKeyword;
	bool	m_bNowHilight;
	bool	m_bAutoHilight;
	CString m_strStatusText;
	bool	m_bExecutedNewWindow;	// for OnMButtonHook
	bool	m_bCancelRButtonUp;		// for PreTranslateMessage
	DWORD	m_dwMarshalDLCtrlFlags;
	bool	m_bInitTravelLog;
	vector<std::pair<CString, CString> >	m_TravelLogFore;
	vector<std::pair<CString, CString> >	m_TravelLogBack;

	int		m_nImgScl;					//+++ zoom,img�T�C�Y�����ݒ�ł̐ݒ�l.
	int		m_nImgSclSav;				//+++ zoom,img�T�C�Y��100%�Ƃ̃g�O���؂�ւ��p
	int		m_nImgSclSw;				//+++ zoom,img�T�C�Y��100%�Ƃ̃g�O���؂�ւ��p
	bool	m_bImagePage;	// �摜�t�@�C�����J�������ǂ���
	CSize	m_ImageSize;

	bool	m_bReload;
	bool	m_bNowNavigate;
	bool	m_bClosing;
	//CBitmap	m_bmpPage;
	HBITMAP*	m_pPageBitmap;
};

#include "ChildFrame.inl"


/////////////////////////////////////////////////////////////
// CChildFrame

CChildFrame::CChildFrame() : pImpl(new Impl(this))
{
}

CChildFrame::~CChildFrame()
{
	delete pImpl;
}

/// �X���b�h�𗧂Ă�CChildFrame�̃C���X�^���X�����
void	CChildFrame::AsyncCreate(NewChildFrameData& data)
{
	CChildFrame*	pChild = new CChildFrame;
	if (data.dwDLCtrl == -1)
		data.dwDLCtrl	= CDLControlOption::s_dwDLControlFlags;
	if (data.dwExStyle == -1)
		data.dwExStyle	= CDLControlOption::s_dwExtendedStyleFlags;
	pChild->pImpl->m_view.SetDefaultFlags(data.dwDLCtrl, data.dwExStyle, data.dwAutoRefresh);

	MultiThreadManager::ExecuteChildFrameThread(pChild, &data);
}

void	CChildFrame::SetThreadRefCount(int* pCount)
{
	pImpl->SetThreadRefCount(pCount);
}

HWND	CChildFrame::CreateEx(HWND hWndParent)
{
	RECT rc;
	::GetClientRect(hWndParent, &rc);
	return pImpl->Create(hWndParent, rc, NULL, WS_CHILD /*| WS_VISIBLE*/);
}

void	CChildFrame::Navigate2(LPCTSTR lpszURL)
{
	pImpl->Navigate2(lpszURL);
}

HWND	CChildFrame::GetHwnd() const
{
	return pImpl->m_hWnd;
}

DWORD	CChildFrame::GetExStyle() const
{
	return pImpl->GetExStyle();
}

void	CChildFrame::SetExStyle(DWORD dwStyle)
{
	pImpl->SetExStyle(dwStyle);
}

void	CChildFrame::SetDLCtrl(DWORD dwDLCtrl)
{
	pImpl->SetDLCtrl(dwDLCtrl);
}

void	CChildFrame::SetMarshalDLCtrl(DWORD dwDLCtrl)
{
	pImpl->SetMarshalDLCtrl(dwDLCtrl);
}

void	CChildFrame::SetAutoRefreshStyle(DWORD dwAutoRefresh)
{
	pImpl->SetAutoRefreshStyle(dwAutoRefresh);
}

void	CChildFrame::SaveSearchWordflg(bool bSave)
{
	pImpl->SaveSearchWordflg(bSave);
}

void	CChildFrame::SetSearchWordAutoHilight(const CString& str, bool bAutoHilight)
{
	pImpl->SetSearchWordAutoHilight(str, bAutoHilight);
}

void	CChildFrame::SetTravelLog(const vector<std::pair<CString, CString> >& fore, const vector<std::pair<CString, CString> >& back)
{
	pImpl->SetTravelLog(fore, back);
}


CComPtr<IWebBrowser2>	CChildFrame::GetIWebBrowser()
{
	return pImpl->m_spBrowser;
}


CComPtr<IWebBrowser2>	CChildFrame::GetMarshalIWebBrowser()
{
	IStream* pStream = (IStream*)pImpl->SendMessage(GET_REGISTERED_MESSAGE(GetMarshalIWebBrowserPtr));
	if (pStream == nullptr)
		return nullptr;
	CComPtr<IWebBrowser2>	spBrowser;
	CoGetInterfaceAndReleaseStream(pStream, IID_IWebBrowser2, (void**)&spBrowser);
	return spBrowser;
}

CString	CChildFrame::GetLocationURL()
{
	CComPtr<IWebBrowser2>	spBrowser = GetMarshalIWebBrowser();
	if (spBrowser == nullptr)
		return CString();
	CComBSTR	strtemp;
	spBrowser->get_LocationURL(&strtemp);
	if (strtemp)
		return CString(strtemp);
	return CString();
}

CString CChildFrame::GetTitle()
{
	CComPtr<IWebBrowser2>	spBrowser = GetMarshalIWebBrowser();
	if (spBrowser == nullptr)
		return CString();
	CComBSTR	strtemp;
	spBrowser->get_LocationName(&strtemp);
	if (strtemp)
		return CString(strtemp);
	return CString();
}