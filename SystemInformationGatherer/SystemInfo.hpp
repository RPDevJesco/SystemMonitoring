#pragma once

#include <string>
#include <vector>
#include <cstdint>

#ifdef _WIN32
#ifdef SYSTEMINFO_EXPORTS
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif
#else
#define DLL_API
#endif

struct CPUCoreInfo {
    int physicalCores;
    int logicalCores;
};

struct CPUUsageInfo {
    double totalUsage;
    double* coreUsage;
    int coreCount;
};

class SystemInfo {
public:
    static std::string GetCPUName();
    static CPUCoreInfo GetCPUCores();
    static std::string GetOSInfo();
    static double GetTotalRAM();
    static double GetUsedRAM();
    static std::vector<std::string> GetGPUInfo();
    static CPUUsageInfo GetCPUUsage();
    static std::vector<double> GetGPUUsage();
    static std::vector<double> GetCPUCoreUsage();
    static void InitializeSystemInfo();
};

extern "C" {
    DLL_API const char* __stdcall GetCPUName();
    DLL_API CPUCoreInfo __stdcall GetCPUCores();
    DLL_API const char* __stdcall GetOSInfo();
    DLL_API double __stdcall GetTotalRAM();
    DLL_API double __stdcall GetUsedRAM();
    DLL_API const char* __stdcall GetGPUInfo();
    DLL_API CPUUsageInfo __stdcall GetCPUUsage();
    DLL_API const char* __stdcall GetGPUUsage();
    DLL_API void __stdcall InitializeSystemInfo();
}