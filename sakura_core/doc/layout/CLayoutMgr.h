/*!	@file
	@brief テキストのレイアウト情報管理

	@author Norio Nakatani
	@date 1998/03/06 新規作成
	@date 1998/04/14 データの削除を実装
	@date 1999/12/20 データの置換を実装
	@date 2009/08/28 nasukoji	CalTextWidthArg定義追加、DoLayout_Range()の引数変更
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2001, jepro
	Copyright (C) 2002, MIK, aroka, genta, YAZAKI
	Copyright (C) 2003, genta
	Copyright (C) 2005, Moca, genta, D.S.Koba
	Copyright (C) 2009, nasukoji

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _CLAYOUTMGR_H_
#define _CLAYOUTMGR_H_

#include <Windows.h>// 2002/2/10 aroka
#include <vector>
#include "doc/CDocListener.h"
#include "_main/global.h"// 2002/2/10 aroka
#include "basis/SakuraBasis.h"
#include "types/CType.h"
#include "CLayoutExInfo.h"
#include "mem/CMemoryIterator.h"
#include "CTsvModeInfo.h"
#include "view/colors/EColorIndexType.h"
#include "COpe.h"
#include "util/container.h"
#include "util/design_template.h"

class CBregexp;// 2002/2/10 aroka
class CLayout;// 2002/2/10 aroka
class CLayoutMgr;
class CDocLineMgr;// 2002/2/10 aroka
class CDocLine;// 2002/2/10 aroka
class CMemory;// 2002/2/10 aroka
class CEditDoc;// 2003/07/20 genta
class CSearchStringPattern;
class CColorStrategy;

//! レイアウト中の禁則タイプ	//@@@ 2002.04.20 MIK
enum EKinsokuType {
	KINSOKU_TYPE_NONE = 0,			//!< なし
	KINSOKU_TYPE_WORDWRAP,			//!< 英文ワードラップ中
	KINSOKU_TYPE_KINSOKU_HEAD,		//!< 行頭禁則中
	KINSOKU_TYPE_KINSOKU_TAIL,		//!< 行末禁則中
	KINSOKU_TYPE_KINSOKU_KUTO,		//!< 句読点ぶら下げ中
};

struct LayoutReplaceArg {
	CLayoutRange	sDelRange;		//!< [in]削除範囲。レイアウト単位。
	COpeLineData*	pcmemDeleted;	//!< [out]削除されたデータ
	COpeLineData*	pInsData;		//!< [in/out]挿入するデータ
	CLayoutInt		nAddLineNum;	//!< [out] 再描画ヒント レイアウト行の増減
	CLayoutInt		nModLineFrom;	//!< [out] 再描画ヒント 変更されたレイアウト行From(レイアウト行の増減が0のとき使う)
	CLayoutInt		nModLineTo;		//!< [out] 再描画ヒント 変更されたレイアウト行To(レイアウト行の増減が0のとき使う)
	CLayoutPoint	ptLayoutNew;	//!< [out]挿入された部分の次の位置の位置(レイアウト桁位置, レイアウト行)
	int				nDelSeq;		//!< [in]削除行のOpeシーケンス
	int				nInsSeq;		//!< [out]挿入行の元のシーケンス
};

// 編集時のテキスト最大幅算出用		// 2009.08.28 nasukoji
struct CalTextWidthArg {
	CLayoutPoint	ptLayout;		//!< 編集開始位置
	CLayoutInt		nDelLines;		//!< 削除に関係する行数 - 1（負数の時削除なし）
	CLayoutInt		nAllLinesOld;	//!< 編集前のテキスト行数
	BOOL			bInsData;		//!< 追加文字列あり
};

class CLogicPointEx: public CLogicPoint{
public:
	CLayoutInt ext;	//!< ピクセル幅
#ifdef BUILD_OPT_ENALBE_PPFONT_SUPPORT
	CLayoutXInt haba;	//!< ext設定時の１文字の幅
#endif
};

class CLaoyutMgrThreadLock
{
public:
	CLaoyutMgrThreadLock(CRITICAL_SECTION &, bool bLock = true);
	~CLaoyutMgrThreadLock();
	void Lock();
	void UnLock();
private:
	int m_nLock;
	CRITICAL_SECTION& m_cs;
};


struct SLayoutMgrThread
{
	CRITICAL_SECTION m_cs;
	HANDLE m_hEvnetWaitDocLine;
	CLayoutMgr* m_pLayoutMgr;
	volatile bool m_bEndDocLineMgr;
	volatile bool m_bWaitLayoutMgr;
	volatile bool m_bSetEvent;
	volatile bool m_bException;
	volatile int  m_nLayoutDocLineNum;
};

/*-----------------------------------------------------------------------
クラスの宣言
-----------------------------------------------------------------------*/
/*!	@brief テキストのレイアウト情報管理

	@date 2005.11.21 Moca 色分け情報をメンバーへ移動．不要となった引数をメンバ関数から削除．
*/
//2007.10.15 XYLogicalToLayoutを廃止。LogicToLayoutに統合。
class CLayoutMgr : public CProgressSubject
{
private:
	typedef CLayoutInt (CLayoutMgr::*CalcIndentProc)( CLayout* );

public:
	//生成と破棄
	CLayoutMgr();
	~CLayoutMgr();
	void Create( CEditDoc*, CDocLineMgr* );
	void Init();
	void _Empty();


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        コンフィグ                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//! タブ幅の取得
#ifdef BUILD_OPT_ENALBE_PPFONT_SUPPORT
	CLayoutInt GetTabSpace() const { return m_nTabSpace * m_nCharLayoutXPerKeta; }
	CKetaXInt  GetTabSpaceKetas() const { return m_nTabSpace; }
#else
	CLayoutInt GetTabSpace() const { return m_nTabSpace; }
	CKetaXInt GetTabSpaceKetas() const { return m_nTabSpace; }
#endif


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                          参照系                             //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//2007.10.09 kobake 関数名変更: Search → SearchLineByLayoutY
	CLayoutInt		GetLineCount() const{ return m_nLines; }	/* 全物理行数を返す */
	const wchar_t*	GetLineStr( CLayoutInt nLine, CLogicInt* pnLineLen ) const;	/* 指定された物理行のデータへのポインタとその長さを返す */
	const wchar_t*	GetLineStr( CLayoutInt nLine, CLogicInt* pnLineLen, const CLayout** ppcLayoutDes ) const;	/* 指定された物理行のデータへのポインタとその長さを返す */

	//先頭と末尾
	CLayout*		GetTopLayout()		{ return m_pLayoutTop; }
	CLayout*		GetBottomLayout()	{ return m_pLayoutBot; }
	const CLayout*	GetTopLayout() const { return m_pLayoutTop; }
	const CLayout*	GetBottomLayout() const { return m_pLayoutBot; }

	//レイアウトを探す
	const CLayout*	SearchLineByLayoutY( CLayoutInt nLineLayout ) const;	/* 指定された物理行のレイアウトデータ(CLayout)へのポインタを返す */
	CLayout*		SearchLineByLayoutY( CLayoutInt nLineLayout ){ return const_cast<CLayout*>(static_cast<const CLayoutMgr*>(this)->SearchLineByLayoutY(nLineLayout)); }

	//ワードを探す
	bool			WhereCurrentWord( CLayoutInt , CLogicInt , CLayoutRange* pSelect, CNativeW*, CNativeW* );	/* 現在位置の単語の範囲を調べる */

	//判定
	bool			IsEndOfLine( const CLayoutPoint& ptLinePos );	/* 指定位置が行末(改行文字の直前)か調べる */	//@@@ 2002.04.18 MIK

	/*! 次のTAB位置までの幅
		@param pos [in] 現在の位置
		@return 次のTAB位置までの文字数．1〜TAB幅
	 */
	CLayoutInt GetActualTabSpace(CLayoutInt pos) const {
#ifdef BUILD_OPT_ENALBE_PPFONT_SUPPORT
		CLayoutInt tabPadding = m_nCharLayoutXPerKeta - 1;
		return GetTabSpace() + tabPadding - ((pos + tabPadding) % GetTabSpace());
#else
		return GetTabSpace() - pos % GetTabSpace();
#endif
	}
	CKetaXInt  GetActualTabSpaceKetas(CKetaXInt pos) const { return GetTabSpaceKetas() - pos % GetTabSpaceKetas(); }

	CMemoryIterator CreateCMemoryIterator(const CLayout* pcLayout) const {
		return CMemoryIterator(pcLayout, this->GetTabSpace(), this->m_tsvInfo,
			this->GetWidthPerKeta(), this->GetCharSpacing());
	}


	/*! 次のTABまたはカンマ位置までの幅
		@param pos [in] 現在の位置
		@param 
		@return 次のTAB位置までの文字数．1〜TAB幅
	 */
	CLayoutInt GetActualTsvSpace(CLayoutInt pos, wchar_t ch) const {
#ifdef BUILD_OPT_ENALBE_PPFONT_SUPPORT
		if (m_tsvInfo.m_nTsvMode == TSV_MODE_NONE && ch == WCODE::TAB) {
			CLayoutInt tabPadding = m_nCharLayoutXPerKeta - 1;
			return GetTabSpace() + tabPadding - ((pos + tabPadding) % GetTabSpace());
		} else if (m_tsvInfo.m_nTsvMode == TSV_MODE_CSV && ch == WCODE::TAB) {
			return m_nCharLayoutXPerKeta; // 1文字幅
		} else if ((m_tsvInfo.m_nTsvMode == TSV_MODE_TSV && ch == WCODE::TAB)
			|| (m_tsvInfo.m_nTsvMode == TSV_MODE_CSV && ch == L',')) {
			return CLayoutInt(m_tsvInfo.GetActualTabLength(pos));
		} else {
			return GetLayoutXOfChar(&ch, 1, 0);
		}
#else
		if (m_tsvInfo.m_nTsvMode == TSV_MODE_NONE && ch == WCODE::TAB) {
			return m_nTabSpace - pos % m_nTabSpace;
		} else if (m_tsvInfo.m_nTsvMode == TSV_MODE_CSV && ch == WCODE::TAB) {
			return CLayoutInt(1);
		} else if ((m_tsvInfo.m_nTsvMode == TSV_MODE_TSV && ch == WCODE::TAB)
			|| (m_tsvInfo.m_nTsvMode == TSV_MODE_CSV && ch == L',')) {
			return CLayoutInt(m_tsvInfo.GetActualTabLength(pos));
		} else {
			return CLayoutInt(1);
		}
#endif
	}
#ifdef BUILD_OPT_ENALBE_PPFONT_SUPPORT
	CLayoutXInt GetActualTsvSpaceMiniMap(CLayoutXInt pos, wchar_t ch) const {
		if (m_tsvInfoMinimap.m_nTsvMode == TSV_MODE_NONE && ch == WCODE::TAB) {
			CLayoutXInt tabSpace = CLayoutXInt(m_tsvInfoMinimap.m_nDx * m_nTabSpace);
			CLayoutXInt tabPadding = CLayoutXInt(m_tsvInfoMinimap.m_nDx - 1);
			return tabSpace + tabPadding - ((pos + tabPadding) % tabSpace);
		} else if (m_tsvInfoMinimap.m_nTsvMode == TSV_MODE_CSV && ch == WCODE::TAB) {
			return CLayoutXInt(m_tsvInfoMinimap.m_nDx); // 1文字幅
		} else if ((m_tsvInfoMinimap.m_nTsvMode == TSV_MODE_TSV && ch == WCODE::TAB)
			|| (m_tsvInfoMinimap.m_nTsvMode == TSV_MODE_CSV && ch == L',')) {
			return CLayoutInt(m_tsvInfoMinimap.GetActualTabLength(pos));
		} else {
			return CNativeW::GetColmOfChar(&ch, 1, 0); // spacing == 0
		}
	}
#endif

	//	Aug. 14, 2005 genta
	// Sep. 07, 2007 kobake 関数名変更 GetMaxLineSize→GetMaxLineKetas
	CKetaXInt GetMaxLineKetas() const { return m_nMaxLineKetas; }
#ifdef BUILD_OPT_ENALBE_PPFONT_SUPPORT
	CLayoutXInt GetMaxLineLayout() const { return m_nMaxLineKetas * m_nCharLayoutXPerKeta; }
#else
	CKetaXInt GetMaxLineLayout() const { return GetMaxLineKetas(); }
#endif

	// 2005.11.21 Moca 引用符の色分け情報を引数から除去
	bool ChangeLayoutParam( CKetaXInt nTabSize, int nTsvMode, CKetaXInt nMaxLineKetas,
		const LOGFONT* pLogfont, bool bMiniMap, ECharWidthCacheMode eTypeFontMode );

	void CreateTsvInfoMinimap(bool bMiniMap, const LOGFONT* pLogfont, ECharWidthCacheMode eTypeFontMode);

	// Jul. 29, 2006 genta
	void GetEndLayoutPos(CLayoutPoint* ptLayoutEnd);

	CLayoutInt GetMaxTextWidth(void) const { return m_nTextWidth; }		// 2009.08.28 nasukoji	テキスト最大幅を返す


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                           検索                              //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
protected:
	int PrevOrNextWord( CLayoutInt, CLogicInt, CLayoutPoint* pptLayoutNew, BOOL, BOOL bStopsBothEnds );	/* 現在位置の左右の単語の先頭位置を調べる */
public:
	int PrevWord( CLayoutInt nLineNum, CLogicInt nIdx, CLayoutPoint* pptLayoutNew, BOOL bStopsBothEnds ){ return PrevOrNextWord(nLineNum, nIdx, pptLayoutNew, TRUE, bStopsBothEnds); }	/* 現在位置の左右の単語の先頭位置を調べる */
	int NextWord( CLayoutInt nLineNum, CLogicInt nIdx, CLayoutPoint* pptLayoutNew, BOOL bStopsBothEnds ){ return PrevOrNextWord(nLineNum, nIdx, pptLayoutNew, FALSE, bStopsBothEnds); }	/* 現在位置の左右の単語の先頭位置を調べる */

	int SearchWord( CLayoutInt nLine, CLogicInt nIdx, ESearchDirection eSearchDirection, CLayoutRange* pMatchRange, const CSearchStringPattern& );	/* 単語検索 */

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        単位の変換                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	//!ロジック→レイアウト
	void LogicToLayoutEx( const CLogicPointEx& ptLogicEx, CLayoutPoint* pptLayout, CLayoutInt nLineHint = CLayoutInt(0) )
	{
		LogicToLayout( ptLogicEx, pptLayout, nLineHint );
		if( 0 < ptLogicEx.ext ){
#ifdef BUILD_OPT_ENALBE_PPFONT_SUPPORT
			// 文字幅換算をする
			int ext = std::max(0, ::MulDiv((Int)ptLogicEx.ext, (Int)m_nCharLayoutXPerKeta, (Int)ptLogicEx.haba));
#else
			CLayoutXInt ext = ptLogicEx.ext;
#endif
			pptLayout->x += ext;
		}
	}
	void LogicToLayout( const CLogicPoint& ptLogic, CLayoutPoint* pptLayout, CLayoutInt nLineHint = CLayoutInt(0) );
	void LogicToLayout( const CLogicRange& rangeLogic, CLayoutRange* prangeLayout )
	{
		LogicToLayout(rangeLogic.GetFrom(), prangeLayout->GetFromPointer());
		LogicToLayout(rangeLogic.GetTo(), prangeLayout->GetToPointer());
	}

	//!レイアウト→ロジック変換
	void LayoutToLogicEx( const CLayoutPoint& ptLayout, CLogicPointEx* pptLogicEx ) const;
	void LayoutToLogic( const CLayoutPoint& ptLayout, CLogicPoint* pptLogic ) const;
	void LayoutToLogic( const CLayoutRange& rangeLayout, CLogicRange* prangeLogic ) const
	{
		LayoutToLogic(rangeLayout.GetFrom(), prangeLogic->GetFromPointer());
		LayoutToLogic(rangeLayout.GetTo(), prangeLogic->GetToPointer());
	}

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         デバッグ                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	void DUMP();	/* テスト用にレイアウト情報をダンプ */


	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                         編集とか                            //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	/*
	|| 更新系
	*/
	/* レイアウト情報の変更
		@date Jun. 01, 2001 JEPRO char* (行コメントデリミタ3用)を1つ追加
		@date 2002.04.13 MIK 禁則,改行文字をぶら下げる,句読点ぶらさげを追加
		@date 2002/04/27 YAZAKI STypeConfigを渡すように変更。
	*/
	void SetLayoutInfo(
		bool			bDoLayout,
		bool			bBlockingHook,
		const STypeConfig&	refType,
		CKetaXInt		nTabSpace,
		int				nTsvMode,
		CKetaXInt		nMaxLineKetas,
		CLayoutXInt		nCharLayoutXPerKeta,
		const LOGFONT*	pLogfont,
		bool			bMiniMap,
		ECharWidthCacheMode eTypeFontMode
	);

	HANDLE CreateThread(SLayoutMgrThread*); // CLayoutMgrを走らせるスレッドを作成
	unsigned int StartThread(SLayoutMgrThread*);	// _beginthreadexからの起動関数

	/* 文字列置換 */
	void ReplaceData_CLayoutMgr(
		LayoutReplaceArg*	pArg
	);

	BOOL CalculateTextWidth( BOOL bCalLineLen = TRUE, CLayoutInt nStart = CLayoutInt(-1), CLayoutInt nEnd = CLayoutInt(-1) );	/* テキスト最大幅を算出する */		// 2009.08.28 nasukoji
	void ClearLayoutLineWidth( void );				/* 各行のレイアウト行長の記憶をクリアする */		// 2009.08.28 nasukoji
	CLayoutXInt GetLayoutXOfChar( const wchar_t* pData, int nDataLen, int i ) const {
#ifdef BUILD_OPT_ENALBE_PPFONT_SUPPORT
		return CNativeW::GetColmOfChar( pData, nDataLen, i ) + m_nSpacing;
#else
		return CNativeW::GetColmOfChar( pData, nDataLen, i );
#endif
	}
	CLayoutXInt GetLayoutXOfChar( const CStringRef& str, int i ) const {
		return GetLayoutXOfChar(str.GetPtr(), str.GetLength(), i);
	}
#ifdef BUILD_OPT_ENALBE_PPFONT_SUPPORT
	CPixelXInt GetWidthPerKeta() const { return Int(m_nCharLayoutXPerKeta); }
	CPixelXInt GetCharSpacing() const { return m_nSpacing; }
#else
	CPixelXInt GetWidthPerKeta() const { return 1; }
	CPixelXInt GetCharSpacing() const { return 0; }
#endif

	void CopyTsvInfo(CTsvModeInfo& tsvInfo){
#ifdef BUILD_OPT_ENALBE_PPFONT_SUPPORT
		tsvInfo.m_nDx = Int(m_nCharLayoutXPerKeta);
		tsvInfo.m_nSpacing = m_nSpacing;
#endif
	}


protected:
	/*
	||  参照系
	*/
	const char* GetFirstLinrStr( int* );	/* 順アクセスモード：先頭行を得る */
	const char* GetNextLinrStr( int* );	/* 順アクセスモード：次の行を得る */


	/*
	|| 更新系
	*/
	// 2005.11.21 Moca 引用符の色分け情報を引数から除去
public:
	void _DoLayout(bool bBlockingHook);	/* 現在の折り返し文字数に合わせて全データのレイアウト情報を再生成します */
protected:
	// 2005.11.21 Moca 引用符の色分け情報を引数から除去
	// 2009.08.28 nasukoji	テキスト最大幅算出用引数追加
	CLayoutInt DoLayout_Range( CLayout* , CLogicInt, CLogicPoint, EColorIndexType, CLayoutColorInfo*, const CalTextWidthArg*, CLayoutInt* );	/* 指定レイアウト行に対応する論理行の次の論理行から指定論理行数だけ再レイアウトする */
	void CalculateTextWidth_Range( const CalTextWidthArg* pctwArg );	/* テキストが編集されたら最大幅を算出する */	// 2009.08.28 nasukoji
	CLayout* DeleteLayoutAsLogical( CLayout*, CLayoutInt, CLogicInt , CLogicInt, CLogicPoint, CLayoutInt* );	/* 論理行の指定範囲に該当するレイアウト情報を削除 */
	void ShiftLogicalLineNum( CLayout* , CLogicInt );	/* 指定行より後の行のレイアウト情報について、論理行番号を指定行数だけシフトする */

	//部品
	struct SLayoutWork{
		//毎ループ初期化
		EKinsokuType	eKinsokuType;
		CLogicInt		nPos;
		CLogicInt		nBgn;
		CStringRef		cLineStr;
		CLogicInt		nWordBgn;
		CLogicInt		nWordLen;
		CLayoutInt		nPosX;
		CLayoutInt		nIndent;
		CLayout*		pLayoutCalculated;

		//ループ外
		CDocLine*		pcDocLine;
		CLayout*		pLayout;
		CColorStrategy*	pcColorStrategy;
		EColorIndexType	colorPrev;
		CLayoutExInfo	exInfoPrev;
		CLogicInt		nCurLine;

		//ループ外 (DoLayoutのみ)
//		CLogicInt		nLineNum;

		//ループ外 (DoLayout_Rangeのみ)
		bool			bNeedChangeCOMMENTMODE;
		CLayoutInt		nModifyLayoutLinesNew;
		
		//ループ外 (DoLayout_Range引数)
		CLayoutInt*		pnExtInsLineNum;
		CLogicPoint		ptDelLogicalFrom;

		//関数
		CLayout* _CreateLayout(CLayoutMgr* mgr);
	};
	//関数ポインタ
	typedef void (CLayoutMgr::*PF_OnLine)(SLayoutWork*);
	//DoLayout用
	bool _DoKinsokuSkip(SLayoutWork* pWork, PF_OnLine pfOnLine);
	void _DoWordWrap(SLayoutWork* pWork, PF_OnLine pfOnLine);
	void _DoKutoBurasage(SLayoutWork* pWork);
	void _DoGyotoKinsoku(SLayoutWork* pWork, PF_OnLine pfOnLine);
	void _DoGyomatsuKinsoku(SLayoutWork* pWork, PF_OnLine pfOnLine);
	bool _DoTab(SLayoutWork* pWork, PF_OnLine pfOnLine);
	void _MakeOneLine(SLayoutWork* pWork, PF_OnLine pfOnLine);
	//DoLayout用コア
	void _OnLine1(SLayoutWork* pWork);
	//DoLayout_Range用コア
	void _OnLine2(SLayoutWork* pWork);


private:
	bool _ExistKinsokuKuto(wchar_t wc) const{ return m_pszKinsokuKuto_1.exist(wc); }
	bool _ExistKinsokuHead(wchar_t wc) const{ return m_pszKinsokuHead_1.exist(wc); }
	bool IsKinsokuHead( wchar_t wc );	/*!< 行頭禁則文字をチェックする */	//@@@ 2002.04.08 MIK
	bool IsKinsokuTail( wchar_t wc );	/*!< 行末禁則文字をチェックする */	//@@@ 2002.04.08 MIK
	bool IsKinsokuKuto( wchar_t wc );	/*!< 句読点文字をチェックする */	//@@@ 2002.04.17 MIK
	//	2005-08-20 D.S.Koba 禁則関連処理の関数化
	/*! 句読点ぶら下げの処理位置か
		@date 2005-08-20 D.S.Koba
		@date Sep. 3, 2005 genta 最適化
	*/
	bool IsKinsokuPosKuto(CLayoutInt nRest, CLayoutInt nCharChars ) const {
		return nRest < nCharChars;
	}
	bool IsKinsokuPosHead(CLayoutInt, CLayoutInt, CLayoutInt);	//!< 行頭禁則の処理位置か
	bool IsKinsokuPosTail(CLayoutInt, CLayoutInt, CLayoutInt);	//!< 行末禁則の処理位置か
private:
	//	Oct. 1, 2002 genta インデント幅計算関数群
	CLayoutInt getIndentOffset_Normal( CLayout* pLayoutPrev );
	CLayoutInt getIndentOffset_Tx2x( CLayout* pLayoutPrev );
	CLayoutInt getIndentOffset_LeftSpace( CLayout* pLayoutPrev );

protected:
	/*
	|| 実装ヘルパ系
	*/
	//@@@ 2002.09.23 YAZAKI
	// 2009.08.28 nasukoji	nPosX引数追加
	CLayout* CreateLayout( CDocLine* pCDocLine, CLogicPoint ptLogicPos, CLogicInt nLength, EColorIndexType nTypePrev, CLayoutInt nIndent, CLayoutInt nPosX, CLayoutColorInfo* );
	CLayout* InsertLineNext( CLayout*, CLayout* );
	void AddLineBottom( CLayout* );

	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
	//                        メンバ変数                           //
	// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
public:
	CDocLineMgr*			m_pcDocLineMgr;	/* 行バッファ管理マネージャ */
	CTsvModeInfo			m_tsvInfo;
#ifdef BUILD_OPT_ENALBE_PPFONT_SUPPORT
	CTsvModeInfo			m_tsvInfoMinimap;
#endif

protected:
	// 2002.10.07 YAZAKI add m_nLineTypeBot
	// 2007.09.07 kobake 変数名変更: m_nMaxLineSize→m_nMaxLineKetas
	// 2007.10.08 kobake 変数名変更: getIndentOffset→m_getIndentOffset

	//参照
	CEditDoc*		m_pcEditDoc;

	//実データ
	CLayout*				m_pLayoutTop;
	CLayout*				m_pLayoutBot;

	//タイプ別設定
	const STypeConfig*		m_pTypeConfig;
	CKetaXInt				m_nMaxLineKetas;
	CKetaXInt				m_nTabSpace;
#ifdef BUILD_OPT_ENALBE_PPFONT_SUPPORT
	CLayoutXInt				m_nCharLayoutXPerKeta;		//CKetaXInt(1)あたりのCLayoutXInt値(Spacing入り)
	CLayoutXInt				m_nCharLayoutXPerKetaMinimap;
	CPixelXInt				m_nSpacing;					//1文字ずつの間隔(px)
#endif
	vector_ex<wchar_t>		m_pszKinsokuHead_1;			// 行頭禁則文字	//@@@ 2002.04.08 MIK
	vector_ex<wchar_t>		m_pszKinsokuTail_1;			// 行末禁則文字	//@@@ 2002.04.08 MIK
	vector_ex<wchar_t>		m_pszKinsokuKuto_1;			// 句読点ぶらさげ文字	//@@@ 2002.04.17 MIK
	CalcIndentProc			m_getIndentOffset;			//	Oct. 1, 2002 genta インデント幅計算関数を保持

	//フラグ等
	EColorIndexType			m_nLineTypeBot;				//!< タイプ 0=通常 1=行コメント 2=ブロックコメント 3=シングルクォーテーション文字列 4=ダブルクォーテーション文字列
	CLayoutExInfo			m_cLayoutExInfoBot;
	CLayoutInt				m_nLines;					// 全レイアウト行数

	mutable CLayoutInt		m_nPrevReferLine;
	mutable CLayout*		m_pLayoutPrevRefer;
	
	// EOFカーソル位置を記憶する(_DoLayout/DoLayout_Rangeで無効にする)	//2006.10.01 Moca
	CLayoutInt				m_nEOFLine; //!< EOF行数
	CLayoutInt				m_nEOFColumn; //!< EOF幅位置

	// テキスト最大幅を記憶（折り返し位置算出に使用）	// 2009.08.28 nasukoji
	CLayoutInt				m_nTextWidth;				// テキスト最大幅の記憶
	CLayoutInt				m_nTextWidthMaxLine;		// 最大幅のレイアウト行

private:
	DISALLOW_COPY_AND_ASSIGN(CLayoutMgr);
};


///////////////////////////////////////////////////////////////////////
#endif /* _CLAYOUTMGR_H_ */



