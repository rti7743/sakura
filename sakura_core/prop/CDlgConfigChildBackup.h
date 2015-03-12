#ifndef SAKURA_CDLGCONFIGCHILDBACKUP_H_
#define SAKURA_CDLGCONFIGCHILDBACKUP_H_

#include "CDlgConfigChild.h"

//IDD_PROP_BACKUP

class CDlgConfigChildBackup : public CDlgConfigChild
{
public:
	CDlgConfigChildBackup( CommonSetting& common ): CDlgConfigChild( common ){}

	HWND DoModeless( HINSTANCE, HWND, SDlgConfigArg*, int = -1 );
protected:
	BOOL OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam );
	BOOL OnNotify( WPARAM wParam, LPARAM lParam );
	BOOL OnBnClicked( int wID );
	BOOL OnEnChange( HWND hwndCtl, int wID );

	int GetData();
	void SetData();
	LPVOID GetHelpIdTable();

	void EnableBackupInput();
	void UpdateBackupFile();
};

#endif
