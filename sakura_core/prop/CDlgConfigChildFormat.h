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
	BOOL OnCbnEditChange( HWND hwndCtl, int wID );
	BOOL OnCbnSelChange( HWND hwndCtl, int wID );
	BOOL OnBnClicked( int wID );

	int GetData();
	void SetData();
	LPVOID GetHelpIdTable();
	void OnComboChange( HWND hwndCtl, int wID, bool );
	void ChangeDateExample(bool);
	void ChangeTimeExample(bool);
	void EnableFormatPropInput();

	bool m_bInit;
};

#endif
