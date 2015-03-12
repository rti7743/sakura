#ifndef SAKURA_CDLGCONFIGCHILDCUSTMENU_H_
#define SAKURA_CDLGCONFIGCHILDCUSTMENU_H_

#include "CDlgConfigChild.h"

//IDD_PROP_CUSTMENU

class CDlgConfigChildCustMenu : public CDlgConfigChild
{
public:
	CDlgConfigChildCustMenu( CommonSetting& common ): CDlgConfigChild( common ){}

	HWND DoModeless( HINSTANCE, HWND, SDlgConfigArg*, int = -1 );
protected:
	BOOL OnInitDialog( HWND hWnd, WPARAM wParam, LPARAM lParam );
	BOOL OnBnClicked( int );
	BOOL OnLbnDblclk( int wID );
	BOOL OnLbnSelChange( HWND hwndCtl, int wID );
	BOOL OnCbnSelChange( HWND hwndCtl, int wID );
	BOOL OnTimer( WPARAM wParam );
	BOOL OnDestroy();

	int GetData();
	void SetData();
	LPVOID GetHelpIdTable();

	void SetDataMenuList(int nIdx);
	void Import();
	void Export();
};

#endif
