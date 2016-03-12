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
#ifndef SAKURA_CCOLORMAKER_H_
#define SAKURA_CCOLORMAKER_H_

// #include "env/CommonSetting.h" // 先行読み込み
#include "util/design_template.h" //TSingleton
#include "doc/CDocListener.h" // CDocListenerEx

class CDocLine;
class CDocLineMgr;

class CColorMarkerManger : public TSingleton<CColorMarkerManger>, public CDocListenerEx{
	friend class TSingleton<CColorMarkerManger>;
	CColorMarkerManger(){}

public:
	void OnAfterLoad(const SLoadInfo& sLoadInfo);
	void OnAfterSave(const SSaveInfo& sSaveInfo);
};

class CColorMarker{
public:
	CColorMarker() : m_pvMarker(NULL){}
	~CColorMarker(){
		delete m_pvMarker;
		m_pvMarker = NULL;
	}
	// 注意：コピーでは複製しない
	CColorMarker& operator=(const CColorMarker& rhs){
		this->m_pvMarker = NULL;
		return *this;
	}
	std::vector<CMarkerItem>* m_pvMarker;
};

class CColorMarkerVisitor{
public:
	int GetColorMarkerCount(const CDocLine* pcDocLine) const;

	const CMarkerItem* GetColorMarker(const CDocLine* pcDocLine, int nIndex) const{
		return const_cast<CColorMarkerVisitor*>(this)->GetColorMarker(const_cast<CDocLine*>(pcDocLine), nIndex);
	}
	CMarkerItem* GetColorMarker(CDocLine* pcDocLine, int nIndex);
	bool AddColorMarker(CDocLine* pcDocLine, const CMarkerItem& item);
	bool DelColorMarker(CDocLine* pcDocLine, int nIndex, int nIndexEnd);
	bool DelColorMarker(CDocLine* pcDocLine, int nIndex){
		return DelColorMarker(pcDocLine, nIndex, nIndex + 1);
	}
	bool SearchColorMarker(
		const CDocLineMgr*	pcDocLineMgr,
		CLogicPoint			ptSearchBegin,	//!< 検索開始位置
		ESearchDirection	ePrevOrNext,	//!< 検索方向
		CLogicPoint&		ptHit	 		//!< マッチ位置
	);

	void ResetAllColorMarker(CDocLineMgr* pcDocLineMgr);
	bool SaveColorMarker(const CDocLineMgr* pcDocLineMgr, LPCTSTR pszFileName);
	bool LoadColorMarker(CDocLineMgr* pcDocLineMgr, LPCTSTR pszFileName);

	bool MarkerItemToStr(const CMarkerItem& item, CLogicYInt nLineNum, LPWSTR pszText);
	bool StrToMarkerItem(int nVer, LPCWSTR pszText, CMarkerItem& item, CLogicYInt& nLineNum);
	bool MarkerItemToStr2(const CMarkerItem& item, LPWSTR pszText);
	bool StrToMarkerItem2(int nVer, LPCWSTR pszText, CMarkerItem& item);
	
};

#endif /* SAKURA_CCOLORMAKER_H_ */
/*[EOF]*/
