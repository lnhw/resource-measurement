#include <Windows.h>
#include <Psapi.h>
#include <iostream>
#include <string>
#include <vector>

#pragma comment(lib, "psapi.lib")

int main()
{
    // Get the process ID of each running process.
    DWORD processes[1024];
    DWORD bytesNeeded;
    if (!EnumProcesses(processes, sizeof(processes), &bytesNeeded))
    {
        std::cerr << "Error: EnumProcesses() failed.\n";
        return 1;
    }
    DWORD numProcesses = bytesNeeded / sizeof(DWORD);

    // Get the handle of the current process.
    HANDLE currentProcess = GetCurrentProcess();

    // Get the frequency of the performance counter.
    LARGE_INTEGER frequency;
    if (!QueryPerformanceFrequency(&frequency))
    {
        std::cerr << "Error: QueryPerformanceFrequency() failed.\n";
        return 1;
    }

    // Print the CPU usage and memory usage of each process.
    std::cout << "PID  CPU Usage  Memory Usage (KB)  Name\n";
    std::cout << "---  ---------  -----------------  ----\n";
    for (DWORD i = 0; i < numProcesses; ++i)
    {
        DWORD processID = processes[i];
        if (processID == 0) continue;

        // Get the handle of the process.
        HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
        if (processHandle == nullptr) continue;

        // Get the process name.
        char processName[MAX_PATH];
        if (GetModuleFileNameExA(processHandle, nullptr, processName, MAX_PATH) == 0)
        {
            CloseHandle(processHandle);
            continue;
        }

        // Get the CPU usage of the process.
        FILETIME creationTime, exitTime, kernelTime, userTime;
        if (!GetProcessTimes(processHandle, &creationTime, &exitTime, &kernelTime, &userTime))
        {
            CloseHandle(processHandle);
            continue;
        }
        ULARGE_INTEGER kernelTimeInt, userTimeInt;
        kernelTimeInt.LowPart = kernelTime.dwLowDateTime;
        kernelTimeInt.HighPart = kernelTime.dwHighDateTime;
        userTimeInt.LowPart = userTime.dwLowDateTime;
        userTimeInt.HighPart = userTime.dwHighDateTime;
        ULONGLONG kernelTime64 = kernelTimeInt.QuadPart;
        ULONGLONG userTime64 = userTimeInt.QuadPart;
        double cpuUsage = (kernelTime64 + userTime64) * 100.0 / frequency.QuadPart;

        // Get the memory usage of the process.
        PROCESS_MEMORY_COUNTERS_EX memoryCounters;
        if (!GetProcessMemoryInfo(processHandle, reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&memoryCounters), sizeof(memoryCounters)))
        {
            CloseHandle(processHandle);
            continue;
        }
        SIZE_T memoryUsage = memoryCounters.PrivateUsage / 1024;

        // Print the process information.
        std::cout << processID << "  " << cpuUsage << "%  " << memoryUsage << " KB  " << processName << '\n';

        CloseHandle(processHandle);
    }

    return 0;
}
