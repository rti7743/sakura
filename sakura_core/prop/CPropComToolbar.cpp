/*!	@file
	@brief 共通設定ダイアログボックス、「ツールバー」ページ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000-2001, genta, jepro, MIK
	Copyright (C) 2002, genta, MIK, YAZAKI, aroka
	Copyright (C) 2003, Moca, KEITA
	Copyright (C) 2005, aroka
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2014, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "prop/CDlgConfigChildToolbar.h"
#include "prop/CDlgConfig.h"
#include "env/CShareData.h"
#include "uiparts/CMenuDrawer.h" // 2002/2/10 aroka
#include "uiparts/CImageListMgr.h" // 2005/8/9 aroka
#include "util/shell.h"
#include "util/window.h"
#include "sakura_rc.h"
#include "sakura.hh"


//@@@ 2001.02.04 Start by MIK: Popup Help
static const DWORD p_helpids[] = {	//11000
	IDC_BUTTON_DELETE,				HIDC_BUTTON_DELETE_TOOLBAR,				//ツールバーから機能削除
	IDC_BUTTON_INSERTSEPARATOR,		HIDC_BUTTON_INSERTSEPARATOR_TOOLBAR,	//セパレータ挿入
	IDC_BUTTON_INSERT,				HIDC_BUTTON_INSERT_TOOLBAR,				//ツールバーへ機能挿入
	IDC_BUTTON_ADD,					HIDC_BUTTON_ADD_TOOLBAR,				//ツールバーへ機能追加
	IDC_BUTTON_UP,					HIDC_BUTTON_UP_TOOLBAR,					//ツールバーの機能を上へ移動
	IDC_BUTTON_DOWN,				HIDC_BUTTON_DOWN_TOOLBAR,				//ツールバーの機能を下へ移動
	IDC_CHECK_TOOLBARISFLAT,		HIDC_CHECK_TOOLBARISFLAT,				//フラットなボタン
	IDC_COMBO_FUNCKIND,				HIDC_COMBO_FUNCKIND_TOOLBAR,			//機能の種別
	IDC_LIST_FUNC,					HIDC_LIST_FUNC_TOOLBAR,					//機能一覧
	IDC_LIST_RES,					HIDC_LIST_RES_TOOLBAR,					//ツールバー一覧
	IDC_BUTTON_INSERTWRAP,			HIDC_BUTTON_INSERTWRAP,					//ツールバー折返	// 2006.08.06 ryoji
	IDC_BUTTON_INITIALIZE,			HIDC_BUTTON_INITIALIZE_TOOLBAR,			//初期状態に戻す
	IDC_LABEL_MENUFUNCKIND,			(DWORD)-1,
	IDC_LABEL_MENUFUNC,				(DWORD)-1,
	IDC_LABEL_TOOLBAR,				(DWORD)-1,
//	IDC_STATIC,						-1,
	0, 0
};
//@@@ 2001.02.04 End

static void SetDataToolbarListItem( HWND hwndDlg, const CommonSetting& sCommonSet, int );

//	From Here Apr. 13, 2002 genta
/*!
	Owner Draw List Boxに指定の値を挿入する (Windows XPの問題回避用)
	
	Windows XP + manifestの時にLB_INSERTSTRINGが値0を受け付けないので
	とりあえず0以外の値を入れてから0に設定し直して回避する。
	1回目の挿入は0でなければ何でもいいはず。
	
	@param hWnd [in] リストボックスのウィンドウハンドル
	@param index [in] 挿入位置
	@param value [in] 挿入する値
	@return 挿入位置。エラーの時はLB_ERRまたはLB_ERRSPACE
	
	@date 2002.04.13 genta 
*/
int Listbox_INSERTDATA(
	HWND hWnd,              //!< handle to destination window 
	int index,          //!< item index
	int value
)
{
	int nIndex1 = List_InsertItemData( hWnd, index, 1 );
	if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
		TopErrorMessage( NULL, LS(STR_PROPCOMTOOL_ERR01), index, nIndex1 );
		return nIndex1;
	}
	else if( List_SetItemData( hWnd, nIndex1, value ) == LB_ERR ){
		TopErrorMessage( NULL, LS(STR_PROPCOMTOOL_ERR02), nIndex1 );
		return LB_ERR;
	}
	return nIndex1;
}

//	From Here Apr. 13, 2002 genta
/*!
	Owner Draw List Boxに指定の値を追加する (Windows XPの問題回避用)
	
	Windows XP + manifestの時にLB_ADDSTRINGが値0を受け付けないので
	とりあえず0以外の値を入れてから0に設定し直して回避する。
	1回目の挿入は0でなければ何でもいいはず。
	
	@param hWnd [in] リストボックスのウィンドウハンドル
	@param index [in] 挿入位置
	@param value [in] 挿入する値
	@return 挿入位置。エラーの時はLB_ERRまたはLB_ERRSPACE
	
	@date 2002.04.13 genta 
*/
int Listbox_ADDDATA(
	HWND hWnd,              //!< handle to destination window 
	int value
)
{
	int nIndex1 = List_AddItemData( hWnd, 1 );
	if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
		TopErrorMessage( NULL, LS(STR_PROPCOMTOOL_ERR03), nIndex1 );
		return nIndex1;
	}
	else if( List_SetItemData( hWnd, nIndex1, value ) == LB_ERR ){
		TopErrorMessage( NULL, LS(STR_PROPCOMTOOL_ERR04), nIndex1 );
		return LB_ERR;
	}
	return nIndex1;
}


HWND CDlgConfigChildToolbar::DoModeless( HINSTANCE hInstance, HWND hwndParent, SDlgConfigArg* pDlgConfigArg, int nTypeIndex )
{
	m_nCurrentTypeIndex = nTypeIndex;
	m_pDlgConfigArg = pDlgConfigArg;

	return CDialog::DoModeless( hInstance, hwndParent, IDD_PROP_TOOLBAR, 0, SW_SHOW );
}


BOOL CDlgConfigChildToolbar::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	m_hwndCombo = ::GetDlgItem( hwndDlg, IDC_COMBO_FUNCKIND );
	m_hwndFuncList = ::GetDlgItem( hwndDlg, IDC_LIST_FUNC );
	m_hwndResList = ::GetDlgItem( hwndDlg, IDC_LIST_RES );
	{
		// 2014.11.25 フォントの高さが正しくなかったバグを修正
		CTextWidthCalc calc(m_hwndResList);
		int nFontHeight = calc.GetTextHeight();
		m_nListItemHeight = 18; //Oct. 18, 2000 JEPRO 「ツールバー」タブでの機能アイテムの行間を少し狭くして表示行数を増やした(20→18 これ以上小さくしても効果はないようだ)
		if( m_nListItemHeight < nFontHeight ){
			m_nListItemHeight = nFontHeight;
			m_nListBoxTopMargin = 0;
		}else{
			m_nListBoxTopMargin = (m_nListItemHeight - (nFontHeight + 1)) / 2;
		}
	}

	BOOL result =  CDlgConfigChild::OnInitDialog( hwndDlg, wParam, lParam );

//	From Here Oct.14, 2000 JEPRO added	(Ref. CPropComCustmenu.cpp 内のWM_INITDIALOGを参考にした)
	/* キー選択時の処理 */
	::SendMessageCmd( hwndDlg, WM_COMMAND, MAKELONG( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)m_hwndCombo );
//	To Here Oct. 14, 2000

	::SetTimer( hwndDlg, 1, 300, NULL );

	return result;
}


BOOL CDlgConfigChildToolbar::OnDrawItem( WPARAM wParam, LPARAM lParam )
{
	int				idCtrl = (UINT) wParam;	/* コントロールのID */
	PDRAWITEMSTRUCT	pDis = (LPDRAWITEMSTRUCT) lParam;	/* 項目描画情報 */
	switch( idCtrl ){
	case IDC_LIST_RES:	/* ツールバーボタン結果リスト */
	case IDC_LIST_FUNC:	/* ボタン一覧リスト */
		DrawToolBarItemList( pDis );	/* ツールバーボタンリストのアイテム描画 */
		return TRUE;
	}
	return TRUE;
}


BOOL CDlgConfigChildToolbar::OnCbnSelChange( HWND hwndCtl, int wID )
{
	int		nIndex1;
	int		nIndex2;
	int		nNum;
	int		i;
	LRESULT	lResult;
	
	nIndex2 = Combo_GetCurSel( m_hwndCombo );

	List_ResetContent( m_hwndFuncList );

	/* 機能一覧に文字列をセット (リストボックス) */
	//	From Here Oct. 15, 2001 genta Lookupを使うように変更
	nNum = m_pDlgConfigArg->m_cLookup.GetItemCount( nIndex2 );
	for( i = 0; i < nNum; ++i ){
		nIndex1 = m_pDlgConfigArg->m_cLookup.Pos2FuncCode( nIndex2, i );
		int nbarNo = m_pDlgConfigArg->m_pcMenuDrawer->FindToolbarNoFromCommandId( nIndex1 );

		if( nbarNo >= 0 ){
			/* ツールバーボタンの情報をセット (リストボックス) */
			lResult = ::Listbox_ADDDATA( m_hwndFuncList, (LPARAM)nbarNo );
			if( lResult == LB_ERR || lResult == LB_ERRSPACE ){
				break;
			}
			lResult = List_SetItemHeight( m_hwndFuncList, lResult, m_nListItemHeight );
		}

	}
	return TRUE;
}


BOOL CDlgConfigChildToolbar::OnBnClicked( int wID )
{
	int		nIndex1;
	int		nIndex2;
	int		i;
	int		j;
	
	switch( wID ){
	case IDC_BUTTON_INSERTSEPARATOR:
		nIndex1 = List_GetCurSel( m_hwndResList );
		if( LB_ERR == nIndex1 ){
//			break;
			nIndex1 = 0;
		}
		//	From Here Apr. 13, 2002 genta
		nIndex1 = ::Listbox_INSERTDATA( m_hwndResList, nIndex1, 0 );
		if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
			break;
		}
		//	To Here Apr. 13, 2002 genta
		List_SetCurSel( m_hwndResList, nIndex1 );
		break;

// 2005/8/9 aroka 折返ボタンが押されたら、右のリストに「ツールバー折返」を追加する。
	case IDC_BUTTON_INSERTWRAP:
		nIndex1 = List_GetCurSel( m_hwndResList );
		if( LB_ERR == nIndex1 ){
//			break;
			nIndex1 = 0;
		}
		//	From Here Apr. 13, 2002 genta
		//	2010.06.25 Moca 折り返しのツールバーのボタン番号定数名を変更。最後ではなく固定値にする
		nIndex1 = ::Listbox_INSERTDATA( m_hwndResList, nIndex1, CMenuDrawer::TOOLBAR_BUTTON_F_TOOLBARWRAP );
		if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
			break;
		}
		//	To Here Apr. 13, 2002 genta
		List_SetCurSel( m_hwndResList, nIndex1 );
		break;

	case IDC_BUTTON_DELETE:
		nIndex1 = List_GetCurSel( m_hwndResList );
		if( LB_ERR == nIndex1 ){
			break;
		}
		i = List_DeleteString( m_hwndResList, nIndex1 );
		if( i == LB_ERR ){
			break;
		}
		if( nIndex1 >= i ){
			if( i == 0 ){
				i = List_SetCurSel( m_hwndResList, 0 );
			}else{
				i = List_SetCurSel( m_hwndResList, i - 1 );
			}
		}else{
			i = List_SetCurSel( m_hwndResList, nIndex1 );
		}
		break;

	case IDC_BUTTON_INSERT:
		nIndex1 = List_GetCurSel( m_hwndResList );
		if( LB_ERR == nIndex1 ){
//			break;
			nIndex1 = 0;
		}
		nIndex2 = List_GetCurSel( m_hwndFuncList );
		if( LB_ERR == nIndex2 ){
			break;
		}
		i = List_GetItemData( m_hwndFuncList, nIndex2 );
		//	From Here Apr. 13, 2002 genta
		nIndex1 = ::Listbox_INSERTDATA( m_hwndResList, nIndex1, i );
		if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
			break;
		}
		//	To Here Apr. 13, 2002 genta
		List_SetCurSel( m_hwndResList, nIndex1 + 1 );
		break;


	case IDC_BUTTON_ADD:
		nIndex1 = List_GetCount( m_hwndResList );
		nIndex2 = List_GetCurSel( m_hwndFuncList );
		if( LB_ERR == nIndex2 ){
			break;
		}
		i = List_GetItemData( m_hwndFuncList, nIndex2 );
		//	From Here Apr. 13, 2002 genta
		//	ここでは i != 0 だとは思うけど、一応保険です。
		nIndex1 = ::Listbox_INSERTDATA( m_hwndResList, nIndex1, i );
		if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
			TopErrorMessage( NULL, LS(STR_PROPCOMTOOL_ERR05), nIndex1 );
			break;
		}
		//	To Here Apr. 13, 2002 genta
		List_SetCurSel( m_hwndResList, nIndex1 );
		break;

	case IDC_BUTTON_UP:
		nIndex1 = List_GetCurSel( m_hwndResList );
		if( LB_ERR == nIndex1 || 0 >= nIndex1 ){
			break;
		}
		i = List_GetItemData( m_hwndResList, nIndex1 );

		j = List_DeleteString( m_hwndResList, nIndex1 );
		if( j == LB_ERR ){
			break;
		}
		//	From Here Apr. 13, 2002 genta
		nIndex1 = ::Listbox_INSERTDATA( m_hwndResList, nIndex1 - 1, i );
		if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
			TopErrorMessage( NULL, LS(STR_PROPCOMTOOL_ERR05), nIndex1 );
			break;
		}
		//	To Here Apr. 13, 2002 genta
		List_SetCurSel( m_hwndResList, nIndex1 );
		break;

	case IDC_BUTTON_DOWN:
		i = List_GetCount( m_hwndResList );
		nIndex1 = List_GetCurSel( m_hwndResList );
		if( LB_ERR == nIndex1 || nIndex1 + 1 >= i ){
			break;
		}
		i = List_GetItemData( m_hwndResList, nIndex1 );

		j = List_DeleteString( m_hwndResList, nIndex1 );
		if( j == LB_ERR ){
			break;
		}
		//	From Here Apr. 13, 2002 genta
		nIndex1 = ::Listbox_INSERTDATA( m_hwndResList, nIndex1 + 1, i );
		if( nIndex1 == LB_ERR || nIndex1 == LB_ERRSPACE ){
			TopErrorMessage( NULL, LS(STR_PROPCOMTOOL_ERR05), nIndex1 );
			break;
		}
		List_SetCurSel( m_hwndResList, nIndex1 );
		//	To Here Apr. 13, 2002 genta
		break;
	// 2014.10.03 初期化
	case IDC_BUTTON_INITIALIZE:
		if( IDYES == ConfirmMessage( GetHwnd(), LS(STR_PROPCOMTOOL_INIT) ) ){
			CShareData::InitToolButtons( &m_Common );
			List_ResetContent( m_hwndResList );
			SetDataToolbarListItem( GetHwnd(), m_Common, m_nListItemHeight );
		}
		break;
	}
	return FALSE;
}


BOOL CDlgConfigChildToolbar::OnTimer( WPARAM wParam )
{
	HWND hwndDlg = GetHwnd();

	int		nIndex1 = List_GetCurSel( m_hwndResList );
	int		nIndex2 = List_GetCurSel( m_hwndFuncList );
	int		i = List_GetCount( m_hwndResList );
	if( LB_ERR == nIndex1 ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELETE ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_UP ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DOWN ), FALSE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELETE ), TRUE );
		if( nIndex1 <= 0 ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_UP ), FALSE );
		}else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_UP ), TRUE );
		}
		if( nIndex1 + 1 >= i ){
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DOWN ), FALSE );
		}else{
			::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DOWN ), TRUE );
		}
	}
	if( LB_ERR == nIndex1 || LB_ERR == nIndex2 ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT ), FALSE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_INSERT ), TRUE );
	}
	if( LB_ERR == nIndex2 ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADD ), FALSE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADD ), TRUE );
	}
	return TRUE;
}


BOOL CDlgConfigChildToolbar::OnDestroy()
{
	::KillTimer( GetHwnd(), 1 );
	return CDlgConfigChild::OnDestroy();
}


/* ダイアログデータの設定 Toolbar */
static void SetDataToolbarListItem( HWND hwndDlg, const CommonSetting& sCommonSet, int nHeight )
{
	int			i;
	LRESULT		lResult;

	/* コントロールのハンドルを取得 */
	HWND hwndResList = ::GetDlgItem( hwndDlg, IDC_LIST_RES );

	/* ツールバーボタンの情報をセット(リストボックス)*/
	for( i = 0; i < sCommonSet.m_sToolBar.m_nToolBarButtonNum; ++i ){
		//	From Here Apr. 13, 2002 genta
		lResult = ::Listbox_ADDDATA( hwndResList, (LPARAM)sCommonSet.m_sToolBar.m_nToolBarButtonIdxArr[i] );
		if( lResult == LB_ERR || lResult == LB_ERRSPACE ){
			break;
		}
		//	To Here Apr. 13, 2002 genta
		lResult = List_SetItemHeight( hwndResList, lResult, nHeight );
	}
	/* ツールバーの先頭の項目を選択(リストボックス)*/
	List_SetCurSel( hwndResList, 0 );	//Oct. 14, 2000 JEPRO ここをコメントアウトすると先頭項目が選択されなくなる
}


/* ダイアログデータの設定 Toolbar */
void CDlgConfigChildToolbar::SetData()
{
	HWND	hwndDlg = GetHwnd();
	HWND	hwndCombo = m_hwndCombo;

	/* 機能種別一覧に文字列をセット(コンボボックス) */
	m_pDlgConfigArg->m_cLookup.SetCategory2Combo( hwndCombo );	//	Oct. 15, 2001 genta
	
	/* 種別の先頭の項目を選択(コンボボックス) */
	Combo_SetCurSel( hwndCombo, 0 );	//Oct. 14, 2000 JEPRO JEPRO 「--未定義--」を表示させないように大元 Funcode.cpp で変更してある
	::PostMessageCmd( hwndCombo, WM_COMMAND, MAKELONG( IDC_COMBO_FUNCKIND, CBN_SELCHANGE ), (LPARAM)hwndCombo );

	SetDataToolbarListItem( hwndDlg, m_Common, m_nListItemHeight );

	/* フラットツールバーにする／しない  */
	::CheckDlgButton( hwndDlg, IDC_CHECK_TOOLBARISFLAT, m_Common.m_sToolBar.m_bToolBarIsFlat );
	return;
}

/* ダイアログデータの取得 Toolbar */
int CDlgConfigChildToolbar::GetData()
{
	HWND hwndDlg = GetHwnd();
	HWND hwndResList = m_hwndResList;
	int		i;
	int		j;
	int		k;

	/* ツールバーボタンの数 */
	m_Common.m_sToolBar.m_nToolBarButtonNum = List_GetCount( hwndResList );

	/* ツールバーボタンの情報を取得 */
	k = 0;
	for( i = 0; i < m_Common.m_sToolBar.m_nToolBarButtonNum; ++i ){
		j = List_GetItemData( hwndResList, i );
		if( LB_ERR != j ){
			m_Common.m_sToolBar.m_nToolBarButtonIdxArr[k] = j;
			k++;
		}
	}
	m_Common.m_sToolBar.m_nToolBarButtonNum = k;

	/* フラットツールバーにする／しない  */
	m_Common.m_sToolBar.m_bToolBarIsFlat = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_TOOLBARISFLAT );

	return TRUE;
}

/* ツールバーボタンリストのアイテム描画
	@date 2003.08.27 Moca システムカラーのブラシはCreateSolidBrushをやめGetSysColorBrushに
	@date 2005.08.09 aroka CPropCommon.cpp から移動
	@date 2007.11.02 ryoji ボタンとセパレータとで処理を分ける
*/
void CDlgConfigChildToolbar::DrawToolBarItemList( DRAWITEMSTRUCT* pDis )
{
	TBBUTTON	tbb;
	HBRUSH		hBrush;
	RECT		rc;
	RECT		rc0;
	RECT		rc1;
	RECT		rc2;


//	hBrush = ::CreateSolidBrush( ::GetSysColor( COLOR_WINDOW ) );
	hBrush = ::GetSysColorBrush( COLOR_WINDOW );
	::FillRect( pDis->hDC, &pDis->rcItem, hBrush );
//	::DeleteObject( hBrush );

	rc  = pDis->rcItem;
	rc0 = pDis->rcItem;
	rc0.left += 18;//20 //Oct. 18, 2000 JEPRO 行先頭のアイコンとそれに続くキャプションとの間を少し詰めた(20→18)
	rc1 = rc0;
	rc2 = rc0;

	if( (int)pDis->itemID < 0 ){
	}else{

//@@@ 2002.01.03 YAZAKI m_tbMyButtonなどをCShareDataからCMenuDrawerへ移動したことによる修正。
//		tbb = m_cShareData.m_tbMyButton[pDis->itemData];
//		tbb = m_pcMenuDrawer->m_tbMyButton[pDis->itemData];
		tbb = m_pDlgConfigArg->m_pcMenuDrawer->getButton(pDis->itemData);

		// ボタンとセパレータとで処理を分ける	2007.11.02 ryoji
		WCHAR	szLabel[256];
		if( tbb.fsStyle & TBSTYLE_SEP ){
			// テキストだけ表示する
			if( tbb.idCommand == F_SEPARATOR ){
				auto_strncpy( szLabel, LSW(STR_PROPCOMTOOL_ITEM1), _countof(szLabel) - 1 );	// nLength 未使用 2003/01/09 Moca
				szLabel[_countof(szLabel) - 1] = L'\0';
			}else if( tbb.idCommand == F_MENU_NOT_USED_FIRST ){
				// ツールバー折返
				auto_strncpy( szLabel, LSW(STR_PROPCOMTOOL_ITEM2), _countof(szLabel) - 1 );
				szLabel[_countof(szLabel) - 1] = L'\0';
			}else{
				auto_strncpy( szLabel, LSW(STR_PROPCOMTOOL_ITEM3), _countof(szLabel) - 1 );
				szLabel[_countof(szLabel) - 1] = L'\0';
			}
		//	From Here Oct. 15, 2001 genta
		}else{
			// アイコンとテキストを表示する
			m_pDlgConfigArg->m_pcIcons->Draw( tbb.iBitmap, pDis->hDC, rc.left + 2, rc.top + 2, ILD_NORMAL );
			m_pDlgConfigArg->m_cLookup.Funccode2Name( tbb.idCommand, szLabel, _countof( szLabel ) );
		}
		//	To Here Oct. 15, 2001 genta

		/* アイテムが選択されている */
		if( pDis->itemState & ODS_SELECTED ){
//			hBrush = ::CreateSolidBrush( ::GetSysColor( COLOR_HIGHLIGHT ) );
			hBrush = ::GetSysColorBrush( COLOR_HIGHLIGHT );
			::SetTextColor( pDis->hDC, ::GetSysColor( COLOR_HIGHLIGHTTEXT ) );
		}else{
//			hBrush = ::CreateSolidBrush( ::GetSysColor( COLOR_WINDOW ) );
			hBrush = ::GetSysColorBrush( COLOR_WINDOW );
			::SetTextColor( pDis->hDC, ::GetSysColor( COLOR_WINDOWTEXT ) );
		}
		rc1.left++;
		rc1.top++;
		rc1.right--;
		rc1.bottom--;
		::FillRect( pDis->hDC, &rc1, hBrush );
//		::DeleteObject( hBrush );

		::SetBkMode( pDis->hDC, TRANSPARENT );
		// 2014.11.25 topマージンが2固定だとフォントが大きい時に見切れるので変数に変更
		TextOutW_AnyBuild( pDis->hDC, rc1.left + 4, rc1.top + m_nListBoxTopMargin, szLabel, wcslen( szLabel ) );

	}

	/* アイテムにフォーカスがある */
	if( pDis->itemState & ODS_FOCUS ){
		::DrawFocusRect( pDis->hDC, &rc2 );
	}
	return;
}


LPVOID CDlgConfigChildToolbar::GetHelpIdTable()
{
	return (LPVOID)p_helpids;
}
