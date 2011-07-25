// DownloadingListView.cpp

#include "stdafx.h"
#include "DownloadingListView.h"
#include "DownloadedListView.h"
#include "DownloadOptionDialog.h"


// Constructor/Destructor
CDownloadingListView::CDownloadingListView() : m_bTimer(false)
{ }

CDownloadingListView::~CDownloadingListView()
{
	m_ImageList.Destroy();
	m_ImgStop.Destroy();
}


void	CDownloadingListView::DoPaint(CDCHandle dc)
{
	CPoint ptOffset;
	GetScrollOffset(ptOffset);

	CRect rcClient;
	GetClientRect(rcClient);
	rcClient.bottom	+= ptOffset.y;

	CMemoryDC	memDC(dc, rcClient);
	HFONT hOldFont = memDC.SelectFont(m_Font);

	// �w�i��`��
	memDC.FillSolidRect(rcClient, RGB(255, 255, 255));

	std::size_t	size = m_vecpDLItem.size();
	for (auto it = m_vecpDLItem.begin(); it != m_vecpDLItem.end(); ++it) {
		CRect rcItem = (*it)->rcItem;
		rcItem.right = rcClient.right;

		memDC.SetBkMode(TRANSPARENT);
		if ((*it)->dwState & DLITEMSTATE_SELECTED) {
			static COLORREF SelectColor = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
			memDC.SetTextColor(SelectColor);
		} else {
			memDC.SetTextColor(0);
		}

		// �I���`��
		if ((*it)->dwState & DLITEMSTATE_SELECTED){
			memDC.FillRect(&rcItem, COLOR_HIGHLIGHT);
		}

		// �A�C�R���`��
		CRect rcImage = rcItem;
		CPoint ptImg(cxImageMargin, rcImage.top + cyImageMargin);
		m_ImageList.Draw(memDC, (*it)->nImgIndex, ptImg, ILD_NORMAL);

		// �t�@�C������`��
		CRect rcFileName = rcItem;
		rcFileName.left = cxFileNameMargin;
		rcFileName.top += cyFileNameMargin;
		memDC.DrawText((*it)->strFileName, -1, rcFileName, DT_SINGLELINE);

		// progress
		CRect rcProgress(cxFileNameMargin, rcItem.top + cyProgressMargin, rcItem.right - cleftProgressMargin, rcItem.top + cyProgressMargin + ProgressHeight);
		if (IsThemeNull() == false) {
			static int PROGRESSBAR  = _CheckOsVersion_VistaLater() ? PP_TRANSPARENTBAR : PP_BAR;
			static int PROGRESSBODY = _CheckOsVersion_VistaLater() ? PP_FILL		   : PP_CHUNK;
			if (IsThemeBackgroundPartiallyTransparent(PROGRESSBAR, 0))
				DrawThemeParentBackground(memDC, rcProgress);
			DrawThemeBackground(memDC, PROGRESSBAR, 0, rcProgress);
			CRect rcContent;
			GetThemeBackgroundContentRect(memDC, PROGRESSBAR, 0, rcProgress, rcContent);

			double Propotion = double((*it)->nProgress) / double((*it)->nProgressMax);
			int nPos = (int)((double)rcContent.Width() * Propotion);
			rcContent.right = rcContent.left + nPos;
			DrawThemeBackground(memDC, PROGRESSBODY, 0, rcContent);
		} else {
			memDC.DrawEdge(rcProgress, EDGE_RAISED, BF_ADJUST | BF_MONO | BF_RECT | BF_MIDDLE);
			double Propotion = double((*it)->nProgress) / double((*it)->nProgressMax);
			int nPos = (int)((double)rcProgress.Width() * Propotion);
			rcProgress.right = rcProgress.left + nPos;
			memDC.FillSolidRect(rcProgress, RGB(0, 255, 0));
		}

		// �����`��
		CRect rcDiscribe = rcItem;
		rcDiscribe.top += cyProgressMargin + ProgressHeight;
		rcDiscribe.left = cxFileNameMargin;
		memDC.DrawText((*it)->strText, -1, rcDiscribe, DT_SINGLELINE);

		// ��~�A�C�R���`��
		CPoint ptStop(rcClient.right - 23, rcItem.top + cyProgressMargin - 2);
		m_ImgStop.Draw(memDC, 0, ptStop, ILD_NORMAL);

		// ���Ƀ��C��������
		static COLORREF BorderColor = ::GetSysColor(COLOR_3DLIGHT);
		HPEN hPen = ::CreatePen(PS_SOLID, 1, BorderColor);
		HPEN hOldPen = memDC.SelectPen(hPen);
		memDC.MoveTo(CPoint(rcItem.left, rcItem.bottom));
		memDC.LineTo(rcItem.right, rcItem.bottom);
		memDC.SelectPen(hOldPen);
		::DeleteObject(hPen);
	}

	dc.SelectFont(hOldFont);
}

// ���X�g�r���[�ɒǉ�
void	CDownloadingListView::_AddItemToList(DLItem* pItem)
{
	// �擪�ɃA�C�e����ǉ�
	m_vecpDLItem.insert(m_vecpDLItem.begin(), std::unique_ptr<DLItem>(pItem));

	// �A�C�R����ǉ�
	_AddIcon(pItem);

	_RefreshList();
}

// �A�C�e���̍��[�ɒu���A�C�R����ǂݍ���
void CDownloadingListView::_AddIcon(DLItem *pItem)
{
	CString strExt = Misc::GetFileExt(pItem->strFileName);
	if (strExt.IsEmpty()) {
		pItem->nImgIndex = 0;
	} else {
		auto it = m_mapImgIndex.find(strExt);
		if (it != m_mapImgIndex.end()) {	// ��������
			pItem->nImgIndex = it->second;
		} else {							// ������Ȃ�����
			SHFILEINFO sfinfo;
			CString strFind;
			strFind.Format(_T("*.%s"), strExt);
			// �A�C�R�����擾
			::SHGetFileInfo(strFind, FILE_ATTRIBUTE_NORMAL, &sfinfo, sizeof(SHFILEINFO)
				, SHGFI_ICON | SHGFI_USEFILEATTRIBUTES);
			ATLASSERT(sfinfo.hIcon);
			// �C���[�W���X�g�ɃA�C�R����ǉ�
			int nImgIndex = m_ImageList.AddIcon(sfinfo.hIcon);
			// �g���q�ƃC���[�W���X�g�̃C���f�b�N�X���֘A�t����
			m_mapImgIndex.insert(std::pair<CString, int>(strExt, nImgIndex));
			::DestroyIcon(sfinfo.hIcon);

			pItem->nImgIndex = nImgIndex;
		}
	}
}



/////////////////////////////////////////////////////////////////////////////
// Message map

int CDownloadingListView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    LRESULT lRet = DefWindowProc();

	// �����X�N���[���o�[���폜
	ModifyStyle(WS_HSCROLL, 0);

	// ��ʍX�V�p�̃^�C�}�[���N��
	//SetTimer(1, 1000);

	static bool bInit = false;
	if (bInit)
		return 0;
	bInit = true;

	SetScrollSize(10, 10);
    SetScrollLine(10, 10);
    //SetScrollPage(100, 100);

	// �C���[�W���X�g�쐬
	m_ImageList.Create(32, 32, ILC_COLOR32 | ILC_MASK, 20, 1);

	// �f�t�H���g�̃A�C�R����ǂݍ���
	SHFILEINFO sfinfo;
	::SHGetFileInfo(_T("*.*"), FILE_ATTRIBUTE_NORMAL, &sfinfo, sizeof(SHFILEINFO)
		, SHGFI_ICON | SHGFI_USEFILEATTRIBUTES);
	int nImgIndex = m_ImageList.AddIcon(sfinfo.hIcon);	/* 0 */
	::DestroyIcon(sfinfo.hIcon);

	// ��~�A�C�R����ǂݍ���
	m_ImgStop.Create(16, 16, ILC_COLOR32 | ILC_MASK, 1, 1);
	HICON hStopIcon = AtlLoadIcon(IDI_DLSTOP);
	m_ImgStop.AddIcon(hStopIcon);
	::DestroyIcon(hStopIcon);

	// �t�H���g��ݒ�
	CLogFont	logfont;
	logfont.SetMenuFont();
	m_Font = logfont.CreateFontIndirectW();

	// �c�[���`�b�v��ݒ�
	m_ToolTip.Create(m_hWnd);
	m_ToolTip.ModifyStyle(0, TTS_ALWAYSTIP);
	CToolInfo ti(TTF_SUBCLASS, m_hWnd);
	ti.hwnd = m_hWnd;
	m_ToolTip.AddTool(ti);
	m_ToolTip.Activate(TRUE);
	m_ToolTip.SetDelayTime(TTDT_AUTOPOP, 30 * 1000);
	m_ToolTip.SetMaxTipWidth(500);

	OpenThemeData(L"PROGRESS");

    return lRet;
}

void CDownloadingListView::OnDestroy()
{
	KillTimer(1);
}
// �T�C�Y�ɍ��킹�ăv���O���X�o�[�̑傫����ύX����
void CDownloadingListView::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);

	DefWindowProc();

	if (size != CSize(0, 0)) {
		CSize	sizeWnd;
		GetScrollSize(sizeWnd);
		sizeWnd.cx = size.cx == 0 ? 1 : size.cx;

		SetScrollSize(sizeWnd, FALSE, false);
		SetScrollLine(10, 10);
	}
	
}

void CDownloadingListView::OnLButtonDown(UINT nFlags, CPoint point)
{
	CPoint ptOffset;
	GetScrollOffset(ptOffset);

	CRect rcClient;
	GetClientRect(rcClient);

	CPoint pt = point;
	pt.y += ptOffset.y;

	int i = 0;
	for (auto it = m_vecpDLItem.cbegin(); it != m_vecpDLItem.cend(); ++it, ++i) {
		DLItem& item = *(*it);
		if (item.dwState & DLITEMSTATE_SELECTED) {
			item.dwState &= ~DLITEMSTATE_SELECTED;
		}
		if (item.rcItem.PtInRect(pt)) {
			CRect rcStop(CPoint(rcClient.right - 23, (ItemHeight + UnderLineHeight) * i + cyProgressMargin - 2), CSize(16, 16));
			if (rcStop.PtInRect(pt)) {
				item.bAbort = true;	// ��~����
			} else {
				item.dwState |= DLITEMSTATE_SELECTED;
			}
		}
	}

	Invalidate(FALSE);
}


void CDownloadingListView::OnTimer(UINT_PTR nIDEvent)
{		
	static DWORD dwTime = ::timeGetTime();
	DWORD dwNowTime = ::timeGetTime();
	DWORD dwTimeMargin = dwNowTime - dwTime;
	if (dwTimeMargin <= 0) {
		return;
	} else {
		dwTime = dwNowTime;
	}

	if (nIDEvent == 1) {
		// SS_ICON
		int	nMaxTotalSecondTime = 0;
		for (auto it = m_vecpDLItem.begin(); it != m_vecpDLItem.end(); ++it) {
			DLItem& item = *(*it);

			// (1.2 MB/sec)
			CString strTransferRate;
			int nProgressMargin = item.nProgress - item.nOldProgress;
			item.nOldProgress = item.nProgress;

			if (item.deq.size() >= 10)
				item.deq.pop_front();
			item.deq.push_back(make_pair(nProgressMargin, (int)dwTimeMargin));
			nProgressMargin = 0;
			int nTotalTime = 0;
			for (auto itq = item.deq.cbegin(); itq != item.deq.cend(); ++itq) {
				nProgressMargin += itq->first;
				nTotalTime		+= itq->second;
			}
			
			int KbTransferRate = nProgressMargin / nTotalTime;	// kbyte / second
			double MbTransferRate = (double)KbTransferRate / 1000.0;
			if (MbTransferRate > 1) {
				::swprintf(strTransferRate.GetBuffer(30), _T(" (%.1lf MB/sec)"), MbTransferRate);
				strTransferRate.ReleaseBuffer();
			} else {
				::swprintf(strTransferRate.GetBuffer(30), _T(" (%d KB/sec)"), KbTransferRate);
				strTransferRate.ReleaseBuffer();
			}
			

			// �c�� 4 ��
			int nRestByte = item.nProgressMax - item.nProgress;
			if (nRestByte > 0) {
				if (KbTransferRate > 0) {
					int nTotalSecondTime = (nRestByte / 1000) / KbTransferRate;	// �c�莞��(�b)
					if (nMaxTotalSecondTime < nTotalSecondTime)
						nMaxTotalSecondTime = nTotalSecondTime;
					int nhourTime	= nTotalSecondTime / (60 * 60);									// ����
					int nMinTime	= (nTotalSecondTime - (nhourTime * (60 * 60))) / 60;			// ��
					int nSecondTime = nTotalSecondTime - (nhourTime * (60 * 60)) - (nMinTime * 60);	// �b
					(*it)->strText = _T("�c�� ");
					if (nhourTime > 0) {
						(*it)->strText.Append(nhourTime) += _T(" ���� ");
					}
					if (nMinTime > 0) {
						(*it)->strText.Append(nMinTime) += _T(" �� ");
					}
					(*it)->strText.Append(nSecondTime) += _T(" �b �\ ");


					// 10.5 / 288 MB
					CString strDownloaded;
					int nMByte = item.nProgressMax / (1000 * 1000);
					if (nMByte > 0) {
						double DownloadMByte = (double)item.nProgress / (1000 * 1000);
						::swprintf(strDownloaded.GetBuffer(30), _T("%.1lf / %d MB"), DownloadMByte, nMByte);
						strDownloaded.ReleaseBuffer();
					} else {
						int nKByte = (*it)->nProgressMax / 1000;
						double DownloadKByte = (double)item.nProgress / 1000;
						::swprintf(strDownloaded.GetBuffer(30), _T("%.1lf / %d KB"), DownloadKByte, nKByte);
						strDownloaded.ReleaseBuffer();
					}
					item.strText += strDownloaded + strTransferRate;
				}
			}
		}
		if (nMaxTotalSecondTime > 0) {
			CString strTitle = _T("�c�� ");
			int nhourTime	= nMaxTotalSecondTime / (60 * 60);									// ����
			int nMinTime	= (nMaxTotalSecondTime - (nhourTime * (60 * 60))) / 60;			// ��
			int nSecondTime = nMaxTotalSecondTime - (nhourTime * (60 * 60)) - (nMinTime * 60);	// �b
			if (nhourTime > 0) {
				strTitle.Append(nhourTime) += _T(" ���� ");
			}
			if (nMinTime > 0) {
				strTitle.Append(nMinTime) += _T(" �� ");
			}
			strTitle.Append(nSecondTime) += _T(" �b");
			GetTopLevelWindow().SetWindowText(strTitle);
		} else {
			GetTopLevelWindow().SetWindowText(NULL);
		}
		Invalidate(FALSE);

	}
}
// ���X�g����폜����
LRESULT CDownloadingListView::OnRemoveFromList(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	DLItem* pItem = (DLItem*)wParam;
	CCustomBindStatusCallBack* pbscb = (CCustomBindStatusCallBack*)lParam;
	//if (pItem->bAbort == false)
	//	delete pbscb;
	//pbscb->Release();
	
	// vector����폜����
	for (auto it = m_vecpDLItem.begin(); it != m_vecpDLItem.end(); ++it) {
		if ((*it).get() == pItem) {
			(*it).release();
			m_vecpDLItem.erase(it);
			break;
		}
	}

	_RefreshList();

	// DLedView�ɒǉ�
	m_funcAddDownloadedItem(pItem);

	if (m_vecpDLItem.size() == 0) {
		KillTimer(1);		// ��ʍX�V�^�C�}�[���~
		m_bTimer = false;
		if (CDLOptions::bCloseAfterAllDL) {	// �S�Ă�DL���I������̂ŕ���
			GetTopLevelWindow().SetWindowText(NULL);
			::PostMessage(GetTopLevelParent(), WM_CLOSE, 0, 0);
		}
	}

	/* �G�N�X�v���[���[�Ƀt�@�C���̕ύX�ʒm */
	::SHChangeNotify(SHCNE_RENAMEITEM, SHCNF_PATH, static_cast<LPCTSTR>(pItem->strFilePath + _T(".incomplete")), static_cast<LPCTSTR>(pItem->strFilePath));

	return 0;
}

LRESULT CDownloadingListView::OnAddToList(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (m_bTimer == false) {	
		SetTimer(1, 1000);		// ��ʍX�V�^�C�}�[���J�n
		m_bTimer = true;
	}
	_AddItemToList((DLItem*)wParam);
	return 0;
}

LRESULT CDownloadingListView::OnTooltipGetDispInfo(LPNMHDR pnmh)
{
    LPNMTTDISPINFO pntdi = (LPNMTTDISPINFO)pnmh;

    if (pntdi->uFlags & TTF_IDISHWND) {
		POINT pt;
		GetCursorPos(&pt);
		ScreenToClient(&pt);
		int nIndex = _HitTest(pt);
		if (nIndex != -1) {
			pntdi->lpszText = m_vecpDLItem[nIndex]->strURL.GetBuffer(0);
		} else {
			pntdi->lpszText = NULL;
		}
    }

    return 0;
}

void CDownloadingListView::OnMouseMove(UINT nFlags, CPoint point)
{
	SetMsgHandled(FALSE);
	static int nOldIndex = -1;
	int nIndex = _HitTest(point);
	if (nIndex != nOldIndex) {
		nOldIndex = nIndex;
		m_ToolTip.Activate(FALSE);
		m_ToolTip.Activate(TRUE);
	}
}



// ���X�g���X�V����
void	CDownloadingListView::_RefreshList()
{
	CRect rcClient;
	GetClientRect(rcClient);

	CPoint	ptOffset;
	GetScrollOffset(ptOffset);

	int cySize = 0;
	std::size_t	size = m_vecpDLItem.size();
	for (auto it = m_vecpDLItem.begin(); it != m_vecpDLItem.end(); ++it) {
		// �A�C�e���̈ʒu��ݒ�
		(*it)->rcItem.top = cySize;
		cySize += ItemHeight;
		(*it)->rcItem.bottom = cySize;
		cySize += UnderLineHeight;

		(*it)->rcItem.right = ItemMinWidth;
	}

	CSize	sizeWnd;
	GetScrollSize(sizeWnd);
	sizeWnd.cy = (cySize == 0) ? 1 : cySize;
	SetScrollSize(sizeWnd, FALSE, false);
	SetScrollLine(10, 10);

	Invalidate(FALSE);
}


int		CDownloadingListView::_HitTest(CPoint pt)
{
	int nCount = (int)m_vecpDLItem.size();
	for (int i = 0; i < nCount; ++i) {
		if (m_vecpDLItem[i]->rcItem.PtInRect(pt)) 
			return i;
	}
	return -1;
}







