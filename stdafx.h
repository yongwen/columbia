// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//	$Revision: 3 $

#if !defined(AFX_STDAFX_H__1801B689_FC90_11D0_8172_ED1BA299B528__INCLUDED_)
#define AFX_STDAFX_H__1801B689_FC90_11D0_8172_ED1BA299B528__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxtempl.h>
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <assert.h>
#include <math.h>
#include "time.h"
#include <sys/timeb.h>
#include <direct.h>
#include <fstream.h>

#include "wcol.h"
#include "defs.h"
#ifdef USE_MEMORY_MANAGER
	#include "bm.h"
#endif
#include "supp.h"

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__1801B689_FC90_11D0_8172_ED1BA299B528__INCLUDED_)
