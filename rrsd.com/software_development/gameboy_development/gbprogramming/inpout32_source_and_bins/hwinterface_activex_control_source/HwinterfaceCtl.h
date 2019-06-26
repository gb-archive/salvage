#if !defined(AFX_HWINTERFACECTL_H__CC85A55B_8CE8_445A_99E5_3AD4679F0F60__INCLUDED_)
#define AFX_HWINTERFACECTL_H__CC85A55B_8CE8_445A_99E5_3AD4679F0F60__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// HwinterfaceCtl.h : Declaration of the CHwinterfaceCtrl ActiveX Control class.




/////////////////////////////////////////////////////////////////////////////
// CHwinterfaceCtrl : See HwinterfaceCtl.cpp for implementation.

class CHwinterfaceCtrl : public COleControl
{
	DECLARE_DYNCREATE(CHwinterfaceCtrl)

// Constructor
public:
	CHwinterfaceCtrl();
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHwinterfaceCtrl)
	public:
	virtual void OnDraw(CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid);
	virtual void DoPropExchange(CPropExchange* pPX);
	virtual void OnResetState();
	//}}AFX_VIRTUAL

// Implementation
protected:
	~CHwinterfaceCtrl();

	DECLARE_OLECREATE_EX(CHwinterfaceCtrl)    // Class factory and guid
	DECLARE_OLETYPELIB(CHwinterfaceCtrl)      // GetTypeInfo
	DECLARE_PROPPAGEIDS(CHwinterfaceCtrl)     // Property page IDs
	DECLARE_OLECTLTYPE(CHwinterfaceCtrl)		// Type name and misc status

// Message maps
	//{{AFX_MSG(CHwinterfaceCtrl)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

// Dispatch maps
	//{{AFX_DISPATCH(CHwinterfaceCtrl)
	afx_msg short InPort(short Address);
	afx_msg void OutPort(short Address, short Data);
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()

	afx_msg void AboutBox();

// Event maps
	//{{AFX_EVENT(CHwinterfaceCtrl)
	//}}AFX_EVENT
	DECLARE_EVENT_MAP()

// Dispatch and event IDs
public:
	enum {
	//{{AFX_DISP_ID(CHwinterfaceCtrl)
	dispidInPort = 1L,
	dispidOutPort = 2L,
	//}}AFX_DISP_ID
	};


};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HWINTERFACECTL_H__CC85A55B_8CE8_445A_99E5_3AD4679F0F60__INCLUDED)
