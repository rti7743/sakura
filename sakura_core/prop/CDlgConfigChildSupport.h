#ifndef SAKURA_CDLGCONFIGCHILDSUPPORT_H_
#define SAKURA_CDLGCONFIGCHILDSUPPORT_H_

#include "CDlgConfigChild.h"

//IDD_PROP_HELPER

class CDlgConfigChildSupport : public CDlgConfigChild
{
public:
	CDlgConfigChildSupport( CommonSetting& common ): CDlgConfigChild( common ){}

	HWND DoModeless( HINSTANCE, HWND, SDlgConfigArg*, int = -1 );
protected:
	BOOL OnInitDialog( HWND hWnd, WPARAM wParam, LPARAM lParam );
	BOOL OnBnClicked( int );
	BOOL OnDestroy();

	int GetData();
	void SetData();
	LPVOID GetHelpIdTable();

	HFONT m_hKeywordHelpFont;
};

#endif
