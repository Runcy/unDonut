/*!	@file	DonutSearchBar.h
	@brief	検索バー
*/

#pragma once


struct SearchPostData {
	LPVOID		pPostData;
	int			nPostBytes;

	SearchPostData() :
		pPostData(NULL),
		nPostBytes(0)
	{ }
};

/////////////////////////
/// 検索バークラス

class CDonutSearchBar
{
public:
	CDonutSearchBar();
	~CDonutSearchBar();

	static CDonutSearchBar* GetInstance() { return s_pThis; }

	HWND	Create(HWND hWndParent);
	void	ReloadSkin(int nCmbStyle);
	void	SetFont(HFONT hFont, BOOL bRedraw = TRUE);

	HWND	GetHWND() const;
	HWND	GetKeywordComboBox() const;
	CEdit	GetEditCtrl() const;
	HWND	GetHWndToolBar() const;

	bool	ForceSetHilightBtnOn(bool bOn);
	bool	GetHilightSw() const;
	void	SetSearchStr(const CString& strWord);
	CMenuHandle GetSearchEngineMenuHandle();
	const SearchPostData&	GetSearchPostData() const;

	void	SearchWeb(CString str = CString());
	void	SearchWebWithEngine(CString str, CString strEngine);
	void	SearchWebWithIndex(CString str, int nIndex);
	void	SearchPage(bool bForward);
	void	SearchHilight();

	void	SetFocusToEngine();
	void	RefreshEngine();

	void	AddToSearchBoxUnique(const CString& str);

	BOOL	PreTranslateMessage(MSG *pMsg);

private:
	class Impl;
	Impl* pImpl;
	static CDonutSearchBar* s_pThis;
};





























