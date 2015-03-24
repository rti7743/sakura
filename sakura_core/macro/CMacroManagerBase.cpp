/*!	@file
	@brief マクロエンジン

	@author genta
	@date 2002.4.29
*/
/*
	Copyright (C) 2002, genta

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
#include "StdAfx.h"
#include "CMacroManagerBase.h"
#include "view/CEditView.h"
#include "cmd/CViewCommander_inline.h"
#include "COpeBlk.h"
#include "macro/CMacro.h"

// CMacroBeforeAfter

void CMacroBeforeAfter::ExecKeyMacroBefore( class CEditView* pcEditView, int flags, CMacroExecVars* vars )
{
	COpeBlk* opeBlk = pcEditView->m_cCommander.GetOpeBlk();
	if( opeBlk ){
		vars->m_nOpeBlkCount = opeBlk->GetRefCount();
	}else{
		vars->m_nOpeBlkCount = 0;
	}
	vars->m_bDrawSwitchOld = pcEditView->GetDrawSwitch();
	vars->m_pcMacroInstanceDataOld = pcEditView->m_pcEditWnd->m_pcMacroInstanceData;
	pcEditView->m_pcEditWnd->m_pcMacroInstanceData = CMacro::CreateCMacroInstanceData();
	vars->m_bSaveResultParentOld = pcEditView->m_pcEditWnd->m_bSaveResultParamParent;
	pcEditView->m_pcEditWnd->m_bSaveResultParamParent = pcEditView->m_pcEditWnd->m_bSaveResultParam;
	pcEditView->m_pcEditWnd->m_bSaveResultParam = false;
}

void CMacroBeforeAfter::ExecKeyMacroAfter( class CEditView* pcEditView, int flags, bool bRet, CMacroExecVars* vars )
{
	if( 0 < vars->m_nOpeBlkCount ){
		COpeBlk* opeBlk = pcEditView->m_cCommander.GetOpeBlk();
		if( opeBlk == NULL ){
			pcEditView->m_cCommander.SetOpeBlk(new COpeBlk());
		}
		if( pcEditView->m_cCommander.GetOpeBlk()->GetRefCount() != vars->m_nOpeBlkCount ){
			pcEditView->m_cCommander.GetOpeBlk()->SetRefCount( vars->m_nOpeBlkCount );
		}
	}else{
		COpeBlk* opeBlk = pcEditView->m_cCommander.GetOpeBlk();
		if( opeBlk ){
			opeBlk->SetRefCount(1); // 強制的にリセットするため1を指定
			pcEditView->SetUndoBuffer();
		}
	}
	pcEditView->m_pcEditWnd->SetDrawSwitchOfAllViews(vars->m_bDrawSwitchOld);
	CMacro::DeleteCMacroInstanceData(pcEditView->m_pcEditWnd->m_pcMacroInstanceData);
	pcEditView->m_pcEditWnd->m_pcMacroInstanceData = vars->m_pcMacroInstanceDataOld;
	pcEditView->m_pcEditWnd->m_bSaveResultParam = pcEditView->m_pcEditWnd->m_bSaveResultParamParent;
	pcEditView->m_pcEditWnd->m_bSaveResultParamParent = vars->m_bSaveResultParentOld;
}

// CMacroManagerBase
//	デフォルトのコンストラクタ・デストラクタ

CMacroManagerBase::CMacroManagerBase()
 : m_nReady( false )
{}

CMacroManagerBase::~CMacroManagerBase()
{}

void CMacroManagerBase::ExecKeyMacro2( class CEditView* pcEditView, int flags )
{
	CMacroExecVars vars; // 再入用変数クラス
	ExecKeyMacroBefore(pcEditView, flags, &vars);
	bool b = ExecKeyMacro(pcEditView, flags);
	ExecKeyMacroAfter(pcEditView, flags, b, &vars);
}

