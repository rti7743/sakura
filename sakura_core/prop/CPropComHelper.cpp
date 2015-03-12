/*!	@file
	@brief 共通設定ダイアログボックス、「支援」ページ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, genta, MIK, jepro, asa-o
	Copyright (C) 2002, YAZAKI, MIK, genta
	Copyright (C) 2003, Moca, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, ryoji
	Copyright (C) 2009, ryoji
	Copyright (C) 2012, Moca
	Copyright (C) 2013, Uchi
	Copyright (C) 2014, Moca

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
#include "prop/CDlgConfigChildSupport.h"
#include "prop/CDlgConfig.h"
#include "dlg/CDialog.h"
#include "util/shell.h"
#include "util/module.h"
#include "sakura_rc.h"
#include "sakura.hh"


//@@@ 2001.02.04 Start by MIK: Popup Help
static const DWORD p_helpids[] = {	//10600
	IDC_BUTTON_OPENHELP1,			HIDC_BUTTON_OPENHELP1,			//外部ヘルプファイル参照
	IDC_BUTTON_OPENEXTHTMLHELP,		HIDC_BUTTON_OPENEXTHTMLHELP,	//外部HTMLファイル参照
//	IDC_CHECK_USEHOKAN,				HIDC_CHECK_USEHOKAN,			//逐次入力補完
	IDC_CHECK_m_bHokanKey_RETURN,	HIDC_CHECK_m_bHokanKey_RETURN,	//候補決定キー（Enter）
	IDC_CHECK_m_bHokanKey_TAB,		HIDC_CHECK_m_bHokanKey_TAB,		//候補決定キー（Tab）
	IDC_CHECK_m_bHokanKey_RIGHT,	HIDC_CHECK_m_bHokanKey_RIGHT,	//候補決定キー（→）
//	IDC_CHECK_m_bHokanKey_SPACE,	HIDC_CHECK_m_bHokanKey_SPACE,	//候補決定キー（Space）
	IDC_CHECK_HTMLHELPISSINGLE,		HIDC_CHECK_HTMLHELPISSINGLE,	//ビューアの複数起動
	IDC_EDIT_EXTHELP1,				HIDC_EDIT_EXTHELP1,				//外部ヘルプファイル名
	IDC_EDIT_EXTHTMLHELP,			HIDC_EDIT_EXTHTMLHELP,			//外部HTMLヘルプファイル名
	//	2007.02.04 genta カーソル位置の単語の辞書検索は共通設定から外した
	//IDC_CHECK_CLICKKEYSEARCH,		HIDC_CHECK_CLICKKEYSEARCH,		//キャレット位置の単語を辞書検索	// 2006.03.24 fon
	IDC_BUTTON_KEYWORDHELPFONT,		HIDC_BUTTON_KEYWORDHELPFONT,	//キーワードヘルプのフォント
	IDC_EDIT_MIGEMO_DLL,			HIDC_EDIT_MIGEMO_DLL,			//Migemo DLLファイル名	// 2006.08.06 ryoji
	IDC_BUTTON_OPENMDLL,			HIDC_BUTTON_OPENMDLL,			//Migemo DLLファイル参照	// 2006.08.06 ryoji
	IDC_EDIT_MIGEMO_DICT,			HIDC_EDIT_MIGEMO_DICT,			//Migemo 辞書ファイル名	// 2006.08.06 ryoji
	IDC_BUTTON_OPENMDICT,			HIDC_BUTTON_OPENMDICT,			//Migemo 辞書ファイル参照	// 2006.08.06 ryoji
//	IDC_STATIC,						-1,
	0, 0
};
//@@@ 2001.02.04 End


HWND CDlgConfigChildSupport::DoModeless( HINSTANCE hInstance, HWND hwndParent, SDlgConfigArg* pDlgConfigArg, int nTypeIndex )
{
	m_nCurrentTypeIndex = nTypeIndex;
	m_pDlgConfigArg  = pDlgConfigArg;

	return CDialog::DoModeless( hInstance, hwndParent, IDD_PROP_HELPER, 0, SW_SHOW );
}


BOOL CDlgConfigChildSupport::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	BOOL result =  CDlgConfigChild::OnInitDialog( hwndDlg, wParam, lParam );

	/* ユーザーがエディット コントロールに入力できるテキストの長さを制限する */
	/* 外部ヘルプ１ */
	EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_EXTHELP1 ), _MAX_PATH - 1 );
	/* 外部HTMLヘルプ */
	EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_EXTHTMLHELP ), _MAX_PATH - 1 );

	return result;
}


BOOL CDlgConfigChildSupport::OnBnClicked( int wID )
{
	HWND hwndDlg = GetHwnd();
	
	/* ダイアログデータの取得 Helper */
	GetData();
	switch( wID ){
	case IDC_BUTTON_OPENHELP1:	/* 外部ヘルプ１の「参照...」ボタン */
		{
			// 2003.06.23 Moca 相対パスは実行ファイルからのパス
			// 2007.05.21 ryoji 相対パスは設定ファイルからのパスを優先
			CDialog::SelectFile(hwndDlg, GetDlgItem(hwndDlg, IDC_EDIT_EXTHELP1), _T("*.hlp;*.chm;*.col"), true, false);
		}
		return TRUE;
	case IDC_BUTTON_OPENEXTHTMLHELP:	/* 外部HTMLヘルプの「参照...」ボタン */
		{
			// 2003.06.23 Moca 相対パスは実行ファイルからのパス
			// 2007.05.21 ryoji 相対パスは設定ファイルからのパスを優先
			CDialog::SelectFile(hwndDlg, GetDlgItem(hwndDlg, IDC_EDIT_EXTHTMLHELP), _T("*.chm;*.col"), true, false);
		}
		return TRUE;
	// ai 02/05/21 Add S
	case IDC_BUTTON_KEYWORDHELPFONT:	/* キーワードヘルプの「フォント」ボタン */
		{
			LOGFONT   lf = m_Common.m_sHelper.m_lf;
			INT nPointSize = m_Common.m_sHelper.m_nPointSize;

			if( MySelectFont( &lf, &nPointSize, hwndDlg, false) ){
				m_Common.m_sHelper.m_lf = lf;
				m_Common.m_sHelper.m_nPointSize = nPointSize;	// 2009.10.01 ryoji
				// キーワードヘルプ フォント表示	// 2013/4/24 Uchi
				HFONT hFont = SetFontLabel( IDC_STATIC_KEYWORDHELPFONT, m_Common.m_sHelper.m_lf, m_Common.m_sHelper.m_nPointSize);
				if(m_hKeywordHelpFont != NULL){
					::DeleteObject( m_hKeywordHelpFont );
				}
				m_hKeywordHelpFont = hFont;
			}
		}
		return TRUE;
	// ai 02/05/21 Add E
	case IDC_BUTTON_OPENMDLL:	/* MIGEMODLL場所指定「参照...」ボタン */
		{
			// 2003.06.23 Moca 相対パスは実行ファイルからのパス
			// 2007.05.21 ryoji 相対パスは設定ファイルからのパスを優先
			CDialog::SelectFile(hwndDlg, GetDlgItem(hwndDlg, IDC_EDIT_MIGEMO_DLL), _T("*.dll"), true, false);
		}
		return TRUE;
	case IDC_BUTTON_OPENMDICT:	/* MigemoDict場所指定「参照...」ボタン */
		{
			TCHAR	szPath[_MAX_PATH];
			/* 検索フォルダ */
			// 2007.05.27 ryoji 相対パスは設定ファイルからのパスを優先
			if( _IS_REL_PATH( m_Common.m_sHelper.m_szMigemoDict ) ){
				GetInidirOrExedir( szPath, m_Common.m_sHelper.m_szMigemoDict, TRUE );
			}else{
				_tcscpy( szPath, m_Common.m_sHelper.m_szMigemoDict );
			}
			if( SelectDir( hwndDlg, LS(STR_PROPCOMHELP_MIGEMODIR), szPath, szPath ) ){
				_tcscpy( m_Common.m_sHelper.m_szMigemoDict, GetRelPath(szPath) ); // 2015.03.03 可能なら相対パスにする
				::DlgItem_SetText( hwndDlg, IDC_EDIT_MIGEMO_DICT, m_Common.m_sHelper.m_szMigemoDict );
			}
		}
		return TRUE;
	}
	return FALSE;
}


BOOL CDlgConfigChildSupport::OnDestroy()
{
	// キーワードヘルプ フォント破棄	// 2013/4/24 Uchi
	if (m_hKeywordHelpFont != NULL) {
		::DeleteObject( m_hKeywordHelpFont );
		m_hKeywordHelpFont = NULL;
	}
	return CDlgConfigChild::OnDestroy();
}


/* ダイアログデータの設定 Helper */
void CDlgConfigChildSupport::SetData()
{
	HWND hwndDlg = GetHwnd();

	/* 補完候補決定キー */
	::CheckDlgButton( hwndDlg, IDC_CHECK_m_bHokanKey_RETURN, m_Common.m_sHelper.m_bHokanKey_RETURN );	//VK_RETURN 補完決定キーが有効/無効
	::CheckDlgButton( hwndDlg, IDC_CHECK_m_bHokanKey_TAB, m_Common.m_sHelper.m_bHokanKey_TAB );		//VK_TAB    補完決定キーが有効/無効
	::CheckDlgButton( hwndDlg, IDC_CHECK_m_bHokanKey_RIGHT, m_Common.m_sHelper.m_bHokanKey_RIGHT );	//VK_RIGHT  補完決定キーが有効/無効

	/* 外部ヘルプ１ */
	::DlgItem_SetText( hwndDlg, IDC_EDIT_EXTHELP1, m_Common.m_sHelper.m_szExtHelp );

	/* 外部HTMLヘルプ */
	::DlgItem_SetText( hwndDlg, IDC_EDIT_EXTHTMLHELP, m_Common.m_sHelper.m_szExtHtmlHelp );

	/* HtmlHelpビューアはひとつ */
	::CheckDlgButton( hwndDlg, IDC_CHECK_HTMLHELPISSINGLE, m_Common.m_sHelper.m_bHtmlHelpIsSingle ? BST_CHECKED : BST_UNCHECKED );

	// キーワードヘルプ フォント	// 2013/4/24 Uchi
	m_hKeywordHelpFont = SetFontLabel( IDC_STATIC_KEYWORDHELPFONT, m_Common.m_sHelper.m_lf, m_Common.m_sHelper.m_nPointSize);

	//migemo dict
	::DlgItem_SetText( hwndDlg, IDC_EDIT_MIGEMO_DLL, m_Common.m_sHelper.m_szMigemoDll);
	::DlgItem_SetText( hwndDlg, IDC_EDIT_MIGEMO_DICT, m_Common.m_sHelper.m_szMigemoDict);
}


/* ダイアログデータの取得 Helper */
int CDlgConfigChildSupport::GetData()
{
	HWND hwndDlg = GetHwnd();

	/* 補完候補決定キー */
	m_Common.m_sHelper.m_bHokanKey_RETURN = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_m_bHokanKey_RETURN );//VK_RETURN 補完決定キーが有効/無効
	m_Common.m_sHelper.m_bHokanKey_TAB = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_m_bHokanKey_TAB );		//VK_TAB    補完決定キーが有効/無効
	m_Common.m_sHelper.m_bHokanKey_RIGHT = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_m_bHokanKey_RIGHT );	//VK_RIGHT  補完決定キーが有効/無効

	/* 外部ヘルプ１ */
	::DlgItem_GetText( hwndDlg, IDC_EDIT_EXTHELP1, m_Common.m_sHelper.m_szExtHelp, _countof( m_Common.m_sHelper.m_szExtHelp ));

	/* 外部HTMLヘルプ */
	::DlgItem_GetText( hwndDlg, IDC_EDIT_EXTHTMLHELP, m_Common.m_sHelper.m_szExtHtmlHelp, _countof( m_Common.m_sHelper.m_szExtHtmlHelp ));

	/* HtmlHelpビューアはひとつ */
	m_Common.m_sHelper.m_bHtmlHelpIsSingle = ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_HTMLHELPISSINGLE ) != 0;

	//migemo dict
	::DlgItem_GetText( hwndDlg, IDC_EDIT_MIGEMO_DLL, m_Common.m_sHelper.m_szMigemoDll, _countof( m_Common.m_sHelper.m_szMigemoDll ));
	::DlgItem_GetText( hwndDlg, IDC_EDIT_MIGEMO_DICT, m_Common.m_sHelper.m_szMigemoDict, _countof( m_Common.m_sHelper.m_szMigemoDict ));

	return TRUE;
}


LPVOID CDlgConfigChildSupport::GetHelpIdTable()
{
	return (LPVOID)p_helpids;
}
