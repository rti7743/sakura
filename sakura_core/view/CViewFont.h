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
#ifndef SAKURA_CVIEWFONT_9E51373D_58BA_4A64_9930_5174F7BF9C929_H_
#define SAKURA_CVIEWFONT_9E51373D_58BA_4A64_9930_5174F7BF9C929_H_

#include "doc/CDocTypeSetting.h" // ColorInfo !!

class CViewFont{
public:
	CViewFont(const LOGFONT *plf, bool bMiniMap = false)
	{
		m_bMiniMap = bMiniMap;
		InitFont();
		CreateFont(plf);
	}
	CViewFont()
	{
		InitFont();
	}
	virtual ~CViewFont()
	{
		DeleteFont();
	}

	void UpdateFont(const LOGFONT *plf, int fontNo = 0)
	{
		DeleteFont2(fontNo);
		CreateFont(plf, fontNo);
	}

	HFONT ChooseFontHandle( const SFontAttr& sFontAttr ) const{
		return ChooseFontHandle( 0, sFontAttr );
	}
	HFONT ChooseFontHandle( int fontNo, const SFontAttr& sFontAttr ) const;		/* フォントを選ぶ */

	HFONT GetFontHan() const
	{
		return m_hFont[0][0];
	}
	HFONT GetFontNormal(int fontNo = 0) const
	{
		return m_hFont[fontNo][0];
	}
	void DeleteFont();

	const LOGFONT& GetLogfont(int fontNo = 0) const
	{
		return m_logFont[fontNo];
	}

private:
	void CreateFont( const LOGFONT *plf, int fontNo = 0 );
	void DeleteFont2( int fontNo );
	void InitFont();


	HFONT	m_hFont[2][32];			/* 現在のフォントハンドル */
	LOGFONT m_logFont[2];
	bool	m_bMiniMap;
};

#endif /* SAKURA_CVIEWFONT_9E51373D_58BA_4A64_9930_5174F7BF9C929_H_ */
/*[EOF]*/
