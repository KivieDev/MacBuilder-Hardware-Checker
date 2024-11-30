#include "CPUChecks.h"
#include "Logger.h"

std::string getIntelCodename(int family, int model, int cpuid) {
    if (family == 6) {
        switch (model) {
        case 158: return "Coffee Lake 8th Gen (Supported)";
        case 165: return "Comet Lake (Supported)";
        case 142: return "Kaby Lake (Supported)";
        case 78:  return "Skylake (Partially Supported)";
        case 61:  return "Broadwell (Supported)";
        case 60:  return "Haswell (Supported)";
        case 58:  return "Ivy Bridge (Supported)";
        case 42:  return "Sandy Bridge (Partially Supported)";
        case 28:  return "Nehalem (Supported)";
        case 26:  return "Westmere (Partially Supported)";
        case 86:  return "Skylake-X (Partially Supported)";
        case 94:  return "Kaby Lake-X (Supported)";
        case 95:  return "Coffee Lake Refresh (Supported)";
        case 126: return "Rocket Lake (Limited Support)";
        case 103: return "Tiger Lake (Limited Support)";
        case 140: return "Tiger Lake (11th Gen, Limited Support)";
        case 134: return "Alder Lake (Unsupported)";
        case 133: return "Raptor Lake (Unsupported)";
        case 149: return "Meteor Lake (Unknown)";
        default:  return "Unknown Intel CPU";
        }
    }
    else if (family == 15) {
        switch (cpuid) {
        case 0x0F41: return "Pentium 4 (Unsupported)";
        case 0x0006E6: return "Yonah (Unsupported)";
        case 0x0006F2: return "Conroe / Merom (Unsupported)";
        case 0x010676: return "Penryn (Unsupported)";
        case 0x0106A2: return "Nehalem (Unsupported)";
        case 0x0106E0: return "Lynnfield / Clarksfield (Unsupported)";
        case 0x0206C0: return "Westmere (Unsupported)";
        case 0x0206A0: return "Sandy Bridge (Unsupported)";
        case 0x0306A0: return "Ivy Bridge (Unsupported)";
        case 0x0306E0: return "Ivy Bridge-E5 (Unsupported)";
        case 0x0306C0: return "Haswell (Unsupported)";
        case 0x0306D4: return "Broadwell (Unsupported)";
        case 0x0506E3: return "Skylake (Partially Supported)";
        case 0x0906E9: return "Kaby Lake (Supported)";
        case 0x0906EA: return "Coffee Lake (Supported)";
        case 0x0806E0: return "Amber / Whiskey (Unsupported)";
        case 0x0906E0: return "Comet Lake (Supported)";
        case 0x0706E5: return "Ice Lake (Partially Supported)";
        case 0x0A0671: return "Rocket Lake (Limited Support)";
        default: return "Unknown Intel CPU";
        }
    }
    return "Unknown or Unsupported Intel CPU";
}

std::string getAMDCodename(int family, int model) {
    if (family == 23) {
        switch (model) {
        case 1: case 8: case 17: case 18: return "Zen (Limited Support)";
        case 49: case 71:                return "Zen+ (Limited Support)";
        case 96: case 113:               return "Zen 2 (Partially Supported)";
        case 144: case 160:              return "Zen 3 (Partially Supported)";
        case 153: case 174:              return "Zen 4 (Unsupported)";
        case 123:                       return "Zen 3+ (Limited Support)";
        default: return "Unknown AMD CPU";
        }
    }
    else if (family == 21) {
        return "Bulldozer (Unsupported)";
    }
    else if (family == 22) {
        return "Piledriver (Unsupported)";
    }
    else if (family == 29) {
        return "Excavator (Unsupported)";
    }
    else if (family == 25) {
        return "Jaguar (Unsupported)";
    }
    return "Unknown or Unsupported AMD CPU";
}

bool checkCPUSupport(const CPUInfo& cpu) {
    if (!cpu.is64Bit) {
        if (cpu.cores <= 24) {
            Logger::getInstance().log(LogLevel::INFO, "Found CPU: " + cpu.name + " is supported for macOS with 32-bit.");
        }
        else {
            Logger::getInstance().log(LogLevel::Error, "Found CPU: " + cpu.name + " exceeds the core/thread count limit for 32-bit.");
            return false;
        }
        return true;
    }

    if (!cpu.supportsSSE3) {
        Logger::getInstance().log(LogLevel::Error, "Found CPU: " + cpu.name + " does not meet the SSE3 requirements.");
        return false;
    }

    std::string sseSupport;
    if (cpu.supportsSSE42) {
        sseSupport = "SSE4.2";
    }
    else if (cpu.supportsSSE4) {
        sseSupport = "SSE4";
    }
    else {
        sseSupport = "SSE3";
    }

    Logger::getInstance().log(LogLevel::INFO, "Found CPU: " + cpu.name + " is supported for macOS with 64-bit and " + sseSupport + ".");

    if (cpu.isAMD) {
        Logger::getInstance().log(LogLevel::WARNING, "Found CPU: " + cpu.name + " is an AMD CPU. Note that some macOS features may be unsupported.");
    }

    return true;
}

CPUInfo getCPUInfo() {
    CPUInfo cpuInfo;
    IWbemLocator* pLoc = NULL;
    IWbemServices* pSvc = NULL;

    HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
        Logger::getInstance().log(LogLevel::Error, "Failed to initialize COM library.");
        return cpuInfo;
    }

    hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);

    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc);

    if (FAILED(hres)) {
        Logger::getInstance().log(LogLevel::Error, "Failed to create IWbemLocator object.");
        CoUninitialize();
        return cpuInfo;
    }

    hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL,
        0, 0, &pSvc);

    if (FAILED(hres)) {
        Logger::getInstance().log(LogLevel::Error, "Could not connect to WMI namespace.");
        pLoc->Release();
        CoUninitialize();
        return cpuInfo;
    }

    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT * FROM Win32_Processor"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (0 == uReturn) {
            break;
        }

        VARIANT vtProp;
        pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
        cpuInfo.name = _com_util::ConvertBSTRToString(vtProp.bstrVal);
        VariantClear(&vtProp);

        pclsObj->Get(L"NumberOfCores", 0, &vtProp, 0, 0);
        cpuInfo.cores = vtProp.uintVal;
        VariantClear(&vtProp);

        pclsObj->Get(L"Architecture", 0, &vtProp, 0, 0);
        cpuInfo.is64Bit = (vtProp.uintVal == 9);
        VariantClear(&vtProp);

        cpuInfo.supportsSSE3 = false;
        cpuInfo.supportsSSE4 = false;
        cpuInfo.supportsSSE42 = false;

        int cpuInfoCPUID[4];
        __cpuid(cpuInfoCPUID, 1);
        int family = (cpuInfoCPUID[0] >> 8) & 0xF;
        int model = ((cpuInfoCPUID[0] >> 4) & 0xF) | ((cpuInfoCPUID[0] >> 12) & 0xF0);
        int cpuid = cpuInfoCPUID[0];

        std::string intelCodename = getIntelCodename(family, model, cpuid);
        std::string amdCodename = getAMDCodename(family, model);

        if (intelCodename != "Unknown or Unsupported Intel CPU") {
            Logger::getInstance().log(LogLevel::INFO, "Detected Intel Codename: " + intelCodename + " (Family: " + std::to_string(family) + ", Model: " + std::to_string(model) + ", CPUID: " + std::to_string(cpuid) + ")");
            cpuInfo.isAMD = false;
            cpuInfo.isIntel = true;
            cpuInfo.codename = intelCodename;
        }
        else if (amdCodename != "Unknown or Unsupported AMD CPU") {
            Logger::getInstance().log(LogLevel::INFO, "Detected AMD Codename: " + amdCodename + " (Family: " + std::to_string(family) + ", Model: " + std::to_string(model) + ")");
            cpuInfo.isAMD = true;
            cpuInfo.isIntel = false;
            cpuInfo.codename = amdCodename;
        }
        else {
            Logger::getInstance().log(LogLevel::Error, "CPU not supported for Hackintosh.");
        }

        int cpuFeatureInfo[4];
        __cpuid(cpuFeatureInfo, 1);

        cpuInfo.supportsSSE3 = (cpuFeatureInfo[2] & (1 << 0)) != 0;
        cpuInfo.supportsSSE4 = (cpuFeatureInfo[2] & (1 << 19)) != 0;
        cpuInfo.supportsSSE42 = (cpuFeatureInfo[2] & (1 << 20)) != 0;

        pclsObj->Release();
    }

    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();
    return cpuInfo;
}
