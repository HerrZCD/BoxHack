#pragma once
#ifndef BASE_PROCESS_UTIL_H_
#define BASE_PROCESS_UTIL_H_

#include <windows.h>

#include <tlHelp32.h>

template <typename T> T RPM(HANDLE hProcess, DWORD address) {
    T buffer;
    ReadProcessMemory(hProcess, (LPCVOID)address, &buffer, sizeof(T), NULL);
    return buffer;
}
template <typename T> T ReadMemory(DWORD address) { return *((T*)address); }
template <typename T> void WriteMemory(DWORD address, T content) {
    *((T*)address) = content;
}
template <typename T> T* PointMemory(DWORD address) { return (T*)address; }

template <typename T> DWORD protectMemory(DWORD address, DWORD prot) {
    DWORD oldProt;
    VirtualProtect((LPVOID)address, sizeof(T), prot, &oldProt);
    return oldProt;
}

//DWORD GetModuleBaseAddress(DWORD processId, const wchar_t* szModuleName)
//{
//    DWORD moduleBase = 0;
//    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);
//    if (hSnapshot != INVALID_HANDLE_VALUE) {
//        MODULEENTRY32 moduleEntry;
//        moduleEntry.dwSize = sizeof(MODULEENTRY32);
//        if (Module32First(hSnapshot, &moduleEntry)) {
//            do {
//                if (wcscmp(moduleEntry.szModule, szModuleName) == 0) {
//                    moduleBase = (DWORD)moduleEntry.modBaseAddr;
//                    break;
//                }
//            } while (Module32Next(hSnapshot, &moduleEntry));
//        }
//        CloseHandle(hSnapshot);
//    }
//    return moduleBase;
//}

char* GetModuleBase(const wchar_t* ModuleName, DWORD procID)
{
    MODULEENTRY32 ModuleEntry = { 0 };
    HANDLE SnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procID);

    if (!SnapShot) return NULL;

    ModuleEntry.dwSize = sizeof(ModuleEntry);

    if (!Module32First(SnapShot, &ModuleEntry)) return NULL;

    do
    {
        if (!wcscmp(ModuleEntry.szModule, ModuleName))
        {
            CloseHandle(SnapShot);
            return (char*)ModuleEntry.modBaseAddr;
        }
    } while (Module32Next(SnapShot, &ModuleEntry));

    CloseHandle(SnapShot);
    return NULL;
}
#endif // BASE_PROCESS_UTIL_H_