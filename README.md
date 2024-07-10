# System Information Gatherer

The System Information Gatherer is a cross-platform project that provides comprehensive system information, including CPU, RAM, and GPU details. This project consists of a native C++ DLL and a C# wrapper to facilitate easy integration with .NET applications.

## Features

- Retrieve CPU name, core count, and usage statistics.
- Retrieve operating system information.
- Retrieve total and used RAM.
- Retrieve GPU information.

## Project Structure

- `SystemInfo.hpp`: Header file defining the interface for the system information functions.
- `SystemInfo.cpp`: Implementation of the system information functions for different platforms.
- `SystemInfoWrapper.cs`: C# wrapper to provide access to the system information from .NET applications.
- `Program.cs`: Example .NET console application demonstrating how to use the `SystemInfoWrapper` class.

## Building the Project

### Prerequisites

- CMake
- Visual Studio (for Windows)
- A C++ compiler (e.g., GCC for Linux)
- .NET SDK

### Steps

1. Clone the repository:
    ```bash
    git clone https://github.com/yourusername/SystemInformationGatherer.git
    cd SystemInformationGatherer
    ```

2. Build the native C++ DLL:

    On Windows:
    ```bash
    mkdir build
    cd build
    cmake ..
    cmake --build .
    ```

    On Linux:
    ```bash
    mkdir build
    cd build
    cmake ..
    make
    ```

3. Build the C# wrapper and example application:

    ```bash
    cd ../SystemInfoSharp
    dotnet build
    ```

## Usage

### Using the C++ Library

Include the `SystemInfo.hpp` header in your C++ project and link against the compiled DLL.

### Using the C# Wrapper

Add a reference to the `SystemInfoSharp` project or DLL in your .NET application. Then, use the `SystemInfoWrapper` class to retrieve system information.

Example:

```csharp
using System;
using SystemInfoSharp;

class Program
{
    static void Main(string[] args)
    {
        Console.WriteLine($"CPU Name: {SystemInfoWrapper.CPUName}");
        var (physicalCores, logicalCores) = SystemInfoWrapper.CPUCores;
        Console.WriteLine($"CPU Cores: {physicalCores} physical, {logicalCores} logical");
        Console.WriteLine($"OS Info: {SystemInfoWrapper.OSInfo}");
        Console.WriteLine($"Total RAM: {SystemInfoWrapper.TotalRAM:F2} GB");
        Console.WriteLine($"Used RAM: {SystemInfoWrapper.UsedRAM:F2} GB");
        Console.WriteLine("GPU Info:");
        Console.WriteLine(SystemInfoWrapper.GPUInfo);

        var (totalCPUUsage, coreUsages) = SystemInfoWrapper.CPUUsage;
        Console.WriteLine($"Total CPU Usage: {totalCPUUsage:F2}%");
        Console.WriteLine("CPU Core Usages:");
        for (int i = 0; i < coreUsages.Length; i++)
        {
            Console.WriteLine($"  Core {i}: {coreUsages[i]:F2}%");
        }
    }
}
```
