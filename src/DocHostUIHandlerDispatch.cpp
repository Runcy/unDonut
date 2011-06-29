// DocHostUIHandlerDispatch.cpp

#include "stdafx.h"
#include "DocHostUIHandlerDispatch.h"
#include "DonutView.h"
#include "MainFrame.h"
#include "option/MainOption.h"


// Constructor
CDocHostUIHandlerDispatch::CDocHostUIHandlerDispatch(CDonutView* pView)
	: m_bNoIECustom(false)
	, m_pView(pView)
{ }


STDMETHODIMP CDocHostUIHandlerDispatch::ShowContextMenu(
	/* [in] */ DWORD				dwID,
	/* [in] */ DWORD				x,
	/* [in] */ DWORD				y,
	/* [in] */ IUnknown*			pcmdtReserved,
	/* [in] */ IDispatch*			pdispReserved,
	/* [retval][out] */ HRESULT*	dwRetVal)
{
	if ( m_bNoIECustom && (GetKeyState(VK_LBUTTON) >= 0) )								//�J�X�^��&���N���b�N����Ă��邩�ǂ���
		return S_FALSE;
	*dwRetVal = CCustomContextMenu().Show(dwID, x, y, pcmdtReserved, pdispReserved);					//�ۓ���
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
	CComQIPtr<IDropTarget>	spDefaultDropTarget = pDropTarget;
	m_pView->SetDefaultDropTarget(spDefaultDropTarget);
	m_pView->QueryInterface(IID_IUnknown, (void**)ppDropTarget);
	return S_OK;
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
	if (CMainOption::s_bUseCustomFindBar && ::GetKeyState(VK_CONTROL) < 0 && nMessage != WM_CHAR) {
		if (wParam == 0x46) {	//F
			g_pMainWnd->SetFocusToSearchBarWithSelectedText();
			*dwRetVal = S_OK;
			return S_OK;
		}
	}

	return S_FALSE; //IE default action
}