/*!	@file
	@brief 設定ダイアログ

	@author D.S.Koba
	@date 2004-03-06 D.S.Koba 
*/
/*
	Copyright (C) 2004, D.S.Koba
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
#include "CDlgConfig.h"

#include "CDlgConfigChildGeneral.h"
#include "CDlgConfigChildWindow.h"
#include "CDlgConfigChildMainMenu.h"
#include "CDlgConfigChildToolbar.h"
#include "CDlgConfigChildTab.h"
#include "CDlgConfigChildStatusbar.h"
#include "CDlgConfigChildEdit.h"
#include "CDlgConfigChildFile.h"
#include "CDlgConfigChildFileName.h"
#include "CDlgConfigChildBackup.h"
#include "CDlgConfigChildFormat.h"
#include "CDlgConfigChildSearch.h"
#include "CDlgConfigChildKeybind.h"
#include "CDlgConfigChildCustMenu.h"
#include "CDlgConfigChildKeyword.h"
#include "CDlgConfigChildSupport.h"
#include "CDlgConfigChildMacro.h"
#include "CDlgConfigChildPlugin.h"
#include "env/CShareData.h"
#include "util/shell.h"
#include "sakura_rc.h"

class CConfigChildHolder
{
#define ChildOpenClose(CLASSNAME) \
private:\
	CDlgConfigChild##CLASSNAME* m_cDlg##CLASSNAME;\
public:\
	static BOOL Open##CLASSNAME(CConfigChildHolder* p, HINSTANCE hInstance, HWND hwndParent, SDlgConfigArg* pDlgConfigArg, int nTypeIndex)\
	{\
		p->m_cDlg##CLASSNAME = new CDlgConfigChild##CLASSNAME(pDlgConfigArg->m_Common);\
		p->m_cDlg##CLASSNAME->DoModeless(hInstance, hwndParent, pDlgConfigArg, nTypeIndex);\
		return TRUE;\
	}\
	static BOOL Close##CLASSNAME(CConfigChildHolder* p)\
	{\
		p->m_cDlg##CLASSNAME->CloseDialog();\
		delete p->m_cDlg##CLASSNAME;\
		p->m_cDlg##CLASSNAME = NULL;\
		return TRUE;\
	}\

	ChildOpenClose(General)
	ChildOpenClose(Window)
	ChildOpenClose(MainMenu)
	ChildOpenClose(Toolbar)
	ChildOpenClose(Tab)
	ChildOpenClose(Statusbar)
	ChildOpenClose(Edit)
	ChildOpenClose(File)
	ChildOpenClose(FileName)
	ChildOpenClose(Backup)
	ChildOpenClose(Format)
	ChildOpenClose(Search)
	ChildOpenClose(Keybind)
	ChildOpenClose(CustMenu)
	ChildOpenClose(Keyword)
	ChildOpenClose(Support)
	ChildOpenClose(Macro)
	ChildOpenClose(Plugin)

#undef ChildOpenClose
public:
	CConfigChildHolder(){}
	static const SConfigChildDlgInfo defaultItems[18];
	static const SConfigChildDlgInfo keywordItems[1];
};
const SConfigChildDlgInfo CConfigChildHolder::defaultItems[18] = {
	{ STR_PROPCOMMON_GENERAL,	COMMON,	OpenGeneral,	CloseGeneral,	F_OPTION_GENERAL },
	{ STR_PROPCOMMON_WINDOW,	COMMON,	OpenWindow,		CloseWindow,	F_OPTION_WINDOW },
	{ STR_PROPCOMMON_MAINMENU,	COMMON,	OpenMainMenu,	CloseMainMenu,	F_OPTION_MAINMENU },
	{ STR_PROPCOMMON_TOOLBAR,	COMMON,	OpenToolbar,	CloseToolbar,	F_OPTION_TOOLBAR },
	{ STR_PROPCOMMON_TABS,		COMMON,	OpenTab,		CloseTab,		F_OPTION_TAB },
	{ STR_PROPCOMMON_STATBAR,	COMMON,	OpenStatusbar,	CloseStatusbar,	F_OPTION_STATUSBAR },
	{ STR_PROPCOMMON_EDITING,	COMMON,	OpenEdit,		CloseEdit,		F_OPTION_EDIT },
	{ STR_PROPCOMMON_FILE,		COMMON,	OpenFile,		CloseFile,		F_OPTION_FILE },
	{ STR_PROPCOMMON_FILENAME,	COMMON,	OpenFileName,	CloseFileName,	F_OPTION_FNAME },
	{ STR_PROPCOMMON_BACKUP,	COMMON,	OpenBackup,		CloseBackup,	F_OPTION_BACKUP },
	{ STR_PROPCOMMON_FORMAT,	COMMON,	OpenFormat,		CloseFormat,	F_OPTION_FORMAT },
	{ STR_PROPCOMMON_SEARCH,	COMMON,	OpenSearch,		CloseSearch,	F_OPTION_GREP },
	{ STR_PROPCOMMON_KEYS,		COMMON,	OpenKeybind,	CloseKeybind,	F_OPTION_KEYBIND },
	{ STR_PROPCOMMON_CUSTMENU,	COMMON,	OpenCustMenu,	CloseCustMenu,	F_OPTION_CUSTMENU },
	{ STR_PROPCOMMON_KEYWORD,	COMMON,	OpenKeyword,	CloseKeyword,	F_OPTION_KEYWORD },
	{ STR_PROPCOMMON_SUPPORT,	COMMON,	OpenSupport,	CloseSupport,	F_OPTION_HELPER },
	{ STR_PROPCOMMON_MACRO,		COMMON,	OpenMacro,		CloseMacro,		F_OPTION_MACRO },
	{ STR_PROPCOMMON_PLUGIN,	COMMON,	OpenPlugin,		ClosePlugin,	F_OPTION_PLUGIN },
};
	
const SConfigChildDlgInfo CConfigChildHolder::keywordItems[1] = {
	{ STR_PROPCOMMON_KEYWORD,	COMMON,	OpenKeyword,	CloseKeyword,	F_OPTION_KEYWORD },
};


CDlgConfig::CDlgConfig()
{
	m_pDlgConfigArg = new SDlgConfigArg();
	m_pDlgConfigArg->m_nKeywordSet1 = -1;
	m_pHolder = new CConfigChildHolder();
	m_nItemSelectNum = -1;
}

CDlgConfig::~CDlgConfig()
{
	delete m_pDlgConfigArg;
	delete m_pHolder;
}

/*!
	モーダルダイアログの表示
	@param hInstance [in] このアプリケーションのインスタンスハンドル
	@param hwndParent [in] 親ウィンドウのハンドル
*/
INT_PTR CDlgConfig::DoModal( HINSTANCE hInstance, HWND hwndParent, CImageListMgr* pcIcons, CMenuDrawer* pMenuDrawer, int PageNum, bool bTrayProc )
{
	m_pDlgConfigArg->m_pcIcons = pcIcons;
	m_pDlgConfigArg->m_pcMenuDrawer = pMenuDrawer;
	m_pDlgConfigArg->m_bTrayProc = bTrayProc;
	m_pDlgConfigArg->m_cLookup.Init( m_pDlgConfigArg->m_Common.m_sMacro.m_MacroTable, &m_pDlgConfigArg->m_Common );
	m_nItemSelectNum = PageNum;
	return CDialog::DoModal( hInstance, hwndParent, IDD_CONFIG, 0 );
}


/*!
	ダイアログを初期化
	
	@param hwndDlg [in] このダイアログのウィンドウハンドル
	@param wParam [in]
	@param lParam [in] CreateDialogParam()の第5引数，つまりこのクラスの this ポインタ

	@retval TRUE  成功
	@retval FALSE 失敗		
*/
BOOL CDlgConfig::OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
{
	BOOL bRet = CDialog::OnInitDialog( hwndDlg, wParam, lParam );

	if( !CShareData::getInstance()->IsPrivateSettings() ){
		::ShowWindow(GetItemHwnd(IDC_BUTTON_CONFIGFOLDER), SW_HIDE);
	}
	/*
	 * MAX_TYPES が影響するのはこの部分のみ
	 */
	m_hItemTypeName.resize( MAX_TYPES );

	//ツリービューを初期化
	InitTreeView( hwndDlg );
	return bRet;
}

BOOL CDlgConfig::OnBnClicked( int wId )
{
	switch( wId )
	{
	case IDC_BUTTON_CONFIGFOLDER:
		CommandSettingFolderMenu( GetHwnd(), GetItemHwnd(wId) );
		return TRUE;
	case IDOK:
		//子ダイアログを閉じる
		CloseChildDialog();
		//ダイアログデータの取得
		CloseDialog( GetData() );
		return TRUE;
	case IDCANCEL:
		//子ダイアログを閉じる
		CloseChildDialog();
		CloseDialog( FALSE );
		return TRUE;
	case IDC_BUTTON_HELP:
		/* ヘルプ */
		unsigned int i;
		for( i = 0; i < m_vecItemAndFunction.size(); i++ ){
			if( m_vecItemAndFunction.at(i).isOpened ){
				EFunctionCode nFuncId = m_vecItemAndFunction.at(i).nFuncId;
				if( -1 != nFuncId ){
					int nContextID = FuncID_To_HelpContextID(nFuncId);
					if( -1 != nContextID ){
						MyWinHelp( GetHwnd(), HELP_CONTEXT, nContextID );
					}
				}
				break;
			}
		}
		return TRUE;
	}
	return CDialog::OnBnClicked( wId );
}


BOOL CDlgConfig::OnNotify( WPARAM wParam, LPARAM lParam )
{
	LPNMHDR pnmhdr = (LPNMHDR)lParam;
	if(pnmhdr->idFrom == IDC_CONFIGTREE)
	{
		switch(pnmhdr->code)
		{
		case TVN_SELCHANGED:
		{
			//表示中の子ダイアログを破棄
			CloseChildDialog();
			//選択されたアイテムを取得
			HTREEITEM hItem = TreeView_GetSelection( m_hwndTree );
			//選択されたアイテムに対応したダイアログを生成，表示
			OpenChildDialog( hItem );
			//生成した子ダイアログにフォーカスが移動するのでツリービューへ戻す
			::SetFocus( m_hwndTree );
			break;
		}
		}
	}
	return CDialog::OnNotify( wParam, lParam );
}

/*!
	ダイアログデータの取得
*/
BOOL CDlgConfig::GetData()
{
	// AppyData()を使用すること
	return TRUE;
}

/*!
	ダイアログデータの設定

	CDialog::OnInitDialog() 内で呼ばれる
*/
void CDlgConfig::SetData()
{
	m_pDlgConfigArg->m_Common = m_pShareData->m_Common;

	int i;
	for( i = 0; i < GetDllShareData().m_nTypesCount; ++i ){
		SKeywordSetIndex indexs;
		STypeConfig type;
		CDocTypeManager().GetTypeConfig(CTypeConfig(i), type);
		indexs.typeId = type.m_id;
		for( int j = 0; j < MAX_KEYWORDSET_PER_TYPE; j++ ){
			indexs.index[j] = type.m_nKeyWordSetIdx[j];
		}
		m_pDlgConfigArg->m_Types_nKeyWordSetIdx.push_back(indexs);
	}
	
	return;
}

/*! 設定の適用
*/
void CDlgConfig::ApplyData(){
	m_pShareData->m_Common = m_pDlgConfigArg->m_Common;

	int i;
	const int nSize = (int)m_pDlgConfigArg->m_Types_nKeyWordSetIdx.size();
	for( i = 0; i < nSize; ++i ){
		CTypeConfig configIdx = CDocTypeManager().GetDocumentTypeOfId( m_pDlgConfigArg->m_Types_nKeyWordSetIdx[i].typeId );
		if( configIdx.IsValidType() ){
			STypeConfig type;
			CDocTypeManager().GetTypeConfig(configIdx, type);
			for( int j = 0; j < MAX_KEYWORDSET_PER_TYPE; j++ ){
				type.m_nKeyWordSetIdx[j] = m_pDlgConfigArg->m_Types_nKeyWordSetIdx[i].index[j];
			}
			CDocTypeManager().SetTypeConfig(configIdx, type);
		}
	}
}

/*!
	ツリービューを初期化

	@param hwndDlg [in] このダイアログのウィンドウハンドル
*/
void CDlgConfig::InitTreeView( HWND hwndDlg )
{
	m_hwndTree = ::GetDlgItem( hwndDlg, IDC_CONFIGTREE );
	//TreeView_DeleteAllItems( m_hwndTree );
	TVINSERTSTRUCT tvis;

	tvis.hInsertAfter = TVI_LAST;
	tvis.item.mask = TVIF_TEXT;

	//共通設定ルートアイテム
	tvis.hParent = TVI_ROOT;
	tvis.item.pszText = const_cast<LPTSTR>(LS(STR_PROPCOMMON));
	HTREEITEM hCommonRoot = TreeView_InsertItem(m_hwndTree, &tvis);

	SItemAndDialog buf;
	buf.isOpened = FALSE;
	unsigned int i;
	HTREEITEM hItemStartSelect = NULL;
	
	for( i = 0; i < m_vecConfigChildDlg.size(); i++ ){
		if( m_vecConfigChildDlg.at(i).m_Type == COMMON ){
			//共通設定アイテム
			tvis.hParent = hCommonRoot;
			tvis.item.pszText = const_cast<LPTSTR>(m_vecConfigChildDlg.at(i).m_strName.c_str());

			buf.hItem    = TreeView_InsertItem(m_hwndTree, &tvis);
			buf.OpenDlg  = m_vecConfigChildDlg.at(i).m_OpenDlg;
			buf.CloseDlg = m_vecConfigChildDlg.at(i).m_CloseDlg;
			buf.nFuncId = m_vecConfigChildDlg.at(i).m_nHelpFuncId;
			m_vecItemAndFunction.push_back( buf );
			if( m_nItemSelectNum == i || m_nItemSelectNum == -1 ){
				m_nItemSelectNum = i;
				hItemStartSelect = buf.hItem;
			}
		}
	}
	if( hItemStartSelect ){
		TreeView_Select( m_hwndTree, hItemStartSelect, TVGN_CARET );
		OpenChildDialog( hItemStartSelect );
	}else{
		TreeView_Select( m_hwndTree, hCommonRoot, TVGN_CARET );
	}
}


void CDlgConfig::SetDefaultChildDialog()
{
	int i;
	for( i = 0; i < _countof(CConfigChildHolder::defaultItems); i++ ){
		const SConfigChildDlgInfo& item = CConfigChildHolder::defaultItems[i];
		SConfigChildDlgInfoStr buf = {
			LS(item.m_NameId), item.m_Type, item.m_OpenDlg, item.m_CloseDlg, item.m_nHelpFuncId
		};
		m_vecConfigChildDlg.push_back( buf );
	}
}


void CDlgConfig::SetKeywordChildDialog()
{
	int i;
	for( i = 0; i < _countof(CConfigChildHolder::keywordItems); i++ ){
		const SConfigChildDlgInfo& item = CConfigChildHolder::keywordItems[i];
		SConfigChildDlgInfoStr buf = {
			LS(item.m_NameId), item.m_Type, item.m_OpenDlg, item.m_CloseDlg, item.m_nHelpFuncId
		};
		m_vecConfigChildDlg.push_back( buf );
	}
}


/*!
	子ダイアログを追加
*/
void CDlgConfig::AddChildDlg( TCHAR* pszName,
							  ConfigItemType Type,
							  BOOL(*OpenDlg)(CConfigChildHolder*, HINSTANCE, HWND, SDlgConfigArg*, int ),
							  BOOL(*CloseDlg)(CConfigChildHolder*),
							  EFunctionCode nFuncId)
{
	SConfigChildDlgInfoStr buf = {
		pszName, Type, OpenDlg, CloseDlg, nFuncId
	};
	m_vecConfigChildDlg.push_back( buf );
}


/*!
	子ダイアログを表示

	ツリーアイテムに対応する子ダイアログが有る場合に子ダイアログを表示する

	@param hItem [in] 選択されたツリーアイテム
*/
void CDlgConfig::OpenChildDialog( const HTREEITEM hItem )
{
	// 子ダイアログを開く
	unsigned int i;
	for( i = 0; i < m_vecItemAndFunction.size(); i++ ){
		if( hItem == m_vecItemAndFunction.at(i).hItem
			&& !m_vecItemAndFunction.at(i).isOpened
			&& m_vecItemAndFunction.at(i).OpenDlg != NULL ){
			m_vecItemAndFunction.at(i).isOpened = TRUE;
			m_vecItemAndFunction.at(i).OpenDlg(m_pHolder, m_hInstance, GetHwnd(), m_pDlgConfigArg, 0 );
			m_nItemSelectNum = i;
		}
	}
}

void CDlgConfig::CloseChildDialog()
{
	unsigned int i;
	for( i = 0; i < m_vecItemAndFunction.size(); i++ ){
		if( m_vecItemAndFunction.at(i).isOpened
			&& m_vecItemAndFunction.at(i).CloseDlg != NULL ){
			m_vecItemAndFunction.at(i).isOpened = FALSE;
			m_vecItemAndFunction.at(i).CloseDlg(m_pHolder);
		}
	}
}


/*!	コントロールにフォント設定する
	@date 2013.04.24 Uchi
*/
HFONT CDlgConfigChild::SetCtrlFont( int idc_ctrl, const LOGFONT& lf )
{
	HWND hwndDlg = GetHwnd();
	HFONT	hFont;
	HWND	hCtrl;

	// 論理フォントを作成
	hCtrl = ::GetDlgItem( hwndDlg, idc_ctrl );
	hFont = ::CreateFontIndirect( &lf );
	if (hFont) {
		// フォントの設定
		::SendMessage( hCtrl, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0) );
	}

	return hFont;
}


/*!	フォントラベルにフォントとフォント名設定する
	@date 2013.04.24 Uchi
*/
HFONT CDlgConfigChild::SetFontLabel( int idc_static, const LOGFONT& lf, int nps, bool bUse )
{
	HWND hwndDlg = GetHwnd();
	if( !bUse ) {
		::DlgItem_SetText( hwndDlg, idc_static, _T("") );
		return NULL;
	}
	HFONT	hFont;
	TCHAR	szFontName[80];
	LOGFONT lfTemp;
	lfTemp = lf;
	// 大きすぎるフォントは小さく表示
	if( lfTemp.lfHeight < -16 ){
		lfTemp.lfHeight = -16;
	}

	hFont = SetCtrlFont( idc_static, lfTemp );

	// フォント名の設定
	auto_sprintf( szFontName, nps % 10 ? _T("%s(%.1fpt)") : _T("%s(%.0fpt)"),
		lf.lfFaceName, double(nps)/10 );
	::DlgItem_SetText( hwndDlg, idc_static, szFontName );

	return hFont;
}
