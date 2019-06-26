// GBCartView.cpp : implementation of the CGBCartView class
//

#include "stdafx.h"
#include "GBCart.h"

#include "GBCartDoc.h"
#include "GBCartView.h"
#include ".\gbcartview.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGBCartView

IMPLEMENT_DYNCREATE(CGBCartView, CView)

BEGIN_MESSAGE_MAP(CGBCartView, CView)
    ON_COMMAND(ID_SHOW_HEX_DATA, OnViewHide)
    ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()

// CGBCartView construction/destruction

CGBCartView::CGBCartView()
{
	// TODO: add construction code here

}

CGBCartView::~CGBCartView()
{
}

BOOL CGBCartView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CGBCartView drawing

void CGBCartView::OnDraw(CDC* /*pDC*/)
{
	CGBCartDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CGBCartView diagnostics

#ifdef _DEBUG
void CGBCartView::AssertValid() const
{
	CView::AssertValid();
}

void CGBCartView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CGBCartDoc* CGBCartView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CGBCartDoc)));
	return (CGBCartDoc*)m_pDocument;
}
#endif //_DEBUG


// CGBCartView message handlers

void CGBCartView::OnViewHide()
{
    // TODO: Add your command handler code here
    CRect r;
    this->GetWindowRect(r);
    int diff = r.bottom - r.top;
    CFrameWnd *pWnd = this->GetParentFrame();
    pWnd->GetWindowRect(r);
    r.bottom -= diff;
    pWnd->MoveWindow(r);

}

void CGBCartView::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
    // TODO: Add your message handler code here and/or call default

    CView::OnGetMinMaxInfo(lpMMI);
}
