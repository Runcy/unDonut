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
CCustomBindStatusCallBack::CCustomBindStatusCallBack(DLItem* pItem, HWND hWndDLing, LPCTSTR defaultDLFolder)
	: m_dwTotalRead(0)
	, m_hFile(INVALID_HANDLE_VALUE)
	, m_pDLItem(pItem)
	, m_hWndDLing(hWndDLing) 
	, m_strDefaultDLFolder(defaultDLFolder)
	, m_cRef(1)
	, m_hWndNotify(NULL)
{	}

// Destructor
CCustomBindStatusCallBack::~CCustomBindStatusCallBack()
{
}


void	CCustomBindStatusCallBack::SetThreadId(DWORD dwID)
{ 
	m_pDLItem->dwThreadId = dwID;
}

//------------------------------
/// ���t�@����ݒ�
void	CCustomBindStatusCallBack::SetReferer(LPCTSTR strReferer)
{
	::wcscpy_s(m_pDLItem->strReferer, strReferer);
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
		::wcscpy_s(m_pDLItem->strURL, szStatusText);
		WCHAR strDomain[INTERNET_MAX_URL_LENGTH];
		DWORD cchResult = INTERNET_MAX_URL_LENGTH;
		if (::CoInternetParseUrl(m_pDLItem->strURL, PARSE_DOMAIN, 0, strDomain, INTERNET_MAX_URL_LENGTH, &cchResult, 0) == S_OK) {
			::wcscpy_s(m_pDLItem->strDomain, strDomain);
		}
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
	if (m_spBinding)
		m_spBinding.Release();

	if (m_spBSCBPrev && m_spBindCtx) {
		HRESULT hr = m_spBindCtx->RegisterObjectParam(L"_BSCB_Holder_", m_spBSCBPrev);
		m_spBSCBPrev.Release();
		m_spBindCtx.Release();
	}

	/* �O����DL�I���ʒm */
	if (m_hWndNotify) {
		UINT uMsg = ::RegisterWindowMessage(REGISTERMESSAGE_DLCOMPLETE);
		::SendMessage(m_hWndNotify, uMsg, 0, 0);
	}

	// ���Еt��
	if (m_hFile != INVALID_HANDLE_VALUE) {
		::CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;

		if (m_pDLItem->nProgress != m_pDLItem->nProgressMax && m_pDLItem->nProgressMax != 0) {
			m_pDLItem->bAbort = true;
			if (::PathFileExists(m_pDLItem->strIncompleteFilePath)) {
				::DeleteFile(m_pDLItem->strIncompleteFilePath);
				::SHChangeNotify(SHCNE_DELETE, SHCNF_PATH, m_pDLItem->strIncompleteFilePath, nullptr);
				TRACEIN(_T("�s���S�t�@�C�����폜���܂����B: %s"), m_pDLItem->strIncompleteFilePath);
			}
		} else {
			TRACEIN(_T("OnStopBinding() : ����I�����܂���(%s)"), m_pDLItem->strFileName);
			::MoveFileEx(m_pDLItem->strIncompleteFilePath, m_pDLItem->strFilePath, MOVEFILE_REPLACE_EXISTING);
			/* �G�N�X�v���[���[�Ƀt�@�C���̕ύX�ʒm */
			::SHChangeNotify(SHCNE_RENAMEITEM, SHCNF_PATH, m_pDLItem->strIncompleteFilePath, m_pDLItem->strFilePath);
		}
	}

	/* DL���X�g����폜 */
	::SendMessage(m_hWndDLing, WM_USER_REMOVEFROMDOWNLIST, (WPARAM)m_pDLItem->unique, 0);

	HANDLE hMap = m_pDLItem->hMapForClose;
	::UnmapViewOfFile(static_cast<LPVOID>(m_pDLItem));
	::CloseHandle(hMap);

	return S_OK;
}

HRESULT CCustomBindStatusCallBack::GetBindInfo( 
    /* [out] */ DWORD *grfBINDF,
    /* [unique][out][in] */ BINDINFO *pbindinfo) 
{
	if (m_strDLFolder.IsEmpty()) {
		*grfBINDF = BINDF_ASYNCHRONOUS | /*BINDF_ASYNCSTORAGE | */BINDF_GETNEWESTVERSION | BINDF_NOWRITECACHE | /*BINDF_PULLDATA | */BINDF_PRAGMA_NO_CACHE | BINDF_FROMURLMON;
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
				//if (m_pDLItem->dwThreadId)
				//	::PostThreadMessage(m_pDLItem->dwThreadId, WM_DECREMENTTHREADREFCOUNT, 0, 0);
				return E_ABORT;
			}

			// �t�H���_�����݂��Ȃ���΍쐬
			CString strDir = MtlGetDirectoryPath(m_pDLItem->strFilePath);
			if (::PathIsDirectory(strDir) == FALSE)
				::SHCreateDirectory(NULL, strDir);

			m_hFile = ::CreateFile(m_pDLItem->strIncompleteFilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (m_hFile == INVALID_HANDLE_VALUE) {
				//ATLASSERT(FALSE);
				CString strError = _T("�t�@�C���̍쐬�Ɏ��s���܂���\n");
				strError += GetLastErrorString();
				MessageBox(NULL, strError, NULL, MB_OK | MB_ICONWARNING);
				Cancel();
				return E_ABORT;
			}

			/* DL���X�g�ɒǉ� */
			::SendMessage(m_hWndDLing, WM_USER_ADDTODOWNLOADLIST, (WPARAM)m_pDLItem->unique, 0);
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

			if (m_pDLItem->nProgress != m_pDLItem->nProgressMax && m_pDLItem->nProgressMax != 0) {
				m_pDLItem->bAbort = true;
				::DeleteFile(m_pDLItem->strIncompleteFilePath);
				::SHChangeNotify(SHCNE_DELETE, SHCNF_PATH, m_pDLItem->strIncompleteFilePath, nullptr);
				TRACEIN(_T("BSCF_LASTDATANOTIFICATION (%s): �T�C�Y����v���܂���I"), m_pDLItem->strFileName);
			} else {
				TRACEIN(_T("BSCF_LASTDATANOTIFICATION : ����I�����܂���(%s)"), m_pDLItem->strFileName);
				::MoveFileEx(m_pDLItem->strIncompleteFilePath, m_pDLItem->strFilePath, MOVEFILE_REPLACE_EXISTING);
				/* �G�N�X�v���[���[�Ƀt�@�C���̕ύX�ʒm */
				::SHChangeNotify(SHCNE_RENAMEITEM, SHCNF_PATH, m_pDLItem->strIncompleteFilePath, m_pDLItem->strFilePath);
			}
		}
	}
	return S_OK;
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
	TRACEIN(_T("CCustomBindStatusCallBack::BeginningTransaction()"));
	::wcscpy_s(m_pDLItem->strURL, szURL);
	if (pszAdditionalHeaders == NULL) 
		return E_POINTER;
	*pszAdditionalHeaders = NULL;

	if (m_pDLItem->strReferer[0] != L'\0') {	// ���t�@���[��ǉ�����
		CString strBuffer;
		strBuffer.Format(_T("Referer: %s\r\n"), m_pDLItem->strReferer);
		TRACEIN(_T("�@Referer : %s"), m_pDLItem->strReferer);

		int nstrSize = (strBuffer.GetLength() + 1) * sizeof(WCHAR);
		LPWSTR wszAdditionalHeaders = (LPWSTR)CoTaskMemAlloc(nstrSize);
		if (wszAdditionalHeaders == NULL) 
			return E_OUTOFMEMORY;

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
	TRACEIN(_T("CCustomBindStatusCallBack::OnResponse()"));
	std::wstring strRespons = szResponseHeaders;
	{	// �t�@�C�������擾
		std::wregex regex(L"Content-Disposition:.*?; filename=\"(.*?)\"");
		std::wsmatch	smatch;
		if (std::regex_search(strRespons, smatch, regex)) {
			CString strBuffer = smatch[1].str().c_str();
			vector<char> filename = Misc::urlstr_decode(strBuffer);
			::wcscpy_s(m_pDLItem->strFileName, Misc::UnknownToCString(filename));
			TRACEIN(_T("�@filename : %s"), (LPCTSTR)m_pDLItem->strFileName);
		} else {
			::wcscpy_s(m_pDLItem->strFileName, Misc::GetFileBaseName(m_pDLItem->strFileName));
			CString temp = m_pDLItem->strFileName;
			int nQuestion = temp.ReverseFind(_T('?'));
			if (nQuestion != -1) {
				::wcscpy_s(m_pDLItem->strFileName, temp.Left(nQuestion));
			}
		}
	}
	
	return S_OK;
}


bool	CCustomBindStatusCallBack::_GetFileName()
{
	HRESULT hr = S_OK;
	CComQIPtr<IWinInetHttpInfo> spInfo = m_spBinding;
	if (spInfo) {
		char buff[1024];
		DWORD dwBuffSize = 1024;
		hr = spInfo->QueryInfo(HTTP_QUERY_CONTENT_DISPOSITION, (LPVOID)buff, &dwBuffSize, 0, NULL);
		if (hr == S_OK) {	// CONTENT_DISPOSITION����t�@�C�������擾����
			std::string	strbuff = buff;
			std::regex rx("filename\\*=(?: |)UTF-8''(.+)");
			std::regex rx1("filename=(?:\"|)([^\";]+)");
			std::smatch result;
			if (std::regex_search(strbuff, result, rx)) {
				CString strtemp = result.str(1).c_str();
				vector<char> strurldecoded = Misc::urlstr_decode(strtemp);
				::wcscpy_s(m_pDLItem->strFileName, Misc::utf8_to_CString(strurldecoded));	//
			} else if (std::regex_search(strbuff, result, rx1))  {
				std::string strtemp1 = result.str(1);

				/* URL�f�R�[�h���� */
				vector<char> strUrlDecoded;
				char tempdecoded[INTERNET_MAX_PATH_LENGTH] = "\0";
				DWORD dwBufferLength = INTERNET_MAX_PATH_LENGTH;
				hr = ::UrlUnescapeA(const_cast<LPSTR>(strtemp1.c_str()), tempdecoded, &dwBufferLength, 0);
				if (FAILED(hr)) {
					TRACEIN(_T("UrlUnescapeA ���s : Error�u%s�v\n�@(%s)"), (LPCTSTR)GetLastErrorString(hr), (LPCTSTR)CString(strtemp1.c_str()));
					::strcpy_s(tempdecoded, strtemp1.c_str());	// ���s�����̂Ō��ɖ߂��Ă���
				} else {
					strtemp1 = tempdecoded;
					for (auto it = strtemp1.begin(); it != strtemp1.end(); ++it) {	// '+'���󔒂ɒu������
						if (*it == '+')
							*it = ' ';
					}
				} 

				if (Misc::IsShiftJIS(strtemp1.c_str(), (int)strtemp1.length())) {
					::wcscpy_s(m_pDLItem->strFileName, Misc::sjis_to_CString(strtemp1.c_str()));
					TRACEIN(_T("_GetFileName(), Shift-JIS������ł��� : %s"), m_pDLItem->strFileName);
				} else {
					strUrlDecoded.resize(strtemp1.length() + 1, '\0');
					::strcpy_s(strUrlDecoded.data(), strtemp1.length() + 1, strtemp1.c_str());
					::wcscpy_s(m_pDLItem->strFileName, Misc::utf8_to_CString(strUrlDecoded));
					TRACEIN(_T("_GetFileName(), UTF8�Ɖ��߂��܂��� : %s"), m_pDLItem->strFileName);
				}
			} else {
				TRACEIN(_T("CONTENT_DISPOSITION��������Ȃ��H ���e: %s"), (LPCTSTR)CString(buff));
			}
		}
#if 0
		if (m_pDLItem->strFileName[0] == L'\0') {
			dwBuffSize = 1024;
			hr = spInfo->QueryOption(INTERNET_OPTION_DATAFILE_NAME, (LPVOID)buff, &dwBuffSize);
			if (hr == S_OK) {
				// �t�@�C�����̕������擾����([?]����������)
				CString strBaseName = Misc::GetFileBaseName(buff);
				int nIndex = strBaseName.ReverseFind(_T('['));
				if (nIndex != -1) {
					::wcscpy_s(m_pDLItem->strFileName, strBaseName.Left(nIndex));
					CString strExt = _T('.') + Misc::GetFileExt(strBaseName);
					if (strExt.IsEmpty() == FALSE) {
						::wcscat_s(m_pDLItem->strFileName, strExt);
					}
				} else {
					::wcscpy_s(m_pDLItem->strFileName, strBaseName);
				}
			}
		}
#endif
	} 

	if (m_pDLItem->strFileName[0] == L'\0') {
		//ATLASSERT(m_strDLFolder.IsEmpty() == FALSE);	// ����ȊO�Ŏ��s����ƍ���
		::wcscpy_s(m_pDLItem->strFileName, Misc::GetFileBaseName(m_pDLItem->strURL));	// [?]��������
		CString temp = m_pDLItem->strFileName;
		int nQIndex = temp.ReverseFind(_T('?'));
		if (nQIndex != -1) {
			::wcscpy_s(m_pDLItem->strFileName, temp.Left(nQIndex));	// "?"����E�͖�������
		}
		if (m_pDLItem->strFileName == L'\0')
			::wcscpy_s(m_pDLItem->strFileName, _T("index"));	// �߂����ɂȂ��Ǝv�����ǈꉞ

		temp = m_pDLItem->strFileName;
		temp.Replace(_T("%20"), _T(" "));
		if (temp.Find(_T('%')) != -1) {	// URL�f�R�[�h����
			//vector<char> filename = Misc::urlstr_decode(m_pDLItem->strFileName);
			::wcscpy_s(m_pDLItem->strFileName, Misc::urlstr_decodeJpn(m_pDLItem->strFileName, 3));//Misc::UnknownToCString(filename);
		} else {
			::wcscpy_s(m_pDLItem->strFileName, temp);
		}
	}

	// �����N���o�_�C�A���O���(�摜��ۑ���)
	if (m_strDLFolder.IsEmpty() == FALSE) {
		::wcscpy_s(m_pDLItem->strFilePath, m_strDLFolder + m_pDLItem->strFileName);
		if (::PathFileExists(m_pDLItem->strFilePath)) {
			if (m_dwDLOption & DLO_OVERWRITEPROMPT) {
				CString strMessage;
				strMessage.Format(_T("%s �͊��ɑ��݂��܂��B\n�㏑�����܂����H\n"), (LPCTSTR)m_pDLItem->strFileName);;
				if (MessageBox(NULL, strMessage, _T("�m�F"), MB_OKCANCEL | MB_ICONWARNING) == IDCANCEL) {
					return false;
				}
			} else if (m_dwDLOption & DLO_USEUNIQUENUMBER) {	// �A�Ԃ�t����
				int nCount = 0;
				CString strOriginalFileName = m_pDLItem->strFileName;
				while (TRUE) {
					CString strAppend;
					strAppend.Format(_T("_[%d]"), nCount);
					CString temp = m_pDLItem->strFileName;
					int nExt = temp.Find(_T('.'));
					if (nExt != -1) {
						temp.Insert(nExt, strAppend);
					} else {
						temp += strAppend;
					}
					::wcscpy_s(m_pDLItem->strFileName, temp);
					::wcscpy_s(m_pDLItem->strFilePath, m_strDLFolder + m_pDLItem->strFileName);
					if (::PathFileExists(m_pDLItem->strFilePath) == FALSE)
						break;

					::wcscpy_s(m_pDLItem->strFileName, strOriginalFileName);
					++nCount;
				}
			}
		}
		::swprintf_s(m_pDLItem->strIncompleteFilePath, L"%s.incomplete", m_pDLItem->strFilePath);
		return true;
	}
	// ���O��t���ĕۑ��_�C�A���O���o��
	if (::SendMessage(m_hWndDLing, WM_USER_USESAVEFILEDIALOG, 0, 0) != 0 || ::GetKeyState(VK_MENU) < 0) {
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
				ShellSaveFileDialog.GetFileTitle(m_pDLItem->strFileName, MAX_PATH);
				ShellSaveFileDialog.GetFilePath(m_pDLItem->strFilePath, MAX_PATH);
			} else {
				return false;
			}
		} else {
			CFileDialog dlg(FALSE, NULL, m_pDLItem->strFileName, OFN_OVERWRITEPROMPT, _T("���ׂẴt�@�C�� (*.*)\0*.*\0\0"));
			if (dlg.DoModal(NULL) == IDOK) {
				::wcscpy_s(m_pDLItem->strFileName, dlg.m_szFileTitle);
				strExt.Insert(0, _T('.'));
				if (Misc::GetFileExt(m_pDLItem->strFileName).IsEmpty())
					::wcscat_s(m_pDLItem->strFileName, strExt);

				::wcscpy_s(m_pDLItem->strFilePath, dlg.m_szFileName);
				if (Misc::GetFileExt(m_pDLItem->strFilePath).IsEmpty())
					::wcscat_s(m_pDLItem->strFilePath, strExt);
			} else {
				return false;
			}
		}
	} else {
		ATLASSERT( m_pDLItem->strFileName[0] != L'\0' );
		::swprintf_s(m_pDLItem->strFilePath, _T("%s%s"), m_strDefaultDLFolder, m_pDLItem->strFileName);

		COPYDATASTRUCT cds;
		cds.dwData	= kDownloadingFileExists;
		cds.lpData	= static_cast<LPVOID>(m_pDLItem->strFilePath);
		cds.cbData	= (::lstrlen(m_pDLItem->strFilePath) + 1) * sizeof(WCHAR);
		bool bExistsFileDownloading = ::SendMessage(m_hWndDLing, WM_COPYDATA, m_pDLItem->unique, (LPARAM)&cds) != 0;
		if (bExistsFileDownloading && (m_dwDLOption & DLO_OVERWRITEPROMPT)) {
			// �ۑ��悪���Ԃ�����c�k�ł��Ȃ��̂�
			CString err;
			err.Format(_T("�u%s�v\n�ۑ��悪���Ԃ��Ă���̂Ń_�E�����[�h�ł��܂���B\n�_�E�����[�h�̓L�����Z������܂�"), m_pDLItem->strFilePath);
			MessageBox(NULL, err, NULL, MB_ICONERROR);
			return false;
		}
		if (::PathFileExists(m_pDLItem->strFilePath)) {
			if (m_dwDLOption & DLO_OVERWRITEPROMPT) {
				CString strMessage;
				strMessage.Format(_T("%s �͊��ɑ��݂��܂��B\n�㏑�����܂����H\n"), (LPCTSTR)m_pDLItem->strFileName);
				if (MessageBox(m_hWndDLing, strMessage, _T("�m�F"), MB_OKCANCEL | MB_ICONWARNING) == IDCANCEL) {
					return false;
				}
			} else if (m_dwDLOption & DLO_USEUNIQUENUMBER) {
				int nCount = 1;
				CString strOriginalFileName = m_pDLItem->strFileName;
				for (;;) {
					CString strAppend;
					strAppend.Format(_T("(%d)"), nCount);
					CString temp = m_pDLItem->strFileName;
					int nExt = temp.Find(_T('.'));
					if (nExt != -1) {
						temp.Insert(nExt, strAppend);
					} else {
						temp += strAppend;
					}
					::wcscpy_s(m_pDLItem->strFileName, temp);
					::wcscpy_s(m_pDLItem->strFilePath, m_strDefaultDLFolder + m_pDLItem->strFileName);
					cds.lpData	= static_cast<LPVOID>(m_pDLItem->strFilePath);
					cds.cbData	= (::lstrlen(m_pDLItem->strFilePath) + 1) * sizeof(WCHAR);
					if (::PathFileExists(m_pDLItem->strFilePath) == FALSE && 
						::SendMessage(m_hWndDLing, WM_COPYDATA, m_pDLItem->unique, (LPARAM)&cds) == 0)
						break;

					::wcscpy_s(m_pDLItem->strFileName, strOriginalFileName);
					++nCount;
				}
			}
		}
	}
	::swprintf_s(m_pDLItem->strIncompleteFilePath, _T("%s.incomplete"), m_pDLItem->strFilePath);
	
	// ���Ƀ_�E�����[�h���̃t�@�C���̕ۑ���Ƃ��Ԃ邩�ǂ���
	if (::SendMessage(m_hWndDLing, WM_USER_ISDOUBLEDOWNLOADING, static_cast<WPARAM>(m_pDLItem->unique), 0)) {
		MessageBox(m_hWndDLing, 
				   _T("�_�E�����[�h���̑��̃t�@�C�������݂��܂��B\n�_�E�����[�h�̓L�����Z������܂��B"), 
				   _T("�G���["), 
				   MB_OK | MB_ICONERROR);
		return false;
	}
	return true;
}















