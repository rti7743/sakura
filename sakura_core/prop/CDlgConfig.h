/*!	@file
	@brief 設定ダイアログ

	@author D.S.Koba
	@date 2004-03-06 D.S.Koba 
*/
/*
	Copyright (C) 2004, D.S.Koba
	Copyright (C) 2014, Moca

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

#ifndef SAKURA_CDLGCONFIG_H_
#define SAKURA_CDLGCONFIG_H_

class CImageListMgr;
class CMenuDrawer;

#include "dlg/CDialog.h"
#include "func/CFuncLookup.h"
#include "env/DLLSHAREDATA.h"
#include "env/CDocTypeManager.h"
#include <vector>

enum ConfigItemType
{
	COMMON,
//	TYPES,
	PLUGIN
};

struct SKeywordSetIndex{
	int typeId;
	int index[MAX_KEYWORDSET_PER_TYPE];
};

struct SDlgConfigArg
{
	CommonSetting	m_Common;
	std::vector<SKeywordSetIndex>	m_Types_nKeyWordSetIdx;

	int				m_nKeywordSet1;
	bool			m_bTrayProc;

	CImageListMgr*	m_pcIcons;
	CMenuDrawer*	m_pcMenuDrawer;
	CFuncLookup		m_cLookup;
};

class CConfigChildHolder;
struct SConfigChildDlgInfo
{
	int					m_NameId;	//!< ツリービューに表示する名前
	ConfigItemType		m_Type;
	BOOL				(*m_OpenDlg)(CConfigChildHolder*, HINSTANCE, HWND, SDlgConfigArg*, int);
	BOOL				(*m_CloseDlg)(CConfigChildHolder*);
	EFunctionCode		m_nHelpFuncId;
};

struct SConfigChildDlgInfoStr
{
	std::tstring		m_strName;	//!< ツリービューに表示する名前
	ConfigItemType		m_Type;
	BOOL				(*m_OpenDlg)(CConfigChildHolder*, HINSTANCE, HWND, SDlgConfigArg*, int);
	BOOL				(*m_CloseDlg)(CConfigChildHolder*);
	EFunctionCode		m_nHelpFuncId;
};


class CDlgConfig : public CDialog
{
public:
	CDlgConfig();
	~CDlgConfig();
	INT_PTR DoModal(HINSTANCE, HWND, CImageListMgr*, CMenuDrawer*, int, bool);	//!< モーダルダイアログの表示

	//! 子ダイアログを追加する
	void AddChildDlg(TCHAR* pszName,
							ConfigItemType Type,
							BOOL (*OpenDlg)(CConfigChildHolder*, HINSTANCE, HWND, SDlgConfigArg*, int),
							BOOL (*CloseDlg)(CConfigChildHolder*),
							EFunctionCode nFuncID);
	void SetDefaultChildDialog();
	void SetKeywordChildDialog();

	void ApplyData();
	int GetPageNum(){ return m_nItemSelectNum; }

	SDlgConfigArg* m_pDlgConfigArg;

protected:
	BOOL OnInitDialog(HWND, WPARAM, LPARAM);
	BOOL OnBnClicked(int wId);
	BOOL OnNotify(WPARAM wParam, LPARAM lParam);

	BOOL GetData();
	void SetData();

	void InitTreeView(HWND);
	void OpenChildDialog(const HTREEITEM);
	void CloseChildDialog();

	HWND							m_hwndTree;				//!< ツリービューハンドル
	std::vector< HTREEITEM >		m_hItemTypeName;		//!< タイプ名を格納するツリーアイテムハンドルのvector

	struct SItemAndDialog
	{
		HTREEITEM	hItem;	//!< ツリーアイテムハンドル
		BOOL (*OpenDlg)(CConfigChildHolder*, HINSTANCE, HWND, SDlgConfigArg*, int);	//!< 子ダイアログを開く
		BOOL (*CloseDlg)(CConfigChildHolder*);			//!< 子ダイアログを閉じる
		BOOL isOpened;		//!< 子ダイアログは開かれている=TRUE / 閉じられている=FALSE
		EFunctionCode	nFuncId;		//!< ヘルプID用FuncID
	};
	std::vector< SItemAndDialog > m_vecItemAndFunction;
	std::vector< SConfigChildDlgInfoStr > m_vecConfigChildDlg;	//!< 子ダイアログ情報
	CConfigChildHolder* m_pHolder;
	int m_nItemSelectNum;

private:
	// 未実装
	INT_PTR DoModal(HINSTANCE, HWND);
	HWND DoModeless(HINSTANCE, HWND, int, LPARAM, int);
	HWND DoModeless(HINSTANCE, HWND, LPCDLGTEMPLATE, LPARAM, int);
};

#endif
