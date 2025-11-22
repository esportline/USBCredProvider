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

    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv);

    // ICredentialProvider
    IFACEMETHODIMP SetUsageScenario(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags);
    IFACEMETHODIMP SetSerialization(const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs);

    IFACEMETHODIMP Advise(ICredentialProviderEvents* pcpe, UINT_PTR upAdviseContext);
    IFACEMETHODIMP UnAdvise();

    IFACEMETHODIMP GetFieldDescriptorCount(DWORD* pdwCount);
    IFACEMETHODIMP GetFieldDescriptorAt(DWORD dwIndex, CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd);

    IFACEMETHODIMP GetCredentialCount(DWORD* pdwCount,
                                       DWORD* pdwDefault,
                                       BOOL* pbAutoLogonWithDefault);
    IFACEMETHODIMP GetCredentialAt(DWORD dwIndex, ICredentialProviderCredential** ppcpc);

    friend HRESULT CSample_CreateInstance(REFIID riid, void** ppv);

protected:
    CUSBCredentialProvider();
    ~CUSBCredentialProvider();

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
