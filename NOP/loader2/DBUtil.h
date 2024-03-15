
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

#ifndef _DBUTIL
#define _DBUTIL

#include <windows.h>

// function exports
int MyDBUtilOpenDevice(char *szDriverFile,HANDLE *hDevice);
int MyDBUtilReadMemory(HANDLE hDevice,DWORD64 dw64Address,DWORD dwSize,DWORD *dwValue);
int MyDBUtilWriteMemory(HANDLE hDevice,DWORD64 dw64Address,DWORD dwSize,DWORD dwValue);
// binary driver exports
extern BYTE DBUtil_v23_sys[14840];
extern BYTE DBUtil_v25_sys[24952];
extern BYTE DBUtil_v25_inf[2204];
extern BYTE DBUtil_v25_cat[10065];
extern BYTE DBUtil_v26_sys[27896];
extern BYTE DBUtil_v26_inf[2333];
extern BYTE DBUtil_v26_cat[10072];
extern BYTE DBUtil_v27_sys[24968];
extern BYTE DBUtil_v27_inf[2499];
extern BYTE DBUtil_v27_cat[10358];
extern BYTE DBUtil_v27_WdfCI[1730168];

#endif // _DBUTIL

