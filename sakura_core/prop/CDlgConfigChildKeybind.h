#ifndef SAKURA_CDLGCONFIGCHILDKEYBIND_H_
#define SAKURA_CDLGCONFIGCHILDKEYBIND_H_

#include "CDlgConfigChild.h"

//IDD_PROP_KEYBIND

class CDlgConfigChildKeybind : public CDlgConfigChild
{
public:
	CDlgConfigChildKeybind( CommonSetting& common ): CDlgConfigChild( common ){}

	HWND DoModeless( HINSTANCE, HWND, SDlgConfigArg*, int = -1 );
protected:
	BOOL OnInitDialog( HWND hWnd, WPARAM wParam, LPARAM lParam );
	BOOL OnBnClicked( int );
	BOOL OnLbnSelChange( HWND hwndCtl, int wID );
	BOOL OnCbnSelChange( HWND hwndCtl, int wID );
	BOOL OnTimer( WPARAM wParam );
	BOOL OnDestroy();

	int GetData();
	void SetData();
	LPVOID GetHelpIdTable();

	void ChangeKeyList();
	void Import();
	void Export();

	HWND	m_hwndCombo;
	HWND	m_hwndFuncList;
	HWND	m_hwndKeyList;
	HWND	m_hwndCheckShift;
	HWND	m_hwndCheckCtrl;
	HWND	m_hwndCheckAlt;
	HWND	m_hwndAssignedkeyList;
	HWND	m_hwndEDIT_KEYSFUNC;
};

#endif
