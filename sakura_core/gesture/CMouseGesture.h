/*
	Copyright (C) 2014, Plugins developers

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
#ifndef CMOUSE_GESTURE_H
#define CMOUSE_GESTURE_H

#include <Windows.h>
#include <string>
#include <map>

#ifdef _UNICODE
typedef std::wstring TString;
#else
typedef std::string TString;
#endif

///////////////////////////////////////////////////////////////////////////////
#define GESTURE_MARGIN_DEFAULT	20		//!< ジェスチャー最小解像度
#define GESTURE_MOTION_MAX		100		//!< モーション記録最大数
/*
direction:
   7  0  1
     ↑
  6 ←x→ 2
     ↓
   5  4  3
*/

///////////////////////////////////////////////////////////////////////////////
class CMotion
{
public:
	CMotion(){
		m_x          = 0;
		m_y          = 0;
		m_button     = 0;
		m_tick_time  = 0;
		m_tick_count = 0;
		m_degree     = -1;
		m_direction  = -1;
		m_repeat     = 0;
	}
	virtual ~CMotion(){
	}

public:
	enum ALTERNATE_KEY {
		ALTERNATE_KEY_SHIFT		= 0x0001,	//Shift
		ALTERNATE_KEY_CONTROL	= 0x0002,	//Control
		ALTERNATE_KEY_MENU		= 0x0004	//Alt
	};

public:
	int		m_x;
	int		m_y;
	DWORD	m_button;		//ALTERNATE_KEY
	DWORD	m_tick_time;	//timeGetTime or GetTickCount
	DWORD	m_tick_count;	//前回からの時間(ms)
	int		m_degree;
	int		m_direction;
	int		m_repeat;

	CMotion& operator=(const CMotion& tmp){
		m_x          = tmp.m_x;
		m_y          = tmp.m_y;
		m_button     = tmp.m_button;
		m_tick_time  = tmp.m_tick_time;
		m_tick_count = tmp.m_tick_count;
		m_degree     = tmp.m_degree;
		m_direction  = tmp.m_direction;
		m_repeat     = tmp.m_repeat;
		return *this;
	}
};

///////////////////////////////////////////////////////////////////////////////
class CMouseGesture
{
public:
	CMouseGesture();
	virtual ~CMouseGesture();

public:
	enum GESTURE_TYPE {
		GESTURE_TYPE_4	= 4,	//4方向
		GESTURE_TYPE_8	= 8		//8方向
	};

	typedef std::map<TString, DWORD> GESTUREMAP;

//アトリビュート
public:
	int			m_margin;		//最小解像度
	int			m_type;			//GESTURE_TYPE
	int			m_count;		//現在の記録数
	CMotion		m_motion[GESTURE_MOTION_MAX];	//マウス移動記録
	TString		m_action;		//マウス移動記録から解析したアクション
	GESTUREMAP	m_mapGesture;	//登録しているジェスチャー
	CommonSetting_MouseGesture* m_lpMouseGesture;
	BOOL		m_bUpdate;

//オペレーション
public:
	virtual void StartMouseGesture(HWND hwnd, const int x, const int y, const DWORD button = 0);
	virtual DWORD EndMouseGesture(const int x, const int y, const DWORD button = 0);
	virtual void RecordMouseGesture(const int x, const int y, const DWORD button = 0);

	DWORD GetButtonState();
	void SetMargin(const int margin){ m_margin = margin; }
	void SetType(const int type){ m_type = type; }
	int GetMargin() const { return m_margin; }
	int GetType() const { return m_type; }

	void AddGesture(LPCTSTR lpszGesture, const DWORD dwGestureId);
	void RemoveGesture(LPCTSTR lpszGesture);
	void RemoveGesture(const DWORD dwGestureId);
	LPCTSTR GetGesture(const DWORD dwGestureId);
	DWORD GetGestureId(LPCTSTR lpszGesture);

	void Initialize(CommonSetting_MouseGesture* lpMouseGesture);

protected:
	DWORD GetTickCount();
	BOOL add_motion(const int x, const int y, const DWORD button);
	double calc_distance(const int previous_x, const int previous_y, const int current_x, const int current_y);
	double calc_degree(const int previous_x, const int previous_y, const int current_x, const int current_y);
	int calc_direction(const double slope);
	void NormalizeMouseGesture();
};

#endif	//CMOUSE_GESTURE_H
