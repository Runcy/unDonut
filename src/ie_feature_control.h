/** +++
 *	@file	ie_feature_control.h
 *	@brief	Feature�R���g���[���֌W�̋N�����ݒ�.
 */

#pragma once


inline void ie_feature_control_setting()
{
	// �g���q�ł͂Ȃ����e�ɂ���ĊJ���A����߂�
	::CoInternetSetFeatureEnabled(FEATURE_MIME_SNIFFING   , SET_FEATURE_ON_PROCESS, TRUE);

	// �^�u�u���E�W���O��L��
	::CoInternetSetFeatureEnabled(FEATURE_TABBED_BROWSING, SET_FEATURE_ON_PROCESS, TRUE);

	// �|�b�v�A�b�v�u���b�N��L��
	//::CoInternetSetFeatureEnabled(FEATURE_WEBOC_POPUPMANAGEMENT, SET_FEATURE_ON_PROCESS, TRUE);

	// SSLerror�_�C�A���O��\�������Ȃ�
	::CoInternetSetFeatureEnabled(FEATURE_SSLUX, SET_FEATURE_ON_PROCESS, FALSE);


#if 0	//* ���Ƃ�
	// ���o�[�\���BActivX�̎��s�I�t��ActiveX�I�u�W�F�N�g������ꍇ���ɕ\��
	bool	sw = _check_flag(MAIN_EX_KILLDIALOG, CMainOption::s_dwMainExtendedStyle);
	ie_coInternetSetFeatureEnabled(FEATURE_SECURITYBAND    , SET_FEATURE_ON_PROCESS, sw);
#endif
#if 0	// ���x�����Ȃ��̂őʖ�
	// swf ���g����悤�ɂ��邽�߁A�X�N���v�g�̐������ɂ߂�.
	ie_coInternetSetFeatureEnabled(FEATURE_BLOCK_LMZ_SCRIPT, SET_FEATURE_ON_PROCESS, FALSE);
#endif
#if 0	//�ʖ�
	//���W�X�g������Windows�̃A�b�v�f�[�g�����擾
	Misc::CRegKey 	reg;
	reg.Open( HKEY_CURRENT_USER, _T("SOFTWARE\\Microsoft\\Internet Explorer\\Main\\FeatureControl") );
	reg.SetDWORDValue(_T("unDonut.exe"), 0);
	reg.Close();
#endif
}


