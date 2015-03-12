#ifndef SAKURA_CDLGCONFIGCHILDEDIT_H_
#define SAKURA_CDLGCONFIGCHILDEDIT_H_

#include "CDlgConfigChild.h"

//IDD_PROP_EDIT

class CDlgConfigChildEdit : public CDlgConfigChild
{
public:
	CDlgConfigChildEdit( CommonSetting& common ): CDlgConfigChild( common ){}

	HWND DoModeless( HINSTANCE, HWND, SDlgConfigArg*, int = -1 );
protected:
	BOOL OnInitDialog( HWND hWnd, WPARAM wParam, LPARAM lParam );
	BOOL OnBnClicked( int );

	int GetData();
	void SetData();
	void EnableEditPropInput();
	LPVOID GetHelpIdTable();
};

#endif
