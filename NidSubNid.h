#pragma once


// CNidSubNid dialog

class CNidSubNid : public CDialog
{
	DECLARE_DYNAMIC(CNidSubNid)

public:
	CNidSubNid(CWnd* pParent = NULL);   // standard constructor
	virtual ~CNidSubNid();

// Dialog Data
	enum { IDD = IDD_DIALOG3 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_nid;
public:
	CString m_subnid;
};
