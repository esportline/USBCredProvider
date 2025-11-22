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

    IFACEMETHODIMP QueryInterface(__in REFIID riid, __deref_out void** ppv);

    // ICredentialProvider
    IFACEMETHODIMP SetUsageScenario(__in CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, __in DWORD dwFlags);
    IFACEMETHODIMP SetSerialization(__in const CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs);

    IFACEMETHODIMP Advise(__in ICredentialProviderEvents* pcpe, __in UINT_PTR upAdviseContext);
    IFACEMETHODIMP UnAdvise();

    IFACEMETHODIMP GetFieldDescriptorCount(__out DWORD* pdwCount);
    IFACEMETHODIMP GetFieldDescriptorAt(__in DWORD dwIndex, __deref_out CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR** ppcpfd);

    IFACEMETHODIMP GetCredentialCount(__out DWORD* pdwCount,
                                       __out DWORD* pdwDefault,
                                       __out BOOL* pbAutoLogonWithDefault);
    IFACEMETHODIMP GetCredentialAt(__in DWORD dwIndex, __deref_out ICredentialProviderCredential** ppcpc);

    friend HRESULT CSample_CreateInstance(__in REFIID riid, __deref_out void** ppv);

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
