// GBCart.h : main header file for the GBCart application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


// CGBCartApp:
// See GBCart.cpp for the implementation of this class
//

class CGBCartApp : public CWinApp
{
public:
	CGBCartApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CGBCartApp theApp;