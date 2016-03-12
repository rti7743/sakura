/*
	Copyright (C) 2016, Moca

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

#ifndef SAKURA_CDLGCOLORMAKER_H_
#define SAKURA_CDLGCOLORMAKER_H_

#include "dlg/CDialog.h"

class CEditDoc;

class CDlgColorMarker : public CDialog
{
public:
	CDlgColorMarker();
	~CDlgColorMarker(){};

	int DoModal(HINSTANCE, HWND, LPARAM, CEditDoc*, bool bGetItemMode);

protected:
	BOOL OnBnClicked(int);
	BOOL OnCbnSelChange(HWND, int);
	LPVOID GetHelpIdTable(void);

	INT_PTR DispatchEvent(HWND, UINT, WPARAM, LPARAM);

	void SetData(void);
	int GetData(void);

	void SetDataPreset(void);
	void SetDataItem(void);

private:
	CEditDoc* m_pcEditDoc;
	bool m_bGetItemMode;
	wchar_t m_szPresetName[MAX_MARKER_NAME];
public:
	CMarkerItem m_cItem;
};



#endif // SAKURA_CDLGCOLORMAKER_H_
