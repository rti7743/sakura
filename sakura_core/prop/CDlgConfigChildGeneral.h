#ifndef SAKURA_CDLGCONFIGCHILDGENERAL_H_
#define SAKURA_CDLGCONFIGCHILDGENERAL_H_

#include "CDlgConfigChild.h"

//IDD_PROP_GENERAL

class CDlgConfigChildGeneral : public CDlgConfigChild
{
public:
	CDlgConfigChildGeneral( CommonSetting& common ): CDlgConfigChild( common ){}

	HWND DoModeless( HINSTANCE, HWND, SDlgConfigArg*, int = -1 );
protected:
	BOOL OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam );
	BOOL OnBnClicked( int );
	BOOL OnCbnSelEndOk( HWND hwndCtl, int wID );
	BOOL OnNotify( WPARAM wParam, LPARAM lParam );

	int GetData();
	void SetData();
	LPVOID GetHelpIdTable();
};

#endif
