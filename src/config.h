//
// Configuration Manager - Gestion des numéros de série autorisés via HTTP ou registre
//

#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <winhttp.h>

#pragma comment(lib, "winhttp.lib")

class ConfigManager {
public:
    // Charge les numéros de série autorisés (depuis HTTP ou registre en fallback)
    static bool LoadAuthorizedSerials();

    // Vérifie si un numéro de série est autorisé
    static bool IsSerialAuthorized(const std::wstring& serial);

    // Récupère tous les numéros de série autorisés
    static const std::vector<std::wstring>& GetAuthorizedSerials();

    // Force le rechargement de la configuration
    static void ReloadConfiguration();

private:
    // Charge les serials depuis le serveur HTTP
    static bool LoadFromHttpServer();

    // Charge les serials depuis le registre (fallback)
    static bool LoadFromRegistry();

    // Parse la réponse HTTP (un serial par ligne)
    static std::vector<std::wstring> ParseHttpResponse(const std::string& response);

    // Lit l'URL du serveur depuis le registre
    static std::wstring GetServerUrl();

    // Sauvegarde les serials dans le registre (cache de secours)
    static bool SaveSerialsToRegistry(const std::vector<std::wstring>& serials);

    static std::vector<std::wstring> s_authorizedSerials;
    static bool s_configLoaded;

    static const wchar_t* REGISTRY_PATH;
    static const wchar_t* SERVER_URL_VALUE;
    static const wchar_t* SERIALS_VALUE;
    static const DWORD HTTP_TIMEOUT_MS;
};
