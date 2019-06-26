// Configuration.cpp : implementation file
//

#include "stdafx.h"
#include "GBCart.h"
#include "Configuration.h"
#include <tchar.h>

// CConfigurationDialog dialog

IMPLEMENT_DYNAMIC(CConfigurationDialog, CDialog)
CConfigurationDialog::CConfigurationDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CConfigurationDialog::IDD, pParent)
{
}

CConfigurationDialog::~CConfigurationDialog()
{
}

void CConfigurationDialog::DoDataExchange(CDataExchange* pDX)
{
    CConfiguration & i(CConfiguration::GetInstance());
    CDialog::DoDataExchange(pDX);
    DDX_CBIndex(pDX, IDC_MEMORY_TYPE, i.mMemoryType);
    DDX_Radio(pDX, IDC_NOT_BANKED, i.mBanked);
    DDX_Radio(pDX, IDC_PROTECT, i.mProtection);
    if(! pDX->m_bSaveAndValidate){
        CString s;
        s.Format("%03X", i.mPort);
        DDX_CBString(pDX, IDC_PORT_ADDRESS, s);
    }
    else{
        TCHAR *p;
        CString s;
        // get string from control
        DDX_CBString(pDX, IDC_PORT_ADDRESS, s);
        // convert to unsigned integer
        i.mPort = _tcstoul(s, &p, 16);
    }
}

BEGIN_MESSAGE_MAP(CConfigurationDialog, CDialog)
END_MESSAGE_MAP()

// CConfigurationDialog message handlers


CConfiguration & CConfiguration::GetInstance(){
    static CConfiguration instance;
    return instance;
}

CConfiguration::CConfiguration(){
    // get the last configuration settings
    mPort = AfxGetApp()->GetProfileInt(
        "Configuration",
        "Port",
        0x378
    );
    mMemoryType = AfxGetApp()->GetProfileInt(
        "Configuration",
        "MemoryType",
        0
    );
    mBanked = AfxGetApp()->GetProfileInt(
        "Configuration",
        "Banked",
        0
    );
    mProtection = AfxGetApp()->GetProfileInt(
        "Configuration",
        "Protection",
        0
    );
}

CConfiguration::~CConfiguration(){
    // TODO: Add your message handler code here and/or call default
    // save configuration settings

    AfxGetApp()->WriteProfileInt(
        "Configuration",
        "Port",
        mPort
    );
    AfxGetApp()->WriteProfileInt(
        "Configuration",
        "MemoryType",
        mMemoryType
    );
    AfxGetApp()->WriteProfileInt(
        "Configuration",
        "Banked",
        mBanked
    );
    AfxGetApp()->WriteProfileInt(
        "Configuration",
        "Protection",
        mProtection
    );
}
