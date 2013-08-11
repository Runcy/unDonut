/**
*	@file	AutoLogin.cpp
*	@brief	���O�C�������Ǘ����܂�
*/

#include "stdafx.h"
#include "AutoLogin.h"
#include <fstream>
#include <regex>
#include "Misc.h"
#include "DonutPFunc.h"
#include "option\MainOption.h"
#include "GlobalConfig.h"

using namespace std;

#pragma comment (lib, "crypt32.lib")


/////////////////////////////////////////////
// CLoginDataManager

#define AUTOLOGINVERSION	_T("3.1")
#define AUTOLOGINSHAREDMEMNAME	_T("DonutAutoLoginSharedMemName")

// ��`
LoginInfoData	CLoginDataManager::s_loginInfoData;
CSharedMemory	CLoginDataManager::s_sharedMem;


// for MainFrame
void CLoginDataManager::CreateOriginalLoginDataList(HWND hWndMainFrame)
{
	Load();

	s_sharedMem.CloseHandle();

	CString sharedMemName;
	sharedMemName.Format(_T("%s%#x"), AUTOLOGINSHAREDMEMNAME, hWndMainFrame);
	s_sharedMem.Serialize(s_loginInfoData, sharedMemName);

	CSharedDataChangeNotify::NotifyObserver(ObserverClass::kLoginDataManager);
}

// for ChildFrame
void CLoginDataManager::UpdateLoginDataList(HWND hWndMainFrame)
{
	CString sharedMemName;
	sharedMemName.Format(_T("%s%#x"), AUTOLOGINSHAREDMEMNAME, hWndMainFrame);
	CSharedMemory sharedMem;
	sharedMem.Deserialize(s_loginInfoData, sharedMemName);
}

bool CLoginDataManager::DoAutoLogin(int nIndex, IWebBrowser2* pBrowser)
{
	ATLASSERT( nIndex != -1 );

	NameValueMap*	pmap;
	CLoginDataManager::GetNameValueMap(nIndex, pmap);
	CheckboxMap*	pmapCheck;
	CLoginDataManager::GetCheckboxMap(nIndex, pmapCheck);

	CComQIPtr<IHTMLDocument2>	spDoc2;
	CComQIPtr<IWebBrowser2>	spBrowser = pBrowser;
	CComPtr<IDispatch>	spDisp;
	spBrowser->get_Document(&spDisp);
	spDoc2 = spDisp;
	if (spDoc2) {
		bool bSuccess = false;
		CComPtr<IHTMLElementCollection>	spCol;
		spDoc2->get_forms(&spCol);
		ForEachHtmlElement(spCol, [=, &pmap, &pmapCheck, &bSuccess](IDispatch* pDisp) -> bool {
			NameValueMap& map2		= *pmap;
			CheckboxMap&  mapCheck  = *pmapCheck;
			CComQIPtr<IHTMLFormElement>	spForm = pDisp;
			if (spForm.p) {
				int nFoundCount = 0;
				CComQIPtr<IHTMLElement> spSubmit;
				ForEachHtmlElement(spForm, [=, &map2, &mapCheck, &spSubmit, &nFoundCount](IDispatch* pDisp) -> bool {
					CComQIPtr<IHTMLInputElement>	spInput = pDisp;
					if (spInput.p) {
						CComBSTR	strType;
						spInput->get_type(&strType);
						if (strType == nullptr)
							return true;
						strType.ToLower();
						if (strType == L"text" || strType == L"email" || strType == L"password") {
							CComBSTR	strName;
							spInput->get_name(&strName);
							if (strName.m_str) {
								auto it = map2.find(WTL::CString(strName));
								if (it != map2.end()) {
									++nFoundCount;	// ��������
									spInput->put_value(CComBSTR(it->second));
								}
							}						
						} else if (strType == L"checkbox") {
							CComBSTR	strName;
							spInput->get_name(&strName);
							if (strName.m_str) {
								auto it = mapCheck.find(WTL::CString(strName));
								if (it != mapCheck.end()) {
									++nFoundCount;
									spInput->put_checked(it->second);
								}
							}
						} else if (strType == L"submit") {
							spSubmit = spInput;
						}

					}
					return true;
				});
				if (nFoundCount == static_cast<int>(map2.size() + mapCheck.size())) {
					if (s_loginInfoData.vecLoginInfo[nIndex].bAutoFillOnly == false) {
						// ��v�����̂Ŏ������O�C������
						if (spSubmit.p) {
							spSubmit->click();
						} else {
							spForm->submit();
							//this->SetTimer(FlashIconTimerID, 1000);
						}
					}
					bSuccess = true;
					return false;
				}

			}
			return true;
		});
		return bSuccess;
	}
	return false;
}

//------------------------
/// �Í����������̂�ǂݍ���
void	CLoginDataManager::Load()
{
	CString LoginDataPath = GetConfigFilePath(_T("AutoLoginData"));

	s_loginInfoData.vecLoginInfo.reserve(20);

	if (::PathFileExists(LoginDataPath) == FALSE)
		return ;

	/* �Í������ꂽ�t�@�C���̓ǂݍ��� */
	vector<char>	data;
	if (_ReadData(LoginDataPath, data) == false)
		return;
	
	/* ������ */
	DATA_BLOB	DataOut;
	DATA_BLOB	DataIn;
	DataIn.cbData	= data.size();
	DataIn.pbData	= (BYTE*)data.data();
	if (!CryptUnprotectData(&DataIn, NULL, NULL, NULL, NULL, 0, &DataOut)) {
		MessageBox(NULL, _T("�f�[�^�̕������Ɏ��s"), NULL, 0);
		// �������Ɏ��s�����t�@�C�������l�[������
		CString strRenamedFilePath = LoginDataPath + _T("_Unprotectfaild");
		MoveFileEx(LoginDataPath, strRenamedFilePath, MOVEFILE_REPLACE_EXISTING);
		return ;
	}
	std::wstring strData = (LPCWSTR)DataOut.pbData;
	LocalFree(DataOut.pbData);

	/* s_loginInfoData.vecLoginInfo �ɐݒ� */
	_DeSerializeLoginData(strData);
}


//-----------------------
/// �Í������ĕۑ�����
void	CLoginDataManager::Save()
{
	/* s_loginInfoData.vecLoginInfo �𒼗� */
	std::wstring strData;
	_SerializeLoginData(strData);

	/* �Í��� */
	DATA_BLOB	DataOut;
	DATA_BLOB	DataIn;
	DataIn.cbData = (strData.length() * sizeof(TCHAR)) + sizeof(TCHAR);
	DataIn.pbData = (BYTE*)strData.c_str();
	if (!CryptProtectData(&DataIn, L"unDonutAutoLoginData", NULL, NULL, NULL, 0, &DataOut)) {
		MessageBox(NULL, _T("�f�[�^�̈Í����Ɏ��s���܂���"), NULL, 0);
		return ;
	}

	/* �Í��������f�[�^���t�@�C���ɏ������� */
	_WriteData(GetConfigFilePath(_T("AutoLoginData")), DataOut.pbData, DataOut.cbData);
}

//-----------------------
void	CLoginDataManager::Import(LPCTSTR strPath)
{
	vector<char> data;
	if (_ReadData(strPath, data)) {
		/* s_loginInfoData.vecLoginInfo �ɐݒ� */
		std::wstring strData = (LPCWSTR)data.data();
		_DeSerializeLoginData(strData);
	}
}

//-----------------------
void	CLoginDataManager::Export(LPCTSTR strPath)
{
	/* s_loginInfoData.vecLoginInfo �𒼗� */
	std::wstring strData;
	_SerializeLoginData(strData);

	/* �Í��������f�[�^���t�@�C���ɏ������� */
	_WriteData(strPath, (BYTE*)strData.c_str(), (strData.length() * sizeof(WCHAR)) + sizeof(WCHAR));
}


//-------------------------------------
/// �}�b�`���郍�O�C��URL�̃C���f�b�N�X��Ԃ�
int		CLoginDataManager::Find(LPCTSTR strUrl)
{
	if (s_loginInfoData.bEnableAutoLogin == false)
		return -1;

	CString strLoginUrl = strUrl;
	strLoginUrl.MakeLower();

	int nCount = s_loginInfoData.vecLoginInfo.size();
	for (int i = 0; i < nCount; ++i) {
		if (strLoginUrl.GetLength() < s_loginInfoData.vecLoginInfo[i].strLoginUrl.GetLength())
			continue;
		if (strLoginUrl.Left(s_loginInfoData.vecLoginInfo[i].strLoginUrl.GetLength()) == s_loginInfoData.vecLoginInfo[i].strLoginUrl)
			return i;
	}
	
	return -1;
}

//----------------------------------
void	CLoginDataManager::GetNameValueMap(int nIndex, NameValueMap*& pmap)
{
	pmap = &s_loginInfoData.vecLoginInfo[nIndex].mapNameValue;
}

//---------------------------------
void	CLoginDataManager::GetCheckboxMap(int nIndex, CheckboxMap*& pmapCheckbox)
{
	pmapCheckbox = &s_loginInfoData.vecLoginInfo[nIndex].mapCheckbox;
}


bool	CLoginDataManager::_ReadData(LPCTSTR strPath, vector<char>& vecData)
{
	std::ifstream	fstream(strPath, std::ios::in | std::ios::binary);
	if (fstream.fail()) {
		CString strMsg;
		strMsg.Format(_T("�t�@�C���̃I�[�v���Ɏ��s���܂���\n�u%s�v"), strPath);
		MessageBox(NULL, strMsg, NULL, 0);
		return false;
	}
	fstream.seekg(0, std::ios::end); 
	long filesize = (long)fstream.tellg();
	fstream.seekg(0, std::ios::beg);

	vecData.resize(filesize);
	fstream.read(vecData.data(), filesize);
	if (fstream.fail()) {
		MessageBox(NULL, _T("�t�@�C���̓ǂݍ��݂Ɏ��s"), NULL, 0);
		return false;
	}
	return true;
}

//----------------------------------
/// strData���烍�O�C�����ɒǉ�����
void	CLoginDataManager::_DeSerializeLoginData(const std::wstring& strData)
{
	s_loginInfoData.vecLoginInfo.clear();

	/* ���s�P�ʂɂ΂炷 */
	vector<std::wstring>	vecLine;
	int nStart = 0;
	int nCount = (int)strData.size();
	for (int i = 0; i < nCount; ++i) {
		if (strData[i] == L'\n') {
			int nLength = i - nStart;	// ���s���܂߂Ȃ�����
			if (nLength == 0)
				continue;
			vecLine.push_back(strData.substr(nStart, nLength));
			nStart = i + 1;	// ���s���΂�
			if (nStart == nCount)
				break;	// �I���
		}
	}
	if (vecLine.size() > 0) {
		auto it = vecLine.cbegin();
		if (::wcsncmp(it->c_str(), _T("EnableAutoLogin="), 16) == 0) {
			if (it->substr(16) == L"true") {
				s_loginInfoData.bEnableAutoLogin = true;
			} else {
				s_loginInfoData.bEnableAutoLogin = false;
			}
			++it;
		}
		/* s_loginInfoData.vecLoginInfo �ɒǉ� */
		for (; it != vecLine.cend(); ++it) {
			wregex	rxUrl(L"\\[(.*?)\\]");
			wsmatch	match;
			if (std::regex_match(*it, match, rxUrl)) {
				LoginInfomation	info;
				info.strLoginUrl = match.str(1).c_str();
				++it;
				if (it == vecLine.cend())
					break;	// �I���

				wregex	rxNameValue(L"([^=]+)=(.*)");
				wsmatch	match2;
				while (std::regex_match(*it, match2, rxNameValue)) {
					CString strName = match2.str(1).c_str();
					CString strValue = match2.str(2).c_str();
					if (strName == _T("AutoFillOnly")) {
						info.bAutoFillOnly	= strValue == _T("true");
					} else if (strName.Left(9) == _T("checkbox:")) {
						info.mapCheckbox.insert(std::make_pair(strName.Mid(9), strValue == L"true"));
					} else {
						info.mapNameValue.insert(std::make_pair(strName, strValue));
					}
					++it;
					if (it == vecLine.cend())
						break;	// �I���
				}
				--it;	// �i�߂������̂Ŗ߂��Ă���
				s_loginInfoData.vecLoginInfo.push_back(info);
			}
	
		}
	}
}

//----------------------------------------------
/// ���O�C������strData�ɒǉ�����
void	CLoginDataManager::_SerializeLoginData(std::wstring& strData)
{
	strData = _T("EnableAutoLogin=");
	strData += s_loginInfoData.bEnableAutoLogin ? _T("true") : _T("false");
	strData += _T("\n");
	for (auto it = s_loginInfoData.vecLoginInfo.cbegin(); it != s_loginInfoData.vecLoginInfo.cend(); ++it) {
		CString temp;
		temp.Format(_T("[%s]\n"), it->strLoginUrl);
		strData += temp;
		for (auto itmap = it->mapNameValue.cbegin(); itmap != it->mapNameValue.cend(); ++itmap) {
			temp.Format(_T("%s=%s\n"), itmap->first, itmap->second);
			strData += temp;
		}
		for (auto itcheck = it->mapCheckbox.cbegin(); itcheck != it->mapCheckbox.cend(); ++itcheck) {
			if (itcheck->first.IsEmpty() == FALSE) {
				temp.Format(_T("checkbox:%s=%s\n"), itcheck->first, itcheck->second ? _T("true") : _T("false"));
				strData += temp;
			}
		}
		strData += _T("AutoFillOnly=");
		strData += it->bAutoFillOnly ? _T("true") : _T("false");
		strData += _T("\n");
	}
}

bool	CLoginDataManager::_WriteData(LPCTSTR strPath, BYTE* pData, DWORD size)
{
	std::fstream fstream(strPath, std::ios::out | std::ios::binary);
	if (fstream.fail()) {
		CString strMsg;
		strMsg.Format(_T("�t�@�C���̃I�[�v���Ɏ��s���܂���\n�u%s�v"), strPath);
		MessageBox(NULL, strMsg, NULL, 0);
		return false;
	}
	fstream.write((LPCSTR)pData, size);
	if (fstream.fail()) {
		MessageBox(NULL, _T("�t�@�C���̏������݂Ɏ��s"), NULL, 0);
		return false;
	}
	return true;
}




////////////////////////////////////////
// CLoginInfoEditDialog : ���O�C�����ҏW�_�C�A���O


CLoginInfoEditDialog::CLoginInfoEditDialog(const LoginInfomation& info) 
	: m_info(info)
{	}


//-------------------------------
/// �_�C�A���O������
BOOL	CLoginInfoEditDialog::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	/* �^�C�g���ύX */
	CString strtemp;
	int nLen = GetWindowTextLength() + 1;
	GetWindowText(strtemp.GetBuffer(nLen), nLen);
	strtemp.ReleaseBuffer();
	CString strTitle;
	CString strVersion = AUTOLOGINVERSION;
	strTitle.Format(_T("%s - ver %s"), strtemp, strVersion);
	SetWindowText(strTitle);

	m_bEnableAutoLogin = s_loginInfoData.bEnableAutoLogin;

	_SetLoginInfoData();

	for (auto it = s_loginInfoData.vecLoginInfo.cbegin(); it != s_loginInfoData.vecLoginInfo.cend(); ++it) {
		m_UrlList.AddString(it->strLoginUrl);
	}
	return 0;
}

//------------------------------
void	CLoginInfoEditDialog::OnApply(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	DoDataExchange(DDX_SAVE, IDC_CHECKBOX_ENABLE_AUTOLOGIN);
	s_loginInfoData.bEnableAutoLogin = m_bEnableAutoLogin;

	int nIndex = m_UrlList.GetCurSel();
	if (nIndex == -1)
		return ;

	_SetCopyLoginInfo();
	LoginInfomation& info = s_loginInfoData.vecLoginInfo[nIndex];
	info = m_info;

	m_UrlList.DeleteString(nIndex);
	m_UrlList.InsertString(nIndex, m_Url);
	m_UrlList.SetCurSel(nIndex);
}

//------------------------------
void	CLoginInfoEditDialog::OnAdd(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	_SetCopyLoginInfo();
	int nIndex = m_UrlList.AddString(m_Url);
	m_UrlList.SetCurSel(nIndex);
	s_loginInfoData.vecLoginInfo.push_back(m_info);
}

//------------------------------
void	CLoginInfoEditDialog::OnDelete(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	int nIndex = m_UrlList.GetCurSel();
	if (nIndex == -1)
		return;

	s_loginInfoData.vecLoginInfo.erase(s_loginInfoData.vecLoginInfo.begin() + nIndex);
	m_UrlList.DeleteString(nIndex);
	if (nIndex < m_UrlList.GetCount() || 0 <= --nIndex) {
		m_UrlList.SetCurSel(nIndex);
		OnUrlListChange(0, 0, NULL);
	} else {
		OnNew(0, 0, NULL);
	}
}

//------------------------------
void	CLoginInfoEditDialog::OnNew(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	m_UrlList.SetCurSel(-1);

	m_info.strLoginUrl.Empty();
	m_info.mapNameValue.clear();
	_SetLoginInfoData();
}

//------------------------------
/// �t�@�C�����烍�O�C�������C���|�[�g����
void	CLoginInfoEditDialog::OnImport(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	CFileDialog dlg(TRUE, _T("txt"), _T("AutoLoginDataFile"), OFN_HIDEREADONLY | OFN_CREATEPROMPT,
		_T("�e�L�X�g �t�@�C�� (*.txt)\0*.txt\0���ׂẴt�@�C�� (*.*)\0*.*\0\0"));
	if (dlg.DoModal() == IDOK) {
		Import(dlg.m_szFileName);
		m_UrlList.ResetContent();
		for (auto it = s_loginInfoData.vecLoginInfo.cbegin(); it != s_loginInfoData.vecLoginInfo.cend(); ++it) {
			m_UrlList.AddString(it->strLoginUrl);
		}
		OnNew(0, 0, NULL);
	}

}

//------------------------------
/// �t�@�C���Ƀ��O�C�������G�N�X�|�[�g����
void	CLoginInfoEditDialog::OnExport(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	CFileDialog dlg(FALSE, _T("txt"), _T("AutoLoginDataFile"), OFN_OVERWRITEPROMPT,
		_T("�e�L�X�g �t�@�C�� (*.txt)\0*.txt\0���ׂẴt�@�C�� (*.*)\0*.*\0\0"));
	if (dlg.DoModal() == IDOK) {
		Export(dlg.m_szFileName);
	}

}

//------------------------------
/// �������O�C���e�X�g
void	CLoginInfoEditDialog::OnTest(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	OnApply(0, 0, NULL);

	if (CMainOption::s_BrowserOperatingMode == BROWSEROPERATINGMODE::kMultiProcessMode) {
		Save();
		CreateOriginalLoginDataList(GetParent());
	}
	m_funcAutoLogin();
}


//------------------------------
void	CLoginInfoEditDialog::OnFinish(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	DoDataExchange(DDX_SAVE, IDC_CHECKBOX_ENABLE_AUTOLOGIN);
	s_loginInfoData.bEnableAutoLogin = m_bEnableAutoLogin;

	Save();

	CreateOriginalLoginDataList(GetParent());

	EndDialog(IDOK);
}

//-----------------------------
void CLoginInfoEditDialog::OnUrlListChange(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	int nIndex = m_UrlList.GetCurSel();
	if (nIndex == -1)
		return ;

	m_info = s_loginInfoData.vecLoginInfo[nIndex];
	_SetLoginInfoData();
}


//-----------------------------
/// �E����m_info����f�[�^��ݒ�
void	CLoginInfoEditDialog::_SetLoginInfoData()
{
	m_Url	= m_info.strLoginUrl;
	{		
		for (int i = 0; i < s_cMaxNameValue; ++i) {
			m_Name[i].Empty();
			m_Value[i].Empty();
		}

		auto it = m_info.mapNameValue.begin();
		int nCount = (int)m_info.mapNameValue.size();
		for (int i = 0; i < nCount && i < s_cMaxNameValue; ++i) {
			m_Name[i]	= it->first;
			m_Value[i]	= it->second;
			++it;
		}

	}
	{
		for (int i = 0; i < s_cMaxCheckbox; ++i) {
			m_CheckboxName[i].Empty();
			m_bCheck[i] = false;
		}

		auto it = m_info.mapCheckbox.begin();
		int nCount = (int)m_info.mapCheckbox.size();
		for (int i = 0; i < nCount && i < s_cMaxCheckbox; ++i) {
			m_CheckboxName[i]	= it->first;
			m_bCheck[i]			= it->second;
			++it;
		}
	}
	m_bAutoFillOnly	= m_info.bAutoFillOnly;
	DoDataExchange(DDX_LOAD);
}

//---------------------------
/// �E������m_info�Ƀf�[�^��ݒ�
void	CLoginInfoEditDialog::_SetCopyLoginInfo()
{
	DoDataExchange(DDX_SAVE);
	m_Url.MakeLower();
	m_info.strLoginUrl	= m_Url;

	m_info.mapNameValue.clear();
	for (int i = 0; i < s_cMaxNameValue; ++i) {
		if (m_Name[i].IsEmpty() == FALSE) {
			m_info.mapNameValue.insert(std::make_pair(m_Name[i], m_Value[i]));
		}
	}

	m_info.mapCheckbox.clear();
	for (int i = 0; i < s_cMaxCheckbox; ++i) {
		if (m_CheckboxName[i].IsEmpty() == FALSE) {
			m_info.mapCheckbox.insert(std::make_pair(m_CheckboxName[i], m_bCheck[i]));
		}
	}
	m_info.bAutoFillOnly = m_bAutoFillOnly;
}