// ProgressBar.cpp : implementation file
//
#include "StdAfx.h"
#include "ProgressBar.h"

CProgressBar::CProgressBar(
    CFrameWnd *pF,
    LPCTSTR szMessage,
    unsigned int min,
    unsigned int max
){
    CStatusBar *pStatusBar = (CStatusBar *)pF->GetControlBar(AFX_IDW_STATUS_BAR);
    CRect rc;
    pStatusBar->GetItemRect(0, & rc);

    if(NULL != szMessage){
        CClientDC dc(pStatusBar);
        CFont * pFont = pStatusBar->GetFont();
        CFont * pOldFont = dc.SelectObject(pFont);
        CSize sizeText = dc.GetTextExtent(szMessage);
        dc.SelectObject(pOldFont);
        rc.left += sizeText.cx + 10;
    }
    pStatusBar->SetPaneText(0, szMessage);
    pStatusBar->RedrawWindow();
    m_progress.Create(WS_CHILD | WS_VISIBLE, rc, pStatusBar, 1);
    m_progress.SetRange(min, max);
}

void CProgressBar::Set(unsigned int percent){
    m_progress.SetPos(percent);
}
