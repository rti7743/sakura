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
#include "CEditView_Paint.h"
#include "window/CEditWnd.h"
#include "doc/CEditDoc.h"
#include "types/CTypeSupport.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           括弧                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	@date 2003/02/18 ai
	@param flag [in] モード(true:登録, false:解除)
*/
void CEditView::SetBracketPairPos( bool flag )
{
	int	mode;

	// 03/03/06 ai すべて置換、すべて置換後のUndo&Redoがかなり遅い問題に対応
	if( m_bDoing_UndoRedo || !GetDrawSwitch() ){
		return;
	}

	if( !m_pTypeData->m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp ){
		return;
	}

	// 対括弧の検索&登録
	/*
	bit0(in)  : 表示領域外を調べるか？ 0:調べない  1:調べる
	bit1(in)  : 前方文字を調べるか？   0:調べない  1:調べる
	bit2(out) : 見つかった位置         0:後ろ      1:前
	*/
	mode = 2;

	CLayoutPoint ptColLine;

	if( flag && !GetSelectionInfo().IsTextSelected() && !GetSelectionInfo().m_bDrawSelectArea
		&& SearchBracket( GetCaret().GetCaretLayoutPos(), &ptColLine, &mode ) )
	{
		// 登録指定(flag=true)			&&
		// テキストが選択されていない	&&
		// 選択範囲を描画していない		&&
		// 対応する括弧が見つかった		場合
		if ( ( ptColLine.x >= GetTextArea().GetViewLeftCol() ) && ( ptColLine.x <= GetTextArea().GetRightCol() )
			&& ( ptColLine.y >= GetTextArea().GetViewTopLine() ) && ( ptColLine.y <= GetTextArea().GetBottomLine() ) )
		{
			// 表示領域内の場合

			// レイアウト位置から物理位置へ変換(強調表示位置を登録)
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic( ptColLine, &m_ptBracketPairPos_PHY );
			m_ptBracketCaretPos_PHY.y = GetCaret().GetCaretLogicPos().y;
			if( 0 == ( mode & 4 ) ){
				// カーソルの後方文字位置
				m_ptBracketCaretPos_PHY.x = GetCaret().GetCaretLogicPos().x;
			}else{
				// カーソルの前方文字位置
				m_ptBracketCaretPos_PHY.x = GetCaret().GetCaretLogicPos().x - 1;
			}
			return;
		}
	}

	// 括弧の強調表示位置情報初期化
	m_ptBracketPairPos_PHY.Set(CLogicInt(-1), CLogicInt(-1));
	m_ptBracketCaretPos_PHY.Set(CLogicInt(-1), CLogicInt(-1));

	return;
}

/*!
	対括弧の強調表示
	@date 2002/09/18 ai
	@date 2003/02/18 ai 再描画対応の為大改造
*/
void CEditView::DrawBracketPair( bool bDraw )
{
	// 03/03/06 ai すべて置換、すべて置換後のUndo&Redoがかなり遅い問題に対応
	if( m_bDoing_UndoRedo || !GetDrawSwitch() ){
		return;
	}

	if( !m_pTypeData->m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp ){
		return;
	}

	// 括弧の強調表示位置が未登録の場合は終了
	if( m_ptBracketPairPos_PHY.HasNegative() || m_ptBracketCaretPos_PHY.HasNegative() ){
		return;
	}

	// 描画指定(bDraw=true)				かつ
	// ( テキストが選択されている		又は
	//   選択範囲を描画している			又は
	//   フォーカスを持っていない		又は
	//   アクティブなペインではない )	場合は終了
	if( bDraw
	 &&( GetSelectionInfo().IsTextSelected() || GetSelectionInfo().m_bDrawSelectArea || !m_bDrawBracketPairFlag
	 || ( m_pcEditWnd->GetActivePane() != m_nMyIndex ) ) ){
		return;
	}

	CGraphics gr;
	gr.Init(::GetDC(GetHwnd()));
	bool bCaretChange = false;
	gr.SetTextBackTransparent(true);

	for( int i = 0; i < 2; i++ )
	{
		// i=0:対括弧,i=1:カーソル位置の括弧
		// 2011.11.23 ryoji 対括弧 -> カーソル位置の括弧 の順に処理順序を変更
		//   ＃ { と } が異なる行にある場合に { を BS で消すと } の強調表示が解除されない問題（Wiki BugReport/89）の対策
		//   ＃ この順序変更によりカーソル位置が括弧でなくなっていても対括弧があれば対括弧側の強調表示は解除される

		CLayoutPoint	ptColLine;

		if( i == 0 ){
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout( m_ptBracketPairPos_PHY,  &ptColLine );
		}else{
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout( m_ptBracketCaretPos_PHY, &ptColLine );
		}

		if ( ( ptColLine.x >= GetTextArea().GetViewLeftCol() ) && ( ptColLine.x <= GetTextArea().GetRightCol() )
			&& ( ptColLine.y >= GetTextArea().GetViewTopLine() ) && ( ptColLine.y <= GetTextArea().GetBottomLine() ) )
		{	// 表示領域内の場合
			if( !bDraw && GetSelectionInfo().m_bDrawSelectArea && ( 0 == IsCurrentPositionSelected( ptColLine ) ) )
			{	// 選択範囲描画済みで消去対象の括弧が選択範囲内の場合
				continue;
			}
			const CLayout* pcLayout;
			CLogicInt		nLineLen;
			const wchar_t*	pLine = m_pcEditDoc->m_cLayoutMgr.GetLineStr( ptColLine.GetY2(), &nLineLen, &pcLayout );
			if( pLine )
			{
				CColor3Setting cColor;
				SColorStrategyInfo sInfo_;
				SColorStrategyInfo* pInfo = &sInfo_;
				cColor.InitMarker();
				CLogicInt	OutputX = LineColumnToIndex( pcLayout, ptColLine.GetX2() );
				if( bDraw ) {
					cColor.eColorIndex = COLORIDX_BRACKET_PAIR;
					cColor.eColorIndex2 = COLORIDX_BRACKET_PAIR;
				}
				else{
					if( IsBracket( pLine, OutputX, CLogicInt(1) ) ){
						DispPos _sPos(0,0); // 注意：この値はダミー。CheckChangeColorでの参照位置は不正確
						pInfo->m_pDispPos = &_sPos;
						pInfo->m_pcView = this;

						// 03/10/24 ai 折り返し行のColorIndexが正しく取得できない問題に対応
						// 2009.02.07 ryoji GetColorIndex に渡すインデックスの仕様変更（元はこっちの仕様だった模様）
						cColor = GetColorIndex( pcLayout, ptColLine.GetY2(), OutputX, pInfo );
					}
					else{
						SetBracketPairPos( false );
						break;
					}
				}
				CTypeSupport    cCuretLineBg(this,COLORIDX_CARETLINEBG);
				EColorIndexType nColorIndexBg = (cCuretLineBg.IsDisp() && ptColLine.GetY2() == GetCaret().GetCaretLayoutPos().GetY2()
					? COLORIDX_CARETLINEBG
					: CTypeSupport(this,COLORIDX_EVENLINEBG).IsDisp() && ptColLine.GetY2() % 2 == 1
						? COLORIDX_EVENLINEBG
						: COLORIDX_TEXT);
				// 03/03/03 ai カーソルの左に括弧があり括弧が強調表示されている状態でShift+←で選択開始すると
				//             選択範囲内に反転表示されない部分がある問題の修正
				CLayoutInt caretX = GetCaret().GetCaretLayoutPos().GetX2();
				bool bCaretHide = (!bCaretChange && (ptColLine.x == caretX || ptColLine.x + 1 == caretX) && GetCaret().GetCaretShowFlag());
				if( bCaretHide ){
					bCaretChange = true;
					GetCaret().HideCaret_( GetHwnd() );	// キャレットが一瞬消えるのを防止
				}
				{
					int nWidth  = GetTextMetrics().GetCharPxWidth();
					int nHeight = GetTextMetrics().GetHankakuDy();
					int nLeft = (GetTextArea().GetDocumentLeftClientPointX()) + GetTextMetrics().GetCharPxWidth(ptColLine.x);
					int nTop  = (Int)( ptColLine.GetY2() - GetTextArea().GetViewTopLine() ) * nHeight + GetTextArea().GetAreaTop();
					CLayoutXInt charsWidth = m_pcEditDoc->m_cLayoutMgr.GetLayoutXOfChar(pLine, nLineLen, OutputX);

					//色設定
					CTypeSupport cTextType(this,COLORIDX_TEXT);
					// 2013.05.24 背景色がテキストの背景色と同じならカーソル行の背景色を適用
					CTypeSupport cColorIndexType(this,cColor.eColorIndex);
					COLORREF  crBack = cColorIndexType.GetBackColor();
					if( bDraw ){
						cColor.eColorIndexBg = nColorIndexBg;
					}
					SetCurrentColor( gr, cColor );
					bool bTrans = false;
					// DEBUG_TRACE( _T("DrawBracket %d %d ") , ptColLine.y, ptColLine.x );
					if( IsBkBitmap() &&
							cTextType.GetBackColor() == gr.GetTextBackColor() ){
						bTrans = true;
						RECT rcChar;
						rcChar.left  = nLeft;
						rcChar.top = nTop;
						rcChar.right = nLeft + GetTextMetrics().GetCharPxWidth(charsWidth);
						rcChar.bottom = nTop + nHeight;
						HDC hdcBgImg = ::CreateCompatibleDC(gr);
						HBITMAP hBmpOld = (HBITMAP)::SelectObject(hdcBgImg, m_pcEditDoc->m_hBackImg);
						DrawBackImage(gr, rcChar, hdcBgImg);
						::SelectObject(hdcBgImg, hBmpOld);
						::DeleteDC(hdcBgImg);
					}
					DispPos sPos(nWidth, nHeight);
					sPos.InitDrawPos(CMyPoint(nLeft, nTop));
					GetTextDrawer().DispText(gr, &sPos, 0, &pLine[OutputX], 1, bTrans);
					GetTextDrawer().DispNoteLine(gr, nTop, nTop + nHeight, nLeft, nLeft + (Int)charsWidth * nWidth);
					// 2006.04.30 Moca 対括弧の縦線対応
					GetTextDrawer().DispVerticalLines(gr, nTop, nTop + nHeight, ptColLine.x, ptColLine.x + charsWidth); //※括弧が全角幅である場合を考慮
				}

				if( ( m_pcEditWnd->GetActivePane() == m_nMyIndex )
					&& ( ( ptColLine.y == GetCaret().GetCaretLayoutPos().GetY() ) || ( ptColLine.y - 1 == GetCaret().GetCaretLayoutPos().GetY() ) ) ){	// 03/02/27 ai 行の間隔が"0"の時にアンダーラインが欠ける事がある為修正
					GetCaret().m_cUnderLine.CaretUnderLineON( true, false );
				}
			}
		}
	}
	if( bCaretChange ){
		GetCaret().ShowCaret_( GetHwnd() );	// キャレットが一瞬消えるのを防止
	}

	::ReleaseDC( GetHwnd(), gr );
}


//======================================================================
//!対括弧の対応表
//2007.10.16 kobake
struct KAKKO_T{
	const wchar_t *sStr;
	const wchar_t *eStr;
};
static const KAKKO_T g_aKakkos[] = {
	//半角
	{ L"(", L")", },
	{ L"[", L"]", },
	{ L"{", L"}", },
	{ L"<", L">", },
	{ L"｢", L"｣", },
	//全角
	{ L"【", L"】", },
	{ L"『", L"』", },
	{ L"「", L"」", },
	{ L"＜", L"＞", },
	{ L"≪", L"≫", },
	{ L"《", L"》", },
	{ L"（", L"）", },
	{ L"〈", L"〉", },
	{ L"｛", L"｝", },
	{ L"〔", L"〕", },
	{ L"［", L"］", },
	{ L"“", L"”", },
	{ L"〝", L"〟", },
	//終端
	{ NULL, NULL, },
};



//	Jun. 16, 2000 genta
/*!
	@brief 対括弧の検索

	カーソル位置の括弧に対応する括弧を探す。カーソル位置が括弧でない場合は
	カーソルの後ろの文字が括弧かどうかを調べる。

	カーソルの前後いずれもが括弧でない場合は何もしない。

	括弧が半角か全角か、及び始まりか終わりかによってこれに続く4つの関数に
	制御を移す。

	@param ptLayout [in] 検索開始点の物理座標
	@param pptLayoutNew [out] 移動先のレイアウト座標
	@param mode [in,out] bit0(in)  : 表示領域外を調べるか？ 0:調べない  1:調べる
						 bit1(in)  : 前方文字を調べるか？   0:調べない  1:調べる (このbitを参照)
						 bit2(out) : 見つかった位置         0:後ろ      1:前     (このbitを更新)

	@retval true 成功
	@retval false 失敗

	@author genta
	@date Jun. 16, 2000 genta
	@date Feb. 03, 2001 MIK 全角括弧に対応
	@date Sep. 18, 2002 ai modeの追加
*/
bool CEditView::SearchBracket(
	const CLayoutPoint&	ptLayout,
	CLayoutPoint*		pptLayoutNew,
	int*				mode
)
{
	CLogicInt len;	//	行の長さ

	CLogicPoint ptPos;

	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic( ptLayout, &ptPos );
	const wchar_t *cline = m_pcEditDoc->m_cDocLineMgr.GetLine(ptPos.GetY2())->GetDocLineStrWithEOL(&len);

	//	Jun. 19, 2000 genta
	if( cline == NULL )	//	最後の行に本文がない場合
		return false;

	// 括弧処理 2007.10.16 kobake
	{
		const KAKKO_T* p;
		for( p = g_aKakkos; p->sStr != NULL;  p++ )
		{
			if( wcsncmp(p->sStr, &cline[ptPos.x], 1) == 0 )
			{
				return SearchBracketForward( ptPos, pptLayoutNew, p->sStr, p->eStr, mode );
			}
			else if( wcsncmp(p->eStr, &cline[ptPos.x], 1) == 0 )
			{
				return SearchBracketBackward( ptPos, pptLayoutNew, p->sStr, p->eStr, mode );
			}
		}
	}

	// 02/09/18 ai Start
	if( 0 == ( *mode & 2 ) ){
		/* カーソルの前方を調べない場合 */
		return false;
	}
	*mode |= 4;
	// 02/09/18 ai End

	//	括弧が見つからなかったら，カーソルの直前の文字を調べる

	if( ptPos.x <= 0 ){
		return false;	//	前の文字はない
	}

	const wchar_t *bPos = CNativeW::GetCharPrev( cline, ptPos.x, cline + ptPos.x );
	int nCharSize = cline + ptPos.x - bPos;
	// 括弧処理 2007.10.16 kobake
	if(nCharSize==1){
		const KAKKO_T* p;
		ptPos.x = bPos - cline;
		for( p = g_aKakkos; p->sStr != NULL; p++ )
		{
			if( wcsncmp(p->sStr, &cline[ptPos.x], 1) == 0 )
			{
				return SearchBracketForward( ptPos, pptLayoutNew, p->sStr, p->eStr, mode );
			}
			else if( wcsncmp(p->eStr, &cline[ptPos.x], 1) == 0 )
			{
				return SearchBracketBackward( ptPos, pptLayoutNew, p->sStr, p->eStr, mode );
			}
		}
	}

	return false;
}

/*!
	@brief 半角対括弧の検索:順方向

	@author genta

	@param ptLayout [in] 検索開始点の物理座標
	@param pptLayoutNew [out] 移動先のレイアウト座標
	@param upChar [in] 括弧の始まりの文字
	@param dnChar [in] 括弧を閉じる文字列
	@param mode   [in] bit0(in)  : 表示領域外を調べるか？ 0:調べない  1:調べる (このbitを参照)
					 bit1(in)  : 前方文字を調べるか？   0:調べない  1:調べる
					 bit2(out) : 見つかった位置         0:後ろ      1:前

	@retval true 成功
	@retval false 失敗
*/
// 03/01/08 ai
bool CEditView::SearchBracketForward(
	CLogicPoint		ptPos,
	CLayoutPoint*	pptLayoutNew,
	const wchar_t*	upChar,
	const wchar_t*	dnChar,
	int*			mode
)
{
	const CDocLine* ci;

	int			len;
	const wchar_t* cPos;
	const wchar_t* nPos;
	const wchar_t* cline;
	const wchar_t* lineend;
	int			level = 0;

	CLayoutPoint ptColLine;

	CLayoutInt	nSearchNum;	// 02/09/19 ai

	//	初期位置の設定
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, &ptColLine );	// 02/09/19 ai
	nSearchNum = ( GetTextArea().GetBottomLine() ) - ptColLine.y;					// 02/09/19 ai
	ci = m_pcEditDoc->m_cDocLineMgr.GetLine( ptPos.GetY2() );
	cline = ci->GetDocLineStrWithEOL( &len );
	lineend = cline + len;
	cPos = cline + ptPos.x;

	do {
		while( cPos < lineend ){
			nPos = CNativeW::GetCharNext( cline, len, cPos );
			if( nPos - cPos > 1 ){
				//	skip
				cPos = nPos;
				continue;
			}
			// 03/01/08 ai Start
			if( wcsncmp(upChar, cPos, 1) == 0 ){
				++level;
			}
			else if( wcsncmp(dnChar, cPos, 1) == 0 ){
				--level;
			}// 03/01/08 ai End

			if( level == 0 ){	//	見つかった！
				ptPos.x = cPos - cline;
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, pptLayoutNew );
				return true;
				//	Happy Ending
			}
			cPos = nPos;	//	次の文字へ
		}

		// 02/09/19 ai Start
		nSearchNum--;
		if( ( 0 > nSearchNum ) && ( 0 == (*mode & 1 ) ) )
		{	// 表示領域外を調べないモードで表示領域の終端の場合
			//SendStatusMessage( "対括弧の検索を中断しました" );
			break;
		}
		// 02/09/19 ai End

		//	次の行へ
		ptPos.y++;
		ci = ci->GetNextLine();	//	次のアイテム
		if( ci == NULL )
			break;	//	終わりに達した

		cline = ci->GetDocLineStrWithEOL( &len );
		cPos = cline;
		lineend = cline + len;
	}while( cline != NULL );

	return false;
}

/*!
	@brief 半角対括弧の検索:逆方向

	@author genta

	@param ptLayout [in] 検索開始点の物理座標
	@param pptLayoutNew [out] 移動先のレイアウト座標
	@param upChar [in] 括弧の始まりの文字
	@param dnChar [in] 括弧を閉じる文字列
	@param mode [in] bit0(in)  : 表示領域外を調べるか？ 0:調べない  1:調べる (このbitを参照)
					 bit1(in)  : 前方文字を調べるか？   0:調べない  1:調べる
					 bit2(out) : 見つかった位置         0:後ろ      1:前

	@retval true 成功
	@retval false 失敗
*/
bool CEditView::SearchBracketBackward(
	CLogicPoint		ptPos,
	CLayoutPoint*	pptLayoutNew,
	const wchar_t*	dnChar,
	const wchar_t*	upChar,
	int*			mode
)
{
	const CDocLine* ci;

	int			len;
	const wchar_t* cPos;
	const wchar_t* pPos;
	const wchar_t* cline;
	int			level = 1;
	
	CLayoutPoint ptColLine;

	CLayoutInt		nSearchNum;	// 02/09/19 ai

	//	初期位置の設定
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, &ptColLine );	// 02/09/19 ai
	nSearchNum = ptColLine.y - GetTextArea().GetViewTopLine();										// 02/09/19 ai
	ci = m_pcEditDoc->m_cDocLineMgr.GetLine( ptPos.GetY2() );
	cline = ci->GetDocLineStrWithEOL( &len );
	cPos = cline + ptPos.x;

	do {
		while( cPos > cline ){
			pPos = CNativeW::GetCharPrev( cline, len, cPos );
			if( cPos - pPos > 1 ){
				//	skip
				cPos = pPos;
				continue;
			}
			// 03/01/08 ai Start
			if( wcsncmp(upChar, pPos, 1) == 0 ){
				++level;
			}
			else if( wcsncmp(dnChar, pPos, 1) == 0 ){
				--level;
			}// 03/01/08 ai End

			if( level == 0 ){	//	見つかった！
				ptPos.x = pPos - cline;
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, pptLayoutNew );
				return true;
				//	Happy Ending
			}
			cPos = pPos;	//	次の文字へ
		}

		// 02/09/19 ai Start
		nSearchNum--;
		if( ( 0 > nSearchNum ) && ( 0 == (*mode & 1 ) ) )
		{	// 表示領域外を調べないモードで表示領域の先頭の場合
			//SendStatusMessage( "対括弧の検索を中断しました" );
			break;
		}
		// 02/09/19 ai End

		//	次の行へ
		ptPos.y--;
		ci = ci->GetPrevLine();	//	次のアイテム
		if( ci == NULL )
			break;	//	終わりに達した

		cline = ci->GetDocLineStrWithEOL( &len );
		cPos = cline + len;
	}while( cline != NULL );

	return false;
}

//@@@ 2003.01.09 Start by ai:
/*!
	@brief 括弧判定

	@author ai

	@param pLine [in] 
	@param x
	@param size

	@retval true 括弧
	@retval false 非括弧
*/
bool CEditView::IsBracket( const wchar_t *pLine, CLogicInt x, CLogicInt size )
{
	// 括弧処理 2007.10.16 kobake
	if( size == 1 ){
		const KAKKO_T *p;
		for( p = g_aKakkos; p->sStr != NULL; p++ )
		{
			if( wcsncmp( p->sStr, &pLine[x], 1 ) == 0 )
			{
				return true;
			}
			else if( wcsncmp( p->eStr, &pLine[x], 1 ) == 0 )
			{
				return true;
			}
		}
	}

	return false;
}
//@@@ 2003.01.09 End

/*!
	@brief 対HTMLタグの検索

	カーソル位置のHTMLタグに対応するHTMLタグを探す。カーソル位置がHTMLタグでない場合は
	カーソルの後ろの文字がHTMLタグかどうかを調べる。

	カーソルの前後いずれもがHTMLタグでない場合は何もしない。

	@param ptLayout [in] 検索開始点の物理座標
	@param pptLayoutNew [out] 移動先のレイアウト座標
	@param mode [in,out] bit0(in)  : 表示領域外を調べるか？ 0:調べない  1:調べる
						 bit1(in)  : 前方文字を調べるか？   0:調べない  1:調べる (このbitを参照)
						 bit2(out) : 見つかった位置         0:後ろ      1:前     (このbitを更新)

	@retval true 成功
	@retval false 失敗

	@author rti7743
	@date Dec. 25, 2016 rti7743
*/
bool CEditView::SearchHTMLTag(
	const CLayoutPoint&	ptLayout,
	CLayoutPoint*		pptLayoutNew,
	int*				mode
)
{
	CLogicInt len;	//	行の長さ

	CLogicPoint ptPos;

	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic( ptLayout, &ptPos );
	const wchar_t *cline = m_pcEditDoc->m_cDocLineMgr.GetLine(ptPos.GetY2())->GetDocLineStrWithEOL(&len);

	//	Jun. 19, 2000 genta
	if( cline == NULL )	//	最後の行に本文がない場合
		return false;

	//HTMLタグ検索
	//2016/12/24 rti7743
	wchar_t html_tag[64];
	int found_html_tag = getHTMLTagName( cline , ptPos , html_tag , 64);
	if(found_html_tag == 0)
	{
		return false;
	}

	//閉じタグいらないタグだったら無視.
	static const wchar_t* aSkipTags[] = {
		 L"BR"
		,L"IMG"
		,L"HR"
		,L"META"
		,L"INPUT"
		,L"EMBED"
		,L"AREA"
		,L"BASE"
		,L"COL"
		,L"KEYGEN"
//		,L"LINK" //RSSなどのxmlで使われることがあるので外すか
		,L"PARAM"
		,L"SOURCE"
		//終端
		,NULL
	};
	for( int i = 0; aSkipTags[i] ; i++ )
	{
		if( wcscmp(html_tag,aSkipTags[i]) == 0 )
		{//閉じタグ不要の検索禁止タグ
			return false;
		}
	}

	if( found_html_tag == 1 )
	{
		return SearchHTMLTagForward( ptPos, pptLayoutNew, html_tag, mode );
	}
	else if( found_html_tag == 2 )
	{
		return SearchHTMLTagBackward( ptPos, pptLayoutNew, html_tag, mode );
	}

	return false;
}

/*!
	@brief 現在位置にあるHTMLタグを取得

	@author rti7743

	@param cline [in] 現在の行データ
	@param ptPos [in] 移動の座標
	@param out_html_tag[out] タグ名 DIV等が入る. 必ず大文字で格納
	@param max_html_tag_size [in] out_html_tagバッファのサイズ

	@retval 0 タグはありません
	@retval 1 開始タグ
	@retval 2 閉じタグ
*/
int CEditView::getHTMLTagName( const wchar_t *cline ,const CLogicPoint& ptPos , wchar_t *out_html_tag, int max_html_tag_size)
{
	int x = ptPos.x;
	for( ; x >= 0 ; x--)
	{
		if( cline[x] == L'<' )
		{
			break;
		}
	}
	if(x < 0)
	{//タグではない
		return 0;
	}
	x++;//skip < tag

	//タグ名の取得
	bool isCloseTag = false;
	if( cline[x] == L'/' )
	{//閉じタグ
		isCloseTag = true;
		x++; // skip /
	}

	int x2 = x;
	for( ; cline[x2] ; x2++ )
	{
		if(cline[x2] == L' '
		 || cline[x2] == L'\t'
		 || cline[x2] == L'/'
		 || cline[x2] == L'>' 
		 || cline[x2] == L'\n' 
		 || cline[x2] == L'\r' 
		)
		{
			break;
		}
	}

	const int size = x2-x;
	if(size+1 >= max_html_tag_size)
	{//タグの長さが長すぎる
		return 0;
	}
	//タグ名を大文字に変換して格納.
	for(int i = 0 ; i < size ; i++)
	{
		wchar_t c = cline[x+i];
		if(c >= L'a' && c <= L'z')
		{
			c = c - L'a' + L'A'; //強制大文字
		}
		out_html_tag[i] = c;
	}
	out_html_tag[size] = 0; //null終端

	if(isCloseTag)
	{
		return 2; //閉じタグ
	}
	return 1; //開始タグ
}



/*!
	@brief HTMLタグの検索:順方向

	@author rti7743

	@param ptPos [in] 検索開始点の物理座標
	@param pptLayoutNew [out] 移動先のレイアウト座標
	@param tagname[in] 括弧の始まりの文字
	@param mode   [in] bit0(in)  : 表示領域外を調べるか？ 0:調べない  1:調べる (このbitを参照)
					 bit1(in)  : 前方文字を調べるか？   0:調べない  1:調べる
					 bit2(out) : 見つかった位置         0:後ろ      1:前

	@retval true 成功
	@retval false 失敗
*/
bool CEditView::SearchHTMLTagForward(
	CLogicPoint		ptPos,
	CLayoutPoint*	pptLayoutNew,
	const wchar_t*	tagname,
	int*			mode
)
{
	const CDocLine* ci;

	int			len;
	const wchar_t* cPos;
	const wchar_t* nPos;
	const wchar_t* cline;
	const wchar_t* lineend;
	int			level = 1;

	CLayoutPoint ptColLine;
	CLayoutInt	nSearchNum;

	//	初期位置の設定
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, &ptColLine );
	nSearchNum = ( GetTextArea().GetBottomLine() ) - ptColLine.y;
	ci = m_pcEditDoc->m_cDocLineMgr.GetLine( ptPos.GetY2() );
	cline = ci->GetDocLineStrWithEOL( &len );
	lineend = cline + len;
	cPos = cline + ptPos.x + 1;

	enum state_need
	{
		 state_need_start_braket   // <
		,state_need_if_slash       // /
		,state_need_tagname        // div
		,state_need_end_braket     // >
	};
	state_need state = state_need_start_braket;

	enum quote_skip{
		 quote_skip_none
		,quote_skip_dquote			//"
		,quote_skip_escape_dquote	//\"
		,quote_skip_htmlcomment_s1	//<! 
		,quote_skip_htmlcomment_s2	//<!- 
		,quote_skip_htmlcomment_s3	//<!--
		,quote_skip_htmlcomment_e1	//
		,quote_skip_htmlcomment_e2	//
		,quote_skip_htmlcomment_e3	//-->
	};
	quote_skip qskip = quote_skip_none;

	bool isCLoseTag = false;
	int tag_match_pos = 0;
	wchar_t lastPos = 0;

	do {
		for( ; cPos < lineend ; cPos = nPos){
			nPos = CNativeW::GetCharNext( cline, len, cPos );
			if( nPos - cPos > 1 ){
				continue; //	skip
			}

			//クウォートの読み飛ばし.

			if(qskip == quote_skip_none){
				if(*cPos == L'"'){
					qskip = quote_skip_dquote;
					continue;
				}
			}
			else if(qskip == quote_skip_dquote){ //" " を読み飛ばす
				if(*cPos == L'\\'){
					qskip = quote_skip_escape_dquote;
				}
				else if(*cPos == L'"'){
					qskip = quote_skip_none;
				}
				continue;
			}
			else if(qskip == quote_skip_escape_dquote){ // \" を読み飛ばす
				qskip = quote_skip_dquote;
				continue;
			}
			else if(qskip == quote_skip_htmlcomment_s1){// <!-- -->
				if(*cPos == L'-'){
					qskip = quote_skip_htmlcomment_s2;
					continue;
				}
				qskip = quote_skip_none;
			}
			else if(qskip == quote_skip_htmlcomment_s2){// <!-- -->
				if(*cPos == L'-'){
					qskip = quote_skip_htmlcomment_e1;
					continue;
				}
				qskip = quote_skip_none;
			}
			else if(qskip == quote_skip_htmlcomment_e1){// <!-- -->
				if(*cPos == L'-'){
					qskip = quote_skip_htmlcomment_e2;
				}
				continue;
			}
			else if(qskip == quote_skip_htmlcomment_e2){// <!-- -->
				if(*cPos == L'-'){
					qskip = quote_skip_htmlcomment_e3;
				}
				else{
					qskip = quote_skip_htmlcomment_e1;
				}
				continue;
			}
			else if(qskip == quote_skip_htmlcomment_e3) {// <!-- -->
				if(*cPos == L'>'){
					qskip = quote_skip_none;
				}
				else{
					qskip = quote_skip_htmlcomment_e1;
				}
				continue;
			}

			if(state == state_need_start_braket){
				if(*cPos == L'<'){
					state = state_need_if_slash;
				}

				continue;
			}

			if(state == state_need_if_slash){
				isCLoseTag =  (*cPos == L'/');
				state = state_need_tagname;
				tag_match_pos = 0;
				if(isCLoseTag){
					continue; //skip /
				}
			}

			if(state == state_need_tagname){
				if(tagname[tag_match_pos]){
					wchar_t c = *cPos;
					if(c >= L'a' && c <= L'z'){
						c = c - L'a' + L'A'; //強制大文字
					}
					if( c == tagname[tag_match_pos]){
						tag_match_pos++;
						continue;
					}

					state = state_need_start_braket;
					if( !isCLoseTag && c == L'!'){ //<!-- のコメントかも知れない
						qskip = quote_skip_htmlcomment_s1;
						continue;
					}
					continue;
				}

				//タグ名の終端までマッチ
				if(*cPos == L'>'){ 
					//<div> 名前後即閉じ
					state = state_need_end_braket;
					lastPos = *cPos;
				}
				else if(*cPos == L' ' || *cPos == L'\t' || *cPos == L'\r' || *cPos == L'\n'){
					//<div >
					state = state_need_end_braket;
					lastPos = *cPos;
				}
				else{
					//<div/> or <divaa> 自己完結しているか、名前違いなので読み飛ばす.
					state = state_need_start_braket;
					continue;
				}
			}

			if(state == state_need_end_braket){
				if(*cPos != L'>'){
					lastPos = *cPos;
					continue;
				}

				//タグの終わり
				if(isCLoseTag){
					--level;
					if( level == 0 ){	//	見つかった！
						ptPos.x = cPos - cline;
						m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, pptLayoutNew );
						return true;
						//	Happy Ending
					}
				}
				else{
					if(lastPos == L'/'){
						//<div /> 自己完結しているので無視
					}
					else{//<div>
						++level;
					}
				}

				//タグ検索の最初に戻る.
				state = state_need_start_braket;
				continue;
			}
		}

		nSearchNum--;
		if( ( 0 > nSearchNum ) && ( 0 == (*mode & 1 ) ) ){
			// 表示領域外を調べないモードで表示領域の終端の場合
			break;
		}

		//	次の行へ
		ptPos.y++;
		ci = ci->GetNextLine();	//	次のアイテム
		if( ci == NULL ){
			break;	//	終わりに達した
		}

		cline = ci->GetDocLineStrWithEOL( &len );
		cPos = cline;
		lineend = cline + len;
	}while( cline != NULL );

	return false;
}

/*!
	@brief HTMLタグの検索:逆方向

	@author rti7743

	@param ptPos [in] 検索開始点の物理座標
	@param pptLayoutNew [out] 移動先のレイアウト座標
	@param tagname [in] 括弧の始まりの文字
	@param mode [in] bit0(in)  : 表示領域外を調べるか？ 0:調べない  1:調べる (このbitを参照)
					 bit1(in)  : 前方文字を調べるか？   0:調べない  1:調べる
					 bit2(out) : 見つかった位置         0:後ろ      1:前

	@retval true 成功
	@retval false 失敗
*/
bool CEditView::SearchHTMLTagBackward(
	CLogicPoint		ptPos,
	CLayoutPoint*	pptLayoutNew,
	const wchar_t*	tagname,
	int*			mode
)
{
	const CDocLine* ci;

	int			len;
	const wchar_t* cPos;
	const wchar_t* pPos;
	const wchar_t* cline;
	int			level = 1;
	
	CLayoutPoint ptColLine;
	CLayoutInt		nSearchNum;

	//	初期位置の設定
	m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, &ptColLine );
	nSearchNum = ptColLine.y - GetTextArea().GetViewTopLine();
	ci = m_pcEditDoc->m_cDocLineMgr.GetLine( ptPos.GetY2() );
	cline = ci->GetDocLineStrWithEOL( &len );
	cPos = cline + ptPos.x;

	enum state_need
	{
		 state_need_end_braket     // >
		,state_need_if_slash       // /
		,state_need_space		   // space
		,state_need_tagname        // div
		,state_need_if_fist_slash  // /
		,state_need_start_braket   // <
	};
	state_need state = state_need_end_braket;

	enum quote_skip{
		 quote_skip_none
		,quote_skip_dquote			//"
		,quote_skip_if_term_dquote	//"

		,quote_skip_htmlcomment_e1	//--> 
		,quote_skip_htmlcomment_s1	//-
		,quote_skip_htmlcomment_s2	//--
		,quote_skip_htmlcomment_s3	//!--
		,quote_skip_htmlcomment_s4	//<!--
	};
	quote_skip qskip = quote_skip_none;

	bool isCLoseTag = false;
	int tag_match_pos = 0;
	const int tagname_size = wcslen(tagname) - 1;
	if(tagname_size < 0)
	{
		return false;
	}

	do {
		for( ; cPos > cline ; cPos = pPos){
			pPos = CNativeW::GetCharPrev( cline, len, cPos );
			if( cPos - pPos > 1 ){
				continue;
			}

			//クウォートの読み飛ばし.
			if(qskip == quote_skip_none){
				if(*pPos == L'"'){
					qskip = quote_skip_dquote;
					continue;
				}
			}
			else if(qskip == quote_skip_dquote){//" " を読み飛ばす
				if(*pPos == L'"')
				{
					qskip = quote_skip_if_term_dquote;
				}
				continue;
			}
			else if(qskip == quote_skip_if_term_dquote){
				if(*pPos == L'\\')
				{// \" なのでまだ継続中
					qskip = quote_skip_dquote;
					continue;
				}
				qskip = quote_skip_none;
			}
			else if(qskip == quote_skip_htmlcomment_e1){// <!-- -->
				if(*pPos == L'-'){
					qskip = quote_skip_htmlcomment_s1;
					continue;
				}
				qskip = quote_skip_none;
			}
			else if(qskip == quote_skip_htmlcomment_s1){// <!-- -->
				if(*pPos == L'-'){
					qskip = quote_skip_htmlcomment_s2;
				}
				continue;
			}
			else if(qskip == quote_skip_htmlcomment_s2){// <!-- -->
				if(*pPos == L'-'){
					qskip = quote_skip_htmlcomment_s3;
				}
				else{
					qskip = quote_skip_htmlcomment_s1;
				}
				continue;
			}
			else if(qskip == quote_skip_htmlcomment_s3){// <!-- -->
				if(*pPos == L'!'){
					qskip = quote_skip_htmlcomment_s4;
				}
				else{
					qskip = quote_skip_htmlcomment_s1;
				}
				continue;
			}
			else if(qskip == quote_skip_htmlcomment_s4){// <!-- -->
				if(*pPos == L'<'){
					qskip = quote_skip_none;
				}
				else{
					qskip = quote_skip_htmlcomment_s1;
				}
				continue;
			}


			if(state == state_need_end_braket) {
				if(*pPos == L'>'){
					state = state_need_if_slash;
				}
				continue;
			}

			//後ろから読んでいるので <div/> みたいに 即閉じの可能性検証
			if(state == state_need_if_slash) {
				if(*pPos == L'/'){ //即閉じタグ <div/> なので読み飛ばす
					state = state_need_end_braket;
					continue;
				}
				if(*pPos == L'-'){ // --> コメントの可能性
					state = state_need_end_braket;
					qskip = quote_skip_htmlcomment_e1;
					continue;
				}
				state = state_need_tagname;
				tag_match_pos = 0;
			}

			//タグのアトリビュートがあった後で、スペースがあってタグの名前があるので、
			//そのスペースを探す<div a=fの スペース
			if(state == state_need_space){
				if(*pPos == L' ' 
					|| *pPos == L'\t' 
					|| *pPos == L'\r' 
					|| *pPos == L'\n'){
					state = state_need_tagname;
					tag_match_pos = 0;
					continue;
				}
				if(*pPos == L'<'){
					state = state_need_end_braket;
					continue;
				}

				if(*pPos == L'(' 
					|| *pPos == L')' 
					|| *pPos == L'{' 
					|| *pPos == L'}' 
					|| *pPos == L';'){//javascript / style内誤爆
					state = state_need_end_braket;
					continue;
				}
			}

			//タグの名前
			if(state == state_need_tagname){
				if(*pPos == L'>'){
					state = state_need_if_slash;
					continue;
				}
				if(tag_match_pos == 0){
					if(*pPos == L' ' 
						|| *pPos == L'\t' 
						|| *pPos == L'\r' 
						|| *pPos == L'\n'){
						continue;
					}
				}
				if(tag_match_pos <= tagname_size){
					if(*pPos == L'<'){
						state = state_need_end_braket;
						continue;
					}

					wchar_t c = *pPos;
					if(c >= L'a' && c <= L'z'){
						c = c - L'a' + L'A'; //強制大文字
					}
					
					if(c != tagname[tagname_size - tag_match_pos]){ //名前が違う
						state = state_need_space; // div で <divaaa> を無視するために必要.
						tag_match_pos=0;
					}
					else{
						tag_match_pos++;
					}
					continue;
				}
				state = state_need_if_fist_slash;
			}

			//名前の後に / があれば</div>みたいな閉じタグ 
			if(state == state_need_if_fist_slash) {
				isCLoseTag =  (*pPos == L'/');
				state = state_need_start_braket;
				if(isCLoseTag) {
					continue;
				}
			}
			
			//タグ開始の<検索
			if(state == state_need_start_braket) {
				if(*pPos != L'<') {
					state = state_need_tagname;
					tag_match_pos = 0;
					continue;
				}
				
				if(isCLoseTag)	{ //閉じタグ
					++level;
				}
				else{ //開始タグ
					--level;
					if( level == 0 ){	//	見つかった！
						ptPos.x = pPos - cline;
						m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, pptLayoutNew );
						return true;
						//	Happy Ending
					}
				}
				state = state_need_end_braket;
			}
		}

		nSearchNum--;
		if( ( 0 > nSearchNum ) && ( 0 == (*mode & 1 ) ) ){	
			// 表示領域外を調べないモードで表示領域の先頭の場合
			break;
		}

		//	次の行へ
		ptPos.y--;
		ci = ci->GetPrevLine();	//	次のアイテム
		if( ci == NULL ){
			break;	//	終わりに達した
		}

		cline = ci->GetDocLineStrWithEOL( &len );
		cPos = cline + len;
	}while( cline != NULL );

	return false;
}
