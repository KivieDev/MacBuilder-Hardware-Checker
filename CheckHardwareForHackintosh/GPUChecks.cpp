#include "GPUChecks.h"
#include <iostream>
#include <string>
#include <Wbemidl.h>
#include <comdef.h>
#include "Logger.h"

#pragma comment(lib, "wbemuuid.lib")

std::string detectGPU() {
    HRESULT hres;
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
        return "Failed to initialize COM library.";
    }

    hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);

    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc);

    IWbemServices* pSvc = NULL;
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL,
        NULL,
        0,
        NULL,
        0,
        0,
        &pSvc);

    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT Name FROM Win32_VideoController"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;

    std::string gpuModel;

    while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (0 == uReturn) {
            break;
        }

        VARIANT vtProp;
        hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
        gpuModel = _com_util::ConvertBSTRToString(vtProp.bstrVal);
        VariantClear(&vtProp);
        pclsObj->Release();
    }

    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();

    return gpuModel;
}

std::string getSupportedMacOSVersion(const std::string& gpuModel) {
    if (gpuModel.find("GCN") != std::string::npos) {
        Logger::getInstance().log(LogLevel::INFO, "Found GPU model: " + gpuModel);
        return "Supported in macOS 10.13 and later (High Sierra and up)";
    }
    if (gpuModel.find("Polaris") != std::string::npos) {
        Logger::getInstance().log(LogLevel::INFO, "Found GPU model: " + gpuModel);
        return "Supported in macOS 10.12 and later (Sierra and up)";
    }
    if (gpuModel.find("Vega") != std::string::npos) {
        Logger::getInstance().log(LogLevel::INFO, "Found GPU model: " + gpuModel);
        return "Supported in macOS 10.13 and later (High Sierra and up)";
    }
    if (gpuModel.find("Navi 10") != std::string::npos) {
        Logger::getInstance().log(LogLevel::INFO, "Found GPU model: " + gpuModel + " - Requires agdpmod=pikera in boot-args");
        return "Supported in macOS 10.15.1 and later";
    }
    if (gpuModel.find("Navi 20") != std::string::npos || gpuModel.find("RX 6600") != std::string::npos) {
        Logger::getInstance().log(LogLevel::INFO, "Found GPU model: " + gpuModel);
        return "Supported in macOS 11.4 and later";
    }
    if (gpuModel.find("Radeon RX 6600") != std::string::npos || gpuModel.find("RDNA 2") != std::string::npos) {
        Logger::getInstance().log(LogLevel::INFO, "Found GPU model: " + gpuModel);
        return "May be supported starting from macOS 11 with patches (Big Sur and later)";
    }

    if (gpuModel.find("GeForce 6") != std::string::npos || gpuModel.find("GeForce 7") != std::string::npos ||
        gpuModel.find("Kepler") != std::string::npos) {
        Logger::getInstance().log(LogLevel::INFO, "Found GPU model: " + gpuModel);
        return "Supported up to macOS 11: Big Sur";
    }
    if (gpuModel.find("Maxwell") != std::string::npos || gpuModel.find("Pascal") != std::string::npos) {
        Logger::getInstance().log(LogLevel::INFO, "Found GPU model: " + gpuModel);
        return "Supported only up to macOS 10.13: High Sierra";
    }
    if (gpuModel.find("Turing") != std::string::npos || gpuModel.find("Ampere") != std::string::npos) {
        Logger::getInstance().log(LogLevel::INFO, "Found GPU model: " + gpuModel);
        return "Not supported in any macOS version";
    }

    if (gpuModel.find("Intel HD Graphics") != std::string::npos || gpuModel.find("Intel(R) HD Graphics") != std::string::npos ||
        gpuModel.find("Intel UHD Graphics") != std::string::npos || gpuModel.find("Intel(R) UHD Graphics") != std::string::npos ||
        gpuModel.find("Intel Iris") != std::string::npos || gpuModel.find("Intel(R) Iris") != std::string::npos) {
        Logger::getInstance().log(LogLevel::INFO, "Found GPU model: " + gpuModel);
        return "Supported in macOS 10.12 and later (Sierra and up)";
    }

    Logger::getInstance().log(LogLevel::Error, "Not Foudnd GPU model: " + gpuModel);
    return "The detected GPU is NOT supported for macOS.";
}

std::string getSupportedMacOSVersionC(const std::string& gpuModel) {
    if (gpuModel.find("GCN") != std::string::npos) {
        return "10.13+";
    }
    if (gpuModel.find("Polaris") != std::string::npos) {
        return "10.12+";
    }
    if (gpuModel.find("Vega") != std::string::npos) {
        return "10.13+";
    }
    if (gpuModel.find("Navi 10") != std::string::npos) {
        return "10.15+";
    }
    if (gpuModel.find("Navi 20") != std::string::npos || gpuModel.find("RX 6600") != std::string::npos) {
        return "11.4+";
    }
    if (gpuModel.find("Radeon RX 6600") != std::string::npos || gpuModel.find("RDNA 2") != std::string::npos) {
        return "11+";
    }

    if (gpuModel.find("GeForce 6") != std::string::npos || gpuModel.find("GeForce 7") != std::string::npos ||
        gpuModel.find("Kepler") != std::string::npos) {
        return "11-";
    }
    if (gpuModel.find("Maxwell") != std::string::npos || gpuModel.find("Pascal") != std::string::npos) {
        return "10.13-";
    }
    if (gpuModel.find("Turing") != std::string::npos || gpuModel.find("Ampere") != std::string::npos) {
        return "";
    }

    if (gpuModel.find("Intel HD Graphics") != std::string::npos || gpuModel.find("Intel(R) HD Graphics") != std::string::npos ||
        gpuModel.find("Intel UHD Graphics") != std::string::npos || gpuModel.find("Intel(R) UHD Graphics") != std::string::npos ||
        gpuModel.find("Intel Iris") != std::string::npos || gpuModel.find("Intel(R) Iris") != std::string::npos) {
        return "10.12+";
    }
    return "";
}

bool getSupportedMacOSVersionB(const std::string& gpuModel) {
    if (gpuModel.find("GCN") != std::string::npos) {
        return true;
    }
    if (gpuModel.find("Polaris") != std::string::npos) {
        return true;
    }
    if (gpuModel.find("Vega") != std::string::npos) {
        return true;
    }
    if (gpuModel.find("Navi 10") != std::string::npos) {
        return true;
    }
    if (gpuModel.find("Navi 20") != std::string::npos || gpuModel.find("RX 6600") != std::string::npos) {
        return true;
    }
    if (gpuModel.find("Radeon RX 6600") != std::string::npos || gpuModel.find("RDNA 2") != std::string::npos) {
        return true;
    }

    if (gpuModel.find("GeForce 6") != std::string::npos || gpuModel.find("GeForce 7") != std::string::npos ||
        gpuModel.find("Kepler") != std::string::npos) {
        return true;
    }
    if (gpuModel.find("Maxwell") != std::string::npos || gpuModel.find("Pascal") != std::string::npos) {
        return true;
    }
    if (gpuModel.find("Turing") != std::string::npos || gpuModel.find("Ampere") != std::string::npos) {
        return true;
    }

    if (gpuModel.find("Intel HD Graphics") != std::string::npos || gpuModel.find("Intel(R) HD Graphics") != std::string::npos ||
        gpuModel.find("Intel UHD Graphics") != std::string::npos || gpuModel.find("Intel(R) UHD Graphics") != std::string::npos ||
        gpuModel.find("Intel Iris") != std::string::npos || gpuModel.find("Intel(R) Iris") != std::string::npos) {
        return true;
    }
    return false;
}

bool checkGPU(const std::string& gpuModel) {
    
    return getSupportedMacOSVersionB(gpuModel);
}

std::string trimSpaces(const std::string& str) {
    size_t first = str.find_first_not_of(' ');
    if (std::string::npos == first) {
        return str;
    }
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, (last - first + 1));
}