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
#include "dlg/CDlgMouseGesture.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"

const DWORD p_helpids[] = {
	IDC_BUTTON_LEFT_UP,		HIDC_GESTURE_BUTTON_LEFT_UP,		//左上
	IDC_BUTTON_UP,			HIDC_GESTURE_BUTTON_UP,				//上
	IDC_BUTTON_RIGHT_UP,	HIDC_GESTURE_BUTTON_RIGHT_UP,		//右上
	IDC_BUTTON_LEFT,		HIDC_GESTURE_BUTTON_LEFT,			//左
	IDC_BUTTON_RIGHT,		HIDC_GESTURE_BUTTON_RIGHT,			//右
	IDC_BUTTON_LEFT_DOWN,	HIDC_GESTURE_BUTTON_LEFT_DOWN,		//左下
	IDC_BUTTON_DOWN,		HIDC_GESTURE_BUTTON_DOWN,			//下
	IDC_BUTTON_RIGHT_DOWN,	HIDC_GESTURE_BUTTON_RIGHT_DOWN,		//右下
	IDC_CHECKBOX_SHIFT,		HIDC_GESTURE_CHECKBOX_SHIFT,		//SHIFT
	IDC_CHECKBOX_CTRL,		HIDC_GESTURE_CHECKBOX_CTRL,			//CTRL
	IDC_CHECKBOX_ALT,		HIDC_GESTURE_CHECKBOX_ALT,			//ALT
	IDC_BUTTON_RESET,		HIDC_GESTURE_BUTTON_RESET,			//リセット
	IDC_BUTTON_BROWSE,		HIDC_GESTURE_BUTTON_BROWSE,			//機能割り当て
	IDC_BUTTON_CLEAR,		HIDC_GESTURE_BUTTON_CLEAR,			//解除
	IDOK,					HIDC_GESTURE_IDOK,					//OK
	IDCANCEL,				HIDC_GESTURE_IDCANCEL,				//キャンセル
	0, 0
};


#define GESTURE_4 0
#define GESTURE_8 1

int CDlgMouseGesture::DoModal(HINSTANCE hInstance, HWND hwndParent, int nGestureType, CGestureInfo* info, CFuncLookup* pcLookup)
{
	m_nType = nGestureType;
	m_info = *info;
	m_pcLookup = pcLookup;
	int nRet =  CDialog::DoModal(hInstance, hwndParent, IDD_GESTURE_EDIT, (LPARAM)0);
	if (nRet){
		*info = m_info;
	}
	return nRet;
}


BOOL CDlgMouseGesture::OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	_SetHwnd(hwndDlg);
	::ShowWindow(GetItemHwnd(IDC_CHECKBOX_GESTURE), SW_HIDE);	// よく分からないので非表示
	if (m_nType == GESTURE_4) {
		::ShowWindow(GetItemHwnd(IDC_BUTTON_RIGHT_UP), SW_HIDE);
		::ShowWindow(GetItemHwnd(IDC_BUTTON_RIGHT_DOWN), SW_HIDE);
		::ShowWindow(GetItemHwnd(IDC_BUTTON_LEFT_DOWN), SW_HIDE);
		::ShowWindow(GetItemHwnd(IDC_BUTTON_LEFT_UP), SW_HIDE);
	}
	m_pcLookup->SetCategory2Combo(GetItemHwnd(IDC_COMBO_FUNCKIND));
	m_pcLookup->SetListItem(GetItemHwnd(IDC_LIST_FUNC), 0);

	return CDialog::OnInitDialog(GetHwnd(), wParam, lParam);
}


BOOL CDlgMouseGesture::OnBnClicked(int wID)
{
	switch (wID){
	case IDC_BUTTON_UP:
		AddAction(0);
		break;
	case IDC_BUTTON_RIGHT_UP:
		AddAction(1);
		break;
	case IDC_BUTTON_RIGHT:
		AddAction(2);
		break;
	case IDC_BUTTON_RIGHT_DOWN:
		AddAction(3);
		break;
	case IDC_BUTTON_DOWN:
		AddAction(4);
		break;
	case IDC_BUTTON_LEFT_DOWN:
		AddAction(5);
		break;
	case IDC_BUTTON_LEFT:
		AddAction(6);
		break;
	case IDC_BUTTON_LEFT_UP:
		AddAction(7);
		break;
	case IDC_BUTTON_RESET:
		ResetAction();
		break;
	case IDC_BUTTON_BROWSE:
		BrowseFunc();
		break;
	case IDC_BUTTON_CLEAR:
		ClearFunc();
		break;
	case IDOK:
		GetData();
		CloseDialog(1);
		return TRUE;
	}
	return CDialog::OnBnClicked(wID);
}


BOOL CDlgMouseGesture::OnCbnSelChange(HWND hwndCtl, int wID)
{
	switch (wID){
	case IDC_COMBO_FUNCKIND:
		{
			int nCategory = Combo_GetCurSel(GetItemHwnd(IDC_COMBO_FUNCKIND));
			m_pcLookup->SetListItem(GetItemHwnd(IDC_LIST_FUNC), nCategory);
		}
		return TRUE;
	}
	return CDialog::OnCbnSelChange(hwndCtl, wID);
}


void CDlgMouseGesture::AddAction(int nActionCode)
{
	HWND hwndList = GetItemHwnd(IDC_LIST_GESTURE);
	if ( MAX_GESTURE_LENGTH <= List_GetCount(hwndList)){
		return; // 追加不可
	}
	int nCount = List_GetCount(GetItemHwnd(IDC_LIST_GESTURE));
	if (0 < nCount) {
		TCHAR szItem[64];
		List_GetText(GetItemHwnd(IDC_LIST_GESTURE), nCount - 1, szItem);
		if ((szItem[0] - L'0') == nActionCode) {
			return; // 同じものは連続で追加しない
		}
	}
	static const int nItemsId[] = {
		STR_GESTURE_UP,
		STR_GESTURE_RIGHT_UP,
		STR_GESTURE_RIGHT,
		STR_GESTURE_RIGHT_DOWN,
		STR_GESTURE_DOWN,
		STR_GESTURE_LEFT_DOWN,
		STR_GESTURE_LEFT,
		STR_GESTURE_LEFT_UP
	};
	TCHAR szItem[100];
	auto_sprintf(szItem, _T("%d %ts"), nActionCode, LS(nItemsId[nActionCode]));
	List_InsertString(hwndList, -1, szItem);
}


void CDlgMouseGesture::ResetAction()
{
	List_ResetContent(GetItemHwnd(IDC_LIST_GESTURE));
}


void CDlgMouseGesture::BrowseFunc()
{
	int nCategory = Combo_GetCurSel(GetItemHwnd(IDC_COMBO_FUNCKIND));
	int nIndex = List_GetCurSel(GetItemHwnd(IDC_LIST_FUNC));
	if (nCategory == LB_ERR || nIndex == CB_ERR){
		return;
	}
	m_info.m_nFuncID = m_pcLookup->Pos2FuncCode(nCategory, nIndex);
	WCHAR szFuncName[256];
	m_pcLookup->Funccode2Name(m_info.m_nFuncID, szFuncName, _countof(szFuncName));
	Wnd_SetText(GetItemHwnd(IDC_EDIT_FUNCTION), szFuncName);
}


void CDlgMouseGesture::ClearFunc()
{
	m_info.m_nFuncID = F_DISABLE;
	Wnd_SetText(GetItemHwnd(IDC_EDIT_FUNCTION), LS(STR_PROPCOMKEYBIND_UNASSIGN));
}


void CDlgMouseGesture::SetData()
{
	WCHAR szButton[2] = {m_info.m_sGesture[0], L'\0'};
	int nButton = _wtoi(szButton);
	CheckDlgButtonBool(GetHwnd(), IDC_CHECKBOX_SHIFT, ((nButton & 1) == 1));
	CheckDlgButtonBool(GetHwnd(), IDC_CHECKBOX_CTRL, ((nButton & 2) == 2));
	CheckDlgButtonBool(GetHwnd(), IDC_CHECKBOX_ALT, ((nButton & 4) == 4));
	for (int i = 1; i <= MAX_GESTURE_LENGTH; i++) {
		if (L'0' <= m_info.m_sGesture[i] && m_info.m_sGesture[i] <= L'7'){
			int nCode = m_info.m_sGesture[i] - L'0';
			AddAction(nCode);
		}else{
			break;
		}
	}
	TCHAR szFuncName[512];
	if(m_info.m_nFuncID == F_DISABLE){
		_tcscpy(szFuncName, LS(STR_PROPCOMKEYBIND_UNASSIGN));
	}else{
		WCHAR szFuncNameW[256];
		m_pcLookup->Funccode2Name(m_info.m_nFuncID, szFuncNameW, _countof(szFuncNameW));
		_tcscpy(szFuncName, to_tchar(szFuncNameW));
	}
	Combo_SetCurSel(GetItemHwnd(IDC_COMBO_FUNCKIND), 0);
	Wnd_SetText(GetItemHwnd(IDC_EDIT_FUNCTION), szFuncName);
}


int CDlgMouseGesture::GetData()
{
	WCHAR nButton = 0;
	if (IsDlgButtonCheckedBool(GetHwnd(), IDC_CHECKBOX_SHIFT)) {
		nButton |= 1;
	}
	if (IsDlgButtonCheckedBool(GetHwnd(), IDC_CHECKBOX_CTRL)) {
		nButton |= 2;
	}
	if (IsDlgButtonCheckedBool(GetHwnd(), IDC_CHECKBOX_ALT)) {
		nButton |= 4;
	}
	m_info.m_sGesture[0] = nButton + L'0';
	int nCount = List_GetCount(GetItemHwnd(IDC_LIST_GESTURE));
	for (int i = 0; i < nCount; i++) {
		TCHAR szItem[64];
		List_GetText(GetItemHwnd(IDC_LIST_GESTURE), i, szItem);
		m_info.m_sGesture[i+1] = szItem[0];
		m_info.m_sGesture[i+2] = L'\0';
	}
	return TRUE;
}


LPVOID CDlgMouseGesture::GetHelpIdTable()
{
	return (LPVOID)p_helpids;
}


