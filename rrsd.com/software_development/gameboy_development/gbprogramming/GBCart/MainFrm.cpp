#include "..\dib2gb\mainfrm.h"
#include "..\dib2gb\mainfrm.h"
#include "..\..\projects\vario\dib2gb\mainfrm.h"
#include "..\..\projects\vario\dib2gb\mainfrm.h"
// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "GBCart.h"
#include "GBCartDoc.h"
#include "TestDialog.h"
#include "Configuration.h"

#include "MainFrm.h"
#include ".\mainfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_WM_CREATE()
    ON_COMMAND(ID_TEST, OnTest)
    ON_COMMAND(ID_SHOW_HEX_DATA, OnViewHide)
    ON_COMMAND(ID_CONFIGURE, OnConfigure)
    ON_UPDATE_COMMAND_UI(ID_TEST, OnUpdateTest)
    ON_UPDATE_COMMAND_UI(ID_SHOW_HEX_DATA, OnUpdateViewHide)
    ON_UPDATE_COMMAND_UI(ID_CONFIGURE, OnUpdateConfigure)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR
};

// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_TOOLTIPS | CBRS_FLYBY /*| CBRS_SIZE_DYNAMIC*/) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

    CRect r1;
    int diff;

    // figuure the size of the bars
    RecalcLayout(FALSE);
    m_wndToolBar.GetWindowRect(r1);
    diff = r1.Height();
    m_wndStatusBar.GetWindowRect(r1);
    diff += r1.Height();

    // figure the size of the non-client area
    GetWindowRect(r1);
    diff += r1.Height();
    GetClientRect(r1);
    diff -= r1.Height();
    m_depth = diff;

    // adjust the frame to show just the tool bars
    //SetWindowPos(NULL, 0, 0, r1.Width(), diff, SWP_NOMOVE | SWP_NOZORDER);

	// TODO: Delete these three lines if you don't want the toolbar to be dockable
	//m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	//EnableDocking(CBRS_ALIGN_ANY);
	//DockControlBar(&m_wndToolBar);

    // get the last configuration settings
    CConfiguration::GetInstance().mPort = AfxGetApp()->GetProfileInt(
        "Configuration",
        "Port",
        0x378
    );

    return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.style = WS_OVERLAPPED | WS_CAPTION | FWS_ADDTOTITLE | FWS_PREFIXTITLE
		 | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME /*| FWS_SNAPTOBARS */  ;

	return TRUE;
}

// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

// CMainFrame message handlers

void CMainFrame::OnTest()
{
   // TODO: Add your command handler code here
    CGBCartDoc *pDoc = (CGBCartDoc *)GetActiveDocument();
    CTestDialog dlg(pDoc->m_c);
    dlg.DoModal();
}

void CMainFrame::OnUpdateTest(CCmdUI *pCmdUI)
{
    // TODO: Add your command update UI handler code here
    pCmdUI->Enable();
}

void CMainFrame::OnViewHide()
{
    // TODO: Add your command handler code here
    m_show = ! m_show;
    if(! m_show){
        // adjust the frame to show just the tool bars
        GetWindowRect(& m_r);
        SetWindowPos(NULL, 0, 0, m_r.Width(), m_depth, SWP_NOMOVE | SWP_NOZORDER);
    }
    else{
        // Restore the view to the original size
        SetWindowPos(NULL, 0, 0, m_r.Width(), m_r.Height(), SWP_NOMOVE | SWP_NOZORDER);
    }
}

void CMainFrame::OnUpdateViewHide(CCmdUI *pCmdUI)
{
    // TODO: Add your command update UI handler code here
    pCmdUI->SetCheck(m_show);
}

void CMainFrame::OnConfigure()
{
    // TODO: Add your command handler code here
    CConfigurationDialog Dlg;
    Dlg.DoModal();
}

void CMainFrame::OnUpdateConfigure(CCmdUI *pCmdUI)
{
    // TODO: Add your command update UI handler code here
    pCmdUI->Enable();
}

