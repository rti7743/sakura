/*
	Copyright (C) 2016, Moca

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"
#include "dlg/CDlgColorMarker.h"
#include "dlg/CDlgOpenFile.h"
#include "env/CSakuraEnvironment.h"


/*! カラーマーカー設定
	選択範囲内をマーカーでする
	nIndex:0 最後に使った物
	nIndex:1-10 セット1-10
*/
BOOL CViewCommander::Command_SETCOLORMARKER_SET(int nIndex)
{
	CMarkerItem item;
	Setting_ColorMarker& markerSet = GetDllShareData().m_Common.m_sSearch.m_sColorMarker;
	if( nIndex == 0 ){
		item = markerSet.m_ColorItemLast;
	}else if( 0 < nIndex && nIndex <= _countof(markerSet.m_ColorItems) ){
		item = markerSet.m_ColorItems[nIndex - 1];
		markerSet.m_ColorItemLast = item;
	}else{
		ErrorBeep();
		return FALSE;
	}
	Command_SETCOLORMARKER(item);
	return TRUE;
}

/*! カラーマーカーセット文字列(マクロ)
	選択範囲内をマークする
*/
BOOL CViewCommander::Command_SETCOLORMARKER_STR(LPCWSTR pszMarker)
{
	CMarkerItem item;
	if( CColorMarkerVisitor().StrToMarkerItem2(1, pszMarker, item) ){
		Command_SETCOLORMARKER(item);
	}else{
		ErrorBeep();
		return FALSE;
	}
	return TRUE;
}

/*! カラーマーカーセット文字列(Sub)
	選択範囲内をマークする
*/
void CViewCommander::Command_SETCOLORMARKER(const CMarkerItem& marker)
{
	CMarkerItem item = marker;
	bool bLineMode = ((item.m_nGyouLine & 0x03) != 0);
	if( bLineMode ){
		item.m_nBegin = 0;
		item.m_nEnd = 0;
		// 未選択でも行モードは許可
		if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
			// カーソル行を登録
			Command_SETCOLORMARKER_ITEM(item, GetCaret().GetCaretLogicPos().y, false);
			GetDocument()->m_pcEditWnd->Views_Redraw();
			return;
		}
	}else{
		if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
			return; // 未選択
		}
	}
	CLayoutRange rgSelect = GetSelect();
	if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
		CLayoutRect rcSelect;
		TwoPointToRect(&rcSelect, rgSelect.GetFrom(), rgSelect.GetTo());
		CLogicYInt nLinePrev = CLogicYInt(-1);
		for( CLayoutYInt nLineNum = rcSelect.top; nLineNum <= rcSelect.bottom; nLineNum++ ){
			CLogicPoint ptLogic;
			GetDocument()->m_cLayoutMgr.LayoutToLogic(CLayoutPoint(rcSelect.left, nLineNum), &ptLogic);
			if( !bLineMode ){
				item.m_nBegin = ptLogic.x;
			}
			GetDocument()->m_cLayoutMgr.LayoutToLogic(CLayoutPoint(rcSelect.right, nLineNum), &ptLogic);
			if( !bLineMode ){
				item.m_nEnd = ptLogic.x;
			}
			if( item.m_nBegin != item.m_nEnd ){
				if( nLinePrev == ptLogic.y ){
					continue; // 折り返し行の同じロジック行は重複させない
				}
				nLinePrev = ptLogic.y;
				Command_SETCOLORMARKER_ITEM(item, ptLogic.y, false);
			}
		}
	}else{
		CLayoutRect rc;
		TwoPointToRect(&rc, rgSelect.GetFrom(), rgSelect.GetTo());
		CLayoutRange rgFixedSelect(CLayoutPoint(rc.left, rc.top), CLayoutPoint(rc.right, rc.bottom));
		CLogicRange rgLogicRange;
		GetDocument()->m_cLayoutMgr.LayoutToLogic(rgFixedSelect, &rgLogicRange);
		const CLogicYInt nBegin = rgLogicRange.GetFrom().y;
		const CLogicYInt nEnd = rgLogicRange.GetTo().y;
		CLogicYInt nEnd2 = nEnd;
		if( rgLogicRange.GetFrom().y != rgLogicRange.GetTo().y
			 && rgLogicRange.GetTo().x == CLogicXInt(0) ){
			nEnd2--; // 次の行頭まで選択だった場合は、前の行までにする
		}
		for( CLogicYInt nLineNum = nBegin; nLineNum <= nEnd2; nLineNum++ ){
			if( !bLineMode ){
				CLogicXInt nFromX = CLogicXInt(0);
				CLogicXInt nToX = CLogicXInt(-1);
				if( nLineNum == nBegin ){
					nFromX = rgLogicRange.GetFrom().x;
				}
				if( nLineNum == nEnd ){
					nToX = rgLogicRange.GetTo().x;
				}
				if( CLogicXInt(-1) == nToX ){
					const CDocLine* docLine = GetDocument()->m_cDocLineMgr.GetLine(nLineNum);
					if( docLine ){
						nToX = docLine->GetLengthWithEOL();
					}else{
						nToX = 0;
					}
				}
				if( nFromX == nToX ){
					// 選択範囲がEOLより右などで範囲がない場合は無視
					continue;
				}
				item.m_nBegin = nFromX;
				item.m_nEnd = nToX;
			}
			Command_SETCOLORMARKER_ITEM(item, nLineNum, false);
		}
	}
	GetDocument()->m_pcEditWnd->Views_Redraw();
}

BOOL CViewCommander::Command_SETCOLORMARKER_DIRECT(LPCWSTR pszMarker)
{
	CMarkerItem item;
	CLogicYInt nLineNum;
	if( CColorMarkerVisitor().StrToMarkerItem(1, pszMarker, item, nLineNum) ){
		return Command_SETCOLORMARKER_ITEM(item, nLineNum, true);
	}
	ErrorBeep();
	return FALSE;
}

/*! カラーマーカーセット(Sub)
	指定された位置を指定色でマークする
*/
BOOL CViewCommander::Command_SETCOLORMARKER_ITEM(const CMarkerItem& item, CLogicYInt nLineNum, bool bDirect)
{
	CDocLine* docLine = GetDocument()->m_cDocLineMgr.GetLine(nLineNum);
	if( docLine ){
		if( CColorMarkerVisitor().AddColorMarker(docLine, item) ){
			if( bDirect ){
				GetDocument()->m_pcEditWnd->Views_Redraw();
			}
			return TRUE;
		}
	}
	ErrorBeep();
	return FALSE;
}

/*! カラーマーカー検索設定(セットn)
*/
BOOL CViewCommander::Command_SETCOLORMARKER_SEARCH_SET(int nIndex)
{
	CMarkerItem item;
	Setting_ColorMarker& markerSet = GetDllShareData().m_Common.m_sSearch.m_sColorMarker;
	if( nIndex == 0 ){
		item = markerSet.m_ColorItemLast;
	}else if( 0 < nIndex && nIndex <= _countof(markerSet.m_ColorItems) ){
		item = markerSet.m_ColorItems[nIndex - 1];
		markerSet.m_ColorItemLast = item;
	}else{
		ErrorBeep();
		return FALSE;
	}
	Command_SETCOLORMARKER_SEARCH(item);
	return TRUE;
}

/*! カラーマーカー検索設定(マーカー)
*/
BOOL CViewCommander::Command_SETCOLORMARKER_SEARCH_STR(LPCWSTR pszMarker)
{
	CMarkerItem item;
	if( CColorMarkerVisitor().StrToMarkerItem2(1, pszMarker, item) ){
		Command_SETCOLORMARKER_SEARCH(item);
	}else{
		ErrorBeep();
		return FALSE;
	}
	return TRUE;
}

/*! カラーマーカー検索設定
*/
void CViewCommander::Command_SETCOLORMARKER_SEARCH(CMarkerItem& marker)
{
	// m_sSearchOption選択のための先に適用
	if( !m_pCommanderView->ChangeCurRegexp() ){
		return;
	}
	const bool bLineMode = ((marker.m_nGyouLine & 0x03) != 0);

	if( !bLineMode ){
		// 検索マークとマーカーがダブるためオフにする
		m_pCommanderView->m_bCurSrchKeyMark = false;
	}

	bool bDisplayUpdate = false;

	CDocLineMgr& rDocLineMgr = GetDocument()->m_cDocLineMgr;
	CLogicRange cSelectLogic;
	int nRet = CSearchAgent(&rDocLineMgr).SearchWord(
		CLogicPoint(CLogicXInt(0), CLogicYInt(0)), SEARCH_FORWARD,
		&cSelectLogic, m_pCommanderView->m_sSearchPattern
	);

	while( nRet != 0 ){
		CLogicPoint ptLogicCursor = cSelectLogic.GetTo();
		// 正規表現で0文字マッチの場合1文字にする
		if( cSelectLogic.GetFrom() == cSelectLogic.GetTo() ){
			const CDocLine* docLine = rDocLineMgr.GetLine(cSelectLogic.GetTo().y);
			CLogicXInt nPosAdd = CLogicXInt(0);
			if( docLine ){
				const wchar_t* pLine = docLine->GetPtr();
				nPosAdd = CNativeW::GetSizeOfChar(pLine, docLine->GetLengthWithEOL(), cSelectLogic.GetTo().x);
				if( nPosAdd < CLogicXInt(1) ){
					nPosAdd = CLogicXInt(1);
				}
			}
			ptLogicCursor.x += nPosAdd;
		}

		CDocLine* docLine = rDocLineMgr.GetLine(cSelectLogic.GetFrom().y);
		if( docLine ){
			CMarkerItem item = marker;
			if( bLineMode ){
				item.m_nBegin = CLogicXInt(0);
				item.m_nEnd = CLogicXInt(0);
			}else{
				item.m_nBegin = cSelectLogic.GetFrom().x;
				item.m_nEnd = ptLogicCursor.x;
			}
			CColorMarkerVisitor().AddColorMarker(docLine, item);
			if( bLineMode ){
				ptLogicCursor.y++;
				ptLogicCursor.x = CLogicXInt(0);
			}
		}
		
		nRet = CSearchAgent(&rDocLineMgr).SearchWord(
			ptLogicCursor, SEARCH_FORWARD,
			&cSelectLogic, m_pCommanderView->m_sSearchPattern
		);
	}
	GetDocument()->m_pcEditWnd->Views_Redraw();
}

static void CViewCommander_ColorMarkerDelItemPos(CDocLine* docLine, CLogicXInt nFromX, CLogicXInt nToX)
{
	const int nCount = CColorMarkerVisitor().GetColorMarkerCount(docLine);
	std::vector<CMarkerItem> vecIns; // ソート順の関係で後でまとめて登録
	int nIndexLast = -1;
	for( int i = nCount - 1; 0 <= i; i-- ){
		const CMarkerItem& item = *CColorMarkerVisitor().GetColorMarker(docLine, i);
		bool bDelete = false;
		if( nFromX <= item.m_nBegin ){
			if( item.m_nEnd <= nToX ){
				// [<>] :削除 []選択範囲 <>マーカー範囲
				bDelete = true;
			}else if( item.m_nBegin < nToX ){
				//[<] > :[]<> 右に縮小
				CMarkerItem ins = item;
				ins.m_nBegin = nToX;
				vecIns.push_back(ins);
				bDelete = true;
			}else{
				//[]<>:範囲外
			}
		}else if( nFromX < item.m_nEnd ){
			if( item.m_nEnd <= nToX ){
				//< [>] : <>[ ] 左に縮小
				CMarkerItem ins = item;
				ins.m_nEnd= nFromX;
				vecIns.push_back(ins);
				bDelete = true;
			}else{
				//< [] > : <>[]<> 左右分割
				CMarkerItem insBefore = item;
				CMarkerItem insAfter = item;
				insAfter.m_nBegin = nToX;
				insBefore.m_nEnd = nFromX;
				vecIns.push_back(insAfter);
				vecIns.push_back(insBefore);
				bDelete = true;
			}
		}else{
			// <>[] :範囲外
		}
		if( bDelete ){
			if( nIndexLast == -1 ){
				nIndexLast = i + 1;
			}
		}else if( nIndexLast != -1 ){
			CColorMarkerVisitor().DelColorMarker(docLine, i + 1, nIndexLast);
			nIndexLast = -1;
		}
	}
	if( nIndexLast != -1 ){
		CColorMarkerVisitor().DelColorMarker(docLine, 0, nIndexLast);
	}
	const int nInsCount = static_cast<int>(vecIns.size());
	// 後ろから追加したので逆順で挿入。これで同じ範囲のアイテムが登録順を保てる
	for( int i = nInsCount - 1; 0 <= i; i-- ){
		CColorMarkerVisitor().AddColorMarker(docLine, vecIns[i]);
	}
}

/*! カラーマーカー削除
*/
void CViewCommander::Command_DELCOLORMARKER(void)
{
	CLayoutRange rgSelect = GetSelect();
	if( m_pCommanderView->GetSelectionInfo().IsBoxSelecting() ){
		CLayoutRect rcSelect;
		TwoPointToRect(&rcSelect, rgSelect.GetFrom(), rgSelect.GetTo());
		CLogicYInt nLinePrev = CLogicYInt(-1);
		for( CLayoutYInt nLineNum = rcSelect.top; nLineNum <= rcSelect.bottom; nLineNum++ ){
			CLogicPoint ptLogic;
			GetDocument()->m_cLayoutMgr.LayoutToLogic(CLayoutPoint(rcSelect.left, nLineNum), &ptLogic);
			CLogicPoint ptLogicTo;
			GetDocument()->m_cLayoutMgr.LayoutToLogic(CLayoutPoint(rcSelect.right, nLineNum), &ptLogicTo);
			CDocLine* docLine = GetDocument()->m_cDocLineMgr.GetLine(ptLogic.y);
			if( docLine ){
				CViewCommander_ColorMarkerDelItemPos(docLine, ptLogic.x, ptLogicTo.x);
			}
		}
	}else{
		if( !m_pCommanderView->GetSelectionInfo().IsTextSelected() ){
			rgSelect.SetFrom(GetCaret().GetCaretLayoutPos());
			rgSelect.SetTo(GetCaret().GetCaretLayoutPos());
		}
		CLayoutRect rc;
		TwoPointToRect(&rc, rgSelect.GetFrom(), rgSelect.GetTo());
		CLayoutRange rgFixedSelect(CLayoutPoint(rc.left, rc.top), CLayoutPoint(rc.right, rc.bottom));
		CLogicRange rgLogicRange;
		GetDocument()->m_cLayoutMgr.LayoutToLogic(rgFixedSelect, &rgLogicRange);
		const CLogicYInt nBegin = rgLogicRange.GetFrom().y;
		const CLogicYInt nEnd = rgLogicRange.GetTo().y;
		CLogicYInt nEnd2 = nEnd;
		if( rgLogicRange.GetFrom().y != rgLogicRange.GetTo().y
			 && rgLogicRange.GetTo().x == CLogicXInt(0) ){
			nEnd2--; // 次の行頭まで選択だった場合は、前の行までにする
		}
		for( CLogicYInt nLineNum = nBegin; nLineNum <= nEnd2; nLineNum++ ){
			CDocLine* docLine = GetDocument()->m_cDocLineMgr.GetLine(nLineNum);
			if( docLine ){
				CLogicXInt nFromX = CLogicXInt(0);
				CLogicXInt nToX = CLogicXInt(-1);
				if( nLineNum == nBegin ){
					nFromX = rgLogicRange.GetFrom().x;
				}
				if( nLineNum == nEnd ){
					nToX = rgLogicRange.GetTo().x;
				}
				if( CLogicXInt(-1) == nToX ){
					nToX = CLogicXInt(INT_MAX);
				}
				CViewCommander_ColorMarkerDelItemPos(docLine, nFromX, nToX); 
			}
		}
	}
	GetDocument()->m_pcEditWnd->Views_Redraw();
}

/*! カラーマーカー詳細設定(ダイアログ)
*/
void CViewCommander::Command_DLGCOLORMARKER(void)
{
	CDlgColorMarker cDlg;
	cDlg.DoModal(G_AppInstance(), m_pCommanderView->GetHwnd(), (LPARAM)0, GetDocument(), false);
}

/*! 次のカラーマーカー
*/
void CViewCommander::Command_COLORMARKER_NEXT(void)
{
	CLogicPoint	ptXY = GetCaret().GetCaretLogicPos();
	CLogicPoint	ptOld = ptXY;

	for( int n = 0; n < 2; n++ ){
		if( CColorMarkerVisitor().SearchColorMarker(&GetDocument()->m_cDocLineMgr, ptXY, SEARCH_FORWARD, ptXY) ){
			CLayoutPoint ptLayout;
			GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY,&ptLayout);
			m_pCommanderView->MoveCursorSelecting(ptLayout,
				m_pCommanderView->GetSelectionInfo().m_bSelectingLock);
			if( ptXY < ptOld ){
				m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRNEXT1));
			}
			return;
		}
		if( !GetDllShareData().m_Common.m_sSearch.m_bSearchAll ){
			break;
		}
		ptXY.x = 0;
		ptXY.y = 0;
	}
	m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRNEXT2));
	AlertNotFound(m_pCommanderView->GetHwnd(), false, LS(STR_MARKER_NEXT_NOT_FOUND));
	return;
}

/*! 前のカラーマーカー
*/
void CViewCommander::Command_COLORMARKER_PREV(void)
{
	CLogicPoint	ptXY = GetCaret().GetCaretLogicPos();
	CLogicPoint	ptOld = ptXY;

	for( int n = 0; n < 2; n++ ){
		if( CColorMarkerVisitor().SearchColorMarker(&GetDocument()->m_cDocLineMgr, ptXY, SEARCH_BACKWARD, ptXY) ){
			CLayoutPoint ptLayout;
			GetDocument()->m_cLayoutMgr.LogicToLayout(ptXY,&ptLayout);
			m_pCommanderView->MoveCursorSelecting(ptLayout,
				m_pCommanderView->GetSelectionInfo().m_bSelectingLock);
			if( ptOld <= ptOld ){
				m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRPREV1));
			}
			return;
		}
		if( !GetDllShareData().m_Common.m_sSearch.m_bSearchAll ){
			break;
		}
		ptXY.x = INT_MAX;
		ptXY.y = GetDocument()->m_cDocLineMgr.GetLineCount() - 1;
	}
	m_pCommanderView->SendStatusMessage(LS(STR_ERR_SRPREV2));
	AlertNotFound(m_pCommanderView->GetHwnd(), false, LS(STR_MARKER_PREV_NOT_FOUND));
	return;
}

/*! カラーマーカー指定削除
*/
BOOL CViewCommander::Command_DELCOLORMARKER_STR(LPCWSTR pszMarker)
{
	CMarkerItem item;
	if( !CColorMarkerVisitor().StrToMarkerItem2(1, pszMarker, item) ){
		ErrorBeep();
		return FALSE;
	}
	CDocLine* docLine = GetDocument()->m_cDocLineMgr.GetDocLineTop();
	while( docLine ){
		const int nCount = CColorMarkerVisitor().GetColorMarkerCount(docLine);
		int nDelIndexLast = -1;
		for( int i = nCount - 1; 0 <= i; i-- ){
			CMarkerItem& data = *CColorMarkerVisitor().GetColorMarker(docLine, i);
			
			if( item.m_nBold == data.m_nBold
				&& item.m_nUnderLine == data.m_nUnderLine
				&& item.m_cTEXT == data.m_cTEXT
				&& item.m_cBACK == data.m_cBACK
				&& item.m_nGyouLine == data.m_nGyouLine
				&& item.m_nExtValue == data.m_nExtValue
			){
				if( nDelIndexLast == -1 ){
					nDelIndexLast = i + 1;
				}
			}else if( nDelIndexLast != -1 ){
				CColorMarkerVisitor().DelColorMarker(docLine, i + 1, nDelIndexLast);
				nDelIndexLast = -1;
			}
		}
		if( nDelIndexLast != -1 ){
			CColorMarkerVisitor().DelColorMarker(docLine, 0, nDelIndexLast);
		}
		docLine = docLine->GetNextLine();
	}
	GetDocument()->m_pcEditWnd->Views_Redraw();
	return TRUE;
}

/*! カラーマーカー全削除
*/
void CViewCommander::Command_DELCOLORMARKER_ALL(void)
{
	CColorMarkerVisitor().ResetAllColorMarker(&GetDocument()->m_cDocLineMgr);
	GetDocument()->m_pcEditWnd->Views_Redraw();
}

/*! カラーマーカー削除識別値
*/
void CViewCommander::Command_DELCOLORMARKER_EXTVALUE(UINT nExtValue, UINT nExtValue2)
{
	CDocLine* docLine = GetDocument()->m_cDocLineMgr.GetDocLineTop();
	while( docLine ){
		int nDelIndexLast = -1;
		const int nCount = CColorMarkerVisitor().GetColorMarkerCount(docLine);
		for( int i = nCount - 1; 0 <= i; i-- ){
			CMarkerItem& data = *CColorMarkerVisitor().GetColorMarker(docLine, i);
			if( nExtValue <= data.m_nExtValue && data.m_nExtValue <= nExtValue2 ){
				if( nDelIndexLast == -1 ){
					nDelIndexLast = i + 1;
				}
			}else if( nDelIndexLast != -1 ){
				CColorMarkerVisitor().DelColorMarker(docLine, i + 1, nDelIndexLast);
				nDelIndexLast = -1;
			}
		}
		if( nDelIndexLast != -1 ){
			CColorMarkerVisitor().DelColorMarker(docLine, 0, nDelIndexLast);
		}
		docLine = docLine->GetNextLine();
	}
	GetDocument()->m_pcEditWnd->Views_Redraw();
}

static bool GetSCMFileName(CEditDoc* pcEditDoc, LPCWSTR pszFileName, bool bSave, std::tstring& tstr)
{
	if( pszFileName == NULL || pszFileName[0] == L'\0' ){
		TCHAR szPath[_MAX_PATH];
		CDlgOpenFile cDlgOpenFile;
		cDlgOpenFile.Create(
			G_AppInstance(),
			pcEditDoc->m_pcEditWnd->GetHwnd(),
			_T("*.scm"),
			CSakuraEnvironment::GetDlgInitialDir().c_str()
		);
		auto_strcpy(szPath, pcEditDoc->m_cDocFile.GetFilePath());
		if( szPath[0] ){
			if( lstrlen(szPath) + 4 < _countof(szPath) ){
				auto_strcat(szPath, _T(".scm"));
			}
		}
		if( bSave ){
			if( false == cDlgOpenFile.DoModal_GetSaveFileName(szPath) ){
				return false; // Cancel
			}
		}else{
			if( false == cDlgOpenFile.DoModal_GetOpenFileName(szPath) ){
				return false; // Cancel
			}
		}
		tstr = szPath;
	}else{
		tstr = to_tchar(pszFileName);
	}
	return true;
}

/*! カラーマーカー読み込み
*/
void CViewCommander::Command_COLORMARKER_LOAD(LPCWSTR pszFileName)
{
	std::tstring tstr;
	if( GetSCMFileName(GetDocument(), pszFileName, false, tstr) ){
		if( CColorMarkerVisitor().LoadColorMarker(&GetDocument()->m_cDocLineMgr, tstr.c_str()) ){
			GetDocument()->m_pcEditWnd->Views_Redraw();
		}else{
			ErrorMessage(m_pCommanderView->GetHwnd(), LS(STR_ERR_LOADCOLORMAKER), tstr.c_str());
		}
	}
}

/*! カラーマーカー書き込み
*/
void CViewCommander::Command_COLORMARKER_SAVE(LPCWSTR pszFileName)
{
	std::tstring tstr;
	if( GetSCMFileName(GetDocument(), pszFileName, true, tstr) ){
		if( CColorMarkerVisitor().SaveColorMarker(&GetDocument()->m_cDocLineMgr, tstr.c_str()) ){
		}else{
			ErrorMessage(m_pCommanderView->GetHwnd(), LS(STR_ERR_SAVECOLORMAKER), tstr.c_str());
		}
	}
}

/*! カラーマーカープリセット・履歴設定
*/
BOOL CViewCommander::Command_COLORMARKER_SETPRESET(int nIndex, LPCWSTR pszMarker, LPCWSTR pszName)
{
	Setting_ColorMarker& markerSet = GetDllShareData().m_Common.m_sSearch.m_sColorMarker;
	CMarkerItem item;
	CColorMarkerVisitor().StrToMarkerItem2(1, pszMarker, item);
	if( nIndex == 0 ){
		markerSet.m_ColorItemLast = item;
	}else if( 0 < nIndex && nIndex <= _countof(markerSet.m_ColorItems) ){
		markerSet.m_ColorItems[nIndex - 1] = item;
		wcscpyn(markerSet.m_szSetNames[nIndex - 1], pszName, _countof(markerSet.m_szSetNames[0]));
	}else{
		ErrorBeep();
		return FALSE;
	}
	return TRUE;
}
