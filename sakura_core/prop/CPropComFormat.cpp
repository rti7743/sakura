/*!	@file
	共通設定ダイアログボックス、「書式」ページ

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, MIK, genta
	Copyright (C) 2002, YAZAKI, MIK, aroka
	Copyright (C) 2003, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2014, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "prop/CDlgConfigChildFormat.h"
#include "prop/CDlgConfig.h"
#include "util/shell.h"
#include "env/DLLSHAREDATA.h" // CFormatManager.hより前に必要
#include "env/CFormatManager.h"
#include "sakura_rc.h"
#include "sakura.hh"


//@@@ 2001.02.04 Start by MIK: Popup Help
static const DWORD p_helpids[] = {	//10400
	IDC_EDIT_DFORM,						HIDC_EDIT_DFORM,		//日付書式
	IDC_EDIT_TFORM,						HIDC_EDIT_TFORM,		//時刻書式
	IDC_EDIT_DFORM_EX,					HIDC_EDIT_DFORM_EX,		//日付書式（表示例）
	IDC_EDIT_TFORM_EX,					HIDC_EDIT_TFORM_EX,		//時刻書式（表示例）
	IDC_EDIT_MIDASHIKIGOU,				HIDC_EDIT_MIDASHIKIGOU,	//見出し記号
	IDC_EDIT_INYOUKIGOU,				HIDC_EDIT_INYOUKIGOU,	//引用符
	IDC_RADIO_DFORM_0,					HIDC_RADIO_DFORM_0,		//日付書式（標準）
	IDC_RADIO_DFORM_1,					HIDC_RADIO_DFORM_1,		//日付書式（カスタム）
	IDC_RADIO_TFORM_0,					HIDC_RADIO_TFORM_0,		//時刻書式（標準）
	IDC_RADIO_TFORM_1,					HIDC_RADIO_TFORM_1,		//時刻書式（カスタム）
//	IDC_STATIC,							-1,
	0, 0
};
//@@@ 2001.02.04 End

//@@@ 2002.01.12 add start
static const char *p_date_form[] = {
	"yyyy'年'M'月'd'日'",
	"yyyy'年'M'月'd'日('dddd')'",
	"yyyy'年'MM'月'dd'日'",
	"yyyy'年'M'月'd'日' dddd",
	"yyyy'年'MM'月'dd'日' dddd",
	"yyyy/MM/dd",
	"yy/MM/dd",
	"yy/M/d",
	"yyyy/M/d",
	"yy/MM/dd' ('ddd')'",
	"yy/M/d' ('ddd')'",
	"yyyy/MM/dd' ('ddd')'",
	"yyyy/M/d' ('ddd')'",
	"yyyy/M/d' ('ddd')'",
	NULL
};

static const char *p_time_form[] = {
	"hh:mm:ss",
	"tthh'時'mm'分'ss'秒'",
	"H:mm:ss",
	"HH:mm:ss",
	"tt h:mm:ss",
	"tt hh:mm:ss",
	NULL
};
//@@@ 2002.01.12 add end


HWND CDlgConfigChildFormat::DoModeless( HINSTANCE hInstance, HWND hwndParent, SDlgConfigArg* pDlgConfigArg, int nTypeIndex )
{
	m_nCurrentTypeIndex = nTypeIndex;
	m_pDlgConfigArg  = pDlgConfigArg;
	m_bInit = false;

	return CDialog::DoModeless( hInstance, hwndParent, IDD_PROP_FORMAT, 0, SW_SHOW );
}


BOOL CDlgConfigChildFormat::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	BOOL result =  CDlgConfigChild::OnInitDialog( hwndDlg, wParam, lParam );

	m_bInit = true;

	ChangeDateExample();
	ChangeTimeExample();

	/* 見出し記号 */
	EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_MIDASHIKIGOU ), _countof(m_Common.m_sFormat.m_szMidashiKigou) - 1 );

	/* 引用符 */
	EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_INYOUKIGOU ), _countof(m_Common.m_sFormat.m_szInyouKigou) - 1 );

	/* 日付書式 */
	EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_DFORM ), _countof(m_Common.m_sFormat.m_szDateFormat) - 1 );

	/* 時刻書式 */
	EditCtl_LimitText( ::GetDlgItem( hwndDlg, IDC_EDIT_TFORM ), _countof(m_Common.m_sFormat.m_szTimeFormat) - 1 );

	return result;
}


void CDlgConfigChildFormat::ChangeDateExample()
{
	HWND hwndDlg = GetHwnd();
	/* ダイアログデータの取得 Format */
	GetData();

	/* 日付をフォーマット */
	TCHAR szText[1024];
	SYSTEMTIME systime;
	::GetLocalTime( &systime );
	CFormatManager().MyGetDateFormat( systime, szText, _countof( szText ) - 1, m_Common.m_sFormat.m_nDateFormatType, m_Common.m_sFormat.m_szDateFormat );
	::DlgItem_SetText( hwndDlg, IDC_EDIT_DFORM_EX, szText );
	return;
}


void CDlgConfigChildFormat::ChangeTimeExample()
{
	HWND hwndDlg = GetHwnd();
	/* ダイアログデータの取得 Format */
	GetData();

	/* 時刻をフォーマット */
	TCHAR szText[1024];
	SYSTEMTIME systime;
	::GetLocalTime( &systime );
	CFormatManager().MyGetTimeFormat( systime, szText, _countof( szText ) - 1, m_Common.m_sFormat.m_nTimeFormatType, m_Common.m_sFormat.m_szTimeFormat );
	::DlgItem_SetText( hwndDlg, IDC_EDIT_TFORM_EX, szText );
	return;
}


BOOL CDlgConfigChildFormat::OnEnChange( HWND hwndCtl, int wID )
{
	if( !m_bInit ){
		return FALSE;
	}
	if( IDC_EDIT_DFORM == wID ){
		ChangeDateExample();
		return FALSE;
	}
	if( IDC_EDIT_TFORM == wID  ){
		ChangeTimeExample();
		return FALSE;
	}
	return FALSE;
}


BOOL CDlgConfigChildFormat::OnBnClicked( int wID )
{
	if( !m_bInit ){
		return FALSE;
	}
	/* ボタン／チェックボックスがクリックされた */
	switch( wID ){
	case IDC_RADIO_DFORM_0:
	case IDC_RADIO_DFORM_1:
		ChangeDateExample();
	//	From Here Sept. 10, 2000 JEPRO
	//	日付書式 0=標準 1=カスタム
	//	日付書式をカスタムにするときだけ書式指定文字入力をEnableに設定
		EnableFormatPropInput();
	//	To Here Sept. 10, 2000
		return 0;
	case IDC_RADIO_TFORM_0:
	case IDC_RADIO_TFORM_1:
		ChangeTimeExample();
	//	From Here Sept. 10, 2000 JEPRO
	//	時刻書式 0=標準 1=カスタム
	//	時刻書式をカスタムにするときだけ書式指定文字入力をEnableに設定
		EnableFormatPropInput();
	//	To Here Sept. 10, 2000
		return 0;
	}
	return FALSE;
}


/* ダイアログデータの設定 Format */
void CDlgConfigChildFormat::SetData()
{
	HWND hwndDlg = GetHwnd();

	/* 見出し記号 */
	::DlgItem_SetText( hwndDlg, IDC_EDIT_MIDASHIKIGOU, m_Common.m_sFormat.m_szMidashiKigou );

	/* 引用符 */
	::DlgItem_SetText( hwndDlg, IDC_EDIT_INYOUKIGOU, m_Common.m_sFormat.m_szInyouKigou );


	//日付書式のタイプ
	if( 0 == m_Common.m_sFormat.m_nDateFormatType ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_DFORM_0, BST_CHECKED );
	}else{
		::CheckDlgButton( hwndDlg, IDC_RADIO_DFORM_1, BST_CHECKED );
	}
	//日付書式
	::DlgItem_SetText( hwndDlg, IDC_EDIT_DFORM, m_Common.m_sFormat.m_szDateFormat );

	//時刻書式のタイプ
	if( 0 == m_Common.m_sFormat.m_nTimeFormatType ){
		::CheckDlgButton( hwndDlg, IDC_RADIO_TFORM_0, BST_CHECKED );
	}else{
		::CheckDlgButton( hwndDlg, IDC_RADIO_TFORM_1, BST_CHECKED );
	}
	//時刻書式
	::DlgItem_SetText( hwndDlg, IDC_EDIT_TFORM, m_Common.m_sFormat.m_szTimeFormat );

	//	From Here Sept. 10, 2000 JEPRO
	//	日付/時刻書式 0=標準 1=カスタム
	//	日付/時刻書式をカスタムにするときだけ書式指定文字入力をEnableに設定
	EnableFormatPropInput();
	//	To Here Sept. 10, 2000

	return;
}




/* ダイアログデータの取得 Format */
int CDlgConfigChildFormat::GetData()
{
	HWND hwndDlg = GetHwnd();

	/* 見出し記号 */
	::DlgItem_GetText( hwndDlg, IDC_EDIT_MIDASHIKIGOU, m_Common.m_sFormat.m_szMidashiKigou, _countof(m_Common.m_sFormat.m_szMidashiKigou) );

//	/* 外部ヘルプ１ */
//	::DlgItem_GetText( hwndDlg, IDC_EDIT_EXTHELP1, m_Common.m_sFormat.m_szExtHelp1, MAX_PATH - 1 );
//
//	/* 外部HTMLヘルプ */
//	::DlgItem_GetText( hwndDlg, IDC_EDIT_EXTHTMLHELP, m_Common.m_sFormat.m_szExtHtmlHelp, MAX_PATH - 1 );

	/* 引用符 */
	::DlgItem_GetText( hwndDlg, IDC_EDIT_INYOUKIGOU, m_Common.m_sFormat.m_szInyouKigou, _countof(m_Common.m_sFormat.m_szInyouKigou) );


	//日付書式のタイプ
	if( BST_CHECKED == ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_DFORM_0 ) ){
		m_Common.m_sFormat.m_nDateFormatType = 0;
	}else{
		m_Common.m_sFormat.m_nDateFormatType = 1;
	}
	//日付書式
	::DlgItem_GetText( hwndDlg, IDC_EDIT_DFORM, m_Common.m_sFormat.m_szDateFormat, _countof( m_Common.m_sFormat.m_szDateFormat ));

	//時刻書式のタイプ
	if( BST_CHECKED == ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_TFORM_0 ) ){
		m_Common.m_sFormat.m_nTimeFormatType = 0;
	}else{
		m_Common.m_sFormat.m_nTimeFormatType = 1;
	}

	//時刻書式
	::DlgItem_GetText( hwndDlg, IDC_EDIT_TFORM, m_Common.m_sFormat.m_szTimeFormat, _countof( m_Common.m_sFormat.m_szTimeFormat ));

	return TRUE;
}





//	From Here Sept. 10, 2000 JEPRO
//	チェック状態に応じてダイアログボックス要素のEnable/Disableを
//	適切に設定する
void CDlgConfigChildFormat::EnableFormatPropInput()
{
	HWND hwndDlg = GetHwnd();
	//	日付書式をカスタムにするかどうか
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_DFORM_1 ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_DFORM ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_DFORM ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_DFORM ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_DFORM ), FALSE );
	}

	//	時刻書式をカスタムにするかどうか
	if( ::IsDlgButtonChecked( hwndDlg, IDC_RADIO_TFORM_1 ) ){
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_TFORM ), TRUE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_TFORM ), TRUE );
	}else{
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_LABEL_TFORM ), FALSE );
		::EnableWindow( ::GetDlgItem( hwndDlg, IDC_EDIT_TFORM ), FALSE );
	}
}
//	To Here Sept. 10, 2000


LPVOID CDlgConfigChildFormat::GetHelpIdTable()
{
	return (LPVOID)p_helpids;
}
