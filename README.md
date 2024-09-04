# Try to allocate max size memory for each device

For each device found, query the max memory allocation size, and try to allocate device memory of that size.

## Build

### Prerequisites
Microsoft Visual Studio 2022 (64-bit) Version 17.10.5 or later with the "Desktop development with C++" Workload installed.

### Build
- Run a Developer PowerShell for Visual Studio 2022
- Change directory to AllocateMaxMemorySize
- Build and execute the reproducer

```
PS C:\code\AllocateMaxMemorySize> cmake.exe -B Build
PS C:\code\AllocateMaxMemorySize> cmake.exe --build Build --config DEBUG
PS C:\code\AllocateMaxMemorySize> .\Build\Debug\AllocateMaxMemorySize.exe
```
Example output 1: allocating the maximum size:
```
PS D:\code\github\AllocateMaxMemorySize> .\Build\Debug\AllocateMaxMemorySize.exe
AMD Radeon RX 7900 XT
max allocation size: 2147483648 bytes.
allocation size larger than maximum allowed, clamping...
allocating 2147483648 bytes.
successfully allocated 2147483648 bytes.

Intel(R) Arc(TM) A770 Graphics
max allocation size: 16777216000 bytes.
allocation size larger than maximum allowed, clamping...
allocating 16777216000 bytes.
VK_ERROR_OUT_OF_DEVICE_MEMORY

NVIDIA RTX A6000
max allocation size: 4292870144 bytes.
allocation size larger than maximum allowed, clamping...
allocating 4292870144 bytes.
successfully allocated 4292870144 bytes.
```

For AMD and NVIDIA, the maximum size (min of heap size from VkPhysicalDeviceMemoryProperties and max allocation size from VkPhysicalDeviceMaintenance3Properties) can be allocated successfully. On Intel, this size is too large and results in an out of memory error.

Example output 2: allocating 4 GiB: (note the '4' as an argument)
```
PS C:\Users\foeijord\code\AllocateMaxMemorySize> .\Build\Debug\AllocateMaxMemorySize.exe 4
AMD Radeon RX 7900 XT
max allocation size: 2147483648 bytes.
allocation size larger than maximum allowed, clamping...
allocating 2147483648 bytes.
successfully allocated 2147483648 bytes.

Intel(R) Arc(TM) A770 Graphics
max allocation size: 16777216000 bytes.
allocating 4294967296 bytes.

==> crash!

 	igvk64.dll!00007ffa7f274f7c()	Unknown
 	igvk64.dll!00007ffa7f5f7890()	Unknown
 	igvk64.dll!00007ffa7f5d2c50()	Unknown
 	igvk64.dll!00007ffa7f5a87ff()	Unknown
 	igvk64.dll!00007ffa7f526c5f()	Unknown
 	igvk64.dll!00007ffa7f5261fc()	Unknown
 	igvk64.dll!00007ffa7f4d04f1()	Unknown
 	VkLayer_khronos_validation.dll!DispatchAllocateMemory(VkDevice_T * device, const VkMemoryAllocateInfo * pAllocateInfo, const VkAllocationCallbacks * pAllocator, VkDeviceMemory_T * * pMemory) Line 498	C++
 	VkLayer_khronos_validation.dll!vulkan_layer_chassis::AllocateMemory(VkDevice_T * device, const VkMemoryAllocateInfo * pAllocateInfo, const VkAllocationCallbacks * pAllocator, VkDeviceMemory_T * * pMemory) Line 1580	C++
 	[External Code]	
>	AllocateMaxMemorySize.exe!VulkanMemory::VulkanMemory(std::shared_ptr<VulkanDevice> device, unsigned __int64 allocationSize, unsigned int memoryTypeIndex) Line 282	C++
 	[External Code]	
 	AllocateMaxMemorySize.exe!main(int argc, char * * argv) Line 44	C++
 	[External Code]	
```