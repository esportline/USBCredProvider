//
// USB Detector - Module de détection et vérification des clés USB
//

#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <memory>

class USBDetector {
public:
    struct USBDevice {
        std::wstring serial;
        std::wstring deviceName;
        std::wstring manufacturer;
        std::wstring driveLetter;
    };

    // Récupère toutes les clés USB connectées
    static std::vector<USBDevice> GetConnectedUSBDevices();

    // Vérifie si un numéro de série spécifique est présent
    static bool IsSerialPresent(const std::wstring& serial);

    // Vérifie si au moins un des numéros de série de la liste est présent
    static bool IsAnySerialPresent(const std::vector<std::wstring>& serials);

    // Récupère le numéro de série d'un périphérique à partir de son chemin
    static std::wstring GetDeviceSerial(const std::wstring& devicePath);

private:
    // Extrait le numéro de série depuis les propriétés du périphérique
    static std::wstring ExtractSerialFromInstanceId(const std::wstring& instanceId);

    // Récupère la lettre de lecteur associée à un périphérique
    static std::wstring GetDriveLetter(const std::wstring& devicePath);
};
