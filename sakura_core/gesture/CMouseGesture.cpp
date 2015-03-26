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
#include "stdafx.h"
#include "CMouseGesture.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#define USE_TIME_GET_TIME

#ifdef USE_TIME_GET_TIME
#include <mmsystem.h>	//timeGetTime
#pragma comment(lib, "winmm.lib")
#endif

///////////////////////////////////////////////////////////////////////////////
#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif

///////////////////////////////////////////////////////////////////////////////
CMouseGesture::CMouseGesture()
{
	m_margin = GESTURE_MARGIN_DEFAULT;
	m_type   = GESTURE_TYPE_8;
	m_count  = 0;
	m_lpMouseGesture = NULL;
	m_bUpdate = FALSE;
}


///////////////////////////////////////////////////////////////////////////////
CMouseGesture::~CMouseGesture()
{
	m_mapGesture.clear();
}

///////////////////////////////////////////////////////////////////////////////
void CMouseGesture::StartMouseGesture(HWND hwnd, const int x, const int y, const DWORD button)
{
	Initialize(m_lpMouseGesture);

	m_count = 0;
	m_action.clear();
	add_motion(x, y, button);

	SetCapture(hwnd);
}

///////////////////////////////////////////////////////////////////////////////
DWORD CMouseGesture::EndMouseGesture(const int x, const int y, const DWORD button)
{
	ReleaseCapture();

	add_motion(x, y, button);
	NormalizeMouseGesture();
#if 0
	MessageBox(NULL, m_action, _T("action"), MB_OK);
#endif
	return GetGestureId(m_action.c_str());
}

///////////////////////////////////////////////////////////////////////////////
void CMouseGesture::RecordMouseGesture(const int x, const int y, const DWORD button)
{
	add_motion(x, y, button);
}

///////////////////////////////////////////////////////////////////////////////
DWORD CMouseGesture::GetButtonState()
{
	DWORD dwButton = 0;
	if(GetKeyState(VK_SHIFT) & 0x8000) dwButton |= 0x0001;
	if(GetKeyState(VK_CONTROL) & 0x8000) dwButton |= 0x0002;
	if(GetKeyState(VK_MENU) & 0x8000) dwButton |= 0x0004;
	return dwButton;
}

///////////////////////////////////////////////////////////////////////////////
DWORD CMouseGesture::GetTickCount()
{
#ifdef USE_TIME_GET_TIME
	return ::timeGetTime();
#else
	return ::GetTickCount();
#endif
}

///////////////////////////////////////////////////////////////////////////////
BOOL CMouseGesture::add_motion(const int x, const int y, const DWORD button)
{
	//記録数オーバーをチェックする
	if(m_count >= GESTURE_MOTION_MAX){
		return FALSE;
	}

	DWORD  tick_time  = GetTickCount();
	DWORD  tick_count = 0;
	int    direction  = -1;
	double slope      = -1;
	int    repeat     = 0;
	if(m_count > 0){
		//移動距離が小さい場合は記録しない
		int distance = (int)calc_distance(m_motion[m_count-1].m_x, m_motion[m_count-1].m_y, x, y);
		if(distance < m_margin){
			return TRUE;
		}

		//角度を計算する
		slope = calc_degree(m_motion[m_count-1].m_x, m_motion[m_count-1].m_y, x, y);

		//方向を計算する
		direction = calc_direction(slope);
		if(m_motion[m_count-1].m_direction == direction){
			repeat = 1;
			m_motion[m_count-1].m_repeat = repeat;
		}

		//前回からの経過時間を計算する(49日であふれる)
		if(tick_time >= m_motion[m_count-1].m_tick_time){
			tick_count = tick_time - m_motion[m_count-1].m_tick_time;
		}else{
			tick_count = (UINT_MAX - m_motion[m_count-1].m_tick_time) + 1 + tick_time;
		}
	}

	//記録する
	m_motion[m_count].m_x          = x;
	m_motion[m_count].m_y          = y;
	m_motion[m_count].m_button     = button;
	m_motion[m_count].m_tick_time  = tick_time;
	m_motion[m_count].m_tick_count = tick_count;
	m_motion[m_count].m_degree     = (int)slope;
	m_motion[m_count].m_direction  = direction;
	m_motion[m_count].m_repeat     = repeat;
	m_count++;

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// 前回位置と今回位置の距離を計算する
double CMouseGesture::calc_distance(const int previous_x, const int previous_y, const int current_x, const int current_y)
{
	return sqrt(pow((double)(current_x - previous_x), 2) + pow((double)(current_y - previous_y), 2));
}

///////////////////////////////////////////////////////////////////////////////
//角度を計算する
double CMouseGesture::calc_degree(const int previous_x, const int previous_y, const int current_x, const int current_y)
{
	double slope = 0;
	if(previous_x == current_x){
		if(current_y > previous_y){
			slope = 180.0;
		}else{
			slope = 0.0;
		}
	}else if(previous_x >= current_x){
		slope = atan((double)(current_y - previous_y) / (double)(current_x - previous_x));
		slope = (slope * 180.0) / M_PI;
		slope += 270.0;
	}else{
		slope = atan((double)(current_y - previous_y) / (double)(previous_x - current_x));
		slope = ((slope * 180.0) / M_PI) * (-1);
		slope += 90.0;
	}
	return slope;
}

///////////////////////////////////////////////////////////////////////////////
//8方向または4方向に変換する
int CMouseGesture::calc_direction(const double slope)
{
	int direction = -1;
	if(m_type == GESTURE_TYPE_8){
		double slope2 = slope + 22.5;
		if(slope2 >= 360.0) slope2 -= 360.0;
		direction = (int)(slope2 / 45.0);
		if(direction >= 8) direction = 7;
	}else{
		double slope2 = slope + 45.0;
		if(slope2 >= 360.0) slope2 -= 360.0;
		direction = (int)(slope2 / 90.0);
		if(direction >= 4) direction = 3;
		direction *= 2;
	}
	return direction;
}

///////////////////////////////////////////////////////////////////////////////
//マウス記録からジェスチャー文字列を作成する
void CMouseGesture::NormalizeMouseGesture()
{
	m_action.clear();
	int direction = -1;
	int count = 0;
	for(int i = 0; i < m_count; i++){
		if(i == 0){
			//ボタンの種類を先頭に付ける
			m_action += (TCHAR)(_T('0') + m_motion[i].m_button);
			count++;
		} else{
			if(m_motion[i].m_direction != direction){
				if(m_motion[i].m_repeat == 1){
					//2回以上続けて記録したものだけを対象にする
					direction = m_motion[i].m_direction;
					m_action += (TCHAR)(_T('0') + direction);
					count++;
				}
			}
		}
	}

#if 0
	FILE* fp = _tfopen(_T("record.txt"), _T("w"));
	for(int i = 0; i < m_count; i++){
		int dx = 0;
		int dy = 0;
		if(i > 0){
			dx = m_motion[i].m_x - m_motion[i-1].m_x;
			dy = m_motion[i].m_y - m_motion[i-1].m_y;
		}
		_ftprintf(fp, _T("%3d : %5d (%5d, %5d) %d (%5d, %5d) %3d %d\n"), i, m_motion[i].m_tick_count, m_motion[i].m_x, m_motion[i].m_y, m_motion[i].m_button, dx, dy, (int)m_motion[i].m_degree, m_motion[i].m_direction);
	}
	fclose(fp);
#endif
}

///////////////////////////////////////////////////////////////////////////////
//ジェスチャーを登録する
void CMouseGesture::AddGesture(LPCTSTR lpszGesture, const DWORD dwGestureId)
{
	m_mapGesture[lpszGesture] = dwGestureId;
}

///////////////////////////////////////////////////////////////////////////////
void CMouseGesture::RemoveGesture(LPCTSTR lpszGesture)
{
	m_mapGesture.erase(lpszGesture);
}

///////////////////////////////////////////////////////////////////////////////
void CMouseGesture::RemoveGesture(const DWORD dwGestureId)
{
	TString strGesture = GetGesture(dwGestureId);
	if(strGesture.length() != 0){
		RemoveGesture(strGesture.c_str());
	}
}

///////////////////////////////////////////////////////////////////////////////
//ジェスチャーIDからジェスチャー文字列を検索する
LPCTSTR CMouseGesture::GetGesture(const DWORD dwGestureId)
{
	GESTUREMAP::iterator it = m_mapGesture.begin();
	for(it = m_mapGesture.begin(); it != m_mapGesture.end(); ++it){
		if(it->second == dwGestureId){
			return it->first.c_str();
		}
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
//ジェスチャー文字列からジェスチャーIDを検索する
DWORD CMouseGesture::GetGestureId(LPCTSTR lpszGesture)
{
	if(lpszGesture != NULL){
		GESTUREMAP::iterator it = m_mapGesture.find(lpszGesture);
		if(it != m_mapGesture.end()){
			return it->second;
		}
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
void CMouseGesture::Initialize(CommonSetting_MouseGesture* lpMouseGesture)
{
	m_lpMouseGesture = lpMouseGesture;

	m_mapGesture.clear();
	if(m_lpMouseGesture != NULL){
		if(m_lpMouseGesture->m_nUse){
			for(int i = 0; i < lpMouseGesture->m_nGestureNum; i++){
				AddGesture(lpMouseGesture->m_cGesture[i].m_sGesture, lpMouseGesture->m_cGesture[i].m_nFuncID);
			}
		}
		if(m_lpMouseGesture->m_nType){
			m_type = GESTURE_TYPE_8;
		}else{
			m_type = GESTURE_TYPE_4;
		}
	}
	m_bUpdate = TRUE;
}
