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

#include "StdAfx.h"
#include "docplus/CColorMarker.h"
#include "doc/CEditDoc.h"
#include "doc/logic/CDocLineMgr.h"
#include "doc/logic/CDocLine.h"
#include "window/CEditWnd.h"
#include "env/CShareData.h"
#include "env/DLLSHAREDATA.h"
#include "io/CTextStream.h"
#include "util/string_ex2.h"

void CColorMarkerManger::OnAfterLoad(const SLoadInfo& sLoadInfo){
	if( GetDllShareData().m_Common.m_sSearch.m_sColorMarker.m_bSaveColorMarker ){
		CEditDoc* pcDoc = GetListeningDoc();
		std::tstring fileName = sLoadInfo.cFilePath.c_str();
		fileName += _T(".scm");
		if( false == CColorMarkerVisitor().LoadColorMarker(&pcDoc->m_cDocLineMgr, fileName.c_str()) ){
			// カラーマーカーのロード失敗
		}
	}
}

void CColorMarkerManger::OnAfterSave(const SSaveInfo& sSaveInfo){
	if( GetDllShareData().m_Common.m_sSearch.m_sColorMarker.m_bSaveColorMarker ){
		CEditDoc* pcDoc = GetListeningDoc();

		// カラーマーカーがある場合だけ保存
		const CDocLine* docLine = pcDoc->m_cDocLineMgr.GetDocLineTop();
		bool bFound = false;
		while( docLine ){
			const std::vector<CMarkerItem>* pVec = docLine->m_sMark.m_cColorMarker.m_pvMarker;
			if( pVec ){
				bFound = true;
				break;
			}
			docLine = docLine->GetNextLine();
		}
		std::tstring fileName = sSaveInfo.cFilePath.c_str();
		fileName += _T(".scm");
		if( bFound ){
			
			if( false == CColorMarkerVisitor().SaveColorMarker(&pcDoc->m_cDocLineMgr, fileName.c_str()) ){
				::ErrorMessage(pcDoc->m_pcEditWnd->GetHwnd(), LS(STR_ERR_SAVECOLORMAKER), fileName.c_str());
			}
		}else{
			// マーカーがなくてscmファイルがあるときはファイル削除
			CFile file(fileName.c_str());
			if( file.IsFileExist() ){
				::DeleteFile(fileName.c_str());
			}
		}
	}
}

int CColorMarkerVisitor::GetColorMarkerCount(const CDocLine* pcDocLine) const
{
	const std::vector<CMarkerItem>* pVec = pcDocLine->m_sMark.m_cColorMarker.m_pvMarker;
	if( pVec ){
		return static_cast<int>(pVec->size());
	}
	return 0;
}

CMarkerItem* CColorMarkerVisitor::GetColorMarker(CDocLine* pcDocLine, int nIndex)
{
	std::vector<CMarkerItem>* pVec = pcDocLine->m_sMark.m_cColorMarker.m_pvMarker;
	if( pVec ){
		if( 0 <= nIndex && nIndex < static_cast<int>(pVec->size()) ){
			return &((*pVec)[nIndex]);
		}
	}
	return NULL;
}

bool CColorMarkerVisitor::AddColorMarker(CDocLine* pcDocLine, const CMarkerItem& item)
{
	if( item.m_nEnd < item.m_nBegin ){
		return false;
	}
	if( item.m_nBegin < 0 ){
		return false;
	}
	CMarkerItem ins = item;
	if( item.IsGyouOrLine() ){
		ins.m_nBegin = ins.m_nEnd = CLogicXInt(0);
	}
	std::vector<CMarkerItem>* pVec = pcDocLine->m_sMark.m_cColorMarker.m_pvMarker;
	if( pVec ){
		int nCount = static_cast<int>(pVec->size());
		// 二分挿入ソート:小さい順にソート(同じのは先に入れたのが前)
		int left = 0;
		int right = nCount;
		while( left < right ){
			int center = (left + right) / 2;
			CMarkerItem& it = (*pVec)[center];
			if( it.m_nBegin < ins.m_nBegin
				|| (it.m_nBegin == ins.m_nBegin && it.m_nEnd <= ins.m_nEnd)
			){
				// it <= ins
				left = center + 1;
			}else{
				right = center;
			}
		}
		if( left < nCount ){
			pVec->insert(pVec->begin() + left, ins);
		}else{
			pVec->push_back(ins);
		}
	}else{
		pVec = pcDocLine->m_sMark.m_cColorMarker.m_pvMarker = new std::vector<CMarkerItem>();
		pVec->push_back(ins);
	}
	return true;
}

bool CColorMarkerVisitor::DelColorMarker(CDocLine* pcDocLine, int nIndex, int nIndexEnd)
{
	std::vector<CMarkerItem>* pVec = pcDocLine->m_sMark.m_cColorMarker.m_pvMarker;
	if( pVec ){
		const int nCount = static_cast<int>(pVec->size());
		if( 0 <= nIndex && nIndexEnd <= nCount ){
			pVec->erase(pVec->begin() + nIndex, pVec->begin() + nIndexEnd );
			if( 0 == pVec->size() ){
				delete pVec;
				pcDocLine->m_sMark.m_cColorMarker.m_pvMarker = NULL;
			}
			return true;
		}
	}
	return false;
}

bool CColorMarkerVisitor::SearchColorMarker(
	const CDocLineMgr*	pcDocLineMgr,
	CLogicPoint			ptSearchBegin,	//!< 検索開始位置
	ESearchDirection	ePrevOrNext,	//!< 検索方向
	CLogicPoint&		ptHit	 		//!< マッチ位置
)
{
	const CDocLine* docLine;
	CLogicYInt nLineNum = ptSearchBegin.y;

	if( ePrevOrNext == SEARCH_BACKWARD ){
		// 後方検索(↑)
		docLine = pcDocLineMgr->GetLine(nLineNum);
		while( docLine ){
			int nCount = GetColorMarkerCount(docLine);
			if( 0 < nCount ){
				for( int i = nCount - 1; 0 <= i; i-- ){
					const CMarkerItem& item = *GetColorMarker(docLine, i);
					CLogicPoint pt(item.m_nBegin, nLineNum);
					if( pt < ptSearchBegin ){
						ptHit = pt;
						return true;
					}
				}
			}
			nLineNum--;
			docLine = docLine->GetPrevLine();
		}
	}else{
		// 前方検索(↓)
		docLine = pcDocLineMgr->GetLine(nLineNum);
		while( docLine ){
			int nCount = GetColorMarkerCount(docLine);
			if( 0 < nCount ){
				for( int i = 0; i < nCount; i++ ){
					const CMarkerItem& item = *GetColorMarker(docLine, i);
					CLogicPoint pt(item.m_nBegin, nLineNum);
					if( ptSearchBegin < pt ){
						ptHit = pt;
						return true;
					}
				}
			}
			nLineNum++;
			docLine = docLine->GetNextLine();
		}
	}
	return false;
}

void CColorMarkerVisitor::ResetAllColorMarker(CDocLineMgr* pcDocLineMgr)
{
	CDocLine* docLine = pcDocLineMgr->GetDocLineTop();
	while( docLine ){
		delete docLine->m_sMark.m_cColorMarker.m_pvMarker;
		docLine->m_sMark.m_cColorMarker.m_pvMarker = NULL;
		docLine = docLine->GetNextLine();
	}
}

static const wchar_t* COLORMARKER_VER1 = L"//sakura color marker v1";

bool CColorMarkerVisitor::SaveColorMarker(const CDocLineMgr* pcDocLineMgr, LPCTSTR pszFileName)
{
	{
		CTextOutputStream file(pszFileName);
		if( !file ){
			return false;
		}
		file.WriteString(COLORMARKER_VER1);
		file.WriteString(L"\r\n");

		const CDocLine* docLine = pcDocLineMgr->GetDocLineTop();
		CLogicYInt nLineNum = CLogicYInt(0);
		while( docLine ){
			const std::vector<CMarkerItem>* pVec = docLine->m_sMark.m_cColorMarker.m_pvMarker;
			if( pVec ){
				const std::vector<CMarkerItem>& items = *pVec;
				int nCount = static_cast<int>(items.size());
				for( int i = 0; i < nCount; i++ ){
					wchar_t szText[MAX_MARKER_INFO];
					MarkerItemToStr(items[i], nLineNum, szText);
					file.WriteString(szText);
					file.WriteString(L"\r\n");
				}
			}
			docLine = docLine->GetNextLine();
			nLineNum++;
		}

		if( !file ){
			return false;
		}
	}
	return true;
}

bool CColorMarkerVisitor::LoadColorMarker(CDocLineMgr* pcDocLineMgr, LPCTSTR pszFileName)
{
	CTextInputStream file(pszFileName);
	if( !file ){
		return false;
	}
	
	std::wstring data = file.ReadLineW();
	int nDataVersion = 0;
	if( data == COLORMARKER_VER1 ){
		nDataVersion = 1;
	}
	if( 0 < nDataVersion ){
		while( file ){
			data = file.ReadLineW();
			CMarkerItem item;
			CLogicYInt nLineNum = CLogicYInt(0);
			if( StrToMarkerItem(1, data.c_str(), item, nLineNum) ){
				if( 0 <= nLineNum && 0 <= item.m_nBegin && (item.m_nBegin < item.m_nEnd || item.IsGyouOrLine()) ){
					CDocLine* docLine = pcDocLineMgr->GetLine(CLogicInt(nLineNum));
					if( docLine ){
						AddColorMarker(docLine, item);
					}
				}
			}
		}
		return true;
	}
	return false;
}

/*!
	@param pszText MAX_MARKER_INFO以上の配列
*/
bool CColorMarkerVisitor::MarkerItemToStr(const CMarkerItem& item, CLogicYInt nLineNum, LPWSTR pszText)
{
	int nItalic = 0;
	int nStrikeOut = 0;
	auto_sprintf(pszText, L"%d,%d,%d,%d,%d,%d,%d,%08x,%08x,%d,%d,%x",
		nLineNum, item.m_nBegin, item.m_nEnd,
		item.m_nBold, item.m_nUnderLine, nItalic, nStrikeOut,
		item.m_cTEXT,
		item.m_cBACK,
		item.m_nGyouLine,
		item.IsGyou(),
		item.IsLineAll(),
		item.m_nExtValue
	);
	return true;
}

bool CColorMarkerVisitor::StrToMarkerItem(int nVer, LPCWSTR pszText, CMarkerItem& item, CLogicYInt& nLineNum)
{
	if( nVer == 1 ){
		int buf[12];
		int nRet = scan_ints(pszText, L"%d,%d,%d,%d,%d,%d,%d,%08x,%08x,%d,%d,%x", buf);
		if( (11 <= nRet && nRet <= 12) ){
			nLineNum = CLogicYInt(buf[0]);
			item.m_nBegin = CLogicXInt(buf[1]);
			item.m_nEnd = CLogicXInt(buf[2]);
			item.m_nBold = (char)(buf[3]);
			item.m_nUnderLine = (char)(buf[4]);
			// buf[5]; 取り消し線
			// buf[6]; 斜体
			item.m_cTEXT = buf[7];
			item.m_cBACK = buf[8];
			item.m_nGyouLine  = ((0 != buf[9]) ? 0x01: 0);
			item.m_nGyouLine |= ((0 != buf[10]) ? 0x02: 0);
			if( item.IsGyouOrLine() ){
				item.m_nBegin = CLogicXInt(0);
				item.m_nEnd = CLogicXInt(0);
			}
			if( 12 == nRet ){
				item.m_nExtValue = buf[11];
			}else{
				item.m_nExtValue = 0;
			}
			return true;
		}
	}
	return false;
}

/*!
	@param pszText MAX_MARKER_INFO以上の配列
*/
bool CColorMarkerVisitor::MarkerItemToStr2(const CMarkerItem& item, LPWSTR pszText)
{
	// 予約
	int nItalic = 0;
	int nStrikeOut = 0;
	auto_sprintf(pszText, L"%d,%d,%d,%d,%08x,%08x,%d,%d,%x",
		item.m_nBold, item.m_nUnderLine, nItalic, nStrikeOut,
		item.m_cTEXT,
		item.m_cBACK,
		item.IsGyou(),
		item.IsLineAll(),
		item.m_nExtValue
	);
	return true;
}

bool CColorMarkerVisitor::StrToMarkerItem2(int nVer, LPCWSTR pszText, CMarkerItem& item)
{
	if( nVer == 1 ){
		int buf[9];
		int nRet = scan_ints(pszText, L"%d,%d,%d,%d,%08x,%08x,%d,%d,%x", buf);
		if( (8 <= nRet && nRet <= 9) ){
			item.m_nBegin = CLogicXInt(0);
			item.m_nEnd = CLogicXInt(0);
			item.m_nBold = (char)(buf[0]);
			item.m_nUnderLine = (char)(buf[1]);
			// buf[2]; 取り消し線
			// buf[3]; 斜体
			item.m_cTEXT = buf[4];
			item.m_cBACK = buf[5];
			item.m_nGyouLine  = ((0 != buf[6]) ? 0x01: 0);
			item.m_nGyouLine |= ((0 != buf[7]) ? 0x02: 0);
			if( item.IsGyouOrLine() ){
				item.m_nBegin = CLogicXInt(0);
				item.m_nEnd = CLogicXInt(0);
			}
			if( 9 == nRet ){
				item.m_nExtValue = buf[8];
			}else{
				item.m_nExtValue = 0;
			}
			return true;
		}
	}
	return false;
}
/*[EOF]*/
