// HwinterfacePpg.cpp : Implementation of the CHwinterfacePropPage property page class.

#include "stdafx.h"
#include "hwinterface.h"
#include "HwinterfacePpg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CHwinterfacePropPage, COlePropertyPage)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CHwinterfacePropPage, COlePropertyPage)
	//{{AFX_MSG_MAP(CHwinterfacePropPage)
	// NOTE - ClassWizard will add and remove message map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CHwinterfacePropPage, "HWINTERFACE.HwinterfacePropPage.1",
	0x5617c51c, 0x9675, 0x41c8, 0xb6, 0x39, 0x90, 0, 0x7f, 0x8a, 0xe, 0xf2)


/////////////////////////////////////////////////////////////////////////////
// CHwinterfacePropPage::CHwinterfacePropPageFactory::UpdateRegistry -
// Adds or removes system registry entries for CHwinterfacePropPage

BOOL CHwinterfacePropPage::CHwinterfacePropPageFactory::UpdateRegistry(BOOL bRegister)
{
	if (bRegister)
		return AfxOleRegisterPropertyPageClass(AfxGetInstanceHandle(),
			m_clsid, IDS_HWINTERFACE_PPG);
	else
		return AfxOleUnregisterClass(m_clsid, NULL);
}


/////////////////////////////////////////////////////////////////////////////
// CHwinterfacePropPage::CHwinterfacePropPage - Constructor

CHwinterfacePropPage::CHwinterfacePropPage() :
	COlePropertyPage(IDD, IDS_HWINTERFACE_PPG_CAPTION)
{
	//{{AFX_DATA_INIT(CHwinterfacePropPage)
	// NOTE: ClassWizard will add member initialization here
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA_INIT
}


/////////////////////////////////////////////////////////////////////////////
// CHwinterfacePropPage::DoDataExchange - Moves data between page and properties

void CHwinterfacePropPage::DoDataExchange(CDataExchange* pDX)
{
	//{{AFX_DATA_MAP(CHwinterfacePropPage)
	// NOTE: ClassWizard will add DDP, DDX, and DDV calls here
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_DATA_MAP
	DDP_PostProcessing(pDX);
}


/////////////////////////////////////////////////////////////////////////////
// CHwinterfacePropPage message handlers
