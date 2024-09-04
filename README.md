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
Example output:
```
PS D:\code\github\AllocateMaxMemorySize> .\Build\Debug\AllocateMaxMemorySize.exe
device name: NVIDIA RTX A6000
max allocation size: 4292870144 bytes.
successfully allocated 4292870144 bytes.
```