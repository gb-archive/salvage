#pragma once
#include "afxwin.h"


class CConfiguration {
private:
    CConfiguration();
    CConfiguration(CConfiguration &);
    CConfiguration & operator=(CConfiguration &);
public:
    unsigned int mPort;
    int mMemoryType;
    int mBanked;
    int mProtection;
    ~CConfiguration();
    static CConfiguration & GetInstance();
};

// CConfigurationDialog dialog

class CConfigurationDialog : public CDialog
{
	DECLARE_DYNAMIC(CConfigurationDialog)
public:
	CConfigurationDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CConfigurationDialog();

// Dialog Data
	enum { IDD = IDD_CONFIGURE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
};

