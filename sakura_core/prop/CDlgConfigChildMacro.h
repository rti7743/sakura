#ifndef SAKURA_CDLGCONFIGCHILDMACRO_H_
#define SAKURA_CDLGCONFIGCHILDMACRO_H_

#include "CDlgConfigChild.h"

//IDD_PROP_MACRO

class CDlgConfigChildMacro : public CDlgConfigChild
{
public:
	CDlgConfigChildMacro( CommonSetting& common ): CDlgConfigChild( common ){}

	HWND DoModeless( HINSTANCE, HWND, SDlgConfigArg*, int = -1 );
protected:
	BOOL OnInitDialog( HWND hWnd, WPARAM wParam, LPARAM lParam );
	BOOL OnNotify( WPARAM wParam, LPARAM lParam );
	BOOL OnBnClicked( int );
	BOOL OnCbnDropDown( HWND hwndCtl, int wID );
	BOOL OnEnKillFocus( HWND hwndCtl, int wID );

	int GetData();
	void SetData();
	LPVOID GetHelpIdTable();

	void InitDialog( HWND hwndDlg );	//!< Macroページの初期化
	void SetMacro2List_Macro();			//!< Macroデータの設定
	void SelectBaseDir_Macro();			//!< Macroディレクトリの選択
	void OnFileDropdown_Macro();		//!< ファイルドロップダウンが開かれるとき
	void CheckListPosition_Macro();		//!< リストビューのFocus位置確認

	int				m_nLastPos; //!< 前回フォーカスのあった場所
};

#endif
