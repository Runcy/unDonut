/**
 *	@file	RenameFileDialog.cpp
 *	@brief	�t�@�C�����ύX�_�C�A���O
 */
#include "stdafx.h"
#include "RenameFileDialog.h"
#include "../MtlFile.h"
#include "../MtlWin.h"

/////////////////////////////////////////////////////////////////
// CRenameDialog


CRenameDialog::CRenameDialog(LPCTSTR strOldFileName, LPCTSTR strFilePath, bool bDoRename /*= true*/) : 
	m_strOldFileName(strOldFileName), 
	m_bDoRename(bDoRename)
{
	m_strFolder = Misc::GetDirName(CString(strFilePath)) + _T("\\");
}

void	CRenameDialog::DoRename() const
{
	// ���l�[��
	::MoveFileEx(m_strFolder + m_strOldFileName, m_strFolder + m_strNewFileName, MOVEFILE_REPLACE_EXISTING);

	/* �G�N�X�v���[���[�Ƀt�@�C���̕ύX�ʒm */
	::SHChangeNotify(SHCNE_RENAMEITEM, SHCNF_PATH, 
		static_cast<LPCTSTR>(m_strFolder + m_strOldFileName), 
		static_cast<LPCTSTR>(m_strFolder + m_strNewFileName));
}

BOOL CRenameDialog::OnInitDialog(CWindow wndFocus, LPARAM lInitParam)
{
	CEdit edit = GetDlgItem(IDC_EDIT);
	edit.SetWindowText(m_strOldFileName);
	CString ext = Misc::GetFileExt(m_strOldFileName);
	if (ext.GetLength() > 0) {
		int nSel = m_strOldFileName.GetLength() - ext.GetLength() - 1;
		PostMessage(WM_SELTEXTWITHOUTEXT, nSel);
	}
		
	//WTL::CLogFont	lf;
	//lf.SetMenuFont();
	//GetDlgItem(IDC_EDIT).SetFont(lf.CreateFontIndirect());
	return 0;
}

LRESULT CRenameDialog::OnSelTextWithoutExt(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CEdit edit = GetDlgItem(IDC_EDIT);
	edit.SetSel(0, (int)wParam, TRUE);
	return 0;
}

void CRenameDialog::OnOk(UINT uNotifyCode, int nID, CWindow wndCtl)
{
	m_strNewFileName = MtlGetWindowText(GetDlgItem(IDC_EDIT));
	if (m_strNewFileName.IsEmpty()) {
		MessageBox(_T("�t�@�C��������͂��Ă��������B"), NULL, MB_ICONERROR);
		return ;
	}
	if (MtlIsValidateFileName(m_strNewFileName) == false) {
		MessageBox(_T("�L���ȃt�@�C�����ł͂���܂���B\n�u\\/:*?\"<>|�v�̓t�@�C�����Ɋ܂߂邱�Ƃ͂ł��܂���B"), NULL, MB_ICONERROR);
		return ;
	}

	if (m_strOldFileName != m_strNewFileName) {
		if (::PathFileExists(m_strFolder + m_strNewFileName)) {
			if (MessageBox(_T("�������Ƀt�@�C�������݂��܂��B\n�㏑�����܂����H"), NULL, MB_ICONQUESTION | MB_YESNO) != IDYES)
				return ;
		}
		if (m_bDoRename)
			DoRename();
	}

	EndDialog(nID);
}








