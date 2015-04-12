/*
	Copyright (C) 2014, Plugins developers
	Copyright (C) 2015, Moca

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
#include "prop/CDlgCOnfigChildGesture.h"
#include "prop/CDlgCOnfig.h"
#include "env/CShareData.h"
#include "env/CDocTypeManager.h"
#include "typeprop/CImpExpManager.h"
#include "dlg/CDlgMouseGesture.h"
#include "util/shell.h"
#include <memory>
#include "sakura_rc.h"
#include "sakura.hh"

///////////////////////////////////////////////////////////////////////////////
//Popup Help
static const DWORD p_helpids[] = {
	IDC_BUTTON_ADD,				HIDC_BUTTON_ADD_GESTURE,	//ジェスチャー追加
	IDC_BUTTON_EDIT,			HIDC_BUTTON_EDIT_GESTURE,	//ジェスチャー編集
	IDC_BUTTON_DELETE,			HIDC_BUTTON_DEL_GESTURE,	//ジェスチャー削除
	IDC_BUTTON_IMPORT,			HIDC_BUTTON_IMPORT_GESTURE,	//ジェスチャーインポート
	IDC_BUTTON_EXPORT,			HIDC_BUTTON_EXPORT_GESTURE,	//ジェスチャーエクスポート
	IDC_COMBO_TYPE,				HIDC_COMBO_GESTURE,			//ジェスチャータイプ
	IDC_STATIC_TYPE,			HIDC_COMBO_GESTURE,			//ジェスチャータイプ
	IDC_CHECKBOX_USE,			HIDC_CHECKBOX_GESTURE,		//ジェスチャーを使用する
	IDC_LIST_GESTURE,			HIDC_LIST_GESTURE,			//ジェスチャー一覧
//	IDC_STATIC,					-1,
	0, 0
};

///////////////////////////////////////////////////////////////////////////////
HWND CDlgConfigChildGesture::DoModeless(HINSTANCE hInstance, HWND hwndParent, SDlgConfigArg* pDlgConfigArg, int nTypeIndex)
{
	m_nCurrentTypeIndex = nTypeIndex;
	m_pDlgConfigArg  = pDlgConfigArg;

	return CDialog::DoModeless(hInstance, hwndParent, IDD_PROP_GESTURE, 0, SW_SHOW);
}

///////////////////////////////////////////////////////////////////////////////
BOOL CDlgConfigChildGesture::OnInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HWND hwndDlg = hWnd;
	HWND hCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_TYPE);
	HWND hList = ::GetDlgItem(hwndDlg, IDC_LIST_GESTURE);
	HWND hwndOK = ::GetDlgItem(hwndDlg, IDOK);
	HWND hwndCancel = ::GetDlgItem(hwndDlg, IDCANCEL);
	RECT		rc;
	LV_COLUMN	lvc;

	::GetWindowRect(hList, &rc);
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = (rc.right - rc.left) * 6 / 10;
	lvc.pszText = const_cast<TCHAR*>(LS(STR_GESTURE_GESTURE));
	lvc.iSubItem = 0;
	ListView_InsertColumn(hList, 0, &lvc);
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = (rc.right - rc.left) * 3 / 10;
	lvc.pszText = const_cast<TCHAR*>(LS(STR_GESTURE_FUNCTION));
	lvc.iSubItem = 1;
	ListView_InsertColumn(hList, 1, &lvc);

	LONG_PTR lStyle = ListView_GetExtendedListViewStyle(hList);
	ListView_SetExtendedListViewStyle(hList, lStyle | LVS_EX_FULLROWSELECT);

	// ダイアログデータの設定
	BOOL result = CDlgConfigChild::OnInitDialog( hwndDlg, wParam, lParam );
	if(wParam == IDOK){ // 独立ウィンドウ
		GetWindowRect(hwndOK, &rc);
		int nBottom = rc.bottom; // OK,CANCELボタンの下端

		GetWindowRect(hwndDlg, &rc);
		SetWindowPos(hwndDlg, NULL, 0, 0, (rc.right - rc.left), (nBottom - rc.top + 10), SWP_NOZORDER | SWP_NOMOVE);
		std::tstring title = LS(STR_PROPCOMMON);
		title += _T(" - ");
		title += LS(STR_PROPCOMMON_GESTURE);
		SetWindowText(hwndDlg, title.c_str());
	}else{
		ShowWindow(hwndOK, SW_HIDE);
		ShowWindow(hwndCancel, SW_HIDE);
	}

	// コントロール更新のタイミング用のタイマーを起動
	::SetTimer(hwndDlg, 1, 300, NULL);

	return result;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CDlgConfigChildGesture::OnNotify(WPARAM wParam, LPARAM lParam)
{
	HWND hwndDlg = GetHwnd();
	NMHDR* pNMHDR = (NMHDR*)lParam;
	HWND hList = ::GetDlgItem(hwndDlg, IDC_LIST_GESTURE);

	switch(pNMHDR->code){
	case NM_DBLCLK:
		switch(pNMHDR->idFrom){
		case IDC_LIST_GESTURE:
			EditGesture(hwndDlg, hList);
			return TRUE;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return CDialog::OnNotify(wParam, lParam);
}

///////////////////////////////////////////////////////////////////////////////
BOOL CDlgConfigChildGesture::OnBnClicked(int wID)
{
	HWND hwndDlg = GetHwnd();
	HWND hList   = ::GetDlgItem(hwndDlg, IDC_LIST_GESTURE);
	switch(wID){
	case IDC_BUTTON_ADD:	// ジェスチャー追加
		AddGesture(hwndDlg, hList);
		return TRUE;
	case IDC_BUTTON_EDIT:	// ジェスチャー編集
		EditGesture(hwndDlg, hList);
		return TRUE;
	case IDC_BUTTON_DELETE:	// ジェスチャー削除
		DeleteGesture(hwndDlg, hList);
		return TRUE;
	case IDC_BUTTON_IMPORT:	// インポート
		ImportGesture(hwndDlg, hList);
		return TRUE;
	case IDC_BUTTON_EXPORT:	// エクスポート
		ExportGesture(hwndDlg, hList);
		return TRUE;
		
	// 独立ウィンドウで使用する
	case IDOK:
		EndDialog(hwndDlg, IDOK);
		return TRUE;
	case IDCANCEL:
		EndDialog(hwndDlg, IDCANCEL);
		return TRUE;
	default:
		break;
	}
	return CDialog::OnBnClicked(wID);
}

///////////////////////////////////////////////////////////////////////////////
BOOL CDlgConfigChildGesture::OnTimer(WPARAM wParam)
{
	HWND hwndDlg = GetHwnd();
	HWND hCombo  = ::GetDlgItem(hwndDlg, IDC_COMBO_TYPE);
	HWND hList   = ::GetDlgItem(hwndDlg, IDC_LIST_GESTURE);
	HWND hAdd    = ::GetDlgItem(hwndDlg, IDC_BUTTON_ADD);
	HWND hEdit   = ::GetDlgItem(hwndDlg, IDC_BUTTON_EDIT);
	HWND hDelete = ::GetDlgItem(hwndDlg, IDC_BUTTON_DELETE);
	HWND hLabel  = ::GetDlgItem(hwndDlg, IDC_STATIC_TYPE);
	HWND hImport = ::GetDlgItem(hwndDlg, IDC_BUTTON_IMPORT);
	HWND hExport = ::GetDlgItem(hwndDlg, IDC_BUTTON_EXPORT);

	int nIndex = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
	int nCount = ListView_GetItemCount(hList);
	BOOL bEnable = ::IsDlgButtonChecked(hwndDlg, IDC_CHECKBOX_USE);
	BOOL bButton = (bEnable != FALSE) && (nIndex >= 0);
	BOOL bButtonAdd = bEnable && (nCount < MAX_MOUSE_GESTURE_COUNT);

	::EnableWindow(hCombo, bEnable);
	::EnableWindow(hLabel, bEnable);
	::EnableWindow(hList, bEnable);
	::EnableWindow(hAdd, bButtonAdd);
	::EnableWindow(hEdit, bButton);
	::EnableWindow(hDelete, bButton);
	::EnableWindow(hImport, TRUE);
	::EnableWindow(hExport, TRUE);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
BOOL CDlgConfigChildGesture::OnDestroy()
{
	::KillTimer(GetHwnd(), 1);
	return CDlgConfigChild::OnDestroy();
}

///////////////////////////////////////////////////////////////////////////////
// ジェスチャーを追加する
void CDlgConfigChildGesture::AddGesture(HWND hwndDlg, HWND hList)
{
	int nIndex = ListView_GetItemCount(hList);
	if((nIndex < 0) || (nIndex >= MAX_MOUSE_GESTURE_COUNT)) return;

	CGestureInfo info = { _T("0"), F_MENU_RBUTTON };
	int nRet = InputGesture(hwndDlg, info);
	if(nRet != IDOK) return;

	LVITEM lvi;
	memset_raw(&lvi, 0, sizeof(lvi));
	lvi.iItem    = nIndex;
	lvi.iSubItem = 0;
	lvi.mask     = LVIF_TEXT | LVIF_PARAM;
	lvi.pszText  = info.m_sGesture;
	lvi.lParam   = info.m_nFuncID;
	nIndex = ListView_InsertItem(hList, &lvi);

	TCHAR szBuffer[512];
	_tcscpy(szBuffer, _T(""));
	if(info.m_nFuncID == F_DISABLE){
		_tcscpy(szBuffer, LS(STR_PROPCOMKEYBIND_UNASSIGN));
	}else{
		WCHAR szBufferW[256];
		m_pDlgConfigArg->m_cLookup.Funccode2Name(info.m_nFuncID, szBufferW, _countof(szBufferW));
		_tcscpy(szBuffer, to_tchar(szBufferW));
	}
	ListView_SetItemText(hList, nIndex, 1, szBuffer);

	_tcscpy(m_Common.m_sMouseGesture.m_cGesture[m_Common.m_sMouseGesture.m_nGestureNum].m_sGesture, info.m_sGesture);
	m_Common.m_sMouseGesture.m_cGesture[m_Common.m_sMouseGesture.m_nGestureNum].m_nFuncID = info.m_nFuncID;
	m_Common.m_sMouseGesture.m_nGestureNum++;
}

///////////////////////////////////////////////////////////////////////////////
// ジェスチャーを編集する
void CDlgConfigChildGesture::EditGesture(HWND hwndDlg, HWND hList)
{
	int nIndex = ListView_GetNextItem(hList, -1, LVNI_SELECTED | LVNI_FOCUSED);
	if(nIndex < 0) return;

	CGestureInfo info;
	_tcscpy(info.m_sGesture, m_Common.m_sMouseGesture.m_cGesture[nIndex].m_sGesture);
	info.m_nFuncID = m_Common.m_sMouseGesture.m_cGesture[nIndex].m_nFuncID;
	int nRet = InputGesture(hwndDlg, info);
	if(nRet != IDOK) return;

	_tcscpy(m_Common.m_sMouseGesture.m_cGesture[nIndex].m_sGesture, info.m_sGesture);
	m_Common.m_sMouseGesture.m_cGesture[nIndex].m_nFuncID = info.m_nFuncID;

	TCHAR szBuffer[512];
	_tcscpy(szBuffer, _T(""));
	if(info.m_nFuncID == F_DISABLE){
		auto_strcpy(szBuffer, LS(STR_PROPCOMKEYBIND_UNASSIGN));
	}else{
		WCHAR szBufferW[256];
		m_pDlgConfigArg->m_cLookup.Funccode2Name(info.m_nFuncID, szBufferW, _countof(szBufferW));
		_tcscpy(szBuffer, to_tchar(szBufferW));
	}

	LVITEM lvi;
	memset_raw(&lvi, 0, sizeof(lvi));
	lvi.iItem    = nIndex;
	lvi.iSubItem = 0;
	lvi.mask     = LVIF_TEXT | LVIF_PARAM;
	lvi.pszText  = info.m_sGesture;
	lvi.lParam   = info.m_nFuncID;
	ListView_SetItem(hList, &lvi);

	ListView_SetItemText(hList, nIndex, 1, szBuffer);
}

///////////////////////////////////////////////////////////////////////////////
// ジェスチャーを削除する
void CDlgConfigChildGesture::DeleteGesture(HWND hwndDlg, HWND hList)
{
	int nIndex = ListView_GetNextItem(hList, -1, LVNI_SELECTED | LVNI_FOCUSED);
	if(nIndex < 0) return;

	ListView_DeleteItem(hList, nIndex);

	for(int i = nIndex; i < m_Common.m_sMouseGesture.m_nGestureNum - 1; i++){
		_tcscpy(m_Common.m_sMouseGesture.m_cGesture[i].m_sGesture, m_Common.m_sMouseGesture.m_cGesture[i+1].m_sGesture);
		m_Common.m_sMouseGesture.m_cGesture[i].m_nFuncID = m_Common.m_sMouseGesture.m_cGesture[i+1].m_nFuncID;
	}
	m_Common.m_sMouseGesture.m_nGestureNum--;
}

///////////////////////////////////////////////////////////////////////////////
// インポートする
void CDlgConfigChildGesture::ImportGesture(HWND hwndDlg, HWND hList)
{
	CImpExpMouseGesture cImpExp(m_Common);
	if(cImpExp.ImportUI(G_AppInstance(), hwndDlg)){
		//インポートした
		SetData();
	}
}

///////////////////////////////////////////////////////////////////////////////
// エクスポートする
void CDlgConfigChildGesture::ExportGesture(HWND hwndDlg, HWND hList)
{
	CImpExpMouseGesture cImpExp(m_Common);
	if(cImpExp.ExportUI(G_AppInstance(), hwndDlg)){
		//エクスポートした
	}
}

///////////////////////////////////////////////////////////////////////////////
// ダイアログデータの設定
void CDlgConfigChildGesture::SetData()
{
	HWND hwndDlg = GetHwnd();
	//マウスジェスチャーを使用する
	::CheckDlgButton(hwndDlg, IDC_CHECKBOX_USE, m_Common.m_sMouseGesture.m_nUse);

	//ジェスチャーの種類
	HWND hCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_TYPE);
	Combo_ResetContent(hCombo);
	Combo_InsertString(hCombo, 0, LS(STR_GESTURE_4DIRECTION));
	Combo_InsertString(hCombo, 1, LS(STR_GESTURE_8DIRECTION));
	Combo_SetCurSel(hCombo, m_Common.m_sMouseGesture.m_nType);

	//ジェスチャーリスト
	HWND hList = ::GetDlgItem(hwndDlg, IDC_LIST_GESTURE);
	ListView_DeleteAllItems(hList);
	LVITEM lvi;
	TCHAR szBuffer[512];
	for(int i = 0; i < m_Common.m_sMouseGesture.m_nGestureNum; i++){
		memset_raw(&lvi, 0, sizeof(lvi));
		lvi.iItem    = i;
		lvi.iSubItem = 0;
		lvi.mask     = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText  = m_Common.m_sMouseGesture.m_cGesture[i].m_sGesture;
		lvi.lParam   = m_Common.m_sMouseGesture.m_cGesture[i].m_nFuncID;
		int nIndex = ListView_InsertItem(hList, &lvi);

		_tcscpy(szBuffer, _T(""));
		if(m_Common.m_sMouseGesture.m_cGesture[i].m_nFuncID == F_DISABLE){
			_tcscpy(szBuffer, LS(STR_PROPCOMKEYBIND_UNASSIGN));
		}else{
			WCHAR szBufferW[256];
			m_pDlgConfigArg->m_cLookup.Funccode2Name(m_Common.m_sMouseGesture.m_cGesture[i].m_nFuncID, szBufferW, _countof(szBufferW));
			_tcscpy(szBuffer, to_tchar(szBufferW));
		}
		ListView_SetItemText(hList, nIndex, 1, szBuffer);
	}
}

///////////////////////////////////////////////////////////////////////////////
// ダイアログデータの取得
int CDlgConfigChildGesture::GetData()
{
	HWND hwndDlg = GetHwnd();
	//マウスジェスチャーを使用する
	m_Common.m_sMouseGesture.m_nUse = ::IsDlgButtonChecked(hwndDlg, IDC_CHECKBOX_USE);
	//ジェスチャーの種類
	m_Common.m_sMouseGesture.m_nType = GetType(hwndDlg);

	//ジェスチャーリスト
	HWND hList = ::GetDlgItem(hwndDlg, IDC_LIST_GESTURE);
	m_Common.m_sMouseGesture.m_nGestureNum = ListView_GetItemCount(hList);
	for(int i = 0; i < m_Common.m_sMouseGesture.m_nGestureNum; i++){
		LVITEM lvi;
		memset_raw(&lvi, 0, sizeof(lvi));
		lvi.iItem    = i;
		lvi.iSubItem = 0;
		lvi.mask     = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText  = m_Common.m_sMouseGesture.m_cGesture[i].m_sGesture;
		lvi.lParam   = 0;
		lvi.cchTextMax = _countof(m_Common.m_sMouseGesture.m_cGesture[i].m_sGesture);
		int nIndex = ListView_GetItem(hList, &lvi);
		m_Common.m_sMouseGesture.m_cGesture[i].m_nFuncID = lvi.lParam;
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
int CDlgConfigChildGesture::GetType(HWND hwndDlg)
{
	//ジェスチャーの種類
	HWND hCombo = ::GetDlgItem(hwndDlg, IDC_COMBO_TYPE);
	return Combo_GetCurSel(hCombo);
}

///////////////////////////////////////////////////////////////////////////////
int CDlgConfigChildGesture::InputGesture(HWND hwndDlg, CGestureInfo& info)
{
	CDlgMouseGesture dlg;
	return dlg.DoModal(G_AppInstance(), hwndDlg, GetType(hwndDlg), &info, &m_pDlgConfigArg->m_cLookup);
}

///////////////////////////////////////////////////////////////////////////////
LPVOID CDlgConfigChildGesture::GetHelpIdTable()
{
	return (LPVOID)p_helpids;
}
