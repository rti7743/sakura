#ifndef SAKURA_CDLGCONFIGCHILDTOOLBAR_H_
#define SAKURA_CDLGCONFIGCHILDTOOLBAR_H_

#include "CDlgConfigChild.h"

//IDD_PROP_TOOLBAR

class CDlgConfigChildToolbar : public CDlgConfigChild
{
public:
	CDlgConfigChildToolbar( CommonSetting& common ): CDlgConfigChild( common ){}

	HWND DoModeless( HINSTANCE, HWND, SDlgConfigArg*, int = -1 );
protected:
	BOOL OnInitDialog( HWND hWnd, WPARAM wParam, LPARAM lParam );
	BOOL OnDrawItem( WPARAM wParam, LPARAM lParam );
	BOOL OnCbnSelChange( HWND hwndCtl, int wID );
	BOOL OnBnClicked( int );
	BOOL OnTimer( WPARAM wParam );
	BOOL OnDestroy();

	int GetData();
	void SetData();
	LPVOID GetHelpIdTable();

	void DrawToolBarItemList( DRAWITEMSTRUCT* pDis );

	int m_nListItemHeight;
	int m_nListBoxTopMargin;
	HWND m_hwndCombo;
	HWND m_hwndFuncList;
	HWND m_hwndResList;
};

#endif
