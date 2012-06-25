// DocHostUIHandlerDispatch.cpp

#include "stdafx.h"
#include "DocHostUIHandlerDispatch.h"
#include "DonutView.h"
#include "MainFrame.h"
#include "option\MainOption.h"
#include "option\RightClickMenuDialog.h"
#include "option\MenuDialog.h"
#include "Download\DownloadManager.h"


// Constructor
CDocHostUIHandlerDispatch::CDocHostUIHandlerDispatch(CDonutView* pView)
	: m_bNoIECustom(false)
	, m_pView(pView)
	, m_nContextMenuMode(0)
	, m_pt(-1, -1)
{ }


STDMETHODIMP CDocHostUIHandlerDispatch::ShowContextMenu(
	/* [in] */ DWORD				dwID,
	/* [in] */ DWORD				x,
	/* [in] */ DWORD				y,
	/* [in] */ IUnknown*			pcmdtReserved,
	/* [in] */ IDispatch*			pdispReserved,
	/* [retval][out] */ HRESULT*	dwRetVal)
{
	if ( m_bNoIECustom && (GetKeyState(VK_LBUTTON) >= 0) )	//�J�X�^��&���N���b�N����Ă��邩�ǂ���
		return S_FALSE;

	POINT	pt = { (int)x, (int)y };
	*dwRetVal = _ShowCustomContextMenu(dwID, &pt, pcmdtReserved, pdispReserved);
	return S_OK;
}

STDMETHODIMP CDocHostUIHandlerDispatch::GetHostInfo(
	/* [out][in] */ DWORD* pdwFlags,
	/* [out][in] */ DWORD* pdwDoubleClick)
{
	//�ꎞ�I�ɊO��IDispatch�𖳌��ɂ��f�t�H���g���삳����
	CComQIPtr<IAxWinHostWindow> pHostWindow = m_pDefaultHandler;
	pHostWindow->SetExternalUIHandler(NULL);

	DOCHOSTUIINFO	info= { sizeof (DOCHOSTUIINFO) };
	info.dwFlags		= *pdwFlags;
	info.dwDoubleClick	= *pdwDoubleClick;
	HRESULT 		hr	= m_pDefaultHandler->GetHostInfo(&info);	//�f�t�H���g����

	//�O��IDispatch��L���ɂ���
	pHostWindow->SetExternalUIHandler(this);

	*pdwFlags		   = info.dwFlags;
	*pdwDoubleClick    = info.dwDoubleClick;
	return hr;
}

STDMETHODIMP CDocHostUIHandlerDispatch::GetDropTarget(/* [in] */ IUnknown* pDropTarget, /* [out] */ IUnknown** ppDropTarget)
{
	return m_pView->QueryInterface(IID_IUnknown, (void**)ppDropTarget);;
}

STDMETHODIMP CDocHostUIHandlerDispatch::TranslateAccelerator(
	/* [in] */ DWORD_PTR		hWnd,
	/* [in] */ DWORD			nMessage,
	/* [in] */ DWORD_PTR		wParam,
	/* [in] */ DWORD_PTR		lParam,
	/* [in] */ BSTR 			bstrGuidCmdGroup,
	/* [in] */ DWORD			nCmdID,
	/* [retval][out] */HRESULT* dwRetVal)
{
	if (::GetKeyState(VK_CONTROL) < 0 && nMessage != WM_CHAR) {
		if (wParam == 'F') {
			if (m_pView->GetParent().SendMessage(WM_COMMAND, ID_EDIT_FIND)) {
				*dwRetVal = S_OK;
				return S_OK;
			}
		}
	}

	return S_FALSE; //IE default action
}

/// �J�X�^���R���e�L�X�g���j���[��\������
HRESULT	CDocHostUIHandlerDispatch::_ShowCustomContextMenu(DWORD dwID, POINT* pptPosition, IUnknown* pCommandTarget, IDispatch* pDispatchObjectHit)
{
	CComQIPtr<IOleWindow>	pWindow = pCommandTarget;
	ATLASSERT(pWindow);
	HWND hWndTarget = NULL;
	HRESULT hr = pWindow->GetWindow(&hWndTarget);
	ATLASSERT(hWndTarget);

	HWND hWndTopLevel = CWindow(hWndTarget).GetTopLevelWindow();

	m_nContextMenuMode	= 0;	//+++
	// �܂����j���[�p�̃��\�[�X���g�p����Ă���΂�����J������
	::SendMessage(hWndTopLevel, WM_RELEASE_CONTEXTMENU, 0, 0);

	if (::GetKeyState(VK_LBUTTON) < 0) 
		dwID = CONTEXT_MENU_HOLDLEFTBUTTON;

	if (   dwID != CONTEXT_MENU_DEFAULT
		&& dwID != CONTEXT_MENU_IMAGE
		&& dwID != CONTEXT_MENU_TEXTSELECT
		&& dwID != CONTEXT_MENU_ANCHOR
		&& dwID != CONTEXT_MENU_HOLDLEFTBUTTON)
	{
		return S_FALSE;
	}

	m_pt = *pptPosition;
	m_nContextMenuMode	= dwID;	//+++
	//if (pptPosition) 					//+++
	//	m_pt	= *pptPosition;			//+++
	
	CString imageAnchorUrl;
	if (dwID == CONTEXT_MENU_ANCHOR) {
		if (pDispatchObjectHit) {
			CComQIPtr<IHTMLAnchorElement>	spAnchor = pDispatchObjectHit;
			ATLASSERT(spAnchor);
			CComBSTR strURL;
			HRESULT hr = spAnchor->get_href(&strURL);
			if (strURL) {
				m_strUrl = strURL;
			}
		}
		//m_strUrl = Donut_GetActiveStatusStr();
	} else if (dwID == CONTEXT_MENU_IMAGE) {
		if (pDispatchObjectHit) {
			CComQIPtr<IHTMLImgElement>	spImage = pDispatchObjectHit;
			ATLASSERT(spImage);
			CComBSTR strURL;
			HRESULT hr = spImage->get_src(&strURL);
			if (strURL) 
				m_strUrl = strURL;
			
			CComQIPtr<IHTMLElement> spElmImage = spImage;
			CComPtr<IHTMLElement> spParentElement;
			spElmImage->get_parentElement(&spParentElement);
			CComQIPtr<IHTMLAnchorElement> spAnchor = spParentElement;
			if (spAnchor) {
				CComBSTR strURL2;
				spAnchor->get_href(&strURL2);
				if (strURL2)
					imageAnchorUrl = strURL2;
			}
		}
	}


	enum {
		IDR_BROWSE_CONTEXT_MENU 	= 24641,
		IDR_FORM_CONTEXT_MENU		= 24640,
		SHDVID_GETMIMECSETMENU		=	 27,
		SHDVID_ADDMENUEXTENSIONS	=	 53,
	};

	CComQIPtr<IOleCommandTarget>	spOleCommandTarget = pCommandTarget;
	ATLASSERT(spOleCommandTarget);

	CMenuHandle	menu = CCustomContextMenuOption::GetContextMenuFromID(dwID);

	if (dwID == CONTEXT_MENU_DEFAULT) {	
		// Get the language submenu	// �G���R�[�h�̃T�u���j���[��ǉ� ���������ƃ��j���[�̕\���Ɏ��Ԃ�������...
		VARIANT var;
		hr = spOleCommandTarget->Exec(&CGID_ShellDocView, SHDVID_GETMIMECSETMENU, 0, NULL, &var);

		MENUITEMINFO	mii = { sizeof(MENUITEMINFO) };
		mii.fMask	   = MIIM_SUBMENU;
		mii.hSubMenu   = (HMENU) var.byref;

		#ifndef IDM_LANGUAGE
		enum { IDM_LANGUAGE 	= 2292 };
		#endif
		menu.SetMenuItemInfo(IDM_LANGUAGE, FALSE, &mii);
	}
	/* ���j���[�̗L��/������ݒ� */
	_SetMenuEnable(menu, spOleCommandTarget);


	CSimpleArray<HMENU> arrDestroyMenu;
	int nExtIndex = -1;
	CCustomContextMenuOption::AddSubMenu(menu, hWndTopLevel, arrDestroyMenu, nExtIndex);

	{	// Insert Shortcut Menu Extensions from registry
		VARIANT					var1;
		VARIANT					var2;

		V_VT(&var1)    = VT_INT_PTR;
		V_BYREF(&var1) = menu.m_hMenu;

		V_VT(&var2)    = VT_I4;
		V_I4(&var2)    = dwID;

		hr = spOleCommandTarget->Exec(&CGID_ShellDocView, SHDVID_ADDMENUEXTENSIONS, 0, &var1, &var2);
	}

	{
		// �������O���b�Z�[�W�𑗂�
	//	for (int ii = 0; ii < mapCmd.GetSize(); ii++) {
	//		_BeforeInitSpecialMenu( mapCmd.GetValueAt( ii ) );
	//	}

		// Show shortcut menu
		int 	nCmdAddFav	= MtlGetCmdIDFromAccessKey( menu, _T("&F") );

		DWORD	dwMenuStyle = TPM_LEFTALIGN | TPM_RETURNCMD;
		if (CMenuOption::s_bR_Equal_L) 
			dwMenuStyle |= TPM_RIGHTBUTTON;

		//{	// ���j���[�̉E�ɃV���[�g�J�b�g�L�[��\�����Ȃ��悤�ɂ���(���C���t���[���Őݒ肳���̂�h��)
		//	MENUITEMINFO menuInfo = { sizeof (MENUITEMINFO) };
		//	menuInfo.fMask	= MIIM_TYPE;
		//	menuInfo.fType	= MF_SEPARATOR;
		//	menu.InsertMenuItem(0, MF_BYPOSITION, &menuInfo);
		//}

		/* ���j���[��\�� */
		int iSelection = menu.TrackPopupMenu(dwMenuStyle, pptPosition->x, pptPosition->y, m_pView->m_hWnd);//hWndTarget);//hWndTopLevel);

		enum { ID_SAVEDIALOG = 2268 };	// �Ώۂ��t�@�C���ɕۑ�
		if (iSelection == ID_SAVEDIALOG && 
			m_pView->UseDownloadManager() ) 
		{	// DLManager�ɑ���
			m_pView->StartTheDownload(imageAnchorUrl.IsEmpty() ? m_strUrl : imageAnchorUrl);
		} else {
			// Send selected shortcut menu item command to shell
			LRESULT  lRes	= S_OK;
			if (iSelection != 0) {
				lRes = ::PostMessage(hWndTarget, WM_COMMAND, iSelection, NULL);
			}

			{	// MainFrame�Ƀ��b�Z�[�W�𑗐M���邩�ǂ���
																						
#if 0
		BOOL	bSendFrm = FALSE;

		// �R�}���h�͈�
		if (COMMAND_RANGE_START <= iSelection && iSelection <= COMMAND_RANGE_END)
			bSendFrm = TRUE;

		// ���C�ɓ���A�O���[�v
		else if ( (FAVORITE_MENU_ID_MIN <= iSelection && iSelection <= FAVORITE_MENU_ID_MAX)
				||(FAVGROUP_MENU_ID_MIN <= iSelection && iSelection <= FAVGROUP_MENU_ID_MAX) )
			bSendFrm = TRUE;

		// �X�N���v�g
		else if (ID_INSERTPOINT_SCRIPTMENU <= iSelection && iSelection <= ID_INSERTPOINT_SCRIPTMENU_END)
			bSendFrm = TRUE;

		#if 1	//+++ �����G���W��
		else if (ID_INSERTPOINT_SEARCHENGINE <= iSelection && iSelection <= ID_INSERTPOINT_SEARCHENGINE_END)
			bSendFrm = TRUE;
		#endif

		if ( bSendFrm == FALSE && mapCmd.Lookup( (DWORD) iSelection ) )
			bSendFrm = TRUE;

		if (true/*bSendFrm*/) {		// ���C���t���[���ɃR�}���h�𑗐M����
			lRes = ::SendMessage(m_hWndTopLevel, WM_COMMAND, iSelection, NULL);
		}
		
#endif

				//\\ ��Α��M����悤�ɂ��Ă݂�
				lRes = m_pView->GetParent().SendMessage(WM_COMMAND, iSelection);
				//lRes = ::SendMessage(hWndTopLevel, WM_COMMAND, iSelection, NULL);

				//���̎��_�ŃE�B���h�E��������Ă��܂��\�������邯��ǂ��E�E
				if (iSelection == nCmdAddFav) {
					::SendMessage(hWndTopLevel, WM_REFRESH_EXPBAR, 0, 0);
				}
			}
		}
	}
	// �����ł��ЂÂ�
	CCustomContextMenuOption::RemoveSubMenu(menu, arrDestroyMenu, nExtIndex);


//	for (int ii = 0; ii < mapCmd.GetSize(); ii++) {
//		::RemoveMenu( hMenuSub, mapCmd.GetKeyAt( ii ), MF_BYCOMMAND );
//		_RestoreSpecialMenu( mapCmd.GetValueAt( ii ) );
//	}

	//m_nContextMenuMode = 0;	//+++
	m_pt.SetPoint(-1, -1);
	m_strUrl.Empty();

	return S_OK;
}

/// ���j���[�̗L���E������spOleCommandTarget����ǂݍ���
void	CDocHostUIHandlerDispatch::_SetMenuEnable(HMENU hMenu, IOleCommandTarget* spOleCommandTarget)
{
	int nCount = GetMenuItemCount(hMenu);
	for (int i = 0; i < nCount; ++i) {
		OLECMD olecmd = { 0 };
		olecmd.cmdID = GetMenuItemID(hMenu, i);
		HRESULT hr = spOleCommandTarget->QueryStatus(&CGID_MSHTML, 1, &olecmd, NULL);
		if (SUCCEEDED(hr)) {
			if (olecmd.cmdf & OLECMDF_SUPPORTED) {
				UINT mf = MF_BYCOMMAND;
				mf |= (olecmd.cmdf & OLECMDF_ENABLED) ? MF_ENABLED : MF_DISABLED;
				mf |= (olecmd.cmdf & OLECMDF_LATCHED) ? MF_CHECKED : MF_UNCHECKED;
				::EnableMenuItem(hMenu, olecmd.cmdID, mf);
				::CheckMenuItem(hMenu, olecmd.cmdID, mf);
			}
		}
	}
}

