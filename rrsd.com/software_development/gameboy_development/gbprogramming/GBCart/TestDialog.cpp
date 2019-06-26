// TestDialog.cpp : implementation file
//

#include "stdafx.h"
//#include "GBCart.h"
#include "resource.h"
#include "TestDialog.h"
#include ".\testdialog.h"
#include "CartIO.h"

// CTestDialog dialog

IMPLEMENT_DYNAMIC(CTestDialog, CDialog)
CTestDialog::CTestDialog(CartIO & cio, CWnd* pParent /*=NULL*/) :
	CDialog(CTestDialog::IDD, pParent),
    m_c(cio)
{
}

CTestDialog::~CTestDialog()
{
}

void CTestDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTestDialog, CDialog)
    ON_BN_CLICKED(IDC_HIGH, OnBnClickedHigh)
    ON_BN_CLICKED(IDC_LOW, OnBnClickedLow)
    ON_BN_CLICKED(IDC_READ_BYTE, OnBnClickedReadByte)
    ON_BN_CLICKED(IDC_WRITE_BYTE, OnBnClickedWriteByte)
    ON_BN_CLICKED(IDC_SET_SIGNALS, OnBnClickedSetSignals)
END_MESSAGE_MAP()

// CTestDialog message handlers

void CTestDialog::OnTest(BYTE data)
{
    // TODO: Add your control notification handler code here
    switch(GetCheckedRadioButton(IDC_D0_D7, IDC_AUDIO_IN)){
    case IDC_D0_D7:
        m_c.WRITE(data);
        m_c.LOAD(CartIO::DATA_REG);
        m_c.OUTPORT(m_c.CTRL, CartIO::SET_DATA);
        break;
    case IDC_A0_A7:
        m_c.ADDRESS_LOW(data);
        break;
    case IDC_A8_A15:
        m_c.ADDRESS_HIGH(data);
        break;
    case IDC_CTRL:
        if(0 == data)
            m_c.OUTPORT(m_c.CTRL, CartIO::MREQ);
        else
            m_c.OUTPORT(m_c.CTRL, CartIO::NOP);
        break;
    case IDC_READ:
        if(0 == data)
            m_c.OUTPORT(m_c.CTRL, CartIO::NOP);
        else
            m_c.OUTPORT(m_c.CTRL, CartIO::SET_DATA);
        break;
    case IDC_WRITE:
        if(0 == data){
            m_c.WRITE(0x01);
            m_c.LOAD(m_c.DATA_REG);
            m_c.OUTPORT(m_c.CTRL, CartIO::SET_DATA);
        }
        else{
            m_c.OUTPORT(m_c.CTRL, CartIO::NOP);
        }
        break;
    case IDC_AUDIO_IN:
        if(0 == data){
            m_c.WRITE(0x02);
            m_c.LOAD(m_c.DATA_REG);
            m_c.OUTPORT(m_c.CTRL, CartIO::SET_DATA);
        }
        else{
            m_c.OUTPORT(m_c.CTRL, CartIO::NOP);
        }
        break;
    default:
        break;
    }
}

void CTestDialog::SetHexText(unsigned int b){
    const char * ctable = "0123456789ABCDEF";
    char hex[3];
    hex[0] = ctable[b >> 4];
    hex[1] = ctable[b & 0x0f];
    hex[2] = 0;
    SetDlgItemText(IDC_BYTE, hex);
    m_byte = b;
}

void CTestDialog::ParseHexText(){
    CString txt;
    CWnd *pWnd = GetDlgItem(IDC_BYTE);
    pWnd->GetWindowText(txt);
    txt.MakeLower();

    if(0 == txt.GetLength())
        return;

    unsigned int c1 = 0;
    unsigned int c2 = 0;
    TCHAR x;
    x = txt[0];
    if('a' <= x && x <= 'f')
        c1 = x - 'a' + 10;
    else
        c1 = x - '0';
    x = txt[1];
    if('a' <= x && x <= 'f')
        c2 = x - 'a' + 10;
    else
        c2 = x - '0';

    m_byte = (c1 << 4) | c2;
}

void CTestDialog::OnBnClickedHigh()
{
    // TODO: Add your control notification handler code here
    SetHexText(0xff);
}

void CTestDialog::OnBnClickedLow()
{
    // TODO: Add your control notification handler code here
    SetHexText(0);
}

void CTestDialog::OnBnClickedReadByte()
{
    // TODO: Add your control notification handler code here
    unsigned int b = m_c.READ(0);
    SetHexText(b);
}

void CTestDialog::OnBnClickedWriteByte()
{
    // TODO: Add your control notification handler code here
    ParseHexText();
    /*
    if (m_Algorithm == AMD29FXXX){
        m_c.WRITE_BKSW(0x01,0x2100); 
        m_c.ADDRESS_LOW(0x55); m_c.ADDRESS_HIGH(0x55); m_c.WRITE_FLASH(0xAA);
        m_c.ADDRESS_LOW(0xAA); m_c.ADDRESS_HIGH(0x2A); m_c.WRITE_FLASH(0x55);
        m_c.ADDRESS_LOW(0x55); m_c.ADDRESS_HIGH(0x55); m_c.WRITE_FLASH(0xA0);
    }
    SetAddr(Addr);
    */
    m_c.WRITE_FLASH(m_byte);
}

void CTestDialog::OnBnClickedSetSignals()
{
    // TODO: Add your control notification handler code here
    ParseHexText();
    OnTest(m_byte);
}
