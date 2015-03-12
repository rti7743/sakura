/*!	@file
	@brief 共通設定ダイアログボックス、「ステータスバー」ページ

	@author Uchi
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2000, jepro
	Copyright (C) 2001, MIK, jepro, genta
	Copyright (C) 2002, YAZAKI, MIK, aroka
	Copyright (C) 2003, KEITA
	Copyright (C) 2006, ryoji
	Copyright (C) 2007, genta
	Copyright (C) 2007, Uchi
	Copyright (C) 2014, Moca

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "prop/CDlgConfigChildStatusbar.h"
#include "prop/CDlgConfig.h"
#include "util/shell.h"
#include "sakura_rc.h"
#include "sakura.hh"


static const DWORD p_helpids[] = {
	IDC_CHECK_DISP_UNICODE_IN_SJIS,		HIDC_CHECK_DISP_UNICODE_IN_SJIS,		// SJISで文字コード値をUnicodeで表示する
	IDC_CHECK_DISP_UNICODE_IN_JIS,		HIDC_CHECK_DISP_UNICODE_IN_JIS,			// JISで文字コード値をUnicodeで表示する
	IDC_CHECK_DISP_UNICODE_IN_EUC,		HIDC_CHECK_DISP_UNICODE_IN_EUC,			// EUCで文字コード値をUnicodeで表示する
	IDC_CHECK_DISP_UTF8_CODEPOINT,		HIDC_CHECK_DISP_UTF8_CODEPOINT,			// UTF-8をコードポイントで表示する
	IDC_CHECK_DISP_SP_CODEPOINT,		HIDC_CHECK_DISP_SP_CODEPOINT,			// サロゲートペアをコードポイントで表示する
	IDC_CHECK_DISP_SELCOUNT_BY_BYTE,	HIDC_CHECK_DISP_SELCOUNT_BY_BYTE,		// 選択文字数を文字単位ではなくバイト単位で表示する
	0, 0
};


HWND CDlgConfigChildStatusbar::DoModeless( HINSTANCE hInstance, HWND hwndParent, SDlgConfigArg* pDlgConfigArg, int nTypeIndex )
{
	m_nCurrentTypeIndex = nTypeIndex;
	m_pDlgConfigArg = pDlgConfigArg;

	return CDialog::DoModeless( hInstance, hwndParent, IDD_PROP_STATUSBAR, 0, SW_SHOW );
}


BOOL CDlgConfigChildStatusbar::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	BOOL result =  CDlgConfigChild::OnInitDialog( hwndDlg, wParam, lParam );

	return result;
}


/* ダイアログデータの設定 */
void CDlgConfigChildStatusbar::SetData()
{
	HWND hwndDlg = GetHwnd();

	// 示文字コードの指定
	// SJISで文字コード値をUnicodeで出力する
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_SJIS, m_Common.m_sStatusbar.m_bDispUniInSjis );
	// JISで文字コード値をUnicodeで出力する
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_JIS,  m_Common.m_sStatusbar.m_bDispUniInJis );
	// EUCで文字コード値をUnicodeで出力する
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_EUC,  m_Common.m_sStatusbar.m_bDispUniInEuc );
	// UTF-8で表示をバイトコードで行う
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_UTF8_CODEPOINT,  m_Common.m_sStatusbar.m_bDispUtf8Codepoint );
	// サロゲートペアをコードポイントで表示
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_SP_CODEPOINT,    m_Common.m_sStatusbar.m_bDispSPCodepoint );
	// 選択文字数を文字単位ではなくバイト単位で表示する
	::CheckDlgButton( hwndDlg, IDC_CHECK_DISP_SELCOUNT_BY_BYTE,m_Common.m_sStatusbar.m_bDispSelCountByByte );
	return;
}


/* ダイアログデータの取得 */
int CDlgConfigChildStatusbar::GetData()
{
	HWND hwndDlg = GetHwnd();

	// 示文字コードの指定
	// SJISで文字コード値をUnicodeで出力する
	m_Common.m_sStatusbar.m_bDispUniInSjis		= ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_SJIS );
	// JISで文字コード値をUnicodeで出力する
	m_Common.m_sStatusbar.m_bDispUniInJis		= ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_JIS );
	// EUCで文字コード値をUnicodeで出力する
	m_Common.m_sStatusbar.m_bDispUniInEuc		= ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_UNICODE_IN_EUC );
	// UTF-8で表示をバイトコードで行う
	m_Common.m_sStatusbar.m_bDispUtf8Codepoint	= ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_UTF8_CODEPOINT );
	// サロゲートペアをコードポイントで表示
	m_Common.m_sStatusbar.m_bDispSPCodepoint	= ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_SP_CODEPOINT );
	// 選択文字数を文字単位ではなくバイト単位で表示する
	m_Common.m_sStatusbar.m_bDispSelCountByByte	= ::IsDlgButtonChecked( hwndDlg, IDC_CHECK_DISP_SELCOUNT_BY_BYTE );

	return TRUE;
}


LPVOID CDlgConfigChildStatusbar::GetHelpIdTable()
{
	return (LPVOID)p_helpids;
}
