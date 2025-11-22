//
// CUSBCredential - Représente une tentative de connexion avec authentification USB
//

#pragma once

#include "helpers.h"
#include "common.h"
#include "dll.h"
#include <credentialprovider.h>

class CUSBCredential : public ICredentialProviderCredential {
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

    // ICredentialProviderCredential
    IFACEMETHODIMP Advise(_In_ ICredentialProviderCredentialEvents* pcpce);
    IFACEMETHODIMP UnAdvise();

    IFACEMETHODIMP SetSelected(_Out_ BOOL* pbAutoLogon);
    IFACEMETHODIMP SetDeselected();

    IFACEMETHODIMP GetFieldState(_In_ DWORD dwFieldID,
                                  _Out_ CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs,
                                  _Out_ CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis);

    IFACEMETHODIMP GetStringValue(_In_ DWORD dwFieldID, _Outptr_ PWSTR* ppwsz);
    IFACEMETHODIMP GetBitmapValue(_In_ DWORD dwFieldID, _Out_ HBITMAP* phbmp);
    IFACEMETHODIMP GetCheckboxValue(_In_ DWORD dwFieldID, _Out_ BOOL* pbChecked, _Outptr_ PWSTR* ppwszLabel);
    IFACEMETHODIMP GetSubmitButtonValue(_In_ DWORD dwFieldID, _Out_ DWORD* pdwAdjacentTo);
    IFACEMETHODIMP GetComboBoxValueCount(_In_ DWORD dwFieldID, _Out_ DWORD* pcItems, _Out_ DWORD* pdwSelectedItem);
    IFACEMETHODIMP GetComboBoxValueAt(_In_ DWORD dwFieldID, _In_ DWORD dwItem, _Outptr_ PWSTR* ppwszItem);

    IFACEMETHODIMP SetStringValue(_In_ DWORD dwFieldID, _In_ PCWSTR pwz);
    IFACEMETHODIMP SetCheckboxValue(_In_ DWORD dwFieldID, _In_ BOOL bChecked);
    IFACEMETHODIMP SetComboBoxSelectedValue(_In_ DWORD dwFieldID, _In_ DWORD dwSelectedItem);
    IFACEMETHODIMP CommandLinkClicked(_In_ DWORD dwFieldID);

    IFACEMETHODIMP GetSerialization(_Out_ CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr,
                                     _Out_ CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs,
                                     _Outptr_opt_ PWSTR* ppwszOptionalStatusText,
                                     _Out_ CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon);
    IFACEMETHODIMP ReportResult(_In_ NTSTATUS ntsStatus,
                                 _In_ NTSTATUS ntsSubstatus,
                                 _Outptr_opt_ PWSTR* ppwszOptionalStatusText,
                                 _Out_ CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon);

public:
    HRESULT Initialize(_In_ CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
                        _In_ const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* rgcpfd,
                        _In_ const FIELD_STATE_PAIR* rgfsp,
                        _In_opt_ PCWSTR pwzUsername,
                        _In_opt_ PCWSTR pwzDomain);
    CUSBCredential();

    virtual ~CUSBCredential();

private:
    // Vérifie si une clé USB autorisée est présente
    HRESULT CheckUSBPresence();

    // Récupère le nom d'utilisateur actuel de Windows
    HRESULT GetCurrentUsername(_Outptr_ PWSTR* ppwszUsername);

private:
    LONG _cRef;
    CREDENTIAL_PROVIDER_USAGE_SCENARIO _cpus; // Le scénario d'utilisation pour lequel nous avons été énumérés
    CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR _rgCredProvFieldDescriptors[SFI_NUM_FIELDS]; // Un tableau contenant le type et le nom de chaque champ
    FIELD_STATE_PAIR _rgFieldStatePairs[SFI_NUM_FIELDS]; // Un tableau indiquant quels champs sont actifs et interactifs
    PWSTR _rgFieldStrings[SFI_NUM_FIELDS]; // Un tableau contenant les valeurs de chaîne des champs
    ICredentialProviderCredentialEvents* _pCredProvCredentialEvents; // Pointeur vers notre UI

    PWSTR _pwzUsername; // Nom d'utilisateur
    PWSTR _pwzDomain; // Domaine
    BOOL _fChecked; // Checkbox
    DWORD _dwComboIndex; // Combobox

    PWSTR _pwzQualifiedUserName; // Domaine\nom d'utilisateur
    bool _fIsLocalUser; // Si l'utilisateur est local ou du domaine
};
