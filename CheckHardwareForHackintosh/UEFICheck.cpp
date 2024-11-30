#include "UEFICheck.h"

bool isUEFISupported() {
    FIRMWARE_TYPE firmwareType;
    BOOL result = GetFirmwareType(&firmwareType);

    if (result) {
        if (firmwareType == FirmwareTypeUefi) {
            return true;
        }
    }

    return false;
}