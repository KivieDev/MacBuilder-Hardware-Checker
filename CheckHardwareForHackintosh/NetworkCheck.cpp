#include "NetworkCheck.h"
#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <Wbemidl.h>
#include <comdef.h>
#include <map>
#include <sstream>
#pragma comment(lib, "wbemuuid.lib")


NetworkInfo checkNetworkAdapterSupport() {
    std::string adapterName;
    bool isSupported = true;

    HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
        return { "Failed to initialize COM", false };
    }

    hres = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);

    if (FAILED(hres)) {
        CoUninitialize();
        return { "Failed to initialize security", false };
    }

    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc);

    if (FAILED(hres)) {
        CoUninitialize();
        return { "Failed to create IWbemLocator", false };
    }

    IWbemServices* pSvc = NULL;
    hres = pLoc->ConnectServer(_bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);

    if (FAILED(hres)) {
        pLoc->Release();
        CoUninitialize();
        return { "Failed to connect to WMI", false };
    }

    hres = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE);

    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return { "Failed to set proxy blanket", false };
    }

    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(bstr_t("WQL"),
        bstr_t("SELECT Name, DeviceID FROM Win32_NetworkAdapter WHERE AdapterTypeID = 0"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL, &pEnumerator);

    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return { "Failed to execute query", false };
    }

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;

    std::map<std::string, std::string> incompatibleDevices = { };

    while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
        if (0 == uReturn) {
            break;
        }

        VARIANT vtProp;
        hr = pclsObj->Get(L"Name", 0, &vtProp, 0, 0);
        adapterName = vtProp.bstrVal ? _com_util::ConvertBSTRToString(vtProp.bstrVal) : "Unknown";
        VariantClear(&vtProp);

        hr = pclsObj->Get(L"DeviceID", 0, &vtProp, 0, 0);
        std::string fullDeviceId = vtProp.bstrVal ? _com_util::ConvertBSTRToString(vtProp.bstrVal) : "Unknown";
        VariantClear(&vtProp);

        std::size_t pos = fullDeviceId.find('\\');
        std::string vendorId, deviceId;
        if (pos != std::string::npos) {
            vendorId = fullDeviceId.substr(pos + 5, 4);
            deviceId = fullDeviceId.substr(pos + 10, 4);
        }

        if (incompatibleDevices.find(vendorId + ":" + deviceId) != incompatibleDevices.end()) {
            isSupported = false;
        }

        pclsObj->Release();
        pSvc->Release();
        pLoc->Release();
        pEnumerator->Release();
        CoUninitialize();

        return { adapterName, isSupported };
    }

    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();

    return { "No network adapter found", false };
}