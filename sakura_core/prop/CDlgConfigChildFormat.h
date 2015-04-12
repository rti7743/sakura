#ifndef SAKURA_CDLGCONFIGCHILDFORMAT_H_
#define SAKURA_CDLGCONFIGCHILDFORMAT_H_

#include "CDlgConfigChild.h"

//IDD_PROP_FORMAT

class CDlgConfigChildFormat : public CDlgConfigChild
{
public:
	CDlgConfigChildFormat( CommonSetting& common ): CDlgConfigChild( common ), m_bInit(false){}

	HWND DoModeless( HINSTANCE, HWND, SDlgConfigArg*, int = -1 );
protected:
	BOOL OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam );
	BOOL OnEnChange( HWND hwndCtl, int wID );
	BOOL OnBnClicked( int wID );

	int GetData();
	void SetData();
	LPVOID GetHelpIdTable();
	void ChangeDateExample();
	void ChangeTimeExample();
	void EnableFormatPropInput();

	bool m_bInit;
};

#endif
