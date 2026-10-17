// NVIDIA Broadcast Video Frame Generation Enabler for RTX 30 Series (Ampere sm_86)
// Standalone Proxy DLL (version.dll) - Zero external dependencies
#include <windows.h>
#include <stdint.h>

#pragma comment(linker, "/export:GetFileVersionInfoA=C:\\Windows\\System32\\version.GetFileVersionInfoA")
#pragma comment(linker, "/export:GetFileVersionInfoByHandle=C:\\Windows\\System32\\version.GetFileVersionInfoByHandle")
#pragma comment(linker, "/export:GetFileVersionInfoExA=C:\\Windows\\System32\\version.GetFileVersionInfoExA")
#pragma comment(linker, "/export:GetFileVersionInfoExW=C:\\Windows\\System32\\version.GetFileVersionInfoExW")
#pragma comment(linker, "/export:GetFileVersionInfoSizeA=C:\\Windows\\System32\\version.GetFileVersionInfoSizeA")
#pragma comment(linker, "/export:GetFileVersionInfoSizeExA=C:\\Windows\\System32\\version.GetFileVersionInfoSizeExA")
#pragma comment(linker, "/export:GetFileVersionInfoSizeExW=C:\\Windows\\System32\\version.GetFileVersionInfoSizeExW")
#pragma comment(linker, "/export:GetFileVersionInfoSizeW=C:\\Windows\\System32\\version.GetFileVersionInfoSizeW")
#pragma comment(linker, "/export:GetFileVersionInfoW=C:\\Windows\\System32\\version.GetFileVersionInfoW")
#pragma comment(linker, "/export:VerFindFileA=C:\\Windows\\System32\\version.VerFindFileA")
#pragma comment(linker, "/export:VerFindFileW=C:\\Windows\\System32\\version.VerFindFileW")
#pragma comment(linker, "/export:VerInstallFileA=C:\\Windows\\System32\\version.VerInstallFileA")
#pragma comment(linker, "/export:VerInstallFileW=C:\\Windows\\System32\\version.VerInstallFileW")
#pragma comment(linker, "/export:VerLanguageNameA=C:\\Windows\\System32\\version.VerLanguageNameA")
#pragma comment(linker, "/export:VerLanguageNameW=C:\\Windows\\System32\\version.VerLanguageNameW")
#pragma comment(linker, "/export:VerQueryValueA=C:\\Windows\\System32\\version.VerQueryValueA")
#pragma comment(linker, "/export:VerQueryValueW=C:\\Windows\\System32\\version.VerQueryValueW")

typedef int (__stdcall* PFN_cuDeviceGetAttribute)(int* pi, int attrib, int dev);
typedef int (__stdcall* PFN_cuModuleLoadData)(void** module, const void* image);

static PFN_cuDeviceGetAttribute Real_cuDeviceGetAttribute = nullptr;
static PFN_cuModuleLoadData Real_cuModuleLoadData = nullptr;
static HMODULE g_lastPatchedCore = nullptr;
static HMODULE g_lastPatchedVFG  = nullptr;

void PatchBroadcastCoreUI(HMODULE hCore) {
    if (!hCore || hCore == g_lastPatchedCore) return;
    __try {
        uint8_t* base = reinterpret_cast<uint8_t*>(hCore);
        PIMAGE_DOS_HEADER dos = reinterpret_cast<PIMAGE_DOS_HEADER>(base);
        PIMAGE_NT_HEADERS nt  = reinterpret_cast<PIMAGE_NT_HEADERS>(base + dos->e_lfanew);
        PIMAGE_SECTION_HEADER sec = IMAGE_FIRST_SECTION(nt);

        uint8_t* textStart = base + sec[0].VirtualAddress;
        DWORD textSize = sec[0].Misc.VirtualSize;

        const uint8_t pat[12] = { 0x81, 0xBB, 0xB8, 0xBD, 0x00, 0x00, 0x60, 0x01, 0x00, 0x00, 0x7C, 0x04 };
        const uint8_t patch[12] = { 0xC7, 0x83, 0xB8, 0xBD, 0x00, 0x00, 0xB0, 0x01, 0x00, 0x00, 0x90, 0x90 };

        for (DWORD i = 0; i + 16 < textSize; i++) {
            if (memcmp(textStart + i, pat, 12) == 0) {
                DWORD oldProt;
                if (VirtualProtect(textStart + i, 12, PAGE_EXECUTE_READWRITE, &oldProt)) {
                    memcpy(textStart + i, patch, 12);
                    VirtualProtect(textStart + i, 12, oldProt, &oldProt);
                    g_lastPatchedCore = hCore;
                }
                break;
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
}

int __stdcall Hook_cuDeviceGetAttribute(int* pi, int attrib, int dev) {
    int res = Real_cuDeviceGetAttribute(pi, attrib, dev);
    if (res == 0 && pi && attrib == 76 && *pi == 6) {
        *pi = 9;
    }
    return res;
}

int __stdcall Hook_cuModuleLoadData(void** module, const void* image) {
    uint8_t* ptr = const_cast<uint8_t*>(reinterpret_cast<const uint8_t*>(image));

    if (ptr && *reinterpret_cast<uint32_t*>(ptr) == 0xBA55ED50) {
        uint64_t fatSize = *reinterpret_cast<uint64_t*>(ptr + 8) + 16;
        DWORD oldProt;
        if (VirtualProtect(ptr, static_cast<SIZE_T>(fatSize), PAGE_READWRITE, &oldProt)) {
            for (size_t i = 0; i + 64 < fatSize; i++) {
                if (ptr[i] == 0x02 && ptr[i+1] == 0x00 && ptr[i+4] == 0x40 && ptr[i+5] == 0x00) {
                    uint32_t* pSm = reinterpret_cast<uint32_t*>(ptr + i + 28);
                    if (*pSm == 89) *pSm = 86;
                }
                if (ptr[i] == 0x7F && ptr[i+1] == 'E' && ptr[i+2] == 'L' && ptr[i+3] == 'F') {
                    uint32_t* pFlags = reinterpret_cast<uint32_t*>(ptr + i + 48);
                    if ((*pFlags & 0xFF) == 89) *pFlags = 0x00560556;
                }
            }
            VirtualProtect(ptr, static_cast<SIZE_T>(fatSize), oldProt, &oldProt);
        }
    }
    return Real_cuModuleLoadData(module, image);
}

void InstallHooksOnVFG(HMODULE hMod) {
    if (!hMod || hMod == g_lastPatchedVFG) return;
    __try {
        uint8_t* base = reinterpret_cast<uint8_t*>(hMod);
        MEMORY_BASIC_INFORMATION mbi;
        if (!VirtualQuery(base + 0x3B000, &mbi, sizeof(mbi)) || mbi.State != MEM_COMMIT) return;

        HMODULE hNvcuda = GetModuleHandleW(L"nvcuda.dll");
        if (!hNvcuda) return;

        void* fnGetAttr = GetProcAddress(hNvcuda, "cuDeviceGetAttribute");
        void* fnLoadData = GetProcAddress(hNvcuda, "cuModuleLoadData");
        if (!fnGetAttr || !fnLoadData) return;

        void** iatStart = reinterpret_cast<void**>(base + 0x3B000);
        void** iatEnd   = reinterpret_cast<void**>(base + 0x3B800);
        int hookedCount = 0;

        for (void** p = iatStart; p < iatEnd; p++) {
            if (*p == fnLoadData) {
                DWORD oldProt;
                if (VirtualProtect(p, 8, PAGE_READWRITE, &oldProt)) {
                    Real_cuModuleLoadData = reinterpret_cast<PFN_cuModuleLoadData>(*p);
                    *p = reinterpret_cast<void*>(Hook_cuModuleLoadData);
                    VirtualProtect(p, 8, oldProt, &oldProt);
                    hookedCount++;
                }
            } else if (*p == fnGetAttr) {
                DWORD oldProt;
                if (VirtualProtect(p, 8, PAGE_READWRITE, &oldProt)) {
                    Real_cuDeviceGetAttribute = reinterpret_cast<PFN_cuDeviceGetAttribute>(*p);
                    *p = reinterpret_cast<void*>(Hook_cuDeviceGetAttribute);
                    VirtualProtect(p, 8, oldProt, &oldProt);
                    hookedCount++;
                }
            }
        }
        if (hookedCount == 2) {
            g_lastPatchedVFG = hMod;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
}

DWORD WINAPI MonitorThread(LPVOID lpParam) {
    while (true) {
        HMODULE hCore = GetModuleHandleW(L"NvBroadcastCore.dll");
        if (hCore) PatchBroadcastCoreUI(hCore);
        else g_lastPatchedCore = nullptr;

        HMODULE hVFG = GetModuleHandleW(L"nvVFXVideoFrameGeneration.dll");
        if (hVFG) InstallHooksOnVFG(hVFG);
        else g_lastPatchedVFG = nullptr;

        Sleep(1);
    }
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        HMODULE hCore = GetModuleHandleW(L"NvBroadcastCore.dll");
        if (hCore) PatchBroadcastCoreUI(hCore);
        CreateThread(nullptr, 0, MonitorThread, nullptr, 0, nullptr);
    }
    return TRUE;
}