#include "StorageCheck.h"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <Wbemidl.h>
#include <comdef.h>
#pragma comment(lib, "wbemuuid.lib")

StorageInfo checkStorageSupport() {
    bool isSupported = true;
    std::string driveModel;

    HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
        return { driveModel, false };
    }

    hres = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL
    );

    if (FAILED(hres)) {
        CoUninitialize();
        return { driveModel, false };
    }

    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc);

    if (FAILED(hres)) {
        CoUninitialize();
        return { driveModel, false };
    }

    IWbemServices* pSvc = NULL;

    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL,
        NULL,
        0,
        NULL,
        0,
        0,
        &pSvc
    );

    if (FAILED(hres)) {
        pLoc->Release();
        CoUninitialize();
        return { driveModel, false };
    }

    hres = CoSetProxyBlanket(
        pSvc,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE
    );

    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return { driveModel, false };
    }

    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT Model, InterfaceType FROM Win32_DiskDrive"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return { driveModel, false };
    }

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (0 == uReturn) {
            break;
        }

        VARIANT vtPropModel, vtPropInterface;
        hr = pclsObj->Get(L"Model", 0, &vtPropModel, 0, 0);
        hr = pclsObj->Get(L"InterfaceType", 0, &vtPropInterface, 0, 0);

        driveModel = vtPropModel.bstrVal ? _com_util::ConvertBSTRToString(vtPropModel.bstrVal) : "Unknown";
        std::string interfaceType = vtPropInterface.bstrVal ? _com_util::ConvertBSTRToString(vtPropInterface.bstrVal) : "Unknown";
        VariantClear(&vtPropModel);
        VariantClear(&vtPropInterface);

        if (interfaceType == "USB" || driveModel.find("Generic") != std::string::npos) {
            isSupported = false;
            driveModel += " (External/Unsupported)";
        }

        for (const auto& unsupported : { "Samsung PM981", "Samsung PM991", "Micron 2200S", "Intel 600p", "Samsung 970 EVO Plus", "Intel Optane", "Micron 3D XPoint" }) {
            if (driveModel.find(unsupported) != std::string::npos) {
                isSupported = false;
            }
        }

        pclsObj->Release();
    }

    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();

    return { driveModel, isSupported };
}
