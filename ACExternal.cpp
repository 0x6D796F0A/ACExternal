#include <Windows.h>
#include <iostream>
#include <processthreadsapi.h>
#include <tlhelp32.h>
#include <wchar.h>
#include <tchar.h> 


#define LOCAL_PLAYER_OFFSET 0x17E0A8
#define HEALTH_OFFSET 0xEC

void viewModuleInfo(MODULEENTRY32* m)
{
    _tprintf(TEXT("\n\n     MODULE NAME:     %s"), m->szModule);
    _tprintf(TEXT("\n     executable     = %s"), m->szExePath);
    _tprintf(TEXT("\n     process ID     = 0x%08X"), m->th32ProcessID);
    _tprintf(TEXT("\n     ref count (g)  =     0x%04X"), m->GlblcntUsage);
    _tprintf(TEXT("\n     ref count (p)  =     0x%04X"), m->ProccntUsage);
    _tprintf(TEXT("\n     base address   = 0x%08X"), (DWORD)m->modBaseAddr);
    _tprintf(TEXT("\n     base size      = %d"), m->modBaseSize);
}

void GetProcessIdAndModuleBaseAddressFromName(const WCHAR* name, DWORD* processId, BYTE** moduleBaseAddress)
{
    PROCESSENTRY32 p;
    p.dwSize = sizeof(PROCESSENTRY32);

    MODULEENTRY32 m;
    m.dwSize = sizeof(MODULEENTRY32);

    HANDLE processHandle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    

    if (Process32First(processHandle, &p) == TRUE)
    {
        while (Process32Next(processHandle, &p) == TRUE)
        {
            if (wcscmp(p.szExeFile, name) == 0)
            {
                *processId = p.th32ProcessID;
                break;
            }
                
        }
    }

    CloseHandle(processHandle);

    HANDLE moduleHandle = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, p.th32ProcessID);

    if (Module32First(moduleHandle, &m) == TRUE)
    {
        //viewModuleInfo(&m);
        if (wcscmp(m.szModule, name) == 0)
        {
            //viewModuleInfo(&m);
            *moduleBaseAddress = m.modBaseAddr;
        }  
    }

    CloseHandle(moduleHandle);
}

int main()
{
    DWORD processId = NULL;
    BYTE* moduleBaseAddress = NULL;

    // Current health offset to use: [ac_client.exe+17E0A8] + EC
    GetProcessIdAndModuleBaseAddressFromName(L"ac_client.exe", &processId, &moduleBaseAddress);

    if (processId == NULL)
    {
        printf("ProcessID is null\n");
        return EXIT_FAILURE;
    }

    if (moduleBaseAddress == NULL)
    {
        printf("Module Base Address is null\n");
        return EXIT_FAILURE;
    }

    printf("Process ID: %lu\n", processId);
    printf("Module Base Address: %p\n", moduleBaseAddress);

    // Get handle to process
    HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, false, processId);

    if (handle == NULL)
    {
        printf("Open Process Failed\n");
        return EXIT_FAILURE;
    }

    printf("Got handle to process!\n");

    while (true)
    {
        system("cls");
        LPCVOID localPlayerPtr = moduleBaseAddress + LOCAL_PLAYER_OFFSET;
        LPVOID localPlayer = NULL;

        // Read Health at [ac_client.exe+17E0A8] + EC
        if (ReadProcessMemory(handle, localPlayerPtr, &localPlayer, sizeof(localPlayer), NULL) == 0)
            printf("Getting Local Player Failed.");

        LPCVOID healthAddress = (BYTE*)localPlayer + HEALTH_OFFSET;

        int health = NULL;
        ReadProcessMemory(handle, healthAddress, &health, sizeof(health), NULL);

        printf("Health address is %p\n", healthAddress);
        printf("Health is %i\n", health);
        Sleep(200);
    }
    
    

    CloseHandle(handle);
}
