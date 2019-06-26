#pragma once

class CartIO;

// CTestDialog dialog

class CTestDialog : public CDialog
{
	DECLARE_DYNAMIC(CTestDialog)
    CartIO & m_c;
    BYTE m_byte;
public:
	CTestDialog(CartIO & cio, CWnd* pParent = NULL);   // standard constructor
	virtual ~CTestDialog();
// Dialog Data
	enum { IDD = IDD_TEST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    void SetHexText(unsigned int val);
    void ParseHexText();
    afx_msg void OnTest(unsigned char);
    afx_msg void OnBnClickedHigh();
    afx_msg void OnBnClickedLow();
    afx_msg void OnBnClickedReadByte();
    afx_msg void OnBnClickedWriteByte();
    afx_msg void OnBnClickedSetSignals();
};
