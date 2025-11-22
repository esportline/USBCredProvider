//
// USB Device Lister - Outil pour lister les clés USB connectées
// Utile pour obtenir les numéros de série à configurer
//

#include "../src/usb_detector.h"
#include <iostream>
#include <iomanip>
#include <windows.h>
#include <io.h>
#include <fcntl.h>

void printSeparator() {
    std::wcout << L"━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
}

void printHeader() {
    printSeparator();
    std::wcout << L"         DÉTECTEUR DE PÉRIPHÉRIQUES USB" << std::endl;
    printSeparator();
    std::wcout << std::endl;
}

int wmain(int argc, wchar_t* argv[]) {
    // Activer l'affichage Unicode dans la console
    _setmode(_fileno(stdout), _O_U16TEXT);

    printHeader();

    std::wcout << L"Scan des périphériques USB en cours..." << std::endl << std::endl;

    auto devices = USBDetector::GetConnectedUSBDevices();

    if (devices.empty()) {
        std::wcout << L"❌ Aucun périphérique USB détecté." << std::endl;
        std::wcout << L"   Assurez-vous qu'au moins une clé USB est branchée." << std::endl;
        return 0;
    }

    std::wcout << L"✓ " << devices.size() << L" périphérique(s) USB détecté(s):" << std::endl << std::endl;

    int index = 1;
    for (const auto& device : devices) {
        std::wcout << L"┌─ Périphérique #" << index << std::endl;

        std::wcout << L"│  📌 Numéro de série: ";
        if (!device.serial.empty()) {
            std::wcout << L"\033[1;32m" << device.serial << L"\033[0m" << std::endl;
        } else {
            std::wcout << L"\033[1;31m(Non disponible)\033[0m" << std::endl;
        }

        if (!device.deviceName.empty()) {
            std::wcout << L"│  📝 Nom: " << device.deviceName << std::endl;
        }

        if (!device.manufacturer.empty()) {
            std::wcout << L"│  🏭 Fabricant: " << device.manufacturer << std::endl;
        }

        if (!device.driveLetter.empty()) {
            std::wcout << L"│  💾 Lecteur: " << device.driveLetter << std::endl;
        }

        std::wcout << L"└─" << std::endl << std::endl;

        index++;
    }

    printSeparator();
    std::wcout << L"CONFIGURATION" << std::endl;
    printSeparator();
    std::wcout << std::endl;
    std::wcout << L"Pour autoriser ces périphériques, ajoutez leurs numéros de série" << std::endl;
    std::wcout << L"dans l'une de ces configurations:" << std::endl << std::endl;

    std::wcout << L"Option 1 - Serveur HTTP (recommandé):" << std::endl;
    std::wcout << L"  Configurez votre serveur pour retourner:" << std::endl;
    for (const auto& device : devices) {
        if (!device.serial.empty()) {
            std::wcout << L"  " << device.serial << std::endl;
        }
    }
    std::wcout << std::endl;

    std::wcout << L"Option 2 - Registre Windows (fallback):" << std::endl;
    std::wcout << L"  1. Ouvrir regedit" << std::endl;
    std::wcout << L"  2. Aller à HKEY_LOCAL_MACHINE\\SOFTWARE\\USBCredProvider" << std::endl;
    std::wcout << L"  3. Créer/modifier la valeur REG_MULTI_SZ 'AuthorizedSerials'" << std::endl;
    std::wcout << L"  4. Ajouter les numéros de série (un par ligne)" << std::endl;
    std::wcout << std::endl;

    printSeparator();

    // Pause si lancé depuis l'explorateur Windows
    if (argc == 1) {
        std::wcout << std::endl << L"Appuyez sur Entrée pour quitter...";
        std::wcin.get();
    }

    return 0;
}
