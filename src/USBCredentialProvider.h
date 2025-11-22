//
// CUSBCredentialProvider - Provider principal pour l'authentification USB
//

#pragma once

#include "helpers.h"
#include "dll.h"
#include <credentialprovider.h>
#include <windows.h>

class CUSBCredentialProvider : public ICredentialProvider {
public:
    // IUnknown
    IFACEMETHODIMP_(ULONG) AddRef() {
        return ++_cRef;
    }

    IFACEMETHODIMP_(ULONG) Release() {
        LONG cRef = --_cRef;
        if (!cRef) {
            delete this;
        }
        return cRef;
    }

    IFACEMETHODIMP QueryInterface(_In_ REFIID riid, _Outptr_ void** ppv);

    // ICredentialProvider
    IFACEMETHODIMP SetUsageScenario(_In_ CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, _In_ DWORD dwFlags);
    IFACEMETHODIMP SetSerialization(_In_ const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs);

    IFACEMETHODIMP Advise(_In_ ICredentialProviderEvents* pcpe, _In_ UINT_PTR upAdviseContext);
    IFACEMETHODIMP UnAdvise();

    IFACEMETHODIMP GetFieldDescriptorCount(_Out_ DWORD* pdwCount);
    IFACEMETHODIMP GetFieldDescriptorAt(_In_ DWORD dwIndex, _Outptr_ CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd);

    IFACEMETHODIMP GetCredentialCount(_Out_ DWORD* pdwCount,
                                       _Out_ DWORD* pdwDefault,
                                       _Out_ BOOL* pbAutoLogonWithDefault);
    IFACEMETHODIMP GetCredentialAt(_In_ DWORD dwIndex, _Outptr_ ICredentialProviderCredential** ppcpc);

    friend HRESULT CSample_CreateInstance(_In_ REFIID riid, _Outptr_ void** ppv);

protected:
    CUSBCredentialProvider();
    __override ~CUSBCredentialProvider();

private:
    // Crée les credentials énumérées
    HRESULT _EnumerateCredentials();

    // Libère les credentials
    void _ReleaseEnumeratedCredentials();

private:
    LONG _cRef; // Compteur de référence
    ICredentialProviderCredential* _pCredential; // Notre credential unique
    CREDENTIAL_PROVIDER_USAGE_SCENARIO _cpus; // Le scénario d'utilisation
    DWORD _dwCredUIFlags; // Flags de l'UI
};
