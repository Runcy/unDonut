/**
*	@file	DonutCommandBar.h
*	@brief	�R�}���h�o�[�N���X
*/

#pragma once

// �O���錾
class CRecentClosedTabList;


///////////////////////////////////////////////////////
// �R�}���h�o�[

class CDonutCommandBar
{
public:
	CDonutCommandBar();
	~CDonutCommandBar();

	HWND	Create(HWND hWndParent);
	void	SetFont(HFONT hFont);

	HWND	GetHWND() const;

	void	SetRecentClosedTabList(CRecentClosedTabList* pList);

private:
	class Impl;
	Impl*	pImpl;
};















