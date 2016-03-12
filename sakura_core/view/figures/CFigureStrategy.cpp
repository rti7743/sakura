/*
	Copyright (C) 2008, kobake

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
#include "view/CEditView.h" // SColorStrategyInfo
#include "view/CViewFont.h"
#include "CFigureStrategy.h"
#include "doc/layout/CLayout.h"
#include "charset/charcode.h"
#include "types/CTypeSupport.h"

bool CFigure_Text::DrawImp(SColorStrategyInfo* pInfo)
{
	int nIdx = pInfo->GetPosInLogic();
	int nLength =	CNativeW::GetSizeOfChar(	// サロゲートペア対策	2008.10.12 ryoji
						pInfo->m_pLineOfLogic,
						pInfo->GetDocLine()->GetLengthWithoutEOL(),
						nIdx
					);
	bool bTrans = pInfo->m_pcView->IsBkBitmap() && CTypeSupport(pInfo->m_pcView, COLORIDX_TEXT).GetBackColor() == GetBkColor(pInfo->m_gr);
	int fontNo = (nLength == 2 ? WCODE::GetFontNo2(pInfo->m_pLineOfLogic[nIdx], pInfo->m_pLineOfLogic[nIdx+1]):
			WCODE::GetFontNo(pInfo->m_pLineOfLogic[nIdx]));
	if( fontNo ){
		CTypeSupport cCurrentType(pInfo->m_pcView, pInfo->GetCurrentColor());	// 周辺の色（現在の指定色/選択色）
		CTypeSupport cCurrentType2(pInfo->m_pcView, pInfo->GetCurrentColor2());	// 周辺の色（現在の指定色）
		bool blendColor = pInfo->GetCurrentColor() != pInfo->GetCurrentColor2() && cCurrentType.GetTextColor() == cCurrentType.GetBackColor();
		SFONT sFont;
		sFont.m_sFontAttr.m_bBoldFont  = (blendColor ? cCurrentType2.IsBoldFont() : cCurrentType.IsBoldFont());
		sFont.m_sFontAttr.m_bUnderLine = (blendColor ? cCurrentType2.HasUnderLine() : cCurrentType.HasUnderLine());
		sFont.m_hFont = pInfo->m_pcView->GetFontset().ChooseFontHandle(fontNo, sFont.m_sFontAttr);
		pInfo->m_gr.PushMyFont(sFont);
	}
	int nHeightMargin = pInfo->m_pcView->GetTextMetrics().GetCharHeightMarginByFontNo(fontNo);
	pInfo->m_pcView->GetTextDrawer().DispText(
		pInfo->m_gr,
		pInfo->m_pDispPos,
		nHeightMargin,
		&pInfo->m_pLineOfLogic[nIdx],
		nLength,
		bTrans
	);
	if( fontNo ){
		pInfo->m_gr.PopMyFont();
	}
	pInfo->m_nPosInLogic += nLength;
	return true;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         描画統合                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      CFigureSpace                           //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
bool CFigureSpace::DrawImp(SColorStrategyInfo* pInfo)
{
	bool bTrans = DrawImp_StyleSelect(pInfo);
	DispPos sPos(*pInfo->m_pDispPos);	// 現在位置を覚えておく
	DispSpace(pInfo->m_gr, pInfo->m_pDispPos,pInfo->m_pcView, bTrans);	// 空白描画
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

ColorInfoBase CFigureSpace::SetColorInfoFromMarker(SColorStrategyInfo* pInfo, const ColorInfoBase& colorBase, int nColorIdx)
{
	ColorInfoBase color = colorBase;
	if( pInfo->GetEnableMarker() && !(COLORIDX_SEARCH <= nColorIdx && nColorIdx <= COLORIDX_SEARCHTAIL) ){
		const CMarkerItem& marker = pInfo->GetMarkerItem();
		if( marker.IsBoldSet() ){
			color.m_sFontAttr.m_bBoldFont = marker.IsBold();
		}
		if( marker.IsUnderLineSet() ){
			color.m_sFontAttr.m_bUnderLine = marker.IsUnderLine();
		}
		if( marker.m_cTEXT != -1 ){
			color.m_sColorAttr.m_cTEXT = marker.m_cTEXT;
		}
		if( marker.m_cBACK != -1 ){
			color.m_sColorAttr.m_cBACK = marker.m_cBACK;
		}
	}
	return color;
}

bool CFigureSpace::DrawImp_StyleSelect(SColorStrategyInfo* pInfo)
{
	// この DrawImp はここ（基本クラス）でデフォルト動作を実装しているが
	// 仮想関数なので派生クラス側のオーバーライドで個別に仕様変更可能
	CEditView* pcView = pInfo->m_pcView;
	const int		nColorIdx = ToColorInfoArrIndex(pInfo->GetCurrentColor());
	const int		nColorIdx2 = ToColorInfoArrIndex(pInfo->GetCurrentColor2());
	const int		nColorIdxBg = ToColorInfoArrIndex(pInfo->GetCurrentColorBg());

	const ColorInfoBase& colorType = m_pTypeData->m_ColorInfoArr[nColorIdx];		// 周辺の色（現在の指定色/選択色）
	const ColorInfoBase& colorType2 = m_pTypeData->m_ColorInfoArr[nColorIdx2];	// 周辺の色（現在の指定色）
	const ColorInfoBase& colorText = m_pTypeData->m_ColorInfoArr[COLORIDX_TEXT];
	const ColorInfoBase& colorSpace = m_pTypeData->m_ColorInfoArr[GetDispColorIdx()];
	const ColorInfoBase& colorBg = m_pTypeData->m_ColorInfoArr[nColorIdxBg];
	const COLORREF crTextBack = colorText.GetBackColor();

	ColorInfoBase colorType3 = SetColorInfoFromMarker(pInfo, colorType2, nColorIdx2);
	const ColorInfoBase& colorType4 = (colorType3.GetBackColor() == crTextBack ? colorBg: colorType3);

	// 空白記号類は特に明示指定した部分以外はなるべく周辺の指定に合わせるようにしてみた	// 2009.05.30 ryoji
	// 例えば、下線を指定していない場合、正規表現キーワード内なら正規表現キーワード側の下線指定に従うほうが自然な気がする。
	// （そのほうが空白記号の「表示」をチェックしていない場合の表示に近い）
	//
	// 前景色・背景色の扱い
	// ・通常テキストとは異なる色が指定されている場合は空白記号の側の指定色を使う
	// ・通常テキストと同じ色が指定されている場合は周辺の色指定に合わせる
	// 太字の扱い
	// ・空白記号か周辺のどちらか一方でも太字指定されていれば「前景色・背景色の扱い」で決定した前景色で太字にする
	// 下線の扱い
	// ・空白記号で下線指定されていれば「前景色・背景色の扱い」で決定した前景色で下線を引く
	// ・空白記号で下線指定されておらず周辺で下線指定されていれば周辺の前景色で下線を引く
	// [選択]レンダリング中
	// ・混合色の場合は従来通り。
	COLORREF crText;
	COLORREF crBack;
	bool bBold;
	bool bItalic;
	const bool bNotSelecting = pInfo->GetCurrentColor() == pInfo->GetCurrentColor2();
	const bool blendColor = !bNotSelecting && colorType.GetTextColor() == colorType.GetBackColor(); // 選択混合色
	const bool bText = colorSpace.GetTextColor() == colorText.GetTextColor();
	const bool bBack = colorSpace.GetBackColor() == crTextBack;
	if( bNotSelecting ){
		// 通常色(未選択)
		crText = (bText ? colorType3 : colorSpace).GetTextColor();
		crBack = (bBack ? colorType4 : colorSpace).GetBackColor();
		bBold = colorType3.IsBoldFont();
		bItalic = colorType3.IsItalic();
	}else if( !blendColor ){
		// 指定選択色
		const ColorInfoBase& colorType5 = (colorType.GetBackColor() == crTextBack ? colorBg: colorType);
		crText = (bText ? colorType : colorSpace).GetTextColor();
		crBack = (bBack ? colorType5 : colorSpace).GetBackColor();
		bBold = colorType.IsBoldFont();
		bItalic = colorType.IsItalic();
	}else{
		// アルファor反転
		const ColorInfoBase& cText = (bText ? colorType3 : colorSpace);
		const ColorInfoBase& cBack = (bBack ? colorType4 : colorSpace);
		crText = pcView->GetTextColorByColorInfo2(colorType, cText);
		crBack = pcView->GetBackColorByColorInfo2(colorType, cBack);
		bBold = colorType3.IsBoldFont();
		bItalic = colorType3.IsItalic();
	}

	pInfo->m_gr.PushTextForeColor(crText);
	pInfo->m_gr.PushTextBackColor(crBack);
	// Figureが下線指定ならこちらで下線を指定。元の色のほうが下線指定なら、DrawImp_DrawUnderlineで下線だけ指定
	SFONT sFont;
	sFont.m_sFontAttr.m_bBoldFont = colorSpace.IsBoldFont() || bBold;
	sFont.m_sFontAttr.m_bUnderLine = colorSpace.HasUnderLine();
	sFont.m_sFontAttr.m_bItalic = colorSpace.IsItalic() || bItalic;
	sFont.m_sFontAttr.m_bStrikeOut = colorSpace.IsStrikeOut();
	sFont.m_hFont = pInfo->m_pcView->GetFontset().ChooseFontHandle( 0, sFont.m_sFontAttr );
	pInfo->m_gr.PushMyFont(sFont);
	bool bTrans = pcView->IsBkBitmap() && colorText.GetBackColor() == crBack;
	return bTrans;
}

void CFigureSpace::DrawImp_StylePop(SColorStrategyInfo* pInfo)
{
	pInfo->m_gr.PopTextForeColor();
	pInfo->m_gr.PopTextBackColor();
	pInfo->m_gr.PopMyFont();
}

void CFigureSpace::DrawImp_DrawUnderline(SColorStrategyInfo* pInfo, DispPos& sPos)
{
	CEditView* pcView = pInfo->m_pcView;

	const int		nColorIdx = ToColorInfoArrIndex(pInfo->GetCurrentColor());
	const int		nColorIdx2 = ToColorInfoArrIndex(pInfo->GetCurrentColor2());
	const ColorInfoBase& colorType = m_pTypeData->m_ColorInfoArr[nColorIdx];
	const ColorInfoBase& colorType2 = m_pTypeData->m_ColorInfoArr[nColorIdx2];

	ColorInfoBase colorType3 = SetColorInfoFromMarker(pInfo, colorType2, nColorIdx2);

	// 指定選択色
	bool bSelectColor = (pInfo->GetCurrentColor() != pInfo->GetCurrentColor2() && colorType.GetTextColor() != colorType.GetBackColor());

	const ColorInfoBase& colorStyle = (bSelectColor ? colorType : colorType3);

	CTypeSupport cSpaceType(pcView, GetDispColorIdx());	// 空白の指定色

	if( (!cSpaceType.HasUnderLine() && colorStyle.HasUnderLine())
	 || (!cSpaceType.IsStrikeOut() && colorStyle.IsStrikeOut())
	)
	{
		int fontNo = WCODE::GetFontNo(' ');
		// 下線を周辺の前景色で描画する
		SFONT sFont;
		sFont.m_sFontAttr.m_bBoldFont = false;
		sFont.m_sFontAttr.m_bUnderLine = colorStyle.HasUnderLine();
		sFont.m_sFontAttr.m_bItalic = false;
		sFont.m_sFontAttr.m_bStrikeOut = colorStyle.IsStrikeOut();
		sFont.m_hFont = pInfo->m_pcView->GetFontset().ChooseFontHandle( fontNo, sFont.m_sFontAttr );
		pInfo->m_gr.PushMyFont(sFont);

		int nHeightMargin = pInfo->m_pcView->GetTextMetrics().GetCharHeightMarginByFontNo(fontNo);
		CLayoutXInt nColLength = CLayoutXInt(pInfo->m_pDispPos->GetDrawCol() - sPos.GetDrawCol());
		int nSpWidth = pcView->GetTextMetrics().CalcTextWidth3(L" ", 1);
		int nLength = (Int)(nColLength + nSpWidth - 1) / nSpWidth;
		wchar_t* pszText = new wchar_t[nLength];
		std::vector<int> vDxArray(nLength);
		for( int i = 0; i < nLength; i++ ){
			pszText[i] = L' ';
			vDxArray[i] = nSpWidth;
		}
		RECT rcClip2;
		rcClip2.left = sPos.GetDrawPos().x;
		rcClip2.right = rcClip2.left + (Int)(nColLength); // 前提条件：CLayoutInt == px
		if( rcClip2.left < pcView->GetTextArea().GetAreaLeft() ){
			rcClip2.left = pcView->GetTextArea().GetAreaLeft();
		}
		rcClip2.top = sPos.GetDrawPos().y;
		rcClip2.bottom = sPos.GetDrawPos().y + sPos.GetCharHeight();
		::ExtTextOutW_AnyBuild(
			pInfo->m_gr,
			sPos.GetDrawPos().x,
			sPos.GetDrawPos().y + nHeightMargin,
			ExtTextOutOption() & ~(ETO_OPAQUE),
			&rcClip2,
			pszText,
			nLength,
			&vDxArray[0]
		);
		delete []pszText;
		pInfo->m_gr.PopMyFont();
	}
}

