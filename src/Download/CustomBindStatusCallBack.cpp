// CustomBindStatusCallBack.cpp

#include "stdafx.h"
#include "CustomBindStatusCallBack.h"
#include <regex>
#include <MLang.h>
#include <atldlgs.h>
#include "../Misc.h"
#include "../MtlMisc.h"
#include "DownloadOptionDialog.h"
#include "DownloadingListView.h"

#pragma comment(lib,"wininet.lib")
#pragma comment(lib,"Urlmon.lib")
#pragma comment(lib,"winmm.lib")


////////////////////////////////////////////////////////////////////////////////////////
// CCustomBindStatusCallBack

// �ǂ���m_spBSCBPrev��NULL���Ƃc�k������I�����Ȃ��悤�Łi�w�����Ă��~�܂�Ȃ����j

// Constructor;
CCustomBindStatusCallBack::CCustomBindStatusCallBack(DLItem* pItem, HWND hWndDLing)
	: m_dwTotalRead(0)
	, m_hFile(INVALID_HANDLE_VALUE)
	, m_pDLItem(pItem)
	, m_hWndDLing(hWndDLing) 
	, m_cRef(0)
	, m_hWndNotify(NULL)
{
	AddRef();
}

// Destructor
CCustomBindStatusCallBack::~CCustomBindStatusCallBack()
{
}


//------------------------------
/// ���t�@����ݒ�
void	CCustomBindStatusCallBack::SetReferer(LPCTSTR strReferer)
{
	m_pDLItem->strReferer = strReferer;
}



void	CCustomBindStatusCallBack::SetOption(LPCTSTR strDLFolder, HWND hWnd, DWORD dwOption)
{
	m_strDLFolder = strDLFolder;
	m_hWndNotify = hWnd;
	m_dwDLOption = dwOption;
}


// DL���L�����Z������
void	CCustomBindStatusCallBack::Cancel()
{
	if (m_pDLItem) 
		m_pDLItem->bAbort = true;
	if (m_spBinding)
		m_spBinding->Abort();
}



// �őO�ʂɗ��郁�b�Z�[�W�{�b�N�X��\������
int CCustomBindStatusCallBack::_ActiveMessageBox(const CString& strText, UINT uType)
{
	// �őO�ʃv���Z�X�̃X���b�hID���擾���� 
	int foregroundID = ::GetWindowThreadProcessId( ::GetForegroundWindow(), NULL); 
	// �őO�ʃA�v���P�[�V�����̓��͏����@�\�ɐڑ����� 
	AttachThreadInput( ::GetCurrentThreadId(), foregroundID, TRUE); 
	// �őO�ʃE�B���h�E��ύX���� 
	::SetForegroundWindow(m_hWndDLing);

	int nReturn = MessageBox(m_hWndDLing, strText, NULL, uType);
	// �ڑ�����������
	AttachThreadInput( ::GetCurrentThreadId(), foregroundID, FALSE);

	return nReturn;
}

//////////////////////////////////////////////////////////
// IUnknown
HRESULT CCustomBindStatusCallBack::QueryInterface(REFIID riid, void **ppvObject) 
{
	if (IsEqualIID(riid, IID_IUnknown)) {
		*ppvObject = (IUnknown*)(IBindStatusCallback*)this;
		AddRef();
		return S_OK;
	}
	if (IsEqualIID(riid, IID_IHttpNegotiate)){
		*ppvObject = (IHttpNegotiate*)this;
		AddRef();
		return S_OK;
	}
	return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CCustomBindStatusCallBack::AddRef()
{
	return ::InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) CCustomBindStatusCallBack::Release()
{
	::InterlockedDecrement(&m_cRef);
	if (m_cRef == 0) {
        delete this;
		return 0;
	}
	return m_cRef;
}

//////////////////////////////////////////////////////////
// IBindStatusCallback

// �o�C���h�J�n
HRESULT CCustomBindStatusCallBack::OnStartBinding( 
    /* [in] */ DWORD dwReserved,
	/* [in] */ IBinding *pib)
{
	m_spBinding = pib;

	if (m_spBSCBPrev) 
		m_spBSCBPrev->OnStopBinding(HTTP_STATUS_OK, NULL);
	return S_OK;
}

HRESULT CCustomBindStatusCallBack::OnProgress( 
    /* [in] */ ULONG ulProgress,
    /* [in] */ ULONG ulProgressMax,
    /* [in] */ ULONG ulStatusCode,
    /* [in] */ LPCWSTR szStatusText) 
{
	if (ulStatusCode == BINDSTATUS_BEGINDOWNLOADDATA) {
		m_pDLItem->strURL = szStatusText;
	}

	if (ulStatusCode & BINDSTATUS_DOWNLOADINGDATA) {
		m_pDLItem->nProgress = ulProgress;
		m_pDLItem->nProgressMax = ulProgressMax;
	}

	if (m_pDLItem->bAbort)
		Cancel();

	if (m_spBSCBPrev) {
		if (ulStatusCode == BINDSTATUS_CONTENTDISPOSITIONATTACH)
			return S_OK;
		m_spBSCBPrev->OnProgress(ulProgress, ulProgressMax, ulStatusCode, szStatusText);
	}
	return S_OK;
}
   
// �o�C���h�I��
HRESULT CCustomBindStatusCallBack::OnStopBinding( 
    /* [in] */ HRESULT hresult,
    /* [unique][in] */ LPCWSTR szError) 
{
	if (m_hFile != INVALID_HANDLE_VALUE) {
		::CloseHandle(m_hFile);
	}
	if (m_spBinding)
		m_spBinding.Release();

	if (m_spBSCBPrev && m_spBindCtx) {
		HRESULT hr = m_spBindCtx->RegisterObjectParam(L"_BSCB_Holder_", m_spBSCBPrev);
		m_spBSCBPrev.Release();
		m_spBindCtx.Release();
	}

	if (m_hWndNotify) {
		UINT uMsg = ::RegisterWindowMessage(REGISTERMESSAGE_DLCOMPLETE);
		::SendMessage(m_hWndNotify, uMsg, (WPARAM)(LPCTSTR)m_pDLItem->strText, 0);
	}

	/* DL���X�g����폜 */
	::PostMessage(m_hWndDLing, WM_USER_REMOVEFROMDOWNLIST, (WPARAM)m_pDLItem, (LPARAM)this);

	// ���Еt��
	if (::PathFileExists(m_pDLItem->strFilePath  + _T(".incomplete")))
		::DeleteFile(m_pDLItem->strFilePath  + _T(".incomplete"));

	return S_OK;
}
    
HRESULT CCustomBindStatusCallBack::GetBindInfo( 
    /* [out] */ DWORD *grfBINDF,
    /* [unique][out][in] */ BINDINFO *pbindinfo) 
{
	if (m_strDLFolder.IsEmpty()) {
		*grfBINDF = BINDF_ASYNCHRONOUS | BINDF_ASYNCSTORAGE | BINDF_GETNEWESTVERSION | BINDF_NOWRITECACHE | BINDF_PULLDATA;
	} else {
		*grfBINDF = BINDF_ASYNCHRONOUS | BINDF_GETNEWESTVERSION | BINDF_NOWRITECACHE;
	}
	return S_OK;
}

HRESULT CCustomBindStatusCallBack::OnDataAvailable( 
    /* [in] */ DWORD grfBSCF,
    /* [in] */ DWORD dwSize,
    /* [in] */ FORMATETC *pformatetc,
    /* [in] */ STGMEDIUM *pstgmed )
{ 
	HRESULT hr = S_OK;

	// Get the Stream passed
	if (BSCF_FIRSTDATANOTIFICATION & grfBSCF)  {
		if (m_spStream == NULL && pstgmed->tymed == TYMED_ISTREAM) {
			m_spStream = pstgmed->pstm;	

			// �t�@�C�������擾����
			if (_GetFileName() == false) {
				// �L�����Z�����ꂽ�̂ŋA��
				Cancel();
				return E_ABORT;
			}

			// �t�H���_�����݂��Ȃ���΍쐬
			CString strDir = MtlGetDirectoryPath(m_pDLItem->strFilePath);
			if (::PathIsDirectory(strDir) == FALSE)
				::SHCreateDirectory(NULL, strDir);

			m_hFile = ::CreateFile(m_pDLItem->strFilePath + _T(".incomplete"), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (m_hFile == INVALID_HANDLE_VALUE) {
				//ATLASSERT(FALSE);
				CString strError = _T("�t�@�C���̍쐬�Ɏ��s���܂���\n�ڍ�:");
				strError += GetLastErrorString();
				MessageBox(NULL, strError, NULL, MB_OK | MB_ICONWARNING);
				Cancel();
				return E_ABORT;
			}

			/* DL���X�g�ɒǉ� */
			::PostMessage(m_hWndDLing, WM_USER_ADDTODOWNLOADLIST, (WPARAM)m_pDLItem, 0);
		}
	}

	if (m_spStream && dwSize > m_dwTotalRead) {
		DWORD dwRead = dwSize - m_dwTotalRead; // �܂��ǂ܂�Ă��Ȃ��f�[�^��
		DWORD dwActuallyRead = 0;
		if (dwRead > 0) {
			do {
				BYTE* pBytes = NULL;
				ATLTRY(pBytes = new BYTE[dwRead + 1]);
				if (pBytes == NULL) {
					// ����grfBSCF��BSCF_LASTDATANOTIFICATION�Ȃ�
					// �A��O��m_spStream���J�����ׂ�
					hr = E_OUTOFMEMORY;
				} else {
					hr = m_spStream->Read(pBytes, dwRead, &dwActuallyRead);
					pBytes[dwActuallyRead] = 0;
					if (dwActuallyRead > 0) {
						DWORD dwWritten = 0;
						m_dwTotalRead += dwActuallyRead;
						::WriteFile(m_hFile, (LPCVOID)pBytes, dwActuallyRead, &dwWritten, NULL);
						ATLASSERT(dwWritten == dwActuallyRead);
					}
					delete[] pBytes;
				}

			//} while (!(hr == E_PENDING || hr == S_FALSE)/* && SUCCEEDED(hr)*/);
			} while (hr == S_OK);

		}
	}

	// Clean up
	if (BSCF_LASTDATANOTIFICATION & grfBSCF) {
		m_spStream.Release();
		if (m_hFile != INVALID_HANDLE_VALUE) {
			::CloseHandle(m_hFile);
			m_hFile = INVALID_HANDLE_VALUE;
			::MoveFileEx(m_pDLItem->strFilePath + _T(".incomplete"), m_pDLItem->strFilePath, MOVEFILE_REPLACE_EXISTING);
		}
	}
	return hr;
}


//////////////////////////////////////////////////
// IHttpNegotiate

// �ڑ�
HRESULT CCustomBindStatusCallBack::BeginningTransaction(      
	LPCWSTR szURL,
	LPCWSTR szHeaders,
	DWORD	dwReserved,
	LPWSTR *pszAdditionalHeaders ) 
{
	m_pDLItem->strURL	 = szURL;
	if (pszAdditionalHeaders == NULL) 
		return E_POINTER;
	*pszAdditionalHeaders = NULL;

	if (m_pDLItem->strReferer.IsEmpty() == FALSE) {	// ���t�@���[��ǉ�����
		CString strBuffer;
		strBuffer.Format(_T("Referer: %s\r\n"), m_pDLItem->strReferer);

		int nstrSize = (wcslen(strBuffer) + 1) * sizeof(WCHAR);
		LPWSTR wszAdditionalHeaders = (LPWSTR)CoTaskMemAlloc(nstrSize);
		if (wszAdditionalHeaders == NULL) return E_OUTOFMEMORY;

		wcscpy(wszAdditionalHeaders, (LPCTSTR)strBuffer);
		*pszAdditionalHeaders = wszAdditionalHeaders;
	}



	return S_OK;
}
	
// �ԓ�
HRESULT CCustomBindStatusCallBack::OnResponse(      
	DWORD dwResponseCode,
	LPCWSTR szResponseHeaders,
	LPCWSTR szRequestHeaders,
	LPWSTR *pszAdditionalRequestHeaders ) 
{
	std::wstring strRespons = szResponseHeaders;
	{	// �t�@�C�������擾
		std::wregex regex(L"Content-Disposition:.*?; filename=\"(.*?)\"");
		std::wsmatch	smatch;
		if (std::regex_search(strRespons, smatch, regex)) {
			CString strBuffer = smatch[1].str().c_str();
#if 0
			// WideChar(UTF-8)�ُ� -> MultiByte(UTF-8)����
			int nMultiSize = ::WideCharToMultiByte(CP_OEMCP, 0, strBuffer, -1, 0, 0, NULL, NULL);
			std::unique_ptr<char> strMultiByte(new char[nMultiSize + 1]);
			::WideCharToMultiByte(CP_OEMCP, 0, strBuffer, -1, strMultiByte.get(), nMultiSize, NULL, NULL);

			UINT nCodePage = CP_UTF8;
			CComPtr<IMultiLanguage2>	spMultiLang;
			HRESULT hr = spMultiLang.CoCreateInstance(CLSID_CMultiLanguage);
			if (SUCCEEDED(hr)) {
				int Scores = 5; // �擾������̐�
				DetectEncodingInfo Encoding[5] = { 0 };
				hr = spMultiLang->DetectInputCodepage(MLDETECTCP_NONE, 0, strMultiByte.get(), &nMultiSize, Encoding, &Scores);
				if (SUCCEEDED(hr)) {
					if (Encoding[0].nCodePage == 932) {	// Shift-JIS
						nCodePage = CP_OEMCP;
					}
				}
			}
#endif
			vector<char> filename = Misc::urlstr_decode(strBuffer);
			m_pDLItem->strFileName = Misc::UnknownToCString(filename);
#if 0
			// MultiByte(UTF-8)���� -> WideChar(UTF-16)(wchar_t)����
			int nWideSize = ::MultiByteToWideChar(nCodePage, 0, strMultiByte.get(), -1, 0, 0);
			std::unique_ptr<WCHAR> strWideChar(new WCHAR[nWideSize + 1]);
			::MultiByteToWideChar(nCodePage, 0, strMultiByte.get(), -1, strWideChar.get(), nWideSize);

			m_pDLItem->strFileName = strWideChar.get();
#endif
		} else {
			m_pDLItem->strFileName = m_pDLItem->strURL;
			m_pDLItem->strFileName = Misc::GetFileBaseName(m_pDLItem->strFileName);
			int nQuestion = m_pDLItem->strFileName.ReverseFind(_T('?'));
			if (nQuestion != -1) {
				m_pDLItem->strFileName = m_pDLItem->strFileName.Left(nQuestion);
			}
		}
	}
	return S_OK;
}


bool	CCustomBindStatusCallBack::_GetFileName()
{
	CComQIPtr<IWinInetHttpInfo> spInfo = m_spBinding;
	if (spInfo) {
		char buff[1024];
		DWORD dwBuffSize = 1024;
		HRESULT hr = spInfo->QueryOption(INTERNET_OPTION_DATAFILE_NAME, (LPVOID)buff, &dwBuffSize);
		if (hr == S_OK) {
			// �t�@�C�����̕������擾����([?]����������)
			CString strBaseName = Misc::GetFileBaseName(buff);
			int nIndex = strBaseName.ReverseFind(_T('['));
			if (nIndex != -1) {
				m_pDLItem->strFileName = strBaseName.Left(nIndex);
				CString strExt = Misc::GetFileExt(strBaseName);
				if (strExt.IsEmpty() == FALSE) {
					m_pDLItem->strFileName += _T('.') + strExt;
				}
			} else {
				m_pDLItem->strFileName = strBaseName;
			}
		}
	} else {
		ATLASSERT(m_strDLFolder.IsEmpty() == FALSE);	// ����ȊO�Ŏ��s����ƍ���
		m_pDLItem->strFileName = Misc::GetFileBaseName(m_pDLItem->strURL);	// [?]��������
	}
	// URL�f�R�[�h
	if (m_pDLItem->strFileName.Find(_T('%')) != -1) {
		vector<char> filename = Misc::urlstr_decode(m_pDLItem->strFileName);
		m_pDLItem->strFileName = Misc::UnknownToCString(filename);
	}
	//m_pDLItem->strFileName.Replace(_T("%20"), _T(" "));

	// �����N���o�_�C�A���O���
	if (m_strDLFolder.IsEmpty() == FALSE) {
		m_pDLItem->strFilePath = m_strDLFolder + m_pDLItem->strFileName;
		if (::PathFileExists(m_pDLItem->strFilePath)) {
			if (m_dwDLOption & DLO_OVERWRITEPROMPT) {
				CString strMessage = _T("�㏑�����܂����H\n") + m_pDLItem->strFilePath;
				if (MessageBox(m_hWndNotify, strMessage, _T("�m�F"), MB_OKCANCEL | MB_ICONWARNING) == IDCANCEL) {
					Cancel();
					return false;
				}
			} else if (m_dwDLOption & DLO_USEUNIQUENUMBER) {
				int nCount = 0;
				CString strOriginalFileName = m_pDLItem->strFileName;
				while (TRUE) {
					CString strAppend;
					strAppend.Format(_T("_[%d]"), nCount);
					int nExt = m_pDLItem->strFileName.Find(_T('.'));
					if (nExt != -1) {
						m_pDLItem->strFileName.Insert(nExt, strAppend);
					} else {
						m_pDLItem->strFileName += strAppend;
					}
					m_pDLItem->strFilePath = m_strDLFolder + m_pDLItem->strFileName;
					if (::PathFileExists(m_pDLItem->strFilePath) == FALSE)
						break;

					m_pDLItem->strFileName = strOriginalFileName;
					++nCount;
				}
			}
		}
		return true;
	}
	// ���O��t���ĕۑ��_�C�A���O���o��
	if (CDLOptions::bUseSaveFileDialog || ::GetKeyState(VK_CONTROL) < 0) {
		COMDLG_FILTERSPEC filter[] = {
			{ L"�e�L�X�g���� (*.txt)", L"*.txt" },// �_�~�[
			{ L"���ׂẴt�@�C��", L"*.*" }
		};
		CString strExt = Misc::GetFileExt(m_pDLItem->strFileName);
		filter[0].pszName	= strExt.GetBuffer(0);
		CString strSpec = _T("*.") + strExt;
		filter[0].pszSpec	= strSpec.GetBuffer(0);

		CShellFileSaveDialog	ShellSaveFileDialog(m_pDLItem->strFileName, FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST | FOS_OVERWRITEPROMPT, strExt, filter, 2);
		if (ShellSaveFileDialog.IsNull()  == false) {
			if (ShellSaveFileDialog.DoModal(NULL) == IDOK) {		
				ShellSaveFileDialog.GetFileTitle(m_pDLItem->strFileName);
				ShellSaveFileDialog.GetFilePath(m_pDLItem->strFilePath);
			} else {
				// �L�����Z��
				Cancel();
				return false;
			}
		} else {
			CFileDialog dlg(FALSE, NULL, m_pDLItem->strFileName, OFN_OVERWRITEPROMPT, _T("���ׂẴt�@�C�� (*.*)\0*.*\0\0"));
			if (dlg.DoModal(NULL) == IDOK) {
				m_pDLItem->strFileName = dlg.m_szFileTitle;
				if (Misc::GetFileExt(m_pDLItem->strFileName).IsEmpty())
					m_pDLItem->strFileName += _T(".") + strExt;
				m_pDLItem->strFilePath = dlg.m_szFileName;
				if (Misc::GetFileExt(m_pDLItem->strFilePath).IsEmpty())
					m_pDLItem->strFilePath += _T(".") + strExt;
			} else {
				// �L�����Z��
				Cancel();
				return false;
			}
		}
	} else {
		m_pDLItem->strFilePath = CDLOptions::strDLFolderPath + m_pDLItem->strFileName;
		if (::PathFileExists(m_pDLItem->strFilePath)) {
			CString strMessage = _T("�㏑�����܂����H\n") + m_pDLItem->strFilePath;
			if (MessageBox(NULL, strMessage, _T("�m�F"), MB_OKCANCEL | MB_ICONWARNING) == IDCANCEL)
				return false;
		}
	}
	return true;
}















