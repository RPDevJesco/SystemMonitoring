#define SYSTEMINFO_EXPORTS
#include <thread>
#include <chrono>
#include "SystemInfo.hpp"
#include <sstream>
#include <codecvt>
#include <locale>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winternl.h>
#include <ntstatus.h>
#include <intrin.h>
#include <iphlpapi.h>
#include <wbemidl.h>
#include <comdef.h>
#include <dxgi.h>
#include <pdh.h>
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "pdh.lib")

static PDH_HQUERY cpuQuery;
static PDH_HCOUNTER cpuTotal;

void InitializeCPUUsage() {
    PdhOpenQuery(NULL, NULL, &cpuQuery);
    PdhAddCounter(cpuQuery, "\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
    PdhCollectQueryData(cpuQuery);
}

double GetCurrentCPUUsage() {
    PDH_FMT_COUNTERVALUE counterVal;
    PdhCollectQueryData(cpuQuery);
    PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
    return counterVal.doubleValue;
}

#elif defined(__APPLE__) || defined(__linux__)
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <fstream>
#endif

const int CPU_BRAND_STRING_PART1 = 0x80000002;
const int CPU_BRAND_STRING_PART2 = 0x80000003;
const int CPU_BRAND_STRING_PART3 = 0x80000004;
const int CPU_BRAND_STRING_LENGTH = 0x40;  // 64 bytes
const int CPU_BRAND_CHUNK_SIZE = 16;  // Each __cpuid call returns 16 bytes

std::string SystemInfo::GetCPUName() {
#ifdef _WIN32
    int cpuInfo[4] = {-1};
    char cpuBrandString[CPU_BRAND_STRING_LENGTH];

    __cpuid(cpuInfo, CPU_BRAND_STRING_PART1);
    memcpy(cpuBrandString, cpuInfo, CPU_BRAND_CHUNK_SIZE);

    __cpuid(cpuInfo, CPU_BRAND_STRING_PART2);
    memcpy(cpuBrandString + CPU_BRAND_CHUNK_SIZE, cpuInfo, CPU_BRAND_CHUNK_SIZE);

    __cpuid(cpuInfo, CPU_BRAND_STRING_PART3);
    memcpy(cpuBrandString + (2 * CPU_BRAND_CHUNK_SIZE), cpuInfo, CPU_BRAND_CHUNK_SIZE);

    return std::string(cpuBrandString);
#elif defined(__APPLE__) || defined(__linux__)
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.substr(0, 10) == "model name") {
            return line.substr(line.find(":") + 2);
        }
    }
    return "Unknown CPU";
#else
    return "Unsupported platform";
#endif
}

CPUCoreInfo SystemInfo::GetCPUCores() {
#ifdef _WIN32
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    int logicalCores = sysInfo.dwNumberOfProcessors;

    DWORD length = 0;
    GetLogicalProcessorInformation(nullptr, &length);
    std::vector<SYSTEM_LOGICAL_PROCESSOR_INFORMATION> buffer(length / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION));
    GetLogicalProcessorInformation(&buffer[0], &length);

    int physicalCores = 0;
    for (const auto& info : buffer) {
        if (info.Relationship == RelationProcessorCore) {
            physicalCores++;
        }
    }

    return {physicalCores, logicalCores};
#elif defined(__APPLE__) || defined(__linux__)
    int logicalCores = sysconf(_SC_NPROCESSORS_ONLN);
    // Note: Getting physical cores on Unix systems is more complex and may require parsing /proc/cpuinfo
    // This is a simplification
    int physicalCores = logicalCores / 2;  // Assuming SMT is enabled
    return {physicalCores, logicalCores};
#else
    return {-1, -1};
#endif
}

std::string SystemInfo::GetOSInfo() {
#ifdef _WIN32
    typedef NTSTATUS (WINAPI *RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
    HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
    if (hNtdll) {
        RtlGetVersionPtr RtlGetVersion = (RtlGetVersionPtr)GetProcAddress(hNtdll, "RtlGetVersion");
        if (RtlGetVersion) {
            RTL_OSVERSIONINFOW osvi = {0};
            osvi.dwOSVersionInfoSize = sizeof(osvi);
            if (NT_SUCCESS(RtlGetVersion(&osvi))) {
                std::string windowsVersion;
                if (osvi.dwMajorVersion == 10) {
                    if (osvi.dwBuildNumber >= 22000) {
                        windowsVersion = "11";
                    } else {
                        windowsVersion = "10";
                    }
                } else if (osvi.dwMajorVersion == 6) {
                    switch (osvi.dwMinorVersion) {
                        case 0: windowsVersion = "Vista"; break;
                        case 1: windowsVersion = "7"; break;
                        case 2: windowsVersion = "8"; break;
                        case 3: windowsVersion = "8.1"; break;
                        default: windowsVersion = "Unknown";
                    }
                } else {
                    windowsVersion = "Unknown";
                }

                return "Windows " + windowsVersion + " (Version " +
                       std::to_string(osvi.dwMajorVersion) + "." +
                       std::to_string(osvi.dwMinorVersion) +
                       ", Build " + std::to_string(osvi.dwBuildNumber) + ")";
            }
        }
    }
    return "Windows (Version unknown)";
#elif defined(__APPLE__) || defined(__linux__)
    struct utsname unameData;
    uname(&unameData);
    return std::string(unameData.sysname) + " " + unameData.release;
#else
    return "Unsupported platform";
#endif
}

double SystemInfo::GetTotalRAM() {
#ifdef _WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        return static_cast<double>(memInfo.ullTotalPhys) / (1024.0 * 1024.0 * 1024.0);
    }
    return 0.0;
#elif defined(__APPLE__) || defined(__linux__)
    struct sysinfo memInfo;
    if (sysinfo(&memInfo) == 0) {
        return static_cast<double>(memInfo.totalram * memInfo.mem_unit) / (1024.0 * 1024.0 * 1024.0);
    }
    return 0.0;
#else
    return 0.0;
#endif
}

std::vector<std::string> SystemInfo::GetGPUInfo() {
    std::vector<std::string> gpuInfo;

#ifdef _WIN32
    IDXGIFactory* pFactory;
    HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&pFactory));

    if (FAILED(hr)) {
        gpuInfo.push_back("Failed to create DXGIFactory");
        return gpuInfo;
    }

    IDXGIAdapter* pAdapter;
    for (UINT i = 0; pFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
        DXGI_ADAPTER_DESC adapterDesc;
        pAdapter->GetDesc(&adapterDesc);

        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        std::string description = converter.to_bytes(adapterDesc.Description);

        std::stringstream ss;
        ss << description << "\n"
           << "  Dedicated Video Memory: " << adapterDesc.DedicatedVideoMemory / (1024 * 1024) << " MB\n"
           << "  Dedicated System Memory: " << adapterDesc.DedicatedSystemMemory / (1024 * 1024) << " MB\n"
           << "  Shared System Memory: " << adapterDesc.SharedSystemMemory / (1024 * 1024) << " MB\n"
           << "  Vendor ID: 0x" << std::hex << adapterDesc.VendorId << "\n"
           << "  Device ID: 0x" << std::hex << adapterDesc.DeviceId << "\n"
           << "  Subsystem ID: 0x" << std::hex << adapterDesc.SubSysId;

        gpuInfo.push_back(ss.str());
        pAdapter->Release();
    }

    pFactory->Release();

#elif defined(__APPLE__)
    // ... (keep existing macOS implementation)
#elif defined(__linux__)
    // ... (keep existing Linux implementation)
#else
    gpuInfo.push_back("Unsupported platform");
#endif

    return gpuInfo;
}

std::vector<double> SystemInfo::GetCPUCoreUsage() {
#ifdef _WIN32
    PDH_FMT_COUNTERVALUE counterVal;
    std::vector<double> coreUsages;
    const int numSamples = 5;
    const int sampleIntervalMs = 100;

    for (int i = 0; i < SystemInfo::GetCPUCores().logicalCores; ++i) {
        HQUERY hQuery;
        HCOUNTER hCounter;
        std::string counterPath = "\\Processor(" + std::to_string(i) + ")\\% Processor Time";

        PdhOpenQuery(NULL, NULL, &hQuery);
        PdhAddCounter(hQuery, counterPath.c_str(), NULL, &hCounter);
        double totalUsage = 0.0;

        for (int sample = 0; sample < numSamples; ++sample) {
            PdhCollectQueryData(hQuery);
            std::this_thread::sleep_for(std::chrono::milliseconds(sampleIntervalMs));
            PdhCollectQueryData(hQuery);
            PdhGetFormattedCounterValue(hCounter, PDH_FMT_DOUBLE, NULL, &counterVal);
            totalUsage += counterVal.doubleValue;
        }

        coreUsages.push_back(totalUsage / numSamples);
        PdhCloseQuery(hQuery);
    }

    return coreUsages;
#else
    return std::vector<double>(); // Placeholder for non-Windows implementation
#endif
}

CPUUsageInfo SystemInfo::GetCPUUsage() {
    CPUUsageInfo usageInfo;
    usageInfo.totalUsage = 0.0;
    const int numSamples = 5;
    const int sampleIntervalMs = 100;

    for (int sample = 0; sample < numSamples; ++sample) {
        usageInfo.totalUsage += GetCurrentCPUUsage();
        std::this_thread::sleep_for(std::chrono::milliseconds(sampleIntervalMs));
    }
    usageInfo.totalUsage /= numSamples;

    std::vector<double> coreUsageVec = GetCPUCoreUsage();
    usageInfo.coreCount = coreUsageVec.size();
    usageInfo.coreUsage = new double[usageInfo.coreCount];
    std::copy(coreUsageVec.begin(), coreUsageVec.end(), usageInfo.coreUsage);

    return usageInfo;
}

double SystemInfo::GetUsedRAM() {
#ifdef _WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    return static_cast<double>(memInfo.ullTotalPhys - memInfo.ullAvailPhys) / (1024.0 * 1024.0 * 1024.0);
#elif defined(__APPLE__) || defined(__linux__)
    struct sysinfo memInfo;
    sysinfo(&memInfo);
    return static_cast<double>(memInfo.totalram - memInfo.freeram) * memInfo.mem_unit / (1024.0 * 1024.0 * 1024.0);
#else
    return 0.0;
#endif
}

// GPU usage retrieval requires vendor-specific APIs, e.g., NVIDIA's NVML
std::vector<double> SystemInfo::GetGPUUsage() {
    std::vector<double> gpuUsage;

#ifdef _WIN32
    // Implement NVIDIA NVML or similar here
    gpuUsage.push_back(0.0); // Placeholder
#elif defined(__APPLE__)
    // Implement macOS GPU usage retrieval here
    gpuUsage.push_back(0.0); // Placeholder
#elif defined(__linux__)
    // Implement Linux GPU usage retrieval here
    gpuUsage.push_back(0.0); // Placeholder
#endif

    return gpuUsage;
}

// C-style interface implementation
extern "C" {
    DLL_API const char* __stdcall GetCPUName() {
        static std::string cpuName = SystemInfo::GetCPUName();
        return cpuName.c_str();
    }

    DLL_API CPUCoreInfo __stdcall GetCPUCores() {
        return SystemInfo::GetCPUCores();
    }

    DLL_API const char* __stdcall GetOSInfo() {
        static std::string osInfo = SystemInfo::GetOSInfo();
        return osInfo.c_str();
    }

    DLL_API double __stdcall GetTotalRAM() {
        return SystemInfo::GetTotalRAM();
    }

    DLL_API double __stdcall GetUsedRAM() {
        return SystemInfo::GetUsedRAM();
    }

    DLL_API const char* __stdcall GetGPUInfo() {
        static std::string gpuInfoStr;
        std::vector<std::string> gpuInfoVec = SystemInfo::GetGPUInfo();
        for (const auto& info : gpuInfoVec) {
            gpuInfoStr += info + "\n";
        }
        return gpuInfoStr.c_str();
    }

    DLL_API CPUUsageInfo __stdcall GetCPUUsage() {
        return SystemInfo::GetCPUUsage();
    }

    DLL_API const char* __stdcall GetGPUUsage() {
        static std::string gpuUsageStr;
        std::vector<double> gpuUsageVec = SystemInfo::GetGPUUsage();
        for (const auto& usage : gpuUsageVec) {
            gpuUsageStr += std::to_string(usage) + "%\n";
        }
        return gpuUsageStr.c_str();
    }

    DLL_API void __stdcall InitializeSystemInfo() {
        InitializeCPUUsage();
    }
}