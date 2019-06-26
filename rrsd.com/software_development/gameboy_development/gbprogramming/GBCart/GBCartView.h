// GBCartView.h : interface of the CGBCartView class
//


#pragma once


class CGBCartView : public CView
{
protected: // create from serialization only
	CGBCartView();
	DECLARE_DYNCREATE(CGBCartView)

// Attributes
public:
	CGBCartDoc* GetDocument() const;

// Operations
public:

// Overrides
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// Implementation
public:
	virtual ~CGBCartView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnViewHide();
    afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
};

#ifndef _DEBUG  // debug version in GBCartView.cpp
inline CGBCartDoc* CGBCartView::GetDocument() const
   { return reinterpret_cast<CGBCartDoc*>(m_pDocument); }
#endif

