using System;
using System.Runtime.InteropServices;

namespace SystemInfoSharp
{
    public class SystemInfoWrapper
    {
        private const string DllName = "SystemInfoGatherer";  // Without file extension

        [StructLayout(LayoutKind.Sequential)]
        private struct CPUCoreInfo
        {
            public int PhysicalCores;
            public int LogicalCores;
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct CPUUsageInfo
        {
            public double TotalUsage;
            public IntPtr CoreUsage;
            public int CoreCount;
        }

        [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
        private static extern CPUCoreInfo GetCPUCores();

        [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
        private static extern void InitializeSystemInfo();

        public static (int Physical, int Logical) CPUCores
        {
            get
            {
                var coreInfo = GetCPUCores();
                return (coreInfo.PhysicalCores, coreInfo.LogicalCores);
            }
        }

        [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
        private static extern IntPtr GetCPUName();

        [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
        private static extern IntPtr GetOSInfo();

        [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
        private static extern double GetTotalRAM();

        [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
        private static extern double GetUsedRAM();

        [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
        private static extern IntPtr GetGPUInfo();

        [DllImport(DllName, CallingConvention = CallingConvention.StdCall)]
        private static extern CPUUsageInfo GetCPUUsage();

        static SystemInfoWrapper()
        {
            InitializeSystemInfo();
        }

        public static string CPUName => Marshal.PtrToStringAnsi(GetCPUName());

        public static string OSInfo => Marshal.PtrToStringAnsi(GetOSInfo());

        public static double TotalRAM => GetTotalRAM();

        public static double UsedRAM => GetUsedRAM();

        public static string GPUInfo => Marshal.PtrToStringAnsi(GetGPUInfo());

        public static (double TotalUsage, double[] CoreUsage) CPUUsage
        {
            get
            {
                var usageInfo = GetCPUUsage();
                double[] coreUsages = new double[usageInfo.CoreCount];
                Marshal.Copy(usageInfo.CoreUsage, coreUsages, 0, usageInfo.CoreCount);
                return (usageInfo.TotalUsage, coreUsages);
            }
        }
    }
}