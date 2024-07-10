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