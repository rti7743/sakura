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
#include "CViewFont.h"

/*! フォント作成
*/
void CViewFont::CreateFont(const LOGFONT *plf, int fontNo)
{
	assert( 0 <= fontNo && fontNo < _countof(m_hFont) );
	m_logFont[fontNo] = *plf;
	if( m_bMiniMap ){
		m_logFont[fontNo].lfHeight = GetDllShareData().m_Common.m_sWindow.m_nMiniMapFontSize;;
		m_logFont[fontNo].lfQuality = GetDllShareData().m_Common.m_sWindow.m_nMiniMapQuality;
		m_logFont[fontNo].lfOutPrecision = OUT_TT_ONLY_PRECIS;	// FixedSys等でMiniMapのフォントが小さくならない修正
	}

	/* フォント作成 */
	m_hFont[fontNo][0] = CreateFontIndirect( &m_logFont[fontNo] );
}

/*! フォント削除
*/
void CViewFont::DeleteFont()
{
	for( int k = 0; k < _countof(m_hFont); k++ ){
		DeleteFont2(k);
	}
}

void CViewFont::DeleteFont2( int fontNo )
{
	assert( 0 <= fontNo && fontNo < _countof(m_hFont) );

	for( int i = 0; i < _countof(m_hFont[0]); i++ ){
		if( m_hFont[fontNo][i] != NULL ){
			::DeleteObject( m_hFont[fontNo][i] );
			m_hFont[fontNo][i] = NULL;
		}
	}
}

void CViewFont::InitFont()
{
	for( int k = 0; k < _countof(m_hFont); k++ ){
		for( int i = 0; i < _countof(m_hFont[0]); i++ ){
			m_hFont[k][i] = NULL;
		}
	}
}

/*! フォントを選ぶ
	@param m_bBoldFont trueで太字
	@param m_bUnderLine trueで下線
*/
HFONT CViewFont::ChooseFontHandle( int fontNo, const SFontAttr& sFontAttr ) const
{
	assert( 0 <= fontNo && fontNo < _countof(m_hFont) );

	int index = 0;
	if( sFontAttr.m_bBoldFont ){	// 太字か
		index |= 0x01;
	}
	if( sFontAttr.m_bUnderLine ){	// 下線か
		index |= 0x02;
	}
	if( sFontAttr.m_bItalic ){		// 斜体か
		index |= 0x04;
	}
	if( sFontAttr.m_bStrikeOut ){	// 取り消し線か
		index |= 0x08;
	}
	if( m_hFont[fontNo][index] == NULL ){
		LOGFONT lf = m_logFont[fontNo];
		if( sFontAttr.m_bBoldFont ){
			lf.lfWeight += 300;
		}
		if( 1000 < lf.lfWeight ){
			lf.lfWeight = 1000;
		}
		if( sFontAttr.m_bUnderLine ){
			lf.lfUnderline = TRUE;
		}
		if( sFontAttr.m_bItalic ){
			lf.lfItalic = TRUE;
		}
		if( sFontAttr.m_bStrikeOut ){	// 取り消し線か
			lf.lfStrikeOut = TRUE;
		}
		CViewFont* this_ = const_cast<CViewFont*>(this);
		this_->m_hFont[fontNo][index] = CreateFontIndirect( &lf );
	}
	return m_hFont[fontNo][index];
}

