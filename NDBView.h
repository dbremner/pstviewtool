// NDBView.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

class NDBViewCommands : public CCommandLineInfo
{
public:
	NDBViewCommands() : m_paramNum(0), m_fConsistencyReport(FALSE), m_fValid(FALSE) {}
	virtual void ParseParam(const TCHAR* pszParam, BOOL bFlag, BOOL bLast);
	BOOL FValid() { return m_fValid; }
	BOOL DoCC() { return FValid() && m_fConsistencyReport; }
	CString GetFilename() { return m_filename; }

private:
	int m_paramNum;
	CString m_filename;
	BOOL m_fValid;
	BOOL m_fConsistencyReport;
};

// CNDBViewApp:
// See NDBView.cpp for the implementation of this class
//

class CNDBViewApp : public CWinApp
{
public:
	CNDBViewApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CNDBViewApp theApp;