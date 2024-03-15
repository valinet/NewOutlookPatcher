
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

#ifndef _MYFUNCTIONS
#define _MYFUNCTIONS

// Attention: The library file msvcrt.lib is copied from "C:\WinDDK\7600.16385.1\lib\Crt\amd64".
// This import library reduces the executable size and we have not to install any Microsoft
// redistributables to run the executable.

#define APPNAME "DSE-Patcher"
#define VERSION "V1.0"
#define BUILD   "Build 20221127"

// deprecate unsafe function warnings e.g. strcpy, sprintf
#define _CRT_SECURE_NO_DEPRECATE

#include <windows.h>
#include <commctrl.h>
// sprintf
#include <stdio.h>
// NtQuerySystemInformation
#include <winternl.h>
#include <SetupAPI.h>
// MAX_CLASS_NAME_LEN
#include <cfgmgr32.h>
// UpdateDriverForPlugAndPlayDevices
#include <newdev.h>
// ACL function e.g. SetNamedSecurityInfo
#include <Aclapi.h>
// PathFileExists
#include <Shlwapi.h>
#include "MyDialog1.h"

// SetupAPI functions
#pragma comment(lib,"setupapi.lib")
// UpdateDriverForPlugAndPlayDevices
#pragma comment(lib,"newdev.lib")
// PathFileExists
#pragma comment(lib,"shlwapi.lib")

// maximum number of supported vulnerable drivers
#define MAX_VULNERABLE_DRIVERS 5
// maximum number of supported driver files
#define MAX_DRIVER_FILES 4

// NtQuerySystemInformation structures
typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
	HANDLE Section;
	PVOID MappedBase;
	PVOID ImageBase;
	ULONG ImageSize;
	ULONG Flags;
	USHORT LoadOrderIndex;
	USHORT InitOrderIndex;
	USHORT LoadCount;
	USHORT OffsetToFileName;
	UCHAR FullPathName[256];
}RTL_PROCESS_MODULE_INFORMATION,*PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES
{
	ULONG NumberOfModules;
	RTL_PROCESS_MODULE_INFORMATION Modules[1];
}RTL_PROCESS_MODULES,*PRTL_PROCESS_MODULES;

// forward declaration of structure for use in function pointers of start and stop driver
struct _VULNERABLE_DRIVER;

// vulnerable driver structure function prototypes
typedef int (*FunctionOpenDevice)(char *szDriverFile,HANDLE *hDevice);
typedef int (*FunctionReadMemory)(HANDLE hDevice,DWORD64 dw64Address,DWORD dwSize,DWORD *dwValue);
typedef int (*FunctionWriteMemory)(HANDLE hDevice,DWORD64 dw64Address,DWORD dwSize,DWORD dwValue);
typedef int (*FunctionStartDriver)(_VULNERABLE_DRIVER *vd);
typedef int (*FunctionStopDriver)(_VULNERABLE_DRIVER *vd);

// driver files structure
typedef struct _DRIVER_FILE
{
	char szFilePath[MAX_PATH];
	BYTE *bData;
	DWORD dwSize;
}DRIVER_FILE,*PDRIVER_FILE;

// vulnerable driver structure
typedef struct _VULNERABLE_DRIVER
{
	const char *szProvider;
	const char *szToolTipText;
	FunctionOpenDevice pFunctionOpenDevice;
	FunctionReadMemory pFunctionReadMemory;
	FunctionWriteMemory pFunctionWriteMemory;
	FunctionStartDriver pFunctionStartDriver;
	FunctionStopDriver pFunctionStopDriver;
	const char *szServiceName;
	const char *szDriverSymLink;
	// reserve space for max number of driver files (sys, inf, cat and WDFCoInstaller DLL)
	DRIVER_FILE driverFile[MAX_DRIVER_FILES];
	const char *szHardwareId;
	HDEVINFO DeviceInfoSet;
	SP_DEVINFO_DATA DeviceInfoData;
	DWORD dwMinSupportedBuildNumber;
	DWORD dwMaxSupportedBuildNumber;
}VULNERABLE_DRIVER,*PVULNERABLE_DRIVER;

// patch data structure
typedef struct _PATCH_DATA
{
	// operating system
	const char *szOS;
	// module to patch
	const char *szModuleName;
	// variable name in module to patch e.g. g_CiEnabled, g_CiOptions
	const char *szVariableName;
	// DSE original value
	DWORD dwDSEOriginalValue;
	// DSE disable value
	DWORD dwDSEDisableValue;
	// DSE enable value
	DWORD dwDSEEnableValue;
	// DSE actual value
	DWORD dwDSEActualValue;
	// patch size in bytes
	DWORD dwPatchSize;
	// image base of module to patch
	UINT64 ui64ImageBase;
	// image size of module to patch
	ULONG ulImageSize;
	// variable address to patch
	UINT64 ui64PatchAddress;
	// DSE status
	const char *szDSEStatus;
}PATCH_DATA,*PPATCH_DATA;

// thread task number enumeration
typedef enum
{
	ThreadTaskReadDSEOnFirstRun = 1,
	ThreadTaskDisableDSE = 2,
	ThreadTaskEnableDSE = 3,
	ThreadTaskRestoreDSE = 4
}THREAD_TASK_NO;

// thread parameter structure
typedef struct _THREAD_PARAMS
{
	THREAD_TASK_NO ttno;
}THREAD_PARAMS,*PTHREAD_PARAMS;

// dialog1 structure
typedef struct _DIALOG1
{
	HWND hDialog1;
	HWND hButton1;
	HWND hButton2;
	HWND hButton3;
	HWND hCombo1;
	HWND hStatic1;
	HWND hStatusBar1;
	unsigned int uiTimerSeconds;
	unsigned int uiTimerMinutes;
	unsigned int uiTimerHours;
}DIALOG1,*PDIALOG1;

// globals structure
typedef struct _GLOBALS
{
	DIALOG1 Dlg1;
	HMODULE hInstance;
	unsigned char ucRunning;
	THREAD_PARAMS ThreadParams;
	VULNERABLE_DRIVER vd[MAX_VULNERABLE_DRIVERS];
	PATCH_DATA pd;
	char szMsg[1024];
}GLOBALS,*PGLOBALS;

//------------------------------------------------------------------------------
// exported functions
//------------------------------------------------------------------------------

int MyInitVulnerableDrivers(VULNERABLE_DRIVER *vd,DWORD dwElements);
DWORD WINAPI MyThreadProc1(PVOID pvoid);

#endif // _MYFUNCTIONS

