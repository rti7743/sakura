/*!	@file
	@brief 共通設定ダイアログボックス、「強調キーワード」ページ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, MIK
	Copyright (C) 2001, genta, MIK
	Copyright (C) 2002, YAZAKI, MIK, genta
	Copyright (C) 2003, KEITA
	Copyright (C) 2005, genta, Moca
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2009, ryoji
	Copyright (C) 2014, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "prop/CDlgConfigChildKeyword.h"
#include "prop/CDlgConfig.h"
#include "env/CShareData.h"
#include "env/CDocTypeManager.h"
#include "typeprop/CImpExpManager.h"	// 20210/4/23 Uchi
#include "dlg/CDlgInput1.h"
#include "util/shell.h"
#include <memory>
#include "sakura_rc.h"
#include "sakura.hh"


//@@@ 2001.02.04 Start by MIK: Popup Help
static const DWORD p_helpids[] = {	//10800
	IDC_BUTTON_ADDSET,				HIDC_BUTTON_ADDSET,			//キーワードセット追加
	IDC_BUTTON_DELSET,				HIDC_BUTTON_DELSET,			//キーワードセット削除
	IDC_BUTTON_ADDKEYWORD,			HIDC_BUTTON_ADDKEYWORD,		//キーワード追加
	IDC_BUTTON_EDITKEYWORD,			HIDC_BUTTON_EDITKEYWORD,	//キーワード編集
	IDC_BUTTON_DELKEYWORD,			HIDC_BUTTON_DELKEYWORD,		//キーワード削除
	IDC_BUTTON_IMPORT,				HIDC_BUTTON_IMPORT_KEYWORD,	//インポート
	IDC_BUTTON_EXPORT,				HIDC_BUTTON_EXPORT_KEYWORD,	//エクスポート
	IDC_CHECK_KEYWORDCASE,			HIDC_CHECK_KEYWORDCASE,		//キーワードの英大文字小文字区別
	IDC_COMBO_SET,					HIDC_COMBO_SET,				//強調キーワードセット名
	IDC_LIST_KEYWORD,				HIDC_LIST_KEYWORD,			//キーワード一覧
	IDC_BUTTON_KEYCLEAN		,		HIDC_BUTTON_KEYCLEAN,		//キーワード整理	// 2006.08.06 ryoji
	IDC_BUTTON_KEYSETRENAME,		HIDC_BUTTON_KEYSETRENAME,	//セットの名称変更	// 2006.08.06 ryoji
//	IDC_STATIC,						-1,
	0, 0
};
//@@@ 2001.02.04 End


HWND CDlgConfigChildKeyword::DoModeless( HINSTANCE hInstance, HWND hwndParent, SDlgConfigArg* pDlgConfigArg, int nTypeIndex )
{
	m_nCurrentTypeIndex = nTypeIndex;
	m_pDlgConfigArg  = pDlgConfigArg;

	return CDialog::DoModeless( hInstance, hwndParent, IDD_PROP_KEYWORD, 0, SW_SHOW );
}


BOOL CDlgConfigChildKeyword::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	BOOL result =  CDlgConfigChild::OnInitDialog( hwndDlg, wParam, lParam );

	/* コントロールのハンドルを取得 */
	HWND hwndCOMBO_SET = ::GetDlgItem( hwndDlg, IDC_COMBO_SET );
	HWND hwndLIST_KEYWORD = ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD );
	if( m_pDlgConfigArg->m_nKeywordSet1 != -1 ){
		Combo_SetCurSel( hwndCOMBO_SET, m_pDlgConfigArg->m_nKeywordSet1 );
	}

	RECT		rc;
	LV_COLUMN	lvc;
	::GetWindowRect( hwndLIST_KEYWORD, &rc );
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.fmt = LVCFMT_LEFT;
	lvc.cx = rc.right - rc.left;
	lvc.pszText = const_cast<TCHAR*>(_T(""));
	lvc.iSubItem = 0;
	ListView_InsertColumn( hwndLIST_KEYWORD, 0, &lvc );

	LONG_PTR lStyle = ::GetWindowLongPtr( hwndLIST_KEYWORD, GWL_STYLE );
	::SetWindowLongPtr( hwndLIST_KEYWORD, GWL_STYLE, lStyle | LVS_SHOWSELALWAYS );

	/* コントロール更新のタイミング用のタイマーを起動 */
	::SetTimer( hwndDlg, 1, 300, NULL );

	return result;
}


BOOL CDlgConfigChildKeyword::OnNotify( WPARAM wParam, LPARAM lParam )
{
	HWND hwndDlg = GetHwnd();

	NMHDR*	pNMHDR = (NMHDR*)lParam;
	HWND hwndLIST_KEYWORD = ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD );
	if( hwndLIST_KEYWORD == pNMHDR->hwndFrom ){
		switch( pNMHDR->code ){
		case NM_DBLCLK:
//				MYTRACE( _T("NM_DBLCLK     \n") );
			/* リスト中で選択されているキーワードを編集する */
			Edit_List_KeyWord( hwndLIST_KEYWORD );
			return TRUE;
		case LVN_BEGINLABELEDIT:
		{
#ifdef _DEBUG
			LV_DISPINFO*	plvdi = (LV_DISPINFO*)lParam;
			LV_ITEM*		plvi = &plvdi->item;
			MYTRACE( _T("LVN_BEGINLABELEDIT\n") );
											MYTRACE( _T("	plvi->mask =[%xh]\n"), plvi->mask );
											MYTRACE( _T("	plvi->iItem =[%d]\n"), plvi->iItem );
											MYTRACE( _T("	plvi->iSubItem =[%d]\n"), plvi->iSubItem );
			if (plvi->mask & LVIF_STATE)	MYTRACE( _T("	plvi->state =[%xf]\n"), plvi->state );
											MYTRACE( _T("	plvi->stateMask =[%xh]\n"), plvi->stateMask );
			if (plvi->mask & LVIF_TEXT)		MYTRACE( _T("	plvi->pszText =[%ts]\n"), plvi->pszText );
											MYTRACE( _T("	plvi->cchTextMax=[%d]\n"), plvi->cchTextMax );
			if (plvi->mask & LVIF_IMAGE)	MYTRACE( _T("	plvi->iImage=[%d]\n"), plvi->iImage );
			if (plvi->mask & LVIF_PARAM)	MYTRACE( _T("	plvi->lParam=[%xh(%d)]\n"), plvi->lParam, plvi->lParam );
#endif
			return TRUE;
		}
		case LVN_ENDLABELEDIT:
		{
			LV_DISPINFO*	plvdi = (LV_DISPINFO*)lParam;
			LV_ITEM*		plvi = &plvdi->item;
#ifdef _DEBUG
			MYTRACE( _T("LVN_ENDLABELEDIT\n") );
											MYTRACE( _T("	plvi->mask =[%xh]\n"), plvi->mask );
											MYTRACE( _T("	plvi->iItem =[%d]\n"), plvi->iItem );
											MYTRACE( _T("	plvi->iSubItem =[%d]\n"), plvi->iSubItem );
			if (plvi->mask & LVIF_STATE)	MYTRACE( _T("	plvi->state =[%xf]\n"), plvi->state );
											MYTRACE( _T("	plvi->stateMask =[%xh]\n"), plvi->stateMask );
			if (plvi->mask & LVIF_TEXT)		MYTRACE( _T("	plvi->pszText =[%ts]\n"), plvi->pszText  );
											MYTRACE( _T("	plvi->cchTextMax=[%d]\n"), plvi->cchTextMax );
			if (plvi->mask & LVIF_IMAGE)	MYTRACE( _T("	plvi->iImage=[%d]\n"), plvi->iImage );
			if (plvi->mask & LVIF_PARAM)	MYTRACE( _T("	plvi->lParam=[%xh(%d)]\n"), plvi->lParam, plvi->lParam );
#endif
			if( NULL == plvi->pszText ){
				return TRUE;
			}
			if( plvi->pszText[0] != _T('\0') ){
				if( MAX_KEYWORDLEN < _tcslen( plvi->pszText ) ){
					InfoMessage( hwndDlg, LS(STR_PROPCOMKEYWORD_ERR_LEN), MAX_KEYWORDLEN );
					return TRUE;
				}
				/* ｎ番目のセットにキーワードを編集 */
				m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.UpdateKeyWord(
					m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx,
					plvi->lParam,
					to_wchar(plvi->pszText)
				);
			}else{
				/* ｎ番目のセットのｍ番目のキーワードを削除 */
				m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.DelKeyWord( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, plvi->lParam );
			}
			/* ダイアログデータの設定 Keyword 指定キーワードセットの設定 */
			SetKeyWordSet( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );

			ListView_SetItemState( hwndLIST_KEYWORD, plvi->iItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );

			return TRUE;
		}
		case LVN_KEYDOWN:
//				MYTRACE( _T("LVN_KEYDOWN\n") );
		{
			LV_KEYDOWN*		pnkd = (LV_KEYDOWN*)lParam;
			switch( pnkd->wVKey ){
			case VK_DELETE:
				/* リスト中で選択されているキーワードを削除する */
				Delete_List_KeyWord( hwndLIST_KEYWORD );
				break;
			case VK_SPACE:
				/* リスト中で選択されているキーワードを編集する */
				Edit_List_KeyWord( hwndLIST_KEYWORD );
				break;
			}
			return TRUE;
		}
		}
	}
	return TRUE;
}


BOOL CDlgConfigChildKeyword::OnCbnSelChange( HWND hwndCtl, int wID )
{
	if( IDC_COMBO_SET == wID ){
		int nIndex1 = Combo_GetCurSel( hwndCtl );
		/* ダイアログデータの設定 Keyword 指定キーワードセットの設定 */
		SetKeyWordSet( nIndex1 );
	}
	return TRUE;
}

BOOL CDlgConfigChildKeyword::OnBnClicked( int wID )
{
	HWND hwndDlg = GetHwnd();
	HWND hwndCOMBO_SET = ::GetDlgItem( hwndDlg, IDC_COMBO_SET );
	HWND hwndLIST_KEYWORD = ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD );
	int					i;
	int					nIndex1;
	wchar_t				szKeyWord[MAX_KEYWORDLEN + 1];
	CDlgInput1			cDlgInput1;
	
	switch( wID ){
	case IDC_BUTTON_ADDSET:	/* セット追加 */
		if( MAX_SETNUM <= m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nKeyWordSetNum ){
			InfoMessage( hwndDlg, LS(STR_PROPCOMKEYWORD_SETMAX), MAX_SETNUM );
			return TRUE;
		}
		/* モードレスダイアログの表示 */
		szKeyWord[0] = L'\0';
		//	Oct. 5, 2002 genta 長さ制限の設定を修正．バッファオーバーランしていた．
		if( !cDlgInput1.DoModal(
			G_AppInstance(),
			hwndDlg,
			LS(STR_PROPCOMKEYWORD_SETNAME1),
			LS(STR_PROPCOMKEYWORD_SETNAME2),
			MAX_SETNAMELEN,
			szKeyWord
			)
		){
			return TRUE;
		}
		if( szKeyWord[0] != L'\0' ){
			/* セットの追加 */
			m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.AddKeyWordSet( szKeyWord, false );

			m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx = m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nKeyWordSetNum - 1;

			/* ダイアログデータの設定 Keyword */
			SetData();
		}
		return TRUE;
	case IDC_BUTTON_DELSET:	/* セット削除 */
		nIndex1 = Combo_GetCurSel( hwndCOMBO_SET );
		if( CB_ERR == nIndex1 ){
			return TRUE;
		}
		/* 削除対象のセットを使用しているファイルタイプを列挙 */
		static TCHAR		pszLabel[1024];
		_tcscpy( pszLabel, _T("") );
		for( i = 0; i < GetDllShareData().m_nTypesCount; ++i ){
			std::auto_ptr<STypeConfig> type(new STypeConfig());
			CDocTypeManager().GetTypeConfig( CTypeConfig(i), *type );
			// 2002/04/25 YAZAKI STypeConfig全体を保持する必要はないし、m_pShareDataを直接見ても問題ない。
			if( nIndex1 == m_pDlgConfigArg->m_Types_nKeyWordSetIdx[i].index[0]
			||  nIndex1 == m_pDlgConfigArg->m_Types_nKeyWordSetIdx[i].index[1]
			||  nIndex1 == m_pDlgConfigArg->m_Types_nKeyWordSetIdx[i].index[2]
			||  nIndex1 == m_pDlgConfigArg->m_Types_nKeyWordSetIdx[i].index[3]
			||  nIndex1 == m_pDlgConfigArg->m_Types_nKeyWordSetIdx[i].index[4]
			||  nIndex1 == m_pDlgConfigArg->m_Types_nKeyWordSetIdx[i].index[5]
			||  nIndex1 == m_pDlgConfigArg->m_Types_nKeyWordSetIdx[i].index[6]
			||  nIndex1 == m_pDlgConfigArg->m_Types_nKeyWordSetIdx[i].index[7]
			||  nIndex1 == m_pDlgConfigArg->m_Types_nKeyWordSetIdx[i].index[8]
			||  nIndex1 == m_pDlgConfigArg->m_Types_nKeyWordSetIdx[i].index[9] ){
				_tcscat( pszLabel, _T("・") );
				_tcscat( pszLabel, type->m_szTypeName );
				_tcscat( pszLabel, _T("（") );
				_tcscat( pszLabel, type->m_szTypeExts );
				_tcscat( pszLabel, _T("）") );
				_tcscat( pszLabel, _T("\n") );
			}
		}
		if( IDCANCEL == ::MYMESSAGEBOX(	hwndDlg, MB_OKCANCEL | MB_ICONQUESTION, GSTR_APPNAME,
			LS(STR_PROPCOMKEYWORD_SETDEL),
			m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetTypeName( nIndex1 ),
			pszLabel
		) ){
			return TRUE;
		}
		/* 削除対象のセットを使用しているファイルタイプのセットをクリア */
		for( i = 0; i < GetDllShareData().m_nTypesCount; ++i ){
			// 2002/04/25 YAZAKI STypeConfig全体を保持する必要はない。
			for( int j = 0; j < MAX_KEYWORDSET_PER_TYPE; j++ ){
				if( nIndex1 == m_pDlgConfigArg->m_Types_nKeyWordSetIdx[i].index[j] ){
					m_pDlgConfigArg->m_Types_nKeyWordSetIdx[i].index[j] = -1;
				}
				else if( nIndex1 < m_pDlgConfigArg->m_Types_nKeyWordSetIdx[i].index[j] ){
					m_pDlgConfigArg->m_Types_nKeyWordSetIdx[i].index[j]--;
				}
			}
		}
		/* ｎ番目のセットを削除 */
		m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.DelKeyWordSet( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
		/* ダイアログデータの設定 Keyword */
		SetData();
		return TRUE;
	case IDC_BUTTON_KEYSETRENAME: // キーワードセットの名称変更
		// モードレスダイアログの表示
		wcscpy( szKeyWord, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetTypeName( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx ) );
		{
			BOOL bDlgInputResult = cDlgInput1.DoModal(
				G_AppInstance(),
				hwndDlg,
				LS(STR_PROPCOMKEYWORD_RENAME1),
				LS(STR_PROPCOMKEYWORD_RENAME2),
				MAX_SETNAMELEN,
				szKeyWord
			);
			if( !bDlgInputResult ){
				return TRUE;
			}
		}
		if( szKeyWord[0] != L'\0' ){
			m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.SetTypeName( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, szKeyWord );

			// ダイアログデータの設定 Keyword
			SetData();
		}
		return TRUE;
	case IDC_CHECK_KEYWORDCASE:	/* キーワードの英大文字小文字区別 */
//					m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_bKEYWORDCASEArr[ m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx ] = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_KEYWORDCASE );	//MIK 2000.12.01 case sense
		m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.SetKeyWordCase(m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_KEYWORDCASE ));			//MIK 2000.12.01 case sense
		return TRUE;
	case IDC_BUTTON_ADDKEYWORD:	/* キーワード追加 */
		/* ｎ番目のセットのキーワードの数を返す */
		if( !m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.CanAddKeyWord( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx ) ){
			InfoMessage( hwndDlg, LS(STR_PROPCOMKEYWORD_KEYMAX) );
			return TRUE;
		}
		/* モードレスダイアログの表示 */
		szKeyWord[0] = L'\0';
		if( !cDlgInput1.DoModal( G_AppInstance(), hwndDlg, LS(STR_PROPCOMKEYWORD_KEYADD1), LS(STR_PROPCOMKEYWORD_KEYADD2), MAX_KEYWORDLEN, szKeyWord ) ){
			return TRUE;
		}
		if( szKeyWord[0] != L'\0' ){
			/* ｎ番目のセットにキーワードを追加 */
			if( 0 == m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.AddKeyWord( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, szKeyWord ) ){
				// ダイアログデータの設定 Keyword 指定キーワードセットの設定
				SetKeyWordSet( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
			}
		}
		return TRUE;
	case IDC_BUTTON_EDITKEYWORD:	/* キーワード編集 */
		/* リスト中で選択されているキーワードを編集する */
		Edit_List_KeyWord( hwndLIST_KEYWORD );
		return TRUE;
	case IDC_BUTTON_DELKEYWORD:	/* キーワード削除 */
		/* リスト中で選択されているキーワードを削除する */
		Delete_List_KeyWord( hwndLIST_KEYWORD );
		return TRUE;
	// From Here 2005.01.26 Moca
	case IDC_BUTTON_KEYCLEAN:
		Clean_List_KeyWord( hwndLIST_KEYWORD );
		return TRUE;
	// To Here 2005.01.26 Moca
	case IDC_BUTTON_IMPORT:	/* インポート */
		/* リスト中のキーワードをインポートする */
		Import_List_KeyWord( hwndLIST_KEYWORD );
		return TRUE;
	case IDC_BUTTON_EXPORT:	/* エクスポート */
		/* リスト中のキーワードをエクスポートする */
		Export_List_KeyWord( hwndLIST_KEYWORD );
		return TRUE;
	}
	return FALSE;
}


BOOL CDlgConfigChildKeyword::OnDestroy()
{
	::KillTimer( GetHwnd(), 1 );
	return CDlgConfigChild::OnDestroy();
}

/* リスト中で選択されているキーワードを編集する */
void CDlgConfigChildKeyword::Edit_List_KeyWord( HWND hwndLIST_KEYWORD )
{
	HWND hwndDlg = GetHwnd();

	int			nIndex1;
	LV_ITEM		lvi;
	wchar_t		szKeyWord[MAX_KEYWORDLEN + 1];
	CDlgInput1	cDlgInput1;

	nIndex1 = ListView_GetNextItem( hwndLIST_KEYWORD, -1, LVNI_ALL | LVNI_SELECTED );
	if( -1 == nIndex1 ){
		return;
	}
	lvi.mask = LVIF_PARAM;
	lvi.iItem = nIndex1;
	lvi.iSubItem = 0;
	ListView_GetItem( hwndLIST_KEYWORD, &lvi );

	/* ｎ番目のセットのｍ番目のキーワードを返す */
	wcscpy( szKeyWord, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWord( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, lvi.lParam ) );

	/* モードレスダイアログの表示 */
	if( !cDlgInput1.DoModal( G_AppInstance(), hwndDlg, LS(STR_PROPCOMKEYWORD_KEYEDIT1), LS(STR_PROPCOMKEYWORD_KEYEDIT2), MAX_KEYWORDLEN, szKeyWord ) ){
		return;
	}
	if( szKeyWord[0] != L'\0' ){
		/* ｎ番目のセットにキーワードを編集 */
		m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.UpdateKeyWord(
			m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx,
			lvi.lParam,
			szKeyWord
		);
	}else{
		/* ｎ番目のセットのｍ番目のキーワードを削除 */
		m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.DelKeyWord( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, lvi.lParam );
	}
	/* ダイアログデータの設定 Keyword 指定キーワードセットの設定 */
	SetKeyWordSet( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );

	ListView_SetItemState( hwndLIST_KEYWORD, nIndex1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );
	return;
}



/* リスト中で選択されているキーワードを削除する */
void CDlgConfigChildKeyword::Delete_List_KeyWord( HWND hwndLIST_KEYWORD )
{
	HWND hwndDlg = GetHwnd();

	int			nIndex1;
	LV_ITEM		lvi;

	nIndex1 = ListView_GetNextItem( hwndLIST_KEYWORD, -1, LVNI_ALL | LVNI_SELECTED );
	if( -1 == nIndex1 ){
		return;
	}
	lvi.mask = LVIF_PARAM;
	lvi.iItem = nIndex1;
	lvi.iSubItem = 0;
	ListView_GetItem( hwndLIST_KEYWORD, &lvi );
	/* ｎ番目のセットのｍ番目のキーワードを削除 */
	m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.DelKeyWord( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, lvi.lParam );
	/* ダイアログデータの設定 Keyword 指定キーワードセットの設定 */
	SetKeyWordSet( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	ListView_SetItemState( hwndLIST_KEYWORD, nIndex1, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED );

	//キーワード数を表示する。
	DispKeywordCount();

	return;
}


/* リスト中のキーワードをインポートする */
void CDlgConfigChildKeyword::Import_List_KeyWord( HWND hwndLIST_KEYWORD )
{
	HWND hwndDlg = GetHwnd();

	bool	bCase = false;
	int		nIdx = m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx;
	m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.SetKeyWordCase( nIdx, bCase );
	CImpExpKeyWord	cImpExpKeyWord( m_Common, nIdx, bCase );

	// インポート
	if (!cImpExpKeyWord.ImportUI( G_AppInstance(), hwndDlg )) {
		// インポートをしていない
		return;
	}

	/* ダイアログデータの設定 Keyword 指定キーワードセットの設定 */
	SetKeyWordSet( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	return;
}


/* リスト中のキーワードをエクスポートする */
void CDlgConfigChildKeyword::Export_List_KeyWord( HWND hwndLIST_KEYWORD )
{
	HWND hwndDlg = GetHwnd();

	/* ダイアログデータの設定 Keyword 指定キーワードセットの設定 */
	SetKeyWordSet( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );

	bool	bCase;
	CImpExpKeyWord	cImpExpKeyWord( m_Common, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx, bCase );

	// エクスポート
	if (!cImpExpKeyWord.ExportUI( G_AppInstance(), hwndDlg )) {
		// エクスポートをしていない
		return;
	}
}


//! キーワードを整頓する
void CDlgConfigChildKeyword::Clean_List_KeyWord( HWND hwndLIST_KEYWORD )
{
	HWND hwndDlg = GetHwnd();

	if( IDYES == ::MessageBox( hwndDlg, LS(STR_PROPCOMKEYWORD_DEL),
			GSTR_APPNAME, MB_YESNO | MB_ICONQUESTION ) ){	// 2009.03.26 ryoji MB_ICONSTOP->MB_ICONQUESTION
		if( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.CleanKeyWords( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx ) ){
		}
		SetKeyWordSet( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	}
}

/* ダイアログデータの設定 Keyword */
void CDlgConfigChildKeyword::SetData()
{
	HWND hwndDlg = GetHwnd();

	int		i;
	HWND	hwndWork;


	/* セット名コンボボックスの値セット */
	hwndWork = ::GetDlgItem( hwndDlg, IDC_COMBO_SET );
	Combo_ResetContent( hwndWork );  /* コンボボックスを空にする */
	if( 0 < m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nKeyWordSetNum ){
		for( i = 0; i < m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nKeyWordSetNum; ++i ){
			Combo_AddString( hwndWork, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetTypeName( i ) );
		}
		/* セット名コンボボックスのデフォルト選択 */
		Combo_SetCurSel( hwndWork, m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );

		/* ダイアログデータの設定 Keyword 指定キーワードセットの設定 */
		SetKeyWordSet( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	}else{
		/* ダイアログデータの設定 Keyword 指定キーワードセットの設定 */
		SetKeyWordSet( -1 );
	}

	return;
}


/* ダイアログデータの設定 Keyword 指定キーワードセットの設定 */
void CDlgConfigChildKeyword::SetKeyWordSet( int nIdx )
{
	HWND hwndDlg = GetHwnd();

	int		i;
	int		nNum;
	HWND	hwndList;
	LV_ITEM	lvi;

	ListView_DeleteAllItems( ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD ) );
	if( 0 <= nIdx ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELSET ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_KEYWORDCASE ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADDKEYWORD ), TRUE );
		//	Jan. 29, 2005 genta キーワードセット切り替え直後はキーワードは未選択
		//	そのため有効にしてすぐにタイマーで無効になる．
		//	なのでここで無効にしておく．
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_EDITKEYWORD ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELKEYWORD ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_IMPORT ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_EXPORT ), TRUE );
	}else{
		::CheckDlgButton( hwndDlg, IDC_CHECK_KEYWORDCASE, FALSE );

		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELSET ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_CHECK_KEYWORDCASE ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_ADDKEYWORD ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_EDITKEYWORD ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_DELKEYWORD ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_IMPORT ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_BUTTON_EXPORT ), FALSE );
		return;
	}

	/* キーワードの英大文字小文字区別 */
	if( true == m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWordCase(nIdx) ){		//MIK 2000.12.01 case sense
		::CheckDlgButton( hwndDlg, IDC_CHECK_KEYWORDCASE, TRUE );
	}else{
		::CheckDlgButton( hwndDlg, IDC_CHECK_KEYWORDCASE, FALSE );
	}

	/* ｎ番目のセットのキーワードの数を返す */
	nNum = m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWordNum( nIdx );
	hwndList = ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD );

	// 2005.01.25 Moca/genta リスト追加中は再描画を抑制してすばやく表示
	::SendMessageAny( hwndList, WM_SETREDRAW, FALSE, 0 );

	for( i = 0; i < nNum; ++i ){
		/* ｎ番目のセットのｍ番目のキーワードを返す */
		const TCHAR* pszKeyWord = to_tchar(m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWord( nIdx, i ));

		lvi.mask = LVIF_TEXT | LVIF_PARAM;
		lvi.pszText = const_cast<TCHAR*>(pszKeyWord);
		lvi.iItem = i;
		lvi.iSubItem = 0;
		lvi.lParam	= i;
		ListView_InsertItem( hwndList, &lvi );

	}
	m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx = nIdx;

	// 2005.01.25 Moca/genta リスト追加完了のため再描画許可
	::SendMessageAny( hwndList, WM_SETREDRAW, TRUE, 0 );

	//キーワード数を表示する。
	DispKeywordCount();

	return;
}



/* ダイアログデータの取得 Keyword */
int CDlgConfigChildKeyword::GetData()
{
	return TRUE;
}

//キーワード数を表示する。
void CDlgConfigChildKeyword::DispKeywordCount()
{
	HWND hwndDlg = GetHwnd();

	HWND	hwndList;
	int		n;
	TCHAR szCount[ 256 ];

	hwndList = ::GetDlgItem( hwndDlg, IDC_LIST_KEYWORD );
	n = ListView_GetItemCount( hwndList );
	if( n < 0 ) n = 0;

	int		nAlloc;
	nAlloc = m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetAllocSize( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	nAlloc -= m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetKeyWordNum( m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.m_nCurrentKeyWordSetIdx );
	nAlloc += m_Common.m_sSpecialKeyword.m_CKeyWordSetMgr.GetFreeSize();
	
	auto_sprintf( szCount, LS(STR_PROPCOMKEYWORD_INFO), MAX_KEYWORDLEN, n, nAlloc );
	::SetWindowText( ::GetDlgItem( hwndDlg, IDC_STATIC_KEYWORD_COUNT ), szCount );
}


LPVOID CDlgConfigChildKeyword::GetHelpIdTable()
{
	return (LPVOID)p_helpids;
}
