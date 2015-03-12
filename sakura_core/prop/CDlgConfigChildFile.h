#ifndef SAKURA_CDLGCONFIGCHILDFILE_H_
#define SAKURA_CDLGCONFIGCHILDFILE_H_

#include "CDlgConfigChild.h"

//IDD_PROP_FILE

class CDlgConfigChildFile : public CDlgConfigChild
{
public:
	CDlgConfigChildFile( CommonSetting& common ): CDlgConfigChild( common ){}

	HWND DoModeless( HINSTANCE, HWND, SDlgConfigArg*, int = -1 );
protected:
	BOOL OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam );
	BOOL OnNotify( WPARAM wParam, LPARAM lParam );
	BOOL OnCbnSelChange( HWND hwndCtl, int wID );
	BOOL OnBnClicked( int wID );

	int GetData();
	void SetData();
	LPVOID GetHelpIdTable();

	void EnableFilePropInput();
};

#endif
