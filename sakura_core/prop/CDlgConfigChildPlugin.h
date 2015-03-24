#ifndef SAKURA_CDLGCONFIGCHILDPLUGIN_H_
#define SAKURA_CDLGCONFIGCHILDPLUGIN_H_

#include "CDlgConfigChild.h"

//IDD_PROP_PLUGIN

class CDlgConfigChildPlugin : public CDlgConfigChild
{
public:
	CDlgConfigChildPlugin( CommonSetting& common ): CDlgConfigChild( common ){}

	HWND DoModeless( HINSTANCE, HWND, SDlgConfigArg*, int = -1 );

	std::tstring GetReadMeFile(const std::tstring&);
	bool BrowseReadMe(const std::tstring&);
protected:
	BOOL OnInitDialog( HWND hWnd, WPARAM wParam, LPARAM lParam );
	BOOL OnNotify( WPARAM wParam, LPARAM lParam );
	BOOL OnBnClicked( int );

	int GetData();
	void SetData();
	LPVOID GetHelpIdTable();

	void SetData_LIST();
	void InitDialog( HWND );
	void EnablePluginPropInput();
};

#endif
