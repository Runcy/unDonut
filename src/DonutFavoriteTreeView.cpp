/**
*	@file	DonutFavoriteTreeView.cpp
*	@brief	���C�ɓ���c���[�r���[
*/

#include "stdafx.h"
#include "DonutFavoriteTreeView.h"
#include <deque>
#include "PopupMenu.h"
#include "LinkPopupMenu.h"
#include "HLinkDataObject.h"

CDonutFavoriteTreeView::CDonutFavoriteTreeView() : m_hItemDragging(NULL)
{
}


CDonutFavoriteTreeView::~CDonutFavoriteTreeView()
{
}


HWND	CDonutFavoriteTreeView::Create(HWND hWndParent)
{
	DWORD dwStyle =	 WS_CHILD  | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_BORDER
					| TVS_INFOTIP | TVS_SHOWSELALWAYS  | TVS_FULLROWSELECT | TVS_TRACKSELECT | TVS_SINGLEEXPAND;

	HWND hWnd = __super::Create(hWndParent, 0, nullptr, dwStyle);  
	ATLASSERT( ::IsWindow(hWnd) );
	return hWnd;
}


int CDonutFavoriteTreeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	DefWindowProc();

	SetWindowText(_T("���C�ɓ���"));

#ifndef TVS_EX_RICHTOOLTIP
#define TVS_EX_RICHTOOLTIP          0x0010
#endif
#ifndef TVS_EX_DOUBLEBUFFER
#define TVS_EX_DOUBLEBUFFER         0x0004
#endif

	SetExtendedStyle(TVS_EX_RICHTOOLTIP | TVS_EX_DOUBLEBUFFER, TVS_EX_RICHTOOLTIP | TVS_EX_DOUBLEBUFFER);

	m_imageList.Create(16, 16, ILC_COLOR32 | ILC_MASK, 200, 500);
	m_imageList.AddIcon(CLinkPopupMenu::s_iconFolder);
	m_imageList.AddIcon(CLinkPopupMenu::s_iconLink);
	SetImageList(m_imageList);

	if (CRootFavoritePopupMenu::IsLoadingBookmarkList()) {
		SetTimer(kLoadBookmarkListTimerId, kLoadBookmarkListTimerInterval);
	} else {
		_RefreshTree();
	}

	RegisterDragDrop();

	return 0;
}

void CDonutFavoriteTreeView::OnDestroy()
{
	DefWindowProc();

	m_imageList.Destroy();

	RevokeDragDrop();

	CRootFavoritePopupMenu::SetRefreshNotify(m_hWnd, std::bind(&CDonutFavoriteTreeView::_RefreshTree, this), false);
}

HTREEITEM CDonutFavoriteTreeView::_AddTreeItem(HTREEITEM htRoot, HTREEITEM htInsertAfter, LinkItem* pItem)
{
	HTREEITEM htItem = InsertItem(pItem->strName, htRoot, htInsertAfter);
	SetItemData(htItem, (DWORD_PTR)pItem);
	if (pItem->pFolder) {
		TVITEM tvi = {};
		tvi.hItem	= htItem;
		tvi.mask	= TVIF_CHILDREN;
		tvi.cChildren = TRUE;
		SetItem(&tvi);
		SetItemImage(htItem, kIconFolderIndex, kIconFolderIndex);

	} else {
		if (pItem->icon.IsNull()) {
			SetItemImage(htItem, kIconLinkIndex, kIconLinkIndex);
		} else {
			int& iconIndex = m_mapIconIndex[pItem->icon.m_hIcon];
			if (iconIndex == 0)
				iconIndex = m_imageList.AddIcon(pItem->icon);
			SetItemImage(htItem, iconIndex, iconIndex);
		}
	}
	return htItem;
}

void	CDonutFavoriteTreeView::_AddTreeItemList(HTREEITEM htRoot, LinkFolderPtr list)
{
	for (auto& item : *list) {
		_AddTreeItem(htRoot, TVI_LAST, item.get());
	}
}

void CDonutFavoriteTreeView::_RefreshTree()
{
	DeleteAllItems();

	auto pBookmarkList = CRootFavoritePopupMenu::GetBookmarkListPtr();
	_AddTreeItemList(TVI_ROOT, pBookmarkList);
}

void CDonutFavoriteTreeView::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == kLoadBookmarkListTimerId) {
		if (CRootFavoritePopupMenu::IsLoadingBookmarkList() == false) {
			KillTimer(kLoadBookmarkListTimerId);

			_RefreshTree();
		}
	} else if (nIDEvent == kDragOverExpandTimerId) {
		Expand(m_htLastDragOver);
	}
}


void CDonutFavoriteTreeView::OnShowWindow(BOOL bShow, UINT nStatus)
{
	// �c���[�̕ύX��ʒm���Ă��炤
	CRootFavoritePopupMenu::SetRefreshNotify(m_hWnd, std::bind(&CDonutFavoriteTreeView::_RefreshTree, this), bShow != 0);
}



LRESULT CDonutFavoriteTreeView::OnTreeItemExpanding(LPNMHDR pnmh)
{
	LPNMTREEVIEW pnmtv = (LPNMTREEVIEW)pnmh;
	auto pLinkItem = (LinkItem*)pnmtv->itemNew.lParam;
	if (pLinkItem->pFolder) {
		if (GetChildItem(pnmtv->itemNew.hItem))
			return 0;

		_AddTreeItemList(pnmtv->itemNew.hItem, pLinkItem->pFolder);
	}
	return 0;
}


void CDonutFavoriteTreeView::OnLButtonDown(UINT nFlags, CPoint point)
{
	UINT flags = 0;
	HTREEITEM htHit = HitTest(point, &flags);
	_DoDragDrop(htHit, point, 0);
}


void CDonutFavoriteTreeView::_DoDragDrop(HTREEITEM htDragItem, CPoint pt, UINT nFlags)
{
	if ( PreDoDragDrop(m_hWnd, NULL, false) ) {			// now dragging

		m_pParentDragFolder = GetParentLinkFolderPtr(htDragItem);
		ATLASSERT( m_pParentDragFolder );

		LinkItem*	pDragLinkItem = (LinkItem*)GetItemData(htDragItem);
		int nIndex = -1;
		int nCount = (int)m_pParentDragFolder->size();
		for (int i = 0; i < nCount; ++i) {
			if (m_pParentDragFolder->at(i).get() == pDragLinkItem) {
				nIndex = i;
				break;
			}
		}
		ATLASSERT( nIndex != -1 );

		CComPtr<IDataObject> spDataObject = CLinkItemDataObject::CreateInstance(m_pParentDragFolder, nIndex);
		if (spDataObject) {
			SelectItem(htDragItem);
			Expand(htDragItem, TVE_COLLAPSE);
			m_hItemDragging 		= htDragItem;
			DROPEFFECT	dropEffect	= DoDragDrop(spDataObject, DROPEFFECT_MOVE | DROPEFFECT_COPY | DROPEFFECT_LINK);
			if (dropEffect == (DROPEFFECT_MOVE | DROPEFFECT_LINK)) {
				if (static_cast<int>(m_pParentDragFolder->size()) <= nIndex || m_pParentDragFolder->at(nIndex).get() != pDragLinkItem)
					DeleteItem(m_hItemDragging);	// �O���ֈړ����ꂽ�̂ō폜
			}
			m_hItemDragging 		= NULL;
		}
		m_pParentDragFolder = nullptr;
	} else {
		if (GetItemState(htDragItem, TVIS_EXPANDED) != 0) {
			Expand(htDragItem, TVE_COLLAPSE);
		} else {
			Select(htDragItem, TVGN_CARET);
			Expand(htDragItem, TVE_EXPAND);	// required. cuz if already selected, Select(hTreeItem, TVGN_CARET) does nothing.
		}

		auto pLinkItem = (LinkItem*)GetItemData(htDragItem);
		ATLASSERT( pLinkItem );
		if (m_funcItemClick) {
			m_funcItemClick(pLinkItem);
		} else if (pLinkItem->pFolder == nullptr) {
			/// �I�����ꂽ�����N���J��
			CLinkPopupMenu::OpenLink(*pLinkItem, DonutGetStdOpenFlag());

		} else {
			::SetFocus(m_hWnd); 	// set focus later
		}
	}
}



LinkFolderPtr	CDonutFavoriteTreeView::GetParentLinkFolderPtr(HTREEITEM htItem)
{
	HTREEITEM htParent = GetParentItem(htItem);
	if (htParent == NULL) {
		return CRootFavoritePopupMenu::GetBookmarkListPtr();
	} else {
		LinkItem* pLink = (LinkItem*)GetItemData(htParent);
		return pLink->pFolder;
	}
}

// IDropTargetImpl

bool CDonutFavoriteTreeView::OnScroll(UINT nScrollCode, UINT nPos, bool bDoScroll /*= true*/)
{
	bool bResult = false;

	if (HIBYTE(nScrollCode) == SB_LINEUP) {
		if (bDoScroll)
			bResult = _ScrollItem(false);
		else
			bResult = _CanScrollItem(false);
	} else if (HIBYTE(nScrollCode) == SB_LINEDOWN) {
		if (bDoScroll)
			bResult = _ScrollItem(true);
		else
			bResult = _CanScrollItem(true);
	}

	return bResult;
}



bool CDonutFavoriteTreeView::_ScrollItem(bool bDown)
{
	CRect rc;
	GetClientRect(&rc);

	if (bDown) {
		SendMessage(WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), 0);
		return true;
	} else {
		SendMessage(WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), 0);
		return true;
	}
}


bool CDonutFavoriteTreeView::_CanScrollItem(bool bDown)
{
	if (bDown) {
		CRect rc;
		GetClientRect(&rc);

		UINT flags = 0;
		HTREEITEM htHit = HitTest(CPoint(rc.left + 1, rc.bottom - 1), &flags);
		return htHit != NULL;
	} else {
		return GetRootItem() != GetFirstVisibleItem();
	}
}


DROPEFFECT CDonutFavoriteTreeView::OnDragEnter(IDataObject *pDataObject, DWORD dwKeyState, CPoint point)
{
	m_bAcceptDrag = m_bDragItemIsLinkFile = CLinkItemDataObject::IsAcceptDrag(pDataObject);

	FORMATETC fmt = { 0 };
	fmt.cfFormat	= CF_DONUTLINKITEM;
	if (SUCCEEDED(pDataObject->QueryGetData(&fmt))) {
		CLinkItemDataObject*	pLinkItem;
		pDataObject->QueryInterface(IID_NULL, (void**)&pLinkItem);
		m_DragItemData = pLinkItem->GetFolderAndIndex();
		m_bAcceptDrag = true;
		m_bDragItemIsLinkFile = false;
		return _MtlStandardDropEffect(dwKeyState);
	}

	return DROPEFFECT_NONE;
}

bool CDonutFavoriteTreeView::_IsFolderMargin(HTREEITEM htFolder, CPoint pt)
{
	CRect rcItem;
	GetItemRect(htFolder, &rcItem, FALSE);
	rcItem.top	+= kDragFolderMargin;
	rcItem.bottom -= kDragFolderMargin;
	if (rcItem.PtInRect(pt))
		return false;
	else
		return true;
}

DROPEFFECT CDonutFavoriteTreeView::OnDragOver(IDataObject *pDataObject, DWORD dwKeyState, CPoint point, DROPEFFECT dropOkEffect)
{
	if (m_bAcceptDrag == false)
		return DROPEFFECT_NONE;

	UINT flags = 0;
	HTREEITEM htHit = HitTest(point, &flags);
	if (htHit) {
		LinkItem*	pHitLinkItem = (LinkItem*)GetItemData(htHit);
		auto pParentLinkFolder = GetParentLinkFolderPtr(htHit);
		int nIndex = -1;
		int nCount = (int)pParentLinkFolder->size();
		for (int i = 0; i < nCount; ++i) {
			if (pParentLinkFolder->at(i).get() == pHitLinkItem) {
				nIndex = i;
				break;
			}
		}
		ATLASSERT( nIndex != -1 );
		if (pParentLinkFolder == m_DragItemData.first && nIndex == m_DragItemData.second) {
			KillTimer(kDragOverExpandTimerId);
			RemoveInsertMark();
			return DROPEFFECT_NONE;	// �������g�Ȃ̂ŋA��
		}

		if (pHitLinkItem->pFolder && _IsFolderMargin(htHit, point) == false) {
			SelectDropTarget(htHit);
			if (m_htLastDragOver != htHit)
				SetTimer(kDragOverExpandTimerId, kDragOverExpandTimerInterval);
			m_htLastDragOver = htHit;
			RemoveInsertMark();
			
		} else {
			m_htLastDragOver = NULL;
			KillTimer(kDragOverExpandTimerId);
			SelectDropTarget(NULL);

			CRect rcItem;
			GetItemRect(htHit, &rcItem, FALSE);
			rcItem.bottom	-= (rcItem.Height() / 2);
			if (rcItem.PtInRect(point)) {
				SetInsertMark(htHit, FALSE);
			} else {
				SetInsertMark(htHit, TRUE);
			}
		}
		
	} else {
		KillTimer(kDragOverExpandTimerId);
		SelectDropTarget(NULL);
		RemoveInsertMark();
	}
	return DROPEFFECT_MOVE;

}

DROPEFFECT CDonutFavoriteTreeView::OnDrop(IDataObject *pDataObject, DROPEFFECT dropEffect, DROPEFFECT dropEffectList, CPoint point)
{
	if (m_bAcceptDrag == false)
		return DROPEFFECT_NONE;

	KillTimer(kDragOverExpandTimerId);
	RemoveInsertMark();

	auto nameUrl = CLinkItemDataObject::GetNameAndURL(pDataObject);

	LinkItem* pLinkItemForFavicon = nullptr;
	unique_ptr<LinkItem> pItem;
	if (m_bDragItemIsLinkFile) {
		pItem.reset(new LinkItem);
		pItem->strName	= nameUrl.first;
		pItem->strUrl	= nameUrl.second;
		pLinkItemForFavicon = pItem.get();
		//pItem->icon = CFaviconManager::GetFaviconFromURL(pItem->strUrl);
	} else {
		pItem.reset(m_DragItemData.first->at(m_DragItemData.second).release());
	}

	UINT flags = 0;
	HTREEITEM htHit = HitTest(point, &flags);
	if (htHit) {
		LinkItem*	pHitLinkItem = (LinkItem*)GetItemData(htHit);
		auto pParentLinkFolder = GetParentLinkFolderPtr(htHit);
		int nIndex = -1;
		int nCount = (int)pParentLinkFolder->size();
		for (int i = 0; i < nCount; ++i) {
			if (pParentLinkFolder->at(i).get() == pHitLinkItem) {
				nIndex = i;
				break;
			}
		}
		ATLASSERT( nIndex != -1 );
		LinkItem*	pInsertItem = pItem.get();

		if (pHitLinkItem->pFolder && _IsFolderMargin(htHit, point) == false) {
			pHitLinkItem->pFolder->push_back(std::move(pItem));
			// �������g����h���b�O���ꂽ�̂Ńh���b�O���������
			if (m_bDragItemIsLinkFile == false)
				m_DragItemData.first->erase(m_DragItemData.first->begin() + m_DragItemData.second);
			else
				CLinkPopupMenu::GetFaviconToLinkItem(pLinkItemForFavicon->strUrl, pHitLinkItem->pFolder, pLinkItemForFavicon, m_hWnd);

			// �W�J����Ă���ꍇ�̂ݒǉ�����
			if (GetItemState(htHit, TVIS_EXPANDED) != 0) {
				SelectItem(_AddTreeItem(htHit, TVI_LAST, pInsertItem));
			} else {
				// �q��S������
				while (HTREEITEM htItem = GetChildItem(htHit))
					DeleteItem(htItem);
			}

			if (m_bDragItemIsLinkFile == false)
				DeleteItem(m_hItemDragging);	// �c���[�������

		} else {
			// �}���|�C���g��������
			int nInsertPos = 0;
			HTREEITEM htParent = GetParentItem(htHit);
			HTREEITEM htItem = GetChildItem(htParent);
			HTREEITEM htInsert = NULL;
			do {
				CRect rcItem;
				GetItemRect(htItem, &rcItem, FALSE);
				if (rcItem.PtInRect(point)) {
					rcItem.bottom	-= (rcItem.Height() / 2);
					if (rcItem.PtInRect(point) == false) {
						++nInsertPos;
						htInsert = _AddTreeItem(htParent, htItem, pInsertItem);
					} else {
						htItem = GetPrevSiblingItem(htItem);
						if (htItem == NULL)
							htItem = TVI_FIRST;
						htInsert = _AddTreeItem(htParent, htItem, pInsertItem);
					}
					break;					
				}
				++nInsertPos;
			} while (htItem = GetNextSiblingItem(htItem));

			SelectItem(htInsert);

			// �����T�u���j���[���̃A�C�e���̈ړ�
			if (pParentLinkFolder == m_DragItemData.first) {
				if (m_DragItemData.second < nInsertPos) {	// �h���b�O���Ă�A�C�e���̉E���ɑ}�������
					pParentLinkFolder->insert(pParentLinkFolder->begin() + nInsertPos, std::move(pItem));
					pParentLinkFolder->erase(pParentLinkFolder->begin() + m_DragItemData.second);
				} else {
					pParentLinkFolder->insert(pParentLinkFolder->begin() + nInsertPos, std::move(pItem));
					pParentLinkFolder->erase(pParentLinkFolder->begin() + m_DragItemData.second + 1);	// ��������̂ł��炷
				}
			} else {
				// �ǉ�
				pParentLinkFolder->insert(pParentLinkFolder->begin() + nInsertPos, std::move(pItem));

				// �������g����h���b�O���ꂽ�̂Ńh���b�O���������
				if (m_bDragItemIsLinkFile == false)
					m_DragItemData.first->erase(m_DragItemData.first->begin() + m_DragItemData.second);
			}

			// Favicon���擾
			if (m_bDragItemIsLinkFile) 
				CLinkPopupMenu::GetFaviconToLinkItem(pLinkItemForFavicon->strUrl, pParentLinkFolder, pLinkItemForFavicon, m_hWnd);

			if (m_bDragItemIsLinkFile == false && m_hItemDragging)
				DeleteItem(m_hItemDragging);	// �c���[�������
		}
	} else {
		// ���[�g�t�H���_�̍Ō�ɒǉ�
		auto pRootLinkFolder = CRootFavoritePopupMenu::GetBookmarkListPtr();

		// �������g����h���b�O���ꂽ�̂Ńh���b�O���������
		if (m_bDragItemIsLinkFile == false) {
			m_DragItemData.first->erase(m_DragItemData.first->begin() + m_DragItemData.second);
		} else {
			CLinkPopupMenu::GetFaviconToLinkItem(pLinkItemForFavicon->strUrl, pRootLinkFolder, pLinkItemForFavicon, m_hWnd);
		}
		LinkItem*	pInsertItem = pItem.get();
		pRootLinkFolder->push_back(std::move(pItem));

		SelectItem(_AddTreeItem(TVI_ROOT, TVI_LAST, pInsertItem));

		if (m_bDragItemIsLinkFile == false && m_hItemDragging)
			DeleteItem(m_hItemDragging);	// �c���[�������
	}
	// �ύX��ۑ�
	CRootFavoritePopupMenu::SaveFavoriteBookmark();

	return DROPEFFECT_MOVE;
}

void	CDonutFavoriteTreeView::OnDragLeave()
{
	KillTimer(kDragOverExpandTimerId);
	SelectDropTarget(NULL);
	RemoveInsertMark();
}


/// �E�N���b�N���j���[�̕\��
LRESULT CDonutFavoriteTreeView::OnTreeItemRClick(LPNMHDR pnmh)
{
	CPoint pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);
	UINT flags = 0;
	HTREEITEM htHit = HitTest(pt, &flags);
	if (htHit) {
		LinkFolderPtr pParentLinkFolder = GetParentLinkFolderPtr(htHit);
		LinkItem* pLinkItem = (LinkItem*)GetItemData(htHit);
		int nCmd = CLinkPopupMenu::ShowRClickMenuAndExecCommand(pParentLinkFolder, pLinkItem, m_hWnd);
		switch (nCmd) {
		case ID_EDITLINKITEM:
		case ID_EDITEXPROPERTY:
		case ID_EDITFOLDERNAME:
			SetItemText(htHit, pLinkItem->strName);
			break;

		case ID_DELETELINKITEM:
			DeleteItem(htHit);
			break;

		case ID_UNDOMENUITEM:
		case ID_ADDLINK:
		case ID_ADDFOLDER:
		case ID_SORTBYNAME:
			_RefreshTree();
			break;
		}
	}
	return TRUE;
}

