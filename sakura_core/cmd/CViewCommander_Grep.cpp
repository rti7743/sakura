/*!	@file
@brief CViewCommanderクラスのコマンド(Grep)関数群

*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2003, MIK
	Copyright (C) 2005, genta
	Copyright (C) 2006, ryoji

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/
#include "StdAfx.h"
#include "CViewCommander.h"
#include "CViewCommander_inline.h"

#include "_main/CControlTray.h"
#include "CEditApp.h"
#include "CGrepAgent.h"
#include "plugin/CPlugin.h"
#include "plugin/CJackManager.h"

// static
void CViewCommander::SetGrepParams(CMacro& params, const CEditView* pcEditView, bool bReplace)
{
	CDlgGrepReplace* pcDlgGrepRep;
	CDlgGrep* pcDlgGrep;
	if( bReplace ){
		pcDlgGrepRep = NULL;
		pcDlgGrep = &pcEditView->m_pcEditWnd->m_cDlgGrep;
	}else{
		pcDlgGrepRep = &pcEditView->m_pcEditWnd->m_cDlgGrepReplace;
		pcDlgGrep = pcDlgGrepRep;
	}
	params.AddStringParam( pcDlgGrep->m_strText.c_str() );
	if( bReplace ){
		params.AddStringParam( pcDlgGrepRep->m_strText2.c_str() );
	}
	params.AddStringParam( GetDllShareData().m_sSearchKeywords.m_aGrepFiles[0] );
	params.AddStringParam( GetDllShareData().m_sSearchKeywords.m_aGrepFolders[0] );

	LPARAM lFlag = 0x00;
	const CommonSetting_Search& search = GetDllShareData().m_Common.m_sSearch;
	lFlag |= search.m_bGrepSubFolder					? 0x01 : 0x00;
	//			この編集中のテキストから検索する(0x02.未実装)
	lFlag |= pcDlgGrep->m_sSearchOption.bLoHiCase		? 0x04 : 0x00;
	lFlag |= pcDlgGrep->m_sSearchOption.bRegularExp		? 0x08 : 0x00;
	lFlag |= (search.m_nGrepCharSet == CODE_AUTODETECT) ? 0x10 : 0x00;	//	2002/09/21 Moca 下位互換性のための処理
	lFlag |= search.m_nGrepOutputLineType == 1			? 0x20 : 0x00;
	lFlag |= search.m_nGrepOutputLineType == 2			? 0x400000 : 0x00;	// 2014.09.23 否ヒット行
	lFlag |= (search.m_nGrepOutputStyle == 2)			? 0x40 : 0x00;
	lFlag |= (search.m_nGrepOutputStyle == 3)			? 0x80 : 0x00;
	ECodeType code = search.m_nGrepCharSet;
	if( IsValidCodeType(code) || CODE_AUTODETECT == code ){
		lFlag |= code << 8;
	}
	lFlag |= search.m_sSearchOption.bWordOnly			? 0x10000 : 0x00;
	lFlag |= search.m_bGrepOutputFileOnly				? 0x20000 : 0x00;
	lFlag |= search.m_bGrepOutputBaseFolder				? 0x40000 : 0x00;
	lFlag |= search.m_bGrepSeparateFolder				? 0x80000 : 0x00;
	if( bReplace ){
		lFlag |= pcDlgGrepRep->m_bPaste					? 0x100000 : 0x00;
		lFlag |= search.m_bGrepBackup					? 0x200000 : 0x00;
	}
	params.AddIntParam( lFlag );
	params.AddIntParam( code );
}

/*! GREPダイアログの表示

	@date 2005.01.10 genta CEditView_Commandより移動
	@author Yazaki
*/
void CViewCommander::Command_GREP_DIALOG( EFunctionFlags flags )
{
	CNativeW	cmemCurText;
	// 2014.07.01 複数Grepウィンドウを使い分けている場合などに影響しないように、未設定のときだけHistoryを見る
	bool bGetHistory = GetEditWindow()->m_cDlgGrep.m_bSetText == false;

	/* 現在カーソル位置単語または選択範囲より検索等のキーを取得 */
	bool bSet = m_pCommanderView->GetCurrentTextForSearchDlg( cmemCurText, bGetHistory );	// 2006.08.23 ryoji ダイアログ専用関数に変更

	if( bSet ){
		GetEditWindow()->m_cDlgGrep.m_strText = cmemCurText.GetStringPtr();
		GetEditWindow()->m_cDlgGrep.m_bSetText = true;
	}

	/* Grepダイアログの表示 */
	int nRet = GetEditWindow()->m_cDlgGrep.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), GetDocument()->m_cDocFile.GetFilePath() );
//	MYTRACE( _T("nRet=%d\n"), nRet );
	if( !nRet ){
		if( GetSaveResultParam() ){
			GetMacroResultVal().SetIntParam( nRet );
		}
		return;
	}
	CMacro params(F_0);
	if( GetSaveResultParam() ){
		// 実行に時間がかかるので先に取得
		const CEditView* pcEditView = m_pCommanderView;
		SetGrepParams(params, m_pCommanderView, false);
	}
	
	HandleCommand(static_cast<EFunctionCode>(F_GREP | flags), true, 0, 0, 0, 0);	//	GREPコマンドの発行
	if( GetSaveResultParam() ){
		GetMacroResultParam().AddParamCopyFromCMacro( &params );
		GetMacroResultVal().SetIntParam( nRet );
	}
}

/*! GREP実行

	@date 2005.01.10 genta CEditView_Commandより移動
*/
void CViewCommander::Command_GREP( void )
{
	CNativeW		cmWork1;
	CNativeT		cmWork2;
	CNativeT		cmWork3;
	CNativeW		cmWork4;

	cmWork1.SetString( GetEditWindow()->m_cDlgGrep.m_strText.c_str() );
	cmWork2.SetString( GetEditWindow()->m_cDlgGrep.m_szFile );
	cmWork3.SetString( GetEditWindow()->m_cDlgGrep.m_szFolder );

	/*	今のEditViewにGrep結果を表示する。
		Grepモードのとき、または未編集で無題かつアウトプットでない場合。
		自ウィンドウがGrep実行中も、(異常終了するので)別ウィンドウにする
	*/
	if( (  CEditApp::getInstance()->m_pcGrepAgent->m_bGrepMode &&
		  !CEditApp::getInstance()->m_pcGrepAgent->m_bGrepRunning ) ||
		( !GetDocument()->m_cDocEditor.IsModified() &&
		  !GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() &&		/* 現在編集中のファイルのパス */
		  !CAppMode::getInstance()->IsDebugMode()
		)
	){
		// 2011.01.23 Grepタイプ別適用
		if( !GetDocument()->m_cDocEditor.IsModified() && GetDocument()->m_cDocLineMgr.GetLineCount() == 0 ){
			CTypeConfig cTypeGrep = CDocTypeManager().GetDocumentTypeOfExt( _T("grepout") );
			const STypeConfigMini* pConfig;
			CDocTypeManager().GetTypeConfigMini( cTypeGrep, &pConfig );
			GetDocument()->m_cDocType.SetDocumentTypeIdx( pConfig->m_id );
			GetDocument()->m_cDocType.LockDocumentType();
			GetDocument()->OnChangeType();
		}
		
		CEditApp::getInstance()->m_pcGrepAgent->DoGrep(
			m_pCommanderView,
			false,
			&cmWork1,
			&cmWork4,
			&cmWork2,
			&cmWork3,
			false,
			GetEditWindow()->m_cDlgGrep.m_bSubFolder,
			false,
			true, // Header
			GetEditWindow()->m_cDlgGrep.m_sSearchOption,
			GetEditWindow()->m_cDlgGrep.m_nGrepCharSet,
			GetEditWindow()->m_cDlgGrep.m_nGrepOutputLineType,
			GetEditWindow()->m_cDlgGrep.m_nGrepOutputStyle,
			GetEditWindow()->m_cDlgGrep.m_bGrepOutputFileOnly,
			GetEditWindow()->m_cDlgGrep.m_bGrepOutputBaseFolder,
			GetEditWindow()->m_cDlgGrep.m_bGrepSeparateFolder,
			false,
			false
		);

		//プラグイン：DocumentOpenイベント実行
		CPlug::Array plugs;
		CWSHIfObj::List params;
		CJackManager::getInstance()->GetUsablePlug( PP_DOCUMENT_OPEN, 0, &plugs );
		for( CPlug::ArrayIter it = plugs.begin(); it != plugs.end(); it++ ){
			(*it)->Invoke(&GetEditWindow()->GetActiveView(), params);
		}
	}
	else{
		// 編集ウィンドウの上限チェック
		if( GetDllShareData().m_sNodes.m_nEditArrNum >= MAX_EDITWINDOWS ){	//最大値修正	//@@@ 2003.05.31 MIK
			OkMessage( m_pCommanderView->GetHwnd(), LS(STR_MAXWINDOW), MAX_EDITWINDOWS );
			return;
		}

		/*======= Grepの実行 =============*/
		/* Grep結果ウィンドウの表示 */
		CControlTray::DoGrepCreateWindow(G_AppInstance(), m_pCommanderView->GetHwnd(), GetEditWindow()->m_cDlgGrep);
	}
	return;
}

/*! GREP置換ダイアログの表示
*/
void CViewCommander::Command_GREP_REPLACE_DLG( EFunctionFlags flags )
{
	CNativeW	cmemCurText;
	CDlgGrepReplace& cDlgGrepRep = GetEditWindow()->m_cDlgGrepReplace;

	// 複数Grepウィンドウを使い分けている場合などに影響しないように、未設定のときだけHistoryを見る
	bool bGetHistory = cDlgGrepRep.m_bSetText == false;

	m_pCommanderView->GetCurrentTextForSearchDlg( cmemCurText, bGetHistory );

	if( 0 < cmemCurText.GetStringLength() ){
		cDlgGrepRep.m_strText = cmemCurText.GetStringPtr();
		cDlgGrepRep.m_bSetText = true;
	}
	if( 0 < GetDllShareData().m_sSearchKeywords.m_aReplaceKeys.size() ){
		if( cDlgGrepRep.m_nReplaceKeySequence < GetDllShareData().m_Common.m_sSearch.m_nReplaceKeySequence ){
			cDlgGrepRep.m_strText2 = GetDllShareData().m_sSearchKeywords.m_aReplaceKeys[0];
		}
	}

	int nRet = cDlgGrepRep.DoModal( G_AppInstance(), m_pCommanderView->GetHwnd(), GetDocument()->m_cDocFile.GetFilePath(), (LPARAM)m_pCommanderView );
	if( !nRet ){
		if( GetSaveResultParam() ){
			GetMacroResultVal().SetIntParam( nRet );
		}
		return;
	}
	CMacro params(F_0);
	if( GetSaveResultParam() ){
		// 実行に時間がかかるので先に取得
		const CEditView* pcEditView = m_pCommanderView;
		SetGrepParams(params, m_pCommanderView, true);
	}
	
	HandleCommand(static_cast<EFunctionCode>(F_GREP_REPLACE | flags), true, 0, 0, 0, 0);	//	GREP置換コマンドの発行
	if( GetSaveResultParam() ){
		GetMacroResultParam().AddParamCopyFromCMacro( &params );
		GetMacroResultVal().SetIntParam( nRet );
	}
}

/*! GREP置換実行
*/
void CViewCommander::Command_GREP_REPLACE( void )
{
	CNativeW		cmWork1;
	CNativeT		cmWork2;
	CNativeT		cmWork3;
	CNativeW		cmWork4;

	CDlgGrepReplace& cDlgGrepRep = GetEditWindow()->m_cDlgGrepReplace;
	cmWork1.SetString( cDlgGrepRep.m_strText.c_str() );
	cmWork2.SetString( cDlgGrepRep.m_szFile );
	cmWork3.SetString( cDlgGrepRep.m_szFolder );
	cmWork4.SetString( cDlgGrepRep.m_strText2.c_str() );

	/*	今のEditViewにGrep結果を表示する。
		Grepモードのとき、または未編集で無題かつアウトプットでない場合。
		自ウィンドウがGrep実行中も、(異常終了するので)別ウィンドウにする
	*/
	if( (  CEditApp::getInstance()->m_pcGrepAgent->m_bGrepMode &&
		  !CEditApp::getInstance()->m_pcGrepAgent->m_bGrepRunning ) ||
		( !GetDocument()->m_cDocEditor.IsModified() &&
		  !GetDocument()->m_cDocFile.GetFilePathClass().IsValidPath() &&		/* 現在編集中のファイルのパス */
		  !CAppMode::getInstance()->IsDebugMode()
		)
	){
		CEditApp::getInstance()->m_pcGrepAgent->DoGrep(
			m_pCommanderView,
			true,
			&cmWork1,
			&cmWork4,
			&cmWork2,
			&cmWork3,
			false,
			cDlgGrepRep.m_bSubFolder,
			false, // Stdout
			true, // Header
			cDlgGrepRep.m_sSearchOption,
			cDlgGrepRep.m_nGrepCharSet,
			cDlgGrepRep.m_nGrepOutputLineType,
			cDlgGrepRep.m_nGrepOutputStyle,
			cDlgGrepRep.m_bGrepOutputFileOnly,
			cDlgGrepRep.m_bGrepOutputBaseFolder,
			cDlgGrepRep.m_bGrepSeparateFolder,
			cDlgGrepRep.m_bPaste,
			cDlgGrepRep.m_bBackup
		);
	}
	else{
		// 編集ウィンドウの上限チェック
		if( GetDllShareData().m_sNodes.m_nEditArrNum >= MAX_EDITWINDOWS ){	//最大値修正	//@@@ 2003.05.31 MIK
			OkMessage( m_pCommanderView->GetHwnd(), _T("編集ウィンドウ数の上限は%dです。\nこれ以上は同時に開けません。"), MAX_EDITWINDOWS );
			return;
		}
		/*======= Grepの実行 =============*/
		/* Grep結果ウィンドウの表示 */
		cmWork1.Replace( L"\"", L"\"\"" );
		cmWork2.Replace( _T("\""), _T("\"\"") );
		cmWork3.Replace( _T("\""), _T("\"\"") );
		cmWork4.Replace( L"\"", L"\"\"" );

		// -GREPMODE -GKEY="1" -GREPR="2" -GFILE="*.*;*.c;*.h" -GFOLDER="c:\" -GCODE=0 -GOPT=S
		CNativeT cCmdLine;
		TCHAR szTemp[20];
		cCmdLine.AppendString(_T("-GREPMODE -GKEY=\""));
		cCmdLine.AppendStringW(cmWork1.GetStringPtr());
		cCmdLine.AppendString(_T("\" -GREPR=\""));
		cCmdLine.AppendStringW(cmWork4.GetStringPtr());
		cCmdLine.AppendString(_T("\" -GFILE=\""));
		cCmdLine.AppendString(cmWork2.GetStringPtr());
		cCmdLine.AppendString(_T("\" -GFOLDER=\""));
		cCmdLine.AppendString(cmWork3.GetStringPtr());
		cCmdLine.AppendString(_T("\" -GCODE="));
		auto_sprintf( szTemp, _T("%d"), cDlgGrepRep.m_nGrepCharSet );
		cCmdLine.AppendString(szTemp);

		//GOPTオプション
		TCHAR	pOpt[64];
		pOpt[0] = _T('\0');
		if( cDlgGrepRep.m_bSubFolder				)_tcscat( pOpt, _T("S") );	// サブフォルダからも検索する
		if( cDlgGrepRep.m_sSearchOption.bWordOnly	)_tcscat( pOpt, _T("W") );	// 単語単位で探す
		if( cDlgGrepRep.m_sSearchOption.bLoHiCase	)_tcscat( pOpt, _T("L") );	// 英大文字と英小文字を区別する
		if( cDlgGrepRep.m_sSearchOption.bRegularExp	)_tcscat( pOpt, _T("R") );	// 正規表現
		if( cDlgGrepRep.m_nGrepOutputLineType == 1     )_tcscat( pOpt, _T("P") );	// 行を出力する
		// if( cDlgGrepRep.m_nGrepOutputLineType == 2     )_tcscat( pOpt, _T("N") );	// 否ヒット行を出力する 2014.09.23
		if( 1 == cDlgGrepRep.m_nGrepOutputStyle		)_tcscat( pOpt, _T("1") );	// Grep: 出力形式
		if( 2 == cDlgGrepRep.m_nGrepOutputStyle		)_tcscat( pOpt, _T("2") );	// Grep: 出力形式
		if( 3 == cDlgGrepRep.m_nGrepOutputStyle		)_tcscat( pOpt, _T("3") );
		if( cDlgGrepRep.m_bGrepOutputFileOnly		)_tcscat( pOpt, _T("F") );
		if( cDlgGrepRep.m_bGrepOutputBaseFolder		)_tcscat( pOpt, _T("B") );
		if( cDlgGrepRep.m_bGrepSeparateFolder		)_tcscat( pOpt, _T("D") );
		if( cDlgGrepRep.m_bPaste					)_tcscat( pOpt, _T("C") );	// クリップボードから貼り付け
		if( cDlgGrepRep.m_bBackup					)_tcscat( pOpt, _T("O") );	// バックアップ作成
		if( 0 < _tcslen( pOpt ) ){
			cCmdLine.AppendString( _T(" -GOPT=") );
			cCmdLine.AppendString( pOpt );
		}

		SLoadInfo sLoadInfo;
		sLoadInfo.cFilePath = _T("");
		sLoadInfo.eCharCode = CODE_NONE;
		sLoadInfo.bViewMode = false;
		CControlTray::OpenNewEditor( G_AppInstance(), m_pCommanderView->GetHwnd(), sLoadInfo, cCmdLine.GetStringPtr(),
			false, NULL, GetDllShareData().m_Common.m_sTabBar.m_bNewWindow? true : false );
	}
	return;
}

