// HexView.h

#ifndef HEXVIEW_H
#define HEXVIEW_H

#include <afxwin.h>

class CHexView : public CScrollView
{
    DECLARE_DYNCREATE(CHexView)

protected:
	CHexView();

protected:
    //{{AFX_VIRTUAL(CHexView)
   	virtual void OnInitialUpdate();
    virtual void OnDraw(CDC*);
    //}}AFX_VIRTUAL

public:
	//{{AFX_MSG(CHexView)
	afx_msg void OnSizing(UINT, LPRECT);
	afx_msg void OnVScroll(UINT, UINT, CScrollBar*);
	//{{AFX_MSG

private:
	DWORD m_dwOffset;
	DWORD m_dwTotalLines;
	DWORD m_dwPageLines;
	CFont m_Font;

    DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
protected:
    virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
};

#endif // HEXVIEW_H
