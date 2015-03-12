/*
	Copyright (C) 2008, kobake

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
#ifndef SAKURA_CPROPERTYMANAGER_03C7D94F_54C6_4772_86BE_4A00A554FCAE_H_
#define SAKURA_CPROPERTYMANAGER_03C7D94F_54C6_4772_86BE_4A00A554FCAE_H_

class CImageListMgr;
class CMenuDrawer;

/*! プロパティシート番号
	@date 2008.6.22 Uchi #define -> enum に変更	
	@date 2008.6.22 Uchi順序変更 Win,Toolbar,Tab,Statusbarの順に、File,FileName 順に
	@date 2014.11.02 一部名称変更
*/
enum PropComSheetOrder {
	ID_PROPCOM_PAGENUM_GENERAL = 0,		//!< 全般
	ID_PROPCOM_PAGENUM_WINDOW,				//!< ウィンドウ
	ID_PROPCOM_PAGENUM_MAINMENU,		//!< メインメニュー
	ID_PROPCOM_PAGENUM_TOOLBAR,			//!< ツールバー
	ID_PROPCOM_PAGENUM_TABS,				//!< タブバー
	ID_PROPCOM_PAGENUM_STATUSBAR,		//!< ステータスバー
	ID_PROPCOM_PAGENUM_EDIT,			//!< 編集
	ID_PROPCOM_PAGENUM_FILE,			//!< ファイル
	ID_PROPCOM_PAGENUM_FILENAME,		//!< ファイル名表示
	ID_PROPCOM_PAGENUM_BACKUP,			//!< バックアップ
	ID_PROPCOM_PAGENUM_FORMAT,			//!< 書式
	ID_PROPCOM_PAGENUM_SEARCH,			//!< 検索
	ID_PROPCOM_PAGENUM_KEYBOARD,		//!< キー割り当て
	ID_PROPCOM_PAGENUM_CUSTMENU,		//!< カスタムメニュー
	ID_PROPCOM_PAGENUM_KEYWORD,			//!< 強調キーワード
	ID_PROPCOM_PAGENUM_SUPPORT,			//!< 支援
	ID_PROPCOM_PAGENUM_MACRO,			//!< マクロ
	ID_PROPCOM_PAGENUM_PLUGIN,			//!< プラグイン
	ID_PROPCOM_PAGENUM_MAX,
	ID_PROPCOM_PAGENUM_NONE = -1		//!< 未指定
};

class CPropertyManager{
public:
	void Create( HWND, CImageListMgr*, CMenuDrawer* );

	/*
	|| その他
	*/
	bool OpenPropertySheet( HWND hWnd, int nPageNum, bool bTrayProc );	/* 共通設定 */
	bool OpenPropertySheetTypes( HWND hWnd, int nPageNum, CTypeConfig nSettingType );	/* タイプ別設定 */

private:
	HWND			m_hwndOwner;
	CImageListMgr*	m_pImageList;
	CMenuDrawer*	m_pMenuDrawer;

	int				m_nPropComPageNum;
	int				m_nPropTypePageNum;
};

#endif /* SAKURA_CPROPERTYMANAGER_03C7D94F_54C6_4772_86BE_4A00A554FCAE_H_ */
/*[EOF]*/
