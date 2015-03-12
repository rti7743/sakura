#ifndef SAKURA_CDLGCONFIGCHILDMAINMENU_H_
#define SAKURA_CDLGCONFIGCHILDMAINMENU_H_

#include "CDlgConfigChild.h"

//IDD_PROP_MAINMENU

class CDlgConfigChildMainMenu : public CDlgConfigChild
{
public:
	CDlgConfigChildMainMenu( CommonSetting& common ): CDlgConfigChild( common ){}

	HWND DoModeless( HINSTANCE, HWND, SDlgConfigArg*, int = -1 );
protected:
	BOOL OnInitDialog( HWND hWnd, WPARAM wParam, LPARAM lParam );
	BOOL OnBnClicked( int );
	BOOL OnNotify( WPARAM wParam, LPARAM lParam );
	BOOL OnCbnSelChange( HWND hwndCtl, int wID );
	BOOL OnTimer( WPARAM wParam );
	BOOL OnDestroy();

	int GetData();
	void SetData();
	LPVOID GetHelpIdTable();

	void Import();
	void Export();
	bool GetDataTree( HWND, HTREEITEM, int );
	bool Check_MainMenu( HWND, std::wstring& );
	bool Check_MainMenu_Sub( HWND, HTREEITEM, int, std::wstring& );

	HWND m_hwndComboFunkKind;
	HWND m_hwndListFunk;
	HWND m_hwndTreeRes;
	bool m_bInMove;
};

#endif
