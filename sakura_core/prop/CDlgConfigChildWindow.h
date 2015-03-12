#ifndef SAKURA_CDLGCONFIGCHILDWINDOW_H_
#define SAKURA_CDLGCONFIGCHILDWINDOW_H_

#include "CDlgConfigChild.h"

//IDD_PROP_WIN

class CDlgConfigChildWindow : public CDlgConfigChild
{
public:
	CDlgConfigChildWindow( CommonSetting& common ): CDlgConfigChild( common ), m_hDialogFont(NULL){}

	HWND DoModeless( HINSTANCE, HWND, SDlgConfigArg*, int = -1 );
protected:
	BOOL OnInitDialog( HWND hWnd, WPARAM wParam, LPARAM lParam );
	BOOL OnBnClicked( int );
	BOOL OnNotify( WPARAM wParam, LPARAM lParam );
	BOOL OnDestroy();

	int GetData();
	void SetData();
	LPVOID GetHelpIdTable();

	void EnableWinPropInput();

	HFONT m_hDialogFont;
};

#endif
