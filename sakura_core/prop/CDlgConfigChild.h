#ifndef SAKURA_CDLGCONFIGCHILD_H_
#define SAKURA_CDLGCONFIGCHILD_H_

#include "dlg/CDialog.h"

struct SDlgConfigArg;

class CDlgConfigChild : public CDialog
{
public:
	CDlgConfigChild(CommonSetting& common): m_Common(common){}

	//! モードレスダイアログの表示
	//@note 必ずオーバーライドすること
	virtual HWND DoModeless( HINSTANCE hInstance,			//!< アプリケーションインスタンスハンドル
	                         HWND hwndParent,				//!< 親ウィンドウハンドル
	                         SDlgConfigArg* pDlgConfigArg,	//!< 作業用共有データ構造体
	                         int nTypeIndex					//!< タイプindex
	) = 0;

	virtual void CloseDialog()
	{
		if( NULL != GetHwnd() ){
			GetData();
			::DestroyWindow( GetHwnd() );
			_SetHwnd(NULL);
		}
		return;
	}

protected:
	/*! ダイアログの初期化
	    @note オーバーライドした場合はこの関数を呼び出す，または同内容を実装すること
	*/
	virtual BOOL OnInitDialog( HWND hwndDlg, WPARAM wParam, LPARAM lParam )
	{
		_SetHwnd(hwndDlg);
		::SetWindowLongPtr( hwndDlg, DWLP_USER, lParam );
		// ダイアログデータの設定
		SetData();
		//ウィンドウ位置，サイズを記憶しないのでCDialog::OnInitDialog()を呼ばない
		return TRUE;
	}

	/*! WM_DESTROY メッセージを処理
	    @note オーバーライドした場合はこの関数を呼び出す，または同内容を実装すること
	*/
	virtual BOOL OnDestroy()
	{
		//// ダイアログデータの取得
		//GetData();
		//_SetHwnd(NULL);
		//ウィンドウ位置，サイズを記憶しないのでCDialog::OnDestroy()を呼ばない
		return TRUE;
	}

	/*! WM_SIZE メッセージを処理
	    @note ダイアログの位置は記憶しない
	*/
	virtual BOOL OnMove( WPARAM, LPARAM ){ return FALSE; }
	/*! WM_MOVE メッセージを処理
	    @note ダイアログのサイズは記憶しない
	*/
	virtual BOOL OnSize( WPARAM, LPARAM ){ return FALSE; }
	/*! WM_COMMAND メッセージの BN_CLICKED を処理
	    @note OK/キャンセル ボタンは無い
	*/
	virtual BOOL OnBnClicked( int ){ return FALSE; }

	HFONT SetCtrlFont( int idc_static, const LOGFONT& lf );			//!< コントロールにフォント設定する		// 2013/4/24 Uchi
	HFONT SetFontLabel( int idc_static, const LOGFONT& lf, int nps, bool bUse = true );	//!< フォントラベルにフォントとフォント名設定する	// 2013/4/24 Uchi
protected:
	int				m_nCurrentTypeIndex;
	SDlgConfigArg*	m_pDlgConfigArg;
	CommonSetting&	m_Common;		//!< 作業用共有データ構造体のポインタ

};

#endif
