// GBCartDoc.h : interface of the CGBCartDoc class
//

#pragma once

#include "CartIO.h"

class CGBCartDoc : public CDocument
{
protected: // create from serialization only
	CGBCartDoc();
	DECLARE_DYNCREATE(CGBCartDoc)

// Attributes
public:
    enum Algorithm{
        EEPROM32K = 0,
        AMD29FXXX  = 1, // AMD Flash
        AT2XCXXX = 2    // ATMel Flass
    };
    enum ProtectionEnum{
        PROTECT = 0,
        UNPROTECT  = 1,
        UNCHANGED= 2
    };
    unsigned m_retries;
    unsigned int m_fail_count;
    unsigned int m_Addr;
    bool m_loaded;
    bool m_is_programming;

    unsigned int m_Size;
    BYTE m_Buffer[128*1024]; // largest size supported
    CartIO m_c;
// Operations
public:
// Overrides
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CGBCartDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
    bool CheckPageErase(
        unsigned int address,
        const unsigned int page_size
    ) const;
    void WritePage(
        const BYTE *bp, 
        unsigned int address,
        const unsigned int page_size
    ) const;
    bool CheckPage(
        const BYTE *bp,
        unsigned int address,
        const unsigned int page_size
    ) const;
    void ReadPage(
        BYTE *bp, 
        unsigned int address,
        const unsigned int page_size
    );
    void UnProtect();
    void UnProtectPage();
    void Protect();
    bool CheckErase() const;
    bool ProgramDefaultChip();
    bool ProgramAtMelFlash();
    bool ProgramAMDChip();
    unsigned int BankSet(unsigned int bank) const;
    unsigned int PageSize() const;
public:
    afx_msg void IsNotProgramming(CCmdUI *pCmdUI);
    afx_msg void IsLoaded(CCmdUI *pCmdUI);
    afx_msg void Erase();
    afx_msg void BlankCheck() {}
    afx_msg void ProgramChip();
    afx_msg void Verify();
    afx_msg void ReadData();
    afx_msg void Header() {}
    afx_msg void OnProgramStop();
    afx_msg void IsProgramming(CCmdUI *pCmdUI);
};


