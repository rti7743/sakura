#ifndef SAKURA_CDLGCONFIGCHILDGESTURE_H_
#define SAKURA_CDLGCONFIGCHILDGESTURE_H_

#include "CDlgConfigChild.h"

//IDD_PROP_GESTURE

class CDlgConfigChildGesture : public CDlgConfigChild
{
public:
	CDlgConfigChildGesture( CommonSetting& common ): CDlgConfigChild( common ){}

	HWND DoModeless( HINSTANCE, HWND, SDlgConfigArg*, int = -1 );
protected:
	BOOL OnInitDialog( HWND hWnd, WPARAM wParam, LPARAM lParam );
	BOOL OnNotify( WPARAM wParam, LPARAM lParam );
	BOOL OnBnClicked( int );
	BOOL OnTimer( WPARAM wParam );
	BOOL OnDestroy();

	void AddGesture(HWND hwndDlg, HWND hList);
	void EditGesture(HWND hwndDlg, HWND hList);
	void DeleteGesture(HWND hwndDlg, HWND hList);
	void ImportGesture(HWND hwndDlg, HWND hList);
	void ExportGesture(HWND hwndDlg, HWND hList);
	void SetData();
	int GetData();
	int GetType(HWND hwndDlg);
	int InputGesture(HWND hwndDlg, CGestureInfo& info);
	LPVOID GetHelpIdTable();
};

#endif
