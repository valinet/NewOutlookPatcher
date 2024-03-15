#include <wrl.h>
#include <wil/com.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Web.Syndication.h>
#include <iostream>
#include <Windows.h>
#include <commctrl.h>
#include "WebView2.h"
#include "WebView2EnvironmentOptions.h"

#pragma region "IAT patching routine"
// https://blog.neteril.org/blog/2016/12/23/diverting-functions-windows-iat-patching/
inline bool VnPatchIAT(HMODULE hMod, const char* libName, const char* funcName, uintptr_t hookAddr) {
    // Increment module reference count to prevent other threads from unloading it while we're working with it
    HMODULE module;
    if (!::GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)hMod, &module)) return false;

    // Get a reference to the import table to locate the kernel32 entry
    PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)module;
    PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)((uintptr_t)module + dos->e_lfanew);
    PIMAGE_IMPORT_DESCRIPTOR importDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((uintptr_t)module +
        nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    // In the import table find the entry that corresponds to kernel32
    bool found = false;
    while (importDescriptor->Characteristics && importDescriptor->Name) {
        PSTR importName = (PSTR)((PBYTE)module + importDescriptor->Name);
        if (::_stricmp(importName, libName) == 0) { found = true; break; }
        importDescriptor++;
    }
    if (!found) { ::FreeLibrary(module); return false; }

    // From the kernel32 import descriptor, go over its IAT thunks to
    // find the one used by the rest of the code to call GetProcAddress
    PIMAGE_THUNK_DATA oldthunk = (PIMAGE_THUNK_DATA)((PBYTE)module + importDescriptor->OriginalFirstThunk);
    PIMAGE_THUNK_DATA thunk = (PIMAGE_THUNK_DATA)((PBYTE)module + importDescriptor->FirstThunk);
    while (thunk->u1.Function) {
        PROC* funcStorage = (PROC*)&thunk->u1.Function;

        bool bFound = false;
        if (oldthunk->u1.Ordinal & IMAGE_ORDINAL_FLAG) {
            bFound = (!(*((WORD*)&(funcName)+1)) && IMAGE_ORDINAL32(oldthunk->u1.Ordinal) == (DWORD_PTR)funcName);
        }
        else {
            PIMAGE_IMPORT_BY_NAME byName = (PIMAGE_IMPORT_BY_NAME)((uintptr_t)module + oldthunk->u1.AddressOfData);
            bFound = ((*((WORD*)&(funcName)+1)) && !::_stricmp((char*)byName->Name, funcName));
        }

        // Found it, now let's patch it
        if (bFound) {
            // Get the memory page where the info is stored
            MEMORY_BASIC_INFORMATION mbi;
            ::VirtualQuery(funcStorage, &mbi, sizeof(MEMORY_BASIC_INFORMATION));

            // Try to change the page to be writable if it's not already
            if (!::VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_READWRITE, &mbi.Protect)) {
                ::FreeLibrary(module);
                return false;
            }

            // Store our hook
            *funcStorage = (PROC)hookAddr;

            // Restore the old flag on the page
            DWORD dwOldProtect;
            ::VirtualProtect(mbi.BaseAddress, mbi.RegionSize, mbi.Protect, &dwOldProtect);

            // Profit
            ::FreeLibrary(module);
            return true;
        }

        thunk++;
        oldthunk++;
    }

    ::FreeLibrary(module);
    return false;
}
#pragma endregion

#pragma region "Exports"
/*
extern "C" {
    static HRESULT(*__CreateDXGIFactory1)(void*, void**);
    __declspec(dllexport) HRESULT _CreateDXGIFactory1(void* p1, void** p2)
    {
        MessageBoxW(nullptr, L"Hello, world !", L"", 0);
        return __CreateDXGIFactory1(p1, p2);
    }
}
#pragma comment(linker, "/export:CreateDXGIFactory1=_CreateDXGIFactory1,@11")
*/
#pragma comment(linker, "/export:ApplyCompatResolutionQuirking=C:\\Windows\\System32\\dxgi.dll.ApplyCompatResolutionQuirking,@1")
#pragma comment(linker, "/export:CompatString=C:\\Windows\\System32\\dxgi.dll.CompatString,@2")
#pragma comment(linker, "/export:CompatValue=C:\\Windows\\System32\\dxgi.dll.CompatValue,@3")
#pragma comment(linker, "/export:CreateDXGIFactory=C:\\Windows\\System32\\dxgi.dll.CreateDXGIFactory,@10")
#pragma comment(linker, "/export:CreateDXGIFactory1=C:\\Windows\\System32\\dxgi.dll.CreateDXGIFactory1,@11")
#pragma comment(linker, "/export:CreateDXGIFactory2=C:\\Windows\\System32\\dxgi.dll.CreateDXGIFactory2,@12")
#pragma comment(linker, "/export:DXGID3D10CreateDevice=C:\\Windows\\System32\\dxgi.dll.DXGID3D10CreateDevice,@13")
#pragma comment(linker, "/export:DXGID3D10CreateLayeredDevice=C:\\Windows\\System32\\dxgi.dll.DXGID3D10CreateLayeredDevice,@14")
#pragma comment(linker, "/export:DXGID3D10GetLayeredDeviceSize=C:\\Windows\\System32\\dxgi.dll.DXGID3D10GetLayeredDeviceSize,@15")
#pragma comment(linker, "/export:DXGID3D10RegisterLayers=C:\\Windows\\System32\\dxgi.dll.DXGID3D10RegisterLayers,@16")
#pragma comment(linker, "/export:DXGIDeclareAdapterRemovalSupport=C:\\Windows\\System32\\dxgi.dll.DXGIDeclareAdapterRemovalSupport,@17")
#pragma comment(linker, "/export:DXGIDumpJournal=C:\\Windows\\System32\\dxgi.dll.DXGIDumpJournal,@4")
#pragma comment(linker, "/export:DXGIGetDebugInterface1=C:\\Windows\\System32\\dxgi.dll.DXGIGetDebugInterface1,@18")
#pragma comment(linker, "/export:DXGIReportAdapterConfiguration=C:\\Windows\\System32\\dxgi.dll.DXGIReportAdapterConfiguration,@19")
#pragma comment(linker, "/export:PIXBeginCapture=C:\\Windows\\System32\\dxgi.dll.PIXBeginCapture,@5")
#pragma comment(linker, "/export:PIXEndCapture=C:\\Windows\\System32\\dxgi.dll.PIXEndCapture,@6")
#pragma comment(linker, "/export:PIXGetCaptureState=C:\\Windows\\System32\\dxgi.dll.PIXGetCaptureState,@7")
#pragma comment(linker, "/export:SetAppCompatStringPointer=C:\\Windows\\System32\\dxgi.dll.SetAppCompatStringPointer,@8")
#pragma comment(linker, "/export:UpdateHMDEmulationStatus=C:\\Windows\\System32\\dxgi.dll.UpdateHMDEmulationStatus,@9")
#pragma endregion

#pragma region "Hooks"
/*
LRESULT(*__WndProc)(HWND, UINT, WPARAM, LPARAM) = nullptr;
LRESULT _WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    return __WndProc(hWnd, uMsg, wParam, lParam);
}
*/

HRESULT(*__ICoreWebView2CreateCoreWebView2ControllerCompletedHandler_Invoke)(ICoreWebView2CreateCoreWebView2ControllerCompletedHandler* _this, HRESULT, ICoreWebView2Controller*) = nullptr;
HRESULT STDMETHODCALLTYPE _ICoreWebView2CreateCoreWebView2ControllerCompletedHandler_Invoke(ICoreWebView2CreateCoreWebView2ControllerCompletedHandler* _this, HRESULT errorCode, ICoreWebView2Controller* createdController) {
    if (createdController != nullptr) {
        winrt::com_ptr<ICoreWebView2> webview;
        winrt::check_hresult(createdController->get_CoreWebView2(webview.put()));

        EventRegistrationToken tkn_NavigationCompleted;
        winrt::check_hresult(webview->add_NavigationCompleted(Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>([](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {

            auto script = L"\
const styleElement = document.createElement('style');\n\
const cssClass = \"\
#OwaContainer, "                                                     /* First "email" ad when online */ L"\
.kk1xx._Bfyd.iIsOF.IjQyD, "                                          /* First "email" ad when offline */ L"\
.syTot, "                                                            /* Lower left OneDrive subscription banner */ L"\
[id='34318026-c018-414b-abb3-3e32dfb9cc4c'], "                       /* Word button in sidebar */ L"\
[id='c5251a9b-a95d-4595-91ee-a39e6eed3db2'], "                       /* Excel button in sidebar */ L"\
[id='48cb9ead-1c19-4e1f-8ed9-3d60a7e52b18'], "                       /* PowerPoint button in sidebar */ L"\
[id='39109bd4-9389-4731-b8d6-7cc1a128d0b3'], "                       /* OneDrive button in sidebar */ L"\
.___1fkhojs.f22iagw.f122n59.f1vx9l62.f1c21dwh.fqerorx.f1i5mqs4, "     /* More apps button in sidebar */ L"\
[id='D64D0004-2A11-442B-9586-F49009D4852B'] { display: none !important; }\";\n\
styleElement.appendChild(document.createTextNode(cssClass));\n\
document.head.appendChild(styleElement);\n\
";
            // .root-192, .splitButtonMenuButton-220 { background-color: transparent !important; color: var(--neutralDark) !important; } " /* Deemphasize New mail button */ L"\

            //::MessageBoxW(nullptr, script, L"", 0);
            sender->ExecuteScript(script, Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>([&](HRESULT errorCode, LPCWSTR resultObjectAsJson) -> HRESULT {
                return S_OK;
                }).Get());

            return S_OK;
            }).Get(), &tkn_NavigationCompleted));

        volatile int dummyF12Enabled = 0;
        const wchar_t* isF12Enabled = L"y_1A36CD25-E20F-4D0D-B1E6-3CC4307E1488";
        if (isF12Enabled[0 + dummyF12Enabled] == L'y') {
            EventRegistrationToken tkn_AcceleratorKeyPressed;
            winrt::check_hresult(createdController->add_AcceleratorKeyPressed(Microsoft::WRL::Callback<ICoreWebView2AcceleratorKeyPressedEventHandler>([](ICoreWebView2Controller* sender, ICoreWebView2AcceleratorKeyPressedEventArgs* args) -> HRESULT {

                COREWEBVIEW2_KEY_EVENT_KIND kind;
                winrt::check_hresult(args->get_KeyEventKind(&kind));
                if (kind == COREWEBVIEW2_KEY_EVENT_KIND_KEY_UP) {
                    UINT key;
                    winrt::check_hresult(args->get_VirtualKey(&key));
                    if (key == VK_F12) {
                        winrt::check_hresult(args->put_Handled(true));
                        winrt::com_ptr<ICoreWebView2> webview;
                        winrt::check_hresult(sender->get_CoreWebView2(webview.put()));
                        webview->OpenDevToolsWindow();
                    }
                }

                return S_OK;
                }).Get(), &tkn_AcceleratorKeyPressed));
        }

        /*
        HWND parentWindow = nullptr;
        createdController->get_ParentWindow(&parentWindow);
        ::SetLastError(0);
        __WndProc = reinterpret_cast<LRESULT(*)(HWND, UINT, WPARAM, LPARAM)>(::GetWindowLongPtrW(parentWindow, GWLP_WNDPROC));
        if (::GetLastError() == ERROR_SUCCESS && __WndProc) {
            ::SetWindowLongPtrW(parentWindow, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(_WndProc));
        }
        */
    }

    //::MessageBoxW(nullptr, L"Hello from _ICoreWebView2CreateCoreWebView2ControllerCompletedHandler_Invoke", L"", 0);
    return __ICoreWebView2CreateCoreWebView2ControllerCompletedHandler_Invoke(_this, errorCode, createdController);
}

HRESULT(*__ICoreWebView2Environment_CreateCoreWebView2Controller)(ICoreWebView2Environment*, HWND, ICoreWebView2CreateCoreWebView2ControllerCompletedHandler*) = nullptr;
HRESULT STDMETHODCALLTYPE _ICoreWebView2Environment_CreateCoreWebView2Controller(ICoreWebView2Environment* _this, HWND parentWindow, ICoreWebView2CreateCoreWebView2ControllerCompletedHandler* controllerCompletedHandler) {
    void** controllerCompletedHandlerVtbl = *(void***)controllerCompletedHandler;
    if (controllerCompletedHandlerVtbl[3] != _ICoreWebView2CreateCoreWebView2ControllerCompletedHandler_Invoke) {
        //::MessageBoxW(nullptr, L"Patching controllerCompletedHandlerVtbl", L"", 0);
        DWORD oldProtect = 0;
        if (::VirtualProtect(&controllerCompletedHandlerVtbl[3], sizeof(uintptr_t), PAGE_EXECUTE_READWRITE, &oldProtect)) {
            __ICoreWebView2CreateCoreWebView2ControllerCompletedHandler_Invoke = reinterpret_cast<HRESULT(*)(ICoreWebView2CreateCoreWebView2ControllerCompletedHandler*, HRESULT, ICoreWebView2Controller*)>(controllerCompletedHandlerVtbl[3]);
            controllerCompletedHandlerVtbl[3] = _ICoreWebView2CreateCoreWebView2ControllerCompletedHandler_Invoke;
            ::VirtualProtect(&controllerCompletedHandlerVtbl[3], sizeof(uintptr_t), oldProtect, &oldProtect);
        }
    }

    //::MessageBoxW(nullptr, L"Hello from _ICoreWebView2Environment_CreateCoreWebView2Controller", L"", 0);
    return __ICoreWebView2Environment_CreateCoreWebView2Controller(_this, parentWindow, controllerCompletedHandler);
}

HRESULT(*__ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler_Invoke)(ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* _this, HRESULT, ICoreWebView2Environment*) = nullptr;
HRESULT STDMETHODCALLTYPE _ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler_Invoke(ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* _this, HRESULT errorCode, ICoreWebView2Environment* createdEnvironment) {
    void** createdEnvironmentVtbl = *(void***)createdEnvironment;
    if (createdEnvironmentVtbl[3] != _ICoreWebView2Environment_CreateCoreWebView2Controller) {
        //::MessageBoxW(nullptr, L"Patching createdEnvironmentVtbl", L"", 0);
        DWORD oldProtect = 0;
        if (::VirtualProtect(&createdEnvironmentVtbl[3], sizeof(uintptr_t), PAGE_EXECUTE_READWRITE, &oldProtect)) {
            __ICoreWebView2Environment_CreateCoreWebView2Controller = reinterpret_cast<HRESULT(*)(ICoreWebView2Environment*, HWND, ICoreWebView2CreateCoreWebView2ControllerCompletedHandler*)>(createdEnvironmentVtbl[3]);
            createdEnvironmentVtbl[3] = _ICoreWebView2Environment_CreateCoreWebView2Controller;
            ::VirtualProtect(&createdEnvironmentVtbl[3], sizeof(uintptr_t), oldProtect, &oldProtect);
        }
    }

    //::MessageBoxW(nullptr, L"Hello from _ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler_Invoke", L"", 0);
    return __ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler_Invoke(_this, errorCode, createdEnvironment);
}

HRESULT(*__CreateCoreWebView2EnvironmentWithOptions)(PCWSTR, PCWSTR, ICoreWebView2EnvironmentOptions*, ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*) = nullptr;
STDAPI _CreateCoreWebView2EnvironmentWithOptions(PCWSTR browserExecutableFolder, PCWSTR userDataFolder, ICoreWebView2EnvironmentOptions* environmentOptions, ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler* environmentCreatedHandler) {
    void** environmentCreatedHandlerVtbl = *(void***)environmentCreatedHandler;
    if (environmentCreatedHandlerVtbl[3] != _ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler_Invoke) {
        //::MessageBoxW(nullptr, L"Patching environmentCreatedHandlerVtbl", L"", 0);
        DWORD oldProtect = 0;
        if (::VirtualProtect(&environmentCreatedHandlerVtbl[3], sizeof(uintptr_t), PAGE_EXECUTE_READWRITE, &oldProtect)) {
            __ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler_Invoke = reinterpret_cast<HRESULT(*)(ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*, HRESULT, ICoreWebView2Environment*)>(environmentCreatedHandlerVtbl[3]);
            environmentCreatedHandlerVtbl[3] = _ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler_Invoke;
            ::VirtualProtect(&environmentCreatedHandlerVtbl[3], sizeof(uintptr_t), oldProtect, &oldProtect);
        }
    }

    if (!__CreateCoreWebView2EnvironmentWithOptions) {
        auto hMod = ::GetModuleHandleW(L"WebView2Loader.dll");
        winrt::check_bool(hMod);
        __CreateCoreWebView2EnvironmentWithOptions = reinterpret_cast<HRESULT(*)(PCWSTR, PCWSTR, ICoreWebView2EnvironmentOptions*, ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler*)>(::GetProcAddress(hMod, "CreateCoreWebView2EnvironmentWithOptions"));
        winrt::check_bool(__CreateCoreWebView2EnvironmentWithOptions);
    }
    //::MessageBoxW(nullptr, L"Hello from _CreateCoreWebView2EnvironmentWithOptions", L"", 0);
    return __CreateCoreWebView2EnvironmentWithOptions(browserExecutableFolder, userDataFolder, environmentOptions, environmentCreatedHandler);
}
#pragma endregion

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    UNREFERENCED_PARAMETER(lpvReserved);

    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        ::DisableThreadLibraryCalls(hinstDLL);
        //__CreateDXGIFactory1 = (HRESULT(*)(void*, void**))::GetProcAddress(::GetModuleHandleW(L"C:\\Windows\\System32\\dxgi.dll"), "CreateDXGIFactory1");
        ::VnPatchIAT(::GetModuleHandleW(nullptr), "WebView2Loader.dll", "CreateCoreWebView2EnvironmentWithOptions", reinterpret_cast<uintptr_t>(_CreateCoreWebView2EnvironmentWithOptions));
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    }
    return true;
}
