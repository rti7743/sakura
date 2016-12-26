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
//                           ����                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
	@date 2003/02/18 ai
	@param flag [in] ���[�h(true:�o�^, false:����)
*/
void CEditView::SetBracketPairPos( bool flag )
{
	int	mode;

	// 03/03/06 ai ���ׂĒu���A���ׂĒu�����Undo&Redo�����Ȃ�x�����ɑΉ�
	if( m_bDoing_UndoRedo || !GetDrawSwitch() ){
		return;
	}

	if( !m_pTypeData->m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp ){
		return;
	}

	// �Ί��ʂ̌���&�o�^
	/*
	bit0(in)  : �\���̈�O�𒲂ׂ邩�H 0:���ׂȂ�  1:���ׂ�
	bit1(in)  : �O�������𒲂ׂ邩�H   0:���ׂȂ�  1:���ׂ�
	bit2(out) : ���������ʒu         0:���      1:�O
	*/
	mode = 2;

	CLayoutPoint ptColLine;

	if( flag && !GetSelectionInfo().IsTextSelected() && !GetSelectionInfo().m_bDrawSelectArea
		&& SearchBracket( GetCaret().GetCaretLayoutPos(), &ptColLine, &mode ) )
	{
		// �o�^�w��(flag=true)			&&
		// �e�L�X�g���I������Ă��Ȃ�	&&
		// �I��͈͂�`�悵�Ă��Ȃ�		&&
		// �Ή����銇�ʂ���������		�ꍇ
		if ( ( ptColLine.x >= GetTextArea().GetViewLeftCol() ) && ( ptColLine.x <= GetTextArea().GetRightCol() )
			&& ( ptColLine.y >= GetTextArea().GetViewTopLine() ) && ( ptColLine.y <= GetTextArea().GetBottomLine() ) )
		{
			// �\���̈���̏ꍇ

			// ���C�A�E�g�ʒu���畨���ʒu�֕ϊ�(�����\���ʒu��o�^)
			m_pcEditDoc->m_cLayoutMgr.LayoutToLogic( ptColLine, &m_ptBracketPairPos_PHY );
			m_ptBracketCaretPos_PHY.y = GetCaret().GetCaretLogicPos().y;
			if( 0 == ( mode & 4 ) ){
				// �J�[�\���̌�������ʒu
				m_ptBracketCaretPos_PHY.x = GetCaret().GetCaretLogicPos().x;
			}else{
				// �J�[�\���̑O�������ʒu
				m_ptBracketCaretPos_PHY.x = GetCaret().GetCaretLogicPos().x - 1;
			}
			return;
		}
	}

	// ���ʂ̋����\���ʒu��񏉊���
	m_ptBracketPairPos_PHY.Set(CLogicInt(-1), CLogicInt(-1));
	m_ptBracketCaretPos_PHY.Set(CLogicInt(-1), CLogicInt(-1));

	return;
}

/*!
	�Ί��ʂ̋����\��
	@date 2002/09/18 ai
	@date 2003/02/18 ai �ĕ`��Ή��̈ב����
*/
void CEditView::DrawBracketPair( bool bDraw )
{
	// 03/03/06 ai ���ׂĒu���A���ׂĒu�����Undo&Redo�����Ȃ�x�����ɑΉ�
	if( m_bDoing_UndoRedo || !GetDrawSwitch() ){
		return;
	}

	if( !m_pTypeData->m_ColorInfoArr[COLORIDX_BRACKET_PAIR].m_bDisp ){
		return;
	}

	// ���ʂ̋����\���ʒu�����o�^�̏ꍇ�͏I��
	if( m_ptBracketPairPos_PHY.HasNegative() || m_ptBracketCaretPos_PHY.HasNegative() ){
		return;
	}

	// �`��w��(bDraw=true)				����
	// ( �e�L�X�g���I������Ă���		����
	//   �I��͈͂�`�悵�Ă���			����
	//   �t�H�[�J�X�������Ă��Ȃ�		����
	//   �A�N�e�B�u�ȃy�C���ł͂Ȃ� )	�ꍇ�͏I��
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
		// i=0:�Ί���,i=1:�J�[�\���ʒu�̊���
		// 2011.11.23 ryoji �Ί��� -> �J�[�\���ʒu�̊��� �̏��ɏ���������ύX
		//   �� { �� } ���قȂ�s�ɂ���ꍇ�� { �� BS �ŏ����� } �̋����\������������Ȃ����iWiki BugReport/89�j�̑΍�
		//   �� ���̏����ύX�ɂ��J�[�\���ʒu�����ʂłȂ��Ȃ��Ă��Ă��Ί��ʂ�����ΑΊ��ʑ��̋����\���͉��������

		CLayoutPoint	ptColLine;

		if( i == 0 ){
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout( m_ptBracketPairPos_PHY,  &ptColLine );
		}else{
			m_pcEditDoc->m_cLayoutMgr.LogicToLayout( m_ptBracketCaretPos_PHY, &ptColLine );
		}

		if ( ( ptColLine.x >= GetTextArea().GetViewLeftCol() ) && ( ptColLine.x <= GetTextArea().GetRightCol() )
			&& ( ptColLine.y >= GetTextArea().GetViewTopLine() ) && ( ptColLine.y <= GetTextArea().GetBottomLine() ) )
		{	// �\���̈���̏ꍇ
			if( !bDraw && GetSelectionInfo().m_bDrawSelectArea && ( 0 == IsCurrentPositionSelected( ptColLine ) ) )
			{	// �I��͈͕`��ς݂ŏ����Ώۂ̊��ʂ��I��͈͓��̏ꍇ
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
						DispPos _sPos(0,0); // ���ӁF���̒l�̓_�~�[�BCheckChangeColor�ł̎Q�ƈʒu�͕s���m
						pInfo->m_pDispPos = &_sPos;
						pInfo->m_pcView = this;

						// 03/10/24 ai �܂�Ԃ��s��ColorIndex���������擾�ł��Ȃ����ɑΉ�
						// 2009.02.07 ryoji GetColorIndex �ɓn���C���f�b�N�X�̎d�l�ύX�i���͂������̎d�l�������͗l�j
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
				// 03/03/03 ai �J�[�\���̍��Ɋ��ʂ����芇�ʂ������\������Ă����Ԃ�Shift+���őI���J�n�����
				//             �I��͈͓��ɔ��]�\������Ȃ�������������̏C��
				CLayoutInt caretX = GetCaret().GetCaretLayoutPos().GetX2();
				bool bCaretHide = (!bCaretChange && (ptColLine.x == caretX || ptColLine.x + 1 == caretX) && GetCaret().GetCaretShowFlag());
				if( bCaretHide ){
					bCaretChange = true;
					GetCaret().HideCaret_( GetHwnd() );	// �L�����b�g����u������̂�h�~
				}
				{
					int nWidth  = GetTextMetrics().GetCharPxWidth();
					int nHeight = GetTextMetrics().GetHankakuDy();
					int nLeft = (GetTextArea().GetDocumentLeftClientPointX()) + GetTextMetrics().GetCharPxWidth(ptColLine.x);
					int nTop  = (Int)( ptColLine.GetY2() - GetTextArea().GetViewTopLine() ) * nHeight + GetTextArea().GetAreaTop();
					CLayoutXInt charsWidth = m_pcEditDoc->m_cLayoutMgr.GetLayoutXOfChar(pLine, nLineLen, OutputX);

					//�F�ݒ�
					CTypeSupport cTextType(this,COLORIDX_TEXT);
					// 2013.05.24 �w�i�F���e�L�X�g�̔w�i�F�Ɠ����Ȃ�J�[�\���s�̔w�i�F��K�p
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
					// 2006.04.30 Moca �Ί��ʂ̏c���Ή�
					GetTextDrawer().DispVerticalLines(gr, nTop, nTop + nHeight, ptColLine.x, ptColLine.x + charsWidth); //�����ʂ��S�p���ł���ꍇ���l��
				}

				if( ( m_pcEditWnd->GetActivePane() == m_nMyIndex )
					&& ( ( ptColLine.y == GetCaret().GetCaretLayoutPos().GetY() ) || ( ptColLine.y - 1 == GetCaret().GetCaretLayoutPos().GetY() ) ) ){	// 03/02/27 ai �s�̊Ԋu��"0"�̎��ɃA���_�[���C���������鎖������׏C��
					GetCaret().m_cUnderLine.CaretUnderLineON( true, false );
				}
			}
		}
	}
	if( bCaretChange ){
		GetCaret().ShowCaret_( GetHwnd() );	// �L�����b�g����u������̂�h�~
	}

	::ReleaseDC( GetHwnd(), gr );
}


//======================================================================
//!�Ί��ʂ̑Ή��\
//2007.10.16 kobake
struct KAKKO_T{
	const wchar_t *sStr;
	const wchar_t *eStr;
};
static const KAKKO_T g_aKakkos[] = {
	//���p
	{ L"(", L")", },
	{ L"[", L"]", },
	{ L"{", L"}", },
	{ L"<", L">", },
	{ L"�", L"�", },
	//�S�p
	{ L"�y", L"�z", },
	{ L"�w", L"�x", },
	{ L"�u", L"�v", },
	{ L"��", L"��", },
	{ L"��", L"��", },
	{ L"�s", L"�t", },
	{ L"�i", L"�j", },
	{ L"�q", L"�r", },
	{ L"�o", L"�p", },
	{ L"�k", L"�l", },
	{ L"�m", L"�n", },
	{ L"�g", L"�h", },
	{ L"��", L"��", },
	//�I�[
	{ NULL, NULL, },
};



//	Jun. 16, 2000 genta
/*!
	@brief �Ί��ʂ̌���

	�J�[�\���ʒu�̊��ʂɑΉ����銇�ʂ�T���B�J�[�\���ʒu�����ʂłȂ��ꍇ��
	�J�[�\���̌��̕��������ʂ��ǂ����𒲂ׂ�B

	�J�[�\���̑O�ア����������ʂłȂ��ꍇ�͉������Ȃ��B

	���ʂ����p���S�p���A�y�юn�܂肩�I��肩�ɂ���Ă���ɑ���4�̊֐���
	������ڂ��B

	@param ptLayout [in] �����J�n�_�̕������W
	@param pptLayoutNew [out] �ړ���̃��C�A�E�g���W
	@param mode [in,out] bit0(in)  : �\���̈�O�𒲂ׂ邩�H 0:���ׂȂ�  1:���ׂ�
						 bit1(in)  : �O�������𒲂ׂ邩�H   0:���ׂȂ�  1:���ׂ� (����bit���Q��)
						 bit2(out) : ���������ʒu         0:���      1:�O     (����bit���X�V)

	@retval true ����
	@retval false ���s

	@author genta
	@date Jun. 16, 2000 genta
	@date Feb. 03, 2001 MIK �S�p���ʂɑΉ�
	@date Sep. 18, 2002 ai mode�̒ǉ�
*/
bool CEditView::SearchBracket(
	const CLayoutPoint&	ptLayout,
	CLayoutPoint*		pptLayoutNew,
	int*				mode
)
{
	CLogicInt len;	//	�s�̒���

	CLogicPoint ptPos;

	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic( ptLayout, &ptPos );
	const wchar_t *cline = m_pcEditDoc->m_cDocLineMgr.GetLine(ptPos.GetY2())->GetDocLineStrWithEOL(&len);

	//	Jun. 19, 2000 genta
	if( cline == NULL )	//	�Ō�̍s�ɖ{�����Ȃ��ꍇ
		return false;

	// ���ʏ��� 2007.10.16 kobake
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
		/* �J�[�\���̑O���𒲂ׂȂ��ꍇ */
		return false;
	}
	*mode |= 4;
	// 02/09/18 ai End

	//	���ʂ�������Ȃ�������C�J�[�\���̒��O�̕����𒲂ׂ�

	if( ptPos.x <= 0 ){
		return false;	//	�O�̕����͂Ȃ�
	}

	const wchar_t *bPos = CNativeW::GetCharPrev( cline, ptPos.x, cline + ptPos.x );
	int nCharSize = cline + ptPos.x - bPos;
	// ���ʏ��� 2007.10.16 kobake
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
	@brief ���p�Ί��ʂ̌���:������

	@author genta

	@param ptLayout [in] �����J�n�_�̕������W
	@param pptLayoutNew [out] �ړ���̃��C�A�E�g���W
	@param upChar [in] ���ʂ̎n�܂�̕���
	@param dnChar [in] ���ʂ���镶����
	@param mode   [in] bit0(in)  : �\���̈�O�𒲂ׂ邩�H 0:���ׂȂ�  1:���ׂ� (����bit���Q��)
					 bit1(in)  : �O�������𒲂ׂ邩�H   0:���ׂȂ�  1:���ׂ�
					 bit2(out) : ���������ʒu         0:���      1:�O

	@retval true ����
	@retval false ���s
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

	//	�����ʒu�̐ݒ�
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

			if( level == 0 ){	//	���������I
				ptPos.x = cPos - cline;
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, pptLayoutNew );
				return true;
				//	Happy Ending
			}
			cPos = nPos;	//	���̕�����
		}

		// 02/09/19 ai Start
		nSearchNum--;
		if( ( 0 > nSearchNum ) && ( 0 == (*mode & 1 ) ) )
		{	// �\���̈�O�𒲂ׂȂ����[�h�ŕ\���̈�̏I�[�̏ꍇ
			//SendStatusMessage( "�Ί��ʂ̌����𒆒f���܂���" );
			break;
		}
		// 02/09/19 ai End

		//	���̍s��
		ptPos.y++;
		ci = ci->GetNextLine();	//	���̃A�C�e��
		if( ci == NULL )
			break;	//	�I���ɒB����

		cline = ci->GetDocLineStrWithEOL( &len );
		cPos = cline;
		lineend = cline + len;
	}while( cline != NULL );

	return false;
}

/*!
	@brief ���p�Ί��ʂ̌���:�t����

	@author genta

	@param ptLayout [in] �����J�n�_�̕������W
	@param pptLayoutNew [out] �ړ���̃��C�A�E�g���W
	@param upChar [in] ���ʂ̎n�܂�̕���
	@param dnChar [in] ���ʂ���镶����
	@param mode [in] bit0(in)  : �\���̈�O�𒲂ׂ邩�H 0:���ׂȂ�  1:���ׂ� (����bit���Q��)
					 bit1(in)  : �O�������𒲂ׂ邩�H   0:���ׂȂ�  1:���ׂ�
					 bit2(out) : ���������ʒu         0:���      1:�O

	@retval true ����
	@retval false ���s
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

	//	�����ʒu�̐ݒ�
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

			if( level == 0 ){	//	���������I
				ptPos.x = pPos - cline;
				m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, pptLayoutNew );
				return true;
				//	Happy Ending
			}
			cPos = pPos;	//	���̕�����
		}

		// 02/09/19 ai Start
		nSearchNum--;
		if( ( 0 > nSearchNum ) && ( 0 == (*mode & 1 ) ) )
		{	// �\���̈�O�𒲂ׂȂ����[�h�ŕ\���̈�̐擪�̏ꍇ
			//SendStatusMessage( "�Ί��ʂ̌����𒆒f���܂���" );
			break;
		}
		// 02/09/19 ai End

		//	���̍s��
		ptPos.y--;
		ci = ci->GetPrevLine();	//	���̃A�C�e��
		if( ci == NULL )
			break;	//	�I���ɒB����

		cline = ci->GetDocLineStrWithEOL( &len );
		cPos = cline + len;
	}while( cline != NULL );

	return false;
}

//@@@ 2003.01.09 Start by ai:
/*!
	@brief ���ʔ���

	@author ai

	@param pLine [in] 
	@param x
	@param size

	@retval true ����
	@retval false �񊇌�
*/
bool CEditView::IsBracket( const wchar_t *pLine, CLogicInt x, CLogicInt size )
{
	// ���ʏ��� 2007.10.16 kobake
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
	@brief ��HTML�^�O�̌���

	�J�[�\���ʒu��HTML�^�O�ɑΉ�����HTML�^�O��T���B�J�[�\���ʒu��HTML�^�O�łȂ��ꍇ��
	�J�[�\���̌��̕�����HTML�^�O���ǂ����𒲂ׂ�B

	�J�[�\���̑O�ア�������HTML�^�O�łȂ��ꍇ�͉������Ȃ��B

	@param ptLayout [in] �����J�n�_�̕������W
	@param pptLayoutNew [out] �ړ���̃��C�A�E�g���W
	@param mode [in,out] bit0(in)  : �\���̈�O�𒲂ׂ邩�H 0:���ׂȂ�  1:���ׂ�
						 bit1(in)  : �O�������𒲂ׂ邩�H   0:���ׂȂ�  1:���ׂ� (����bit���Q��)
						 bit2(out) : ���������ʒu         0:���      1:�O     (����bit���X�V)

	@retval true ����
	@retval false ���s

	@author rti7743
	@date Dec. 25, 2016 rti7743
*/
bool CEditView::SearchHTMLTag(
	const CLayoutPoint&	ptLayout,
	CLayoutPoint*		pptLayoutNew,
	int*				mode
)
{
	CLogicInt len;	//	�s�̒���

	CLogicPoint ptPos;

	m_pcEditDoc->m_cLayoutMgr.LayoutToLogic( ptLayout, &ptPos );
	const wchar_t *cline = m_pcEditDoc->m_cDocLineMgr.GetLine(ptPos.GetY2())->GetDocLineStrWithEOL(&len);

	//	Jun. 19, 2000 genta
	if( cline == NULL )	//	�Ō�̍s�ɖ{�����Ȃ��ꍇ
		return false;

	//HTML�^�O����
	//2016/12/24 rti7743
	wchar_t html_tag[64];
	int found_html_tag = getHTMLTagName( cline , ptPos , html_tag , 64);
	if(found_html_tag == 0)
	{
		return false;
	}

	//���^�O����Ȃ��^�O�������疳��.
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
//		,L"LINK" //RSS�Ȃǂ�xml�Ŏg���邱�Ƃ�����̂ŊO����
		,L"PARAM"
		,L"SOURCE"
		//�I�[
		,NULL
	};
	for( int i = 0; aSkipTags[i] ; i++ )
	{
		if( wcscmp(html_tag,aSkipTags[i]) == 0 )
		{//���^�O�s�v�̌����֎~�^�O
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
	@brief ���݈ʒu�ɂ���HTML�^�O���擾

	@author rti7743

	@param cline [in] ���݂̍s�f�[�^
	@param ptPos [in] �ړ��̍��W
	@param out_html_tag[out] �^�O�� DIV��������. �K���啶���Ŋi�[
	@param max_html_tag_size [in] out_html_tag�o�b�t�@�̃T�C�Y

	@retval 0 �^�O�͂���܂���
	@retval 1 �J�n�^�O
	@retval 2 ���^�O
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
	{//�^�O�ł͂Ȃ�
		return 0;
	}
	x++;//skip < tag

	//�^�O���̎擾
	bool isCloseTag = false;
	if( cline[x] == L'/' )
	{//���^�O
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
	{//�^�O�̒�������������
		return 0;
	}
	//�^�O����啶���ɕϊ����Ċi�[.
	for(int i = 0 ; i < size ; i++)
	{
		wchar_t c = cline[x+i];
		if(c >= L'a' && c <= L'z')
		{
			c = c - L'a' + L'A'; //�����啶��
		}
		out_html_tag[i] = c;
	}
	out_html_tag[size] = 0; //null�I�[

	if(isCloseTag)
	{
		return 2; //���^�O
	}
	return 1; //�J�n�^�O
}



/*!
	@brief HTML�^�O�̌���:������

	@author rti7743

	@param ptPos [in] �����J�n�_�̕������W
	@param pptLayoutNew [out] �ړ���̃��C�A�E�g���W
	@param tagname[in] ���ʂ̎n�܂�̕���
	@param mode   [in] bit0(in)  : �\���̈�O�𒲂ׂ邩�H 0:���ׂȂ�  1:���ׂ� (����bit���Q��)
					 bit1(in)  : �O�������𒲂ׂ邩�H   0:���ׂȂ�  1:���ׂ�
					 bit2(out) : ���������ʒu         0:���      1:�O

	@retval true ����
	@retval false ���s
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

	//	�����ʒu�̐ݒ�
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

			//�N�E�H�[�g�̓ǂݔ�΂�.

			if(qskip == quote_skip_none){
				if(*cPos == L'"'){
					qskip = quote_skip_dquote;
					continue;
				}
			}
			else if(qskip == quote_skip_dquote){ //" " ��ǂݔ�΂�
				if(*cPos == L'\\'){
					qskip = quote_skip_escape_dquote;
				}
				else if(*cPos == L'"'){
					qskip = quote_skip_none;
				}
				continue;
			}
			else if(qskip == quote_skip_escape_dquote){ // \" ��ǂݔ�΂�
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
						c = c - L'a' + L'A'; //�����啶��
					}
					if( c == tagname[tag_match_pos]){
						tag_match_pos++;
						continue;
					}

					state = state_need_start_braket;
					if( !isCLoseTag && c == L'!'){ //<!-- �̃R�����g�����m��Ȃ�
						qskip = quote_skip_htmlcomment_s1;
						continue;
					}
					continue;
				}

				//�^�O���̏I�[�܂Ń}�b�`
				if(*cPos == L'>'){ 
					//<div> ���O�㑦��
					state = state_need_end_braket;
					lastPos = *cPos;
				}
				else if(*cPos == L' ' || *cPos == L'\t' || *cPos == L'\r' || *cPos == L'\n'){
					//<div >
					state = state_need_end_braket;
					lastPos = *cPos;
				}
				else{
					//<div/> or <divaa> ���Ȋ������Ă��邩�A���O�Ⴂ�Ȃ̂œǂݔ�΂�.
					state = state_need_start_braket;
					continue;
				}
			}

			if(state == state_need_end_braket){
				if(*cPos != L'>'){
					lastPos = *cPos;
					continue;
				}

				//�^�O�̏I���
				if(isCLoseTag){
					--level;
					if( level == 0 ){	//	���������I
						ptPos.x = cPos - cline;
						m_pcEditDoc->m_cLayoutMgr.LogicToLayout( ptPos, pptLayoutNew );
						return true;
						//	Happy Ending
					}
				}
				else{
					if(lastPos == L'/'){
						//<div /> ���Ȋ������Ă���̂Ŗ���
					}
					else{//<div>
						++level;
					}
				}

				//�^�O�����̍ŏ��ɖ߂�.
				state = state_need_start_braket;
				continue;
			}
		}

		nSearchNum--;
		if( ( 0 > nSearchNum ) && ( 0 == (*mode & 1 ) ) ){
			// �\���̈�O�𒲂ׂȂ����[�h�ŕ\���̈�̏I�[�̏ꍇ
			break;
		}

		//	���̍s��
		ptPos.y++;
		ci = ci->GetNextLine();	//	���̃A�C�e��
		if( ci == NULL ){
			break;	//	�I���ɒB����
		}

		cline = ci->GetDocLineStrWithEOL( &len );
		cPos = cline;
		lineend = cline + len;
	}while( cline != NULL );

	return false;
}

/*!
	@brief HTML�^�O�̌���:�t����

	@author rti7743

	@param ptPos [in] �����J�n�_�̕������W
	@param pptLayoutNew [out] �ړ���̃��C�A�E�g���W
	@param tagname [in] ���ʂ̎n�܂�̕���
	@param mode [in] bit0(in)  : �\���̈�O�𒲂ׂ邩�H 0:���ׂȂ�  1:���ׂ� (����bit���Q��)
					 bit1(in)  : �O�������𒲂ׂ邩�H   0:���ׂȂ�  1:���ׂ�
					 bit2(out) : ���������ʒu         0:���      1:�O

	@retval true ����
	@retval false ���s
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

	//	�����ʒu�̐ݒ�
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

			//�N�E�H�[�g�̓ǂݔ�΂�.
			if(qskip == quote_skip_none){
				if(*pPos == L'"'){
					qskip = quote_skip_dquote;
					continue;
				}
			}
			else if(qskip == quote_skip_dquote){//" " ��ǂݔ�΂�
				if(*pPos == L'"')
				{
					qskip = quote_skip_if_term_dquote;
				}
				continue;
			}
			else if(qskip == quote_skip_if_term_dquote){
				if(*pPos == L'\\')
				{// \" �Ȃ̂ł܂��p����
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

			//��납��ǂ�ł���̂� <div/> �݂����� �����̉\������
			if(state == state_need_if_slash) {
				if(*pPos == L'/'){ //�����^�O <div/> �Ȃ̂œǂݔ�΂�
					state = state_need_end_braket;
					continue;
				}
				if(*pPos == L'-'){ // --> �R�����g�̉\��
					state = state_need_end_braket;
					qskip = quote_skip_htmlcomment_e1;
					continue;
				}
				state = state_need_tagname;
				tag_match_pos = 0;
			}

			//�^�O�̃A�g���r���[�g����������ŁA�X�y�[�X�������ă^�O�̖��O������̂ŁA
			//���̃X�y�[�X��T��<div a=f�� �X�y�[�X
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
					|| *pPos == L';'){//javascript / style���딚
					state = state_need_end_braket;
					continue;
				}
			}

			//�^�O�̖��O
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
						c = c - L'a' + L'A'; //�����啶��
					}
					
					if(c != tagname[tagname_size - tag_match_pos]){ //���O���Ⴄ
						state = state_need_space; // div �� <divaaa> �𖳎����邽�߂ɕK�v.
						tag_match_pos=0;
					}
					else{
						tag_match_pos++;
					}
					continue;
				}
				state = state_need_if_fist_slash;
			}

			//���O�̌�� / �������</div>�݂����ȕ��^�O 
			if(state == state_need_if_fist_slash) {
				isCLoseTag =  (*pPos == L'/');
				state = state_need_start_braket;
				if(isCLoseTag) {
					continue;
				}
			}
			
			//�^�O�J�n��<����
			if(state == state_need_start_braket) {
				if(*pPos != L'<') {
					state = state_need_tagname;
					tag_match_pos = 0;
					continue;
				}
				
				if(isCLoseTag)	{ //���^�O
					++level;
				}
				else{ //�J�n�^�O
					--level;
					if( level == 0 ){	//	���������I
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
			// �\���̈�O�𒲂ׂȂ����[�h�ŕ\���̈�̐擪�̏ꍇ
			break;
		}

		//	���̍s��
		ptPos.y--;
		ci = ci->GetPrevLine();	//	���̃A�C�e��
		if( ci == NULL ){
			break;	//	�I���ɒB����
		}

		cline = ci->GetDocLineStrWithEOL( &len );
		cPos = cline + len;
	}while( cline != NULL );

	return false;
}
