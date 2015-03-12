#ifndef SAKURA_CDLGCONFIGCHILDSTATUSBAR_H_
#define SAKURA_CDLGCONFIGCHILDSTATUSBAR_H_

#include "CDlgConfigChild.h"

//IDD_PROP_STATUSBAR

class CDlgConfigChildStatusbar : public CDlgConfigChild
{
public:
	CDlgConfigChildStatusbar( CommonSetting& common ): CDlgConfigChild( common ){}

	HWND DoModeless( HINSTANCE, HWND, SDlgConfigArg*, int = -1 );
protected:
	BOOL OnInitDialog( HWND hWnd, WPARAM wParam, LPARAM lParam );

	int GetData();
	void SetData();
	LPVOID GetHelpIdTable();
};

#endif
