#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CFigure_Comma.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "types/CTypeSupport.h"

void _DispTab( CGraphics& gr, DispPos* pDispPos, const CEditView* pcView );

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         CFigure_Comma                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_Comma::Match(const wchar_t* pText, int nTextLen) const
{
	if ( pText[0] == L',' && m_pTypeData->m_nTsvMode == TSV_MODE_CSV ) {
		return true;
	}
	return false;
}




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         描画実装                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*! カンマ描画
*/
void CFigure_Comma::DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const
{
	DispPos& sPos=*pDispPos;

	//必要なインターフェース
	const CTextMetrics* pMetrics=&pcView->GetTextMetrics();
	const CTextArea* pArea=&pcView->GetTextArea();

	int nLineHeight = pMetrics->GetHankakuDy();
	int nCharWidth = pMetrics->GetCharPxWidth();	// Layout→Px

	CTypeSupport cTabType(pcView,COLORIDX_TAB);

	// これから描画するタブ幅
	CLayoutXInt nCol;
	if( pcView->m_bMiniMap ){
		nCol = pcView->m_pcEditDoc->m_cLayoutMgr.GetActualTsvSpaceMiniMap( sPos.GetDrawCol(), L',' );
	}else
	{
		nCol = pcView->m_pcEditDoc->m_cLayoutMgr.GetActualTsvSpace( sPos.GetDrawCol(), L',' );
	}
	int tabDispWidth = (Int)nCol;

	// タブ記号領域
	RECT rcClip2;
	rcClip2.left = sPos.GetDrawPos().x;
	rcClip2.right = rcClip2.left + nCharWidth * tabDispWidth;
	if( rcClip2.left < pArea->GetAreaLeft() ){
		rcClip2.left = pArea->GetAreaLeft();
	}
	rcClip2.top = sPos.GetDrawPos().y;
	rcClip2.bottom = sPos.GetDrawPos().y + nLineHeight;

	if( pArea->IsRectIntersected(rcClip2) ){
		if( cTabType.IsDisp() ){	//CSVモード
			::ExtTextOutW_AnyBuild(
				gr,
				sPos.GetDrawPos().x,
				sPos.GetDrawPos().y,
				ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
				&rcClip2,
				L",       ",
				8, // tabDispWidth <= 8 ? tabDispWidth : 8, // Sep. 22, 2002 genta
				pMetrics->GetDxArray_AllHankaku()
			);
		}
	}

	//Xを進める
	sPos.ForwardDrawCol(nCol);
}

