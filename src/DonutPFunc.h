/**
 *	@file DonutPFunc.h
 *	@brief	DonutP �̔ėp�֐�.
 */

#ifndef __DONUTPFUNC_H__
#define __DONUTPFUNC_H__

#pragma once

#include "Misc.h"		//+++

extern const TCHAR	g_cSeparater[];

//////////////////////////////////////////////////////

//---------------------------------------
/// length������item��3�̈��������R���N�V������p
template<class collection>
void	ForEachHtmlElement(collection col, function<bool (IDispatch*)> func)
{
	if (col) {
		long length = 0;
		col->get_length(&length);
		for (long i = 0; i < length; ++i) {
			CComVariant	vIndex(i);
			CComPtr<IDispatch>	spDisp;
			col->item(vIndex, vIndex, &spDisp);
			if (func(spDisp) == false)
				break;
		}
	}
}


//-----------------------------------------
/// �A�C�e���h�c���X�g����A�C�R�������
HICON	CreateIconFromIDList(PCIDLIST_ABSOLUTE pidl);

//-----------------------------------------
/// �A�C�R������r�b�g�}�b�v�����
HBITMAP	CreateBitmapFromHICON(HICON hIcon);


double	_GetAngle(CPoint pt1, CPoint pt2);


BOOL	_CheckOsVersion_XPLater();
BOOL	_CheckOsVersion_VistaLater();

//----------------------------------------

/// ini�t�@�C���p�X������������
void	InitDonutConfigFilePath(LPTSTR iniFilePath, int sizeInWord);

/// filename��Config�t�H���_�ȉ��̃p�X�ɂ��ĕԂ�
CString GetConfigFilePath(const CString& filename);

/// undonut.exe�Ɠ����f�B���N�g���� strFile ��������̂Ƃ��ăt���p�X��Ԃ�.
inline CString _GetFilePath(const CString& strFile) { return Misc::GetExeDirectory() + strFile; }

//----------------------------------------
/// �ݒ肳�ꂽ�X�L���t�H���_�̃p�X��Ԃ�(�Ō��'\\'����)
CString _GetSkinDir();

//----------------------------------------
/// �R�}���h�Ɋ֘A�t����ꂽ��������擾����
bool	_LoadToolTipText(int nCmdID, CString &strText);

//----------------------------------------
/// �������̗\��̈���ꎞ�I�ɍŏ����B�E�B���h�E���ŏ��������ꍇ�Ɠ���	//+++ DonutRAPT ���p�N���Ă������[�`��
void	RtlSetMinProcWorkingSetSize();


//-------------------------------------------
/// �G�f�B�b�g�R���g���[���őI������Ă��镶�����Ԃ�
CString _GetSelectText(const CEdit &edit);

//-------------------------------------------
///+++ �G�f�B�b�g�́A�I���e�L�X�g�̎擾. ���I���̏ꍇ�́A�J�[�\���ʒu(�L�����b�g�̂��ƁH)�̒P����擾.(�y�[�W�������p)
CString _GetSelectText_OnCursor(const CEdit &edit);		//+++


BOOL	_AddSimpleReBarBandCtrl(HWND hWndReBar,
								HWND hWndBand,
								int nID 				= 0,
								LPTSTR lpstrTitle		= NULL,
								BOOL bNewRow			= FALSE,
								int cxWidth 			= 0,
								BOOL bFullWidthAlways	= FALSE);


//----------------------------------------------
/// strFile �� Strings�����s����������
bool	FileWriteString(const CString& strFile, const std::list<CString>& Strings);

//----------------------------------------------
/// strFile ���� 1�s���Ƃ�Strings�ɓ���Ă���(��1�s�ɂ�4096�����ȏ�ǂݍ��ނƕ��������)
bool	FileReadString(const CString& strFile, std::list<CString>& Strings);


//---------------------------------------------
/// ���j���[�̕�����������R���{�{�b�N�X�ɒǉ�����
BOOL	_SetCombboxCategory(CComboBox &cmb, HMENU hMenu);
BOOL	_DontUseID(UINT uID);
//----------------------------------------------
/// ���j���[�̃R�}���h�Ɋ��蓖�Ă�ꂽ��������R���{�{�b�N�X/���X�g�{�b�N�X�ɒǉ�����
template <class boxT>
void	PickUpCommandSub(CMenuHandle menuSub, boxT box)
{
	int nPopStartIndex = box.AddString(g_cSeparater);
	int nAddCnt = 0;

	int nCount = menuSub.GetMenuItemCount();
	for (int ii = 0; ii < nCount; ++ii) {
		HMENU	hMenuSub2 = menuSub.GetSubMenu(ii);
		if (hMenuSub2)
			PickUpCommandSub(hMenuSub2, box);

		UINT	nCmdID	  = menuSub.GetMenuItemID(ii);
		if ( _DontUseID(nCmdID) )
			break;

		CString strMenu;
		CToolTipManager::LoadToolTipText(nCmdID, strMenu);

		if ( strMenu.IsEmpty() )
			continue;

		int 	nIndex	  = box.AddString(strMenu);
		box.SetItemData(nIndex, nCmdID);
		nAddCnt++;
	}

	if (nAddCnt != 0)
		box.AddString(g_cSeparater);
	else
		box.DeleteString(nPopStartIndex);
}
//-------------------------------------------------
/// hMenu��nPopup�Ԗڂ̃T�u���j���[�̃R�}���h�Ɋ��蓖�Ă�ꂽ��������R���{�{�b�N�X�ɒǉ�����
template <class boxT>
void	PickUpCommand(CMenuHandle menuRoot, int nPopup, boxT box)
{
	CMenu	menu = menuRoot.GetSubMenu(nPopup);
	box.ResetContent();

	int nCount = menu.GetMenuItemCount();
	for (int ii = 0; ii < nCount; ++ii) {
		HMENU	hMenuSub = menu.GetSubMenu(ii);
		if (hMenuSub)
			PickUpCommandSub(hMenuSub, box);

		UINT	nCmdID	 = menu.GetMenuItemID(ii);

		if ( _DontUseID(nCmdID) )
			break;

		CString strMenu;
		CToolTipManager::LoadToolTipText(nCmdID, strMenu);

		if ( strMenu.IsEmpty() )
			continue;

		int nIndex = box.AddString(strMenu);
		box.SetItemData(nIndex, nCmdID);
	}

	menu.Detach();

	//�s�v�ȃZ�p���[�^�̍폜
	int   nCountSep = 0;
	int   nCountCmb = box.GetCount();

	for (int i = 0; i < nCountCmb - 1; i++) {
		if (box.GetItemData(i) == 0) {
			nCountSep++;

			if (box.GetItemData(i + 1) == 0) {
				box.DeleteString(i);
				nCountCmb--;
				i--;
			}
		}
	}

	if (nCountSep > 2) {
		if (box.GetItemData(0) == 0)
			box.DeleteString(0);

		int nIndexLast = box.GetCount() - 1;

		if (box.GetItemData(nIndexLast) == 0)
			box.DeleteString(nIndexLast);
	}
}

//-------------------------------------------------
/// �C���[�W���X�g�̃C���[�W��strBmpFile�̃r�b�g�}�b�v�Œu������
BOOL	_ReplaceImageList(const CString& strBmpFile, CImageList& imgs, DWORD defaultResID = 0);


//+++ .manifest�̑��݃`�F�b�N	�� CThemeDLLLoader ����Ɨ� & ������ƃ��l�[��
#if (defined UNICODE) && (defined USE_INNER_MANIFEST)
inline bool IsExistManifestFile() { return 1; }
#else
inline bool IsExistManifestFile() { return ::PathFileExists(Misc::GetExeFileName() + _T(".manifest")) != 0; }
#endif

//------------------------------------------
/// �t�H���g�̍�����Ԃ�
int		GetFontHeight(HFONT hFont);


//------------------------------------------
///�����I�ɁA���̏�Ń��b�Z�[�W�����΂�...
int 	ForceMessageLoop(HWND hWnd = NULL);


//------------------------------------------
/// �G���[�R�[�h�ɑΉ�����G���[���b�Z�[�W�������Ԃ�
CString	GetLastErrorString(HRESULT hr = -1);


//------------------------------------------
/// �ꎞ�t�@�C���u����̃p�X��Ԃ�
bool	GetDonutTempPath(CString& strTempPath);

//------------------------------------------
/// �G�N�X�v���[���[���J���ăA�C�e����I������
void	OpenFolderAndSelectItem(const CString& strPath);

///------------------------------------------
/// vecText�� "�e�L�X�g�P\0�e�L�X�g�Q\0�e�L�X�g�R\0\0" �Ƃ������`���ɕϊ����ĕԂ�
std::pair<std::unique_ptr<WCHAR[]>, int>	CreateMultiText(const std::vector<CString>& vecText);

///------------------------------------------
/// "�e�L�X�g�P\0�e�L�X�g�Q\0�e�L�X�g�R\0\0" �Ƃ������������ vector<CString> �ɂ��ĕԂ�
std::vector<CString>	GetMultiText(LPCWSTR multiText);

#endif	// __DONUTPFUNC_H__

