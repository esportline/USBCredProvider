//
// Configuration Manager - Implémentation
//

#include "config.h"
#include <shlwapi.h>
#include <sstream>
#include <algorithm>
#include <memory>

// Initialisation des membres statiques
std::vector<std::wstring> ConfigManager::s_authorizedSerials;
bool ConfigManager::s_configLoaded = false;
const wchar_t* ConfigManager::REGISTRY_PATH = L"SOFTWARE\\USBCredProvider";
const wchar_t* ConfigManager::SERVER_URL_VALUE = L"ServerUrl";
const wchar_t* ConfigManager::SERIALS_VALUE = L"AuthorizedSerials";
const DWORD ConfigManager::HTTP_TIMEOUT_MS = 5000; // 5 secondes

bool ConfigManager::LoadAuthorizedSerials() {
    if (s_configLoaded) {
        return true;
    }

    s_authorizedSerials.clear();

    // Tenter de charger depuis le serveur HTTP
    if (LoadFromHttpServer()) {
        s_configLoaded = true;
        return true;
    }

    // En cas d'échec, fallback sur le registre
    if (LoadFromRegistry()) {
        s_configLoaded = true;
        return true;
    }

    s_configLoaded = true;
    return false;
}

std::wstring ConfigManager::GetServerUrl() {
    HKEY hKey;
    LONG result = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        REGISTRY_PATH,
        0,
        KEY_READ,
        &hKey
    );

    if (result != ERROR_SUCCESS) {
        return L"";
    }

    wchar_t url[1024];
    DWORD urlSize = sizeof(url);
    DWORD type;

    result = RegQueryValueEx(
        hKey,
        SERVER_URL_VALUE,
        nullptr,
        &type,
        (LPBYTE)url,
        &urlSize
    );

    RegCloseKey(hKey);

    if (result == ERROR_SUCCESS && type == REG_SZ) {
        return url;
    }

    return L"";
}

bool ConfigManager::LoadFromHttpServer() {
    std::wstring serverUrl = GetServerUrl();
    if (serverUrl.empty()) {
        return false;
    }

    // Parser l'URL pour extraire host, path, etc.
    URL_COMPONENTS urlComp;
    ZeroMemory(&urlComp, sizeof(urlComp));
    urlComp.dwStructSize = sizeof(urlComp);

    wchar_t hostName[256];
    wchar_t urlPath[1024];

    urlComp.lpszHostName = hostName;
    urlComp.dwHostNameLength = sizeof(hostName) / sizeof(hostName[0]);
    urlComp.lpszUrlPath = urlPath;
    urlComp.dwUrlPathLength = sizeof(urlPath) / sizeof(urlPath[0]);

    if (!WinHttpCrackUrl(serverUrl.c_str(), (DWORD)serverUrl.length(), 0, &urlComp)) {
        return false;
    }

    // Ouvrir une session WinHTTP
    HINTERNET hSession = WinHttpOpen(
        L"USBCredProvider/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        0
    );

    if (!hSession) {
        return false;
    }

    // Définir les timeouts
    WinHttpSetTimeouts(hSession, HTTP_TIMEOUT_MS, HTTP_TIMEOUT_MS, HTTP_TIMEOUT_MS, HTTP_TIMEOUT_MS);

    // Se connecter au serveur
    HINTERNET hConnect = WinHttpConnect(
        hSession,
        hostName,
        urlComp.nPort,
        0
    );

    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Ouvrir la requête
    DWORD flags = (urlComp.nScheme == INTERNET_SCHEME_HTTPS) ? WINHTTP_FLAG_SECURE : 0;

    HINTERNET hRequest = WinHttpOpenRequest(
        hConnect,
        L"GET",
        urlPath,
        nullptr,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        flags
    );

    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Envoyer la requête
    BOOL result = WinHttpSendRequest(
        hRequest,
        WINHTTP_NO_ADDITIONAL_HEADERS,
        0,
        WINHTTP_NO_REQUEST_DATA,
        0,
        0,
        0
    );

    if (!result) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Recevoir la réponse
    result = WinHttpReceiveResponse(hRequest, nullptr);
    if (!result) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Vérifier le code de statut HTTP
    DWORD statusCode = 0;
    DWORD statusCodeSize = sizeof(statusCode);
    WinHttpQueryHeaders(
        hRequest,
        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
        nullptr,
        &statusCode,
        &statusCodeSize,
        nullptr
    );

    if (statusCode != 200) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    // Lire les données
    std::string responseData;
    DWORD bytesAvailable = 0;
    DWORD bytesRead = 0;
    char buffer[4096];

    while (WinHttpQueryDataAvailable(hRequest, &bytesAvailable) && bytesAvailable > 0) {
        DWORD bytesToRead = (bytesAvailable < sizeof(buffer)) ? bytesAvailable : sizeof(buffer);

        if (WinHttpReadData(hRequest, buffer, bytesToRead, &bytesRead)) {
            responseData.append(buffer, bytesRead);
        } else {
            break;
        }
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    if (responseData.empty()) {
        return false;
    }

    // Parser la réponse
    s_authorizedSerials = ParseHttpResponse(responseData);

    // Sauvegarder dans le registre comme cache
    if (!s_authorizedSerials.empty()) {
        SaveSerialsToRegistry(s_authorizedSerials);
        return true;
    }

    return false;
}

std::vector<std::wstring> ConfigManager::ParseHttpResponse(const std::string& response) {
    std::vector<std::wstring> serials;
    std::istringstream stream(response);
    std::string line;

    while (std::getline(stream, line)) {
        // Supprimer les espaces et retours chariot
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
        line.erase(std::remove(line.begin(), line.end(), '\t'), line.end());

        if (line.empty()) {
            continue;
        }

        // Convertir en wstring
        int wideSize = MultiByteToWideChar(CP_UTF8, 0, line.c_str(), -1, nullptr, 0);
        if (wideSize > 0) {
            auto wideStr = std::make_unique<wchar_t[]>(wideSize);
            MultiByteToWideChar(CP_UTF8, 0, line.c_str(), -1, wideStr.get(), wideSize);
            serials.push_back(wideStr.get());
        }
    }

    return serials;
}

bool ConfigManager::LoadFromRegistry() {
    HKEY hKey;
    LONG result = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        REGISTRY_PATH,
        0,
        KEY_READ,
        &hKey
    );

    if (result != ERROR_SUCCESS) {
        return false;
    }

    DWORD serialsSize = 0;
    DWORD type;

    result = RegQueryValueEx(hKey, SERIALS_VALUE, nullptr, &type, nullptr, &serialsSize);

    if (result == ERROR_SUCCESS && type == REG_MULTI_SZ && serialsSize > 0) {
        auto serialsBuffer = std::make_unique<wchar_t[]>(serialsSize / sizeof(wchar_t));

        if (RegQueryValueEx(hKey, SERIALS_VALUE, nullptr, &type,
            (LPBYTE)serialsBuffer.get(), &serialsSize) == ERROR_SUCCESS) {

            // Parser REG_MULTI_SZ (chaînes séparées par \0, terminées par \0\0)
            wchar_t* current = serialsBuffer.get();
            while (*current) {
                s_authorizedSerials.push_back(current);
                current += wcslen(current) + 1;
            }
        }
    }

    RegCloseKey(hKey);
    return !s_authorizedSerials.empty();
}

bool ConfigManager::SaveSerialsToRegistry(const std::vector<std::wstring>& serials) {
    if (serials.empty()) {
        return false;
    }

    HKEY hKey;
    LONG result = RegCreateKeyEx(
        HKEY_LOCAL_MACHINE,
        REGISTRY_PATH,
        0,
        nullptr,
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        nullptr,
        &hKey,
        nullptr
    );

    if (result != ERROR_SUCCESS) {
        return false;
    }

    // Calculer la taille nécessaire pour REG_MULTI_SZ
    size_t totalSize = 1; // Pour le \0\0 final
    for (const auto& serial : serials) {
        totalSize += serial.length() + 1; // +1 pour le \0
    }

    auto buffer = std::make_unique<wchar_t[]>(totalSize);
    wchar_t* current = buffer.get();

    for (const auto& serial : serials) {
        wcscpy_s(current, serial.length() + 1, serial.c_str());
        current += serial.length() + 1;
    }
    *current = L'\0'; // Double \0 final

    result = RegSetValueEx(
        hKey,
        SERIALS_VALUE,
        0,
        REG_MULTI_SZ,
        (const BYTE*)buffer.get(),
        (DWORD)(totalSize * sizeof(wchar_t))
    );

    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
}

bool ConfigManager::IsSerialAuthorized(const std::wstring& serial) {
    if (!s_configLoaded) {
        LoadAuthorizedSerials();
    }

    for (const auto& authorizedSerial : s_authorizedSerials) {
        if (authorizedSerial == serial) {
            return true;
        }
    }

    return false;
}

const std::vector<std::wstring>& ConfigManager::GetAuthorizedSerials() {
    if (!s_configLoaded) {
        LoadAuthorizedSerials();
    }

    return s_authorizedSerials;
}

void ConfigManager::ReloadConfiguration() {
    s_configLoaded = false;
    s_authorizedSerials.clear();
    LoadAuthorizedSerials();
}
