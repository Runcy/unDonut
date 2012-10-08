/**
 *	@file	SupressPopupOption.h
 *	@brief	donut�̃I�v�V���� : �|�b�v�A�b�v/�^�C�g���}�~
 */

#pragma once

#include "../resource.h"
#include "../SharedMemoryUtil.h"

struct PopupBlockData
{
	bool	bValidIgnoreURL;
	std::list<CString>	IgnoreURLList;
	bool	bValidCloseTitle;
	std::list<CString>	CloseTitleList;

	PopupBlockData() : bValidIgnoreURL(false), bValidCloseTitle(false)
	{	}


private:
	friend class boost::serialization::access;  
	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & bValidIgnoreURL  & IgnoreURLList;
		ar & bValidCloseTitle & CloseTitleList;
	}
};

class CSupressPopupOption 
{
public:
	static PopupBlockData	s_PopupBlockData;

	// for MainFrame
	static void CreateSupressPopupData(HWND hWndMainFrame);

	// for ChildFrame
	static void UpdateSupressPopupData(HWND hWndMainFrame);

	static bool SearchURLString(const CString &strURL);
	static bool SearchTitleString(const CString &strTitle);

	// for MainFrame
	static void AddIgnoreURL(const CString &strURL);
	static void AddCloseTitle(const CString &strTitle);

	static void ReCreateSupressPopupDataAndNotify();

protected:
	static void GetProfile();
	static void WriteProfile(bool bIgnoreURLPropertyPage);	
	static void	NotifyUpdateToChildFrame();

	// Data members
	static HWND				s_hWndMainFrame;
	static CSharedMemory	s_sharedMem;
	
};

/**
	CIgnoredURLsPropertyPage
	URL�ɂ��\���}�~��ݒ肷�邽�߂̃v���p�e�B�x�[�W�_�C�A���O�N���X

	�\���֎~�^�C�g���̈ꗗ��ҏW���邽�߂̃_�C�A���O
 */
class CIgnoredURLsPropertyPage
	: public CPropertyPageImpl<CIgnoredURLsPropertyPage>
	, public CWinDataExchange<CIgnoredURLsPropertyPage>
	, protected CSupressPopupOption
{
public:
	// Declarations
	enum { IDD = IDD_PROPPAGE_IGNOREDURLS };

	// Constructor
	CIgnoredURLsPropertyPage(const CString &strAddressBar);

	// DDX map
	BEGIN_DDX_MAP( CIgnoredURLsPropertyPage )
		DDX_CHECK( IDC_IGNORED_URL_VALID, m_nValid )
	END_DDX_MAP()

	// Overrides
	BOOL	OnSetActive();
	BOOL	OnKillActive();
	BOOL	OnApply();

private:
	// overrides
	BOOL	_DoDataExchange(BOOL bSaveAndValidate); // get data from controls?

public:
	// Message map and handlers
	BEGIN_MSG_MAP( CIgnoredURLsPropertyPage )
		MESSAGE_HANDLER( WM_INITDIALOG	, OnInitDialog )
		MESSAGE_HANDLER( WM_DESTROY 	, OnDestroy    )
		COMMAND_ID_HANDLER_EX( IDC_ADD_BUTTON	, OnAddCmd	  )
		COMMAND_ID_HANDLER_EX( IDC_APPLY		, OnApply	)
		COMMAND_ID_HANDLER_EX( IDC_DELALL_BUTTON, OnDelAllCmd )
		COMMAND_ID_HANDLER_EX( IDC_DEL_BUTTON	, OnDelCmd	  )
		COMMAND_HANDLER_EX( IDC_IGNORED_URL_LIST, LBN_SELCHANGE, OnSelChange )
		CHAIN_MSG_MAP( CPropertyPageImpl<CIgnoredURLsPropertyPage> )
	ALT_MSG_MAP(1)
		MSG_WM_KEYUP( OnListKeyUp )
	END_MSG_MAP()

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);

private:
	void	OnDelCmd(UINT wNotifyCode, int wID, HWND hWndCtl);
	void	OnApply(UINT wNotifyCode, int wID, HWND hWndCtl);
	void	OnDelAllCmd(UINT wNotifyCode, int wID, HWND hWndCtl);
	void	OnAddCmd(UINT wNotifyCode, int wID, HWND hWndCtl);
	void	OnSelChange(UINT code, int id, HWND hWnd);
	void	OnListKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

	// Implementation
	// function objects
	struct _AddToListBox : public std::unary_function<const CString &, void> {
		CListBox &_box;
		_AddToListBox(CListBox &box) : _box(box) { }
		result_type operator ()(argument_type src)
		{
			_box.AddString(src);
		}
	};

	void _GetData();

	// Data members
	bool	m_bInit;
	CString 						m_strAddressBar;
	CListBox						m_listbox;
	CEdit							m_edit;
	int 							m_nValid;
	CContainedWindow				m_wndList;
};




/**
	CCloseTitlesPropertyPage
	�y�[�W�^�C�g���ɂ��\���}�~��ݒ肷�邽�߂̃v���p�e�B�x�[�W�_�C�A���O�N���X

	�\���֎~�^�C�g���̈ꗗ��ҏW���邽�߂̃_�C�A���O
 */
class CCloseTitlesPropertyPage
	: public CPropertyPageImpl<CCloseTitlesPropertyPage>
	, public CWinDataExchange<CCloseTitlesPropertyPage>
	, protected CSupressPopupOption
{
public:
	//�_�C�A���O�̃��\�[�XID
	enum { IDD = IDD_PROPPAGE_CLOSETITLES };

	//�R���X�g���N�^
	CCloseTitlesPropertyPage(const CString &strAddressBar);

	//DDX�}�b�v
	BEGIN_DDX_MAP(CCloseTitlesPropertyPage) 			//+++
		DDX_CHECK( IDC_CHK_TITLECLOSE, m_nValid )
	END_DDX_MAP()

	//�v���p�e�B�x�[�W�Ƃ��ẴI�[�o�[���C�h�֐�
	BOOL	OnSetActive();
	BOOL	OnKillActive();
	BOOL	OnApply();


	//���b�Z�[�W�}�b�v
	BEGIN_MSG_MAP( CCloseTitlesPropertyPage )
		MESSAGE_HANDLER 	 ( WM_INITDIALOG	, OnInitDialog	)
		MESSAGE_HANDLER 	 ( WM_DESTROY		, OnDestroy 	)
		COMMAND_ID_HANDLER_EX( IDC_ADD_BUTTON	, OnAddCmd		)
		COMMAND_ID_HANDLER_EX( IDC_APPLY		, OnApply	)
		COMMAND_ID_HANDLER_EX( IDC_DELALL_BUTTON, OnDelAllCmd	)
		COMMAND_ID_HANDLER_EX( IDC_DEL_BUTTON	, OnDelCmd		)
		COMMAND_HANDLER_EX	 ( IDC_IGNORED_URL_LIST, LBN_SELCHANGE, OnSelChange )
		CHAIN_MSG_MAP( CPropertyPageImpl<CCloseTitlesPropertyPage> )
	ALT_MSG_MAP(1)
		MSG_WM_KEYUP( OnListKeyUp )
	END_MSG_MAP()

	//���b�Z�[�W�n���h��
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);


	//�R�}���h�n���h��
	void	OnDelCmd(UINT /*wNotifyCode*/, int /*wID*/, HWND /*hWndCtl*/);
	void	OnApply(UINT wNotifyCode, int wID, HWND hWndCtl);
	void	OnDelAllCmd(UINT /*wNotifyCode*/, int /*wID*/, HWND /*hWndCtl*/);
	void	OnAddCmd(UINT /*wNotifyCode*/, int /*wID*/, HWND /*hWndCtl*/);
	void	OnSelChange(UINT code, int id, HWND hWnd);
	void	OnListKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

	//�����֐�
	void	GetData();									//�_�C�A���O����f�[�^�̎擾
	BOOL	DataExchange(BOOL bSaveAndValidate);		//�R���g���[���̏�Ԃƃf�[�^�̕ϊ�

private:
	//�����o�ϐ�
	bool	m_bInit;
	CString 						  m_strAddressBar;	//���J���Ă���y�[�W�̃^�C�g��
	CListBox						  m_listbox;		//���X�g�{�b�N�X�̑���N���X
	CEdit							  m_edit;			//�e�L�X�g�{�b�N�X�̑���N���X
	int 							  m_nValid; 		//�^�C�g���}�~�@�\�͗L����
	CContainedWindow				  m_wndList;		//���X�g�{�b�N�X�̑���N���X

	//�֐��I�u�W�F�N�g
	struct AddToListBox : public std::unary_function<const CString &, void> {
		CListBox &m_box;
		AddToListBox(CListBox &box) : m_box(box) { }
		result_type operator ()(argument_type src)
		{
			m_box.AddString(src);
		}
	};
};
