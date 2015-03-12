#ifndef SAKURA_CDLGCONFIGCHILDTAB_H_
#define SAKURA_CDLGCONFIGCHILDTAB_H_

#include "CDlgConfigChild.h"

//IDD_PROP_TAB

class CDlgConfigChildTab : public CDlgConfigChild
{
public:
	CDlgConfigChildTab( CommonSetting& common ): CDlgConfigChild( common ), m_hTabFont(NULL){}

	HWND DoModeless(HINSTANCE, HWND, SDlgConfigArg*, int = -1);
protected:
	BOOL OnInitDialog( HWND hWnd, WPARAM wParam, LPARAM lParam );
	BOOL OnBnClicked( int );
	BOOL OnDestroy();

	int GetData();
	void SetData();
	LPVOID GetHelpIdTable();

	void EnableTabPropInput();

	HFONT m_hTabFont;
};

#endif
