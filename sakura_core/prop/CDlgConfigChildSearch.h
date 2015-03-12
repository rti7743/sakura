#ifndef SAKURA_CDLGCONFIGCHILDSEARCH_H_
#define SAKURA_CDLGCONFIGCHILDSEARCH_H_

#include "CDlgConfigChild.h"

//IDD_PROP_GREP

class CDlgConfigChildSearch : public CDlgConfigChild
{
public:
	CDlgConfigChildSearch( CommonSetting& common ): CDlgConfigChild( common ){}

	HWND DoModeless( HINSTANCE, HWND, SDlgConfigArg*, int = -1 );
protected:
	BOOL OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam );
	BOOL OnEnKillFocus( HWND hwndCtl, int wID );

	int GetData();
	void SetData();
	LPVOID GetHelpIdTable();

	void SetRegexpVersion();
};

#endif
