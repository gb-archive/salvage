// MainFrm.h : interface of the CMainFrame class
//


#pragma once
class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CToolBar    m_wndToolBar;
    bool m_show;
    CRect m_r;
    int m_depth;

// Generated message map functions
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnTest();
    afx_msg void OnUpdateTest(CCmdUI *pCmdUI);
    afx_msg void OnViewHide();
    afx_msg void OnUpdateViewHide(CCmdUI *pCmdUI);
    afx_msg void OnConfigure();
    afx_msg void OnUpdateConfigure(CCmdUI *pCmdUI);
    afx_msg void OnClose();
};


