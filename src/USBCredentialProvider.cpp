//
// CUSBCredentialProvider - Implémentation
//

#include "USBCredentialProvider.h"
#include "USBCredential.h"
#include "guid.h"
#include "common.h"
#include <new>

CUSBCredentialProvider::CUSBCredentialProvider() :
    _cRef(1),
    _pCredential(nullptr),
    _cpus(CPUS_INVALID),
    _dwCredUIFlags(0) {
    DllAddRef();
}

CUSBCredentialProvider::~CUSBCredentialProvider() {
    if (_pCredential) {
        _pCredential->Release();
        _pCredential = nullptr;
    }

    DllRelease();
}

HRESULT CUSBCredentialProvider::QueryInterface(REFIID riid, void** ppv) {
    static const QITAB qit[] = {
        QITABENT(CUSBCredentialProvider, ICredentialProvider),
        {0},
    };
    return QISearch(this, qit, riid, ppv);
}

HRESULT CUSBCredentialProvider::SetUsageScenario(
    CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
    DWORD dwFlags) {

    HRESULT hr;

    // Déterminer si le scénario est supporté
    switch (cpus) {
        case CPUS_LOGON:
        case CPUS_UNLOCK_WORKSTATION:
            // Ces scénarios sont supportés
            _cpus = cpus;
            _dwCredUIFlags = dwFlags;
            hr = S_OK;
            break;

        case CPUS_CHANGE_PASSWORD:
            // Pas de changement de mot de passe pour USB auth
            hr = E_NOTIMPL;
            break;

        case CPUS_CREDUI:
            // CredUI n'est pas supporté
            hr = E_NOTIMPL;
            break;

        default:
            hr = E_INVALIDARG;
            break;
    }

    return hr;
}

HRESULT CUSBCredentialProvider::SetSerialization(
    const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs) {
    UNREFERENCED_PARAMETER(pcpcs);
    return E_NOTIMPL;
}

HRESULT CUSBCredentialProvider::Advise(
    ICredentialProviderEvents* pcpe,
    UINT_PTR upAdviseContext) {
    UNREFERENCED_PARAMETER(pcpe);
    UNREFERENCED_PARAMETER(upAdviseContext);
    return E_NOTIMPL;
}

HRESULT CUSBCredentialProvider::UnAdvise() {
    return E_NOTIMPL;
}

HRESULT CUSBCredentialProvider::GetFieldDescriptorCount(DWORD* pdwCount) {
    *pdwCount = SFI_NUM_FIELDS;
    return S_OK;
}

HRESULT CUSBCredentialProvider::GetFieldDescriptorAt(
    DWORD dwIndex,
    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd) {

    *ppcpfd = nullptr;

    if (dwIndex < SFI_NUM_FIELDS) {
        return FieldDescriptorCoAllocCopy(s_rgCredProvFieldDescriptors[dwIndex], ppcpfd);
    }

    return E_INVALIDARG;
}

HRESULT CUSBCredentialProvider::GetCredentialCount(
    DWORD* pdwCount,
    DWORD* pdwDefault,
    BOOL* pbAutoLogonWithDefault) {

    *pdwCount = 0;
    *pdwDefault = CREDENTIAL_PROVIDER_NO_DEFAULT;
    *pbAutoLogonWithDefault = FALSE;

    // Énumérer les credentials
    HRESULT hr = _EnumerateCredentials();

    if (SUCCEEDED(hr)) {
        if (_pCredential) {
            *pdwCount = 1;
            *pdwDefault = 0; // Notre credential unique est le défaut
            *pbAutoLogonWithDefault = FALSE; // Pas d'auto-logon par défaut
        }
    }

    return hr;
}

HRESULT CUSBCredentialProvider::GetCredentialAt(
    DWORD dwIndex,
    ICredentialProviderCredential** ppcpc) {

    *ppcpc = nullptr;

    if (dwIndex == 0 && _pCredential) {
        *ppcpc = _pCredential;
        (*ppcpc)->AddRef();
        return S_OK;
    }

    return E_INVALIDARG;
}

HRESULT CUSBCredentialProvider::_EnumerateCredentials() {
    // Libérer les credentials existantes
    _ReleaseEnumeratedCredentials();

    // Créer une nouvelle credential
    CUSBCredential* pCredential = new (std::nothrow) CUSBCredential();

    if (pCredential) {
        HRESULT hr = pCredential->Initialize(
            _cpus,
            s_rgCredProvFieldDescriptors,
            s_rgFieldStatePairs,
            nullptr, // Pas de username spécifique
            nullptr  // Pas de domaine spécifique
        );

        if (SUCCEEDED(hr)) {
            _pCredential = pCredential;
            return S_OK;
        } else {
            pCredential->Release();
        }

        return hr;
    }

    return E_OUTOFMEMORY;
}

void CUSBCredentialProvider::_ReleaseEnumeratedCredentials() {
    if (_pCredential) {
        _pCredential->Release();
        _pCredential = nullptr;
    }
}

// Fonction de création appelée par la factory
HRESULT CSample_CreateInstance(REFIID riid, void** ppv) {
    HRESULT hr;

    CUSBCredentialProvider* pProvider = new (std::nothrow) CUSBCredentialProvider();

    if (pProvider) {
        hr = pProvider->QueryInterface(riid, ppv);
        pProvider->Release();
    } else {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}
