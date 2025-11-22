//
// CUSBCredential - Implémentation
//

#include "USBCredential.h"
#include "usb_detector.h"
#include "config.h"
#include "guid.h"
#include <shlwapi.h>

CUSBCredential::CUSBCredential() :
    _cRef(1),
    _pCredProvCredentialEvents(nullptr),
    _pwzUsername(nullptr),
    _pwzDomain(nullptr),
    _pwzQualifiedUserName(nullptr),
    _fChecked(FALSE),
    _dwComboIndex(0),
    _fIsLocalUser(false) {
    DllAddRef();

    ZeroMemory(_rgCredProvFieldDescriptors, sizeof(_rgCredProvFieldDescriptors));
    ZeroMemory(_rgFieldStatePairs, sizeof(_rgFieldStatePairs));
    ZeroMemory(_rgFieldStrings, sizeof(_rgFieldStrings));
}

CUSBCredential::~CUSBCredential() {
    if (_rgFieldStrings[SFI_PASSWORD]) {
        SecureZeroMemory(_rgFieldStrings[SFI_PASSWORD], wcslen(_rgFieldStrings[SFI_PASSWORD]) * sizeof(wchar_t));
    }

    for (int i = 0; i < ARRAYSIZE(_rgFieldStrings); i++) {
        CoTaskMemFree(_rgFieldStrings[i]);
        CoTaskMemFree(_rgCredProvFieldDescriptors[i].pszLabel);
    }

    CoTaskMemFree(_pwzUsername);
    CoTaskMemFree(_pwzDomain);
    CoTaskMemFree(_pwzQualifiedUserName);

    DllRelease();
}

HRESULT CUSBCredential::Initialize(
    _In_ CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
    _In_ const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* rgcpfd,
    _In_ const FIELD_STATE_PAIR* rgfsp,
    _In_opt_ PCWSTR pwzUsername,
    _In_opt_ PCWSTR pwzDomain) {

    _cpus = cpus;

    GUID guidProvider;
    CoCreateGuid(&guidProvider);

    // Copier les descripteurs de champs et les états
    for (DWORD i = 0; i < ARRAYSIZE(_rgCredProvFieldDescriptors); i++) {
        _rgFieldStatePairs[i] = rgfsp[i];
        HRESULT hr = FieldDescriptorCopy(rgcpfd[i], &_rgCredProvFieldDescriptors[i]);
        if (FAILED(hr)) {
            return hr;
        }
    }

    // Initialiser les valeurs de chaîne
    HRESULT hr = SHStrDupW(L"USB Credential Provider", &_rgFieldStrings[SFI_LARGE_TEXT]);
    if (FAILED(hr)) {
        return hr;
    }

    hr = SHStrDupW(L"Insérez votre clé USB autorisée", &_rgFieldStrings[SFI_PASSWORD]);
    if (FAILED(hr)) {
        return hr;
    }

    hr = SHStrDupW(L"Connexion", &_rgFieldStrings[SFI_SUBMIT_BUTTON]);
    if (FAILED(hr)) {
        return hr;
    }

    // Stocker le nom d'utilisateur et le domaine
    if (pwzUsername) {
        hr = SHStrDupW(pwzUsername, &_pwzUsername);
        if (FAILED(hr)) {
            return hr;
        }
    } else {
        // Si pas de username fourni, utiliser le username actuel
        hr = GetCurrentUsername(&_pwzUsername);
        if (FAILED(hr)) {
            return hr;
        }
    }

    if (pwzDomain) {
        hr = SHStrDupW(pwzDomain, &_pwzDomain);
    } else {
        hr = SHStrDupW(L".", &_pwzDomain); // Domaine local
        _fIsLocalUser = true;
    }

    return hr;
}

HRESULT CUSBCredential::QueryInterface(_In_ REFIID riid, _Outptr_ void** ppv) {
    static const QITAB qit[] = {
        QITABENT(CUSBCredential, ICredentialProviderCredential),
        {0},
    };
    return QISearch(this, qit, riid, ppv);
}

HRESULT CUSBCredential::Advise(_In_ ICredentialProviderCredentialEvents* pcpce) {
    if (_pCredProvCredentialEvents != nullptr) {
        _pCredProvCredentialEvents->Release();
    }
    _pCredProvCredentialEvents = pcpce;
    _pCredProvCredentialEvents->AddRef();
    return S_OK;
}

HRESULT CUSBCredential::UnAdvise() {
    if (_pCredProvCredentialEvents) {
        _pCredProvCredentialEvents->Release();
    }
    _pCredProvCredentialEvents = nullptr;
    return S_OK;
}

HRESULT CUSBCredential::SetSelected(_Out_ BOOL* pbAutoLogon) {
    *pbAutoLogon = FALSE;

    // Vérifier si une clé USB autorisée est présente
    HRESULT hr = CheckUSBPresence();

    if (SUCCEEDED(hr)) {
        // Si une clé est détectée, mettre à jour le texte
        if (_pCredProvCredentialEvents) {
            CoTaskMemFree(_rgFieldStrings[SFI_PASSWORD]);
            SHStrDupW(L"Clé USB détectée - Cliquez pour vous connecter", &_rgFieldStrings[SFI_PASSWORD]);
            _pCredProvCredentialEvents->SetFieldString(this, SFI_PASSWORD, _rgFieldStrings[SFI_PASSWORD]);
        }
        *pbAutoLogon = TRUE; // Connexion automatique si clé détectée
    } else {
        if (_pCredProvCredentialEvents) {
            CoTaskMemFree(_rgFieldStrings[SFI_PASSWORD]);
            SHStrDupW(L"Clé USB non détectée - Insérez votre clé", &_rgFieldStrings[SFI_PASSWORD]);
            _pCredProvCredentialEvents->SetFieldString(this, SFI_PASSWORD, _rgFieldStrings[SFI_PASSWORD]);
        }
    }

    return S_OK;
}

HRESULT CUSBCredential::SetDeselected() {
    return S_OK;
}

HRESULT CUSBCredential::GetFieldState(
    _In_ DWORD dwFieldID,
    _Out_ CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs,
    _Out_ CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis) {

    if (dwFieldID < ARRAYSIZE(_rgFieldStatePairs)) {
        *pcpfs = _rgFieldStatePairs[dwFieldID].cpfs;
        *pcpfis = _rgFieldStatePairs[dwFieldID].cpfis;
        return S_OK;
    }

    return E_INVALIDARG;
}

HRESULT CUSBCredential::GetStringValue(_In_ DWORD dwFieldID, _Outptr_ PWSTR* ppwsz) {
    *ppwsz = nullptr;

    if (dwFieldID < ARRAYSIZE(_rgFieldStrings) && _rgFieldStrings[dwFieldID]) {
        return SHStrDupW(_rgFieldStrings[dwFieldID], ppwsz);
    }

    return E_INVALIDARG;
}

HRESULT CUSBCredential::GetBitmapValue(_In_ DWORD dwFieldID, _Out_ HBITMAP* phbmp) {
    if (dwFieldID == SFI_TILEIMAGE) {
        *phbmp = nullptr;
        return S_OK;
    }
    return E_INVALIDARG;
}

HRESULT CUSBCredential::GetCheckboxValue(_In_ DWORD dwFieldID, _Out_ BOOL* pbChecked, _Outptr_ PWSTR* ppwszLabel) {
    *ppwszLabel = nullptr;
    *pbChecked = FALSE;

    if (dwFieldID == SFI_CHECKBOX) {
        *pbChecked = _fChecked;
        return SHStrDupW(L"Option", ppwszLabel);
    }

    return E_INVALIDARG;
}

HRESULT CUSBCredential::GetSubmitButtonValue(_In_ DWORD dwFieldID, _Out_ DWORD* pdwAdjacentTo) {
    if (dwFieldID == SFI_SUBMIT_BUTTON) {
        *pdwAdjacentTo = SFI_PASSWORD;
        return S_OK;
    }
    return E_INVALIDARG;
}

HRESULT CUSBCredential::GetComboBoxValueCount(_In_ DWORD dwFieldID, _Out_ DWORD* pcItems, _Out_ DWORD* pdwSelectedItem) {
    if (dwFieldID == SFI_COMBOBOX) {
        *pcItems = ARRAYSIZE(s_rgComboBoxStrings);
        *pdwSelectedItem = _dwComboIndex;
        return S_OK;
    }
    return E_INVALIDARG;
}

HRESULT CUSBCredential::GetComboBoxValueAt(_In_ DWORD dwFieldID, _In_ DWORD dwItem, _Outptr_ PWSTR* ppwszItem) {
    *ppwszItem = nullptr;

    if (dwFieldID == SFI_COMBOBOX && dwItem < ARRAYSIZE(s_rgComboBoxStrings)) {
        return SHStrDupW(s_rgComboBoxStrings[dwItem], ppwszItem);
    }

    return E_INVALIDARG;
}

HRESULT CUSBCredential::SetStringValue(_In_ DWORD dwFieldID, _In_ PCWSTR pwz) {
    if (dwFieldID == SFI_PASSWORD || dwFieldID == SFI_EDIT_TEXT) {
        CoTaskMemFree(_rgFieldStrings[dwFieldID]);
        return SHStrDupW(pwz, &_rgFieldStrings[dwFieldID]);
    }
    return E_INVALIDARG;
}

HRESULT CUSBCredential::SetCheckboxValue(_In_ DWORD dwFieldID, _In_ BOOL bChecked) {
    if (dwFieldID == SFI_CHECKBOX) {
        _fChecked = bChecked;
        return S_OK;
    }
    return E_INVALIDARG;
}

HRESULT CUSBCredential::SetComboBoxSelectedValue(_In_ DWORD dwFieldID, _In_ DWORD dwSelectedItem) {
    if (dwFieldID == SFI_COMBOBOX) {
        _dwComboIndex = dwSelectedItem;
        return S_OK;
    }
    return E_INVALIDARG;
}

HRESULT CUSBCredential::CommandLinkClicked(_In_ DWORD dwFieldID) {
    return E_NOTIMPL;
}

HRESULT CUSBCredential::GetSerialization(
    _Out_ CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr,
    _Out_ CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs,
    _Outptr_opt_ PWSTR* ppwszOptionalStatusText,
    _Out_ CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon) {

    *pcpgsr = CPGSR_NO_CREDENTIAL_NOT_FINISHED;
    *ppwszOptionalStatusText = nullptr;
    *pcpsiOptionalStatusIcon = CPSI_NONE;

    ZeroMemory(pcpcs, sizeof(*pcpcs));

    // Vérifier la présence de la clé USB
    HRESULT hr = CheckUSBPresence();

    if (FAILED(hr)) {
        SHStrDupW(L"Clé USB non autorisée ou non détectée", ppwszOptionalStatusText);
        *pcpsiOptionalStatusIcon = CPSI_ERROR;
        return S_OK;
    }

    // La clé USB est présente et autorisée, créer les credentials
    PWSTR pwzProtectedPassword;
    hr = ProtectIfNecessaryAndCopyPassword(L"", _cpus, &pwzProtectedPassword);

    if (FAILED(hr)) {
        return hr;
    }

    // Créer le paquet Kerberos pour l'authentification
    KERB_INTERACTIVE_UNLOCK_LOGON kiul;
    hr = KerbInteractiveUnlockLogonInit(_pwzDomain, _pwzUsername, pwzProtectedPassword, _cpus, &kiul);

    if (SUCCEEDED(hr)) {
        BYTE* rgbSerialization;
        DWORD cbSerialization;

        hr = KerbInteractiveUnlockLogonPack(kiul, &rgbSerialization, &cbSerialization);

        if (SUCCEEDED(hr)) {
            pcpcs->rgbSerialization = rgbSerialization;
            pcpcs->cbSerialization = cbSerialization;

            ULONG ulAuthPackage;
            hr = RetrieveNegotiateAuthPackage(&ulAuthPackage);

            if (SUCCEEDED(hr)) {
                pcpcs->ulAuthenticationPackage = ulAuthPackage;
                pcpcs->clsidCredentialProvider = CLSID_CSample;

                *pcpgsr = CPGSR_RETURN_CREDENTIAL_FINISHED;
                SHStrDupW(L"Authentification par clé USB réussie", ppwszOptionalStatusText);
                *pcpsiOptionalStatusIcon = CPSI_SUCCESS;
            }
        }
    }

    CoTaskMemFree(pwzProtectedPassword);
    return hr;
}

HRESULT CUSBCredential::ReportResult(
    _In_ NTSTATUS ntsStatus,
    _In_ NTSTATUS ntsSubstatus,
    _Outptr_opt_ PWSTR* ppwszOptionalStatusText,
    _Out_ CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon) {

    *ppwszOptionalStatusText = nullptr;
    *pcpsiOptionalStatusIcon = CPSI_NONE;

    return S_OK;
}

HRESULT CUSBCredential::CheckUSBPresence() {
    // Charger la configuration
    ConfigManager::LoadAuthorizedSerials();

    // Récupérer les numéros de série autorisés
    const auto& authorizedSerials = ConfigManager::GetAuthorizedSerials();

    if (authorizedSerials.empty()) {
        return E_FAIL; // Pas de configuration
    }

    // Vérifier si une des clés autorisées est présente
    if (USBDetector::IsAnySerialPresent(authorizedSerials)) {
        return S_OK;
    }

    return E_FAIL;
}

HRESULT CUSBCredential::GetCurrentUsername(_Outptr_ PWSTR* ppwszUsername) {
    *ppwszUsername = nullptr;

    wchar_t username[UNLEN + 1];
    DWORD usernameLen = UNLEN + 1;

    if (GetUserName(username, &usernameLen)) {
        return SHStrDupW(username, ppwszUsername);
    }

    return E_FAIL;
}
