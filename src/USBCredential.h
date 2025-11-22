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

    IFACEMETHODIMP QueryInterface(__in REFIID riid, __deref_out void** ppv);

    // ICredentialProviderCredential
    IFACEMETHODIMP Advise(__in ICredentialProviderCredentialEvents* pcpce);
    IFACEMETHODIMP UnAdvise();

    IFACEMETHODIMP SetSelected(__out BOOL* pbAutoLogon);
    IFACEMETHODIMP SetDeselected();

    IFACEMETHODIMP GetFieldState(__in DWORD dwFieldID,
                                  __out CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs,
                                  __out CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis);

    IFACEMETHODIMP GetStringValue(__in DWORD dwFieldID, __deref_out PWSTR* ppwsz);
    IFACEMETHODIMP GetBitmapValue(__in DWORD dwFieldID, __out HBITMAP* phbmp);
    IFACEMETHODIMP GetCheckboxValue(__in DWORD dwFieldID, __out BOOL* pbChecked, __deref_out PWSTR* ppwszLabel);
    IFACEMETHODIMP GetSubmitButtonValue(__in DWORD dwFieldID, __out DWORD* pdwAdjacentTo);
    IFACEMETHODIMP GetComboBoxValueCount(__in DWORD dwFieldID, __out DWORD* pcItems, __out DWORD* pdwSelectedItem);
    IFACEMETHODIMP GetComboBoxValueAt(__in DWORD dwFieldID, __in DWORD dwItem, __deref_out PWSTR* ppwszItem);

    IFACEMETHODIMP SetStringValue(__in DWORD dwFieldID, __in PCWSTR pwz);
    IFACEMETHODIMP SetCheckboxValue(__in DWORD dwFieldID, __in BOOL bChecked);
    IFACEMETHODIMP SetComboBoxSelectedValue(__in DWORD dwFieldID, __in DWORD dwSelectedItem);
    IFACEMETHODIMP CommandLinkClicked(__in DWORD dwFieldID);

    IFACEMETHODIMP GetSerialization(__out CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr,
                                     __out CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs,
                                     __deref_out_opt PWSTR* ppwszOptionalStatusText,
                                     __out CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon);
    IFACEMETHODIMP ReportResult(__in NTSTATUS ntsStatus,
                                 __in NTSTATUS ntsSubstatus,
                                 __deref_out_opt PWSTR* ppwszOptionalStatusText,
                                 __out CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon);

public:
    HRESULT Initialize(__in CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
                        __in const CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR* rgcpfd,
                        __in const FIELD_STATE_PAIR* rgfsp,
                        __in_opt PCWSTR pwzUsername,
                        __in_opt PCWSTR pwzDomain);
    CUSBCredential();

    virtual ~CUSBCredential();

private:
    // Vérifie si une clé USB autorisée est présente
    HRESULT CheckUSBPresence();

    // Récupère le nom d'utilisateur actuel de Windows
    HRESULT GetCurrentUsername(__deref_out PWSTR* ppwszUsername);

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
