#include <windows.h>
#include <iostream>
#include <sysinfoapi.h>
#include <intrin.h>

int getLogicalProcessorCount() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    return sysInfo.dwNumberOfProcessors;
}

bool isLaptop() {
    SYSTEM_POWER_STATUS powerStatus;
    if (GetSystemPowerStatus(&powerStatus)) {
        if (powerStatus.BatteryFlag != 128) {
            return true;
        }
    }
    return false;
}

bool isHighEndDesktop() {
    int processorCount = getLogicalProcessorCount();

    return processorCount >= 8;
}

std::string checkSystemType() {
    if (isLaptop()) {
        return "Laptop";
    }
    else {
        if (isHighEndDesktop()) {
            return "High-End Desktop";
        }
        else {
            return "Desktop";
        }
    }
}