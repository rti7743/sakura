#ifndef SAKURA_CDLGCONFIGCHILDFILENAME_H_
#define SAKURA_CDLGCONFIGCHILDFILENAME_H_

#include "CDlgConfigChild.h"

//IDD_PROP_FNAME

class CDlgConfigChildFileName : public CDlgConfigChild
{
public:
	CDlgConfigChildFileName( CommonSetting& common ): CDlgConfigChild( common ){}

	HWND DoModeless( HINSTANCE, HWND, SDlgConfigArg*, int = -1 );
protected:
	BOOL OnInitDialog( HWND hWnd, WPARAM wParam, LPARAM lParam );
	BOOL OnNotify( WPARAM wParam, LPARAM lParam );
	BOOL OnBnClicked( int );
	
	int GetData();
	void SetData();
	LPVOID GetHelpIdTable();

	int SetListViewItem_FILENAME( HWND, int, LPTSTR, LPTSTR, bool );
	void GetListViewItem_FILENAME( HWND, int, LPTSTR, LPTSTR );
	int MoveListViewItem_FILENAME( HWND, int, int );

	int m_nLastPos; //!< 前回フォーカスのあった場所 ファイル名タブ用
};

#endif
