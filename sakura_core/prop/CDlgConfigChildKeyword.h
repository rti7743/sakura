#ifndef SAKURA_CDLGCONFIGCHILDKEYWORD_H_
#define SAKURA_CDLGCONFIGCHILDKEYWORD_H_

#include "CDlgConfigChild.h"

//IDD_PROP_KEYWORD

class CDlgConfigChildKeyword : public CDlgConfigChild
{
public:
	CDlgConfigChildKeyword( CommonSetting& common ): CDlgConfigChild( common ){}

	HWND DoModeless( HINSTANCE, HWND, SDlgConfigArg*, int = -1 );
protected:
	BOOL OnInitDialog( HWND hWnd, WPARAM wParam, LPARAM lParam );
	BOOL OnNotify( WPARAM wParam, LPARAM lParam );
	BOOL OnCbnSelChange( HWND hwndCtl, int wID );
	BOOL OnBnClicked( int );
	BOOL OnDestroy();
	
	int GetData();
	void SetData();
	LPVOID GetHelpIdTable();

	void Edit_List_KeyWord( HWND hwndLIST_KEYWORD );
	void Delete_List_KeyWord( HWND hwndLIST_KEYWORD );
	void Import_List_KeyWord( HWND hwndLIST_KEYWORD );
	void Export_List_KeyWord( HWND hwndLIST_KEYWORD );
	void Clean_List_KeyWord( HWND hwndLIST_KEYWORD );
	void SetKeyWordSet( int nIdx );
	void DispKeywordCount();
};

#endif
