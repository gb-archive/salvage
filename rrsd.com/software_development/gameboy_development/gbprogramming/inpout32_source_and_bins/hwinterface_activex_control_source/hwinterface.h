#if !defined(AFX_HWINTERFACE_H__DDDED257_931F_4C53_8A01_F172BB2FF7BB__INCLUDED_)
#define AFX_HWINTERFACE_H__DDDED257_931F_4C53_8A01_F172BB2FF7BB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// hwinterface.h : main header file for HWINTERFACE.DLL

#if !defined( __AFXCTL_H__ )
	#error include 'afxctl.h' before including this file
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CHwinterfaceApp : See hwinterface.cpp for implementation.

class CHwinterfaceApp : public COleControlModule
{
public:
	BOOL InitInstance();
	int ExitInstance();
};

extern const GUID CDECL _tlid;
extern const WORD _wVerMajor;
extern const WORD _wVerMinor;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_HWINTERFACE_H__DDDED257_931F_4C53_8A01_F172BB2FF7BB__INCLUDED)
