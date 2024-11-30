#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <Windows.h>
#include <comdef.h>
#include <Wbemidl.h>
#include <intrin.h>
#include "GPUChecks.h"
#include <map>


#pragma comment(lib, "wbemuuid.lib")

struct CPUInfo {
    std::string name;
    std::string codename;
    int cores;
    bool is64Bit;
    bool supportsSSE3;
    bool supportsSSE4;
    bool supportsSSE42;
    bool isIntel;
    bool isAMD;
};

bool checkCPUSupport(const CPUInfo& cpu);

CPUInfo getCPUInfo();