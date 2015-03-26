/*
	Copyright (C) 2015, Moca

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/

#ifndef SAKRUA_CDLGMOUSEGESTURE_H_
#define SAKRUA_CDLGMOUSEGESTURE_H_

#include "dlg/CDialog.h"

class CDlgMouseGesture : public CDialog
{
public:
	int DoModal(HINSTANCE, HWND, int, CGestureInfo*, CFuncLookup*);	// モーダルダイアログの表示
protected:
	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnBnClicked(int);
	BOOL OnCbnSelChange( HWND hwndCtl, int wID );
	LPVOID GetHelpIdTable(void);
	void SetData();
	int GetData();

	void AddAction(int);
	void ResetAction();
	void BrowseFunc();
	void ClearFunc();

	CGestureInfo m_info;
	int m_nType;
	CFuncLookup* m_pcLookup;
};

#endif /* SAKRUA_CDLGMOUSEGESTURE_H_ */
