/**
*	@file	PageMoveConfirmDialogAutoClose.h
*	@brief	�y�[�W�ړ��m�F�_�C�A���O�������ŕ��܂�
*/

#pragma once

#include <UIAutomationClient.h>
#include <atlcomcli.h>
#include "MainFrame.h"
#include "MultiThreadManager.h"


class CPageMoveConfirmDialogAutoClose : public IUIAutomationEventHandler
{
public:
	bool	WatchStart()
	{
		try {
			m_spUIAutomation.CoCreateInstance(__uuidof(CUIAutomation));
			if (m_spUIAutomation == nullptr)
				throw 3;

			CComPtr<IUIAutomationElement>	spElmRoot;
			//m_spUIAutomation->GetRootElement(&spElmRoot);
			//if (spElmRoot == nullptr)
			//	throw 1;


			m_spUIAutomation->ElementFromHandle(g_pMainWnd->GetHWND(), &spElmRoot);
			ATLASSERT(spElmRoot);

			// �_�C�A���O���J�����̂��Ď�����
			HRESULT hr = m_spUIAutomation->AddAutomationEventHandler(UIA_Window_WindowOpenedEventId, spElmRoot, TreeScope::TreeScope_Children, nullptr, (IUIAutomationEventHandler*)this);
			if (FAILED(hr))
				throw 2;

		} catch (...) {
			return false;
		}
		return true;
	}

	void	WatchStop()
	{
		if (m_spUIAutomation) {
			m_spUIAutomation->RemoveAllEventHandlers();
			m_spUIAutomation.Release();
		}
	}

	// IUnknown methods.
	ULONG STDMETHODCALLTYPE AddRef() { return 1; }
	ULONG STDMETHODCALLTYPE Release() { return 1; }

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppInterface)
	{
		if (riid == __uuidof(IUnknown)) {
			*ppInterface = static_cast<IUIAutomationEventHandler*>(this);
		} else if (riid == __uuidof(IUIAutomationEventHandler)) {
			*ppInterface = static_cast<IUIAutomationEventHandler*>(this);
		} else {
			*ppInterface = NULL;
			return E_NOINTERFACE;
		}
		return S_OK;
	}

	// IUIAutomationEventHandler methods
	HRESULT STDMETHODCALLTYPE HandleAutomationEvent(IUIAutomationElement * pSender, EVENTID eventID)
	{
		if (eventID == UIA_Window_WindowOpenedEventId) {
			CComBSTR strClassName;
			pSender->get_CurrentClassName(&strClassName);
			if (strClassName != L"#32770")
				return S_OK;

			//bool bOwnProcessDialog = false;
			//int dialogProcessId = 0;
			//pSender->get_CurrentProcessId(&dialogProcessId);
			//DWORD mainWindowProcessId = 0;
			//::GetWindowThreadProcessId(g_pMainWnd->GetHWND(), &mainWindowProcessId);
			//if (static_cast<DWORD>(dialogProcessId) == mainWindowProcessId) {
			//	bOwnProcessDialog = true;
			//} else {
			//	for (auto& childProcess : MultiThreadManager::g_vecChildProcessProcessThreadId) {
			//		if (static_cast<DWORD>(dialogProcessId) == childProcess.dwProcessId) {
			//			bOwnProcessDialog = true;
			//			break;
			//		}
			//	}
			//}
			//if (bOwnProcessDialog == false)
			//	return S_OK;

			//CComPtr<IUIAutomationElement>	spDialogElm;
			//pSender->FindFirst(TreeScope_Children, nullptr, &spDialogElm);
			//if (spDialogElm == nullptr)
			//	return S_OK;

			CComBSTR	strDialogName;
			pSender->get_CurrentName(&strDialogName);
			if (strDialogName != L"Windows Internet Explorer")
				return S_OK;

			CComVariant vButtonName(L"���̃y�[�W����ړ�(L)");
			CComPtr<IUIAutomationCondition>	spNameCond;
			m_spUIAutomation->CreatePropertyCondition(UIA_NamePropertyId, vButtonName, &spNameCond);
			ATLASSERT(spNameCond);

			CComPtr<IUIAutomationElement>	spMoveButtonElm;
			pSender->FindFirst(TreeScope_Descendants, spNameCond, &spMoveButtonElm);
			if (spMoveButtonElm == nullptr)
				return S_OK;

			CComPtr<IUIAutomationInvokePattern>	spInvoke;
			spMoveButtonElm->GetCurrentPattern(UIA_InvokePatternId, (IUnknown**)&spInvoke);
			if (spInvoke == nullptr)
				return S_OK;

			spInvoke->Invoke();
			return S_OK;
#if 0
			if (strClassName == L"CabinetWClass" || strClassName == L"Hidemaru32Class") {
				HWND hWndExplorer = NULL;
				pSender->get_CurrentNativeWindowHandle((UIA_HWND*)&hWndExplorer);
				// ���O�̕ύX�_�C�A���O���J�����̂��Ď�����
				RegisterWatchExplorer(hWndExplorer);

			} else if (strClassName == L"#32770") {		// �_�C�A���O���J���ꂽ				
				CComBSTR strCaption;
				pSender->get_CurrentName(&strCaption);
				if (strCaption == L"���O�̕ύX") {
					LogStream() << "���O�̕ύX�_�C�A���O���J����܂����I �͂�(Y)�������܂�" << std::endl;
					CComBSTR name(L"�͂�(Y)");
					CComVariant v(name);
					CComPtr<IUIAutomationCondition>	spConditionName;
					HRESULT hr = m_spUIAutomation->CreatePropertyCondition(UIA_NamePropertyId, v, &spConditionName);
					if (FAILED(hr))
						return S_OK;
					CComPtr<IUIAutomationElement>	spElmOK;
					pSender->FindFirst(TreeScope::TreeScope_Descendants, spConditionName, &spElmOK);
					if (spElmOK == nullptr)
						return S_OK;
					CComPtr<IUIAutomationInvokePattern>	spInvoke;
					spElmOK->GetCurrentPatternAs(UIA_InvokePatternId, IID_IUIAutomationInvokePattern, (void**)&spInvoke);
					if (spInvoke == nullptr)
						return S_OK;
					spInvoke->Invoke();
				} else if (strCaption == L"�G�ۃG�f�B�^") {
					HWND hWndHideDlg = NULL;
					pSender->get_CurrentNativeWindowHandle((UIA_HWND*)&hWndHideDlg);
					bool	bComform = false;
					::EnumChildWindows(hWndHideDlg, [](HWND hWnd, LPARAM lParam) -> BOOL {
						CString text;
						::GetWindowText(hWnd, text.GetBuffer(129), 128);
						text.ReleaseBuffer();
						if (text.Left(16) == L"�G�ۃG�f�B�^�̓V�F�A�E�F�A�ł��B") {
							(*(bool*)lParam) = true;
							return FALSE;
						}
						return TRUE;
					}, (LPARAM)&bComform);
					if (bComform) {
						LogStream() << "�Ñ��̃_�C�A���O���o�܂����I �͂�(Y)�������܂�" << std::endl;
						CComBSTR name(L"�͂�(Y)");
						CComVariant v(name);
						CComPtr<IUIAutomationCondition>	spConditionName;
						HRESULT hr = m_spUIAutomation->CreatePropertyCondition(UIA_NamePropertyId, v, &spConditionName);
						if (FAILED(hr))
							return S_OK;
						CComPtr<IUIAutomationElement>	spElmOK;
						pSender->FindFirst(TreeScope::TreeScope_Descendants, spConditionName, &spElmOK);
						if (spElmOK == nullptr)
							return S_OK;
						CComPtr<IUIAutomationInvokePattern>	spInvoke;
						spElmOK->GetCurrentPatternAs(UIA_InvokePatternId, IID_IUIAutomationInvokePattern, (void**)&spInvoke);
						if (spInvoke == nullptr)
							return S_OK;
						spInvoke->Invoke();
					}
				}
			}
#endif
		}
		return S_OK;
	}

private:
	CComPtr<IUIAutomation>	m_spUIAutomation;
};


































