#if !defined(AFX_HWINTERFACEPPG_H__F8EE2830_CDFC_468C_9D25_BC19DC5047A0__INCLUDED_)
#define AFX_HWINTERFACEPPG_H__F8EE2830_CDFC_468C_9D25_BC19DC5047A0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// HwinterfacePpg.h : Declaration of the CHwinterfacePropPage property page class.

////////////////////////////////////////////////////////////////////////////
// CHwinterfacePropPage : See HwinterfacePpg.cpp.cpp for implementation.

class CHwinterfacePropPage : public COlePropertyPage
{
	DECLARE_DYNCREATE(CHwinterfacePropPage)
	DECLARE_OLECREATE_EX(CHwinterfacePropPage)

// Constructor
public:
	CHwinterfacePropPage();

// Dialog Data
	//{{AFX_DATA(CHwinterfacePropPage)
	enum { IDD = IDD_PROPPAGE_HWINTERFACE };
		// NOTE - ClassWizard will add data members here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA

// Implementation
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Message maps
protected:
	//{{AFX_MSG(CHwinterfacePropPage)
		// NOTE - ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HWINTERFACEPPG_H__F8EE2830_CDFC_468C_9D25_BC19DC5047A0__INCLUDED)
