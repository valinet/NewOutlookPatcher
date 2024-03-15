
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

#ifndef _RTCORE64
#define _RTCORE64

#include <windows.h>

// function exports
int MyRTCore64OpenDevice(char *szDriverFile,HANDLE *hDevice);
int MyRTCore64ReadMemory(HANDLE hDevice,DWORD64 dw64Address,DWORD dwSize,DWORD *dwValue);
int MyRTCore64WriteMemory(HANDLE hDevice,DWORD64 dw64Address,DWORD dwSize,DWORD dwValue);
// binary driver export
extern BYTE RTCore64Driver[14024];

#endif // _RTCORE64

