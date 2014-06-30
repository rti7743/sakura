#include "StdAfx.h"
#include "view/CEditView.h" // SColorStrategyInfo
#include "CFigure_CtrlCode.h"
#include "types/CTypeSupport.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CFigure_CtrlCode                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_CtrlCode::Match(const wchar_t* pText, int nTextLen) const
{
	//当面はASCII制御文字（C0 Controls, IsHankaku()で半角扱い）だけを制御文字表示にする
	//そうしないと IsHankaku(0x0600)==false なのに iswcntrl(0x0600)!=0 のようなケースで表示桁がずれる
	//U+0600: ARABIC NUMBER SIGN
#ifdef BUILD_OPT_ENALBE_PPFONT_SUPPORT
	if(WCODE::IsControlCode(pText[0])){
		return true;
	}
#else
	if(!(pText[0] & 0xFF80) && WCODE::IsControlCode(pText[0])){
		return true;
	}
#endif
	return false;
}

#ifdef BUILD_OPT_ENALBE_PPFONT_SUPPORT
bool CFigure_CtrlCode::DrawImp(SColorStrategyInfo* pInfo)
{
	bool bTrans = DrawImp_StyleSelect(pInfo);
	DispPos sPos(*pInfo->m_pDispPos);	// 現在位置を覚えておく
	int width = pInfo->m_pcView->GetTextMetrics().CalcTextWidth3(&pInfo->m_pLineOfLogic[pInfo->GetPosInLogic()], 1);
	DispSpaceEx(pInfo->m_gr, pInfo->m_pDispPos,pInfo->m_pcView, bTrans, width);	// 空白描画
	DrawImp_StylePop(pInfo);
	DrawImp_DrawUnderline(pInfo, sPos);
	// 1文字前提
	pInfo->m_nPosInLogic += CNativeW::GetSizeOfChar(	// 行末以外はここでスキャン位置を１字進める
		pInfo->m_pLineOfLogic,
		pInfo->GetDocLine()->GetLengthWithoutEOL(),
		pInfo->GetPosInLogic()
		);
	return true;
}

void CFigure_CtrlCode::DispSpaceEx(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans, int width) const
{
	int dx[1];
	dx[0] = width;

	RECT rc;
	//クリッピング矩形を計算。画面外なら描画しない
	if(pcView->GetTextArea().GenerateClipRect(&rc, *pDispPos, CHabaXInt(dx[0])))
	{
		//描画
		int fontNo = WCODE::GetFontNo(GetAlternateChar());
		if( fontNo ){
			SFONT sFont;
			sFont.m_sFontAttr = gr.GetCurrentMyFontAttr();
			sFont.m_hFont = pcView->GetFontset().ChooseFontHandle(fontNo, sFont.m_sFontAttr);
			gr.PushMyFont(sFont);
		}
		int nHeightMargin = pcView->GetTextMetrics().GetCharHeightMarginByFontNo(fontNo);
		wchar_t wc[1] = {GetAlternateChar()};
		ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y + nHeightMargin,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rc,
			wc,
			1,
			dx
		);
		if( fontNo ){
			gr.PopMyFont();
		}
	}
	pDispPos->ForwardDrawCol(CLayoutXInt(dx[0]));
}
#endif

#ifndef BUILD_OPT_ENALBE_PPFONT_SUPPORT
void CFigure_CtrlCode::DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const
{
	//クリッピング矩形を計算。画面外なら描画しない
	RECT rc;
	if(pcView->GetTextArea().GenerateClipRect(&rc,*pDispPos,CLayoutXInt(1)))
	{
		int nHeightMargin = pcView->GetTextMetrics().GetCharHeightMarginByFontNo(0);
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y + nHeightMargin,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rc,
			L"･",
			1,
			pcView->GetTextMetrics().GetDxArray_AllHankaku()
		);
	}

	//位置進める
	pDispPos->ForwardDrawCol(CLayoutXInt(1));
}
#endif

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CFigure_HanBinary                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_HanBinary::Match(const wchar_t* pText, int nTextLen) const
{
#ifndef BUILD_OPT_ENALBE_PPFONT_SUPPORT
	int nLen = pText[1]? 2:1;	// ※ pText は常に終端よりも手前
	if(CNativeW::GetKetaOfChar(pText, nLen, 0) == 1){	// 半角
		ECharSet e;
		CheckUtf16leChar(pText, nLen, &e, UC_NONCHARACTER);
		if(e == CHARSET_BINARY){
			return true;
		}
	}
#endif
	return false;
}

#ifndef BUILD_OPT_ENALBE_PPFONT_SUPPORT
void CFigure_HanBinary::DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const
{
	//クリッピング矩形を計算。画面外なら描画しない
	RECT rc;
	if(pcView->GetTextArea().GenerateClipRect(&rc,*pDispPos,CLayoutXInt(1)))
	{
		int fontNo = WCODE::GetFontNo(L'〓');
		int nHeightMargin = pcView->GetTextMetrics().GetCharHeightMarginByFontNo(fontNo);
		if( fontNo ){
			SFONT sFont;
			sFont.m_sFontAttr = gr.GetCurrentMyFontAttr();
			sFont.m_hFont = pcView->GetFontset().ChooseFontHandle(fontNo, sFont.m_sFontAttr);
			gr.PushMyFont(sFont);
		}
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y + nHeightMargin,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rc,
			L"〓",
			1,
			pcView->GetTextMetrics().GetDxArray_AllHankaku()
		);
		if( fontNo ){
			gr.PopMyFont();
		}
	}

	//位置進める
	pDispPos->ForwardDrawCol(CLayoutXInt(1));
}
#endif


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                     CFigure_ZenBinary                       //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

bool CFigure_ZenBinary::Match(const wchar_t* pText, int nTextLen) const
{
#ifndef BUILD_OPT_ENALBE_PPFONT_SUPPORT
	int nLen = pText[1]? 2:1;	// ※ pText は常に終端よりも手前
	if(CNativeW::GetKetaOfChar(pText, nLen, 0) > 1){	// 全角
		ECharSet e;
		CheckUtf16leChar(pText, nLen, &e, UC_NONCHARACTER);
		if(e == CHARSET_BINARY){
			return true;
		}
	}
#endif
	return false;
}

#ifndef BUILD_OPT_ENALBE_PPFONT_SUPPORT
void CFigure_ZenBinary::DispSpace(CGraphics& gr, DispPos* pDispPos, CEditView* pcView, bool bTrans) const
{
	//クリッピング矩形を計算。画面外なら描画しない
	RECT rc;
	if(pcView->GetTextArea().GenerateClipRect(&rc,*pDispPos,CLayoutXInt(2)))
	{
		int fontNo = WCODE::GetFontNo(L'〓');
		if( fontNo ){
			SFONT sFont;
			sFont.m_sFontAttr = gr.GetCurrentMyFontAttr();
			sFont.m_hFont = pcView->GetFontset().ChooseFontHandle(fontNo, sFont.m_sFontAttr);
			gr.PushMyFont(sFont);
		}
		int nHeightMargin = pcView->GetTextMetrics().GetCharHeightMarginByFontNo(fontNo);
		::ExtTextOutW_AnyBuild(
			gr,
			pDispPos->GetDrawPos().x,
			pDispPos->GetDrawPos().y + nHeightMargin,
			ExtTextOutOption() & ~(bTrans? ETO_OPAQUE: 0),
			&rc,
			L"〓",
			1,
			pcView->GetTextMetrics().GetDxArray_AllZenkaku()
		);
		if( fontNo ){
			gr.PopMyFont();
		}
	}

	//位置進める
	pDispPos->ForwardDrawCol(CLayoutXInt(2));
}
#endif
