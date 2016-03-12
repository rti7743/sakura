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
#include "dlg/CDlgColorMarker.h"
#include "doc/CEditDoc.h"
#include "window/CEditWnd.h"
#include "typeprop/CPropTypes.h"
#include "func/Funccode.h"
#include "util/shell.h"
#include "util/string_ex.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

const DWORD p_helpids[] = {
	IDOK,							HIDOK_COLORMARKER,				//マーカー設定
	IDCANCEL,						HIDCANCEL_COLORMARKER,			//キャンセル
	IDC_BUTTON_HELP,				HIDC_COLORMARKER_BUTTON_HELP,	//ヘルプ
	IDC_COMBO_COLORSET,				HIDC_COMBO_COLORSET,			//プリセット
	IDC_BUTTON_SETCOLOR,			HIDC_BUTTON_SETCOLOR,			//セットに反映
	IDC_EDIT_LABEL,					HIDC_EDIT_COLORMARKER_LABEL,	//名前
	IDC_CHECK_COLOR_TEXT,			HIDC_CHECK_COLOR_TEXT,			//テキストの有効化
	IDC_CHECK_COLOR_BACK,			HIDC_CHECK_COLOR_BACK,			//背景の有効化
	IDC_CHECK_ENABLE_BOLD,			HIDC_CHECK_ENABLE_BOLD,			//太字の有効化
	IDC_CHECK_ENABLE_UNDERLINE,		HIDC_CHECK_ENABLE_UNDERLINE,	//下線の有効化
	IDC_BUTTON_COLOR_TEXT,			HIDC_BUTTON_COLOR_TEXT,			//テキスト
	IDC_BUTTON_COLOR_BACK,			HIDC_BUTTON_COLOR_BACK,			//背景
	IDC_CHECK_BOLD,					HIDC_CHECK_MARKER_BOLD,			//太字
	IDC_CHECK_UNDERLINE,			HIDC_CHECK_MARKER_UNDERLINE,	//下線
	IDC_CHECK_GYOU,					HIDC_CHECK_GYOU,				//行番号
	IDC_CHECK_LINE,					HIDC_CHECK_LINE,				//行全体
	IDC_EDIT_EXTVALUE,				HIDC_EDIT_EXTVALUE,				//識別値
	IDC_EDIT_EXTVALUE2,				HIDC_EDIT_EXTVALUE2,			//識別値削除範囲
	IDC_BUTTON_DELCOLOR,			HIDC_BUTTON_DELCOLOR,			//指定色の全削除
	IDC_BUTTON_DELEXTVALUE,			HIDC_BUTTON_DELEXTVALUE,		//識別値削除
	IDC_BUTTON_DELCOLORALL,			HIDC_BUTTON_DELCOLORALL,		//全削除
	IDC_BUTTON_LOADMARKER,			HIDC_BUTTON_LOADMARKER,			//読み込み
	IDC_BUTTON_SAVEMARKER,			HIDC_BUTTON_SAVEMARKER,			//保存
	0, 0
};

static UINT GetExtValue(HWND hwndDlg, int nID)
{
	TCHAR szExtValue[100];
	szExtValue[0] = L'\0';
	DlgItem_GetText(hwndDlg, nID, szExtValue, _countof(szExtValue));
	return (UINT)_tcstoul(szExtValue, NULL, 0); // 基数に0なので0xが付いてると自動で16進数になる
}

CDlgColorMarker::CDlgColorMarker()
{
	m_pcEditDoc = NULL;
	m_bGetItemMode = NULL;
	return;
}

int CDlgColorMarker::DoModal(HINSTANCE hInstance, HWND hwndParent, LPARAM lParam, CEditDoc* pcDoc, bool bGetItemMode)
{
	m_pcEditDoc = pcDoc;
	m_bGetItemMode = bGetItemMode;
	return CDialog::DoModal(hInstance, hwndParent, IDD_COLORMARKER, lParam);
}

CViewCommander& Marker_MyGetCommander(CEditDoc* pcEditDoc)
{
	return pcEditDoc->m_pcEditWnd->GetActiveView().GetCommander();
}

BOOL CDlgColorMarker::OnBnClicked(int wID)
{
	switch( wID ){
	case IDC_BUTTON_HELP:
		MyWinHelp(GetHwnd(), HELP_CONTEXT, ::FuncID_To_HelpContextID(F_DLGCOLORMARKER));
		return TRUE;
	case IDOK:
		GetData();
		m_pShareData->m_Common.m_sSearch.m_sColorMarker.m_ColorItemLast = m_cItem;
		if( !m_bGetItemMode ){
			wchar_t szMarker[MAX_MARKER_INFO];
			CColorMarkerVisitor().MarkerItemToStr2(m_cItem, szMarker);
			Marker_MyGetCommander(m_pcEditDoc).HandleCommand(F_SETCOLORMARKER_STR, true, (LPARAM)szMarker, 0, 0, 0);
		}
		::EndDialog(GetHwnd(), TRUE);
		return TRUE;
	case IDCANCEL:
		::EndDialog(GetHwnd(), FALSE);
		return TRUE;
	case IDC_BUTTON_SETCOLOR:
		{
			int nCurSel = Combo_GetCurSel(GetItemHwnd(IDC_COMBO_COLORSET));
			GetData();
			wchar_t szMarker[MAX_MARKER_INFO];
			CColorMarkerVisitor().MarkerItemToStr2(m_cItem, szMarker);
			Marker_MyGetCommander(m_pcEditDoc).HandleCommand(F_COLORMARKER_SETPRESET, true, nCurSel, (LPARAM)szMarker, (LPARAM)m_szPresetName, 0);
			if( nCurSel != 0 ){
				SetDataPreset();
			}
		}
		return TRUE;
	case IDC_CHECK_COLOR_TEXT:
	case IDC_CHECK_COLOR_BACK:
	case IDC_CHECK_ENABLE_BOLD:
	case IDC_CHECK_ENABLE_UNDERLINE:
		return TRUE;

	case IDC_BUTTON_COLOR_TEXT:
		{
			if( CPropTypesColor::SelectColor(GetHwnd(), &m_cItem.m_cTEXT,
					m_pShareData->m_Common.m_sSearch.m_sColorMarker.m_dwChooseColor) ){
				::InvalidateRect(GetItemHwnd(IDC_BUTTON_COLOR_TEXT), NULL, TRUE);
			}
		}
		return TRUE;
	case IDC_BUTTON_COLOR_BACK:
		{
			if( CPropTypesColor::SelectColor(GetHwnd(), &m_cItem.m_cBACK,
					m_pShareData->m_Common.m_sSearch.m_sColorMarker.m_dwChooseColor) ){
				::InvalidateRect(GetItemHwnd(IDC_BUTTON_COLOR_BACK), NULL, TRUE);
			}
		}
		return TRUE;
	case IDC_BUTTON_DELCOLOR:
		{
			GetData();
			const CMarkerItem& item = m_cItem;
			wchar_t szMarker[MAX_MARKER_INFO];
			CColorMarkerVisitor().MarkerItemToStr2(item, szMarker);
			Marker_MyGetCommander(m_pcEditDoc).HandleCommand(F_DELCOLORMARKER_STR, true, (LPARAM)szMarker, 0, 0, 0);
		}
		return TRUE;
	case IDC_BUTTON_DELEXTVALUE:
		{
			UINT extValue = GetExtValue(GetHwnd(), IDC_EDIT_EXTVALUE);
			UINT extValue2 = GetExtValue(GetHwnd(), IDC_EDIT_EXTVALUE2);
			Marker_MyGetCommander(m_pcEditDoc).HandleCommand(F_DELCOLORMARKER_EXTVALUE, true, (LPARAM)extValue, (LPARAM)extValue2, 0, 0);
		}
		return TRUE;
	case IDC_BUTTON_DELCOLORALL:
		{
			Marker_MyGetCommander(m_pcEditDoc).HandleCommand(F_DELCOLORMARKER_ALL, true, 0, 0, 0, 0);
		}
		return TRUE;
	case IDC_BUTTON_LOADMARKER:
		{
			Marker_MyGetCommander(m_pcEditDoc).HandleCommand(F_COLORMARKER_LOAD, true, (LPARAM)NULL, 0, 0, 0);
		}
		return TRUE;
	case IDC_BUTTON_SAVEMARKER:
		{
			Marker_MyGetCommander(m_pcEditDoc).HandleCommand(F_COLORMARKER_SAVE, true, (LPARAM)NULL, 0, 0, 0);
		}
		return TRUE;
	}
	return CDialog::OnBnClicked(wID);
}

BOOL CDlgColorMarker::OnCbnSelChange(HWND hwndCtl, int wID)
{
	if( wID == IDC_COMBO_COLORSET ){
		Setting_ColorMarker sMarker = m_pShareData->m_Common.m_sSearch.m_sColorMarker;
		int nCurSel = Combo_GetCurSel(GetItemHwnd(IDC_COMBO_COLORSET));
		if( nCurSel == 0 ){
			m_cItem = sMarker.m_ColorItemLast;
			auto_strcpy(m_szPresetName, L"");
			SetDataItem();
			::EnableWindow(GetItemHwnd(IDC_EDIT_LABEL), FALSE);
		}else if( 0 < nCurSel && nCurSel <= _countof(sMarker.m_ColorItems) ){
			m_cItem = sMarker.m_ColorItems[nCurSel - 1];
			auto_strcpy(m_szPresetName, sMarker.m_szSetNames[nCurSel - 1]);
			SetDataItem();
			::EnableWindow(GetItemHwnd(IDC_EDIT_LABEL), TRUE);
		}
	}
	return FALSE;
}

INT_PTR CDlgColorMarker::DispatchEvent(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	switch( wMsg ){
	case WM_DRAWITEM:
		{
			UINT idCtrl = (UINT)wParam;
			LPDRAWITEMSTRUCT pDis = (LPDRAWITEMSTRUCT)lParam;
			switch( idCtrl ){
			case IDC_BUTTON_COLOR_TEXT:
				CPropTypesColor::DrawColorButton(pDis, m_cItem.m_cTEXT);
				return TRUE;
			case IDC_BUTTON_COLOR_BACK:
				CPropTypesColor::DrawColorButton(pDis, m_cItem.m_cBACK);
				return TRUE;
			}
		}
		break;
	default:
		break;
	}
	return CDialog::DispatchEvent(hWnd, wMsg, wParam, lParam);
}

void CDlgColorMarker::SetData(void)
{
	EditCtl_LimitText(GetItemHwnd(IDC_EDIT_LABEL), _countof(m_szPresetName) - 1);
	EditCtl_LimitText(GetItemHwnd(IDC_EDIT_EXTVALUE), 10); // 0x34567890
	EditCtl_LimitText(GetItemHwnd(IDC_EDIT_EXTVALUE2), 10);

	HWND hStyle = GetItemHwnd(IDC_COMBO_STYLE1);
	for( int id = STR_MARKER_STYLE_NONE; id <= STR_MARKER_STYLE_UNDER_NOBOLD; id++ ){
		Combo_AddString(hStyle, LS(id));
	}
	

	SetDataPreset();
	Combo_SetCurSel(GetItemHwnd(IDC_COMBO_COLORSET), 0);
	m_cItem = m_pShareData->m_Common.m_sSearch.m_sColorMarker.m_ColorItemLast;
	m_szPresetName[0] = L'\0';
	SetDataItem();
	::EnableWindow(GetItemHwnd(IDC_EDIT_LABEL), FALSE); // 初期表示は履歴
	return;
}

void CDlgColorMarker::SetDataPreset(void)
{
	int nCurSel = Combo_GetCurSel(GetItemHwnd(IDC_COMBO_COLORSET));
	HWND combo = GetItemHwnd(IDC_COMBO_COLORSET);
	Combo_ResetContent(combo);
	Combo_AddString(combo, LS(STR_DLG_MARKER_CURRENT));
	Setting_ColorMarker markerSet = m_pShareData->m_Common.m_sSearch.m_sColorMarker;
	for( int i = 0; i < _countof(markerSet.m_ColorItems); i++ ){
		TCHAR szBuf[200];
		const wchar_t* pName = markerSet.m_szSetNames[i];
		if( L'\0' == pName[0] ){
			pName = LSW(STR_MARKER_PRESET_NAME1 + i);
		}
		auto_sprintf(szBuf, LS(STR_DLG_MARKER_COLORSET), i + 1, pName);
		Combo_AddString(combo, szBuf);
	}
	if( 0 <= nCurSel ){
		Combo_SetCurSel(combo, nCurSel);
	}
}

static char nStyleTable[][4] = {
	{0, 0},{1, 0},{0, 1},
	{1, 1},{2, 0},{0, 2},
	{2, 2},{1, 2},{2, 1},
};
#undef BT
#undef BF

void CDlgColorMarker::SetDataItem(void)
{
	DlgItem_SetText(GetHwnd(), IDC_EDIT_LABEL, m_szPresetName);
	CheckDlgButtonBool(GetHwnd(), IDC_CHECK_COLOR_TEXT, m_cItem.m_cTEXT != -1);
	CheckDlgButtonBool(GetHwnd(), IDC_CHECK_COLOR_BACK, m_cItem.m_cBACK != -1);

	int nIndex = 0;
	for( int i = 0; i < (int)_countof(nStyleTable); i++){
		if( nStyleTable[i][0] == m_cItem.m_nBold && nStyleTable[i][1] == m_cItem.m_nUnderLine ){
			nIndex = i;
			break;
		}
	}
	Combo_SetCurSel(GetItemHwnd(IDC_COMBO_STYLE1), nIndex);

	bool bGyou = false;
	bool bLineAll = false;
	int nGyouLine = m_cItem.m_nGyouLine;
	if( nGyouLine & 0x01 ){
		bGyou = true;
	}
	if( nGyouLine & 0x02 ){
		bLineAll = true;
	}
	m_cItem.m_nGyouLine = nGyouLine;
	CheckDlgButtonBool(GetHwnd(), IDC_CHECK_GYOU, bGyou);
	CheckDlgButtonBool(GetHwnd(), IDC_CHECK_LINE, bLineAll);

	::InvalidateRect(GetItemHwnd(IDC_BUTTON_COLOR_TEXT), NULL, TRUE);
	::InvalidateRect(GetItemHwnd(IDC_BUTTON_COLOR_BACK), NULL, TRUE);
	TCHAR szExtValue[100];
	auto_sprintf(szExtValue, _T("0x%08x"), m_cItem.m_nExtValue);
	DlgItem_SetText( GetHwnd(), IDC_EDIT_EXTVALUE, szExtValue);
	DlgItem_SetText( GetHwnd(), IDC_EDIT_EXTVALUE2, szExtValue); // 同じ値を設定
	return;
}

int CDlgColorMarker::GetData( void )
{
	DlgItem_GetText(GetHwnd(), IDC_EDIT_LABEL, m_szPresetName, _countof(m_szPresetName));
	if( !IsDlgButtonCheckedBool(GetHwnd(), IDC_CHECK_COLOR_TEXT) ){
		m_cItem.m_cTEXT = -1;
	}
	if( !IsDlgButtonCheckedBool(GetHwnd(), IDC_CHECK_COLOR_BACK) ){
		m_cItem.m_cBACK = -1;
	}
	int nCurSel = Combo_GetCurSel(GetItemHwnd(IDC_COMBO_STYLE1));
	if( nCurSel != -1 ){
		m_cItem.m_nBold = nStyleTable[nCurSel][0];
		m_cItem.m_nUnderLine = nStyleTable[nCurSel][1];
	}
	int nGyouLine = 0;
	if( IsDlgButtonCheckedBool(GetHwnd(), IDC_CHECK_GYOU) ){
		nGyouLine |= 0x01;
	}
	if( IsDlgButtonCheckedBool(GetHwnd(), IDC_CHECK_LINE) ){
		nGyouLine |= 0x02;
	}
	m_cItem.m_nGyouLine = nGyouLine;
	m_cItem.m_nExtValue = GetExtValue(GetHwnd(), IDC_EDIT_EXTVALUE);
	return TRUE;
}

LPVOID CDlgColorMarker::GetHelpIdTable(void)
{
	return (LPVOID)p_helpids;
}

