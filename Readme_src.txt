
undonut+mod.1.48�̃\�[�X�t�@�C������WTL80�������Ă��ăp�X��ʂ��Ă�������

ver 26����boost::thread���g���Ă�̂�
boost��p�ӂ��Ă�������

�R���p�C�������Ƃ�
>libboost~.lib��������Ȃ�
�Ƃ��o����
>bjam --toolset=vc100 link=static runtime-link=static debug release stage 
����Ȋ�����

�������ł�VS2010�ȊO�ŃR���p�C���ʂ�Ȃ��Ǝv���܂��c
���̑��R���p�C���ʂ�Ȃ������猾����


WDK�Ƃ��ɂ��Ă�ATL���g���Ƃ���
atlthunk.lib�����C�u�����w�肳��ĂȂ��̂ŁA 
�����J�̒ǉ��̈ˑ��t�@�C���ɁAatlthunk.lib��t����B 
�������́���K���ȃw�b�_�ɒǉ����� 
#pragma comment(lib,"atlthunk.lib") 



���\�[�X�G�f�B�^�ŕҏW���Ă����v�ɂȂ����͂�
�R�����g�Ƃ����������������resource_back.h�Ɏc���Ă邩����v����

���\�[�X�G�f�B�^���g���ă_�C�A���O�Ȃǂ�ҏW�����Ƃ���
///////////
// ICON
�E�E�E
///////////
// Bitmap
�E�E�E
�Ƃ��ɏ��������̂ŁA���̕������R���p�C������O�ɖ߂��Ă�������
(32bit�ł���邾���Ȃ猳�ɖ߂��Ȃ��Ă������j



���̃\�[�X���g�����o�C�i���̌��J�Ƃ��͎��R�ɂ��Ă�������



--------------------------------------------------------------------

DebugWindow���������Ă���DebugWindow�̃G�f�B�b�g�{�b�N�X��
Backspace�L�[���g���Ȃ��ăn�}�����̂Ń���
�ǂ����u���E�U(IEServer)���L�[��H���Ă��݂����ł��ꂪ�_���������炵��
CMainFrame::PreTranslateMessage ...
hWnd = MDIGetActive();
if (bFocus == 0 && hWnd != NULL && ::SendMessage(hWnd, WM_FORWARDMSG, 0, (LPARAM) pMsg) ) {
	return TRUE;
}
�����WM_FORWARDMSG�̏������
if(spInPlaceActiveObject->TranslateAccelerator(lpMsg) == S_OK)
	return 1;
1���Ԃ����
return TRUE��Ԃ�������::TraslateMessage��::DespatchMessage���Ă΂�Ȃ������̂�����
DebugWindow�Ƀt�H�[�J�X���������Ă�������bFocus = true�Ƃ��邱�Ƃł�����������


����㩂Ɉ�����������
http://ameblo.jp/blueskyame/entry-10398978729.html

