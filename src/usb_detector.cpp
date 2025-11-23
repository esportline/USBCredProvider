//
// USB Detector - Implémentation
//

#include "usb_detector.h"
#include <setupapi.h>
#include <devguid.h>
#include <cfgmgr32.h>
#include <initguid.h>
#include <usbiodef.h>
#include <algorithm>

// Define GUID_DEVINTERFACE_VOLUME if not already defined
#ifndef GUID_DEVINTERFACE_VOLUME
DEFINE_GUID(GUID_DEVINTERFACE_VOLUME, 0x53f5630d, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);
#endif

#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "cfgmgr32.lib")

std::vector<USBDetector::USBDevice> USBDetector::GetConnectedUSBDevices() {
    std::vector<USBDevice> devices;

    // Créer un ensemble d'informations de périphériques pour les périphériques de stockage USB
    HDEVINFO deviceInfoSet = SetupDiGetClassDevs(
        &GUID_DEVINTERFACE_USB_DEVICE,
        nullptr,
        nullptr,
        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
    );

    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        return devices;
    }

    SP_DEVINFO_DATA deviceInfoData;
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    // Énumérer tous les périphériques
    for (DWORD i = 0; SetupDiEnumDeviceInfo(deviceInfoSet, i, &deviceInfoData); i++) {
        // Récupérer l'ID d'instance du périphérique
        wchar_t instanceId[MAX_PATH];
        if (CM_Get_Device_ID(deviceInfoData.DevInst, instanceId, MAX_PATH, 0) != CR_SUCCESS) {
            continue;
        }

        // Vérifier que c'est bien un périphérique USB (commence par "USB\")
        std::wstring instanceIdStr(instanceId);
        if (instanceIdStr.find(L"USB\\") != 0) {
            continue;
        }

        USBDevice device;
        device.serial = ExtractSerialFromInstanceId(instanceIdStr);

        // Récupérer le nom du périphérique
        wchar_t deviceName[256];
        if (SetupDiGetDeviceRegistryProperty(
            deviceInfoSet,
            &deviceInfoData,
            SPDRP_FRIENDLYNAME,
            nullptr,
            (PBYTE)deviceName,
            sizeof(deviceName),
            nullptr
        )) {
            device.deviceName = deviceName;
        }

        // Récupérer le fabricant
        wchar_t manufacturer[256];
        if (SetupDiGetDeviceRegistryProperty(
            deviceInfoSet,
            &deviceInfoData,
            SPDRP_MFG,
            nullptr,
            (PBYTE)manufacturer,
            sizeof(manufacturer),
            nullptr
        )) {
            device.manufacturer = manufacturer;
        }

        // Ne garder que les périphériques avec un numéro de série valide
        if (!device.serial.empty() && device.serial != L"UNKNOWN") {
            devices.push_back(device);
        }
    }

    SetupDiDestroyDeviceInfoList(deviceInfoSet);

    // Aussi chercher parmi les volumes pour les périphériques de stockage
    HDEVINFO volumeInfoSet = SetupDiGetClassDevs(
        &GUID_DEVINTERFACE_VOLUME,
        nullptr,
        nullptr,
        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
    );

    if (volumeInfoSet != INVALID_HANDLE_VALUE) {
        SP_DEVICE_INTERFACE_DATA interfaceData;
        interfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

        for (DWORD i = 0; SetupDiEnumDeviceInterfaces(volumeInfoSet, nullptr,
             &GUID_DEVINTERFACE_VOLUME, i, &interfaceData); i++) {

            DWORD requiredSize = 0;
            SetupDiGetDeviceInterfaceDetail(volumeInfoSet, &interfaceData, nullptr, 0, &requiredSize, nullptr);

            if (requiredSize > 0) {
                auto detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(requiredSize);
                if (detailData) {
                    detailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
                    SP_DEVINFO_DATA devInfoData;
                    devInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

                    if (SetupDiGetDeviceInterfaceDetail(volumeInfoSet, &interfaceData,
                        detailData, requiredSize, nullptr, &devInfoData)) {

                        // Récupérer l'ID d'instance
                        wchar_t instanceId[MAX_PATH];
                        if (CM_Get_Device_ID(devInfoData.DevInst, instanceId, MAX_PATH, 0) == CR_SUCCESS) {
                            std::wstring instanceIdStr(instanceId);

                            // Chercher le parent USB
                            DEVINST parentDevInst;
                            if (CM_Get_Parent(&parentDevInst, devInfoData.DevInst, 0) == CR_SUCCESS) {
                                wchar_t parentInstanceId[MAX_PATH];
                                if (CM_Get_Device_ID(parentDevInst, parentInstanceId, MAX_PATH, 0) == CR_SUCCESS) {
                                    std::wstring parentIdStr(parentInstanceId);

                                    // Si le parent est USB, extraire le serial
                                    if (parentIdStr.find(L"USB") != std::wstring::npos ||
                                        parentIdStr.find(L"USBSTOR") != std::wstring::npos) {
                                        std::wstring serial = ExtractSerialFromInstanceId(parentIdStr);

                                        if (!serial.empty() && serial != L"UNKNOWN") {
                                            // Vérifier si ce serial n'est pas déjà dans la liste
                                            bool found = false;
                                            for (const auto& dev : devices) {
                                                if (dev.serial == serial) {
                                                    found = true;
                                                    break;
                                                }
                                            }

                                            if (!found) {
                                                USBDevice device;
                                                device.serial = serial;
                                                device.driveLetter = GetDriveLetter(detailData->DevicePath);
                                                devices.push_back(device);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    free(detailData);
                }
            }
        }

        SetupDiDestroyDeviceInfoList(volumeInfoSet);
    }

    return devices;
}

bool USBDetector::IsSerialPresent(const std::wstring& serial) {
    auto devices = GetConnectedUSBDevices();

    for (const auto& device : devices) {
        if (device.serial == serial) {
            return true;
        }
    }

    return false;
}

bool USBDetector::IsAnySerialPresent(const std::vector<std::wstring>& serials) {
    auto devices = GetConnectedUSBDevices();

    for (const auto& device : devices) {
        for (const auto& serial : serials) {
            if (device.serial == serial) {
                return true;
            }
        }
    }

    return false;
}

std::wstring USBDetector::ExtractSerialFromInstanceId(const std::wstring& instanceId) {
    // Format typique: USB\VID_XXXX&PID_XXXX\SERIAL_NUMBER
    // ou USBSTOR\Disk&Ven_XXX&Prod_YYY&Rev_ZZZ\SERIAL&0

    size_t lastBackslash = instanceId.find_last_of(L'\\');
    if (lastBackslash != std::wstring::npos && lastBackslash < instanceId.length() - 1) {
        std::wstring serial = instanceId.substr(lastBackslash + 1);

        // Nettoyer le serial (enlever &0, &1, etc. à la fin si présent)
        size_t ampersand = serial.find(L'&');
        if (ampersand != std::wstring::npos) {
            serial = serial.substr(0, ampersand);
        }

        // Vérifier que ce n'est pas juste un nombre générique
        if (serial.length() > 0 && serial != L"0" && serial != L"1") {
            return serial;
        }
    }

    return L"UNKNOWN";
}

std::wstring USBDetector::GetDriveLetter(const std::wstring& devicePath) {
    // Ouvrir le handle du volume
    HANDLE hVolume = CreateFile(
        devicePath.c_str(),
        0,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
    );

    if (hVolume == INVALID_HANDLE_VALUE) {
        return L"";
    }

    // Buffer pour les noms de montage
    wchar_t volumeName[MAX_PATH];
    DWORD returnedLength;

    if (GetVolumeNameForVolumeMountPoint(devicePath.c_str(), volumeName, MAX_PATH)) {
        // Chercher les points de montage
        wchar_t mountPoints[MAX_PATH];
        DWORD length = MAX_PATH;

        if (GetVolumePathNamesForVolumeName(volumeName, mountPoints, length, &returnedLength)) {
            CloseHandle(hVolume);
            return mountPoints; // Retourne le premier point de montage
        }
    }

    CloseHandle(hVolume);
    return L"";
}

std::wstring USBDetector::GetDeviceSerial(const std::wstring& devicePath) {
    HDEVINFO deviceInfoSet = SetupDiGetClassDevs(
        nullptr,
        devicePath.c_str(),
        nullptr,
        DIGCF_PRESENT | DIGCF_ALLCLASSES
    );

    if (deviceInfoSet == INVALID_HANDLE_VALUE) {
        return L"";
    }

    SP_DEVINFO_DATA deviceInfoData;
    deviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);

    if (SetupDiEnumDeviceInfo(deviceInfoSet, 0, &deviceInfoData)) {
        wchar_t instanceId[MAX_PATH];
        if (CM_Get_Device_ID(deviceInfoData.DevInst, instanceId, MAX_PATH, 0) == CR_SUCCESS) {
            SetupDiDestroyDeviceInfoList(deviceInfoSet);
            return ExtractSerialFromInstanceId(instanceId);
        }
    }

    SetupDiDestroyDeviceInfoList(deviceInfoSet);
    return L"";
}
