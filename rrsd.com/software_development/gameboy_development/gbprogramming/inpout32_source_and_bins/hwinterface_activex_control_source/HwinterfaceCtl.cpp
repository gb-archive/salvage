// HwinterfaceCtl.cpp : Implementation of the CHwinterfaceCtrl ActiveX Control class.

#include "stdafx.h"
#include "hwinterface.h"
#include "HwinterfaceCtl.h"
#include "HwinterfacePpg.h"
#include "drvutil.h"

	int sysver;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CHwinterfaceCtrl, COleControl)


/////////////////////////////////////////////////////////////////////////////
// Message map

BEGIN_MESSAGE_MAP(CHwinterfaceCtrl, COleControl)
	//{{AFX_MSG_MAP(CHwinterfaceCtrl)
	//}}AFX_MSG_MAP
	ON_OLEVERB(AFX_IDS_VERB_PROPERTIES, OnProperties)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Dispatch map

BEGIN_DISPATCH_MAP(CHwinterfaceCtrl, COleControl)
	//{{AFX_DISPATCH_MAP(CHwinterfaceCtrl)
	DISP_FUNCTION(CHwinterfaceCtrl, "InPort", InPort, VT_I2, VTS_I2)
	DISP_FUNCTION(CHwinterfaceCtrl, "OutPort", OutPort, VT_EMPTY, VTS_I2 VTS_I2)
	//}}AFX_DISPATCH_MAP
	DISP_FUNCTION_ID(CHwinterfaceCtrl, "AboutBox", DISPID_ABOUTBOX, AboutBox, VT_EMPTY, VTS_NONE)
END_DISPATCH_MAP()


/////////////////////////////////////////////////////////////////////////////
// Event map

BEGIN_EVENT_MAP(CHwinterfaceCtrl, COleControl)
	//{{AFX_EVENT_MAP(CHwinterfaceCtrl)
	// NOTE - ClassWizard will add and remove event map entries
	//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_EVENT_MAP
END_EVENT_MAP()


/////////////////////////////////////////////////////////////////////////////
// Property pages

// TODO: Add more property pages as needed.  Remember to increase the count!
BEGIN_PROPPAGEIDS(CHwinterfaceCtrl, 1)
	PROPPAGEID(CHwinterfacePropPage::guid)
END_PROPPAGEIDS(CHwinterfaceCtrl)


/////////////////////////////////////////////////////////////////////////////
// Initialize class factory and guid

IMPLEMENT_OLECREATE_EX(CHwinterfaceCtrl, "HWINTERFACE.HwinterfaceCtrl.1",
	0xb9022892, 0xea92, 0x4f94, 0x81, 0x1, 0xb9, 0xcd, 0xe3, 0xe, 0x66, 0x7d)


/////////////////////////////////////////////////////////////////////////////
// Type library ID and version

IMPLEMENT_OLETYPELIB(CHwinterfaceCtrl, _tlid, _wVerMajor, _wVerMinor)


/////////////////////////////////////////////////////////////////////////////
// Interface IDs

const IID BASED_CODE IID_DHwinterface =
		{ 0x39f91450, 0x46fa, 0x41a9, { 0x91, 0xb, 0x66, 0x27, 0x3d, 0x10, 0x5e, 0xbb } };
const IID BASED_CODE IID_DHwinterfaceEvents =
		{ 0xd7a782fe, 0xf757, 0x4c7c, { 0x9a, 0x29, 0x8c, 0xf0, 0x22, 0x76, 0xa, 0xd6 } };


/////////////////////////////////////////////////////////////////////////////
// Control type information

static const DWORD BASED_CODE _dwHwinterfaceOleMisc =
	OLEMISC_INVISIBLEATRUNTIME |
	OLEMISC_ACTIVATEWHENVISIBLE |
	OLEMISC_SETCLIENTSITEFIRST |
	OLEMISC_INSIDEOUT |
	OLEMISC_CANTLINKINSIDE |
	OLEMISC_RECOMPOSEONRESIZE;

IMPLEMENT_OLECTLTYPE(CHwinterfaceCtrl, IDS_HWINTERFACE, _dwHwinterfaceOleMisc)


/////////////////////////////////////////////////////////////////////////////
// CHwinterfaceCtrl::CHwinterfaceCtrlFactory::UpdateRegistry -
// Adds or removes system registry entries for CHwinterfaceCtrl

BOOL CHwinterfaceCtrl::CHwinterfaceCtrlFactory::UpdateRegistry(BOOL bRegister)
{
	// TODO: Verify that your control follows apartment-model threading rules.
	// Refer to MFC TechNote 64 for more information.
	// If your control does not conform to the apartment-model rules, then
	// you must modify the code below, changing the 6th parameter from
	// afxRegApartmentThreading to 0.

	if (bRegister)
		return AfxOleRegisterControlClass(
			AfxGetInstanceHandle(),
			m_clsid,
			m_lpszProgID,
			IDS_HWINTERFACE,
			IDB_HWINTERFACE,
			afxRegApartmentThreading,
			_dwHwinterfaceOleMisc,
			_tlid,
			_wVerMajor,
			_wVerMinor);
	else
		return AfxOleUnregisterClass(m_clsid, m_lpszProgID);
}


/////////////////////////////////////////////////////////////////////////////
// CHwinterfaceCtrl::CHwinterfaceCtrl - Constructor

CHwinterfaceCtrl::CHwinterfaceCtrl()
{

InitializeIIDs(&IID_DHwinterface, &IID_DHwinterfaceEvents);
			
Opendriver();

}



/////////////////////////////////////////////////////////////////////////////
// CHwinterfaceCtrl::~CHwinterfaceCtrl - Destructor

CHwinterfaceCtrl::~CHwinterfaceCtrl()
{
	Closedriver();
}


/////////////////////////////////////////////////////////////////////////////
// CHwinterfaceCtrl::OnDraw - Drawing function

void CHwinterfaceCtrl::OnDraw(
			CDC* pdc, const CRect& rcBounds, const CRect& rcInvalid)
{
	// TODO: Replace the following code with your own drawing code.
	pdc->FillRect(rcBounds, CBrush::FromHandle((HBRUSH)GetStockObject(WHITE_BRUSH)));
	pdc->Ellipse(rcBounds);
 
}

/////////////////////////////////////////////////////////////////////////////
// CHwinterfaceCtrl::DoPropExchange - Persistence support

void CHwinterfaceCtrl::DoPropExchange(CPropExchange* pPX)
{
	ExchangeVersion(pPX, MAKELONG(_wVerMinor, _wVerMajor));
	COleControl::DoPropExchange(pPX);

	// TODO: Call PX_ functions for each persistent custom property.

}


/////////////////////////////////////////////////////////////////////////////
// CHwinterfaceCtrl::OnResetState - Reset control to default state

void CHwinterfaceCtrl::OnResetState()
{
	COleControl::OnResetState();  // Resets defaults found in DoPropExchange

	// TODO: Reset any other control state here.
}


/////////////////////////////////////////////////////////////////////////////
// CHwinterfaceCtrl::AboutBox - Display an "About" box to the user

void CHwinterfaceCtrl::AboutBox()
{
	CDialog dlgAbout(IDD_ABOUTBOX_HWINTERFACE);
	dlgAbout.DoModal();
}


/////////////////////////////////////////////////////////////////////////////
// CHwinterfaceCtrl message handlers



short CHwinterfaceCtrl::InPort(short Address) 
{
	
	return Inp32(Address);
}

void CHwinterfaceCtrl::OutPort(short Address, short Data) 
{
Out32(Address,Data);

}
